#ifndef SERVICE_OBJECT_DETECTION_SERVICE
#define SERVICE_OBJECT_DETECTION_SERVICE

#include <vector>
#include <boost/filesystem.hpp>
#include <opencv2/opencv.hpp>
#include "../../model/detection/DetectedObject.hpp"

namespace service {

    /**
     * Detect objects in images and videos.
     * Note: the detectable object types are listed in {@link DetectedObjectType}.
     *
     * @author Marc Plouhinec
     */
    class ObjectDetectionService {
        public:
            virtual ~ObjectDetectionService() {}

            /**
             * Detect objects in the given image.
             */
            virtual std::vector<model::DetectedObject> detectObjectsInImage(cv::Mat image) = 0;
    };

}

#endif // SERVICE_OBJECT_DETECTION_SERVICE