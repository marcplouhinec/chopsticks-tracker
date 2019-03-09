package fr.marcsworld.chopstickstracker.model

import java.util.*

data class Tip(
        val id: String,
        val detectionByFrameIndex: TreeMap<Int, DetectedObject>
) {
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as Tip

        if (id != other.id) return false

        return true
    }

    override fun hashCode(): Int {
        return id.hashCode()
    }

    /**
     * @return Date of birth in "frame index".
     */
    fun getDateOfBirth(): Int {
        return detectionByFrameIndex.firstKey()
    }
}