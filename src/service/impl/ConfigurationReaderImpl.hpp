#ifndef SERVICE_CONFIGURATION_READER_IMPL
#define SERVICE_CONFIGURATION_READER_IMPL

#include "../../utils/logging.hpp"
#include "../ConfigurationReader.hpp"

namespace service {

    class ConfigurationReaderImpl : public ConfigurationReader {
        private:
            boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger;

            boost::filesystem::path configurationPath;
            bool configurationLoaded = false;

            std::vector<std::string> yoloModelClassNames;
            boost::filesystem::path yoloModelCfgPath;
            boost::filesystem::path yoloModelWeightsPath;

            float objectDetectionMinTipConfidence;
            float objectDetectionMinChopstickConfidence;
            float objectDetectionMinArmConfidence;
            float objectDetectionNmsThreshold;
            std::string objectDetectionImplementation;
            boost::filesystem::path objectDetectionCacheFolderPath;

            int trackingNbPastFrameDetectionResultsToKeep;
            int trackingMaxTipMatchingDistanceInPixels;
            int trackingNbTipsToUseToDetectCameraMotion;

            boost::filesystem::path renderingOutputPath;
            std::string renderingImplementation;
            int renderingVideoFrameMarginsInPixels;

        public:
            ConfigurationReaderImpl(boost::filesystem::path configurationPath) :
                configurationPath(configurationPath) {}

            virtual ~ConfigurationReaderImpl() {}

            virtual std::vector<std::string> getYoloModelClassNames();
            virtual std::vector<model::DetectedObjectType> getYoloModelClassEnums();
            virtual boost::filesystem::path getYoloModelCfgPath();
            virtual boost::filesystem::path getYoloModelWeightsPath();

            virtual float getObjectDetectionMinTipConfidence();
            virtual float getObjectDetectionMinChopstickConfidence();
            virtual float getObjectDetectionMinArmConfidence();
            virtual float getObjectDetectionNmsThreshold();
            virtual std::string getObjectDetectionImplementation();
            virtual boost::filesystem::path getObjectDetectionCacheFolderPath();

            virtual int getTrackingNbPastFrameDetectionResultsToKeep();
            virtual int getTrackingMaxTipMatchingDistanceInPixels();
            virtual int getTrackingNbTipsToUseToDetectCameraMotion();

            virtual boost::filesystem::path getRenderingOutputPath();
            virtual std::string getRenderingImplementation();
            virtual int getRenderingVideoFrameMarginsInPixels();
        
        private:
            void loadConfiguration();
    };

}

#endif // SERVICE_CONFIGURATION_READER_IMPL