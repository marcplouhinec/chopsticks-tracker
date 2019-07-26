#include <set>
#include <unordered_set>
#include "ChopstickTrackerImpl.hpp"

using namespace model;
using namespace service;
using std::list;
using std::map;
using std::min;
using std::nullopt;
using std::optional;
using std::reference_wrapper;
using std::set;
using std::string;
using std::unordered_set;
using std::vector;
using boost::circular_buffer;

void ChopstickTrackerImpl::updateChopsticksWithNewDetectionResult(
    list<Chopstick>& chopsticks,
    const list<Tip>& tips,
    const vector<DetectedObject>& detectedObjects,
    const FrameOffset accumulatedFrameOffset) {
    
    // Extract the detected chopsticks and translate them according to the frame offset
    auto untranslatedDetectedChopsticks = extractChopstickObjects(detectedObjects);

    vector<DetectedObject> detectedChopsticks;
    for (const Rectangle& untranslatedDetectedChopstick : untranslatedDetectedChopsticks) {
        const DetectedObject& untranslatedDetectedObject =
            (const DetectedObject&) untranslatedDetectedChopstick;

        detectedChopsticks.push_back(untranslatedDetectedObject.copyAndTranslate(
            -accumulatedFrameOffset.dx, -accumulatedFrameOffset.dy));
    }

    // Try to match tips with each others by using detected chopsticks
    auto matchResults = matchTipsWithDetectedChopsticks(tips, detectedChopsticks);
    
    // Find the conflict-less results independently from existing chopsticks
    auto bestMatchResultsInCurrentFrameOnly = filterMatchResultsByRemovingConflictingOnes(matchResults, {});

    // Find the conflict-less results by considering existing chopsticks
    auto bestMatchResults = filterMatchResultsByRemovingConflictingOnes(matchResults, chopsticks);

    // Extract the conflicts between bestMatchResultsInFrame and bestMatchResults
    auto conflictingResults = compareAndExtractConflictingResults(
        bestMatchResults, bestMatchResultsInCurrentFrameOnly);

    // Index tips by their IDs
    map<string, Tip> tipById;
    for (const Tip& tip : tips) {
        tipById.emplace(tip.id, tip);
    }

    // Update the existing chopsticks
    unordered_set<
        ChopstickTrackerImpl::ChopstickMatchResult,
        ChopstickTrackerImpl::ChopstickMatchResult::Hasher> processedMatchResults;
    for (Chopstick& chopstick : chopsticks) {
        // Check if the chopstick is lost because one of its tips doesn't exist anymore
        if (tipById.find(chopstick.tip1Id) == tipById.end() || tipById.find(chopstick.tip2Id) == tipById.end()) {
            chopstick.recentTrackingStatuses.push_back(TrackingStatus::LOST);
            continue;
        }
        const Tip& tip1 = tipById[chopstick.tip1Id];
        const Tip& tip2 = tipById[chopstick.tip2Id];

        // Check if the chopstick was matched in this frame
        auto matchResultOptional = findMatchResultByTips(bestMatchResults, chopstick.tip1Id, chopstick.tip2Id);
        if (matchResultOptional.has_value()) {
            auto matchResult = matchResultOptional.value();
            processedMatchResults.insert(matchResult);

            chopstick.recentTrackingStatuses.push_back(TrackingStatus::DETECTED);
            chopstick.recentIous.push_back(matchResult.iou);
            chopstick.isRejectedBecauseOfConflict = false;
            continue;
        }

        // Check if the chopstick would have been matched without considering history (= conflicts with previous detections)
        auto conflictingMatchResultOptional = findMatchResultByTips(conflictingResults, chopstick.tip1Id, chopstick.tip2Id);
        if (conflictingMatchResultOptional.has_value()) {
            auto matchResult = conflictingMatchResultOptional.value();
            processedMatchResults.insert(matchResult);

            chopstick.recentTrackingStatuses.push_back(TrackingStatus::DETECTED);
            chopstick.recentIous.push_back(matchResult.iou);
            chopstick.isRejectedBecauseOfConflict = true;
            continue;
        }

        // Check if the chopstick is hidden by an arm
        TrackingStatus tip1Status = tip1.recentTrackingStatuses.back();
        TrackingStatus tip2Status = tip2.recentTrackingStatuses.back();
        if (tip1Status == TrackingStatus::HIDDEN_BY_ARM || tip2Status == TrackingStatus::HIDDEN_BY_ARM) {
            chopstick.recentTrackingStatuses.push_back(TrackingStatus::HIDDEN_BY_ARM);
            continue;
        }

        // Check if the chopstick is lost because it has been undetected for too long
        bool chopstickLost = true;
        for (TrackingStatus status : chopstick.recentTrackingStatuses) {
            if (status == TrackingStatus::DETECTED || status == TrackingStatus::HIDDEN_BY_ARM) {
                chopstickLost = false;
                break;
            }
        }

        chopstick.recentTrackingStatuses.push_back(
            chopstickLost ? TrackingStatus::LOST : TrackingStatus::NOT_DETECTED);
    }

    // Remove lost chopsticks
    chopsticks.remove_if([](const Chopstick& chopstick) {
        auto& status = chopstick.recentTrackingStatuses.back();
        return status == TrackingStatus::LOST;
    });

    // Add new chopsticks
    for (auto& matchResult : bestMatchResults) {
        if (processedMatchResults.find(matchResult) == processedMatchResults.end()) {
            chopsticks.push_back(makeChopstick(matchResult, /* isRejectedBecauseOfConflict = */ false));
        }
    }
    for (auto& matchResult : conflictingResults) {
        if (processedMatchResults.find(matchResult) == processedMatchResults.end()) {
            chopsticks.push_back(makeChopstick(matchResult, /* isRejectedBecauseOfConflict = */ true));
        }
    }

    // Find chopsticks in conflicts and switch their "rejected" status by comparing their detections
    struct IouDescComparator {
        bool operator() (const ChopstickAndIou& c1, const ChopstickAndIou& c2) const {
            if (c1.iou != c2.iou) {
                return c1.iou > c2.iou;
            }

            // Avoid different ChopstickAndIous with the same IoU to be considered as equal
            return c1.chopstick.id.compare(c2.chopstick.id) < 0;
        }
    };

    set<ChopstickTrackerImpl::ChopstickAndIou, IouDescComparator> chopsticksAndIous;
    for (const Chopstick& chopstick : chopsticks) {
        double iouSum = 0;
        for (const double iou : chopstick.recentIous) {
            iouSum += iou;
        }

        chopsticksAndIous.insert({chopstick, iouSum});
    }

    // Find chopsticks to accept and reject
    unordered_set<string> rejectedChopstickIds;
    unordered_set<string> acceptedChopstickIds;
    for (auto& chopstickAndIou : chopsticksAndIous) {
        const string& chopstickId = chopstickAndIou.chopstick.id;
        if (rejectedChopstickIds.find(chopstickId) != rejectedChopstickIds.end()) {
            continue;
        }

        acceptedChopstickIds.insert(chopstickId);

        // Rejected conflicts
        for (const Chopstick& chopstick : chopsticks) {
            if (chopstick.id == chopstickId) {
                continue;
            }
            if (chopstickAndIou.chopstick.tip1Id == chopstick.tip1Id ||
                chopstickAndIou.chopstick.tip1Id == chopstick.tip2Id ||
                chopstickAndIou.chopstick.tip2Id == chopstick.tip1Id ||
                chopstickAndIou.chopstick.tip2Id == chopstick.tip2Id) {
                rejectedChopstickIds.insert(chopstick.id);
            }
        }
    }

    // Update the chopsticks rejection statuses
    for (Chopstick& chopstick : chopsticks) {
        if (acceptedChopstickIds.find(chopstick.id) != acceptedChopstickIds.end()) {
            chopstick.isRejectedBecauseOfConflict = false;
        } else if (rejectedChopstickIds.find(chopstick.id) != rejectedChopstickIds.end()) {
            chopstick.isRejectedBecauseOfConflict = true;
        }
    }
}

