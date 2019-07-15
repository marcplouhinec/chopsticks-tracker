#ifndef MODEL_FRAME_DETECTION_RESULT
#define MODEL_FRAME_DETECTION_RESULT

#include <vector>
#include "DetectedObject.hpp"

namespace model {

    class FrameDetectionResult {
        public:
            int frameIndex;
            std::vector<DetectedObject> detectedObjects;

        public:
            FrameDetectionResult() {}

            FrameDetectionResult(int frameIndex, std::vector<DetectedObject> detectedObjects) : 
                frameIndex(frameIndex), detectedObjects(detectedObjects) {}
    };
}

#endif // MODEL_FRAME_DETECTION_RESULT