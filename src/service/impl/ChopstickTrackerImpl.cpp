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
    list<Chopstick>& chopsticks, const list<Tip>& tips, FrameDetectionResult& detectionResult) {

    // Try to match tips with each others by using detected chopsticks
    auto detectedChopsticks = extractChopstickObjects(detectionResult.detectedObjects);
    auto matchResults = matchTipsWithDetectedChopsticks(tips, detectedChopsticks);
    
    // Find the conflict-less results independently from existing chopsticks
    auto bestMatchResultsInCurrentFrameOnly = filterMatchResultsByRemovingConflictingOnes(matchResults, {});

    // Find the conflict-less results by considering existing chopsticks
    auto bestMatchResults = filterMatchResultsByRemovingConflictingOnes(matchResults, chopsticks);

    // Extract the conflicts between bestMatchResultsInFrame and bestMatchResults
    auto conflictingResults = compareAndExtractConflictingResults(
        bestMatchResults, bestMatchResultsInCurrentFrameOnly);

    // Make a set of tip IDs in order to find them faster
    unordered_set<string> tipIds;
    for (const Tip tip : tips) {
        tipIds.insert(tip.id);
    }

    // Update the existing chopsticks
    set<ChopstickTrackerImpl::ChopstickMatchResult> processedMatchResults;
    for (Chopstick& chopstick : chopsticks) {
        // Check if the chopstick is lost because one of its tips doesn't exist anymore
        if (tipIds.find(chopstick.tip1.id) == tipIds.end() || tipIds.find(chopstick.tip2.id) == tipIds.end()) {
            chopstick.recentTrackingStatuses.push_back(TrackingStatus::LOST);
            continue;
        }

        // Check if the chopstick was matched in this frame
        auto matchResultOptional = findMatchResultByTips(bestMatchResults, chopstick.tip1, chopstick.tip2);
        if (matchResultOptional.has_value()) {
            auto matchResult = matchResultOptional.value();
            processedMatchResults.insert(matchResult);

            chopstick.recentTrackingStatuses.push_back(TrackingStatus::DETECTED);
            chopstick.recentIous.push_back(matchResult.iou);
            chopstick.isRejectedBecauseOfConflict = false;
            continue;
        }

        // Check if the chopstick would have been matched without considering history (= conflicts with previous detections)
        auto conflictingMatchResultOptional = findMatchResultByTips(conflictingResults, chopstick.tip1, chopstick.tip2);
        if (conflictingMatchResultOptional.has_value()) {
            auto matchResult = conflictingMatchResultOptional.value();
            processedMatchResults.insert(matchResult);

            chopstick.recentTrackingStatuses.push_back(TrackingStatus::DETECTED);
            chopstick.recentIous.push_back(matchResult.iou);
            chopstick.isRejectedBecauseOfConflict = true;
            continue;
        }

        // Check if the chopstick is hidden by an arm
        TrackingStatus tip1Status = chopstick.tip1.recentTrackingStatuses.back();
        TrackingStatus tip2Status = chopstick.tip2.recentTrackingStatuses.back();
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
    set<ChopstickTrackerImpl::ChopstickAndIou> chopsticksAndIous; // Sorted by iou desc
    for (const Chopstick& chopstick : chopsticks) {
        double iouAvg = 0;
        for (const double iou : chopstick.recentIous) {
            iouAvg += iou;
        }
        iouAvg /= chopstick.recentIous.size();

        chopsticksAndIous.insert({chopstick, iouAvg}); // TODO avg or sum?
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
            if (chopstickAndIou.chopstick.tip1 == chopstick.tip1 ||
                chopstickAndIou.chopstick.tip1 == chopstick.tip2 ||
                chopstickAndIou.chopstick.tip2 == chopstick.tip1 ||
                chopstickAndIou.chopstick.tip2 == chopstick.tip2) {
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

vector<reference_wrapper<DetectedObject>> ChopstickTrackerImpl::extractChopstickObjects(
    vector<DetectedObject>& detectedObjects) {
    vector<reference_wrapper<DetectedObject>> filteredObjects;

    for (DetectedObject& detectedObject : detectedObjects) {
        if (detectedObject.objectType == DetectedObjectType::CHOPSTICK) {
            filteredObjects.push_back(detectedObject);
        }
    }

    return filteredObjects;
}

set<ChopstickTrackerImpl::ChopstickMatchResult> ChopstickTrackerImpl::matchTipsWithDetectedChopsticks(
    const list<Tip>& tips, const vector<reference_wrapper<DetectedObject>>& detectedChopsticks) {

    int minChopstickLength = configurationReader.getTrackingMinChopstickLengthInPixels();
    int maxChopstickLength = configurationReader.getTrackingMaxChopstickLengthInPixels();
    double minIOUToConsiderTwoTipsAsAChopstick =
        configurationReader.getTrackingMinIOUToConsiderTwoTipsAsAChopstick();

    set<ChopstickTrackerImpl::ChopstickMatchResult> matchResults; // Automatically sort by IoU desc
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
            for (DetectedObject& detectedChopstick : detectedChopsticks) {
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

    return matchResults;
}

vector<ChopstickTrackerImpl::ChopstickMatchResult> ChopstickTrackerImpl::filterMatchResultsByRemovingConflictingOnes(
    const set<ChopstickTrackerImpl::ChopstickMatchResult>& matchResults,
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
            if (chopstick.tip1 != matchResult.tip1 && chopstick.tip1 != matchResult.tip2 &&
                chopstick.tip2 != matchResult.tip1 && chopstick.tip2 != matchResult.tip2) {
                continue;
            }

            // Check if the chopstick and the match result are not identical
            if ((chopstick.tip1 == matchResult.tip1 && chopstick.tip2 == matchResult.tip2) ||
                (chopstick.tip2 == matchResult.tip1 && chopstick.tip1 == matchResult.tip2)) {
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
    const Tip& tip1,
    const Tip& tip2) {
    
    for (auto& matchResult : matchResults) {
        Tip& mrTip1 = (Tip&) matchResult.tip1;
        Tip& mrTip2 = (Tip&) matchResult.tip2;

        if ((mrTip1 == tip1 && mrTip2 == tip2) || (mrTip1 == tip2 && mrTip2 == tip1)) {
            return std::optional<ChopstickTrackerImpl::ChopstickMatchResult>{ matchResult };
        }
    }

    return nullopt;
}

Chopstick ChopstickTrackerImpl::makeChopstick(
    const ChopstickTrackerImpl::ChopstickMatchResult& matchResult,
    const bool isRejectedBecauseOfConflict) {

    int maxFramesAfterWhichAChopstickIsConsideredLost = 
        configurationReader.getTrackingMaxFramesAfterWhichAChopstickIsConsideredLost();

    circular_buffer<TrackingStatus> recentTrackingStatuses(maxFramesAfterWhichAChopstickIsConsideredLost);
    recentTrackingStatuses.push_back(TrackingStatus::DETECTED_ONCE);

    circular_buffer<double> recentIous(maxFramesAfterWhichAChopstickIsConsideredLost);
    recentIous.push_back(matchResult.iou);
    
    return Chopstick(
        "C_" + matchResult.tip1.id + "_" + matchResult.tip2.id,
        matchResult.tip1,
        matchResult.tip2,
        recentTrackingStatuses,
        recentIous,
        isRejectedBecauseOfConflict);
}