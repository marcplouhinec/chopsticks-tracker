#ifndef SERVICE_VIDEO_FRAME_READER_IMPL
#define SERVICE_VIDEO_FRAME_READER_IMPL

#include <memory>
#include "../../model/Configuration.hpp"
#include "../VideoFrameReader.hpp"

namespace service {

    class VideoFrameReaderImpl : public VideoFrameReader {
        private:
            const model::Configuration& configuration;
            const boost::filesystem::path& videoPath;

            cv::Mat currentFrame;
            int currentFrameIndex = -1;
            std::unique_ptr<cv::VideoCapture> pVideoCapture{};
            model::VideoProperties videoProperties;

        public:
            VideoFrameReaderImpl(
                const model::Configuration& configuration,
                boost::filesystem::path& videoPath) :
                    configuration(configuration),
                    videoPath(videoPath) {}

            virtual ~VideoFrameReaderImpl();

            virtual const cv::Mat readFrameAt(int frameIndex);
            virtual const model::VideoProperties getVideoProperties();
        
        private:
            void initVideoCaptureIfNecessary();
    };

}

#endif // SERVICE_VIDEO_FRAME_READER_IMPL