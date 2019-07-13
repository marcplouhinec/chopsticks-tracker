#ifndef SERVICE_VIDEO_FRAME_READER
#define SERVICE_VIDEO_FRAME_READER

#include <opencv2/opencv.hpp>

namespace service {

    class VideoFrameReader {
        public:
            virtual ~VideoFrameReader() {}

            virtual cv::Mat readFrameAt(int frameIndex) = 0;
            virtual int getNbFrames() = 0;
            virtual int getFps() = 0;
            virtual int getFrameWidth() = 0;
            virtual int getFrameHeight() = 0;
    };

}

#endif // SERVICE_VIDEO_FRAME_READER