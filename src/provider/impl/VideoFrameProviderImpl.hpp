#ifndef PROVIDER_VIDEO_FRAME_PROVIDER_IMPL
#define PROVIDER_VIDEO_FRAME_PROVIDER_IMPL

#include "../VideoFrameProvider.hpp"

namespace provider {

    class VideoFrameProviderImpl : public VideoFrameProvider {
        private:
            boost::filesystem::path videoPath;

            int nbFrames = -1;
            int fps = -1;
            int frameWidth = -1;
            int frameHeight = -1;

            cv::Mat currentFrame;
            int currentFrameIndex = -1;
            cv::VideoCapture* pVideoCapture = nullptr;

        public:
            VideoFrameProviderImpl(boost::filesystem::path videoPath) : videoPath(videoPath) {}
            virtual ~VideoFrameProviderImpl() {
                if (pVideoCapture != nullptr) {
                    pVideoCapture->release();
                    delete pVideoCapture;
                } 
            }

            virtual cv::Mat getFrameAt(int frameIndex);
            virtual int getNbFrames();
            virtual int getFps();
            virtual int getFrameWidth();
            virtual int getFrameHeight();
        
        private:
            cv::VideoCapture* getVideoCapture();
    };

}

#endif // PROVIDER_VIDEO_FRAME_PROVIDER_IMPL