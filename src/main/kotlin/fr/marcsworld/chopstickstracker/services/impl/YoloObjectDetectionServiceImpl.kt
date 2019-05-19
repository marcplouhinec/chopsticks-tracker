package fr.marcsworld.chopstickstracker.services.impl

import fr.marcsworld.chopstickstracker.model.detection.DetectedObject
import fr.marcsworld.chopstickstracker.model.detection.DetectedObjectType
import fr.marcsworld.chopstickstracker.services.ObjectDetectionService
import org.opencv.core.Mat
import org.opencv.core.Scalar
import org.opencv.core.Size
import org.opencv.dnn.Dnn.blobFromImage
import org.opencv.dnn.Dnn.readNetFromDarknet
import org.opencv.videoio.VideoCapture
import org.opencv.videoio.Videoio
import java.io.File
import java.util.stream.Collectors
import java.util.stream.IntStream

class YoloObjectDetectionServiceImpl(
        private val yoloModelConfigFile: File,
        private val yoloModelWeightsFile: File
) : ObjectDetectionService {

    companion object {
        const val minConfidence = 0.1 // TODO Why 0.1?

        val objectTypeByDetectionIndex = arrayOf( // TODO Why theses values?
                DetectedObjectType.ARM,
                DetectedObjectType.BIG_TIP,
                DetectedObjectType.CHOPSTICK,
                DetectedObjectType.SMALL_TIP)
    }

    override fun detectObjectsInVideo(videoFile: File, onFrameProcessed: (frameIndex: Int, frame: Mat, detectedObjects: List<DetectedObject>) -> Unit) {
        val net = readNetFromDarknet(yoloModelConfigFile.absolutePath, yoloModelWeightsFile.absolutePath)
        val layerNames = net.layerNames
        val outLayerNames = net.unconnectedOutLayers.toList().stream()
                .map { layerNames[it - 1] }
                .collect(Collectors.toList())
        println("Yolo net initialized: outLayerNames = $outLayerNames")

        val videoCapture = VideoCapture(videoFile.absolutePath)
        val nbFrames = try {
            videoCapture.get(Videoio.CAP_PROP_FRAME_COUNT).toInt()
        } catch (e: Exception) {
            Int.MAX_VALUE
        }
        println("Number of frames in video file: $nbFrames")

        var frameWidth = -1
        var frameHeight = -1
        val blobSize = Size(416.0, 416.0) // TODO why 416?
        for (frameIndex in 0 until nbFrames) {
            println("    Processing frame ${frameIndex + 1} / $nbFrames")

            val frame = Mat()
            val grabbed = videoCapture.read(frame)
            if (!grabbed) {
                break
            }

            if (frameWidth == -1 || frameHeight == -1) {
                val size = frame.size()
                frameWidth = size.width.toInt()
                frameHeight = size.height.toInt()
                println("        Frame dimension: width = $frameWidth, height = $frameHeight")
            }

            // Detect objects in the frame
            val blob = blobFromImage(frame, 1 / 255.0, blobSize, Scalar(1.0), true, false) // TODO why Scalar(1.0) ?
            net.setInput(blob)
            val layerOutputs = mutableListOf<Mat>()
            net.forward(layerOutputs, outLayerNames)

            // Extract detected objects
            val detectedObjects = mutableListOf<DetectedObject>()
            for (layerOutput in layerOutputs) {
                val layerOutputSize = layerOutput.size()
                for (rowIndex in 0 until layerOutputSize.height.toInt()) {
                    val detection = IntStream.range(0, layerOutputSize.width.toInt())
                            .mapToDouble { layerOutput[rowIndex, it][0] }
                            .boxed()
                            .collect(Collectors.toList())

                    val scores = detection.subList(5, detection.size - 1).map { it }
                    val classID = scores.indices.maxBy { scores[it] } ?: -1
                    val confidence = scores[classID]

                    if (confidence >= minConfidence) {
                        val objectType = objectTypeByDetectionIndex[classID]

                        val centerX = detection[0] * frameWidth
                        val centerY = detection[1] * frameHeight
                        val width = detection[2] * frameWidth
                        val height = detection[3] * frameHeight
                        val x = centerX - (width / 2)
                        val y = centerY - (height / 2)

                        detectedObjects.add(DetectedObject(
                                Math.round(x).toInt(), Math.round(y).toInt(),
                                Math.round(width).toInt(), Math.round(height).toInt(),
                                objectType, confidence))
                    }
                }
            }
            onFrameProcessed(frameIndex, frame, detectedObjects)
        }

        videoCapture.release()
    }
}