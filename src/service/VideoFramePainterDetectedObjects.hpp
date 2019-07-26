#ifndef SERVICE_VIDEO_FRAME_PAINTER_DETECTED_OBJECTS
#define SERVICE_VIDEO_FRAME_PAINTER_DETECTED_OBJECTS

#include <vector>
#include <opencv2/opencv.hpp>
#include "../model/detection/DetectedObject.hpp"
#include "../model/tracking/FrameOffset.hpp"

namespace service {

    class VideoFramePainterDetectedObjects {
        public:
            virtual ~VideoFramePainterDetectedObjects() {}

            virtual void paintOnFrame(
                const cv::Mat& frame,
                const std::vector<model::DetectedObject>& detectedObjects,
                const model::FrameOffset accumulatedFrameOffset) const = 0;
    };

}

#endif // SERVICE_VIDEO_FRAME_PAINTER_DETECTED_OBJECTS