package fr.marcsworld.chopstickstracker.model.detection

import org.opencv.core.Mat

/**
 * Contains the detected objects in one frame.
 *
 * @author Marc Plouhinec
 */
data class FrameDetectionResult(
        val frameIndex: Int,
        val detectedObjects: List<DetectedObject>,
        val frameImageProvider: () -> Mat
)