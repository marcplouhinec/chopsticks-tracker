#ifndef SERVICE_VIDEO_FRAME_PAINTER_IMAGE
#define SERVICE_VIDEO_FRAME_PAINTER_IMAGE

#include <opencv2/opencv.hpp>
#include "../model/tracking/FrameOffset.hpp"

namespace service {

    class VideoFramePainterImage {
        public:
            virtual ~VideoFramePainterImage() {}

            virtual void paintOnFrame(
                cv::Mat& frame,
                const cv::Mat& image,
                const model::FrameOffset accumulatedFrameOffset) = 0;
    };

}

#endif // SERVICE_VIDEO_FRAME_PAINTER_IMAGE