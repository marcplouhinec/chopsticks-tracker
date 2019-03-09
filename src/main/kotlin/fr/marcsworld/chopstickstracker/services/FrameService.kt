package fr.marcsworld.chopstickstracker.services

import fr.marcsworld.chopstickstracker.model.Frame
import java.awt.image.BufferedImage

interface FrameService {

    fun findAll(): List<Frame>

    fun findImageByIndex(frameIndex: Int): BufferedImage

}