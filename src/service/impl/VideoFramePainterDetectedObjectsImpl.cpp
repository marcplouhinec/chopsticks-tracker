#include "VideoFramePainterDetectedObjectsImpl.hpp"

#include <string>
#include <stdexcept>

using namespace model;
using namespace service;
using std::runtime_error;
using std::to_string;

void VideoFramePainterDetectedObjectsImpl::paintOnFrame(int frameIndex, cv::Mat& frame) {
    int frameMargin = configurationReader.getRenderingVideoFrameMarginsInPixels();
    FrameDetectionResult& frameDetectionResult = findFrameDetectionResultByFrameIndex(frameIndex);

    for (DetectedObject& detectedObject : frameDetectionResult.detectedObjects) {
        cv::Scalar color;
        switch (detectedObject.objectType) {
            case DetectedObjectType::ARM:
                color = yellowColor;
                break;
            case DetectedObjectType::BIG_TIP:
                color = greenColor;
                break;
            case DetectedObjectType::SMALL_TIP:
                color = magentaColor;
                break;
            case DetectedObjectType::CHOPSTICK:
            default:
                color = orangeColor;
                break;
        }

        cv::rectangle(
            frame,
            cv::Rect(
                detectedObject.x + frameMargin,
                detectedObject.y + frameMargin,
                detectedObject.width,
                detectedObject.height),
            color);
    }
}

FrameDetectionResult& VideoFramePainterDetectedObjectsImpl::findFrameDetectionResultByFrameIndex(
    int frameIndex) {
    for (auto it = frameDetectionResults.rbegin(); it != frameDetectionResults.rend(); it++) {
        FrameDetectionResult& result = *it;
        if (result.frameIndex == frameIndex) {
            return result;
        }
    }

    throw runtime_error(
        "Unable to find the FrameDetectionResult with the frame index: " + to_string(frameIndex));
}