#ifndef SERVICE_CHOPSTICK_TRACKER
#define SERVICE_CHOPSTICK_TRACKER

#include <list>
#include "../model/detection/FrameDetectionResult.hpp"
#include "../model/tracking/Chopstick.hpp"

namespace service {

    class ChopstickTracker {
        public:
            virtual ~ChopstickTracker() {}

            virtual void updateChopsticksWithNewDetectionResult(
                std::list<model::Chopstick>& chopsticks,
                const std::list<model::Tip>& tips,
                model::FrameDetectionResult& detectionResult) = 0;
    };

}

#endif // SERVICE_CHOPSTICK_TRACKER