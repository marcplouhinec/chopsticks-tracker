package fr.marcsworld.chopstickstracker.services.rendering.impl

import fr.marcsworld.chopstickstracker.model.*
import fr.marcsworld.chopstickstracker.model.detection.DetectedObjectType
import fr.marcsworld.chopstickstracker.model.detection.FrameDetectionResult
import fr.marcsworld.chopstickstracker.services.rendering.VisualizationService
import fr.marcsworld.chopstickstracker.services.rendering.writer.FrameImageWriter
import org.opencv.core.*
import org.opencv.imgproc.Imgproc
import org.slf4j.LoggerFactory
import org.springframework.beans.factory.annotation.Value
import org.springframework.stereotype.Service
import java.util.stream.Collectors

/**
 * Default implementation of [VisualizationService].
 *
 * @author Marc Plouhinec
 */
@Service
class VisualizationServiceImpl(

        @Value("\${tracking.minTipDetectionConfidence}")
        private val minTipDetectionConfidence: Double = 0.9

) : VisualizationService {

    companion object {
        private val LOGGER = LoggerFactory.getLogger(VisualizationServiceImpl::class.java)
    }

    override fun renderTips(
            frameWidth: Int,
            frameHeight: Int,
            frames: List<Frame>,
            tips: List<Tip>,
            frameImageWriter: FrameImageWriter,
            detectedChopstickVisible: Boolean,
            chopsticks: List<Chopstick>,
            alternativeChopsticksVisible: Boolean,
            frameDetectionResults: Iterable<FrameDetectionResult>) {
        // Preparations
        val (firstFrameImageX, firstFrameImageY, outputWidth, outputHeight) = computeNewFrameDimension(
                frameWidth, frameHeight, frames)
        frameImageWriter.initFrameSize(outputWidth, outputHeight)

        val shapesWithTipsByFrameIndex: Map<Int, List<Pair<EstimatedTipShape, Tip>>> = tips.stream()
                .flatMap { tip ->
                    tip.shapes.stream()
                            .filter { it.status != EstimatedShapeStatus.LOST }
                            .map { Pair(it, tip) }
                }
                .collect(Collectors.groupingBy { it.first.frameIndex })

        val shapesWithChopsticksByFrameIndex: Map<Int, List<Pair<EstimatedChopstickShape, Chopstick>>> = chopsticks.stream()
                .flatMap { chopstick ->
                    chopstick.shapes.stream()
                            .filter { it.status != EstimatedShapeStatus.LOST }
                            .map { Pair(it, chopstick) }
                }
                .collect(Collectors.groupingBy { it.first.frameIndex })

        val cyanColor = Scalar(255.0, 255.0, 0.0)
        val greenColor = Scalar(0.0, 255.0, 0.0)
        val orangeColor = Scalar(0.0, 200.0, 255.0)
        val magentaColor = Scalar(255.0, 0.0, 255.0)
        val whiteColor = Scalar(255.0, 255.0, 255.0)

        // Draw the tips on each frame
        for (frameDetectionResult in frameDetectionResults) {
            val frame = frames[frameDetectionResult.frameIndex]
            LOGGER.info("Rendering tips in frame {} / {}...", frame.index, frames.size)

            val frameImage = frameDetectionResult.frameImageProvider()
            val outputImage = Mat(outputHeight, outputWidth, CvType.CV_8UC3, Scalar(0.0, 0.0, 0.0))

            val frameImageX = Math.round(firstFrameImageX + frame.imageX).toInt()
            val frameImageY = Math.round(firstFrameImageY + frame.imageY).toInt()
            val roi = Rect(frameImageX, frameImageY, frameImage.width(), frameImage.height())
            frameImage.copyTo(Mat(outputImage, roi))

            if (detectedChopstickVisible) {
                for (detectedChopstick in frame.objects) {
                    if (detectedChopstick.objectType == DetectedObjectType.CHOPSTICK) {
                        val x = Math.round(firstFrameImageX + detectedChopstick.x).toInt()
                        val y = Math.round(firstFrameImageY + detectedChopstick.y).toInt()

                        Imgproc.rectangle(
                                outputImage,
                                Rect(x, y, detectedChopstick.width, detectedChopstick.height),
                                cyanColor)
                    }
                }
            }

            val shapesWithTips = shapesWithTipsByFrameIndex[frame.index] ?: listOf()

            for (shapeWithTip in shapesWithTips) {
                val shape = shapeWithTip.first
                val tip = shapeWithTip.second

                val color = when {
                    shape.status == EstimatedShapeStatus.DETECTED_ONCE -> greenColor
                    shape.status == EstimatedShapeStatus.NOT_DETECTED -> orangeColor
                    shape.status == EstimatedShapeStatus.HIDDEN_BY_ARM -> magentaColor
                    else -> whiteColor
                }

                val x = Math.round(firstFrameImageX + shape.x).toInt()
                val y = Math.round(firstFrameImageY + shape.y).toInt()
                Imgproc.rectangle(outputImage, Rect(x, y, shape.width, shape.height), color)
                Imgproc.putText(outputImage, tip.id, Point(x.toDouble(), y + 16.0),
                        Imgproc.FONT_HERSHEY_SIMPLEX, 0.5, color)
            }

            val shapesWithChopsticks = shapesWithChopsticksByFrameIndex[frame.index] ?: listOf()
            for (shapeWithChopstick in shapesWithChopsticks) {
                val shape = shapeWithChopstick.first

                if (!shape.isRejectedBecauseOfConflict || (shape.isRejectedBecauseOfConflict && alternativeChopsticksVisible)) {
                    val color = when {
                        shape.status == EstimatedShapeStatus.DETECTED_ONCE -> greenColor
                        shape.status == EstimatedShapeStatus.NOT_DETECTED -> orangeColor
                        shape.status == EstimatedShapeStatus.HIDDEN_BY_ARM -> magentaColor
                        else -> whiteColor
                    }
                    val thickness = if (shape.isRejectedBecauseOfConflict) 1 else 2

                    val x1 = Math.round(firstFrameImageX + shape.tip1X).toDouble()
                    val y1 = Math.round(firstFrameImageY + shape.tip1Y).toDouble()
                    val x2 = Math.round(firstFrameImageX + shape.tip2X).toDouble()
                    val y2 = Math.round(firstFrameImageY + shape.tip2Y).toDouble()
                    Imgproc.line(outputImage, Point(x1, y1), Point(x2, y2), color, thickness)
                }
            }

            frameImageWriter.write(frameDetectionResult.frameIndex, outputImage)
        }
    }

    override fun renderCurrentAndPastTipDetections(
            frameWidth: Int,
            frameHeight: Int,
            frames: List<Frame>,
            maxFramesInPast: Int,
            frameImageWriter: FrameImageWriter,
            armVisible: Boolean,
            frameDetectionResults: Iterable<FrameDetectionResult>) {
        // Preparations
        val yellowColor = Scalar(0.0, 255.0, 255.0)
        val (firstFrameImageX, firstFrameImageY, outputWidth, outputHeight) = computeNewFrameDimension(
                frameWidth, frameHeight, frames)
        frameImageWriter.initFrameSize(outputWidth, outputHeight)

        // Draw the current and past tip detections on each frame
        for (frameDetectionResult in frameDetectionResults) {
            val frame = frames[frameDetectionResult.frameIndex]
            LOGGER.info("Rendering frame {} / {}...", frame.index, frames.size)

            val frameImage = frameDetectionResult.frameImageProvider()
            val outputImage = Mat(outputHeight, outputWidth, CvType.CV_8UC3, Scalar(0.0, 0.0, 0.0))

            val frameImageX = Math.round(firstFrameImageX + frame.imageX).toInt()
            val frameImageY = Math.round(firstFrameImageY + frame.imageY).toInt()
            val roi = Rect(frameImageX, frameImageY, frameImage.width(), frameImage.height())
            frameImage.copyTo(Mat(outputImage, roi))

            for (pastIndex in maxFramesInPast downTo 0) {
                val frameIndex = frame.index - pastIndex
                if (frameIndex >= 0) {
                    //val alpha = (maxFramesInPast - pastIndex) * (1.0 / maxFramesInPast)
                    val color = Scalar(255.0, 255.0, 255.0) //, Math.round(alpha).toDouble())

                    val detectedTips = frames[frameIndex].objects.stream()
                            .filter { it.objectType.isTip() }
                            .filter { it.confidence >= minTipDetectionConfidence }
                            .collect(Collectors.toList())

                    for (detectedTip in detectedTips) {
                        val x = Math.round(firstFrameImageX + detectedTip.x).toInt()
                        val y = Math.round(firstFrameImageY + detectedTip.y).toInt()
                        Imgproc.rectangle(outputImage, Rect(x, y, detectedTip.width, detectedTip.height), color)
                    }
                }
            }

            if (armVisible) {
                for (detectedArm in frame.objects) {
                    if (detectedArm.objectType == DetectedObjectType.ARM) {
                        val x = Math.round(firstFrameImageX + detectedArm.x).toInt()
                        val y = Math.round(firstFrameImageY + detectedArm.y).toInt()
                        Imgproc.rectangle(
                                outputImage,
                                Rect(x, y, detectedArm.width, detectedArm.height),
                                yellowColor)
                    }
                }
            }

            frameImageWriter.write(frameDetectionResult.frameIndex, outputImage)
        }
    }

    override fun renderDetectedObjects(
            frameDetectionResults: Iterable<FrameDetectionResult>,
            frameImageWriter: FrameImageWriter) {
        // Preparations
        val yellowColor = Scalar(0.0, 255.0, 255.0)
        val greenColor = Scalar(0.0, 255.0, 0.0)
        val orangeColor = Scalar(0.0, 200.0, 255.0)
        val magentaColor = Scalar(255.0, 0.0, 255.0)

        var frameImageWriterInitialized = false

        // Draw the current and past tip detections on each frame
        for (frameDetectionResult in frameDetectionResults) {
            LOGGER.info("Rendering frame {}...", frameDetectionResult.frameIndex)

            val frameImage = frameDetectionResult.frameImageProvider()

            if (!frameImageWriterInitialized) {
                val size = frameImage.size()
                frameImageWriter.initFrameSize(size.width.toInt(), size.height.toInt())
                frameImageWriterInitialized = true
            }

            for (detectedObject in frameDetectionResult.detectedObjects) {
                val color = when (detectedObject.objectType) {
                    DetectedObjectType.CHOPSTICK -> orangeColor
                    DetectedObjectType.ARM -> yellowColor
                    DetectedObjectType.BIG_TIP -> greenColor
                    DetectedObjectType.SMALL_TIP -> magentaColor
                }

                Imgproc.rectangle(frameImage,
                        Rect(detectedObject.x, detectedObject.y, detectedObject.width, detectedObject.height),
                        color)
            }

            frameImageWriter.write(frameDetectionResult.frameIndex, frameImage)
        }
    }

    private fun computeNewFrameDimension(
            frameWidth: Int,
            frameHeight: Int,
            frames: List<Frame>): NewFrameDimension {
        var minImageX = 0.0
        var minImageY = 0.0
        var maxImageX = 0.0
        var maxImageY = 0.0

        for (frame in frames) {
            minImageX = Math.min(minImageX, frame.imageX)
            minImageY = Math.min(minImageY, frame.imageY)
            maxImageX = Math.max(maxImageX, frame.imageX)
            maxImageY = Math.max(maxImageY, frame.imageY)
        }

        val width = frameWidth + Math.ceil(Math.abs(minImageX)) + Math.ceil(maxImageX)
        val height = frameHeight + Math.ceil(Math.abs(minImageY)) + Math.ceil(maxImageY)

        return NewFrameDimension(-minImageX, -minImageY, width.toInt(), height.toInt())
    }

    // New frame dimension to compensate for camera motion
    data class NewFrameDimension(
            val firstFrameImageX: Double,
            val firstFrameImageY: Double,
            val width: Int,
            val height: Int
    )
}