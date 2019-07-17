#ifndef SERVICE_VIDEO_FRAME_PAINTER_DETECTED_OBJECTS_IMPL
#define SERVICE_VIDEO_FRAME_PAINTER_DETECTED_OBJECTS_IMPL

#include <boost/circular_buffer.hpp>
#include "../ConfigurationReader.hpp"
#include "../VideoFramePainter.hpp"
#include "../../model/detection/FrameDetectionResult.hpp"

namespace service {

    class VideoFramePainterDetectedObjectsImpl : public VideoFramePainter {
        private:
            const cv::Scalar yellowColor{0.0, 255.0, 255.0};
            const cv::Scalar greenColor{0.0, 255.0, 0.0};
            const cv::Scalar orangeColor{0.0, 200.0, 255.0};
            const cv::Scalar magentaColor{255.0, 0.0, 255.0};

        private:
            ConfigurationReader& configurationReader;
            boost::circular_buffer<model::FrameDetectionResult>& frameDetectionResults;

        public:
            VideoFramePainterDetectedObjectsImpl(
                ConfigurationReader& configurationReader,
                boost::circular_buffer<model::FrameDetectionResult>& frameDetectionResults) :
                    configurationReader(configurationReader),
                    frameDetectionResults(frameDetectionResults) {}
            virtual ~VideoFramePainterDetectedObjectsImpl() {};

            virtual void paintOnFrame(int frameIndex, cv::Mat& frame);

        private:
            model::FrameDetectionResult& findFrameDetectionResultByFrameIndex(int frameIndex);
    };

}

#endif // SERVICE_VIDEO_FRAME_PAINTER_DETECTED_OBJECTS_IMPL