package fr.marcsworld.chopstickstracker.services

import fr.marcsworld.chopstickstracker.model.DetectedObject
import java.io.File

interface ObjectDetectionService {

    fun detectObjectsInVideo(videoFile: File, onFrameProcessed: (frameIndex: Int, detectedObjects: List<DetectedObject>) -> Unit)

}