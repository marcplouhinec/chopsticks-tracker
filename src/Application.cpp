#include <iostream>
#include <memory>
#include <boost/circular_buffer.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "utils/logging.hpp"
#include "service/impl/ConfigurationReaderImpl.hpp"
#include "service/impl/ObjectDetectorCacheImpl.hpp"
#include "service/impl/ObjectDetectorDarknetImpl.hpp"
#include "service/impl/ObjectDetectorOpenCvDnnImpl.hpp"
#include "service/impl/TipTrackerImpl.hpp"
#include "service/impl/ChopstickTrackerImpl.hpp"
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
using boost::circular_buffer;
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
    ConfigurationReaderImpl configurationReader;
    Configuration configuration = configurationReader.read(configurationPath);

    circular_buffer<FrameDetectionResult> frameDetectionResults(2);
    list<Tip> tips;
    list<Chopstick> chopsticks;

    VideoFrameReaderImpl videoFrameReader(videoPath);
    int nbFrames = videoFrameReader.getNbFrames();
    int frameWidth = videoFrameReader.getFrameWidth();
    int frameHeight = videoFrameReader.getFrameHeight();
    int frameMargin = configuration.renderingVideoFrameMarginsInPixels;
    const cv::Scalar blackColor(0, 0, 0);
    cv::Mat outputFrame(frameHeight + 2 * frameMargin, frameWidth + 2 * frameMargin, CV_8UC3, blackColor);

    unique_ptr<ObjectDetector> pInnerObjectDetector{};
    if (configuration.objectDetectionImplementation == "darknet") {
        pInnerObjectDetector.reset(new ObjectDetectorDarknetImpl(configuration, videoFrameReader));
    } else if (configuration.objectDetectionImplementation == "opencvdnn") {
        pInnerObjectDetector.reset(new ObjectDetectorOpenCvDnnImpl(configuration, videoFrameReader));
    }
    ObjectDetector& innerObjectDetector = *pInnerObjectDetector;
    ObjectDetectorCacheImpl objectDetector(configuration, innerObjectDetector, videoPath);
    
    TipTrackerImpl tipTracker(configuration);
    ChopstickTrackerImpl chopstickTracker(configuration);

    unique_ptr<VideoFrameWriter> pVideoFrameWriter{};
    if (configuration.renderingWriterImplementation == "mjpeg") {
        pVideoFrameWriter.reset(new VideoFrameWriterMjpgImpl(
            configuration,
            videoPath,
            videoFrameReader.getFps(),
            outputFrame.cols,
            outputFrame.rows));
    } else if (configuration.renderingWriterImplementation == "multijpeg") {
        pVideoFrameWriter.reset(new VideoFrameWriterMultiJpegImpl(configuration, videoPath));
    }
    VideoFrameWriter& videoFrameWriter = *pVideoFrameWriter;

    vector<unique_ptr<VideoFramePainter>> pVideoFramePainters;
    for (string& renderingPainterImplementation : configuration.renderingPainterImplementations) {
        if (renderingPainterImplementation == "detectedobjects") {
            pVideoFramePainters.push_back(unique_ptr<VideoFramePainter>(
                new VideoFramePainterDetectedObjectsImpl(configuration, frameDetectionResults)));
        } else if (renderingPainterImplementation == "trackedobjects") {
            pVideoFramePainters.push_back(unique_ptr<VideoFramePainter>(
                new VideoFramePainterTrackedObjectsImpl(configuration, tips, chopsticks)));
        }
    }

    // Detect and track objects in the video
    LOG_INFO(logger) << "Detect and track objects in the video...";
    FrameOffset accumulatedFrameOffset(0, 0);

    for (int frameIndex = 0; frameIndex < nbFrames; frameIndex++) {
        LOG_INFO(logger) << "Processing the frame " << frameIndex << "...";

        auto frame = videoFrameReader.readFrameAt(frameIndex);
        LOG_INFO(logger) << "Frame resolution: " << frame.size();

        auto detectedObjects = objectDetector.detectObjectsAt(frameIndex);
        FrameDetectionResult frameDetectionResult(frameIndex, detectedObjects);
        frameDetectionResults.push_back(frameDetectionResult);
        LOG_INFO(logger) << "Nb detected objects: " << detectedObjects.size();

        // Find how much we need to compensate for camera motion
        auto nbDetectionResults = frameDetectionResults.size();
        FrameOffset frameOffset(0, 0);
        if (nbDetectionResults >= 2) {
            auto& prevDetectionResult = frameDetectionResults[nbDetectionResults - 2];

            frameOffset = tipTracker.computeOffsetToCompensateForCameraMotion(
                prevDetectionResult, frameDetectionResult);
            accumulatedFrameOffset += frameOffset;

            LOG_INFO(logger) << "Camera motion compensated: dx = " << frameOffset.dx
                << ", dy = " << frameOffset.dy;
        }

        // Update the tracked tips anc chopsticks
        tipTracker.updateTipsWithNewDetectionResult(
            tips, frameDetectionResult, frameOffset, accumulatedFrameOffset);
        LOG_INFO(logger) << "Nb tracked tips: " << tips.size();

        chopstickTracker.updateChopsticksWithNewDetectionResult(
            chopsticks, tips, frameDetectionResult, accumulatedFrameOffset);
        LOG_INFO(logger) << "Nb tracked chopsticks: " << chopsticks.size();

        // Copy the video frame into a bigger one in order compensate for camera motion
        outputFrame.setTo(blackColor);
        int marginLeft = round(frameMargin - accumulatedFrameOffset.dx);
        int marginTop = round(frameMargin - accumulatedFrameOffset.dy);
        frame.copyTo(outputFrame(cv::Rect(marginLeft, marginTop, frame.cols, frame.rows)));

        for (auto& pPainter : pVideoFramePainters) {
            pPainter->paintOnFrame(frameIndex, outputFrame, accumulatedFrameOffset);
        }
        LOG_INFO(logger) << "Frame painted.";

        videoFrameWriter.writeFrameAt(frameIndex, outputFrame);
        LOG_INFO(logger) << "Frame written.";
    }

    LOG_INFO(logger) << "Application executed with success!";

    return 0;
}