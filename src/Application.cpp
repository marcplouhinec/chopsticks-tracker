#include <iostream>
#include <vector>
#include <boost/move/utility_core.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include "service/detection/impl/DarknetObjectDetectionServiceImpl.hpp"

namespace lg = boost::log;
namespace po = boost::program_options;
namespace pt = boost::property_tree;
namespace fs = boost::filesystem;

int main(int argc, char* argv[]) {
    lg::add_common_attributes();
    lg::sources::logger logger;

    // Parse arguments
    po::options_description programDesc("Allowed options");
    programDesc.add_options()
        ("help", "produce help message")
        ("config-path", po::value<std::string>(), "path to the config.ini file");
    
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
        std::cerr << "--config-path not set.\n";
        return 1;
    }

    // Load configuration
    BOOST_LOG(logger) << "Loading the configuration file: " << configurationPath.string();
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
    BOOST_LOG(logger) << "Configuration:";
    BOOST_LOG(logger) << "\tYOLO model class names: " << yoloClassNamesAsString;
    BOOST_LOG(logger) << "\tYOLO model cfg path: " << yoloCfgPath.string();
    BOOST_LOG(logger) << "\tYOLO model weights path: " << yoloWeightsPath.string();

    // Build the application context
    BOOST_LOG(logger) << "Building the application context...";
    service::DarknetObjectDetectionServiceImpl objectDetectionService;

    // Detect objects in the video
    BOOST_LOG(logger) << "Detect objects in video...";
    std::string videoFilePath = "";
    auto frameDetectionResultRange = objectDetectionService.detectObjectsInVideo(videoFilePath);

    for (auto frameDetectionResult : frameDetectionResultRange) {
        BOOST_LOG(logger) << "Process frame " << frameDetectionResult.frameIndex << "...";
        // TODO
    }

    BOOST_LOG(logger) << "Application executed with success!";
    return 0;
}