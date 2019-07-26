#ifndef SERVICE_VIDEO_FRAME_WRITER
#define SERVICE_VIDEO_FRAME_WRITER

#include <opencv2/opencv.hpp>

namespace service {

    class VideoFrameWriter {
        public:
            virtual ~VideoFrameWriter() {}

            virtual cv::Mat buildOutputFrame() = 0;

            virtual void writeFrameAt(int frameIndex, cv::Mat& frame) = 0;
    };

}

#endif // SERVICE_VIDEO_FRAME_WRITER