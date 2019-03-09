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
            val alternativeTipMatchResults = ArrayList<TipMatchResult>()
            for (tipMatchResult in tipMatchResults) {
                if (tipMatchResult.score / frameDiagonal > configuration.maxTipMatchingScore) {
                    break
                }
                if (!matchedTips.contains(tipMatchResult.currentFrameTip) && !matchedTips.contains(tipMatchResult.prevFrameTip)) {
                    reliableTipMatchResults.add(tipMatchResult)
                    matchedTips.add(tipMatchResult.currentFrameTip)
                    matchedTips.add(tipMatchResult.prevFrameTip)
                } else {
                    // Acceptable alternatives where the score is still good enough
                    alternativeTipMatchResults.add(tipMatchResult)
                }
            }

            // Replace initially chosen match results by preferring alternative with older tips
            val sortedAlternativeTipMatchResults = alternativeTipMatchResults.stream()
                    .sorted(Comparator
                            .comparingInt<TipMatchResult> { it.prevFrameTip.getDateOfBirth() }
                            .thenComparingDouble { it.score })
                    .collect(Collectors.toList())
            val chosenAlternativeTipMatchResults = ArrayList<TipMatchResult>()
            for (alternativeTipMatchResult in sortedAlternativeTipMatchResults) {
                // Find the selected match result
                val reliableTipMatchResult = reliableTipMatchResults.stream()
                        .filter { it.currentFrameTip == alternativeTipMatchResult.currentFrameTip }
                        .findAny()
                        .orElse(null)

                // Check if the alternative is not better
                // TODO Another solution would be to consider that a missing tip is still here, just
                // TODO not detected for a while. See frame 165 where T0_5 become T0_8
                if (reliableTipMatchResult != null &&
                        alternativeTipMatchResult.prevFrameTip.getDateOfBirth() < reliableTipMatchResult.prevFrameTip.getDateOfBirth()) {
                    // TODO Should we put a limit on the score difference with the alternative?
                    // Check if the previous frame tip is not already inside the reliableTipMatchResults
                    val problematicMatchResult = reliableTipMatchResults.stream()
                            .filter { it.prevFrameTip == alternativeTipMatchResult.prevFrameTip }
                            .findAny()
                            .orElse(null)

                    if (problematicMatchResult == null) {
                        reliableTipMatchResults.remove(reliableTipMatchResult)
                        chosenAlternativeTipMatchResults.add(alternativeTipMatchResult)
                    } else {
                        // Check if we have an alternative that would allow us to keep this alternative
                        val prevFrameTipInUse = reliableTipMatchResults.stream()
                                .map { it.prevFrameTip }
                                .collect(Collectors.toSet())
                        val solutionAlternativeMatchResult = sortedAlternativeTipMatchResults.stream()
                                .filter { !chosenAlternativeTipMatchResults.contains(it) }
                                .filter { it.currentFrameTip == problematicMatchResult.currentFrameTip }
                                .filter { it.prevFrameTip.getDateOfBirth() <= alternativeTipMatchResult.prevFrameTip.getDateOfBirth() }
                                .filter { !prevFrameTipInUse.contains(it.prevFrameTip) } // Avoid new problems
                                .findFirst()
                                .orElse(null)

                        if (solutionAlternativeMatchResult != null) {
                            // Check if this solution would not be worst
                            val actualDateOfBirthSum = reliableTipMatchResult.prevFrameTip.getDateOfBirth() +
                                    problematicMatchResult.prevFrameTip.getDateOfBirth()
                            val solutionDateOfBirthSum = alternativeTipMatchResult.prevFrameTip.getDateOfBirth() +
                                    solutionAlternativeMatchResult.prevFrameTip.getDateOfBirth()

                            val actualScore = reliableTipMatchResult.score + problematicMatchResult.score
                            val solutionScore = alternativeTipMatchResult.score + solutionAlternativeMatchResult.score

                            if (solutionDateOfBirthSum < actualDateOfBirthSum ||
                                    (solutionDateOfBirthSum == actualDateOfBirthSum && solutionScore < actualScore)) {
                                // Choose this solution
                                reliableTipMatchResults.remove(reliableTipMatchResult)
                                chosenAlternativeTipMatchResults.add(alternativeTipMatchResult)

                                reliableTipMatchResults.remove(problematicMatchResult)
                                chosenAlternativeTipMatchResults.add(solutionAlternativeMatchResult)
                            }
                        }
                    }
                }
            }
            reliableTipMatchResults.addAll(chosenAlternativeTipMatchResults)

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