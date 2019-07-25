#ifndef SERVICE_VIDEO_FRAME_WRITER_MULTI_JPEG_IMPL
#define SERVICE_VIDEO_FRAME_WRITER_MULTI_JPEG_IMPL

#include <boost/filesystem.hpp>
#include "../../model/Configuration.hpp"
#include "../../utils/logging.hpp"
#include "../VideoFrameWriter.hpp"

namespace service {

    class VideoFrameWriterMultiJpegImpl : public VideoFrameWriter {
        private:
            boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger;

            model::Configuration& configuration;
            boost::filesystem::path inputVideoPath;

            boost::filesystem::path outputFolderPath;

            bool folderInitialized = false;

        public:
            VideoFrameWriterMultiJpegImpl(
                model::Configuration& configuration,
                boost::filesystem::path inputVideoPath) :
                    configuration(configuration),
                    inputVideoPath(inputVideoPath) {}

            virtual ~VideoFrameWriterMultiJpegImpl() {};

            virtual void writeFrameAt(int frameIndex, cv::Mat& frame);
    };

}

#endif // SERVICE_VIDEO_FRAME_WRITER_MULTI_JPEG_IMPL