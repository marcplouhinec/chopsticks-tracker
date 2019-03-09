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
            0.03,
            7)
    val frameService: FrameService = LocalStorageFrameServiceImpl(configuration)
    val videoDetectionService: VideoDetectionService = VideoDetectionServiceImpl(configuration)
    val visualizationService: VisualizationService = VisualizationServiceImpl(configuration, frameService)

    println("Load all frames...")
    val frames = frameService.findAll()
    println("${frames.size} frames loaded.")

    println("Detecting tips...")
    val tips = videoDetectionService.findAllTips(frames)
    println("${tips.size} tips detected.")

    println("Render images...")
    //visualizationService.renderTips(frames, tips, "/Users/marcplouhinec/projects/chopsticks-tracker/output")
    visualizationService.renderCurrentAndPastTipDetections(frames, 10, "/Users/marcplouhinec/projects/chopsticks-tracker/output")
    println("Images rendered.")
}
