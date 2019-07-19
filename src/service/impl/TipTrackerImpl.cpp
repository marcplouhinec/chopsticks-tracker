#include <algorithm>
#include <math.h>
#include <map>
#include <set>
#include <boost/range/algorithm_ext/erase.hpp>
#include "TipTrackerImpl.hpp"

using namespace model;
using namespace service;
using std::find;
using std::make_pair;
using std::map;
using std::max;
using std::min;
using std::pair;
using std::reference_wrapper;
using std::round;
using std::set;
using std::string;
using std::to_string;
using std::unordered_set;
using std::vector;
using boost::circular_buffer;
using boost::remove_erase_if;

FrameOffset TipTrackerImpl::computeOffsetToCompensateForCameraMotion(
    FrameDetectionResult& prevDetectionResult, FrameDetectionResult& currDetectionResult) {

    // Focus exclusively on the detected tips
    auto tipTypes = { DetectedObjectType::SMALL_TIP, DetectedObjectType::BIG_TIP };
    auto prevFrameTips = extractObjectsOfTypes(prevDetectionResult.detectedObjects, tipTypes);
    auto currFrameTips = extractObjectsOfTypes(currDetectionResult.detectedObjects, tipTypes);

    // For each tip of the current frame, try to match it with a tip from the previous frame
    vector<TipTrackerImpl::ObjectMatchResult> matchResults =
        matchEachTipFromTheCurrentFrameWithOneFromThePreviousFrame(prevFrameTips, currFrameTips);

    // Only select the best match results
    int nbTipsToUseToDetectCameraMotion = configurationReader.getTrackingNbTipsToUseToDetectCameraMotion();
    int nbBestMatchResults = min((int) matchResults.size(), nbTipsToUseToDetectCameraMotion);

    // Compute the average distance between the tips from the current frame and their
    // matchings from the previous frame.
    double dx = 0;
    double dy = 0;
    for (int i = 0; i < nbBestMatchResults; i++) {
        auto& matchResult = matchResults[i];

        dx += matchResult.currFrameObject.x - matchResult.prevFrameObject.x;
        dy += matchResult.currFrameObject.y - matchResult.prevFrameObject.y;
    }
    dx = dx / nbBestMatchResults;
    dy = dy / nbBestMatchResults;

    return FrameOffset(dx, dy);
}

