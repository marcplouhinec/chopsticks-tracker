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

Configuration ConfigurationReaderImpl::read(fs::path configurationPath) const {
    Configuration config;

    pt::ptree propTree;
    pt::ini_parser::read_ini(configurationPath.string(), propTree);

    string yoloClassNamesAsString = propTree.get<string>("yoloModel.classnames");
    boost::split(
        config.yoloModelClassNames,
        yoloClassNamesAsString,
        boost::is_any_of(","),
        boost::token_compress_on);

    fs::path relativeYoloCfgPath(propTree.get<string>("yoloModel.cfgpath"));
    fs::path relativeYoloWeightsPath(propTree.get<string>("yoloModel.weightspath"));
    fs::path rootPath = configurationPath.parent_path();
    config.yoloModelCfgPath = fs::canonical(fs::path(rootPath / relativeYoloCfgPath));
    config.yoloModelWeightsPath = fs::canonical(fs::path(rootPath / relativeYoloWeightsPath));

    config.inputVideoCrop = propTree.get<bool>("inputVideo.crop");

    config.objectDetectionMinTipConfidence = propTree.get<float>("objectDetection.minTipConfidence");
    config.objectDetectionMinChopstickConfidence =
        propTree.get<float>("objectDetection.minChopstickConfidence");
    config.objectDetectionMinArmConfidence = propTree.get<float>("objectDetection.minArmConfidence");
    config.objectDetectionNmsThreshold = propTree.get<float>("objectDetection.nmsThreshold");
    config.objectDetectionImplementation = propTree.get<string>("objectDetection.implementation");
    fs::path relativeCacheFolderPath(propTree.get<string>("objectDetection.cacheFolderPath"));
    config.objectDetectionCacheFolderPath = fs::path(rootPath / relativeCacheFolderPath);

    config.trackingMaxTipMatchingDistanceInPixels =
        propTree.get<int>("tracking.maxTipMatchingDistanceInPixels");
    config.trackingNbTipsToUseToDetectCameraMotion =
        propTree.get<int>("tracking.nbTipsToUseToDetectCameraMotion");
    config.trackingNbDetectionsToComputeAverageTipPositionAndSize =
        propTree.get<int>("tracking.nbDetectionsToComputeAverageTipPositionAndSize");
    config.trackingMinMatchingDistanceWithAnyObjectToConsiderTipNotHiddenByArm =
        propTree.get<int>("tracking.minMatchingDistanceWithAnyObjectToConsiderTipNotHiddenByArm");
    config.trackingMaxFramesAfterWhichATipIsConsideredLost =
        propTree.get<int>("tracking.maxFramesAfterWhichATipIsConsideredLost");
    config.trackingMinDistanceToConsiderNewTipAsTheSameAsAnExistingOne =
        propTree.get<int>("tracking.minDistanceToConsiderNewTipAsTheSameAsAnExistingOne");
    config.trackingMinChopstickLengthInPixels =
        propTree.get<int>("tracking.minChopstickLengthInPixels");
    config.trackingMaxChopstickLengthInPixels =
        propTree.get<int>("tracking.maxChopstickLengthInPixels");
    config.trackingMinIOUToConsiderTwoTipsAsAChopstick =
        propTree.get<double>("tracking.minIOUToConsiderTwoTipsAsAChopstick");
    config.trackingMaxFramesAfterWhichAChopstickIsConsideredLost =
        propTree.get<int>("tracking.maxFramesAfterWhichAChopstickIsConsideredLost");

    fs::path relativeRenderingOutputPath(propTree.get<string>("rendering.outputpath"));
    config.renderingOutputPath = fs::path(rootPath / relativeRenderingOutputPath);
    config.renderingDetectedObjectsPainterShowTips =
        propTree.get<bool>("rendering.detectedObjectsPainter_showTips");
    config.renderingDetectedObjectsPainterShowChopsticks =
        propTree.get<bool>("rendering.detectedObjectsPainter_showChopsticks");
    config.renderingDetectedObjectsPainterShowArms =
        propTree.get<bool>("rendering.detectedObjectsPainter_showArms");
    config.renderingTrackedObjectsPainterShowTips =
        propTree.get<bool>("rendering.trackedObjectsPainter_showTips");
    config.renderingTrackedObjectsPainterShowAcceptedChopsticks =
        propTree.get<bool>("rendering.trackedObjectsPainter_showAcceptedChopsticks");
    config.renderingTrackedObjectsPainterShowRejectedChopsticks =
        propTree.get<bool>("rendering.trackedObjectsPainter_showRejectedChopsticks");
    config.renderingTrackedObjectsPainterShowChopstickArrows =
        propTree.get<bool>("rendering.trackedObjectsPainter_showChopstickArrows");
    config.renderingWriterImplementation = propTree.get<string>("rendering.writerImplementation");
    config.renderingVideoFrameMarginsInPixels = propTree.get<int>("rendering.videoFrameMarginsInPixels");

    return config;
}
