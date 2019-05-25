package fr.marcsworld.chopstickstracker

import fr.marcsworld.chopstickstracker.model.Frame
import fr.marcsworld.chopstickstracker.services.detection.ObjectDetectionService
import fr.marcsworld.chopstickstracker.services.detection.impl.CachedYoloObjectDetectionServiceImpl
import fr.marcsworld.chopstickstracker.services.rendering.VisualizationService
import fr.marcsworld.chopstickstracker.services.rendering.writer.FrameImageWriter
import fr.marcsworld.chopstickstracker.services.rendering.writer.impl.VideoFrameImageWriter
import fr.marcsworld.chopstickstracker.services.tracking.VideoTrackingService
import org.opencv.videoio.VideoCapture
import org.opencv.videoio.Videoio
import org.slf4j.LoggerFactory
import org.springframework.context.annotation.AnnotationConfigApplicationContext
import java.io.File

val LOGGER = LoggerFactory.getLogger("main")!!

/**
 * Detect chopsticks in a video.
 *
 * @author Marc Plouhinec
 */
fun main(args: Array<String>) {
    if (args.size != 2) {
        return println("Usage: java -jar chopsticks-tracker-*-jar-with-dependencies.jar " +
                "<input-video-path> " +
                "<output-folder-path>")
    }
    val videoFile = File(args[0])
    val outputFolderFile = File(args[1])

    // Load the context
    val context = AnnotationConfigApplicationContext(ApplicationConfiguration::class.java)
    val objectDetectionService: ObjectDetectionService = context.getBean(CachedYoloObjectDetectionServiceImpl::class.java)
    val videoTrackingService: VideoTrackingService = context.getBean(VideoTrackingService::class.java)
    val visualizationService: VisualizationService = context.getBean(VisualizationService::class.java)

    // Detect objects in the video
    val frameDetectionResultIterable = objectDetectionService.detectObjectsInVideo(videoFile)

    // Process the frames
    LOGGER.info("Load all frames...")
    val frames = frameDetectionResultIterable.map {
        Frame(it.frameIndex, it.detectedObjects)
    }
    LOGGER.info("{} frames loaded.", frames.size)

    LOGGER.info("Remove unreliable detected objects...")
    val reliableFrames = videoTrackingService.removeUnreliableDetectedObjects(frames)

    LOGGER.info("Compensate for camera motion...")
    val compensatedFrames = videoTrackingService.compensateCameraMotion(reliableFrames)

    LOGGER.info("Detecting tips...")
    val tips = videoTrackingService.findAllTips(compensatedFrames)
    LOGGER.info("{} tips detected.", tips.size)

    LOGGER.info("Detecting chopsticks...")
    val chopsticks = videoTrackingService.findAllChopsticks(compensatedFrames, tips)

    LOGGER.info("Render images...")
    // val frameImageWriter: FrameImageWriter = FolderFrameImageWriter(outputFolderFile)
    val videoCapture = VideoCapture(videoFile.absolutePath)
    val fps = videoCapture.get(Videoio.CAP_PROP_FPS)
    val frameWidth = videoCapture.get(Videoio.CAP_PROP_FRAME_WIDTH).toInt()
    val frameHeight = videoCapture.get(Videoio.CAP_PROP_FRAME_HEIGHT).toInt()
    videoCapture.release()
    val frameImageWriter: FrameImageWriter = VideoFrameImageWriter(
            File(outputFolderFile, "${videoFile.nameWithoutExtension}.mpg"),
            fps
    )
    frameImageWriter.use { writer ->
        visualizationService.renderTips(
                frameWidth,
                frameHeight,
                compensatedFrames,
                tips,
                writer,
                false,
                chopsticks,
                false,
                frameDetectionResultIterable)
        /*visualizationService.renderCurrentAndPastTipDetections(
            frameWidth,
            frameHeight,
            compensatedFrames,
            10,
            writer,
            true,
            frameDetectionResultIterable)*/
        /*visualizationService.renderDetectedObjects(
            frameWidth,
            frameHeight,
            frameDetectionResultIterable,
            writer)*/
    }
    LOGGER.info("Images rendered.")
}
