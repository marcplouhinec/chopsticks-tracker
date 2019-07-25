#include <algorithm>
#include <math.h>
#include <map>
#include <set>
#include "TipTrackerImpl.hpp"

using namespace model;
using namespace service;
using std::find;
using std::list;
using std::make_pair;
using std::map;
using std::max;
using std::min;
using std::pair;
using std::reference_wrapper;
using std::set;
using std::string;
using std::to_string;
using std::unordered_set;
using std::vector;
using boost::circular_buffer;

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
    list<Tip>& tips,
    FrameDetectionResult& detectionResult,
    FrameOffset frameOffset,
    FrameOffset accumulatedFrameOffset) {

    // Load configuration
    int nbDetectionsToComputeAverageTipPositionAndSize =
        configurationReader.getTrackingNbDetectionsToComputeAverageTipPositionAndSize();
    
    // Extract the tips and translate them according to the frame offset
    auto untranslatedDetectedTips = extractObjectsOfTypes(
        detectionResult.detectedObjects, { DetectedObjectType::SMALL_TIP, DetectedObjectType::BIG_TIP });
    vector<DetectedObject> detectedTips = copyAndTranslateDetectedObjects(
        untranslatedDetectedTips, -accumulatedFrameOffset.dx, -accumulatedFrameOffset.dy);

    // If there is no existing tip, transform all the detected ones in the frame
    if (tips.size() == 0) {
        int tipIndex = 0;
        for (auto& detectedTip : detectedTips) {
            Tip tip = makeTip(detectedTip, detectionResult.frameIndex, tipIndex);
            tipIndex++;
            tips.push_back(tip);
        }
        return;
    }

    // Match the detected tips in the new frame with the existing tips
    vector<reference_wrapper<Rectangle>> wrappedTips;
    for (auto& tip : tips) {
        wrappedTips.push_back(tip);
    }
    vector<reference_wrapper<Rectangle>> wrappedDetectedTips;
    for (auto& detectedTip : detectedTips) {
        wrappedDetectedTips.push_back(detectedTip);
    }
    auto matchResults =
        matchEachTipFromTheCurrentFrameWithOneFromThePreviousFrame(wrappedTips, wrappedDetectedTips);
    
    map<string, DetectedObject> matchedObjectByTipId;
    for (auto& matchResult : matchResults) {
        Tip& tip = (Tip&) matchResult.prevFrameObject;
        DetectedObject& detectedObject = (DetectedObject&) matchResult.currFrameObject;
        matchedObjectByTipId.insert(make_pair(tip.id, detectedObject));
    }
    
    // Find the tips that are hidden by an arm
    unordered_set<string> hiddenTipIds = findTipIdsHiddenByAnArm(
        tips, detectionResult, accumulatedFrameOffset);

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
            tip.x = avgX / tip.recentShapes.size();
            tip.y = avgY / tip.recentShapes.size();
            tip.width = avgWidth / tip.recentShapes.size();
            tip.height = avgHeight / tip.recentShapes.size();

            // Update the tip status
            tip.recentTrackingStatuses.push_back(TrackingStatus::DETECTED);

            continue;
        }

        // Check if the tip is hidden by an arm
        if (hiddenTipIds.find(tip.id) != hiddenTipIds.end()) {
            // Copy the same position and size
            Rectangle lastShape = tip.recentShapes.back();
            lastShape.x = lastShape.x + frameOffset.dx;
            lastShape.y = lastShape.y + frameOffset.dy;
            tip.recentShapes.push_back(lastShape);
            tip.x = lastShape.x;
            tip.y = lastShape.y;

            // Update the tip status
            tip.recentTrackingStatuses.push_back(TrackingStatus::HIDDEN_BY_ARM);

            continue;
        }

        // Check if the tip is lost
        bool tipLost = true;
        for (TrackingStatus status : tip.recentTrackingStatuses) {
            if (status == TrackingStatus::DETECTED || status == TrackingStatus::HIDDEN_BY_ARM) {
                tipLost = false;
                break;
            }
        }

        // Copy the same position and size
        Rectangle lastShape = tip.recentShapes.back();
        lastShape.x = lastShape.x + frameOffset.dx;
        lastShape.y = lastShape.y + frameOffset.dy;
        tip.recentShapes.push_back(lastShape);
        tip.x = lastShape.x;
        tip.y = lastShape.y;

        // Update the tip status
        tip.recentTrackingStatuses.push_back(tipLost ? TrackingStatus::LOST : TrackingStatus::NOT_DETECTED);
    }

    // Remove tips that are lost
    tips.remove_if([](const Tip& tip) {
        auto& status = tip.recentTrackingStatuses.back();
        return status == TrackingStatus::LOST;
    });

    // Prepare the newly detected tips
    unordered_set<DetectedObject, DetectedObject::Hasher> matchedDetectedTips;
    for (auto& matchResult : matchResults) {
        DetectedObject& detectedTip = (DetectedObject&) matchResult.currFrameObject;
        matchedDetectedTips.insert(detectedTip);
    }
    list<reference_wrapper<DetectedObject>> newDetectedTips;
    for (Rectangle& detectedTipAsRectangle : detectedTips) {
        DetectedObject& detectedTip = (DetectedObject&) detectedTipAsRectangle;
        if (matchedDetectedTips.find(detectedTip) == matchedDetectedTips.end()) {
            newDetectedTips.push_back(detectedTip);
        }
    }

    // Before adding the newly detected tips, filter the ones that are too close to
    // existing tips in the same frame
    newDetectedTips.remove_if([this, tips](const DetectedObject& newDetectedTip) {
        return isDetectedTipTooCloseToExistingTips(newDetectedTip, tips);
    });

    // Add new tips
    int tipIndex = 0;
    for (DetectedObject& newDetectedTip : newDetectedTips) {
        Tip tip = makeTip(newDetectedTip, detectionResult.frameIndex, tipIndex);
        tips.push_back(tip);
        tipIndex++;
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

vector<DetectedObject> TipTrackerImpl::copyAndTranslateDetectedObjects(
    const vector<reference_wrapper<Rectangle>>& detectedObjects, const double dx, const double dy) {

    vector<DetectedObject> translatedObjects;

    for (const Rectangle& detectedRectangle : detectedObjects) {
        DetectedObject& detectedObject = (DetectedObject&) detectedRectangle;
        translatedObjects.push_back(detectedObject.copyAndTranslate(dx, dy));
    }

    return translatedObjects;
}

vector<TipTrackerImpl::ObjectMatchResult> TipTrackerImpl::matchEachTipFromTheCurrentFrameWithOneFromThePreviousFrame(
    vector<reference_wrapper<Rectangle>>& prevFrameDetectedTips,
    vector<reference_wrapper<Rectangle>>& currFrameDetectedTips) {

    int maxMatchingDistance = configurationReader.getTrackingMaxTipMatchingDistanceInPixels();

    // Match each tip from the current frame with all tips from the previous frame
    struct MatchingDistanceComparator {
        bool operator() (const ObjectMatchResult& r1, const ObjectMatchResult& r2) const {
            if (r1.matchingDistance != r2.matchingDistance) {
                return r1.matchingDistance < r2.matchingDistance;
            }

            // Avoid different ObjectMatchResults with the same matchingDistance to be considered as equal
            if (r1.prevFrameObject != r2.prevFrameObject) {
                return r1.prevFrameObject < r2.prevFrameObject;
            }
            return r1.currFrameObject < r2.currFrameObject;
        }
    };
    set<TipTrackerImpl::ObjectMatchResult, MatchingDistanceComparator> matchResults;
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
    list<Tip>& tips, FrameDetectionResult& detectionResult, FrameOffset& accumulatedFrameOffset) {

    int minMatchingDistanceWithAnyObjectToConsiderTipNotHiddenByArm =
        configurationReader.getTrackingMinMatchingDistanceWithAnyObjectToConsiderTipNotHiddenByArm();

    unordered_set<string> hiddenTipIds;

    auto untranslatedetectedArms = extractObjectsOfTypes(
        detectionResult.detectedObjects, {DetectedObjectType::ARM});
    if (untranslatedetectedArms.empty()) {
        return hiddenTipIds;
    }
    vector<DetectedObject> detectedArms = copyAndTranslateDetectedObjects(
        untranslatedetectedArms, -accumulatedFrameOffset.dx, -accumulatedFrameOffset.dy);
    
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

    return hiddenTipIds;
}

bool TipTrackerImpl::isDetectedTipTooCloseToExistingTips(const DetectedObject& detectedTip, const list<Tip>& tips) {
    int minDistance = configurationReader.getTrackingMinDistanceToConsiderNewTipAsTheSameAsAnExistingOne();

    for (auto& tip : tips) {
        double matchingDistance = computeMatchingDistance(tip, detectedTip);
        if (matchingDistance <= minDistance) {
            return true;
        }
    }

    return false;
}

Tip TipTrackerImpl::makeTip(DetectedObject& detectedObject, int frameIndex, int tipIndex) {
    int nbDetectionsToComputeAverageTipPositionAndSize =
        configurationReader.getTrackingNbDetectionsToComputeAverageTipPositionAndSize();
    int maxFramesAfterWhichATipIsConsideredLost =
        configurationReader.getTrackingMaxFramesAfterWhichATipIsConsideredLost();

    string tipId = "T" + to_string(frameIndex) + "_" + to_string(tipIndex);

    circular_buffer<Rectangle> recentShapes(nbDetectionsToComputeAverageTipPositionAndSize);
    recentShapes.push_back(detectedObject);

    circular_buffer<TrackingStatus> recentTrackingStatuses(maxFramesAfterWhichATipIsConsideredLost);
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
    double matchingDistance = Rectangle::distanceBetweenTopLeftPoints(right, left);
    matchingDistance += abs(right.width - left.width);
    matchingDistance += abs(right.height - left.height);
    return matchingDistance;
}