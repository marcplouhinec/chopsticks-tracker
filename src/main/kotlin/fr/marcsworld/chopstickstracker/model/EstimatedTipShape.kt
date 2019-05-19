package fr.marcsworld.chopstickstracker.model

import fr.marcsworld.chopstickstracker.model.detection.DetectedObject

data class EstimatedTipShape(
        override val frameIndex: Int,
        override val status: EstimatedShapeStatus,
        override val detectedObject: DetectedObject?,
        override val x: Int,
        override val y: Int,
        override val width: Int,
        override val height: Int
) : Rectangular, EstimatedShape