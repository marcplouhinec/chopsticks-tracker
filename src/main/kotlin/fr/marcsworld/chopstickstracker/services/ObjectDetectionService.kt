package fr.marcsworld.chopstickstracker.services

import fr.marcsworld.chopstickstracker.model.DetectedObject
import org.opencv.core.Mat
import java.io.File

interface ObjectDetectionService {

    fun detectObjectsInVideo(videoFile: File, onFrameProcessed: (frameIndex: Int, frame: Mat, detectedObjects: List<DetectedObject>) -> Unit)

}