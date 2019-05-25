package fr.marcsworld.chopstickstracker.services.rendering.writer.impl

import fr.marcsworld.chopstickstracker.services.rendering.writer.FrameImageWriter
import org.opencv.core.Mat
import org.opencv.core.Size
import org.opencv.videoio.VideoWriter
import java.io.File

/**
 * Write frame into a video.
 *
 * @author Marc Plouhinec
 */
class VideoFrameImageWriter(
        private val videoFile: File,
        private val fps: Double = 30.0
) : FrameImageWriter {

    private var videoWriter: VideoWriter? = null

    init {
        if (videoFile.exists()) {
            if (!videoFile.delete()) {
                throw IllegalStateException("Unable to delete the video: $videoFile")
            }
        }
    }

    override fun initFrameSize(width: Int, height: Int) {
        val fourcc = VideoWriter.fourcc('M', 'J', 'P', 'G')
        val frameSize = Size(width.toDouble(), height.toDouble())
        videoWriter = VideoWriter(videoFile.absolutePath, fourcc, fps, frameSize, true)
    }

    override fun write(frameIndex: Int, frameImage: Mat) {
        videoWriter?.write(frameImage)
    }

    override fun close() {
        videoWriter?.release()
    }
}