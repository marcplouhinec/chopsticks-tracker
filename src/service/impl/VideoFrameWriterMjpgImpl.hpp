#ifndef SERVICE_VIDEO_FRAME_WRITER_MJPG_IMPL
#define SERVICE_VIDEO_FRAME_WRITER_MJPG_IMPL

#include <memory>
#include <boost/filesystem.hpp>
#include "../../utils/logging.hpp"
#include "../VideoFrameWriter.hpp"
#include "../ConfigurationReader.hpp"

namespace service {

    class VideoFrameWriterMjpgImpl : public VideoFrameWriter {
        private:
            boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger;

            ConfigurationReader& configurationReader;
            boost::filesystem::path inputVideoPath;
            double fps;
            int frameWidth;
            int frameHeight;

            std::unique_ptr<cv::VideoWriter> pVideoWriter{};

        public:
            VideoFrameWriterMjpgImpl(
                ConfigurationReader& configurationReader,
                boost::filesystem::path inputVideoPath,
                double fps,
                int frameWidth,
                int frameHeight) :
                    configurationReader(configurationReader),
                    inputVideoPath(inputVideoPath),
                    fps(fps),
                    frameWidth(frameWidth),
                    frameHeight(frameHeight) {}

            virtual ~VideoFrameWriterMjpgImpl();

            virtual void writeFrameAt(int frameIndex, cv::Mat& frame);
    };

}

#endif // SERVICE_VIDEO_FRAME_WRITER_MJPG_IMPL