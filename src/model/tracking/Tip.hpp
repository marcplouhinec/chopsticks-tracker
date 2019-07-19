#ifndef MODEL_TIP
#define MODEL_TIP

#include <string>
#include <utility>
#include <boost/circular_buffer.hpp>
#include "../Rectangle.hpp"
#include "../detection/DetectedObject.hpp"
#include "TrackingStatus.hpp"

namespace model {

    class Tip : public Rectangle {
        public:
            std::string id;
            boost::circular_buffer<Rectangle> recentShapes;
            boost::circular_buffer<TrackingStatus> recentTrackingStatuses;
            int nbDetectionsAsBigTip = 0;
            int nbDetectionsAsSmallTip = 0;

        public:
            Tip() : Rectangle() {}

            explicit Tip(
                std::string id,
                boost::circular_buffer<Rectangle> recentShapes,
                boost::circular_buffer<TrackingStatus> recentTrackingStatuses,
                int nbDetectionsAsBigTip,
                int nbDetectionsAsSmallTip,
                int x, int y,
                int width, int height) :
                    id(id),
                    recentShapes(recentShapes),
                    recentTrackingStatuses(recentTrackingStatuses),
                    nbDetectionsAsSmallTip(nbDetectionsAsSmallTip),
                    nbDetectionsAsBigTip(nbDetectionsAsBigTip),
                    Rectangle(x, y, width, height) {}
            
            bool isBigTip() {
                return nbDetectionsAsBigTip > nbDetectionsAsSmallTip;
            }

            bool operator== (const Tip& other) const {
                return id.compare(other.id) == 0;
            }

            struct Hasher
            {
                std::size_t operator()(const Tip& t) const
                {
                    std::size_t res = 17;
                    res = res * 31 + std::hash<std::string>()( t.id );
                    return res;
                }
            };
    };
}

#endif // MODEL_TIP