#ifndef MODEL_DETECTED_OBJECT_TYPE
#define MODEL_DETECTED_OBJECT_TYPE

#include <string>
#include <vector>

namespace model {
    enum class DetectedObjectType {
        ARM, CHOPSTICK, BIG_TIP, SMALL_TIP
    };

    class DetectedObjectTypeHelper {
        public:
            static std::string enumToString(DetectedObjectType enumValue) {
                switch (enumValue)
                {
                case DetectedObjectType::ARM:
                    return "ARM";
                case DetectedObjectType::CHOPSTICK:
                    return "CHOPSTICK";
                case DetectedObjectType::BIG_TIP:
                    return "BIG_TIP";
                case DetectedObjectType::SMALL_TIP:
                default:
                    return "SMALL_TIP";
                }
            }

            static DetectedObjectType stringToEnum(std::string stringValue) {
                if (stringValue == "ARM") {
                    return DetectedObjectType::ARM;
                }
                if (stringValue == "CHOPSTICK") {
                    return DetectedObjectType::CHOPSTICK;
                }
                if (stringValue == "BIG_TIP") {
                    return DetectedObjectType::BIG_TIP;
                }
                if (stringValue == "SMALL_TIP") {
                    return DetectedObjectType::SMALL_TIP;
                }
                return DetectedObjectType::SMALL_TIP;
            }

            static std::vector<DetectedObjectType> stringsToEnums(const std::vector<std::string> stringValues) {
                std::vector<DetectedObjectType> enums;

                for (const std::string& stringValue : stringValues) {
                    enums.push_back(stringToEnum(stringValue));
                }

                return enums;
            }

            static std::vector<DetectedObjectType> enumerate() {
                std::vector<DetectedObjectType> enumValues;
                enumValues.push_back(DetectedObjectType::ARM);
                enumValues.push_back(DetectedObjectType::CHOPSTICK);
                enumValues.push_back(DetectedObjectType::BIG_TIP);
                enumValues.push_back(DetectedObjectType::SMALL_TIP);
                return enumValues;
            }

            static bool isTip(DetectedObjectType objectType) {
                return objectType == DetectedObjectType::BIG_TIP
                    || objectType == DetectedObjectType::SMALL_TIP;
            }
    };
}

#endif // MODEL_DETECTED_OBJECT_TYPE