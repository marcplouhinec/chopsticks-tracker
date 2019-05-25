package fr.marcsworld.chopstickstracker.model

data class Configuration(
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

        val nbShapesToConsiderForComputingAverageTipPositionAndSize: Int,

        /**
         * Maximum score between a newly detected tip and an existing one in the same frame in order to
         * consider them as the same tip.
         */
        val maxScoreToConsiderNewTipAsTheSameAsAnExistingOne: Int,

        val minChopstickLengthInPixels: Int,

        val maxChopstickLengthInPixels: Int,

        /**
         * Maximum score between two tips and a detected object (chopstick type), in order to consider them as
         * a chopstick.
         */
        val maxMatchingScoreToConsiderTwoTipsAsAChopstick: Double)