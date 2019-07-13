#ifndef SERVICE_OBJECT_DETECTOR_DARKNET_IMPL
#define SERVICE_OBJECT_DETECTOR_DARKNET_IMPL

#include <map>
#include <boost/filesystem.hpp>
#include <darknet.h>
#include "../../utils/logging.hpp"
#include "../ObjectDetector.hpp"
#include "../ConfigurationReader.hpp"
#include "../VideoFrameReader.hpp"

namespace service {

    /**
     * Implementation of the {@link ObjectDetector} by using the YOLO v3 model running
     * on top of Darknet (very fast with CUDA, slow without it).
     * 
     * @author Marc Plouhinec
     */
    class ObjectDetectorDarknetImpl : public ObjectDetector {
        private:
            boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger;

            ConfigurationReader* pConfigurationReader;
            VideoFrameReader* pVideoFrameReader;
            network* pNeuralNetwork = nullptr;
            layer lastLayer;
            std::vector<model::DetectedObjectType> objectTypesByClassId;

        public:
            ObjectDetectorDarknetImpl(
                ConfigurationReader* pConfigurationReader,
                VideoFrameReader* pVideoFrameReader) : 
                    pConfigurationReader(pConfigurationReader),
                    pVideoFrameReader(pVideoFrameReader) {}

            virtual ~ObjectDetectorDarknetImpl();

            virtual std::vector<model::DetectedObject> detectObjectsAt(int frameIndex);
        
        private:
            image matToImage(cv::Mat& src);
    };

}

#endif // SERVICE_OBJECT_DETECTOR_DARKNET_IMPL