void TipTrackerImpl::updateTipsWithNewDetectionResult(
    vector<Tip>& tips, FrameDetectionResult& detectionResult) {

    // Load configuration
    int nbDetectionsToComputeAverageTipPositionAndSize =
        configurationReader.getTrackingNbDetectionsToComputeAverageTipPositionAndSize();
    int maxFramesAfterWhichATipIsConsideredLost =
        configurationReader.getTrackingMaxFramesAfterWhichATipIsConsideredLost();
    int maxFramesAfterWhichATipHiddenByArmIsConsideredLost =
        configurationReader.getTrackingMaxFramesAfterWhichATipHiddenByArmIsConsideredLost();
    int minDistanceToConsiderNewTipAsTheSameAsAnExistingOne =
        configurationReader.getTrackingMinDistanceToConsiderNewTipAsTheSameAsAnExistingOne();

    // If there is no existing tip, transform all the detected ones in the frame
    if (tips.size() == 0) {
        int tipIndex = 0;
        for (auto& detectedObject : detectionResult.detectedObjects) {
            if (DetectedObjectTypeHelper::isTip(detectedObject.objectType)) {
                Tip tip = makeTip(detectedObject, detectionResult.frameIndex, tipIndex);
                tipIndex++;
                tips.push_back(tip);
            }
        }
        return;
    }

    // Match the detected tips in the new frame with the existing tips
    vector<reference_wrapper<Rectangle>> wrappedTips;
    for (auto& tip : tips) {
        wrappedTips.push_back(tip);
    }
    auto detectedTips = extractObjectsOfTypes(
        detectionResult.detectedObjects, { DetectedObjectType::SMALL_TIP, DetectedObjectType::BIG_TIP });
    auto matchResults =
        matchEachTipFromTheCurrentFrameWithOneFromThePreviousFrame(wrappedTips, detectedTips);
    
    map<string, DetectedObject> matchedObjectByTipId;
    for (auto& matchResult : matchResults) {
        Tip& tip = (Tip&) matchResult.prevFrameObject;
        DetectedObject& detectedObject = (DetectedObject&) matchResult.currFrameObject;
        matchedObjectByTipId.insert(make_pair(tip.id, detectedObject));
    }
    
    // Find the tips that are hidden by an arm
    unordered_set<string> hiddenTipIds = findTipIdsHiddenByAnArm(tips, detectionResult);

    // Update the tips
    for (Tip& tip : tips) {
        // Check if the tip was matched to a detected object
        if (matchedObjectByTipId.find(tip.id) != matchedObjectByTipId.end()) {
            DetectedObject& detectedObject = matchedObjectByTipId[tip.id];

            // Update the tip type counter
            if (detectedObject.objectType == DetectedObjectType::SMALL_TIP) {
                tip.nbDetectionsAsSmallTip++;
            } else {
                tip.nbDetectionsAsBigTip++;
            }

            // Compute the new position and size of the tip
            tip.recentShapes.push_back(detectedObject);
            double avgX = 0;
            double avgY = 0;
            double avgWidth = 0;
            double avgHeight = 0;
            for (Rectangle& shape : tip.recentShapes) {
                avgX += shape.x;
                avgY += shape.y;
                avgWidth += shape.width;
                avgHeight += shape.height;
            }
            tip.x = round(avgX / tip.recentShapes.size());
            tip.y = round(avgY / tip.recentShapes.size());
            tip.width = round(avgWidth / tip.recentShapes.size());
            tip.height = round(avgHeight / tip.recentShapes.size());

            // Update the tip status
            tip.recentTrackingStatuses.push_back(TrackingStatus::DETECTED);

            continue;
        }

        // Check if the tip is hidden by an arm
        if (hiddenTipIds.find(tip.id) != hiddenTipIds.end()) {
            // Copy the same position and size
            Rectangle lastShape = tip.recentShapes.back();
            tip.recentShapes.push_back(lastShape);

            // Update the tip status
            tip.recentTrackingStatuses.push_back(TrackingStatus::HIDDEN_BY_ARM);

            continue;
        }

        // Check if the tip is lost
        bool tipLost = true;
        int nbStatusesToCheck = min(
            (int) tip.recentTrackingStatuses.size(), maxFramesAfterWhichATipIsConsideredLost);
        for (int s = nbStatusesToCheck - 1; s >= 0; s--) {
            auto& status = tip.recentTrackingStatuses[s];
            if (status == TrackingStatus::DETECTED || status == TrackingStatus::HIDDEN_BY_ARM) {
                tipLost = false;
                break;
            }
        }

        // Copy the same position and size
        Rectangle lastShape = tip.recentShapes.back();
        tip.recentShapes.push_back(lastShape);

        // Update the tip status
        tip.recentTrackingStatuses.push_back(tipLost ? TrackingStatus::LOST : TrackingStatus::NOT_DETECTED);
    }

    // Remove tips that are lost
    remove_erase_if(tips, [](const Tip& tip) {
        auto& status = tip.recentTrackingStatuses.back();
        return status == TrackingStatus::LOST;
    });

    // Prepare the newly detected tips
    unordered_set<DetectedObject, DetectedObject::Hasher> matchedDetectedTips;
    for (auto& matchResult : matchResults) {
        DetectedObject& detectedTip = (DetectedObject&) matchResult.currFrameObject;
        matchedDetectedTips.insert(detectedTip);
    }
    vector<reference_wrapper<DetectedObject>> newDetectedTips;
    for (Rectangle& detectedTipAsRectangle : detectedTips) {
        DetectedObject& detectedTip = (DetectedObject&) detectedTipAsRectangle;
        if (matchedDetectedTips.find(detectedTip) == matchedDetectedTips.end()) {
            newDetectedTips.push_back(detectedTip);
        }
    }

    // Before adding the newly detected tips, filter the ones that are too close to
    // existing tips in the same frame
    int minDistance = minDistanceToConsiderNewTipAsTheSameAsAnExistingOne;
    remove_erase_if(newDetectedTips, [this, tips, minDistance](const DetectedObject& newDetectedTip) {
        bool isTooClose = false;
        for (auto& tip : tips) {
            double matchingDistance = computeMatchingDistance(tip, newDetectedTip);
            if (matchingDistance <= minDistance) {
                isTooClose = true;
                break;
            }
        }
        return isTooClose;
    });

    // Add new tips
    for (int tipIndex = 0; tipIndex < newDetectedTips.size(); tipIndex++) {
        DetectedObject& newDetectedTip = newDetectedTips[tipIndex];
        Tip tip = makeTip(newDetectedTip, detectionResult.frameIndex, tipIndex);
        tips.push_back(tip);
    }
}

vector<reference_wrapper<Rectangle>> TipTrackerImpl::extractObjectsOfTypes(
    vector<DetectedObject>& detectedObjects, const vector<DetectedObjectType>& objectTypes) {
    vector<reference_wrapper<Rectangle>> filteredObjects;

    for (DetectedObject& detectedObject : detectedObjects) {
        if (find(objectTypes.begin(), objectTypes.end(), detectedObject.objectType) != objectTypes.end()) {
            filteredObjects.push_back(detectedObject);
        }
    }

    return filteredObjects;
}

