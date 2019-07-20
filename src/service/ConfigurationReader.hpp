#ifndef SERVICE_CONFIGURATION_READER
#define SERVICE_CONFIGURATION_READER

#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include "../model/detection/DetectedObjectType.hpp"

namespace service {

    class ConfigurationReader {
        public:
            virtual ~ConfigurationReader() {}

            virtual std::vector<std::string> getYoloModelClassNames() = 0;
            virtual std::vector<model::DetectedObjectType> getYoloModelClassEnums() = 0;
            virtual boost::filesystem::path getYoloModelCfgPath() = 0;
            virtual boost::filesystem::path getYoloModelWeightsPath() = 0;

            virtual float getObjectDetectionMinTipConfidence() = 0;
            virtual float getObjectDetectionMinChopstickConfidence() = 0;
            virtual float getObjectDetectionMinArmConfidence() = 0;
            virtual float getObjectDetectionNmsThreshold() = 0;
            virtual std::string getObjectDetectionImplementation() = 0;
            virtual boost::filesystem::path getObjectDetectionCacheFolderPath() = 0;

            virtual int getTrackingMaxTipMatchingDistanceInPixels() = 0;
            virtual int getTrackingNbTipsToUseToDetectCameraMotion() = 0;
            virtual int getTrackingNbDetectionsToComputeAverageTipPositionAndSize() = 0;
            virtual int getTrackingMinMatchingDistanceWithAnyObjectToConsiderTipNotHiddenByArm() = 0;
            virtual int getTrackingMaxFramesAfterWhichATipIsConsideredLost() = 0;
            virtual int getTrackingMinDistanceToConsiderNewTipAsTheSameAsAnExistingOne() = 0;

            virtual boost::filesystem::path getRenderingOutputPath() = 0;
            virtual std::string getRenderingImplementation() = 0;
            virtual int getRenderingVideoFrameMarginsInPixels() = 0;
    };

}

#endif // SERVICE_CONFIGURATION_READER