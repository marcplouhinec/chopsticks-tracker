#ifndef SERVICE_CHOPSTICK_TRACKER
#define SERVICE_CHOPSTICK_TRACKER

#include <list>
#include <vector>
#include "../model/detection/DetectedObject.hpp"
#include "../model/tracking/FrameOffset.hpp"
#include "../model/tracking/Chopstick.hpp"

namespace service {

    class ChopstickTracker {
        public:
            virtual ~ChopstickTracker() {}

            virtual void updateChopsticksWithNewDetectionResult(
                std::list<model::Chopstick>& chopsticks,
                const std::list<model::Tip>& tips,
                const std::vector<model::DetectedObject>& detectedObjects,
                const model::FrameOffset accumulatedFrameOffset) = 0;
    };

}

#endif // SERVICE_CHOPSTICK_TRACKER