vector<TipTrackerImpl::ObjectMatchResult> TipTrackerImpl::matchEachTipFromTheCurrentFrameWithOneFromThePreviousFrame(
    vector<reference_wrapper<Rectangle>>& prevFrameDetectedTips,
    vector<reference_wrapper<Rectangle>>& currFrameDetectedTips) {

    int maxMatchingDistance = configurationReader.getTrackingMaxTipMatchingDistanceInPixels();

    // Match each tip from the current frame with all tips from the previous frame
    set<TipTrackerImpl::ObjectMatchResult> matchResults; // Automatically sort by matching distance

    for (auto& currFrameTip : currFrameDetectedTips) {
        for (auto& prevFrameTip : prevFrameDetectedTips) {
            double matchingDistance = computeMatchingDistance(prevFrameTip, currFrameTip);

            if (matchingDistance <= maxMatchingDistance) {
                ObjectMatchResult matchResult = { prevFrameTip, currFrameTip, matchingDistance };
                matchResults.insert(matchResult);
            }
        }
    }

    // Filter the match results by making sure that each tip is used only once
    vector<TipTrackerImpl::ObjectMatchResult> filteredMatchResults;
    unordered_set<Rectangle, Rectangle::Hasher> alreadyMatchedTips;
    for (auto& matchResult : matchResults) {
        if (alreadyMatchedTips.find(matchResult.currFrameObject) == alreadyMatchedTips.end()
            && alreadyMatchedTips.find(matchResult.prevFrameObject) == alreadyMatchedTips.end()) {
            
            filteredMatchResults.push_back(matchResult);
            alreadyMatchedTips.insert(matchResult.currFrameObject);
            alreadyMatchedTips.insert(matchResult.prevFrameObject);
        }
    }

    return filteredMatchResults;
}

unordered_set<string> TipTrackerImpl::findTipIdsHiddenByAnArm(
    vector<Tip>& tips, FrameDetectionResult& detectionResult) {

    int minMatchingDistanceWithAnyObjectToConsiderTipNotHiddenByArm =
        configurationReader.getTrackingMinMatchingDistanceWithAnyObjectToConsiderTipNotHiddenByArm();

    auto detectedArms = extractObjectsOfTypes(detectionResult.detectedObjects, {DetectedObjectType::ARM});
    unordered_set<string> hiddenTipIds;
    if (!detectedArms.empty()) {
        for (Tip& tip : tips) {
            // Ignore tips that are lost of only detected once
            TrackingStatus status = tip.recentTrackingStatuses.back();
            if (status == TrackingStatus::LOST || status == TrackingStatus::DETECTED_ONCE) {
                continue;
            }

            // Detect if the tip is overlapping with an arm
            bool isOverlappingWithArm = false;
            for (Rectangle& arm : detectedArms) {
                if (arm.isOverlappingWith(tip)) {
                    isOverlappingWithArm = true;
                    break;
                }
            }
            if (!isOverlappingWithArm) {
                continue;
            }

            // Check that there is no detected object near the tip, so we can make sure that
            // the tip is indeed hidden
            bool hiddenByArm = true;
            for (auto& object : detectionResult.detectedObjects) {
                double matchingDistance = computeMatchingDistance(tip, object);
                if (matchingDistance <= minMatchingDistanceWithAnyObjectToConsiderTipNotHiddenByArm) {
                    hiddenByArm = false;
                    break;
                }
            }
            if (!hiddenByArm) {
                continue;
            }
            
            // Mark the tip as hidden by an arm
            hiddenTipIds.insert(tip.id);
        }
    }

    return hiddenTipIds;
}

Tip TipTrackerImpl::makeTip(DetectedObject& detectedObject, int frameIndex, int tipIndex) {
    int nbDetectionsToComputeAverageTipPositionAndSize =
        configurationReader.getTrackingNbDetectionsToComputeAverageTipPositionAndSize();
    int maxFramesAfterWhichATipIsConsideredLost =
        configurationReader.getTrackingMaxFramesAfterWhichATipIsConsideredLost();
    int maxFramesAfterWhichATipHiddenByArmIsConsideredLost =
        configurationReader.getTrackingMaxFramesAfterWhichATipHiddenByArmIsConsideredLost();
    int trackingStatusesSize =
        max(maxFramesAfterWhichATipIsConsideredLost, maxFramesAfterWhichATipHiddenByArmIsConsideredLost);

    string tipId = "T" + to_string(frameIndex) + "_" + to_string(tipIndex);

    circular_buffer<Rectangle> recentShapes(nbDetectionsToComputeAverageTipPositionAndSize);
    recentShapes.push_back(detectedObject);

    circular_buffer<TrackingStatus> recentTrackingStatuses(trackingStatusesSize);
    recentTrackingStatuses.push_back(TrackingStatus::DETECTED_ONCE);

    return Tip(
        tipId,
        recentShapes,
        recentTrackingStatuses,
        detectedObject.objectType == DetectedObjectType::BIG_TIP ? 1 : 0,
        detectedObject.objectType == DetectedObjectType::SMALL_TIP ? 1 : 0,
        detectedObject.x,
        detectedObject.y,
        detectedObject.width,
        detectedObject.height);
}

double TipTrackerImpl::computeMatchingDistance(const Rectangle& left, const Rectangle& right) {
    double matchingDistance = distance(right.x, right.y, left.x, left.y);
    matchingDistance += abs(right.width - left.width);
    matchingDistance += abs(right.height - left.height);
    return matchingDistance;
}

double TipTrackerImpl::distance(int x1, int y1, int x2, int y2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    return sqrt(pow(dx, 2.0) + pow(dy, 2.0));
}