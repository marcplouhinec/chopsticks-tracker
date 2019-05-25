package fr.marcsworld.chopstickstracker.services.detection.impl

import com.fasterxml.jackson.databind.ObjectMapper
import fr.marcsworld.chopstickstracker.model.detection.DetectedObject
import fr.marcsworld.chopstickstracker.model.detection.DetectedObjectType
import fr.marcsworld.chopstickstracker.model.detection.FrameDetectionResult
import fr.marcsworld.chopstickstracker.services.detection.ObjectDetectionService
import org.ini4j.Ini
import org.opencv.core.Mat
import org.opencv.core.Scalar
import org.opencv.core.Size
import org.opencv.dnn.Dnn
import org.opencv.dnn.Net
import org.opencv.videoio.VideoCapture
import org.opencv.videoio.Videoio
import org.slf4j.LoggerFactory
import org.springframework.beans.factory.annotation.Value
import org.springframework.core.io.Resource
import org.springframework.stereotype.Service
import java.io.File
import java.io.FileOutputStream
import java.util.stream.Collectors
import java.util.stream.IntStream

/**
 * Implementation of [ObjectDetectionService] by using the YOLO model.
 *
 * @author Marc Plouhinec
 */
@Service
class YoloObjectDetectionServiceImpl(

        @Value("classpath:detection/yolo-model/yolov3.weights")
        val modelWeightsResource: Resource,

        @Value("classpath:detection/yolo-model/yolov3.cfg")
        val modelConfigResource: Resource,

        @Value("classpath:detection/yolo-model/class-names.json")
        val modelClassNamesResource: Resource,

        @Value("\${detection.YoloObjectDetection.minConfidence}")
        val minConfidence: Double = 0.1

) : ObjectDetectionService {

    companion object {
        val LOGGER = LoggerFactory.getLogger(YoloObjectDetectionServiceImpl::class.java)!!
    }

    override fun detectObjectsInVideo(videoFile: File): Iterable<FrameDetectionResult> {
        return object : Iterable<FrameDetectionResult> {
            override fun iterator(): Iterator<FrameDetectionResult> {
                return FrameDetectionIterator(videoFile)
            }
        }
    }

    private inner class FrameDetectionIterator(videoFile: File) : Iterator<FrameDetectionResult> {
        private val net: Net
        private val outLayerNames: List<String>
        private val modelObjectTypes: List<DetectedObjectType>
        private val videoCapture: VideoCapture
        private val blobSize: Size
        private val nbFrames: Int

        private var frameIndex = -1
        private var frameWidth = -1
        private var frameHeight = -1
        private val frame = Mat()
        private var frameProcessed = true
        private var frameGrabbed = true

        init {
            // Load the neural network model
            net = Dnn.readNetFromDarknet(
                    getResourceFilePath(modelConfigResource), getResourceFilePath(modelWeightsResource))
            val layerNames = net.layerNames
            outLayerNames = net.unconnectedOutLayers.toList().stream()
                    .map { layerNames[it - 1] }
                    .collect(Collectors.toList())
            modelObjectTypes = modelClassNamesResource.inputStream.use {
                ObjectMapper().readValue(it, Array<String>::class.java)
            }.map { DetectedObjectType.valueOf(it) }
            val modelConfigIni = modelConfigResource.inputStream.use { Ini(it) }
            val netWidth = modelConfigIni.get("net", "width", Int::class.java)
            val netHeight = modelConfigIni.get("net", "height", Int::class.java)
            blobSize = Size(netWidth.toDouble(), netHeight.toDouble())
            LOGGER.info("Yolo net initialized: outLayerNames = {}, modelClassNames = {}, netWidth = {}, netHeight = {}",
                    outLayerNames, modelObjectTypes, netWidth, netHeight)

            // Load the video
            videoCapture = VideoCapture(videoFile.absolutePath)
            nbFrames = try {
                videoCapture.get(Videoio.CAP_PROP_FRAME_COUNT).toInt()
            } catch (e: Exception) {
                Int.MAX_VALUE
            }
            LOGGER.info("Number of frames in video file: {}", nbFrames)
        }

        override fun hasNext(): Boolean {
            if (frameProcessed) {
                frameProcessed = false

                frameIndex += 1
                frameGrabbed = videoCapture.read(frame)

                if (!frameGrabbed) {
                    videoCapture.release()
                }
            }

            return frameGrabbed
        }

        override fun next(): FrameDetectionResult {
            LOGGER.info("Processing frame {} / {}", frameIndex + 1, nbFrames)
            frameProcessed = true

            if (frameWidth == -1 || frameHeight == -1) {
                val size = frame.size()
                frameWidth = size.width.toInt()
                frameHeight = size.height.toInt()
                LOGGER.info("Frame dimension: width = {}, height = {}", frameWidth, frameHeight)
            }

            // Detect objects in the frame
            val blob = Dnn.blobFromImage(frame, 1 / 255.0, blobSize, Scalar(1.0), true, false) // TODO why Scalar(1.0) ?
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
                        val objectType = modelObjectTypes[classID]

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
            return FrameDetectionResult(frameIndex, detectedObjects) { frame }
        }

        private fun getResourceFilePath(resource: Resource): String {
            return if (resource.isFile) {
                resource.file.absolutePath
            } else {
                val file = File.createTempFile(resource.filename, "")
                resource.inputStream.use { input ->
                    FileOutputStream(file).use { out -> input.copyTo(out) }
                }
                file.absolutePath
            }
        }
    }
}