#include "utils/logging.hpp"
#include "utils/ProgramArgumentsParser.hpp"
#include "ApplicationContext.hpp"

using namespace model;
using namespace service;
using namespace utils;
using std::list;
using std::vector;
namespace lg = boost::log;

int main(int argc, char* argv[]) {
    lg::add_common_attributes();
    lg::sources::severity_logger<lg::trivial::severity_level> logger;

    // Parse arguments
    ProgramArguments programArguments;
    try {
        ProgramArgumentsParser programArgumentsParser;
        programArguments = programArgumentsParser.parse(argc, argv);
    } catch (std::runtime_error e) {
        return 1;
    }
    LOG_INFO(logger) << "Initialization (configuration path = " << programArguments.configurationPath.string()
        << ", video path = " << programArguments.videoPath.string() << ")...";

    // Initialize the application context
    ApplicationContext applicationContext(programArguments.configurationPath, programArguments.videoPath);
    auto& videoProperties = applicationContext.getVideoProperties();
    auto& videoFrameReader = applicationContext.getVideoFrameReader();
    auto& objectDetector = applicationContext.getObjectDetector();
    auto& trackerTip = applicationContext.getTrackerTip();
    auto& trackerChopstick = applicationContext.getTrackerChopstick();
    auto& videoFrameWriter = applicationContext.getVideoFrameWriter();
    auto& videoFramePainterImage = applicationContext.getVideoFramePainterImage();
    auto& videoFramePainterDetectedObjects = applicationContext.getVideoFramePainterDetectedObjects();
    auto& videoFramePainterTrackedObjects = applicationContext.getVideoFramePainterTrackedObjects();

    // Detect and track objects in the video
    LOG_INFO(logger) << "Detect and track objects in the video...";
    FrameOffset accumulatedFrameOffset(0, 0);
    vector<DetectedObject> detectedObjects;
    vector<DetectedObject> prevFrameDetectedObjects;
    list<Tip> tips;
    list<Chopstick> chopsticks;
    cv::Mat outputFrame = videoFrameWriter.buildOutputFrame();

    for (int frameIndex = 0; frameIndex < videoProperties.nbFrames; frameIndex++) {
        LOG_INFO(logger) << "Processing the frame " << frameIndex
            << "/" << (videoProperties.nbFrames - 1) << "...";

        // Read the next frame
        auto frame = videoFrameReader.readFrameAt(frameIndex);

        // Detect the objects in this frame
        prevFrameDetectedObjects = detectedObjects;
        detectedObjects = objectDetector.detectObjectsAt(frameIndex);

        // Find how much we need to compensate for camera motion
        FrameOffset frameOffset(0, 0);
        if (frameIndex >= 1) {
            frameOffset = trackerTip.computeOffsetToCompensateForCameraMotion(
                prevFrameDetectedObjects, detectedObjects);
            accumulatedFrameOffset += frameOffset;
        }

        // Update the tracked tips and chopsticks
        trackerTip.updateTipsWithNewDetectionResult(
            tips, detectedObjects, frameIndex, frameOffset, accumulatedFrameOffset);

        trackerChopstick.updateChopsticksWithNewDetectionResult(
            chopsticks, tips, detectedObjects, accumulatedFrameOffset);

        // Render the detected and tracked objects in an output video frame
        videoFramePainterImage.paintOnFrame(outputFrame, frame, accumulatedFrameOffset);
        videoFramePainterDetectedObjects.paintOnFrame(outputFrame, detectedObjects, accumulatedFrameOffset);
        videoFramePainterTrackedObjects.paintOnFrame(outputFrame, tips, chopsticks, accumulatedFrameOffset);

        videoFrameWriter.writeFrameAt(frameIndex, outputFrame);
    }

    LOG_INFO(logger) << "Application executed with success!";

    return 0;
}