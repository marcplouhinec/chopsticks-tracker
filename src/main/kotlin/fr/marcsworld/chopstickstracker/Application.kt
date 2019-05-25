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

val LOGGER = LoggerFactory.getLogger("main")

fun main() {
    // Load the context
    val context = AnnotationConfigApplicationContext(ApplicationConfiguration::class.java)
    val objectDetectionService: ObjectDetectionService = context.getBean(CachedYoloObjectDetectionServiceImpl::class.java)
    val videoTrackingService: VideoTrackingService = context.getBean(VideoTrackingService::class.java)
    val visualizationService: VisualizationService = context.getBean(VisualizationService::class.java)

    // Detect objects in the video
    val videoFile = File("/Users/marcplouhinec/projects/chopsticks-tracker/data/input-video/VID_20181231_133114.mp4")
    val frameDetectionResultIterable = objectDetectionService.detectObjectsInVideo(videoFile)

    // Process the frames
    val frames = frameDetectionResultIterable.map {
        Frame(it.frameIndex, it.detectedObjects)
    }

    LOGGER.info("Load all frames...")
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
    /*val frameImageWriter: FrameImageWriter = FolderFrameImageWriter(
            File("/Users/marcplouhinec/projects/chopsticks-tracker/output")
    )*/
    val videoCapture = VideoCapture(videoFile.absolutePath)
    val fps = videoCapture.get(Videoio.CAP_PROP_FPS)
    val frameWidth = videoCapture.get(Videoio.CAP_PROP_FRAME_WIDTH).toInt()
    val frameHeight = videoCapture.get(Videoio.CAP_PROP_FRAME_HEIGHT).toInt()
    videoCapture.release()
    val frameImageWriter: FrameImageWriter = VideoFrameImageWriter(
            File("/Users/marcplouhinec/projects/chopsticks-tracker/output/${videoFile.nameWithoutExtension}.mpg"),
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
