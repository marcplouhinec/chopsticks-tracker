#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "utils/logging.hpp"
#include "ApplicationContext.hpp"

using namespace model;
using namespace service;
using std::list;
using std::string;
using std::vector;
namespace lg = boost::log;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int argc, char* argv[]) {
    lg::add_common_attributes();
    lg::sources::severity_logger<lg::trivial::severity_level> logger;

    // Parse arguments
    po::options_description programDesc("Allowed options");
    programDesc.add_options()
        ("help", "produce help message")
        ("config-path", po::value<string>(), "path to the config.ini file")
        ("video-path", po::value<string>(), "path to the video file");
    
    po::variables_map varsMap;
    po::store(po::parse_command_line(argc, argv, programDesc), varsMap);
    po::notify(varsMap);

    if (varsMap.count("help")) {
        std::cout << programDesc << "\n";
        return 1;
    }

    fs::path configurationPath;
    if (varsMap.count("config-path")) {
        string relativeConfigurationPath = varsMap["config-path"].as<string>();
        configurationPath = fs::canonical(fs::path(relativeConfigurationPath));
    } else {
        std::cerr << "--config-path not set. See --help for more info.\n";
        return 1;
    }

    fs::path videoPath;
    if (varsMap.count("video-path")) {
        string relativeVideoPath = varsMap["video-path"].as<string>();
        videoPath = fs::canonical(fs::path(relativeVideoPath));
    } else {
        std::cerr << "--video-path not set. See --help for more info.\n";
        return 1;
    }

    // Initialize the application context
    ApplicationContext applicationContext(configurationPath, videoPath);
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
        LOG_INFO(logger) << "Processing the frame " << frameIndex << "...";

        auto frame = videoFrameReader.readFrameAt(frameIndex);
        LOG_INFO(logger) << "Frame resolution: " << frame.size();

        prevFrameDetectedObjects = detectedObjects;
        detectedObjects = objectDetector.detectObjectsAt(frameIndex);
        LOG_INFO(logger) << "Nb detected objects: " << detectedObjects.size();

        // Find how much we need to compensate for camera motion
        FrameOffset frameOffset(0, 0);
        if (frameIndex >= 1) {
            frameOffset = trackerTip.computeOffsetToCompensateForCameraMotion(
                prevFrameDetectedObjects, detectedObjects);
            accumulatedFrameOffset += frameOffset;

            LOG_INFO(logger) << "Camera motion compensated: dx = " << frameOffset.dx
                << ", dy = " << frameOffset.dy;
        }

        // Update the tracked tips anc chopsticks
        trackerTip.updateTipsWithNewDetectionResult(
            tips, detectedObjects, frameIndex, frameOffset, accumulatedFrameOffset);
        LOG_INFO(logger) << "Nb tracked tips: " << tips.size();

        trackerChopstick.updateChopsticksWithNewDetectionResult(
            chopsticks, tips, detectedObjects, accumulatedFrameOffset);
        LOG_INFO(logger) << "Nb tracked chopsticks: " << chopsticks.size();

        videoFramePainterImage.paintOnFrame(outputFrame, frame, accumulatedFrameOffset);
        videoFramePainterDetectedObjects.paintOnFrame(outputFrame, detectedObjects, accumulatedFrameOffset);
        videoFramePainterTrackedObjects.paintOnFrame(outputFrame, tips, chopsticks, accumulatedFrameOffset);
        LOG_INFO(logger) << "Frame painted.";

        videoFrameWriter.writeFrameAt(frameIndex, outputFrame);
        LOG_INFO(logger) << "Frame written.";
    }

    LOG_INFO(logger) << "Application executed with success!";

    return 0;
}