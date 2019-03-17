package fr.marcsworld.chopstickstracker.services.impl

import fr.marcsworld.chopstickstracker.model.*
import fr.marcsworld.chopstickstracker.services.VideoDetectionService
import java.util.*
import java.util.concurrent.atomic.AtomicInteger
import java.util.stream.Collectors
import kotlin.collections.ArrayList

class VideoDetectionServiceImpl(
        private val configuration: Configuration
) : VideoDetectionService {

    override fun removeUnreliableDetectedObjects(frames: List<Frame>): List<Frame> {
        return frames.map { frame ->
            val reliableObjects = frame.objects.filter {
                val minConfidence = when {
                    it.objectType.isTip() -> configuration.minTipDetectionConfidence
                    it.objectType == DetectedObjectType.CHOPSTICK -> configuration.minChopstickDetectionConfidence
                    else -> configuration.minArmDetectionConfidence
                }
                it.confidence >= minConfidence
            }
            Frame(frame.index, reliableObjects, frame.imageX, frame.imageY)
        }
    }

    override fun compensateCameraMotion(frames: List<Frame>): List<Frame> {
        val compensatedFrames = ArrayList<Frame>()
        if (frames.isEmpty()) {
            return compensatedFrames
        }
        compensatedFrames.add(frames[0])

        val maxScore = configuration.maxTipMatchingScoreInPixels

        for (frameIndex in 1 until frames.size) {
            val prevFrameDetectedTips = findDetectedTips(frames[frameIndex - 1])
            val currFrameDetectedTips = findDetectedTips(frames[frameIndex])

            // Match current frame tips with the ones of the previous frame
            val detectedTipMatchResults = currFrameDetectedTips.stream()
                    .flatMap { currFrameDetectedTip ->
                        prevFrameDetectedTips.stream()
                                .map { prevFrameDetectedTip ->
                                    ObjectMatchResult(
                                            prevFrameDetectedTip,
                                            currFrameDetectedTip,
                                            computeObjectMatchingScore(prevFrameDetectedTip, currFrameDetectedTip))
                                }
                    }
                    .sorted(Comparator.comparing(ObjectMatchResult::score))
                    .filter { it.score <= maxScore }
                    .collect(Collectors.toList())

            // Choose match results by score classification and by avoiding multiple match results with the same tips
            val matchedDetectedObjects = HashSet<DetectedObject>()
            val reliableDetectedTipMatchResults = ArrayList<ObjectMatchResult>()
            for (detectedTipMatchResult in detectedTipMatchResults) {
                if (!matchedDetectedObjects.contains(detectedTipMatchResult.currFrameDetectedObject) &&
                        !matchedDetectedObjects.contains(detectedTipMatchResult.prevFrameDetectedObject)) {
                    reliableDetectedTipMatchResults.add(detectedTipMatchResult)
                    matchedDetectedObjects.add(detectedTipMatchResult.currFrameDetectedObject)
                    matchedDetectedObjects.add(detectedTipMatchResult.prevFrameDetectedObject)
                }
            }

            // Use the best match results to calculate the translation coordinates between this frame and the previous one
            val nbBestMatchResults = Math.min(reliableDetectedTipMatchResults.size, configuration.nbTipsToUseToDetectCameraMotion)
            val dx = reliableDetectedTipMatchResults.stream()
                    .limit(nbBestMatchResults.toLong())
                    .mapToInt { it.currFrameDetectedObject.x - it.prevFrameDetectedObject.x }
                    .average()
                    .orElse(0.0)
            val dy = reliableDetectedTipMatchResults.stream()
                    .limit(nbBestMatchResults.toLong())
                    .mapToInt { it.currFrameDetectedObject.y - it.prevFrameDetectedObject.y }
                    .average()
                    .orElse(0.0)

            // Recalculate the position of each detected object, the create a new frame
            val currFrame = frames[frameIndex]
            val prevCompensatedFrame = compensatedFrames[frameIndex - 1]

            val currFrameImageX = prevCompensatedFrame.imageX - dx
            val currFrameImageY = prevCompensatedFrame.imageY - dy

            val compensatedObjects = currFrame.objects.stream()
                    .map {
                        DetectedObject(
                                Math.round(it.x + currFrameImageX).toInt(),
                                Math.round(it.y + currFrameImageY).toInt(),
                                it.width,
                                it.height,
                                it.objectType,
                                it.confidence)
                    }
                    .collect(Collectors.toList())
            val compensatedFrame = Frame(
                    currFrame.index, compensatedObjects, currFrameImageX, currFrameImageY)
            compensatedFrames.add(compensatedFrame)
        }

        return compensatedFrames
    }

    override fun findAllTips(frames: List<Frame>): List<Tip> {
        val maxScore = configuration.maxTipMatchingScoreInPixels

        val tips = ArrayList(findTipsInFrame(frames[0]))

        for (frameIndex in 1 until frames.size) {
            println("    Detecting tips in frame $frameIndex / ${frames.size}...")
            val frameTips = findTipsInFrame(frames[frameIndex])

            // For each tip in this frame, try to find the corresponding one in the previous frame
            val tipMatchResults = frameTips.stream()
                    .flatMap { currentFrameTip ->
                        val detectedTip = currentFrameTip.detectionByFrameIndex[frameIndex]
                                ?: throw IllegalStateException("Unable to find tip in current frame.")

                        tips.stream()
                                .filter {
                                    // Only consider eligible tips in the previous frames
                                    it.detectionByFrameIndex.lastKey() + 1 >=
                                            frameIndex - configuration.nbFramesAfterWhichATipIsConsideredMissing
                                }
                                .map {
                                    val prevFrameDetectedTip = it.detectionByFrameIndex.lastEntry().value

                                    // Compute a score for each tip (smaller is better)
                                    val score = computeObjectMatchingScore(prevFrameDetectedTip, detectedTip)

                                    TipMatchResult(currentFrameTip, it, score)
                                }
                    }
                    .sorted(Comparator.comparing(TipMatchResult::score))
                    .collect(Collectors.toList())

            // Match the tips together
            val matchedTips = HashSet<Tip>()
            val reliableTipMatchResults = ArrayList<TipMatchResult>()
            for (tipMatchResult in tipMatchResults) {
                if (tipMatchResult.score > maxScore) {
                    break
                }
                if (!matchedTips.contains(tipMatchResult.currentFrameTip) && !matchedTips.contains(tipMatchResult.prevFrameTip)) {
                    reliableTipMatchResults.add(tipMatchResult)
                    matchedTips.add(tipMatchResult.currentFrameTip)
                    matchedTips.add(tipMatchResult.prevFrameTip)
                }
            }

            // Update the tips
            for (tipMatchResult in reliableTipMatchResults) {
                val detectedTip = tipMatchResult.currentFrameTip.detectionByFrameIndex[frameIndex]
                        ?: throw IllegalStateException("Unable to find tip in current frame.")

                tipMatchResult.prevFrameTip.detectionByFrameIndex[frameIndex] = detectedTip
            }

            // Add new tips
            for (frameTip in frameTips) {
                if (!matchedTips.contains(frameTip)) {
                    tips.add(frameTip)
                }
            }
        }

        return tips
    }

    private fun findTipsInFrame(frame: Frame): List<Tip> {
        val nextTipId = AtomicInteger()

        return findDetectedTips(frame).stream()
                .map {
                    val tipId = "T" + frame.index + "_" + nextTipId.incrementAndGet()
                    val detectionByFrameIndex = TreeMap<Int, DetectedObject>()
                    detectionByFrameIndex[frame.index] = it
                    Tip(tipId, detectionByFrameIndex)
                }
                .collect(Collectors.toList())
    }

    private fun findDetectedTips(frame: Frame): List<DetectedObject> {
        return frame.objects.stream()
                .filter { it.confidence > configuration.minTipDetectionConfidence }
                .filter { it.objectType.isTip() }
                .collect(Collectors.toList())
    }

    private fun computeObjectMatchingScore(prevObject: DetectedObject, currObject: DetectedObject): Double {
        val dx = (currObject.x - prevObject.x).toDouble()
        val dy = (currObject.y - prevObject.y).toDouble()
        var score = Math.abs(Math.sqrt(Math.pow(dx, 2.0) + Math.pow(dy, 2.0)))
        score += Math.abs(currObject.width - prevObject.width)
        score += Math.abs(currObject.height - prevObject.height)
        return score
    }

    private fun objectsOverlap(object1: DetectedObject, object2: DetectedObject): Boolean {
        return object1.x < object2.x + object2.width && object1.x + object1.width > object2.x &&
                object1.y < object2.y + object2.height && object1.y + object1.height > object2.y
    }

    private data class ObjectMatchResult(
            val prevFrameDetectedObject: DetectedObject,
            val currFrameDetectedObject: DetectedObject,
            val score: Double
    )

    private data class TipMatchResult(
            val currentFrameTip: Tip,
            val prevFrameTip: Tip,
            val score: Double
    )
}