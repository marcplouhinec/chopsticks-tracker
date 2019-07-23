#include <map>
#include "VideoFramePainterTrackedObjectsImpl.hpp"

using namespace model;
using namespace service;
using std::map;
using std::string;

void VideoFramePainterTrackedObjectsImpl::paintOnFrame(int frameIndex, cv::Mat& frame) {
    // Index tips by their IDs
    map<string, Tip> tipById;
    for (const Tip& tip : tips) {
        tipById.emplace(tip.id, tip);
    }

    int frameMargin = configurationReader.getRenderingVideoFrameMarginsInPixels();

    for (Chopstick& chopstick : chopsticks) {
        const Tip& tip1 = tipById[chopstick.tip1Id];
        const Tip& tip2 = tipById[chopstick.tip2Id];

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
        
        cv::Point point1(tip1.centerX() + frameMargin, tip1.centerY() + frameMargin);
        cv::Point point2(tip2.centerX() + frameMargin, tip2.centerY() + frameMargin);
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