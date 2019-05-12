package fr.marcsworld.chopstickstracker.services.impl

import fr.marcsworld.chopstickstracker.model.DetectedObject
import fr.marcsworld.chopstickstracker.services.ObjectDetectionService
import org.bytedeco.javacpp.Loader
import org.bytedeco.opencv.opencv_java
import org.opencv.dnn.Dnn.readNetFromDarknet
import java.io.File
import java.util.stream.Collectors

class YoloObjectDetectionServiceImpl(
        private val yoloModelConfigFile: File,
        private val yoloModelWeightsFile: File
) : ObjectDetectionService {

    init {
        Loader.load(opencv_java::class.java)
    }

    override fun detectObjectsInVideo(videoFile: File, onFrameProcessed: (frameIndex: Int, detectedObjects: List<DetectedObject>) -> Unit) {
        val net = readNetFromDarknet(yoloModelConfigFile.absolutePath, yoloModelWeightsFile.absolutePath)
        val layerNames = net.layerNames
        val outLayerNames = net.unconnectedOutLayers.toList().stream()
                .map { layerNames[it - 1] }
                .collect(Collectors.toList())
        println("Yolo net initialized: outLayerNames = $outLayerNames")

        // TODO
    }
}