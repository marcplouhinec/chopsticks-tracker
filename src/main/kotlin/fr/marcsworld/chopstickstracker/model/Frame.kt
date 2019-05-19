package fr.marcsworld.chopstickstracker.model

import fr.marcsworld.chopstickstracker.model.detection.DetectedObject

data class Frame(
        val index: Int,
        val objects: List<DetectedObject>,
        val imageX: Double = 0.0,
        val imageY: Double = 0.0)