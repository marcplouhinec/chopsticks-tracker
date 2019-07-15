#include <iostream>
#include <memory>
#include <boost/circular_buffer.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "utils/logging.hpp"
#include "service/impl/ConfigurationReaderImpl.hpp"
#include "service/impl/VideoFrameReaderImpl.hpp"
#include "service/impl/ObjectDetectorDarknetImpl.hpp"
#include "service/impl/ObjectDetectorOpenCvDnnImpl.hpp"
#include "service/impl/ObjectDetectorCacheImpl.hpp"
#include "service/impl/VideoFrameWriterMjpgImpl.hpp"
#include "service/impl/VideoFrameWriterMultiJpegImpl.hpp"
#include "service/impl/VideoFramePainterDetectedObjectsImpl.hpp"

using namespace model;
using namespace service;
using std::string;
using std::unique_ptr;
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
    VideoFrameReaderImpl videoFrameReader(videoPath);

    string detectionImpl = configurationReader.getObjectDetectionImplementation();
    unique_ptr<ObjectDetector> pInnerObjectDetector{};
    if (detectionImpl.compare("darknet") == 0) {
        pInnerObjectDetector.reset(new ObjectDetectorDarknetImpl(configurationReader, videoFrameReader));
    } else if (detectionImpl.compare("opencvdnn") == 0) {
        pInnerObjectDetector.reset(new ObjectDetectorOpenCvDnnImpl(configurationReader, videoFrameReader));
    }
    ObjectDetector& innerObjectDetector = *pInnerObjectDetector;
    ObjectDetectorCacheImpl objectDetector(configurationReader, innerObjectDetector, videoPath);
    
    string renderingImpl = configurationReader.getRenderingImplementation();
    unique_ptr<VideoFrameWriter> pVideoFrameWriter{};
    if (renderingImpl.compare("mjpeg") == 0) {
        pVideoFrameWriter.reset(new VideoFrameWriterMjpgImpl(
            configurationReader,
            videoPath,
            videoFrameReader.getFps(),
            videoFrameReader.getFrameWidth(),
            videoFrameReader.getFrameHeight()));
    } else if (renderingImpl.compare("multijpeg") == 0) {
        pVideoFrameWriter.reset(new VideoFrameWriterMultiJpegImpl(configurationReader, videoPath));
    }
    VideoFrameWriter& videoFrameWriter = *pVideoFrameWriter;

    int nbPastFrameDetectionResultsToKeep = configurationReader.getTrackingNbPastFrameDetectionResultsToKeep();
    circular_buffer<FrameDetectionResult> frameDetectionResults(nbPastFrameDetectionResultsToKeep);
    VideoFramePainterDetectedObjectsImpl videoFramePainter(frameDetectionResults);

    // Detect objects in the video
    LOG_INFO(logger) << "Detect objects in video...";
    int nbFrames = videoFrameReader.getNbFrames();

    for (int frameIndex = 0; frameIndex < nbFrames; frameIndex++) {
        LOG_INFO(logger) << "Processing the frame " << frameIndex << "...";

        auto frame = videoFrameReader.readFrameAt(frameIndex);
        LOG_INFO(logger) << "frame resolution: " << frame.size();

        auto detectedObjects = objectDetector.detectObjectsAt(frameIndex);
        frameDetectionResults.push_back(FrameDetectionResult(frameIndex, detectedObjects));
        LOG_INFO(logger) << "nb detected objects: " << detectedObjects.size();

        videoFramePainter.paintOnFrame(frameIndex, frame);
        LOG_INFO(logger) << "Frame painted.";

        videoFrameWriter.writeFrameAt(frameIndex, frame);
        LOG_INFO(logger) << "Frame written.";
    }
    // TODO

    LOG_INFO(logger) << "Application executed with success!";

    return 0;
}