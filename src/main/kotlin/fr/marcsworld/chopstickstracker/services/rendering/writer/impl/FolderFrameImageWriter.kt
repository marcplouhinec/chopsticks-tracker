package fr.marcsworld.chopstickstracker.services.rendering.writer.impl

import fr.marcsworld.chopstickstracker.services.rendering.writer.FrameImageWriter
import org.opencv.core.Mat
import org.opencv.imgcodecs.Imgcodecs
import java.io.File

/**
 * Write frame images into a folder.
 *
 * @author Marc Plouhinec
 */
class FolderFrameImageWriter(

        private val outputFolderFile: File

) : FrameImageWriter {

    init {
        // Erase the output folder
        if (outputFolderFile.exists()) {
            if (!outputFolderFile.deleteRecursively()) {
                throw IllegalStateException("Unable to delete the directory: $outputFolderFile")
            }
        }
        outputFolderFile.mkdirs()
    }

    override fun initFrameSize(width: Int, height: Int) {
        // Nothing to do
    }

    override fun write(frameIndex: Int, frameImage: Mat) {
        Imgcodecs.imwrite(File(outputFolderFile, "${frameIndex}.jpg").absolutePath, frameImage)
    }

    override fun close() {
        // Nothing to do
    }
}