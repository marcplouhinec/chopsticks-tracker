#include <iostream>
#include <memory>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "utils/logging.hpp"
#include "service/impl/ConfigurationReaderImpl.hpp"
#include "service/impl/VideoFrameReaderImpl.hpp"
#include "service/impl/ObjectDetectorDarknetImpl.hpp"
#include "service/impl/ObjectDetectorOpenCvDnnImpl.hpp"
#include "service/impl/ObjectDetectorCacheImpl.hpp"
#include "service/impl/VideoFrameWriterMjpgImpl.hpp"
#include "service/impl/VideoFrameWriterMultiJpegImpl.hpp"

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
        ("config-path", po::value<std::string>(), "path to the config.ini file")
        ("video-path", po::value<std::string>(), "path to the video file");
    
    po::variables_map varsMap;
    po::store(po::parse_command_line(argc, argv, programDesc), varsMap);
    po::notify(varsMap);

    if (varsMap.count("help")) {
        std::cout << programDesc << "\n";
        return 1;
    }

    fs::path configurationPath;
    if (varsMap.count("config-path")) {
        std::string relativeConfigurationPath = varsMap["config-path"].as<std::string>();
        configurationPath = fs::canonical(fs::path(relativeConfigurationPath));
    } else {
        std::cerr << "--config-path not set. See --help for more info.\n";
        return 1;
    }

    fs::path videoPath;
    if (varsMap.count("video-path")) {
        std::string relativeVideoPath = varsMap["video-path"].as<std::string>();
        videoPath = fs::canonical(fs::path(relativeVideoPath));
    } else {
        std::cerr << "--video-path not set. See --help for more info.\n";
        return 1;
    }

    // Prepare services
    service::ConfigurationReaderImpl configurationReader(configurationPath);
    service::VideoFrameReaderImpl videoFrameReader(videoPath);

    std::string detectionImpl = configurationReader.getObjectDetectionImplementation();
    std::unique_ptr<service::ObjectDetector> pInnerObjectDetector{};
    if (detectionImpl.compare("darknet") == 0) {
        pInnerObjectDetector = std::unique_ptr<service::ObjectDetector>(
            new service::ObjectDetectorDarknetImpl(configurationReader, videoFrameReader));
    } else if (detectionImpl.compare("opencvdnn") == 0) {
        pInnerObjectDetector = std::unique_ptr<service::ObjectDetector>(
            new service::ObjectDetectorOpenCvDnnImpl(configurationReader, videoFrameReader));
    }
    service::ObjectDetector& innerObjectDetector = *pInnerObjectDetector;
    service::ObjectDetectorCacheImpl objectDetector(
        configurationReader, innerObjectDetector, videoPath);
    
    std::string renderingImpl = configurationReader.getRenderingImplementation();
    std::unique_ptr<service::VideoFrameWriter> pVideoFrameWriter{};
    if (renderingImpl.compare("mjpeg") == 0) {
        pVideoFrameWriter = std::unique_ptr<service::VideoFrameWriter>(
            new service::VideoFrameWriterMjpgImpl(
                configurationReader,
                videoPath,
                videoFrameReader.getFps(),
                videoFrameReader.getFrameWidth(),
                videoFrameReader.getFrameHeight()));
    } else if (renderingImpl.compare("multijpeg") == 0) {
        pVideoFrameWriter = std::unique_ptr<service::VideoFrameWriter>(
            new service::VideoFrameWriterMultiJpegImpl(configurationReader, videoPath));
    }
    service::VideoFrameWriter& videoFrameWriter = *pVideoFrameWriter;

    // Detect objects in the video
    LOG_INFO(logger) << "Detect objects in video...";
    int nbFrames = videoFrameReader.getNbFrames();

    for (int frameIndex = 0; frameIndex < nbFrames; frameIndex++) {
        LOG_INFO(logger) << "Processing the frame " << frameIndex << "...";

        auto frame = videoFrameReader.readFrameAt(frameIndex);
        LOG_INFO(logger) << "frame resolution: " << frame.size();

        auto detectedObjects = objectDetector.detectObjectsAt(frameIndex);
        LOG_INFO(logger) << "nb detected objects: " << detectedObjects.size();

        videoFrameWriter.writeFrameAt(frameIndex, frame);
        LOG_INFO(logger) << "Frame written.";
    }
    // TODO

    LOG_INFO(logger) << "Application executed with success!";

    return 0;
}