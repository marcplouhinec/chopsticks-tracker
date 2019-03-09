package fr.marcsworld.chopstickstracker.model

data class Configuration(
        val extractedFrameObjectsPath: String,
        val extractedFrameImagesPath: String,
        val frameWidth: Int,
        val frameHeight: Int,
        val minTipDetectionConfidence: Double,
        val nbTipsToUseToDetectCameraMovements: Int,
        val maxTipMatchingScore: Double,
        val nbFramesAfterWhichATipIsConsideredMissing: Int)