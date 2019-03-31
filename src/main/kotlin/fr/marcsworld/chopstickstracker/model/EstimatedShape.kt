package fr.marcsworld.chopstickstracker.model

interface EstimatedShape {
    val frameIndex: Int
    val status: EstimatedShapeStatus
    val detectedObject: DetectedObject?
}