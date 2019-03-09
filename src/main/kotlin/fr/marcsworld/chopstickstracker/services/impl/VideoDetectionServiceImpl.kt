package fr.marcsworld.chopstickstracker.services.impl

import fr.marcsworld.chopstickstracker.model.*
import fr.marcsworld.chopstickstracker.services.VideoDetectionService
import java.util.*
import java.util.concurrent.atomic.AtomicInteger
import java.util.stream.Collectors

class VideoDetectionServiceImpl(
        private val configuration: Configuration
) : VideoDetectionService {

    override fun findAllTips(frames: List<Frame>): List<Tip> {
        val frameWidth = configuration.frameWidth.toDouble()
        val frameHeight = configuration.frameHeight.toDouble()
        val frameDiagonal = Math.sqrt(Math.pow(frameWidth, 2.0) + Math.pow(frameHeight, 2.0))

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
                                    val dx = (detectedTip.x - prevFrameDetectedTip.x).toDouble()
                                    val dy = (detectedTip.y - prevFrameDetectedTip.y).toDouble()
                                    var score = Math.abs(Math.sqrt(Math.pow(dx, 2.0) + Math.pow(dy, 2.0)))
                                    score += Math.abs(detectedTip.width - prevFrameDetectedTip.width)
                                    score += Math.abs(detectedTip.height - prevFrameDetectedTip.height)

                                    TipMatchResult(currentFrameTip, it, score)
                                }
                    }
                    .sorted(Comparator.comparing(TipMatchResult::score))
                    .collect(Collectors.toList())

            // Match the tips together
            val matchedTips = HashSet<Tip>()
            val reliableTipMatchResults = ArrayList<TipMatchResult>()
            for (tipMatchResult in tipMatchResults) {
                if (tipMatchResult.score / frameDiagonal > configuration.maxTipMatchingScore) {
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

        return frame.objects.stream()
                .filter { it.confidence > configuration.minTipDetectionConfidence }
                .filter { it.objectType == DetectedObjectType.BIG_TIP || it.objectType == DetectedObjectType.SMALL_TIP }
                .map {
                    val tipId = "T" + frame.index + "_" + nextTipId.incrementAndGet()
                    val detectionByFrameIndex = TreeMap<Int, DetectedObject>()
                    detectionByFrameIndex[frame.index] = it
                    Tip(tipId, detectionByFrameIndex)
                }
                .collect(Collectors.toList())
    }

    private data class TipMatchResult(
            val currentFrameTip: Tip,
            val prevFrameTip: Tip,
            val score: Double
    )
}