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
    loadConfigurationIfNecessary();
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
    loadConfigurationIfNecessary();
    return yoloModelCfgPath;
}

fs::path ConfigurationReaderImpl::getYoloModelWeightsPath() {
    loadConfigurationIfNecessary();
    return yoloModelWeightsPath;
}

float ConfigurationReaderImpl::getObjectDetectionMinTipConfidence() {
    loadConfigurationIfNecessary();
    return objectDetectionMinTipConfidence;
}

float ConfigurationReaderImpl::getObjectDetectionMinChopstickConfidence() {
    loadConfigurationIfNecessary();
    return objectDetectionMinChopstickConfidence;
}

float ConfigurationReaderImpl::getObjectDetectionMinArmConfidence() {
    loadConfigurationIfNecessary();
    return objectDetectionMinArmConfidence;
}

float ConfigurationReaderImpl::getObjectDetectionNmsThreshold() {
    loadConfigurationIfNecessary();
    return objectDetectionNmsThreshold;
}

std::string ConfigurationReaderImpl::getObjectDetectionImplementation() {
    loadConfigurationIfNecessary();
    return objectDetectionImplementation;
}

fs::path ConfigurationReaderImpl::getObjectDetectionCacheFolderPath() {
    loadConfigurationIfNecessary();
    return objectDetectionCacheFolderPath;
}

int ConfigurationReaderImpl::getTrackingMaxTipMatchingDistanceInPixels() {
    loadConfigurationIfNecessary();
    return trackingMaxTipMatchingDistanceInPixels;
}

int ConfigurationReaderImpl::getTrackingNbTipsToUseToDetectCameraMotion() {
    loadConfigurationIfNecessary();
    return trackingNbTipsToUseToDetectCameraMotion;
}

int ConfigurationReaderImpl::getTrackingNbDetectionsToComputeAverageTipPositionAndSize() {
    loadConfigurationIfNecessary();
    return trackingNbDetectionsToComputeAverageTipPositionAndSize;
}

int ConfigurationReaderImpl::getTrackingMinMatchingDistanceWithAnyObjectToConsiderTipNotHiddenByArm() {
    loadConfigurationIfNecessary();
    return trackingMinMatchingDistanceWithAnyObjectToConsiderTipNotHiddenByArm;
}

int ConfigurationReaderImpl::getTrackingMaxFramesAfterWhichATipIsConsideredLost() {
    loadConfigurationIfNecessary();
    return trackingMaxFramesAfterWhichATipIsConsideredLost;
}

int ConfigurationReaderImpl::getTrackingMinDistanceToConsiderNewTipAsTheSameAsAnExistingOne() {
    loadConfigurationIfNecessary();
    return trackingMinDistanceToConsiderNewTipAsTheSameAsAnExistingOne;
}

int ConfigurationReaderImpl::getTrackingMinChopstickLengthInPixels() {
    loadConfigurationIfNecessary();
    return trackingMinChopstickLengthInPixels;
}

int ConfigurationReaderImpl::getTrackingMaxChopstickLengthInPixels() {
    loadConfigurationIfNecessary();
    return trackingMaxChopstickLengthInPixels;
}

double ConfigurationReaderImpl::getTrackingMinIOUToConsiderTwoTipsAsAChopstick() {
    loadConfigurationIfNecessary();
    return trackingMinIOUToConsiderTwoTipsAsAChopstick;
}

int ConfigurationReaderImpl::getTrackingMaxFramesAfterWhichAChopstickIsConsideredLost() {
    loadConfigurationIfNecessary();
    return trackingMaxFramesAfterWhichAChopstickIsConsideredLost;
}

fs::path ConfigurationReaderImpl::getRenderingOutputPath() {
    loadConfigurationIfNecessary();
    return renderingOutputPath;
}

std::vector<std::string> ConfigurationReaderImpl::getRenderingPainterImplementations() {
    loadConfigurationIfNecessary();
    return renderingPainterImplementations;
}

bool ConfigurationReaderImpl::getRenderingDetectedObjectsPainterShowTips() {
    loadConfigurationIfNecessary();
    return renderingDetectedObjectsPainterShowTips;
}

bool ConfigurationReaderImpl::getRenderingDetectedObjectsPainterShowChopsticks() {
    loadConfigurationIfNecessary();
    return renderingDetectedObjectsPainterShowChopsticks;
}

bool ConfigurationReaderImpl::getRenderingDetectedObjectsPainterShowArms() {
    loadConfigurationIfNecessary();
    return renderingDetectedObjectsPainterShowArms;
}

std::string ConfigurationReaderImpl::getRenderingWriterImplementation() {
    loadConfigurationIfNecessary();
    return renderingWriterImplementation;
}

int ConfigurationReaderImpl::getRenderingVideoFrameMarginsInPixels() {
    loadConfigurationIfNecessary();
    return renderingVideoFrameMarginsInPixels;
}

