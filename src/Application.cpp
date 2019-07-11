#include <iostream>
#include <vector>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include "utils/logging.hpp"
#include "service/detection/impl/DarknetObjectDetectionServiceImpl.hpp"
#include "service/impl/VideoFrameReaderImpl.hpp"

namespace lg = boost::log;
namespace po = boost::program_options;
namespace pt = boost::property_tree;
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

    // Load configuration
    LOG_INFO(logger) << "Loading the configuration file: " << configurationPath.string();
    pt::ptree propTree;
    pt::ini_parser::read_ini(configurationPath.string(), propTree);
    std::string yoloClassNamesAsString = propTree.get<std::string>("YOLO-model.classnames");
    std::vector<std::string> yoloClassNames;
    boost::split(yoloClassNames, yoloClassNamesAsString, boost::is_any_of(","), boost::token_compress_on);
    fs::path relativeYoloCfgPath(propTree.get<std::string>("YOLO-model.cfgpath"));
    fs::path relativeYoloWeightsPath(propTree.get<std::string>("YOLO-model.weightspath"));
    fs::path rootPath = configurationPath.parent_path();
    fs::path yoloCfgPath = fs::canonical(fs::path(rootPath / relativeYoloCfgPath));
    fs::path yoloWeightsPath = fs::canonical(fs::path(rootPath / relativeYoloWeightsPath));
    LOG_INFO(logger) << "Configuration:";
    LOG_INFO(logger) << "\tYOLO model class names: " << yoloClassNamesAsString;
    LOG_INFO(logger) << "\tYOLO model cfg path: " << yoloCfgPath.string();
    LOG_INFO(logger) << "\tYOLO model weights path: " << yoloWeightsPath.string();

    // Prepare services
    service::DarknetObjectDetectionServiceImpl objectDetectionService;
    service::VideoFrameReaderImpl videoFrameReader(videoPath);

    // Detect objects in the video
    LOG_INFO(logger) << "Detect objects in video...";
    int nbFrames = videoFrameReader.getNbFrames();

    for (int frameIndex = 0; frameIndex < nbFrames; frameIndex++) {
        LOG_INFO(logger) << "Processing the frame " << frameIndex << "...";

        auto frame = videoFrameReader.getFrameAt(frameIndex);
        LOG_INFO(logger) << "frame resolution: " << frame.size();

        auto detectedObjects = objectDetectionService.detectObjectsInImage(frame);
        LOG_INFO(logger) << "nb detected objects: " << detectedObjects.size();
    }
    // TODO

    LOG_INFO(logger) << "Application executed with success!";
    return 0;
}