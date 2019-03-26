package fr.marcsworld.chopstickstracker.model

import com.fasterxml.jackson.annotation.JsonIgnoreProperties
import com.fasterxml.jackson.annotation.JsonProperty

@JsonIgnoreProperties(ignoreUnknown = true)
data class DetectedObject(
        override val x: Int,
        override val y: Int,
        override val width: Int,
        override val height: Int,
        @JsonProperty("object_type") val objectType: DetectedObjectType,
        val confidence: Double) : Rectangle(x, y, width, height)