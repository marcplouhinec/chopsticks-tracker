#ifndef SERVICE_CHOPSTICK_TRACKER_IMPL
#define SERVICE_CHOPSTICK_TRACKER_IMPL

#include <optional>
#include <vector>
#include "../../model/Configuration.hpp"
#include "../../model/tracking/Tip.hpp"
#include "../ChopstickTracker.hpp"

namespace service {

    class ChopstickTrackerImpl : public ChopstickTracker {
        private:
            model::Configuration& configuration;

        public:
            ChopstickTrackerImpl(model::Configuration& configuration) : configuration(configuration) {}

            virtual ~ChopstickTrackerImpl() {}

            virtual void updateChopsticksWithNewDetectionResult(
                std::list<model::Chopstick>& chopsticks,
                const std::list<model::Tip>& tips,
                const std::vector<model::DetectedObject>& detectedObjects,
                const model::FrameOffset accumulatedFrameOffset);
        
        private:
            struct ChopstickMatchResult {
                const model::Tip& tip1;
                const model::Tip& tip2;
                const model::DetectedObject& detectedChopstick;
                const double iou;

                bool operator== (const ChopstickMatchResult& other) const {
                    return tip1 == other.tip1 && tip2 == other.tip2 &&
                        detectedChopstick == other.detectedChopstick;
                }

                struct Hasher
                {
                    model::Tip::Hasher tipHasher;
                    model::DetectedObject::Hasher detectedObjectHasher;

                    std::size_t operator()(const ChopstickMatchResult& r) const
                    {
                        std::size_t res = 17;
                        res = res * 31 + tipHasher(r.tip1);
                        res = res * 31 + tipHasher(r.tip2);
                        res = res * 31 + detectedObjectHasher(r.detectedChopstick);
                        res = res * 31 + std::hash<double>()( r.iou );
                        return res;
                    }
                };
            };
            struct ChopstickAndIou {
                const model::Chopstick& chopstick;
                const double iou;
            };

        private:
            std::vector<std::reference_wrapper<const model::DetectedObject>> extractChopstickObjects(
                const std::vector<model::DetectedObject>& detectedObjects);
            
            std::vector<ChopstickMatchResult> matchTipsWithDetectedChopsticks(
                const std::list<model::Tip>& tips,
                const std::vector<model::DetectedObject>& detectedChopsticks);

            /**
             * The matchResults input parameter contains many potential good matches between tips
             * and chopsticks. This method only keeps the best matches and remove the conflicting ones
             * (e.g same tip or same detected chopsticks).
             * 
             * @param matchResults
             *     Match results to filter.
             * @param existingChopsticks
             *     Existing chopsticks that might cause conflicts with the matchResults. If it happens,
             *     the conflicting existing chopsticks have the priority.
             * @return
             *     Filtered matchResults with the best matches without conflicts.
             */
            std::vector<ChopstickMatchResult> filterMatchResultsByRemovingConflictingOnes(
                const std::vector<ChopstickMatchResult>& matchResults,
                const std::list<model::Chopstick>& existingChopsticks);

            std::vector<ChopstickMatchResult> compareAndExtractConflictingResults(
                const std::vector<ChopstickMatchResult>& referenceResults,
                const std::vector<ChopstickMatchResult>& resultsToFilter);
            
            std::optional<ChopstickMatchResult> findMatchResultByTips(
                const std::vector<ChopstickMatchResult>& matchResults,
                const std::string& tip1Id,
                const std::string& tip2Id);

            model::Chopstick makeChopstick(
                const ChopstickMatchResult& matchResult,
                const bool isRejectedBecauseOfConflict);
    };

}

#endif // SERVICE_CHOPSTICK_TRACKER_IMPL