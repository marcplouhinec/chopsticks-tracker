#ifndef SERVICE_TIP_TRACKER_IMPL
#define SERVICE_TIP_TRACKER_IMPL

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
                std::vector<model::DetectedObject>& prevFrameObjects,
                std::vector<model::DetectedObject>& currFrameObjects);

        private:
            struct ObjectMatchResult {
                model::DetectedObject& prevFrameObject;
                model::DetectedObject& currFrameObject;
                double matchingDistance;

                bool operator< (const ObjectMatchResult& other) const {
                    return matchingDistance < other.matchingDistance;
                }
            };

        private:
            std::vector<model::DetectedObject> extractDetectedTips(
                std::vector<model::DetectedObject>& detectedObjects);

            /**
             * For each tip from the current frame, try to match it with another tip from the previous
             * frame (by computing its "matching distance"). The result is sorted by matching distance
             * (acending order).
             */
            std::vector<ObjectMatchResult> matchEachTipFromTheCurrentFrameWithOneFromThePreviousFrame(
                std::vector<model::DetectedObject>& prevFrameDetectedTips,
                std::vector<model::DetectedObject>& currFrameDetectedTips);

            double computeMatchingDistance(model::DetectedObject& object1, model::DetectedObject& object2);

            double distance(int x1, int y1, int x2, int y2);
    };

}

#endif // SERVICE_TIP_TRACKER_IMPL