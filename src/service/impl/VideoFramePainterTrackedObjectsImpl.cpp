#include <map>
#include "VideoFramePainterTrackedObjectsImpl.hpp"

using namespace model;
using namespace service;
using std::map;
using std::round;
using std::string;

void VideoFramePainterTrackedObjectsImpl::paintOnFrame(
    const cv::Mat& frame,
    const std::list<model::Tip>& tips,
    const std::list<model::Chopstick>& chopsticks,
    const model::FrameOffset accumulatedFrameOffset) const {

    int frameMargin = configuration.renderingVideoFrameMarginsInPixels;

    if (configuration.renderingTrackedObjectsPainterShowAcceptedChopsticks ||
        configuration.renderingTrackedObjectsPainterShowRejectedChopsticks) {
        // Index tips by their IDs
        map<string, Tip> tipById;
        for (const Tip& tip : tips) {
            tipById.emplace(tip.id, tip);
        }

        for (const Chopstick& chopstick : chopsticks) {
            if (!configuration.renderingTrackedObjectsPainterShowAcceptedChopsticks &&
                !chopstick.isRejectedBecauseOfConflict) {
                continue;
            }
            if (!configuration.renderingTrackedObjectsPainterShowRejectedChopsticks &&
                chopstick.isRejectedBecauseOfConflict) {
                continue;
            }

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
            
            cv::Point point1(round(tip1.centerX() + frameMargin), round(tip1.centerY() + frameMargin));
            cv::Point point2(round(tip2.centerX() + frameMargin), round(tip2.centerY() + frameMargin));

            if (!configuration.renderingTrackedObjectsPainterShowChopstickArrows) {
                cv::line(frame, point1, point2, color, thickness);
            } else if (tip1.isBigTip() && !tip2.isBigTip()) {
                cv::arrowedLine(frame, point1, point2, color, thickness, 8, 0, /* tipLength = */0.03);
            } else if (tip2.isBigTip() && !tip1.isBigTip()) {
                cv::arrowedLine(frame, point2, point1, color, thickness, 8, 0, /* tipLength = */0.03);
            } else {
                cv::line(frame, point1, point2, color, thickness);
            }
        }
    }
    
    if (configuration.renderingTrackedObjectsPainterShowTips) {
        for (const Tip& tip : tips) {
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

            cv::Rect rect(
                round(tip.x + frameMargin),
                round(tip.y + frameMargin),
                round(tip.width),
                round(tip.height));
            cv::rectangle(frame, rect, color);
            cv::putText(frame, tip.id, cv::Point(rect.x, rect.y + 16.0), cv::FONT_HERSHEY_SIMPLEX, 0.5, color);
        }
    }
}