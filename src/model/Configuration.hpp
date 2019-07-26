#ifndef MODEL_CONFIGURATION
#define MODEL_CONFIGURATION

#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include "detection/DetectedObjectType.hpp"

namespace model {

    class Configuration {
        public:
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
            bool renderingDetectedObjectsPainterShowTips;
            bool renderingDetectedObjectsPainterShowChopsticks;
            bool renderingDetectedObjectsPainterShowArms;
            bool renderingTrackedObjectsPainterShowTips;
            bool renderingTrackedObjectsPainterShowAcceptedChopsticks;
            bool renderingTrackedObjectsPainterShowRejectedChopsticks;
            std::string renderingWriterImplementation;
            int renderingVideoFrameMarginsInPixels;
        
        public:
            Configuration() {}
    };
}

#endif // MODEL_CONFIGURATION