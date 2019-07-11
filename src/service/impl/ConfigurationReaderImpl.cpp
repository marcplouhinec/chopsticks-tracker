#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "ConfigurationReaderImpl.hpp"

using namespace service;
using std::string;
using std::vector;
namespace fs = boost::filesystem;
namespace lg = boost::log;
namespace pt = boost::property_tree;

lg::sources::severity_logger<lg::trivial::severity_level> logger;

vector<string> ConfigurationReaderImpl::getYoloModelClassNames() {
    if (!configurationLoaded) {
        loadConfiguration();
    }
    return yoloModelClassNames;
}

fs::path ConfigurationReaderImpl::getYoloModelCfgPath() {
    if (!configurationLoaded) {
        loadConfiguration();
    }
    return yoloModelCfgPath;
}

fs::path ConfigurationReaderImpl::getYoloModelWeightsPath() {
    if (!configurationLoaded) {
        loadConfiguration();
    }
    return yoloModelWeightsPath;
}

float ConfigurationReaderImpl::getObjectDetectionMinConfidence() {
    if (!configurationLoaded) {
        loadConfiguration();
    }
    return objectDetectionMinConfidence;
}

float ConfigurationReaderImpl::getObjectDetectionNmsThreshold() {
    if (!configurationLoaded) {
        loadConfiguration();
    }
    return objectDetectionNmsThreshold;
}

void ConfigurationReaderImpl::loadConfiguration() {
    LOG_INFO(logger) << "Loading the configuration file: " << configurationPath.string();

    pt::ptree propTree;
    pt::ini_parser::read_ini(configurationPath.string(), propTree);

    string yoloClassNamesAsString = propTree.get<string>("yoloModel.classnames");
    boost::split(
        yoloModelClassNames,
        yoloClassNamesAsString,
        boost::is_any_of(","),
        boost::token_compress_on);

    fs::path relativeYoloCfgPath(propTree.get<string>("yoloModel.cfgpath"));
    fs::path relativeYoloWeightsPath(propTree.get<string>("yoloModel.weightspath"));
    fs::path rootPath = configurationPath.parent_path();
    yoloModelCfgPath = fs::canonical(fs::path(rootPath / relativeYoloCfgPath));
    yoloModelWeightsPath = fs::canonical(fs::path(rootPath / relativeYoloWeightsPath));

    objectDetectionMinConfidence = propTree.get<float>("objectDetection.minConfidence");
    objectDetectionNmsThreshold = propTree.get<float>("objectDetection.nmsThreshold");

    configurationLoaded = true;

    LOG_INFO(logger) << "Configuration:";
    LOG_INFO(logger) << "\tYOLO model class names: " << yoloClassNamesAsString;
    LOG_INFO(logger) << "\tYOLO model cfg path: " << yoloModelCfgPath.string();
    LOG_INFO(logger) << "\tYOLO model weights path: " << yoloModelWeightsPath.string();
    LOG_INFO(logger) << "\tObject detection min confidence: " << objectDetectionMinConfidence;
    LOG_INFO(logger) << "\tObject detection NMS threshold: " << objectDetectionNmsThreshold;
}
