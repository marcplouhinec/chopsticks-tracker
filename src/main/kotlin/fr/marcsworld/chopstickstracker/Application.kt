package fr.marcsworld.chopstickstracker

import fr.marcsworld.chopstickstracker.model.Configuration
import fr.marcsworld.chopstickstracker.services.FrameService
import fr.marcsworld.chopstickstracker.services.VideoDetectionService
import fr.marcsworld.chopstickstracker.services.VisualizationService
import fr.marcsworld.chopstickstracker.services.impl.LocalStorageFrameServiceImpl
import fr.marcsworld.chopstickstracker.services.impl.VideoDetectionServiceImpl
import fr.marcsworld.chopstickstracker.services.impl.VisualizationServiceImpl


fun main(args: Array<String>) {
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
            15)
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
