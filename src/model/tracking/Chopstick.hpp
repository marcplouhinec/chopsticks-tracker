#ifndef MODEL_CHOPSTICK
#define MODEL_CHOPSTICK

#include <string>
#include <boost/circular_buffer.hpp>
#include "../Rectangle.hpp"
#include "TrackingStatus.hpp"

namespace model {

    class Chopstick {
        public:
            const std::string id;
            const std::string tip1Id;
            const std::string tip2Id;
            boost::circular_buffer<TrackingStatus> recentTrackingStatuses;
            boost::circular_buffer<double> recentIous;
            bool isRejectedBecauseOfConflict;

        public:
            Chopstick(
                const std::string id,
                const std::string tip1Id,
                const std::string tip2Id,
                boost::circular_buffer<TrackingStatus> recentTrackingStatuses,
                boost::circular_buffer<double> recentIous,
                bool isRejectedBecauseOfConflict) :
                    id(id),
                    tip1Id(tip1Id),
                    tip2Id(tip2Id),
                    recentTrackingStatuses(recentTrackingStatuses),
                    recentIous(recentIous),
                    isRejectedBecauseOfConflict(isRejectedBecauseOfConflict) {}
    };
}

#endif // MODEL_CHOPSTICK