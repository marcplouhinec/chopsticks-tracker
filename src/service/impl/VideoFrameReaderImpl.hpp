#ifndef SERVICE_VIDEO_FRAME_READER_IMPL
#define SERVICE_VIDEO_FRAME_READER_IMPL

#include <memory>
#include "../VideoFrameReader.hpp"

namespace service {

    class VideoFrameReaderImpl : public VideoFrameReader {
        private:
            boost::filesystem::path videoPath;

            int nbFrames = -1;
            int fps = -1;
            int frameWidth = -1;
            int frameHeight = -1;

            cv::Mat currentFrame;
            int currentFrameIndex = -1;
            std::unique_ptr<cv::VideoCapture> pVideoCapture{};

        public:
            VideoFrameReaderImpl(boost::filesystem::path videoPath) : videoPath(videoPath) {}
            virtual ~VideoFrameReaderImpl();

            virtual cv::Mat getFrameAt(int frameIndex);
            virtual int getNbFrames();
            virtual int getFps();
            virtual int getFrameWidth();
            virtual int getFrameHeight();
        
        private:
            void initVideoCaptureIfNecessary();
    };

}

#endif // SERVICE_VIDEO_FRAME_READER_IMPL