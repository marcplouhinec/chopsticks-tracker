package fr.marcsworld.chopstickstracker.model

data class Rectangle(
        override val x: Int = 0,
        override val y: Int = 0,
        override val width: Int = 0,
        override val height: Int = 0,
        val confidence: Double = 0.0) : Rectangular