package fr.marcsworld.chopstickstracker.services.detection

import fr.marcsworld.chopstickstracker.model.detection.FrameDetectionResult
import java.io.File

/**
 * Detect objects in images and videos.
 * Note: the detectable object types are listed in [fr.marcsworld.chopstickstracker.model.DetectedObjectType].
 *
 * @author Marc Plouhinec
 */
interface ObjectDetectionService {

    /**
     * Detect objects in each frame of the given video.
     */
    fun detectObjectsInVideo(videoFile: File): Iterable<FrameDetectionResult>

}