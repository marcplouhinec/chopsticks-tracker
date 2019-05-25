package fr.marcsworld.chopstickstracker.services.tracking

import fr.marcsworld.chopstickstracker.model.Chopstick
import fr.marcsworld.chopstickstracker.model.Frame
import fr.marcsworld.chopstickstracker.model.Tip

/**
 * Track tips and chopsticks in a video.
 *
 * @author Marc Plouhinec
 */
interface VideoTrackingService {

    /**
     * Filter all the objects of the frame that have too low confidence.
     */
    fun removeUnreliableDetectedObjects(frames: List<Frame>): List<Frame>

    /**
     * Because the camera moves during the video, this function recompute
     * the coordinates of each detected object relatively to the first frame of the video.
     *
     * In addition, the returned frames contains the coordinates of their corresponding images
     * relatively to the first frame of the video.
     */
    fun compensateCameraMotion(frames: List<Frame>): List<Frame>

    /**
     * Track the tips in the given frames.
     */
    fun findAllTips(frames: List<Frame>): List<Tip>

    /**
     * Link the tips into chopsticks.
     */
    fun findAllChopsticks(frames: List<Frame>, tips: List<Tip>): List<Chopstick>
}