package fr.marcsworld.chopstickstracker.model

data class Chopstick(
        val id: String,
        val tip1: Tip,
        val tip2: Tip,
        val shapes: MutableList<EstimatedChopstickShape>
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
}