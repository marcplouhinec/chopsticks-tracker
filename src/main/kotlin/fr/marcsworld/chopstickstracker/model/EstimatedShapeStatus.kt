package fr.marcsworld.chopstickstracker.model

enum class EstimatedShapeStatus {
    DETECTED, DETECTED_ONCE, NOT_DETECTED, HIDDEN_BY_ARM, LOST;

    fun isDetected(): Boolean {
        return this == DETECTED || this == DETECTED_ONCE
    }
}