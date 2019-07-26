#ifndef SERVICE_VIDEO_FRAME_READER
#define SERVICE_VIDEO_FRAME_READER

#include <opencv2/opencv.hpp>
#include "../model/VideoProperties.hpp"

namespace service {

    class VideoFrameReader {
        public:
            virtual ~VideoFrameReader() {}

            virtual const cv::Mat readFrameAt(int frameIndex) = 0;
            virtual const model::VideoProperties getVideoProperties() = 0;
    };

}

#endif // SERVICE_VIDEO_FRAME_READER