package fr.marcsworld.chopstickstracker.services.impl

import fr.marcsworld.chopstickstracker.model.Configuration
import fr.marcsworld.chopstickstracker.model.DetectedObjectStatus
import fr.marcsworld.chopstickstracker.model.Frame
import fr.marcsworld.chopstickstracker.model.Tip
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

    override fun renderTips(frames: List<Frame>, tips: List<Tip>, outputDirPath: String) {
        // Preparations
        val outputFile = eraseOutputFolder(outputDirPath)
        val (firstFrameImageX, firstFrameImageY, outputWidth, outputHeight) = computeNewFrameDimension(frames)

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

            val tipsInFrame = tips.stream()
                    .filter { it.detectionByFrameIndex.containsKey(frame.index) }
                    .collect(Collectors.toList())

            g.color = Color.WHITE
            for (tip in tipsInFrame) {
                val detectedObject = tip.detectionByFrameIndex[frame.index]
                        ?: throw IllegalStateException("Unable to find tip in frame: ${frame.index}")
                val x = Math.round(firstFrameImageX + detectedObject.x).toInt()
                val y = Math.round(firstFrameImageY + detectedObject.y).toInt()
                g.drawRect(x, y, detectedObject.width, detectedObject.height)
                g.drawString(tip.id, x, y + 16)
            }
            g.dispose()

            ImageIO.write(outputImage, "jpg", File(outputFile, "${frame.index}.jpg"))
        }
    }

    override fun renderCurrentAndPastTipDetections(frames: List<Frame>, maxFramesInPast: Int, outputDirPath: String) {
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

                    val detectedTips = frames[frameIndex].objects.stream()
                            .filter { it.objectType.isTip() }
                            .filter { it.confidence >= configuration.minTipDetectionConfidence }
                            .collect(Collectors.toList())

                    for (detectedTip in detectedTips) {
                        when {
                            detectedTip.status == DetectedObjectStatus.VISIBLE ->
                                g.color = Color(255, 255, 255, Math.round(alpha).toInt())
                            else ->
                                g.color = Color(0, 0, 255, Math.round(alpha).toInt())
                        }

                        val x = Math.round(firstFrameImageX + detectedTip.x).toInt()
                        val y = Math.round(firstFrameImageY + detectedTip.y).toInt()
                        g.drawRect(x, y, detectedTip.width, detectedTip.height)
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