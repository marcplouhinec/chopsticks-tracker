#ifndef MODEL_FRAME_DETECTION_RESULT
#define MODEL_FRAME_DETECTION_RESULT

#include <vector>
#include <functional>
#include <opencv2/opencv.hpp>
#include "DetectedObject.hpp"

namespace model {
    class FrameDetectionResult {
        public:
            int frameIndex;
            std::function<std::vector<DetectedObject>()> detectedObjectsProvider;
            std::function<cv::Mat()> frameImageProvider;

        public:
            FrameDetectionResult() : 
                FrameDetectionResult(0,
                [](){ return std::vector<DetectedObject>(); },
                [](){ return cv::Mat(); }) {};

            explicit FrameDetectionResult(
                int frameIndex,
                std::function<std::vector<DetectedObject>()> detectedObjectsProvider,
                std::function<cv::Mat()> frameImageProvider) : 
                frameIndex(frameIndex),
                detectedObjectsProvider(detectedObjectsProvider),
                frameImageProvider(frameImageProvider) {}
    };
}

#endif // MODEL_FRAME_DETECTION_RESULT