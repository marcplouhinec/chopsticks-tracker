#include <iostream>
#include <memory>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "utils/logging.hpp"
#include "service/impl/ConfigurationReaderImpl.hpp"
#include "service/impl/ObjectDetectorCacheImpl.hpp"
#include "service/impl/ObjectDetectorDarknetImpl.hpp"
#include "service/impl/ObjectDetectorOpenCvDnnImpl.hpp"
#include "service/impl/TipTrackerImpl.hpp"
#include "service/impl/ChopstickTrackerImpl.hpp"
#include "service/impl/VideoFramePainterImageImpl.hpp"
#include "service/impl/VideoFramePainterDetectedObjectsImpl.hpp"
#include "service/impl/VideoFramePainterTrackedObjectsImpl.hpp"
#include "service/impl/VideoFrameReaderImpl.hpp"
#include "service/impl/VideoFrameWriterMjpgImpl.hpp"
#include "service/impl/VideoFrameWriterMultiJpegImpl.hpp"

using namespace model;
using namespace service;
using std::abs;
using std::list;
using std::string;
using std::unique_ptr;
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

    // Initialize services
    const ConfigurationReaderImpl configurationReader;
    const Configuration configuration = configurationReader.read(configurationPath);

    list<Tip> tips;
    list<Chopstick> chopsticks;

    VideoFrameReaderImpl videoFrameReader(videoPath);
    const VideoProperties videoProperties = videoFrameReader.getVideoProperties();

    unique_ptr<ObjectDetector> pInnerObjectDetector{};
    if (configuration.objectDetectionImplementation == "darknet") {
        pInnerObjectDetector.reset(new ObjectDetectorDarknetImpl(
            configuration, videoFrameReader, videoProperties));
    } else if (configuration.objectDetectionImplementation == "opencvdnn") {
        pInnerObjectDetector.reset(new ObjectDetectorOpenCvDnnImpl(
            configuration, videoFrameReader, videoProperties));
    }
    ObjectDetector& innerObjectDetector = *pInnerObjectDetector;
    ObjectDetectorCacheImpl objectDetector(configuration, innerObjectDetector, videoPath);
    
    const TipTrackerImpl tipTracker(configuration);
    const ChopstickTrackerImpl chopstickTracker(configuration);

    unique_ptr<VideoFrameWriter> pVideoFrameWriter{};
    if (configuration.renderingWriterImplementation == "mjpeg") {
        pVideoFrameWriter.reset(new VideoFrameWriterMjpgImpl(configuration, videoPath, videoProperties));
    } else if (configuration.renderingWriterImplementation == "multijpeg") {
        pVideoFrameWriter.reset(new VideoFrameWriterMultiJpegImpl(configuration, videoPath, videoProperties));
    }
    VideoFrameWriter& videoFrameWriter = *pVideoFrameWriter;

    const VideoFramePainterImageImpl videoFramePainterImage(configuration);
    const VideoFramePainterDetectedObjectsImpl videoFramePainterDetectedObjects(configuration);
    const VideoFramePainterTrackedObjectsImpl videoFramePainterTrackedObjects(configuration);

    // Detect and track objects in the video
    LOG_INFO(logger) << "Detect and track objects in the video...";
    FrameOffset accumulatedFrameOffset(0, 0);
    vector<DetectedObject> detectedObjects;
    vector<DetectedObject> prevFrameDetectedObjects;
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
            frameOffset = tipTracker.computeOffsetToCompensateForCameraMotion(
                prevFrameDetectedObjects, detectedObjects);
            accumulatedFrameOffset += frameOffset;

            LOG_INFO(logger) << "Camera motion compensated: dx = " << frameOffset.dx
                << ", dy = " << frameOffset.dy;
        }

        // Update the tracked tips anc chopsticks
        tipTracker.updateTipsWithNewDetectionResult(
            tips, detectedObjects, frameIndex, frameOffset, accumulatedFrameOffset);
        LOG_INFO(logger) << "Nb tracked tips: " << tips.size();

        chopstickTracker.updateChopsticksWithNewDetectionResult(
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