vector<reference_wrapper<const DetectedObject>> ChopstickTrackerImpl::extractChopstickObjects(
    const vector<DetectedObject>& detectedObjects) {

    vector<reference_wrapper<const DetectedObject>> filteredObjects;

    for (const DetectedObject& detectedObject : detectedObjects) {
        if (detectedObject.objectType == DetectedObjectType::CHOPSTICK) {
            filteredObjects.push_back(detectedObject);
        }
    }

    return filteredObjects;
}

vector<ChopstickTrackerImpl::ChopstickMatchResult> ChopstickTrackerImpl::matchTipsWithDetectedChopsticks(
    const list<Tip>& tips, const vector<DetectedObject>& detectedChopsticks) {

    int minChopstickLength = configuration.trackingMinChopstickLengthInPixels;
    int maxChopstickLength = configuration.trackingMaxChopstickLengthInPixels;
    double minIOUToConsiderTwoTipsAsAChopstick = configuration.trackingMinIOUToConsiderTwoTipsAsAChopstick;

    struct IouDescComparator {
        bool operator() (const ChopstickMatchResult& r1, const ChopstickMatchResult& r2) const {
            if (r1.iou != r2.iou) {
                return r1.iou > r2.iou;
            }

            // Avoid different ChopstickMatchResults with the same IoU to be considered as equal
            if (r1.tip1.id != r2.tip1.id) {
                return r1.tip1.id.compare(r2.tip1.id) < 0;
            }
            if (r1.tip2.id != r2.tip2.id) {
                return r1.tip2.id.compare(r2.tip2.id) < 0;
            }
            return ((Rectangle) r1.detectedChopstick) < ((Rectangle) r2.detectedChopstick);
        }
    };

    set<ChopstickTrackerImpl::ChopstickMatchResult, IouDescComparator> matchResults;
    for (const Tip& tip1 : tips) {
        for (const Tip& tip2 : tips) {
            if (tip1 == tip2) {
                continue;
            }

            // Do not consider tips that are too close or too far from each other
            double distance = Rectangle::distanceBetweenTopLeftPoints(tip1, tip2);
            if (distance < minChopstickLength || distance > maxChopstickLength) {
                continue;
            }

            // Try to match the two tips with a detected chopstick
            Rectangle tipsBoundingBox = Rectangle::getBoundingBox(tip1, tip2);
            int boundingBoxArea = tipsBoundingBox.area();
            for (const DetectedObject& detectedChopstick : detectedChopsticks) {
                if (!tipsBoundingBox.isOverlappingWith(detectedChopstick)) {
                    continue;
                }

                Rectangle intersection = Rectangle::getIntersection(tipsBoundingBox, detectedChopstick);
                int intersectionArea = intersection.area();
                int unionArea = boundingBoxArea + detectedChopstick.area() - intersectionArea;
                double iou = ((double) intersectionArea) / ((double) unionArea);

                if (iou >= minIOUToConsiderTwoTipsAsAChopstick) {
                    ChopstickTrackerImpl::ChopstickMatchResult matchResult = {
                        tip1, tip2, detectedChopstick, iou
                    };
                    matchResults.insert(matchResult);
                }
            }
        }
    }

    vector<ChopstickTrackerImpl::ChopstickMatchResult> matchResultsAsVector;
    for (auto& matchResult : matchResults) {
        matchResultsAsVector.push_back(matchResult);
    }
    return matchResultsAsVector;
}

