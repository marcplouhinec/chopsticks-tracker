#ifndef SERVICE_DARKNET_OBJECT_DETECTION_SERVICE_IMPL
#define SERVICE_DARKNET_OBJECT_DETECTION_SERVICE_IMPL

#include "../FrameDetectionResultIterator.hpp"
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
            virtual boost::iterator_range<service::FrameDetectionResultIterator> 
                detectObjectsInVideo(std::string videoFilePath);
    };

}

#endif // SERVICE_DARKNET_OBJECT_DETECTION_SERVICE_IMPL