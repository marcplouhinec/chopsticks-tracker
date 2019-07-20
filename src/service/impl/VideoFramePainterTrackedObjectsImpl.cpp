#include "VideoFramePainterTrackedObjectsImpl.hpp"

using namespace model;
using namespace service;

void VideoFramePainterTrackedObjectsImpl::paintOnFrame(int frameIndex, cv::Mat& frame) {
    int frameMargin = configurationReader.getRenderingVideoFrameMarginsInPixels();

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