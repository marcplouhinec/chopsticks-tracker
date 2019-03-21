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

        val maxTipMatchingScoreInPixels: Int,

        /**
         * Maximum score between a tip from a previous frame that overlaps with an arm and another object
         * within the current frame.
         */
        val maxMatchingScoreToConsiderTipNotHiddenByArm: Int,

        val nbFramesAfterWhichATipIsConsideredMissing: Int,

        val nbShapesToConsiderForComputingAverageTipPositionAndSize: Int)