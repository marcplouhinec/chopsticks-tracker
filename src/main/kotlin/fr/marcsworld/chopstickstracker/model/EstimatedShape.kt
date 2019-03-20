package fr.marcsworld.chopstickstracker.model

data class EstimatedShape(
        val frameIndex: Int,
        val status: EstimatedShapeStatus,
        val detectedObject: DetectedObject?,
        override val x: Int,
        override val y: Int,
        override val width: Int,
        override val height: Int
) : Rectangle