package fr.marcsworld.chopstickstracker.services

import fr.marcsworld.chopstickstracker.model.Chopstick
import fr.marcsworld.chopstickstracker.model.Frame
import fr.marcsworld.chopstickstracker.model.Tip

interface VisualizationService {

    fun renderTips(frames: List<Frame>, tips: List<Tip>, outputDirPath: String, detectedChopstickVisible: Boolean = false, chopsticks: List<Chopstick> = emptyList())

    fun renderCurrentAndPastTipDetections(
            frames: List<Frame>, maxFramesInPast: Int, outputDirPath: String, armVisible: Boolean = false)
}