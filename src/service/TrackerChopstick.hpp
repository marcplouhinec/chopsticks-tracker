#ifndef SERVICE_TRACKER_CHOPSTICK
#define SERVICE_TRACKER_CHOPSTICK

#include <list>
#include <vector>
#include "../model/detection/DetectedObject.hpp"
#include "../model/tracking/FrameOffset.hpp"
#include "../model/tracking/Chopstick.hpp"

namespace service {

    class TrackerChopstick {
        public:
            virtual ~TrackerChopstick() {}

            virtual void updateChopsticksWithNewDetectionResult(
                std::list<model::Chopstick>& chopsticks,
                const std::list<model::Tip>& tips,
                const std::vector<model::DetectedObject>& detectedObjects,
                const model::FrameOffset accumulatedFrameOffset) const = 0;
    };

}

#endif // SERVICE_TRACKER_CHOPSTICK