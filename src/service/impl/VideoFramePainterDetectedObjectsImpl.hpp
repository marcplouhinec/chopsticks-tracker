#ifndef SERVICE_VIDEO_FRAME_PAINTER_DETECTED_OBJECTS_IMPL
#define SERVICE_VIDEO_FRAME_PAINTER_DETECTED_OBJECTS_IMPL

#include <boost/circular_buffer.hpp>
#include "../../model/Configuration.hpp"
#include "../../model/detection/FrameDetectionResult.hpp"
#include "../VideoFramePainter.hpp"

namespace service {

    class VideoFramePainterDetectedObjectsImpl : public VideoFramePainter {
        private:
            const cv::Scalar yellowColor{0.0, 255.0, 255.0};
            const cv::Scalar greenColor{0.0, 255.0, 0.0};
            const cv::Scalar orangeColor{0.0, 200.0, 255.0};
            const cv::Scalar magentaColor{255.0, 0.0, 255.0};

        private:
            model::Configuration& configuration;
            boost::circular_buffer<model::FrameDetectionResult>& frameDetectionResults;

        public:
            VideoFramePainterDetectedObjectsImpl(
                model::Configuration& configuration,
                boost::circular_buffer<model::FrameDetectionResult>& frameDetectionResults) :
                    configuration(configuration),
                    frameDetectionResults(frameDetectionResults) {}
            virtual ~VideoFramePainterDetectedObjectsImpl() {};

            virtual void paintOnFrame(
                int frameIndex, cv::Mat& frame, model::FrameOffset accumulatedFrameOffset);

        private:
            model::FrameDetectionResult& findFrameDetectionResultByFrameIndex(int frameIndex);
    };

}

#endif // SERVICE_VIDEO_FRAME_PAINTER_DETECTED_OBJECTS_IMPL