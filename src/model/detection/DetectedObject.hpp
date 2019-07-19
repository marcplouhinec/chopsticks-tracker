#ifndef MODEL_DETECTED_OBJECT
#define MODEL_DETECTED_OBJECT

#include <functional>
#include "DetectedObjectType.hpp"
#include "../Rectangle.hpp"

namespace model {

    class DetectedObject : public Rectangle {
        public:
            DetectedObjectType objectType = DetectedObjectType::CHOPSTICK;
            float confidence = 0.0;

        public:
            DetectedObject() : Rectangle() {}

            explicit DetectedObject(
                int x, int y, int width, int height, DetectedObjectType objectType, float confidence) : 
                    Rectangle(x, y, width, height),
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