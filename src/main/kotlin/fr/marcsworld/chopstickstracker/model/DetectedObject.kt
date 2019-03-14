package fr.marcsworld.chopstickstracker.model

import com.fasterxml.jackson.annotation.JsonIgnoreProperties
import com.fasterxml.jackson.annotation.JsonProperty

@JsonIgnoreProperties(ignoreUnknown = true)
data class DetectedObject(
        val x: Int,
        val y: Int,
        val width: Int,
        val height: Int,
        @JsonProperty("object_type") val objectType: DetectedObjectType,
        val confidence: Double,
        val status: DetectedObjectStatus = DetectedObjectStatus.VISIBLE)