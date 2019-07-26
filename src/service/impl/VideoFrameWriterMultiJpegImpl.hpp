#ifndef SERVICE_VIDEO_FRAME_WRITER_MULTI_JPEG_IMPL
#define SERVICE_VIDEO_FRAME_WRITER_MULTI_JPEG_IMPL

#include <boost/filesystem.hpp>
#include "../../model/Configuration.hpp"
#include "../../model/VideoProperties.hpp"
#include "../../utils/logging.hpp"
#include "../VideoFrameWriter.hpp"

namespace service {

    class VideoFrameWriterMultiJpegImpl : public VideoFrameWriter {
        private:
            boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger;

            const model::Configuration& configuration;
            const boost::filesystem::path& inputVideoPath;
            const int outputFrameWidth;
            const int outputFrameHeight;

            boost::filesystem::path outputFolderPath;

            bool folderInitialized = false;

        public:
            VideoFrameWriterMultiJpegImpl(
                const model::Configuration& configuration,
                const boost::filesystem::path& inputVideoPath,
                const model::VideoProperties& videoProperties) :
                    configuration(configuration),
                    inputVideoPath(inputVideoPath),
                    outputFrameWidth(videoProperties.frameWidth + 2 * configuration.renderingVideoFrameMarginsInPixels),
                    outputFrameHeight(videoProperties.frameHeight + 2 * configuration.renderingVideoFrameMarginsInPixels) {}

            virtual ~VideoFrameWriterMultiJpegImpl() {};

            virtual cv::Mat buildOutputFrame();

            virtual void writeFrameAt(int frameIndex, cv::Mat& frame);
    };

}

#endif // SERVICE_VIDEO_FRAME_WRITER_MULTI_JPEG_IMPL