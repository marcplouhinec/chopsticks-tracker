#ifndef SERVICE_OBJECT_DETECTION_SERVICE
#define SERVICE_OBJECT_DETECTION_SERVICE

#include <string>
#include <boost/range.hpp>
#include "../../model/detection/FrameDetectionResult.hpp"
#include "FrameDetectionResultIterator.hpp"

namespace service {

    /**
     * Detect objects in images and videos.
     * Note: the detectable object types are listed in [fr.marcsworld.chopstickstracker.model.DetectedObjectType].
     *
     * @author Marc Plouhinec
     */
    class ObjectDetectionService {
        public:
            virtual ~ObjectDetectionService() {}

            /**
             * Detect objects in each frame of the given video.
             */
            virtual boost::iterator_range<service::FrameDetectionResultIterator> 
                detectObjectsInVideo(std::string videoFilePath) = 0;
    };

}

#endif // SERVICE_OBJECT_DETECTION_SERVICE