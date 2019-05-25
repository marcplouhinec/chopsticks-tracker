package fr.marcsworld.chopstickstracker.services.detection.impl

import com.fasterxml.jackson.databind.ObjectMapper
import fr.marcsworld.chopstickstracker.model.detection.DetectedObject
import fr.marcsworld.chopstickstracker.model.detection.FrameDetectionResult
import fr.marcsworld.chopstickstracker.services.detection.ObjectDetectionService
import org.opencv.core.Mat
import org.opencv.imgcodecs.Imgcodecs
import org.slf4j.LoggerFactory
import org.springframework.beans.factory.annotation.Value
import org.springframework.stereotype.Service
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream

/**
 * Cache the results of the [YoloObjectDetectionServiceImpl] on the disk in order to avoid reprocessing the
 * same video several times.
 *
 * @author Marc Plouhinec
 */
@Service
class CachedYoloObjectDetectionServiceImpl(

        @Value("\${user.home}")
        val homeFolderFile: File,

        val sourceObjectDetectionService: YoloObjectDetectionServiceImpl

) : ObjectDetectionService {

    companion object {
        val LOGGER = LoggerFactory.getLogger(CachedYoloObjectDetectionServiceImpl::class.java)!!
    }

    override fun detectObjectsInVideo(videoFile: File): Iterable<FrameDetectionResult> {
        val cacheFolderFile = File(homeFolderFile,
                ".chopsticks-tracker/cache/${videoFile.nameWithoutExtension}_${videoFile.lastModified()}")
        if (!cacheFolderFile.exists()) {
            cacheFolderFile.mkdirs()
        }
        val imagesCacheFile = File(cacheFolderFile, "images")
        if (!imagesCacheFile.exists()) {
            imagesCacheFile.mkdir()
        }
        val objectsCacheFile = File(cacheFolderFile, "objects")
        if (!objectsCacheFile.exists()) {
            objectsCacheFile.mkdir()
        }
        LOGGER.info("Using the cache folders '{}' and '{}'.", imagesCacheFile.absolutePath, objectsCacheFile.absolutePath)

        return object : Iterable<FrameDetectionResult> {
            override fun iterator(): Iterator<FrameDetectionResult> {
                return CachedFrameDetectionIterator(
                        videoFile, sourceObjectDetectionService, imagesCacheFile, objectsCacheFile)
            }
        }
    }

    private inner class CachedFrameDetectionIterator(
            private val videoFile: File,
            private val sourceObjectDetectionService: ObjectDetectionService,
            private val imagesCacheFile: File,
            private val objectsCacheFile: File
    ) : Iterator<FrameDetectionResult> {

        private val objectMapper = ObjectMapper()
        private var lazySourceIterator: Iterator<FrameDetectionResult>? = null

        private var total = -1
        private var frameIndex = -1
        private var frameProcessed = true
        private var hasNext = true
        private var frameDetectionResult = FrameDetectionResult(-1, listOf()) { Mat() }

        override fun hasNext(): Boolean {
            if (frameProcessed) {
                frameProcessed = false
                frameIndex += 1
                hasNext = false

                // Read the total if necessary
                if (total == -1) {
                    val totalFile = File(objectsCacheFile, "total.json")
                    total = if (totalFile.exists()) {
                        FileInputStream(totalFile).use { objectMapper.readValue(it, Int::class.java) }
                    } else {
                        Int.MAX_VALUE
                    }
                }

                // Stop if we have reached the total
                if (frameIndex >= total) {
                    return false
                }

                // Find the data from the cache
                val imageFile = File(imagesCacheFile, "$frameIndex.jpg")
                val objectJsonFile = File(objectsCacheFile, "$frameIndex.json")
                if (imageFile.exists() && objectJsonFile.exists()) {
                    val detectedObjects = FileInputStream(objectJsonFile).use {
                        objectMapper.readValue(it, Array<DetectedObject>::class.java)
                    }
                    frameDetectionResult = FrameDetectionResult(frameIndex, detectedObjects.toList()) {
                        Imgcodecs.imread(imageFile.absolutePath)
                    }
                    hasNext = true
                } else {
                    // Find the data from the source iterator
                    do {
                        val sourceIterator = getSourceIterator()
                        hasNext = sourceIterator.hasNext()
                        if (hasNext) {
                            frameDetectionResult = sourceIterator.next()
                        }
                    } while (hasNext && frameDetectionResult.frameIndex != frameIndex)

                    // Cache this result
                    if (hasNext) {
                        LOGGER.info("Cache the frame {}", frameIndex)
                        val objectsJson = objectMapper.writeValueAsString(frameDetectionResult.detectedObjects)
                        objectJsonFile.bufferedWriter().use { out -> out.write(objectsJson) }
                        Imgcodecs.imwrite(imageFile.absolutePath, frameDetectionResult.frameImageProvider())
                    } else {
                        LOGGER.info("Store the total number of frames: {}", frameIndex)
                        val totalFile = File(objectsCacheFile, "total.json")
                        FileOutputStream(totalFile).use { objectMapper.writeValue(it, frameIndex) }
                    }
                }
            }
            return hasNext
        }

        override fun next(): FrameDetectionResult {
            frameProcessed = true
            return frameDetectionResult
        }

        private fun getSourceIterator(): Iterator<FrameDetectionResult> {
            if (lazySourceIterator == null) {
                lazySourceIterator = sourceObjectDetectionService.detectObjectsInVideo(videoFile).iterator()
            }
            return lazySourceIterator ?: throw IllegalStateException("Unable to get the source iterator.")
        }
    }
}