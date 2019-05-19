package fr.marcsworld.chopstickstracker.model

import fr.marcsworld.chopstickstracker.model.detection.DetectedObject

data class EstimatedChopstickShape(
        override val frameIndex: Int,
        override val status: EstimatedShapeStatus,
        override val detectedObject: DetectedObject?,
        val tip1X: Int,
        val tip1Y: Int,
        val tip2X: Int,
        val tip2Y: Int,
        var isRejectedBecauseOfConflict: Boolean,
        val matchingScore: Double = Double.MAX_VALUE // Smaller is better
        // TODO direction
) : EstimatedShape