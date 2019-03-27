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
                    val recentShapes = ArrayList<EstimatedShape>(tip.shapes.takeLast(maxNbShapes))
                    recentShapes.add(currShape)

                    val avgX = recentShapes.stream().mapToInt { it.detectedObject?.x ?: it.x }.average().orElse(0.0)
                    val avgY = recentShapes.stream().mapToInt { it.detectedObject?.y ?: it.y }.average().orElse(0.0)
                    val avgWith = recentShapes.stream()
                            .mapToInt { it.detectedObject?.width ?: it.width }
                            .average().orElse(0.0)
                    val avgHeight = recentShapes.stream()
                            .mapToInt { it.detectedObject?.height ?: it.height }
                            .average().orElse(0.0)

                    val newShape = EstimatedShape(
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
                    val hiddenShape = EstimatedShape(
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
                    val lostShape = EstimatedShape(
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
                val undetectedShape = EstimatedShape(
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

    override fun findChopsticksByFrameIndex(frames: List<Frame>, tips: List<Tip>): List<List<Chopstick>> {
        val shapesAndTipsByFrameIndex: Map<Int, List<ShapeAndTip>> = tips.stream()
                .flatMap { tip -> tip.shapes.stream().map { shape -> ShapeAndTip(shape, tip) } }
                .filter { it.shape.status != EstimatedShapeStatus.LOST }
                .collect(Collectors.groupingBy { it.shape.frameIndex })

        return frames.map { frame ->

            val shapesAndTips = shapesAndTipsByFrameIndex[frame.index] ?: throw IllegalStateException()
            val detectedChopsticks = frame.objects.filter { it.objectType == DetectedObjectType.CHOPSTICK }

            // Find potential matching tips
            val results = shapesAndTips.stream()
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

            val processedTips = mutableSetOf<Tip>()
            val processedDetectedChopsticks = mutableSetOf<DetectedObject>()
            val chopsticks = mutableListOf<Chopstick>()
            for (result in results) {
                val tip1 = result.shapeAndTip1.tip
                val tip2 = result.shapeAndTip2.tip
                val detectedChopstick = result.detectedChopstick

                if (!processedTips.contains(tip1) && !processedTips.contains(tip2) &&
                        !processedDetectedChopsticks.contains(detectedChopstick)) {
                    processedTips.add(tip1)
                    processedTips.add(tip2)
                    processedDetectedChopsticks.add(detectedChopstick)

                    val chopstickTips = listOf(tip1, tip2).sortedBy { it.id }
                    chopsticks.add(Chopstick("${chopstickTips[0].id}_${chopstickTips[1].id}", chopstickTips))
                }
            }

            chopsticks
        }
    }

    private fun findTipsInFrame(frame: Frame): List<Tip> {
        val nextTipId = AtomicInteger()

        return findDetectedTips(frame).map {
            val tipId = "T" + frame.index + "_" + nextTipId.incrementAndGet()
            val shape = EstimatedShape(frame.index, EstimatedShapeStatus.DETECTED_ONCE, it, it.x, it.y, it.width, it.height)
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
            val shape: EstimatedShape,
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