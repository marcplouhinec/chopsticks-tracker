#ifndef SERVICE_TIP_TRACKER
#define SERVICE_TIP_TRACKER

#include <list>
#include <vector>
#include "../model/detection/DetectedObject.hpp"
#include "../model/tracking/FrameOffset.hpp"
#include "../model/tracking/Tip.hpp"

namespace service {

    class TipTracker {
        public:
            virtual ~TipTracker() {}

            virtual model::FrameOffset computeOffsetToCompensateForCameraMotion(
                const std::vector<model::DetectedObject>& prevDetectedObjects,
                const std::vector<model::DetectedObject>& currDetectedObjects) = 0;

            virtual void updateTipsWithNewDetectionResult(
                std::list<model::Tip>& tips,
                const std::vector<model::DetectedObject>& detectedObjects,
                const int frameIndex,
                const model::FrameOffset frameOffset,
                const model::FrameOffset accumulatedFrameOffset) = 0;
    };

}

#endif // SERVICE_TIP_TRACKER