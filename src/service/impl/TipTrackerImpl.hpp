#ifndef SERVICE_TIP_TRACKER_IMPL
#define SERVICE_TIP_TRACKER_IMPL

#include <functional>
#include <unordered_set>
#include <vector>
#include "../../model/Configuration.hpp"
#include "../TipTracker.hpp"

namespace service {

    class TipTrackerImpl : public TipTracker {
        private:
            const model::Configuration& configuration;

        public:
            TipTrackerImpl(const model::Configuration& configuration) : configuration(configuration) {}

            virtual ~TipTrackerImpl() {}

            virtual model::FrameOffset computeOffsetToCompensateForCameraMotion(
                const std::vector<model::DetectedObject>& prevDetectedObjects,
                const std::vector<model::DetectedObject>& currDetectedObjects) const;

            virtual void updateTipsWithNewDetectionResult(
                std::list<model::Tip>& tips,
                const std::vector<model::DetectedObject>& detectedObjects,
                const int frameIndex,
                const model::FrameOffset frameOffset,
                const model::FrameOffset accumulatedFrameOffset) const;

        private:
            struct ObjectMatchResult {
                const model::Rectangle& prevFrameObject;
                const model::Rectangle& currFrameObject;
                const double matchingDistance;
            };

        private:
            std::vector<std::reference_wrapper<const model::Rectangle>> extractObjectsOfTypes(
                const std::vector<model::DetectedObject>& detectedObjects,
                const std::vector<model::DetectedObjectType>& objectTypes) const;

            std::vector<model::DetectedObject> copyAndTranslateDetectedObjects(
                const std::vector<std::reference_wrapper<const model::Rectangle>>& detectedObjects,
                const double dx,
                const double dy) const;

            /**
             * For each tip from the current frame, try to match it with another tip from the previous
             * frame (by computing its "matching distance"). The result is sorted by matching distance
             * (acending order).
             */
            std::vector<ObjectMatchResult> matchEachTipFromTheCurrentFrameWithOneFromThePreviousFrame(
                const std::vector<std::reference_wrapper<const model::Rectangle>>& prevFrameDetectedTips,
                const std::vector<std::reference_wrapper<const model::Rectangle>>& currFrameDetectedTips) const;

            std::unordered_set<std::string> findTipIdsHiddenByAnArm(
                const std::list<model::Tip>& tips,
                const std::vector<model::DetectedObject>& detectedObjects,
                const model::FrameOffset& accumulatedFrameOffset) const;

            bool isDetectedTipTooCloseToExistingTips(
                const model::DetectedObject& detectedTip,
                const std::list<model::Tip>& tips) const;

            model::Tip makeTip(
                const model::DetectedObject& detectedObject,
                const int frameIndex,
                const int tipIndex) const;

            double computeMatchingDistance(
                const model::Rectangle& object1,
                const model::Rectangle& object2) const;
    };

}

#endif // SERVICE_TIP_TRACKER_IMPL