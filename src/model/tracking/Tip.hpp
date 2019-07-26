#ifndef MODEL_TIP
#define MODEL_TIP

#include <string>
#include <utility>
#include <boost/circular_buffer.hpp>
#include "../Rectangle.hpp"
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
                double x, double y,
                double width, double height) :
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
                return id == other.id;
            }

            bool operator!= (const Tip& other) const {
                return id != other.id;
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