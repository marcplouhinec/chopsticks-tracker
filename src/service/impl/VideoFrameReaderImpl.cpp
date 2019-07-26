#include <algorithm>
#include <string>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include "VideoFrameReaderImpl.hpp"

using namespace model;
using namespace service;
using std::max;
using std::make_unique;
using std::min;
using std::to_string;
using std::out_of_range;
using std::runtime_error;

VideoFrameReaderImpl::~VideoFrameReaderImpl() {
    if (pVideoCapture) {
        pVideoCapture->release();
    }
}

const cv::Mat VideoFrameReaderImpl::readFrameAt(int frameIndex) {
    if (frameIndex == currentFrameIndex) {
        return currentFrame;
    }

    initVideoCaptureIfNecessary();

    // Rewind if necessary
    if (frameIndex < currentFrameIndex) {
        currentFrameIndex = max(0, frameIndex - 20);
        pVideoCapture->set(cv::CAP_PROP_POS_FRAMES, currentFrameIndex);
        // Note: we cannot rewind to the exact position because FFMpeg (used by OpenCV)
        // has some trouble when the frameIndex doesn't point exactly to a key frame.
    }

    // Read the frame image
    while (currentFrameIndex < frameIndex) {
        currentFrameIndex++;

        if (!pVideoCapture->grab()) {
            throw out_of_range("Unable to read the frame " + to_string(frameIndex) + ".");
        }

        if (currentFrameIndex == frameIndex) {
            pVideoCapture->retrieve(currentFrame);
        }
    }

    // Crop the image if necessary
    if (configuration.inputVideoCrop) {
        cv::Rect cropROI(0, 0, videoProperties.frameWidth, videoProperties.frameHeight);
        if (currentFrame.cols > currentFrame.rows) {
            cropROI.x = (currentFrame.cols - videoProperties.frameWidth) / 2;
        } else {
            cropROI.y = (currentFrame.rows - videoProperties.frameHeight) / 2;
        }
        currentFrame = currentFrame(cropROI);
    }
    
    return currentFrame;
}

const VideoProperties VideoFrameReaderImpl::getVideoProperties() {
    initVideoCaptureIfNecessary();

    return videoProperties;
}

void VideoFrameReaderImpl::initVideoCaptureIfNecessary() {
    if (!pVideoCapture) {
        pVideoCapture = make_unique<cv::VideoCapture>(cv::VideoCapture(videoPath.string()));

        if (!pVideoCapture->isOpened()) {
            throw runtime_error("Unable to open the video: " + videoPath.string());
        }

        int frameWidth = pVideoCapture->get(cv::CAP_PROP_FRAME_WIDTH);
        int frameHeight = pVideoCapture->get(cv::CAP_PROP_FRAME_HEIGHT);

        if (configuration.inputVideoCrop) {
            int frameSize = min(frameWidth, frameHeight);
            frameWidth = frameSize;
            frameHeight = frameSize;
        }

        videoProperties = VideoProperties(
            pVideoCapture->get(cv::CAP_PROP_FRAME_COUNT),
            pVideoCapture->get(cv::CAP_PROP_FPS),
            frameWidth,
            frameHeight
        );
    }
}