vector<ChopstickTrackerImpl::ChopstickMatchResult> ChopstickTrackerImpl::filterMatchResultsByRemovingConflictingOnes(
    const vector<ChopstickTrackerImpl::ChopstickMatchResult>& matchResults,
    const list<Chopstick>& existingChopsticks) {
    
    vector<ChopstickTrackerImpl::ChopstickMatchResult> filteredMatchResults;
    unordered_set<Rectangle, Rectangle::Hasher> alreadyMatchedTips;
    unordered_set<Rectangle, Rectangle::Hasher> alreadyMatchedChopsticks;
    for (auto& matchResult : matchResults) {
        // Ignore this result if any of its elements is conflicting with an already selected match result
        if (alreadyMatchedTips.find(matchResult.tip1) != alreadyMatchedTips.end() ||
            alreadyMatchedTips.find(matchResult.tip2) != alreadyMatchedTips.end() ||
            alreadyMatchedChopsticks.find(matchResult.detectedChopstick) != alreadyMatchedChopsticks.end()) {
            continue;
        }

        // Ignore this result if it conflicts with an existing chopstick
        bool hasConflictWithExistingChopstick = false;
        for (const Chopstick& chopstick : existingChopsticks) {
            if (chopstick.isRejectedBecauseOfConflict) {
                continue;
            }

            // Check if the chopstick shares at least one tip with this match result
            if (chopstick.tip1Id != matchResult.tip1.id && chopstick.tip1Id != matchResult.tip2.id &&
                chopstick.tip2Id != matchResult.tip1.id && chopstick.tip2Id != matchResult.tip2.id) {
                continue;
            }

            // Check if the chopstick and the match result are not identical
            if ((chopstick.tip1Id == matchResult.tip1.id && chopstick.tip2Id == matchResult.tip2.id) ||
                (chopstick.tip2Id == matchResult.tip1.id && chopstick.tip1Id == matchResult.tip2.id)) {
                continue;
            }

            hasConflictWithExistingChopstick = true;
            break;
        }

        if (hasConflictWithExistingChopstick) {
            continue;
        }

        // No conflict, keep this match result
        filteredMatchResults.push_back(matchResult);
        alreadyMatchedTips.insert(matchResult.tip1);
        alreadyMatchedTips.insert(matchResult.tip2);
        alreadyMatchedChopsticks.insert(matchResult.detectedChopstick);
    }

    return filteredMatchResults;
}

