package fr.marcsworld.chopstickstracker.model

data class EstimatedChopstickShape(
        override val frameIndex: Int,
        override val status: EstimatedShapeStatus,
        override val detectedObject: DetectedObject?,
        val tip1X: Int,
        val tip1Y: Int,
        val tip2X: Int,
        val tip2Y: Int
        // TODO direction
) : EstimatedShape