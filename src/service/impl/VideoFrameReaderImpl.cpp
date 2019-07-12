#include <algorithm>
#include <string>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include "VideoFrameReaderImpl.hpp"

using namespace service;
using std::max;
using std::to_string;
using std::out_of_range;
using std::runtime_error;

VideoFrameReaderImpl::~VideoFrameReaderImpl() {
    if (pVideoCapture != nullptr) {
        pVideoCapture->release();
        delete pVideoCapture;
    }
}

cv::Mat VideoFrameReaderImpl::getFrameAt(int frameIndex) {
    if (frameIndex == currentFrameIndex) {
        return currentFrame;
    }

    // Rewind if necessary
    if (frameIndex < currentFrameIndex) {
        currentFrameIndex = max(0, frameIndex - 20);
        getVideoCapture()->set(cv::CAP_PROP_POS_FRAMES, currentFrameIndex);
        // Note: we cannot rewind to the exact position because FFMpeg (used by OpenCV)
        // has some trouble when the frameIndex doesn't point exactly to a key frame.
    }

    // Read the frame image
    while (currentFrameIndex < frameIndex) {
        currentFrameIndex++;

        if (!getVideoCapture()->grab()) {
            throw out_of_range("Unable to read the frame " + to_string(frameIndex) + ".");
        }

        if (currentFrameIndex == frameIndex) {
            getVideoCapture()->retrieve(currentFrame);
        }
    }

    return currentFrame;
}

int VideoFrameReaderImpl::getNbFrames() {
    if (nbFrames == -1) {
        nbFrames = getVideoCapture()->get(cv::CAP_PROP_FRAME_COUNT);
    }
    return nbFrames;
}

int VideoFrameReaderImpl::getFps() {
    if (fps == -1) {
        fps = getVideoCapture()->get(cv::CAP_PROP_FPS);
    }
    return fps;
}

int VideoFrameReaderImpl::getFrameWidth() {
    if (frameWidth == -1) {
        frameWidth = getVideoCapture()->get(cv::CAP_PROP_FRAME_WIDTH);
    }
    return frameWidth;
}

int VideoFrameReaderImpl::getFrameHeight() {
    if (frameHeight == -1) {
        frameHeight = getVideoCapture()->get(cv::CAP_PROP_FRAME_HEIGHT);
    }
    return frameHeight;
}

cv::VideoCapture* VideoFrameReaderImpl::getVideoCapture() {
    if (pVideoCapture == nullptr) {
        pVideoCapture = new cv::VideoCapture(videoPath.string());

        if (!pVideoCapture->isOpened()) {
            throw runtime_error("Unable to open the video: " + videoPath.string());
        }
    }
    return pVideoCapture;
}