vector<ChopstickTrackerImpl::ChopstickMatchResult> ChopstickTrackerImpl::compareAndExtractConflictingResults(
    const vector<ChopstickTrackerImpl::ChopstickMatchResult>& referenceResults,
    const vector<ChopstickTrackerImpl::ChopstickMatchResult>& resultsToFilter) {
    
    vector<ChopstickTrackerImpl::ChopstickMatchResult> conflictingResults;

    for (auto& resultToFilter : resultsToFilter) {
        bool hasConflict = true;

        for (auto& referenceResult : referenceResults) {
            bool sameTips =
                (resultToFilter.tip1 == referenceResult.tip1 && resultToFilter.tip2 == referenceResult.tip2) ||
                (resultToFilter.tip2 == referenceResult.tip1 && resultToFilter.tip1 == referenceResult.tip2);

            bool sameChopstick = resultToFilter.detectedChopstick == referenceResult.detectedChopstick;

            if (sameTips && sameChopstick) {
                hasConflict = false;
                break;
            }
        }
            
        if (hasConflict) {
            conflictingResults.push_back(resultToFilter);
        }
    }

    return conflictingResults;
}

optional<ChopstickTrackerImpl::ChopstickMatchResult> ChopstickTrackerImpl::findMatchResultByTips(
    const vector<ChopstickTrackerImpl::ChopstickMatchResult>& matchResults,
    const std::string& tip1Id,
    const std::string& tip2Id) {
    
    for (auto& matchResult : matchResults) {
        Tip& mrTip1 = (Tip&) matchResult.tip1;
        Tip& mrTip2 = (Tip&) matchResult.tip2;

        if ((mrTip1.id == tip1Id && mrTip2.id == tip2Id) || (mrTip1.id == tip2Id && mrTip2.id == tip1Id)) {
            return std::optional<ChopstickTrackerImpl::ChopstickMatchResult>{ matchResult };
        }
    }

    return nullopt;
}

Chopstick ChopstickTrackerImpl::makeChopstick(
    const ChopstickTrackerImpl::ChopstickMatchResult& matchResult,
    const bool isRejectedBecauseOfConflict) {

    int maxFramesAfterWhichAChopstickIsConsideredLost = 
        configuration.trackingMaxFramesAfterWhichAChopstickIsConsideredLost;

    circular_buffer<TrackingStatus> recentTrackingStatuses(maxFramesAfterWhichAChopstickIsConsideredLost);
    recentTrackingStatuses.push_back(TrackingStatus::DETECTED_ONCE);

    circular_buffer<double> recentIous(maxFramesAfterWhichAChopstickIsConsideredLost);
    recentIous.push_back(matchResult.iou);
    
    return Chopstick(
        "C_" + matchResult.tip1.id + "_" + matchResult.tip2.id,
        matchResult.tip1.id,
        matchResult.tip2.id,
        recentTrackingStatuses,
        recentIous,
        isRejectedBecauseOfConflict);
}