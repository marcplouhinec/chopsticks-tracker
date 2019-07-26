#ifndef SERVICE_VIDEO_FRAME_READER_IMPL
#define SERVICE_VIDEO_FRAME_READER_IMPL

#include <memory>
#include "../VideoFrameReader.hpp"

namespace service {

    class VideoFrameReaderImpl : public VideoFrameReader {
        private:
            boost::filesystem::path videoPath;

            cv::Mat currentFrame;
            int currentFrameIndex = -1;
            std::unique_ptr<cv::VideoCapture> pVideoCapture{};

        public:
            VideoFrameReaderImpl(boost::filesystem::path videoPath) : videoPath(videoPath) {}
            virtual ~VideoFrameReaderImpl();

            virtual const cv::Mat readFrameAt(int frameIndex);
            virtual const model::VideoProperties getVideoProperties();
        
        private:
            void initVideoCaptureIfNecessary();
    };

}

#endif // SERVICE_VIDEO_FRAME_READER_IMPL