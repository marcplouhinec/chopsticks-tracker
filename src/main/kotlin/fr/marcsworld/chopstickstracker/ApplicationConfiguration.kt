package fr.marcsworld.chopstickstracker

import org.bytedeco.javacpp.Loader
import org.bytedeco.opencv.opencv_java
import org.springframework.context.annotation.ComponentScan
import org.springframework.context.annotation.Configuration
import org.springframework.context.annotation.PropertySource
import javax.annotation.PostConstruct

/**
 * Spring configuration class.
 *
 * @author Marc Plouhinec
 */
@Configuration
@PropertySource("classpath:application.properties")
@ComponentScan("fr.marcsworld.chopstickstracker.services")
open class ApplicationConfiguration {

    @PostConstruct
    private fun loadOpenCV() {
        Loader.load(opencv_java::class.java)
    }

}