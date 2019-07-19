#ifndef SERVICE_TIP_TRACKER
#define SERVICE_TIP_TRACKER

#include <vector>
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
                std::vector<model::Tip>& tips,
                model::FrameDetectionResult& detectionResult) = 0;
    };

}

#endif // SERVICE_TIP_TRACKER