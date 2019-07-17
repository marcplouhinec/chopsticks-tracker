#include <math.h>
#include <set>
#include <unordered_set>
#include "TipTrackerImpl.hpp"

using namespace model;
using namespace service;
using std::min;
using std::round;
using std::set;
using std::unordered_set;
using std::vector;

FrameOffset TipTrackerImpl::adjustObjectsToCompensateForCameraMotion(
    vector<DetectedObject>& prevFrameObjects, vector<DetectedObject>& currFrameObjects) {

    // Focus exclusively on the detected tips
    vector<DetectedObject> prevFrameTips = extractDetectedTips(prevFrameObjects);
    vector<DetectedObject> currFrameTips = extractDetectedTips(currFrameObjects);

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
    FrameOffset frameOffset(round(dx), round(dy));

    // Adjust all the detected objects of the current frame
    for (auto& currFrameObject : currFrameObjects) {
        currFrameObject.x -= frameOffset.dx;
        currFrameObject.y -= frameOffset.dy;
    }

    return frameOffset;
}

vector<DetectedObject> TipTrackerImpl::extractDetectedTips(vector<DetectedObject>& detectedObjects) {
    vector<DetectedObject> detectedTips;

    for (DetectedObject& detectedObject : detectedObjects) {
        if (detectedObject.objectType == DetectedObjectType::SMALL_TIP
            || detectedObject.objectType == DetectedObjectType::BIG_TIP) {
            detectedTips.push_back(detectedObject);
        }
    }

    return detectedTips;
}

vector<TipTrackerImpl::ObjectMatchResult> TipTrackerImpl::matchEachTipFromTheCurrentFrameWithOneFromThePreviousFrame(
    vector<DetectedObject>& prevFrameDetectedTips,
    vector<DetectedObject>& currFrameDetectedTips) {

    int maxMatchingDistance = configurationReader.getTrackingMaxTipMatchingDistanceInPixels();

    // Match each tip from the current frame with all tips from the previous frame
    set<TipTrackerImpl::ObjectMatchResult> matchResults; // Automatically sort by matching distance

    for (DetectedObject& currFrameTip : currFrameDetectedTips) {
        for (DetectedObject& prevFrameTip : prevFrameDetectedTips) {
            double matchingDistance = computeMatchingDistance(prevFrameTip, currFrameTip);

            if (matchingDistance <= maxMatchingDistance) {
                ObjectMatchResult matchResult = { prevFrameTip, currFrameTip, matchingDistance };
                matchResults.insert(matchResult);
            }
        }
    }

    // Filter the match results by making sure that each tip is used only once
    vector<TipTrackerImpl::ObjectMatchResult> filteredMatchResults;
    unordered_set<DetectedObject, DetectedObject::Hasher> alreadyMatchedTips;
    for (auto matchResult : matchResults) {
        if (alreadyMatchedTips.find(matchResult.currFrameObject) == alreadyMatchedTips.end()
            && alreadyMatchedTips.find(matchResult.prevFrameObject) == alreadyMatchedTips.end()) {
            
            filteredMatchResults.push_back(matchResult);
            alreadyMatchedTips.insert(matchResult.currFrameObject);
            alreadyMatchedTips.insert(matchResult.prevFrameObject);
        }
    }

    return filteredMatchResults;
}

double TipTrackerImpl::computeMatchingDistance(DetectedObject& left, DetectedObject& right) {
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