package fr.marcsworld.chopstickstracker.services.rendering.writer

import org.opencv.core.Mat

/**
 * Write an image into a container.
 *
 * @author Marc Plouhinec
 */
interface FrameImageWriter : AutoCloseable {

    /**
     * Initialize the frame dimension.
     */
    fun initFrameSize(width: Int, height: Int)

    /**
     * Write the given image.
     */
    fun write(frameIndex: Int, frameImage: Mat)

}