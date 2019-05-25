package fr.marcsworld.chopstickstracker

import fr.marcsworld.chopstickstracker.model.Configuration
import fr.marcsworld.chopstickstracker.model.Frame
import fr.marcsworld.chopstickstracker.services.VideoDetectionService
import fr.marcsworld.chopstickstracker.services.VisualizationService
import fr.marcsworld.chopstickstracker.services.detection.ObjectDetectionService
import fr.marcsworld.chopstickstracker.services.detection.impl.CachedYoloObjectDetectionServiceImpl
import fr.marcsworld.chopstickstracker.services.impl.VideoDetectionServiceImpl
import fr.marcsworld.chopstickstracker.services.impl.VisualizationServiceImpl
import org.springframework.context.annotation.AnnotationConfigApplicationContext
import java.io.File

fun main() {
    // Load the context
    val context = AnnotationConfigApplicationContext(ApplicationConfiguration::class.java)
    val objectDetectionService: ObjectDetectionService = context.getBean(CachedYoloObjectDetectionServiceImpl::class.java)

    // Detect objects in the video
    val videoFile = File("/Users/marcplouhinec/projects/chopsticks-tracker/data/input-video/VID_20181231_133114.mp4")
    val frameDetectionResultIterable = objectDetectionService.detectObjectsInVideo(videoFile)

    // Process the frames
    val frames = frameDetectionResultIterable.take(20).map {
        Frame(it.frameIndex, it.detectedObjects)
    }

    val configuration = Configuration(
            "/Users/marcplouhinec/projects/yolo3/output/extracted_frame_objects",
            "/Users/marcplouhinec/projects/yolo3/output/extracted_frame_images",
            1920, 1080,
            0.9,
            0.7,
            0.7,
            7,
            66,
            20,
            7,
            9,
            15,
            350,
            550,
            0.8)
    val videoDetectionService: VideoDetectionService = VideoDetectionServiceImpl(configuration)
    val visualizationService: VisualizationService = VisualizationServiceImpl(configuration)

    println("Load all frames...")
    println("${frames.size} frames loaded.")

    println("Remove unreliable detected objects...")
    val reliableFrames = videoDetectionService.removeUnreliableDetectedObjects(frames)

    println("Compensate for camera motion...")
    val compensatedFrames = videoDetectionService.compensateCameraMotion(reliableFrames)

    println("Detecting tips...")
    val tips = videoDetectionService.findAllTips(compensatedFrames)
    println("${tips.size} tips detected.")

    println("Detecting chopsticks...")
    val chopsticks = videoDetectionService.findAllChopsticks(compensatedFrames, tips)

    println("Render images...")
    /*visualizationService.renderTips(
            compensatedFrames,
            tips,
            "/Users/marcplouhinec/projects/chopsticks-tracker/output",
            false,
            chopsticks,
            false,
            frameDetectionResultIterable)*/
    /*visualizationService.renderCurrentAndPastTipDetections(
            compensatedFrames,
            10,
            "/Users/marcplouhinec/projects/chopsticks-tracker/output",
            true,
            frameDetectionResultIterable)*/
    visualizationService.renderDetectedObjects(
            frameDetectionResultIterable.take(20),
            "/Users/marcplouhinec/projects/chopsticks-tracker/output")
    println("Images rendered.")
}
