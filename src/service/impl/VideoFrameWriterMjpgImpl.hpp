#ifndef SERVICE_VIDEO_FRAME_WRITER_MJPG_IMPL
#define SERVICE_VIDEO_FRAME_WRITER_MJPG_IMPL

#include <memory>
#include <boost/filesystem.hpp>
#include "../../model/Configuration.hpp"
#include "../../utils/logging.hpp"
#include "../VideoFrameWriter.hpp"

namespace service {

    class VideoFrameWriterMjpgImpl : public VideoFrameWriter {
        private:
            boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger;

            model::Configuration& configuration;
            boost::filesystem::path inputVideoPath;
            double fps;
            int frameWidth;
            int frameHeight;
            int lastWrittenFrameIndex = -1;

            std::unique_ptr<cv::VideoWriter> pVideoWriter{};

        public:
            VideoFrameWriterMjpgImpl(
                model::Configuration& configuration,
                boost::filesystem::path inputVideoPath,
                double fps,
                int frameWidth,
                int frameHeight) :
                    configuration(configuration),
                    inputVideoPath(inputVideoPath),
                    fps(fps),
                    frameWidth(frameWidth),
                    frameHeight(frameHeight) {}

            virtual ~VideoFrameWriterMjpgImpl();

            virtual void writeFrameAt(int frameIndex, cv::Mat& frame);
    };

}

#endif // SERVICE_VIDEO_FRAME_WRITER_MJPG_IMPL