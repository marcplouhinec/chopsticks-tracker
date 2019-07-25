#include "VideoFramePainterDetectedObjectsImpl.hpp"

#include <math.h>
#include <stdexcept>
#include <string>

using namespace model;
using namespace service;
using std::runtime_error;
using std::to_string;
using std::round;

void VideoFramePainterDetectedObjectsImpl::paintOnFrame(
    int frameIndex, cv::Mat& frame, FrameOffset accumulatedFrameOffset) {

    int frameMargin = configuration.renderingVideoFrameMarginsInPixels;
    bool showTips = configuration.renderingDetectedObjectsPainterShowTips;
    bool showChopsticks = configuration.renderingDetectedObjectsPainterShowChopsticks;
    bool showArms = configuration.renderingDetectedObjectsPainterShowArms;

    FrameDetectionResult& frameDetectionResult = findFrameDetectionResultByFrameIndex(frameIndex);

    for (DetectedObject& detectedObject : frameDetectionResult.detectedObjects) {
        if (!showTips && DetectedObjectTypeHelper::isTip(detectedObject.objectType)) {
            continue;
        }
        if (!showChopsticks && detectedObject.objectType == DetectedObjectType::CHOPSTICK) {
            continue;
        }
        if (!showArms && detectedObject.objectType == DetectedObjectType::ARM) {
            continue;
        }

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
                round(detectedObject.x + frameMargin - accumulatedFrameOffset.dx),
                round(detectedObject.y + frameMargin - accumulatedFrameOffset.dy),
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