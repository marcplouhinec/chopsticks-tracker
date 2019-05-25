package fr.marcsworld.chopstickstracker.services.rendering

import fr.marcsworld.chopstickstracker.model.Chopstick
import fr.marcsworld.chopstickstracker.model.Frame
import fr.marcsworld.chopstickstracker.model.Tip
import fr.marcsworld.chopstickstracker.model.detection.FrameDetectionResult
import fr.marcsworld.chopstickstracker.services.rendering.writer.FrameImageWriter

/**
 * Draw computed objects on images.
 *
 * @author Marc Plouhinec
 */
interface VisualizationService {

    fun renderTips(
            frameWidth: Int,
            frameHeight: Int,
            frames: List<Frame>,
            tips: List<Tip>,
            frameImageWriter: FrameImageWriter,
            detectedChopstickVisible: Boolean = false,
            chopsticks: List<Chopstick> = emptyList(),
            alternativeChopsticksVisible: Boolean = false,
            frameDetectionResults: Iterable<FrameDetectionResult>)

    fun renderCurrentAndPastTipDetections(
            frameWidth: Int,
            frameHeight: Int,
            frames: List<Frame>,
            maxFramesInPast: Int,
            frameImageWriter: FrameImageWriter,
            armVisible: Boolean = false,
            frameDetectionResults: Iterable<FrameDetectionResult>)

    fun renderDetectedObjects(
            frameDetectionResults: Iterable<FrameDetectionResult>,
            frameImageWriter: FrameImageWriter)
}