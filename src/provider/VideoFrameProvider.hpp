#ifndef PROVIDER_VIDEO_FRAME_PROVIDER
#define PROVIDER_VIDEO_FRAME_PROVIDER

#include <opencv2/opencv.hpp>

namespace provider {

    class VideoFrameProvider {
        public:
            virtual ~VideoFrameProvider() {}

            virtual cv::Mat getFrameAt(int frameIndex) = 0;
            virtual int getNbFrames() = 0;
            virtual int getFps() = 0;
            virtual int getFrameWidth() = 0;
            virtual int getFrameHeight() = 0;
    };

}

#endif // PROVIDER_VIDEO_FRAME_PROVIDER