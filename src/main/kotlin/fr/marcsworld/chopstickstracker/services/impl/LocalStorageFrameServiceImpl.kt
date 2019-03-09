package fr.marcsworld.chopstickstracker.services.impl

import com.fasterxml.jackson.module.kotlin.jacksonObjectMapper
import com.fasterxml.jackson.module.kotlin.readValue
import fr.marcsworld.chopstickstracker.model.Configuration
import fr.marcsworld.chopstickstracker.model.DetectedObject
import fr.marcsworld.chopstickstracker.model.Frame
import fr.marcsworld.chopstickstracker.services.FrameService
import java.awt.Image
import java.awt.image.BufferedImage
import java.io.File
import java.util.*
import java.util.stream.Collectors
import javax.imageio.ImageIO

class LocalStorageFrameServiceImpl(
        private val configuration: Configuration
) : FrameService {

    private val mapper = jacksonObjectMapper()

    override fun findAll(): List<Frame> {
        val files = File(configuration.extractedFrameObjectsPath).listFiles()

        return Arrays.stream(files)
                .sorted(Comparator.comparing { file: File -> getFrameIndex(file) })
                .map { file ->
                    val detectedObjects = mapper.readValue<Array<DetectedObject>>(file)
                    Frame(getFrameIndex(file), detectedObjects.toList())
                }
                .collect(Collectors.toList())
    }

    override fun findImageByIndex(frameIndex: Int): BufferedImage {
        val file = File(configuration.extractedFrameImagesPath + "/" + frameIndex + ".jpg")

        if (!file.exists()) {
            throw IllegalArgumentException("Unable to load the frame image with the index: $frameIndex")
        }

        return ImageIO.read(file)
    }

    private fun getFrameIndex(file: File): Int {
        return Integer.parseInt(file.nameWithoutExtension)
    }

}