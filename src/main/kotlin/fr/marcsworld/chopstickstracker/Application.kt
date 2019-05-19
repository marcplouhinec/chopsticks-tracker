package fr.marcsworld.chopstickstracker

import com.fasterxml.jackson.databind.ObjectMapper
import fr.marcsworld.chopstickstracker.model.Configuration
import fr.marcsworld.chopstickstracker.services.FrameService
import fr.marcsworld.chopstickstracker.services.VideoDetectionService
import fr.marcsworld.chopstickstracker.services.VisualizationService
import fr.marcsworld.chopstickstracker.services.detection.ObjectDetectionService
import fr.marcsworld.chopstickstracker.services.impl.LocalStorageFrameServiceImpl
import fr.marcsworld.chopstickstracker.services.impl.VideoDetectionServiceImpl
import fr.marcsworld.chopstickstracker.services.impl.VisualizationServiceImpl
import org.opencv.imgcodecs.Imgcodecs
import org.springframework.context.annotation.AnnotationConfigApplicationContext
import java.io.File

fun main() {
    val context = AnnotationConfigApplicationContext(ApplicationConfiguration::class.java)
    val objectDetectionService = context.getBean(ObjectDetectionService::class.java)

    val videoFile = File("/Users/marcplouhinec/projects/chopsticks-tracker/data/input-video/VID_20181231_133114.mp4")
    val objectMapper = ObjectMapper()
    val extractedFrameObjectsPath = "/Users/marcplouhinec/tmp/extracted_frame_objects"
    val extractedFrameImagesPath = "/Users/marcplouhinec/tmp/extracted_frame_images"

    val frameDetectionResultIterable = objectDetectionService.detectObjectsInVideo(videoFile)
    for (result in frameDetectionResultIterable) {
        println("frameIndex = ${result.frameIndex}, frame size = ${result.frameImage.size()}, detectedObjects = ${result.detectedObjects.size}")

        val objectsJson = objectMapper.writeValueAsString(result.detectedObjects)
        val jsonFile = File("$extractedFrameObjectsPath/${result.frameIndex}.json")
        jsonFile.bufferedWriter().use { out ->
            out.write(objectsJson)
        }

        val jpgFile = File("$extractedFrameImagesPath/${result.frameIndex}.jpg")
        Imgcodecs.imwrite(jpgFile.absolutePath, result.frameImage)
    }

    if (1 + 1 == 2) { // TODO
        return
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
    val frameService: FrameService = LocalStorageFrameServiceImpl(configuration)
    val videoDetectionService: VideoDetectionService = VideoDetectionServiceImpl(configuration)
    val visualizationService: VisualizationService = VisualizationServiceImpl(configuration, frameService)

    println("Load all frames...")
    val frames = frameService.findAll()
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
    visualizationService.renderTips(
            compensatedFrames,
            tips,
            "/Users/marcplouhinec/projects/chopsticks-tracker/output",
            false,
            chopsticks,
            false)
    //visualizationService.renderCurrentAndPastTipDetections(
    //        compensatedFrames, 10, "/Users/marcplouhinec/projects/chopsticks-tracker/output", true)
    println("Images rendered.")
}
