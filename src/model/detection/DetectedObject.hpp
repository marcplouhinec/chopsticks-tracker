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
                double x,
                double y,
                double width,
                double height,
                DetectedObjectType objectType,
                float confidence) : 
                    Rectangle(x, y, width, height),
                    objectType(objectType),
                    confidence(confidence) {}

            DetectedObject copyAndTranslate(double dx, double dy) {
                return DetectedObject(x + dx, y + dy, width, height, objectType, confidence);
            }

            bool operator== (const DetectedObject& other) const {
                return std::abs(x - other.x) < std::numeric_limits<double>::epsilon()
                    && std::abs(y - other.y) < std::numeric_limits<double>::epsilon()
                    && std::abs(width - other.width) < std::numeric_limits<double>::epsilon()
                    && std::abs(height - other.height) < std::numeric_limits<double>::epsilon()
                    && objectType == other.objectType;
            }

            bool operator!= (const DetectedObject& other) const {
                return !(*this == other);
            }

            struct Hasher
            {
                Rectangle::Hasher rectangleHasher;

                std::size_t operator()(const DetectedObject& o) const
                {
                    std::size_t res = 17;
                    res = res * 31 + rectangleHasher(o);
                    res = res * 31 + std::hash<DetectedObjectType>()(o.objectType);
                    return res;
                }
            };
    };
}

#endif // MODEL_DETECTED_OBJECT