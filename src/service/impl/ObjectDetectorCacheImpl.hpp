#ifndef SERVICE_OBJECT_DETECTOR_CACHE_IMPL
#define SERVICE_OBJECT_DETECTOR_CACHE_IMPL

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include "../../model/Configuration.hpp"
#include "../../utils/logging.hpp"
#include "../ObjectDetector.hpp"
#include "../VideoFrameReader.hpp"

namespace service {

    /**
     * Implementation of the {@link ObjectDetector} that simply wraps another {@link ObjectDetector}
     * in order to cache the results. The goal is to avoid re-detecting the same objects
     * during development.
     * 
     * @author Marc Plouhinec
     */
    class ObjectDetectorCacheImpl : public ObjectDetector {
        private:
            boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger;

            boost::filesystem::path videoPath;
            model::Configuration& configuration;
            ObjectDetector& wrappedObjectDetector;

            bool cacheFolderInitialized = false;

        public:
            ObjectDetectorCacheImpl(
                model::Configuration& configuration,
                ObjectDetector& wrappedObjectDetector,
                boost::filesystem::path videoPath) : 
                    configuration(configuration),
                    wrappedObjectDetector(wrappedObjectDetector),
                    videoPath(videoPath) {}

            virtual ~ObjectDetectorCacheImpl() {}

            virtual std::vector<model::DetectedObject> detectObjectsAt(int frameIndex);

        private:
            std::string convertToJson(std::vector<model::DetectedObject> detectedObjects);
            std::vector<model::DetectedObject> convertFromJson(std::string detectedObjectsJson);
    };

}

#endif // SERVICE_OBJECT_DETECTOR_CACHE_IMPL