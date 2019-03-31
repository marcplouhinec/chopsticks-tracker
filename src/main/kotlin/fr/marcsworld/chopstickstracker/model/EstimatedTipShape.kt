package fr.marcsworld.chopstickstracker.model

data class EstimatedTipShape(
        override val frameIndex: Int,
        override val status: EstimatedShapeStatus,
        override val detectedObject: DetectedObject?,
        override val x: Int,
        override val y: Int,
        override val width: Int,
        override val height: Int
) : Rectangle(x, y, width, height), EstimatedShape