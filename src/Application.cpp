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
#include "service/impl/VideoFramePainterDetectedObjectsImpl.hpp"
#include "service/impl/VideoFrameReaderImpl.hpp"
#include "service/impl/VideoFrameWriterMjpgImpl.hpp"
#include "service/impl/VideoFrameWriterMultiJpegImpl.hpp"

using namespace model;
using namespace service;
using std::abs;
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

    // Prepare services
    ConfigurationReaderImpl configurationReader(configurationPath);

    int nbPastFrameDetectionResultsToKeep = configurationReader.getTrackingNbPastFrameDetectionResultsToKeep();
    circular_buffer<FrameDetectionResult> frameDetectionResults(nbPastFrameDetectionResultsToKeep);
    circular_buffer<FrameDetectionResult> compensatedFramesDetectionResults(nbPastFrameDetectionResultsToKeep);
    
    VideoFrameReaderImpl videoFrameReader(videoPath);
    int nbFrames = videoFrameReader.getNbFrames();
    int frameWidth = videoFrameReader.getFrameWidth();
    int frameHeight = videoFrameReader.getFrameHeight();
    int frameMargin = configurationReader.getRenderingVideoFrameMarginsInPixels();
    const cv::Scalar blackColor(0, 0, 0);
    cv::Mat outputFrame(frameHeight + 2 * frameMargin, frameWidth + 2 * frameMargin, CV_8UC3, blackColor);

    string detectionImpl = configurationReader.getObjectDetectionImplementation();
    unique_ptr<ObjectDetector> pInnerObjectDetector{};
    if (detectionImpl.compare("darknet") == 0) {
        pInnerObjectDetector.reset(new ObjectDetectorDarknetImpl(configurationReader, videoFrameReader));
    } else if (detectionImpl.compare("opencvdnn") == 0) {
        pInnerObjectDetector.reset(new ObjectDetectorOpenCvDnnImpl(configurationReader, videoFrameReader));
    }
    ObjectDetector& innerObjectDetector = *pInnerObjectDetector;
    ObjectDetectorCacheImpl objectDetector(configurationReader, innerObjectDetector, videoPath);
    
    TipTrackerImpl tipTracker(configurationReader);

    string renderingImpl = configurationReader.getRenderingImplementation();
    unique_ptr<VideoFrameWriter> pVideoFrameWriter{};
    if (renderingImpl.compare("mjpeg") == 0) {
        pVideoFrameWriter.reset(new VideoFrameWriterMjpgImpl(
            configurationReader,
            videoPath,
            videoFrameReader.getFps(),
            outputFrame.cols,
            outputFrame.rows));
    } else if (renderingImpl.compare("multijpeg") == 0) {
        pVideoFrameWriter.reset(new VideoFrameWriterMultiJpegImpl(configurationReader, videoPath));
    }
    VideoFrameWriter& videoFrameWriter = *pVideoFrameWriter;

    VideoFramePainterDetectedObjectsImpl videoFramePainter(configurationReader, compensatedFramesDetectionResults);

    // Detect and track objects in the video
    LOG_INFO(logger) << "Detect and track objects in the video...";
    int bestMargin = 0;
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
        if (nbDetectionResults >= 2) {
            auto& currFrameObjects = frameDetectionResult.detectedObjects;
            auto& prevFrameObjects = frameDetectionResults[nbDetectionResults - 2].detectedObjects;

            FrameOffset frameOffset =
                tipTracker.computeOffsetToCompensateForCameraMotion(prevFrameObjects, currFrameObjects);
            accumulatedFrameOffset += frameOffset;

            LOG_INFO(logger) << "Camera motion compensated: dx = " << frameOffset.dx
                << ", dy = " << frameOffset.dy;
        }

        // Compensate for camera motion
        vector<DetectedObject> compensatedObjects;
        for (auto& o : frameDetectionResult.detectedObjects) {
            compensatedObjects.push_back(DetectedObject(
                o.x - accumulatedFrameOffset.dx, o.y - accumulatedFrameOffset.dy,
                o.width, o.height, o.objectType, o.confidence));
        }
        compensatedFramesDetectionResults.push_back(FrameDetectionResult(frameIndex, compensatedObjects));

        // Copy the video frame into a bigger one in order compensate for camera motion
        outputFrame.setTo(blackColor);
        int marginLeft = round(frameMargin - accumulatedFrameOffset.dx);
        int marginTop = round(frameMargin - accumulatedFrameOffset.dy);
        frame.copyTo(outputFrame(cv::Rect(marginLeft, marginTop, frame.cols, frame.rows)));

        if (abs(accumulatedFrameOffset.dx) > bestMargin) {
            bestMargin = abs(accumulatedFrameOffset.dx);
        }
        if (abs(accumulatedFrameOffset.dy) > bestMargin) {
            bestMargin = abs(accumulatedFrameOffset.dy);
        }

        videoFramePainter.paintOnFrame(frameIndex, outputFrame);
        LOG_INFO(logger) << "Frame painted.";

        videoFrameWriter.writeFrameAt(frameIndex, outputFrame);
        LOG_INFO(logger) << "Frame written.";
    }

    LOG_INFO(logger) << "Best videoFrameMarginsInPixels: " << bestMargin;
    LOG_INFO(logger) << "Application executed with success!";

    return 0;
}