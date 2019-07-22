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

            int trackingMaxTipMatchingDistanceInPixels;
            int trackingNbTipsToUseToDetectCameraMotion;
            int trackingNbDetectionsToComputeAverageTipPositionAndSize;
            int trackingMinMatchingDistanceWithAnyObjectToConsiderTipNotHiddenByArm;
            int trackingMaxFramesAfterWhichATipIsConsideredLost;
            int trackingMinDistanceToConsiderNewTipAsTheSameAsAnExistingOne;
            int trackingMinChopstickLengthInPixels;
            int trackingMaxChopstickLengthInPixels;
            double trackingMinIOUToConsiderTwoTipsAsAChopstick;
            int trackingMaxFramesAfterWhichAChopstickIsConsideredLost;

            boost::filesystem::path renderingOutputPath;
            std::string renderingPainterImplementation;
            std::string renderingWriterImplementation;
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

            virtual int getTrackingMaxTipMatchingDistanceInPixels();
            virtual int getTrackingNbTipsToUseToDetectCameraMotion();
            virtual int getTrackingNbDetectionsToComputeAverageTipPositionAndSize();
            virtual int getTrackingMinMatchingDistanceWithAnyObjectToConsiderTipNotHiddenByArm();
            virtual int getTrackingMaxFramesAfterWhichATipIsConsideredLost();
            virtual int getTrackingMinDistanceToConsiderNewTipAsTheSameAsAnExistingOne();
            virtual int getTrackingMinChopstickLengthInPixels();
            virtual int getTrackingMaxChopstickLengthInPixels();
            virtual double getTrackingMinIOUToConsiderTwoTipsAsAChopstick();
            virtual int getTrackingMaxFramesAfterWhichAChopstickIsConsideredLost();

            virtual boost::filesystem::path getRenderingOutputPath();
            virtual std::string getRenderingPainterImplementation();
            virtual std::string getRenderingWriterImplementation();
            virtual int getRenderingVideoFrameMarginsInPixels();
        
        private:
            void loadConfigurationIfNecessary();
    };

}

#endif // SERVICE_CONFIGURATION_READER_IMPL