package fr.marcsworld.chopstickstracker.services

import fr.marcsworld.chopstickstracker.model.Frame
import fr.marcsworld.chopstickstracker.model.Tip

interface VideoDetectionService {

    fun findAllTips(frames: List<Frame>): List<Tip>

}