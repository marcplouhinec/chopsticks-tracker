#include "VideoFramePainterTrackedObjectsImpl.hpp"

using namespace model;
using namespace service;

void VideoFramePainterTrackedObjectsImpl::paintOnFrame(int frameIndex, cv::Mat& frame) {
    int frameMargin = configurationReader.getRenderingVideoFrameMarginsInPixels();

    for (Chopstick& chopstick : chopsticks) {
        TrackingStatus status = chopstick.recentTrackingStatuses.back();
        cv::Scalar color;
        switch (status) {
            case TrackingStatus::DETECTED_ONCE:
                color = greenColor;
                break;
            case TrackingStatus::NOT_DETECTED:
                color = orangeColor;
                break;
            case TrackingStatus::HIDDEN_BY_ARM:
                color = magentaColor;
                break;
            case TrackingStatus::DETECTED:
            default:
                color = whiteColor;
                break;
        }

        int thickness = chopstick.isRejectedBecauseOfConflict ? 1 : 2;
        
        cv::Point point1(chopstick.tip1.centerX() + frameMargin, chopstick.tip1.centerY() + frameMargin);
        cv::Point point2(chopstick.tip2.centerX() + frameMargin, chopstick.tip2.centerY() + frameMargin);
        cv::line(frame, point1, point2, color, thickness);
    }

    for (Tip& tip : tips) {
        TrackingStatus status = tip.recentTrackingStatuses.back();
        cv::Scalar color;
        switch (status) {
            case TrackingStatus::DETECTED_ONCE:
                color = greenColor;
                break;
            case TrackingStatus::NOT_DETECTED:
                color = orangeColor;
                break;
            case TrackingStatus::HIDDEN_BY_ARM:
                color = magentaColor;
                break;
            case TrackingStatus::DETECTED:
            default:
                color = whiteColor;
                break;
        }

        cv::Rect rect(tip.x + frameMargin, tip.y + frameMargin, tip.width, tip.height);
        cv::rectangle(frame, rect, color);
        cv::putText(frame, tip.id, cv::Point(rect.x, rect.y + 16.0), cv::FONT_HERSHEY_SIMPLEX, 0.5, color);
    }
}