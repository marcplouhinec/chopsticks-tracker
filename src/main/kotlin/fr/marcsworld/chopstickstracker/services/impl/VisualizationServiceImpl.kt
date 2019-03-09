package fr.marcsworld.chopstickstracker.services.impl

import fr.marcsworld.chopstickstracker.model.Configuration
import fr.marcsworld.chopstickstracker.model.DetectedObjectType
import fr.marcsworld.chopstickstracker.model.Frame
import fr.marcsworld.chopstickstracker.model.Tip
import fr.marcsworld.chopstickstracker.services.FrameService
import fr.marcsworld.chopstickstracker.services.VisualizationService
import java.awt.Color
import java.io.File
import java.util.stream.Collectors
import javax.imageio.ImageIO

class VisualizationServiceImpl(
        private val configuration: Configuration,
        private val frameService: FrameService
) : VisualizationService {

    override fun renderTips(frames: List<Frame>, tips: List<Tip>, outputDirPath: String) {
        // Prepare the output folder
        val outputFile = eraseOutputFolder(outputDirPath)

        // Draw the tips on each frame
        for (frame in frames) {
            println("    Rendering tips in frame ${frame.index} / ${frames.size}...")

            val frameImage = frameService.findImageByIndex(frame.index)

            val tipsInFrame = tips.stream()
                    .filter { it.detectionByFrameIndex.containsKey(frame.index) }
                    .collect(Collectors.toList())

            val g = frameImage.createGraphics()

            g.color = Color.WHITE
            for (tip in tipsInFrame) {
                val detectedObject = tip.detectionByFrameIndex[frame.index]
                        ?: throw IllegalStateException("Unable to find tip in frame: ${frame.index}")
                g.drawRect(detectedObject.x, detectedObject.y, detectedObject.width, detectedObject.height)
                g.drawString(tip.id, detectedObject.x, detectedObject.y + 16)
            }
            g.dispose()

            ImageIO.write(frameImage, "jpg", File(outputFile, "${frame.index}.jpg"))
        }
    }

    override fun renderCurrentAndPastTipDetections(frames: List<Frame>, maxFramesInPast: Int, outputDirPath: String) {
        // Prepare the output folder
        val outputFile = eraseOutputFolder(outputDirPath)

        // Draw the current and past tip detections on each frame
        for (frame in frames) {
            println("    Rendering frame ${frame.index} / ${frames.size}...")

            val frameImage = frameService.findImageByIndex(frame.index)

            val g = frameImage.createGraphics()

            for (pastIndex in 0 until maxFramesInPast) {
                val frameIndex = frame.index - pastIndex
                if (frameIndex >= 0) {
                    val alpha = (maxFramesInPast - pastIndex) * (255.0 / maxFramesInPast)
                    g.color = Color(255, 255, 255, Math.round(alpha).toInt())

                    val detectedTips = frames[frameIndex].objects.stream()
                            .filter { it.objectType == DetectedObjectType.BIG_TIP || it.objectType == DetectedObjectType.SMALL_TIP }
                            .filter { it.confidence >= configuration.minTipDetectionConfidence }
                            .collect(Collectors.toList())

                    for (detectedTip in detectedTips) {
                        g.drawRect(detectedTip.x, detectedTip.y, detectedTip.width, detectedTip.height)
                    }
                }
            }

            g.dispose()

            ImageIO.write(frameImage, "jpg", File(outputFile, "${frame.index}.jpg"))
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
}