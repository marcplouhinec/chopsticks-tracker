#ifndef SERVICE_CHOPSTICK_TRACKER_IMPL
#define SERVICE_CHOPSTICK_TRACKER_IMPL

#include <optional>
#include <set>
#include <vector>
#include "../ConfigurationReader.hpp"
#include "../ChopstickTracker.hpp"

namespace service {

    class ChopstickTrackerImpl : public ChopstickTracker {
        private:
            ConfigurationReader& configurationReader;

        public:
            ChopstickTrackerImpl(ConfigurationReader& configurationReader) :
                configurationReader(configurationReader) {}

            virtual ~ChopstickTrackerImpl() {}

            virtual void updateChopsticksWithNewDetectionResult(
                std::list<model::Chopstick>& chopsticks,
                const std::list<model::Tip>& tips,
                model::FrameDetectionResult& detectionResult);
        
        private:
            struct ChopstickMatchResult {
                const model::Tip& tip1;
                const model::Tip& tip2;
                const model::DetectedObject& detectedChopstick;
                const double iou;

                bool operator< (const ChopstickMatchResult& other) const {
                    return iou > other.iou; // Inverted
                }

                bool operator== (const ChopstickMatchResult& other) const {
                    return tip1 == other.tip1 && tip2 == other.tip2 &&
                        detectedChopstick == other.detectedChopstick;
                }
            };
            struct ChopstickAndIou {
                const model::Chopstick& chopstick;
                const double iou;

                bool operator< (const ChopstickAndIou& other) const {
                    return iou > other.iou; // Inverted
                }
            };

        private:
            std::vector<std::reference_wrapper<model::DetectedObject>> extractChopstickObjects(
                std::vector<model::DetectedObject>& detectedObjects);
            
            std::set<ChopstickMatchResult> matchTipsWithDetectedChopsticks(
                const std::list<model::Tip>& tips,
                const std::vector<std::reference_wrapper<model::DetectedObject>>& detectedChopsticks);

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
                const std::set<ChopstickMatchResult>& matchResults,
                const std::list<model::Chopstick>& existingChopsticks);

            std::vector<ChopstickMatchResult> compareAndExtractConflictingResults(
                const std::vector<ChopstickMatchResult>& referenceResults,
                const std::vector<ChopstickMatchResult>& resultsToFilter);
            
            std::optional<ChopstickMatchResult> findMatchResultByTips(
                const std::vector<ChopstickMatchResult>& matchResults,
                const model::Tip& tip1,
                const model::Tip& tip2);

            model::Chopstick makeChopstick(
                const ChopstickMatchResult& matchResult,
                const bool isRejectedBecauseOfConflict);
    };

}

#endif // SERVICE_CHOPSTICK_TRACKER_IMPL