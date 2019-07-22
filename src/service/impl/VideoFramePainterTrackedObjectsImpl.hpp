#ifndef SERVICE_VIDEO_FRAME_PAINTER_TRACKED_OBJECTS_IMPL
#define SERVICE_VIDEO_FRAME_PAINTER_TRACKED_OBJECTS_IMPL

#include <list>
#include "../ConfigurationReader.hpp"
#include "../VideoFramePainter.hpp"
#include "../../model/tracking/Chopstick.hpp"
#include "../../model/tracking/Tip.hpp"

namespace service {

    class VideoFramePainterTrackedObjectsImpl : public VideoFramePainter {
        private:
            const cv::Scalar cyanColor{255.0, 255.0, 0.0};
            const cv::Scalar greenColor{0.0, 255.0, 0.0};
            const cv::Scalar orangeColor{0.0, 200.0, 255.0};
            const cv::Scalar magentaColor{255.0, 0.0, 255.0};
            const cv::Scalar whiteColor{255.0, 255.0, 255.0};

        private:
            ConfigurationReader& configurationReader;
            std::list<model::Tip>& tips;
            std::list<model::Chopstick>& chopsticks;

        public:
            VideoFramePainterTrackedObjectsImpl(
                ConfigurationReader& configurationReader,
                std::list<model::Tip>& tips,
                std::list<model::Chopstick>& chopsticks) :
                    configurationReader(configurationReader),
                    tips(tips),
                    chopsticks(chopsticks) {}

            virtual ~VideoFramePainterTrackedObjectsImpl() {};

            virtual void paintOnFrame(int frameIndex, cv::Mat& frame);
    };

}

#endif // SERVICE_VIDEO_FRAME_PAINTER_TRACKED_OBJECTS_IMPL