#ifndef MODEL_CHOPSTICK
#define MODEL_CHOPSTICK

#include <string>
#include <boost/circular_buffer.hpp>
#include "../Rectangle.hpp"
#include "Tip.hpp"
#include "TrackingStatus.hpp"

namespace model {

    class Chopstick { // TODO orientation (tip1 to ti2 or reverse)
        public:
            std::string id;
            const Tip& tip1;
            const Tip& tip2;
            boost::circular_buffer<TrackingStatus> recentTrackingStatuses;
            boost::circular_buffer<double> recentIous;
            bool isRejectedBecauseOfConflict;

        public:
            Chopstick(
                std::string id,
                const Tip& tip1,
                const Tip& tip2,
                boost::circular_buffer<TrackingStatus> recentTrackingStatuses,
                boost::circular_buffer<double> recentIous,
                bool isRejectedBecauseOfConflict) :
                    id(id),
                    tip1(tip1),
                    tip2(tip2),
                    recentTrackingStatuses(recentTrackingStatuses),
                    recentIous(recentIous),
                    isRejectedBecauseOfConflict(isRejectedBecauseOfConflict) {}
    };
}

#endif // MODEL_CHOPSTICK