package fr.marcsworld.chopstickstracker.model

data class Tip(
        val id: String,
        val shapes: MutableList<EstimatedTipShape>
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

