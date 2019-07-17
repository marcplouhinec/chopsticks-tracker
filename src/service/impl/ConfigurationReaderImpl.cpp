#include <map>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "ConfigurationReaderImpl.hpp"

using namespace model;
using namespace service;
using std::map;
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

vector<DetectedObjectType> ConfigurationReaderImpl::getYoloModelClassEnums() {
    vector<string> classNames = getYoloModelClassNames();

    vector<DetectedObjectType> objectTypes;
    for (int i = 0; i < classNames.size(); i++) {
        auto className = classNames[i];
        auto objectType = DetectedObjectTypeHelper::stringToEnum(className);
        objectTypes.push_back(objectType);
    }

    return objectTypes;
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

float ConfigurationReaderImpl::getObjectDetectionMinTipConfidence() {
    if (!configurationLoaded) {
        loadConfiguration();
    }
    return objectDetectionMinTipConfidence;
}

float ConfigurationReaderImpl::getObjectDetectionMinChopstickConfidence() {
    if (!configurationLoaded) {
        loadConfiguration();
    }
    return objectDetectionMinChopstickConfidence;
}

float ConfigurationReaderImpl::getObjectDetectionMinArmConfidence() {
    if (!configurationLoaded) {
        loadConfiguration();
    }
    return objectDetectionMinArmConfidence;
}

float ConfigurationReaderImpl::getObjectDetectionNmsThreshold() {
    if (!configurationLoaded) {
        loadConfiguration();
    }
    return objectDetectionNmsThreshold;
}

std::string ConfigurationReaderImpl::getObjectDetectionImplementation() {
    if (!configurationLoaded) {
        loadConfiguration();
    }
    return objectDetectionImplementation;
}

fs::path ConfigurationReaderImpl::getObjectDetectionCacheFolderPath() {
    if (!configurationLoaded) {
        loadConfiguration();
    }
    return objectDetectionCacheFolderPath;
}

int ConfigurationReaderImpl::getTrackingNbPastFrameDetectionResultsToKeep() {
    if (!configurationLoaded) {
        loadConfiguration();
    }
    return trackingNbPastFrameDetectionResultsToKeep;
}

int ConfigurationReaderImpl::getTrackingMaxTipMatchingDistanceInPixels() {
    if (!configurationLoaded) {
        loadConfiguration();
    }
    return trackingMaxTipMatchingDistanceInPixels;
}

int ConfigurationReaderImpl::getTrackingNbTipsToUseToDetectCameraMotion() {
    if (!configurationLoaded) {
        loadConfiguration();
    }
    return trackingNbTipsToUseToDetectCameraMotion;
}

fs::path ConfigurationReaderImpl::getRenderingOutputPath() {
    if (!configurationLoaded) {
        loadConfiguration();
    }
    return renderingOutputPath;
}

std::string ConfigurationReaderImpl::getRenderingImplementation() {
    if (!configurationLoaded) {
        loadConfiguration();
    }
    return renderingImplementation;
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

    objectDetectionMinTipConfidence = propTree.get<float>("objectDetection.minTipConfidence");
    objectDetectionMinChopstickConfidence =
        propTree.get<float>("objectDetection.minChopstickConfidence");
    objectDetectionMinArmConfidence = propTree.get<float>("objectDetection.minArmConfidence");
    objectDetectionNmsThreshold = propTree.get<float>("objectDetection.nmsThreshold");
    objectDetectionImplementation = propTree.get<string>("objectDetection.implementation");
    fs::path relativeCacheFolderPath(propTree.get<string>("objectDetection.cacheFolderPath"));
    objectDetectionCacheFolderPath = fs::path(rootPath / relativeCacheFolderPath);

    trackingNbPastFrameDetectionResultsToKeep =
        propTree.get<int>("tracking.nbPastFrameDetectionResultsToKeep");
    trackingMaxTipMatchingDistanceInPixels =
        propTree.get<int>("tracking.maxTipMatchingDistanceInPixels");
    trackingNbTipsToUseToDetectCameraMotion =
        propTree.get<int>("tracking.nbTipsToUseToDetectCameraMotion");

    fs::path relativeRenderingOutputPath(propTree.get<string>("rendering.outputpath"));
    renderingOutputPath = fs::path(rootPath / relativeRenderingOutputPath);
    renderingImplementation = propTree.get<string>("rendering.implementation");

    configurationLoaded = true;

    LOG_INFO(logger) << "Configuration:";
    LOG_INFO(logger) << "\tYOLO model class names: " << yoloClassNamesAsString;
    LOG_INFO(logger) << "\tYOLO model cfg path: " << yoloModelCfgPath.string();
    LOG_INFO(logger) << "\tYOLO model weights path: " << yoloModelWeightsPath.string();
    LOG_INFO(logger) << "\tObject detection min tip confidence: " << objectDetectionMinTipConfidence;
    LOG_INFO(logger) << "\tObject detection min chopstick confidence: "
        << objectDetectionMinChopstickConfidence;
    LOG_INFO(logger) << "\tObject detection min arm confidence: " << objectDetectionMinArmConfidence;
    LOG_INFO(logger) << "\tObject detection NMS threshold: " << objectDetectionNmsThreshold;
    LOG_INFO(logger) << "\tObject detection implementation: " << objectDetectionImplementation;
    LOG_INFO(logger) << "\tObject detection cache folder path: "
        << objectDetectionCacheFolderPath.string();
    LOG_INFO(logger) << "\tTracking nb frame detection results to keep: "
        << trackingNbPastFrameDetectionResultsToKeep;
    LOG_INFO(logger) << "\tTracking maximum tip matching distance (in pixels): "
        << trackingMaxTipMatchingDistanceInPixels;
    LOG_INFO(logger) << "\tTracking nb tips to use to detect camera motion: "
        << trackingNbTipsToUseToDetectCameraMotion;
    LOG_INFO(logger) << "\tRendering output path: " << renderingOutputPath.string();
    LOG_INFO(logger) << "\tRendering implementation: " << renderingImplementation;
}
