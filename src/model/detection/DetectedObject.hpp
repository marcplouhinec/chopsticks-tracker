#ifndef MODEL_DETECTED_OBJECT
#define MODEL_DETECTED_OBJECT

#include <functional>
#include "DetectedObjectType.hpp"

namespace model {

    class DetectedObject {
        public:
            int x = 0;
            int y = 0;
            int width = 0;
            int height = 0;
            DetectedObjectType objectType = DetectedObjectType::CHOPSTICK;
            float confidence = 0.0;

        public:
            DetectedObject() {}

            explicit DetectedObject(
                int x, int y, int width, int height, DetectedObjectType objectType, float confidence) : 
                    x(x), y(y),
                    width(width), height(height),
                    objectType(objectType),
                    confidence(confidence) {}

            bool operator== (const DetectedObject& other) const {
                return x == other.x
                    && y == other.y
                    && width == other.width
                    && height == other.height
                    && objectType == other.objectType;
            }

            struct Hasher
            {
                std::size_t operator()(const DetectedObject& o) const
                {
                    std::size_t res = 17;
                    res = res * 31 + std::hash<int>()( o.x );
                    res = res * 31 + std::hash<int>()( o.y );
                    res = res * 31 + std::hash<int>()( o.width );
                    res = res * 31 + std::hash<int>()( o.height );
                    res = res * 31 + std::hash<DetectedObjectType>()( o.objectType );
                    return res;
                }
            };
    };
}

#endif // MODEL_DETECTED_OBJECT