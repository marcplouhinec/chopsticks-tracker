package fr.marcsworld.chopstickstracker.model.detection

import com.fasterxml.jackson.annotation.JsonIgnoreProperties
import fr.marcsworld.chopstickstracker.model.Rectangular

@JsonIgnoreProperties(ignoreUnknown = true)
data class DetectedObject(
        override val x: Int = 0,
        override val y: Int = 0,
        override val width: Int = 0,
        override val height: Int = 0,
        val objectType: DetectedObjectType = DetectedObjectType.CHOPSTICK,
        val confidence: Double = 0.0) : Rectangular