package fr.marcsworld.chopstickstracker.model

import fr.marcsworld.chopstickstracker.model.detection.DetectedObject

interface EstimatedShape {
    val frameIndex: Int
    val status: EstimatedShapeStatus
    val detectedObject: DetectedObject?
}