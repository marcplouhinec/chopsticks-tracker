#ifndef SERVICE_TIP_TRACKER
#define SERVICE_TIP_TRACKER

#include <vector>
#include "../model/detection/DetectedObject.hpp"
#include "../model/tracking/FrameOffset.hpp"

namespace service {

    class TipTracker {
        public:
            virtual ~TipTracker() {}

            virtual model::FrameOffset computeOffsetToCompensateForCameraMotion(
                std::vector<model::DetectedObject>& prevFrameObjects,
                std::vector<model::DetectedObject>& currFrameObjects) = 0;
    };

}

#endif // SERVICE_TIP_TRACKER