package fr.marcsworld.chopstickstracker.model

enum class DetectedObjectType {
    ARM, CHOPSTICK, BIG_TIP, SMALL_TIP;

    fun isTip(): Boolean {
        return this == BIG_TIP || this == SMALL_TIP
    }
}