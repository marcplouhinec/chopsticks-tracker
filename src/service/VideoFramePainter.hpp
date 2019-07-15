#ifndef SERVICE_VIDEO_FRAME_PAINTER
#define SERVICE_VIDEO_FRAME_PAINTER

#include <opencv2/opencv.hpp>

namespace service {

    class VideoFramePainter {
        public:
            virtual ~VideoFramePainter() {}

            virtual void paintOnFrame(int frameIndex, cv::Mat& frame) = 0;
    };

}

#endif // SERVICE_VIDEO_FRAME_PAINTER