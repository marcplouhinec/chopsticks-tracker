#ifndef SERVICE_VIDEO_FRAME_PAINTER
#define SERVICE_VIDEO_FRAME_PAINTER

#include <opencv2/opencv.hpp>
#include "../model/tracking/FrameOffset.hpp"

namespace service {

    class VideoFramePainter {
        public:
            virtual ~VideoFramePainter() {}

            virtual void paintOnFrame(
                int frameIndex, cv::Mat& frame, model::FrameOffset accumulatedFrameOffset) = 0;
    };

}

#endif // SERVICE_VIDEO_FRAME_PAINTER