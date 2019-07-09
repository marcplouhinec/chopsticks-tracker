#ifndef MODEL_DETECTED_OBJECT
#define MODEL_DETECTED_OBJECT

#include "DetectedObjectType.hpp"

namespace model {
    class DetectedObject {
        public:
            int x = 0;
            int y = 0;
            int width = 0;
            int height = 0;
            DetectedObjectType objectType = DetectedObjectType::CHOPSTICK;
            double confidence = 0.0;

        public:
            DetectedObject(int x, int y, int width, int height,
                DetectedObjectType objectType, double confidence) : 
                x(x), y(y), width(width), height(height), objectType(objectType), confidence(confidence) {}
    };
}

#endif // MODEL_DETECTED_OBJECT