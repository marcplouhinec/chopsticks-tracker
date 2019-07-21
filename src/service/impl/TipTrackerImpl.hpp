#ifndef SERVICE_TIP_TRACKER_IMPL
#define SERVICE_TIP_TRACKER_IMPL

#include <functional>
#include <unordered_set>
#include <vector>
#include "../TipTracker.hpp"
#include "../ConfigurationReader.hpp"

namespace service {

    class TipTrackerImpl : public TipTracker {
        private:
            ConfigurationReader& configurationReader;

        public:
            TipTrackerImpl(ConfigurationReader& configurationReader) :
                configurationReader(configurationReader) {}

            virtual ~TipTrackerImpl() {}

            virtual model::FrameOffset computeOffsetToCompensateForCameraMotion(
                model::FrameDetectionResult& prevDetectionResult,
                model::FrameDetectionResult& currDetectionResult);

            virtual void updateTipsWithNewDetectionResult(
                std::list<model::Tip>& tips,
                model::FrameDetectionResult& detectionResult);

        private:
            struct ObjectMatchResult {
                model::Rectangle& prevFrameObject;
                model::Rectangle& currFrameObject;
                double matchingDistance;

                bool operator< (const ObjectMatchResult& other) const {
                    return matchingDistance < other.matchingDistance;
                }
            };

        private:
            std::vector<std::reference_wrapper<model::Rectangle>> extractObjectsOfTypes(
                std::vector<model::DetectedObject>& detectedObjects,
                const std::vector<model::DetectedObjectType>& objectTypes);

            /**
             * For each tip from the current frame, try to match it with another tip from the previous
             * frame (by computing its "matching distance"). The result is sorted by matching distance
             * (acending order).
             */
            std::vector<ObjectMatchResult> matchEachTipFromTheCurrentFrameWithOneFromThePreviousFrame(
                std::vector<std::reference_wrapper<model::Rectangle>>& prevFrameDetectedTips,
                std::vector<std::reference_wrapper<model::Rectangle>>& currFrameDetectedTips);

            std::unordered_set<std::string> findTipIdsHiddenByAnArm(
                std::list<model::Tip>& tips, model::FrameDetectionResult& detectionResult);

            bool isDetectedTipTooCloseToExistingTips(
                const model::DetectedObject& detectedTip,
                const std::list<model::Tip>& tips);

            model::Tip makeTip(model::DetectedObject& detectedObject, int frameIndex, int tipIndex);

            double computeMatchingDistance(const model::Rectangle& object1, const model::Rectangle& object2);
    };

}

#endif // SERVICE_TIP_TRACKER_IMPL