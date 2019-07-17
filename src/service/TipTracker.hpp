#ifndef SERVICE_TIP_TRACKER
#define SERVICE_TIP_TRACKER

#include <vector>
#include "../model/detection/DetectedObject.hpp"
#include "../model/tracking/FrameOffset.hpp"

namespace service {

    class TipTracker {
        public:
            virtual ~TipTracker() {}

            /**
             * Adapt the coordinates of the current frame objects in order to compensate for the
             * camera motion between the current frame and the previous one.
             * 
             * @param prevFrameObjects
             *     Detected objects in the previous frame.
             * @param currFrameObjects
             *     Detected objects in the current frame. This method modifies their coordinates.
             * @return Offset applied on all objects in the current frame.
             */
            virtual model::FrameOffset adjustObjectsToCompensateForCameraMotion(
                std::vector<model::DetectedObject>& prevFrameObjects,
                std::vector<model::DetectedObject>& currFrameObjects) = 0;
    };

}

#endif // SERVICE_TIP_TRACKER