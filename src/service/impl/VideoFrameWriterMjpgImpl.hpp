#ifndef SERVICE_VIDEO_FRAME_WRITER_MJPG_IMPL
#define SERVICE_VIDEO_FRAME_WRITER_MJPG_IMPL

#include <memory>
#include <boost/filesystem.hpp>
#include "../../model/Configuration.hpp"
#include "../../model/VideoProperties.hpp"
#include "../../utils/logging.hpp"
#include "../VideoFrameWriter.hpp"

namespace service {

    class VideoFrameWriterMjpgImpl : public VideoFrameWriter {
        private:
            boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger;

            const model::Configuration& configuration;
            const boost::filesystem::path& inputVideoPath;
            const int outputFps;
            const int outputFrameWidth;
            const int outputFrameHeight;
            int lastWrittenFrameIndex = -1;

            std::unique_ptr<cv::VideoWriter> pVideoWriter{};

        public:
            VideoFrameWriterMjpgImpl(
                const model::Configuration& configuration,
                const boost::filesystem::path& inputVideoPath,
                const model::VideoProperties& videoProperties) :
                    configuration(configuration),
                    inputVideoPath(inputVideoPath),
                    outputFps(videoProperties.fps),
                    outputFrameWidth(videoProperties.frameWidth + 2 * configuration.renderingVideoFrameMarginsInPixels),
                    outputFrameHeight(videoProperties.frameHeight + 2 * configuration.renderingVideoFrameMarginsInPixels) {}

            virtual ~VideoFrameWriterMjpgImpl();

            virtual cv::Mat buildOutputFrame();

            virtual void writeFrameAt(int frameIndex, cv::Mat& frame);
    };

}

#endif // SERVICE_VIDEO_FRAME_WRITER_MJPG_IMPL