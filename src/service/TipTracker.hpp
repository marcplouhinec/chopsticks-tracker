#ifndef SERVICE_TIP_TRACKER
#define SERVICE_TIP_TRACKER

#include <list>
#include "../model/detection/FrameDetectionResult.hpp"
#include "../model/tracking/FrameOffset.hpp"
#include "../model/tracking/Tip.hpp"

namespace service {

    class TipTracker {
        public:
            virtual ~TipTracker() {}

            virtual model::FrameOffset computeOffsetToCompensateForCameraMotion(
                model::FrameDetectionResult& prevDetectionResult,
                model::FrameDetectionResult& currDetectionResult) = 0;

            virtual void updateTipsWithNewDetectionResult(
                std::list<model::Tip>& tips,
                model::FrameDetectionResult& detectionResult,
                model::FrameOffset frameOffset,
                model::FrameOffset accumulatedFrameOffset) = 0;
    };

}

#endif // SERVICE_TIP_TRACKER