package fr.marcsworld.chopstickstracker.model

data class Configuration(
        val extractedFrameObjectsPath: String,
        val extractedFrameImagesPath: String,
        val frameWidth: Int,
        val frameHeight: Int,
        val minTipDetectionConfidence: Double,
        val minChopstickDetectionConfidence: Double,
        val minArmDetectionConfidence: Double,
        val nbTipsToUseToDetectCameraMotion: Int,
        val maxTipMatchingScore: Double, // TODO better in pixels
        val nbFramesAfterWhichATipIsConsideredMissing: Int)