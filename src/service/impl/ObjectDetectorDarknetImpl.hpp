#ifndef SERVICE_OBJECT_DETECTOR_DARKNET_IMPL
#define SERVICE_OBJECT_DETECTOR_DARKNET_IMPL

#include <memory>
#include <boost/filesystem.hpp>
#include <darknet.h>
#include "../../model/Configuration.hpp"
#include "../../model/VideoProperties.hpp"
#include "../../utils/logging.hpp"
#include "../ObjectDetector.hpp"
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

            const model::Configuration& configuration;
            VideoFrameReader& videoFrameReader;
            const model::VideoProperties& videoProperties;
            std::unique_ptr<network> pNeuralNetwork{};
            layer lastLayer;
            std::vector<model::DetectedObjectType> objectTypesByClassId;

            float minTipConfidence = 0;
            float minChopstickConfidence = 0;
            float minArmConfidence = 0;
            float minConfidence = 0;

        public:
            ObjectDetectorDarknetImpl(
                const model::Configuration& configuration,
                VideoFrameReader& videoFrameReader,
                const model::VideoProperties& videoProperties) : 
                    configuration(configuration),
                    videoFrameReader(videoFrameReader),
                    videoProperties(videoProperties) {}

            virtual ~ObjectDetectorDarknetImpl() {};

            virtual std::vector<model::DetectedObject> detectObjectsAt(int frameIndex);
        
        private:
            image matToImage(cv::Mat& src);
    };

}

#endif // SERVICE_OBJECT_DETECTOR_DARKNET_IMPL