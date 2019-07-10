#ifndef SERVICE_DARKNET_OBJECT_DETECTION_SERVICE_IMPL
#define SERVICE_DARKNET_OBJECT_DETECTION_SERVICE_IMPL

#include "../ObjectDetectionService.hpp"

namespace service {

    /**
     * Implementation of {@link ObjectDetectionService} by using a YOLO v3 model implemented on top
     * of Darknet.
     *
     * @author Marc Plouhinec
     */
    class DarknetObjectDetectionServiceImpl : public ObjectDetectionService {
        public:
            virtual std::vector<model::DetectedObject> detectObjectsInImage(cv::Mat image);
    };

}

#endif // SERVICE_DARKNET_OBJECT_DETECTION_SERVICE_IMPL