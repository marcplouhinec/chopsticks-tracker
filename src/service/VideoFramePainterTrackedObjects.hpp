#ifndef SERVICE_VIDEO_FRAME_PAINTER_TRACKED_OBJECTS
#define SERVICE_VIDEO_FRAME_PAINTER_TRACKED_OBJECTS

#include <list>
#include <opencv2/opencv.hpp>
#include "../model/tracking/Tip.hpp"
#include "../model/tracking/Chopstick.hpp"
#include "../model/tracking/FrameOffset.hpp"

namespace service {

    class VideoFramePainterTrackedObjects {
        public:
            virtual ~VideoFramePainterTrackedObjects() {}

            virtual void paintOnFrame(
                const cv::Mat& frame,
                const std::list<model::Tip>& tips,
                const std::list<model::Chopstick>& chopsticks,
                const model::FrameOffset accumulatedFrameOffset) = 0;
    };

}

#endif // SERVICE_VIDEO_FRAME_PAINTER_TRACKED_OBJECTS