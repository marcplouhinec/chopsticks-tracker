#ifndef SERVICE_TRACKER_TIP
#define SERVICE_TRACKER_TIP

#include <list>
#include <vector>
#include "../model/detection/DetectedObject.hpp"
#include "../model/tracking/FrameOffset.hpp"
#include "../model/tracking/Tip.hpp"

namespace service {

    class TrackerTip {
        public:
            virtual ~TrackerTip() {}

            virtual model::FrameOffset computeOffsetToCompensateForCameraMotion(
                const std::vector<model::DetectedObject>& prevDetectedObjects,
                const std::vector<model::DetectedObject>& currDetectedObjects) const = 0;

            virtual void updateTipsWithNewDetectionResult(
                std::list<model::Tip>& tips,
                const std::vector<model::DetectedObject>& detectedObjects,
                const int frameIndex,
                const model::FrameOffset frameOffset,
                const model::FrameOffset accumulatedFrameOffset) const = 0;
    };

}

#endif // SERVICE_TRACKER_TIP