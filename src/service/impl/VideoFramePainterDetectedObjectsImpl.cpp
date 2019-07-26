#include "VideoFramePainterDetectedObjectsImpl.hpp"

#include <math.h>

using namespace model;
using namespace service;
using std::round;
using std::vector;

void VideoFramePainterDetectedObjectsImpl::paintOnFrame(
    const cv::Mat& frame,
    const vector<DetectedObject>& detectedObjects,
    const FrameOffset accumulatedFrameOffset) {

    int frameMargin = configuration.renderingVideoFrameMarginsInPixels;
    bool showTips = configuration.renderingDetectedObjectsPainterShowTips;
    bool showChopsticks = configuration.renderingDetectedObjectsPainterShowChopsticks;
    bool showArms = configuration.renderingDetectedObjectsPainterShowArms;

    for (const DetectedObject& detectedObject : detectedObjects) {
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