void ConfigurationReaderImpl::loadConfigurationIfNecessary() {
    if (configurationLoaded) {
        return;
    }

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

    trackingMaxTipMatchingDistanceInPixels =
        propTree.get<int>("tracking.maxTipMatchingDistanceInPixels");
    trackingNbTipsToUseToDetectCameraMotion =
        propTree.get<int>("tracking.nbTipsToUseToDetectCameraMotion");
    trackingNbDetectionsToComputeAverageTipPositionAndSize =
        propTree.get<int>("tracking.nbDetectionsToComputeAverageTipPositionAndSize");
    trackingMinMatchingDistanceWithAnyObjectToConsiderTipNotHiddenByArm =
        propTree.get<int>("tracking.minMatchingDistanceWithAnyObjectToConsiderTipNotHiddenByArm");
    trackingMaxFramesAfterWhichATipIsConsideredLost =
        propTree.get<int>("tracking.maxFramesAfterWhichATipIsConsideredLost");
    trackingMinDistanceToConsiderNewTipAsTheSameAsAnExistingOne =
        propTree.get<int>("tracking.minDistanceToConsiderNewTipAsTheSameAsAnExistingOne");
    trackingMinChopstickLengthInPixels =
        propTree.get<int>("tracking.minChopstickLengthInPixels");
    trackingMaxChopstickLengthInPixels =
        propTree.get<int>("tracking.maxChopstickLengthInPixels");
    trackingMinIOUToConsiderTwoTipsAsAChopstick =
        propTree.get<double>("tracking.minIOUToConsiderTwoTipsAsAChopstick");
    trackingMaxFramesAfterWhichAChopstickIsConsideredLost =
        propTree.get<int>("tracking.maxFramesAfterWhichAChopstickIsConsideredLost");

    fs::path relativeRenderingOutputPath(propTree.get<string>("rendering.outputpath"));
    renderingOutputPath = fs::path(rootPath / relativeRenderingOutputPath);
    string renderingPainterImplementationsAsString =
        propTree.get<string>("rendering.painterImplementations");
    boost::split(
        renderingPainterImplementations,
        renderingPainterImplementationsAsString,
        boost::is_any_of(","),
        boost::token_compress_on);
    renderingDetectedObjectsPainterShowTips =
        propTree.get<bool>("rendering.detectedobjectsPainter_showTips");
    renderingDetectedObjectsPainterShowChopsticks =
        propTree.get<bool>("rendering.detectedobjectsPainter_showChopsticks");
    renderingDetectedObjectsPainterShowArms =
        propTree.get<bool>("rendering.detectedobjectsPainter_showArms");
    renderingWriterImplementation = propTree.get<string>("rendering.writerImplementation");
    renderingVideoFrameMarginsInPixels = propTree.get<int>("rendering.videoFrameMarginsInPixels");

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
    LOG_INFO(logger) << "\tTracking maximum tip matching distance (in pixels): "
        << trackingMaxTipMatchingDistanceInPixels;
    LOG_INFO(logger) << "\tTracking nb tips to use to detect camera motion: "
        << trackingNbTipsToUseToDetectCameraMotion;
    LOG_INFO(logger) << "\tTracking nb detections to compute average tip position and size: "
        << trackingNbDetectionsToComputeAverageTipPositionAndSize;
    LOG_INFO(logger) << "\tTracking minimum matching distance with any object to consider tip not hidden by arm: "
        << trackingMinMatchingDistanceWithAnyObjectToConsiderTipNotHiddenByArm;
    LOG_INFO(logger) << "\tTracking maximum frames after which a tip is considered as lost: "
        << trackingMaxFramesAfterWhichATipIsConsideredLost;
    LOG_INFO(logger) << "\tTracking minimum distance to consider new tip as the same as existing one: "
        << trackingMinDistanceToConsiderNewTipAsTheSameAsAnExistingOne;
    LOG_INFO(logger) << "\tTracking minimum chopstick length (in pixels): "
        << trackingMinChopstickLengthInPixels;
    LOG_INFO(logger) << "\tTracking maximum chopstick length (in pixels): "
        << trackingMaxChopstickLengthInPixels;
    LOG_INFO(logger) << "\tTracking minimum IoU to consider two tips as a chopstick: "
        << trackingMinIOUToConsiderTwoTipsAsAChopstick;
    LOG_INFO(logger) << "\tTracking maximum frames after which a chopstick is considered as lost: "
        << trackingMaxFramesAfterWhichAChopstickIsConsideredLost;

    LOG_INFO(logger) << "\tRendering output path: " << renderingOutputPath.string();
    LOG_INFO(logger) << "\tRendering painter implementations: " << renderingPainterImplementationsAsString;
    LOG_INFO(logger) << "\tRendering detected object painter - show tips: "
        << renderingDetectedObjectsPainterShowTips;
    LOG_INFO(logger) << "\tRendering detected object painter - show chopsticks: "
        << renderingDetectedObjectsPainterShowChopsticks;
    LOG_INFO(logger) << "\tRendering detected object painter - show arms: "
        << renderingDetectedObjectsPainterShowArms;
    LOG_INFO(logger) << "\tRendering writer implementation: " << renderingWriterImplementation;
    LOG_INFO(logger) << "\tRendering video frame margins: " << renderingVideoFrameMarginsInPixels;
}
