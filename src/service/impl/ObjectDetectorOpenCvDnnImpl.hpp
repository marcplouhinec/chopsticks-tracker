#ifndef SERVICE_OBJECT_DETECTOR_OPENCV_DNN_IMPL
#define SERVICE_OBJECT_DETECTOR_OPENCV_DNN_IMPL

#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include "../../model/Configuration.hpp"
#include "../../model/VideoProperties.hpp"
#include "../../utils/logging.hpp"
#include "../ObjectDetector.hpp"
#include "../VideoFrameReader.hpp"

namespace service {

    /**
     * Implementation of the {@link ObjectDetector} by using the YOLO v3 model running
     * on top of OpenCV DNN (acceptable performance with CPUs, but cannot use GPUs).
     * 
     * @author Marc Plouhinec
     */
    class ObjectDetectorOpenCvDnnImpl : public ObjectDetector {
        private:
            boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger;

            const model::Configuration& configuration;
            VideoFrameReader& videoFrameReader;
            const model::VideoProperties& videoProperties;

            bool neuralNetworkInitialized = false;
            cv::dnn::Net neuralNetwork;
            cv::Size blobSize;
            cv::Scalar mean;
            std::vector<std::string> outLayerNames;
            std::vector<model::DetectedObjectType> objectTypesByClassId;

            float minTipConfidence = 0;
            float minChopstickConfidence = 0;
            float minArmConfidence = 0;
            float minConfidence = 0;

        public:
            ObjectDetectorOpenCvDnnImpl(
                const model::Configuration& configuration,
                VideoFrameReader& videoFrameReader,
                const model::VideoProperties& videoProperties) : 
                    configuration(configuration),
                    videoFrameReader(videoFrameReader),
                    videoProperties(videoProperties) {}

            virtual ~ObjectDetectorOpenCvDnnImpl() {}

            virtual std::vector<model::DetectedObject> detectObjectsAt(int frameIndex);
    };

}

#endif // SERVICE_OBJECT_DETECTOR_OPENCV_DNN_IMPL