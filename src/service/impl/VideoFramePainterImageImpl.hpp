#ifndef SERVICE_VIDEO_FRAME_PAINTER_IMAGE_IMPL
#define SERVICE_VIDEO_FRAME_PAINTER_IMAGE_IMPL

#include "../../model/Configuration.hpp"
#include "../VideoFramePainterImage.hpp"

namespace service {

    class VideoFramePainterImageImpl : public VideoFramePainterImage {
        private:
            const cv::Scalar blackColor{0, 0, 0};

        private:
            model::Configuration& configuration;

        public:
            VideoFramePainterImageImpl(model::Configuration& configuration) : 
                configuration(configuration) {}
            
            virtual ~VideoFramePainterImageImpl() {};

            virtual void paintOnFrame(
                cv::Mat& frame,
                const cv::Mat& image,
                const model::FrameOffset accumulatedFrameOffset);
    };

}

#endif // SERVICE_VIDEO_FRAME_PAINTER_IMAGE_IMPL