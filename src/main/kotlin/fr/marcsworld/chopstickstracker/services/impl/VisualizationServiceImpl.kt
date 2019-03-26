package fr.marcsworld.chopstickstracker.services.impl

import fr.marcsworld.chopstickstracker.model.*
import fr.marcsworld.chopstickstracker.services.FrameService
import fr.marcsworld.chopstickstracker.services.VisualizationService
import java.awt.Color
import java.awt.image.BufferedImage
import java.io.File
import java.util.stream.Collectors
import javax.imageio.ImageIO


class VisualizationServiceImpl(
        private val configuration: Configuration,
        private val frameService: FrameService
) : VisualizationService {

    override fun renderTips(
            frames: List<Frame>, tips: List<Tip>, outputDirPath: String, chopstickVisible: Boolean, chopsticksByFrameIndex: List<List<Chopstick>>) {
        // Preparations
        val outputFile = eraseOutputFolder(outputDirPath)
        val (firstFrameImageX, firstFrameImageY, outputWidth, outputHeight) = computeNewFrameDimension(frames)

        val shapesWithTipsByFrameIndex: Map<Int, List<Pair<EstimatedShape, Tip>>> = tips.stream()
                .flatMap { tip ->
                    tip.shapes.stream()
                            .filter { it.status != EstimatedShapeStatus.LOST }
                            .map { Pair(it, tip) }
                }
                .collect(Collectors.groupingBy { it.first.frameIndex })

        // Draw the tips on each frame
        for (frame in frames) {
            println("    Rendering tips in frame ${frame.index} / ${frames.size}...")

            val frameImage = frameService.findImageByIndex(frame.index)
            val outputImage = BufferedImage(outputWidth, outputHeight, BufferedImage.TYPE_INT_RGB)

            val g = outputImage.createGraphics()
            g.drawImage(frameImage,
                    Math.round(firstFrameImageX + frame.imageX).toInt(),
                    Math.round(firstFrameImageY + frame.imageY).toInt(),
                    null)

            if (chopstickVisible) {
                g.color = Color.CYAN
                for (detectedChopstick in frame.objects) {
                    if (detectedChopstick.objectType == DetectedObjectType.CHOPSTICK) {
                        val x = Math.round(firstFrameImageX + detectedChopstick.x).toInt()
                        val y = Math.round(firstFrameImageY + detectedChopstick.y).toInt()

                        g.drawRect(x, y, detectedChopstick.width, detectedChopstick.height)
                    }
                }
            }

            val shapesWithTips = shapesWithTipsByFrameIndex[frame.index] ?: listOf()

            for (shapeWithTip in shapesWithTips) {
                val shape = shapeWithTip.first
                val tip = shapeWithTip.second

                g.color = when {
                    shape.status == EstimatedShapeStatus.DETECTED_ONCE -> Color.GREEN
                    shape.status == EstimatedShapeStatus.NOT_DETECTED -> Color.ORANGE
                    shape.status == EstimatedShapeStatus.HIDDEN_BY_ARM -> Color.MAGENTA
                    else -> Color.WHITE
                }

                val x = Math.round(firstFrameImageX + shape.x).toInt()
                val y = Math.round(firstFrameImageY + shape.y).toInt()
                g.drawRect(x, y, shape.width, shape.height)
                g.drawString(tip.id, x, y + 16)
            }

            g.color = Color.RED
            val shapeByTip: Map<Tip, EstimatedShape> = shapesWithTips.stream()
                    .collect(Collectors.toMap({ it.second }, { it.first }))
            for (chopstick in chopsticksByFrameIndex[frame.index]) {
                val shape0 = shapeByTip[chopstick.tips[0]] ?: throw IllegalStateException()
                val shape1 = shapeByTip[chopstick.tips[1]] ?: throw IllegalStateException()

                val x0 = Math.round(firstFrameImageX + shape0.x + shape0.width / 2).toInt()
                val y0 = Math.round(firstFrameImageY + shape0.y + shape0.height / 2).toInt()
                val x1 = Math.round(firstFrameImageX + shape1.x + shape1.width / 2).toInt()
                val y1 = Math.round(firstFrameImageY + shape1.y + shape1.height / 2).toInt()

                g.drawLine(x0, y0, x1, y1)
            }

            g.dispose()

            ImageIO.write(outputImage, "jpg", File(outputFile, "${frame.index}.jpg"))
        }
    }

    override fun renderCurrentAndPastTipDetections(
            frames: List<Frame>, maxFramesInPast: Int, outputDirPath: String, armVisible: Boolean) {
        // Preparations
        val outputFile = eraseOutputFolder(outputDirPath)
        val (firstFrameImageX, firstFrameImageY, outputWidth, outputHeight) = computeNewFrameDimension(frames)

        // Draw the current and past tip detections on each frame
        for (frame in frames) {
            println("    Rendering frame ${frame.index} / ${frames.size}...")

            val frameImage = frameService.findImageByIndex(frame.index)
            val outputImage = BufferedImage(outputWidth, outputHeight, BufferedImage.TYPE_INT_RGB)

            val g = outputImage.createGraphics()
            g.drawImage(frameImage,
                    Math.round(firstFrameImageX + frame.imageX).toInt(),
                    Math.round(firstFrameImageY + frame.imageY).toInt(),
                    null)

            for (pastIndex in maxFramesInPast downTo 0) {
                val frameIndex = frame.index - pastIndex
                if (frameIndex >= 0) {
                    val alpha = (maxFramesInPast - pastIndex) * (255.0 / maxFramesInPast)
                    g.color = Color(255, 255, 255, Math.round(alpha).toInt())

                    val detectedTips = frames[frameIndex].objects.stream()
                            .filter { it.objectType.isTip() }
                            .filter { it.confidence >= configuration.minTipDetectionConfidence }
                            .collect(Collectors.toList())

                    for (detectedTip in detectedTips) {
                        val x = Math.round(firstFrameImageX + detectedTip.x).toInt()
                        val y = Math.round(firstFrameImageY + detectedTip.y).toInt()
                        g.drawRect(x, y, detectedTip.width, detectedTip.height)
                    }
                }
            }

            if (armVisible) {
                g.color = Color.YELLOW
                for (detectedArm in frame.objects) {
                    if (detectedArm.objectType == DetectedObjectType.ARM) {
                        g.drawRect(detectedArm.x, detectedArm.y, detectedArm.width, detectedArm.height)
                    }
                }
            }

            g.dispose()

            ImageIO.write(outputImage, "jpg", File(outputFile, "${frame.index}.jpg"))
        }
    }

    private fun eraseOutputFolder(outputDirPath: String): File {
        val outputFile = File(outputDirPath)
        if (outputFile.exists()) {
            if (!outputFile.deleteRecursively()) {
                throw IllegalStateException("Unable to delete the directory: $outputDirPath")
            }
        }
        outputFile.mkdirs()
        return outputFile
    }

    private fun computeNewFrameDimension(frames: List<Frame>): NewFrameDimension {
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

        val width = configuration.frameWidth + Math.ceil(Math.abs(minImageX)) + Math.ceil(maxImageX)
        val height = configuration.frameHeight + Math.ceil(Math.abs(minImageY)) + Math.ceil(maxImageY)

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