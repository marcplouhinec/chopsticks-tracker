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
            std::vector<DetectedObject> detectedObjects;
            std::function<cv::Mat()> frameImageProvider;

        public:
            FrameDetectionResult() : 
                frameIndex(0),
                detectedObjects(std::vector<DetectedObject>()),
                frameImageProvider([](){ return cv::Mat(); }) {};

            explicit FrameDetectionResult(
                int frameIndex, std::vector<DetectedObject> detectedObjects, std::function<cv::Mat()> frameImageProvider) : 
                frameIndex(frameIndex), detectedObjects(detectedObjects), frameImageProvider(frameImageProvider) {}
    };
}

#endif // MODEL_FRAME_DETECTION_RESULT