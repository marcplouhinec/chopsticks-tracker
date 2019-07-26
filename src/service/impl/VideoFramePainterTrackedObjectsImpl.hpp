#ifndef SERVICE_VIDEO_FRAME_PAINTER_TRACKED_OBJECTS_IMPL
#define SERVICE_VIDEO_FRAME_PAINTER_TRACKED_OBJECTS_IMPL

#include "../../model/Configuration.hpp"
#include "../VideoFramePainterTrackedObjects.hpp"

namespace service {

    class VideoFramePainterTrackedObjectsImpl : public VideoFramePainterTrackedObjects {
        private:
            const cv::Scalar cyanColor{255.0, 255.0, 0.0};
            const cv::Scalar greenColor{0.0, 255.0, 0.0};
            const cv::Scalar orangeColor{0.0, 200.0, 255.0};
            const cv::Scalar magentaColor{255.0, 0.0, 255.0};
            const cv::Scalar whiteColor{255.0, 255.0, 255.0};

        private:
            model::Configuration& configuration;

        public:
            VideoFramePainterTrackedObjectsImpl(model::Configuration& configuration) :
                configuration(configuration) {}

            virtual ~VideoFramePainterTrackedObjectsImpl() {};

            virtual void paintOnFrame(
                const cv::Mat& frame,
                const std::list<model::Tip>& tips,
                const std::list<model::Chopstick>& chopsticks,
                const model::FrameOffset accumulatedFrameOffset);
    };

}

#endif // SERVICE_VIDEO_FRAME_PAINTER_TRACKED_OBJECTS_IMPL