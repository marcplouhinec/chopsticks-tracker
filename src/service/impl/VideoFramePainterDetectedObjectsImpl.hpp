#ifndef SERVICE_VIDEO_FRAME_PAINTER_DETECTED_OBJECTS_IMPL
#define SERVICE_VIDEO_FRAME_PAINTER_DETECTED_OBJECTS_IMPL

#include "../../model/Configuration.hpp"
#include "../VideoFramePainterDetectedObjects.hpp"

namespace service {

    class VideoFramePainterDetectedObjectsImpl : public VideoFramePainterDetectedObjects {
        private:
            const cv::Scalar yellowColor{0.0, 255.0, 255.0};
            const cv::Scalar greenColor{0.0, 255.0, 0.0};
            const cv::Scalar orangeColor{0.0, 200.0, 255.0};
            const cv::Scalar magentaColor{255.0, 0.0, 255.0};

        private:
            const model::Configuration& configuration;

        public:
            VideoFramePainterDetectedObjectsImpl(const model::Configuration& configuration) : 
                configuration(configuration) {}
            
            virtual ~VideoFramePainterDetectedObjectsImpl() {};

            virtual void paintOnFrame(
                const cv::Mat& frame,
                const std::vector<model::DetectedObject>& detectedObjects,
                const model::FrameOffset accumulatedFrameOffset) const;
    };

}

#endif // SERVICE_VIDEO_FRAME_PAINTER_DETECTED_OBJECTS_IMPL