#ifndef SERVICE_OBJECT_DETECTOR
#define SERVICE_OBJECT_DETECTOR

#include <vector>
#include <opencv2/opencv.hpp>
#include "../model/detection/DetectedObject.hpp"

namespace service {

    class ObjectDetector {
        public:
            virtual ~ObjectDetector() {}

            virtual std::vector<model::DetectedObject> detectObjectsAt(int frameIndex) = 0;
    };

}

#endif // SERVICE_OBJECT_DETECTOR