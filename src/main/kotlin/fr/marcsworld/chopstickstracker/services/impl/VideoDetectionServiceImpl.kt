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
                                            computeMatchingScore(prevFrameDetectedTip, currFrameDetectedTip))
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
        val tips = mutableListOf<Tip>()

        for (frame in frames) {
            println("    Detecting tips in frame ${frame.index} / ${frames.size}...")

            // Detect the tips in the current frame
            val frameTips = findTipsInFrame(frame)

            if (frame.index < 1) {
                tips.addAll(frameTips)
                continue
            }

            // For each tip in this frame, try to find the corresponding one in the previous frame
            val tipMatchResults = frameTips.stream()
                    .flatMap { currentFrameTip ->
                        val currentShape = currentFrameTip.shapes.last()

                        tips.stream()
                                .filter { it.shapes.last().status != EstimatedShapeStatus.LOST }
                                .map {
                                    val prevShape = it.shapes.last()
                                    val score = computeMatchingScore(prevShape, currentShape)
                                    TipMatchResult(currentFrameTip, it, score)
                                }
                    }
                    .sorted(Comparator.comparing(TipMatchResult::score))
                    .collect(Collectors.toList())

            // Match the tips together
            val matchedTips = HashSet<Tip>()
            val matchResultByPrevTip = mutableMapOf<Tip, TipMatchResult>()
            for (tipMatchResult in tipMatchResults) {
                if (tipMatchResult.score > configuration.maxTipMatchingScoreInPixels) {
                    break
                }
                if (!matchedTips.contains(tipMatchResult.currentFrameTip) && !matchedTips.contains(tipMatchResult.prevFrameTip)) {
                    matchResultByPrevTip[tipMatchResult.prevFrameTip] = tipMatchResult
                    matchedTips.add(tipMatchResult.currentFrameTip)
                    matchedTips.add(tipMatchResult.prevFrameTip)
                }
            }

            // Find tips hidden by arms
            val detectedArms = frame.objects.filter { it.objectType == DetectedObjectType.ARM }
            val hiddenTips = if (detectedArms.isNotEmpty()) {
                // Find tips in the previous frame that are now under the arms, then
                // because the arm box includes areas that are not hidden by the arm,
                // ignore objects that overlap with other ones from current frame.
                val maxScore = configuration.maxMatchingScoreToConsiderTipNotHiddenByArm
                tips.stream()
                        .filter { it.shapes.last().status != EstimatedShapeStatus.LOST }
                        .filter { it.shapes.last().status != EstimatedShapeStatus.DETECTED_ONCE }
                        .filter { detectedArms.any { arm -> arm.isOverlappingWith(it.shapes.last()) } }
                        .filter { overlappingTip ->
                            !frame.objects.any { computeMatchingScore(overlappingTip.shapes.last(), it) <= maxScore }
                        }
                        .collect(Collectors.toSet())
            } else {
                emptySet<Tip>()
            }

            // Update the tips
            for (tip in tips) {
                // Check if the tip was matched to a detected object
                val matchResult = matchResultByPrevTip[tip]
                if (matchResult != null) {
                    val currShape = matchResult.currentFrameTip.shapes.last()

                    val maxNbShapes = configuration.nbShapesToConsiderForComputingAverageTipPositionAndSize
                    val recentShapes = ArrayList<EstimatedTipShape>(tip.shapes.takeLast(maxNbShapes))
                    recentShapes.add(currShape)

                    val avgX = recentShapes.stream().mapToInt { it.detectedObject?.x ?: it.x }.average().orElse(0.0)
                    val avgY = recentShapes.stream().mapToInt { it.detectedObject?.y ?: it.y }.average().orElse(0.0)
                    val avgWith = recentShapes.stream()
                            .mapToInt { it.detectedObject?.width ?: it.width }
                            .average().orElse(0.0)
                    val avgHeight = recentShapes.stream()
                            .mapToInt { it.detectedObject?.height ?: it.height }
                            .average().orElse(0.0)

                    val newShape = EstimatedTipShape(
                            currShape.frameIndex,
                            if (recentShapes.size == 1) EstimatedShapeStatus.DETECTED_ONCE else EstimatedShapeStatus.DETECTED,
                            currShape.detectedObject,
                            Math.round(avgX).toInt(),
                            Math.round(avgY).toInt(),
                            Math.round(avgWith).toInt(),
                            Math.round(avgHeight).toInt())
                    tip.shapes.add(newShape)

                    continue
                }

                // Check when the tip is hidden by an arm
                val lastTipShape = tip.shapes.last()
                if (hiddenTips.contains(tip)) {
                    val hiddenShape = EstimatedTipShape(
                            frame.index,
                            EstimatedShapeStatus.HIDDEN_BY_ARM,
                            null,
                            lastTipShape.x,
                            lastTipShape.y,
                            lastTipShape.width,
                            lastTipShape.height)
                    tip.shapes.add(hiddenShape)

                    continue
                }

                // Check if the tip is already lost
                if (lastTipShape.status == EstimatedShapeStatus.LOST) {
                    val lostShape = EstimatedTipShape(
                            frame.index,
                            EstimatedShapeStatus.LOST,
                            null,
                            lastTipShape.x,
                            lastTipShape.y,
                            lastTipShape.width,
                            lastTipShape.height)
                    tip.shapes.add(lostShape)

                    continue
                }

                // Check if the tip is lost
                val recentShapes = tip.shapes.takeLast(configuration.nbFramesAfterWhichATipIsConsideredMissing)
                val isNotLost = recentShapes.any { it.status == EstimatedShapeStatus.DETECTED || it.status == EstimatedShapeStatus.HIDDEN_BY_ARM }
                val undetectedShape = EstimatedTipShape(
                        frame.index,
                        if (isNotLost) EstimatedShapeStatus.NOT_DETECTED else EstimatedShapeStatus.LOST,
                        null,
                        lastTipShape.x,
                        lastTipShape.y,
                        lastTipShape.width,
                        lastTipShape.height)
                tip.shapes.add(undetectedShape)
            }

            // Before adding new tips, filter the ones that are too close to existing ones in the same frame
            val frameTipsToIgnore = frameTips.stream()
                    .filter { !matchedTips.contains(it) }
                    .flatMap { frameTip ->
                        tips.stream()
                                .filter { it.shapes.last().status != EstimatedShapeStatus.LOST }
                                .map {
                                    val prevShape = it.shapes.last()
                                    val score = computeMatchingScore(prevShape, frameTip.shapes.last())
                                    TipMatchResult(frameTip, it, score)
                                }
                    }
                    .filter { it.score <= configuration.maxScoreToConsiderNewTipAsTheSameAsAnExistingOne }
                    .map { it.currentFrameTip }
                    .collect(Collectors.toSet())

            // Add new tips
            for (frameTip in frameTips) {
                if (!matchedTips.contains(frameTip) && !frameTipsToIgnore.contains(frameTip)) {
                    tips.add(frameTip)
                }
            }
        }

        return tips
    }

    override fun findAllChopsticks(frames: List<Frame>, tips: List<Tip>): List<Chopstick> {
        val shapesAndTipsByFrameIndex: Map<Int, List<ShapeAndTip>> = tips.stream()
                .flatMap { tip -> tip.shapes.stream().map { shape -> ShapeAndTip(shape, tip) } }
                .filter { it.shape.status != EstimatedShapeStatus.LOST }
                .collect(Collectors.groupingBy { it.shape.frameIndex })

        val chopsticks = mutableListOf<Chopstick>()

        for (frame in frames) {
            println("    Detecting chopsticks in frame ${frame.index} / ${frames.size}...")

            // Try to match tips with each others by using detected chopsticks
            val shapesAndTips = shapesAndTipsByFrameIndex[frame.index] ?: throw IllegalStateException()
            val detectedChopsticks = frame.objects.filter { it.objectType == DetectedObjectType.CHOPSTICK }

            val matchResults = shapesAndTips.stream()
                    .flatMap { shapeAndTip ->
                        shapesAndTips.stream()
                                .filter { it.tip != shapeAndTip.tip }
                                .filter {
                                    val dist = distance(it.shape.x, it.shape.y, shapeAndTip.shape.x, shapeAndTip.shape.y)
                                    dist > 350 && dist < 550 // TODO why these values?
                                }
                                .flatMap { candidate ->
                                    val tipsBoundingBox = Rectangle.getBoundingBox(shapeAndTip.shape, candidate.shape)
                                    val boundingBoxArea = tipsBoundingBox.getArea()

                                    detectedChopsticks.stream()
                                            .filter { detectedChopstick -> tipsBoundingBox.isOverlappingWith(detectedChopstick) }
                                            .map { detectedChopstick ->
                                                val intersection = Rectangle.getIntersection(tipsBoundingBox, detectedChopstick)
                                                val intersectionArea = intersection.getArea()
                                                val unionArea = boundingBoxArea + detectedChopstick.getArea() - intersectionArea
                                                //val score = boundingBoxArea.toDouble() / intersectionArea.toDouble()
                                                val score = Math.abs(intersectionArea.toDouble() / unionArea.toDouble() - 1.0)

                                                ChopstickMatchResult(
                                                        shapeAndTip,
                                                        candidate,
                                                        detectedChopstick,
                                                        boundingBoxArea,
                                                        intersectionArea,
                                                        unionArea,
                                                        score)
                                            }
                                }
                    }
                    .sorted(Comparator.comparingDouble { it.score })
                    .filter { it.score <= 0.8 } // TODO why this value
                    .collect(Collectors.toList())

            // Find the best match results independently from other frames
            val processedTips = mutableSetOf<Tip>()
            val processedDetectedChopsticks = mutableSetOf<DetectedObject>()
            val bestMatchResultsInFrame = mutableListOf<ChopstickMatchResult>()
            for (result in matchResults) {
                val tip1 = result.shapeAndTip1.tip
                val tip2 = result.shapeAndTip2.tip
                val detectedChopstick = result.detectedChopstick

                if (!processedTips.contains(tip1) && !processedTips.contains(tip2) && !processedDetectedChopsticks.contains(detectedChopstick)) {
                    processedTips.add(tip1)
                    processedTips.add(tip2)
                    processedDetectedChopsticks.add(detectedChopstick)

                    bestMatchResultsInFrame.add(result)
                }
            }

            // Find the best match results by considering previous frame results
            val processedTips2 = mutableSetOf<Tip>()
            val processedDetectedChopsticks2 = mutableSetOf<DetectedObject>()
            val bestMatchResults = mutableListOf<ChopstickMatchResult>()
            for (result in matchResults) {
                val tip1 = result.shapeAndTip1.tip
                val tip2 = result.shapeAndTip2.tip
                val detectedChopstick = result.detectedChopstick

                // Ignore this result if any of its elements is conflicting with a selected match result
                if (processedTips2.contains(tip1) || processedTips2.contains(tip2) || processedDetectedChopsticks2.contains(detectedChopstick)) {
                    continue
                }

                // Find conflicts
                val conflictingChopsticks = chopsticks.stream()
                        .filter { it.shapes.last().status != EstimatedShapeStatus.LOST }
                        .filter { it.tip1 == tip1 || it.tip1 == tip2 || it.tip2 == tip1 || it.tip2 == tip2 }
                        .collect(Collectors.toList())

                val hasIdenticalChopsticks =
                        conflictingChopsticks.any { (it.tip1 == tip1 && it.tip2 == tip2) || (it.tip1 == tip2 && it.tip2 == tip1) }

                // Check if we need to keep the result or not
                if (conflictingChopsticks.isEmpty() || hasIdenticalChopsticks) {
                    processedTips2.add(tip1)
                    processedTips2.add(tip2)
                    processedDetectedChopsticks2.add(detectedChopstick)

                    bestMatchResults.add(result)
                }
            }

            // TODO conflicts should be preferred when they are present for several frames

            // Update the existing chopsticks
            val processedMatchResults = mutableSetOf<ChopstickMatchResult>()
            for (chopstick in chopsticks) {
                // Check if the chopstick is already lost
                val lastChopstickShape = chopstick.shapes.last()
                val shapeAndTip1 = shapesAndTips.find { it.tip == chopstick.tip1 }
                val shapeAndTip2 = shapesAndTips.find { it.tip == chopstick.tip2 }
                if (lastChopstickShape.status == EstimatedShapeStatus.LOST || shapeAndTip1 == null || shapeAndTip2 == null) {
                    val lostShape = EstimatedChopstickShape(
                            frame.index,
                            EstimatedShapeStatus.LOST,
                            null,
                            lastChopstickShape.tip1X,
                            lastChopstickShape.tip1Y,
                            lastChopstickShape.tip2X,
                            lastChopstickShape.tip2Y)
                    chopstick.shapes.add(lostShape)

                    continue
                }

                // Check if the chopstick was matched in this frame
                val tip1X = shapeAndTip1.shape.x + shapeAndTip1.shape.width / 2
                val tip1Y = shapeAndTip1.shape.y + shapeAndTip1.shape.height / 2
                val tip2X = shapeAndTip2.shape.x + shapeAndTip2.shape.width / 2
                val tip2Y = shapeAndTip2.shape.y + shapeAndTip2.shape.height / 2

                val identicalMatchResults = bestMatchResults.filter {
                    val tip1 = it.shapeAndTip1.tip
                    val tip2 = it.shapeAndTip2.tip
                    (tip1 == chopstick.tip1 && tip2 == chopstick.tip2) || (tip1 == chopstick.tip2 && tip2 == chopstick.tip1)
                }
                if (identicalMatchResults.isNotEmpty()) {
                    val matchResult = identicalMatchResults[0]
                    processedMatchResults.add(matchResult)

                    val shape = EstimatedChopstickShape(
                            frame.index,
                            EstimatedShapeStatus.DETECTED,
                            matchResult.detectedChopstick,
                            tip1X, tip1Y, tip2X, tip2Y)
                    chopstick.shapes.add(shape)
                    continue
                }

                // Check when the chopstick is hidden by an arm
                if (shapeAndTip1.shape.status == EstimatedShapeStatus.HIDDEN_BY_ARM ||
                        shapeAndTip2.shape.status == EstimatedShapeStatus.HIDDEN_BY_ARM) {
                    val hiddenShape = EstimatedChopstickShape(
                            frame.index,
                            EstimatedShapeStatus.HIDDEN_BY_ARM,
                            null,
                            tip1X, tip1Y, tip2X, tip2Y)
                    chopstick.shapes.add(hiddenShape)
                    continue
                }

                // Check if the chopstick is lost
                // TODO Do not consider a chopstick lost when the tips are still here and there is no conflict
                // TODO see frame 697
                val recentShapes = chopstick.shapes.takeLast(configuration.nbFramesAfterWhichATipIsConsideredMissing) // TODO create different conf
                val isNotLost = recentShapes.any { it.status == EstimatedShapeStatus.DETECTED || it.status == EstimatedShapeStatus.HIDDEN_BY_ARM }
                val undetectedShape = EstimatedChopstickShape(
                        frame.index,
                        if (isNotLost) EstimatedShapeStatus.NOT_DETECTED else EstimatedShapeStatus.LOST,
                        null,
                        tip1X, tip1Y, tip2X, tip2Y)
                chopstick.shapes.add(undetectedShape)
            }

            // Add other match results that would have been selected if they would not have conflicted with previous frames
            // TODO

            // Add new chopsticks
            for (bestMatchResult in bestMatchResults) {
                if (!processedMatchResults.contains(bestMatchResult)) {
                    val chopstickShapeAndTips = listOf(bestMatchResult.shapeAndTip1, bestMatchResult.shapeAndTip2).sortedBy { it.tip.id }
                    val tip1 = chopstickShapeAndTips[0].tip
                    val tip2 = chopstickShapeAndTips[1].tip

                    val tip1X = chopstickShapeAndTips[0].shape.x + chopstickShapeAndTips[0].shape.width / 2
                    val tip1Y = chopstickShapeAndTips[0].shape.y + chopstickShapeAndTips[0].shape.height / 2
                    val tip2X = chopstickShapeAndTips[1].shape.x + chopstickShapeAndTips[1].shape.width / 2
                    val tip2Y = chopstickShapeAndTips[1].shape.y + chopstickShapeAndTips[1].shape.height / 2

                    val shapes = mutableListOf<EstimatedChopstickShape>()
                    val shape = EstimatedChopstickShape(
                            frame.index,
                            EstimatedShapeStatus.DETECTED_ONCE,
                            bestMatchResult.detectedChopstick,
                            tip1X, tip1Y, tip2X, tip2Y)
                    shapes.add(shape)

                    val chopstick = Chopstick("C_${tip1.id}_${tip2.id}", tip1, tip2, shapes)
                    chopsticks.add(chopstick)
                }
            }
        }

        return chopsticks
    }

    private fun findTipsInFrame(frame: Frame): List<Tip> {
        val nextTipId = AtomicInteger()

        return findDetectedTips(frame).map {
            val tipId = "T" + frame.index + "_" + nextTipId.incrementAndGet()
            val shape = EstimatedTipShape(frame.index, EstimatedShapeStatus.DETECTED_ONCE, it, it.x, it.y, it.width, it.height)
            Tip(tipId, mutableListOf(shape))
        }
    }

    private fun findDetectedTips(frame: Frame): List<DetectedObject> {
        return frame.objects.stream()
                .filter { it.confidence > configuration.minTipDetectionConfidence }
                .filter { it.objectType.isTip() }
                .collect(Collectors.toList())
    }

    private fun computeMatchingScore(prevObject: Rectangle, currObject: Rectangle): Double {
        var score = distance(currObject.x, currObject.y, prevObject.x, prevObject.y)
        score += Math.abs(currObject.width - prevObject.width)
        score += Math.abs(currObject.height - prevObject.height)
        return score
    }

    private fun distance(x1: Int, y1: Int, x2: Int, y2: Int): Double {
        val dx = x1 - x2
        val dy = y1 - y2
        return Math.sqrt(Math.pow(dx.toDouble(), 2.0) + Math.pow(dy.toDouble(), 2.0))
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

    private data class ShapeAndTip(
            val shape: EstimatedTipShape,
            val tip: Tip
    )

    private data class ChopstickMatchResult(
            val shapeAndTip1: ShapeAndTip,
            val shapeAndTip2: ShapeAndTip,
            val detectedChopstick: DetectedObject,
            val boundingBoxArea: Int,
            val intersectionArea: Int,
            val unionArea: Int,
            val score: Double)
}