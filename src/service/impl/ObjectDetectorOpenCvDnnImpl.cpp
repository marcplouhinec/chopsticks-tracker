#include <math.h>
#include <fstream>
#include <streambuf>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "ObjectDetectorOpenCvDnnImpl.hpp"

using namespace model;
using namespace service;
using std::ifstream;
using std::istreambuf_iterator;
using std::min;
using std::stringstream;
using std::string;
using std::vector;
namespace pt = boost::property_tree;

vector<DetectedObject> ObjectDetectorOpenCvDnnImpl::detectObjectsAt(int frameIndex) {
    cv::Mat frame = videoFrameReader.readFrameAt(frameIndex);

    if (!neuralNetworkInitialized) {
        LOG_INFO(logger) << "Loading the YOLO neural network model...";
        string yoloModelCfgPath = configuration.yoloModelCfgPath.string();
        string yoloModelWeights = configuration.yoloModelWeightsPath.string();
        neuralNetwork = cv::dnn::readNetFromDarknet(yoloModelCfgPath, yoloModelWeights);
        
        outLayerNames = neuralNetwork.getUnconnectedOutLayersNames();
        objectTypesByClassId = DetectedObjectTypeHelper::stringsToEnums(configuration.yoloModelClassNames);

        ifstream yoloModelCfgStream(yoloModelCfgPath);
        string yoloModelCfg((istreambuf_iterator<char>(yoloModelCfgStream)), istreambuf_iterator<char>());
        yoloModelCfgStream.close();
        auto indexOfNet = yoloModelCfg.find("[net]");
        auto indexOfNextSection = yoloModelCfg.find("[", indexOfNet + 5);
        string cfgNetSection = yoloModelCfg.substr(indexOfNet, indexOfNextSection);
        stringstream cfgNetSectionStream(cfgNetSection);

        pt::ptree propTree;
        pt::ini_parser::read_ini(cfgNetSectionStream, propTree);
        int netWidth = propTree.get<int>("net.width");
        int netHeight = propTree.get<int>("net.height");
        blobSize = cv::Size(netWidth, netHeight);
        mean = cv::Scalar(0, 0, 0, 0);
        
        LOG_INFO(logger) << "YOLO model initialized: outLayerNames = " << "outLayerNames"
            << ", netWidth = " << netWidth << ", netHeight = " << netHeight;
        
        minTipConfidence = configuration.objectDetectionMinTipConfidence;
        minChopstickConfidence = configuration.objectDetectionMinChopstickConfidence;
        minArmConfidence = configuration.objectDetectionMinArmConfidence;
        minConfidence = min(minTipConfidence, min(minChopstickConfidence, minArmConfidence));

        neuralNetworkInitialized = true;
    }

    // Detect objects
    cv::Mat blob = cv::dnn::blobFromImage(
        frame, 1 / 255.0, blobSize, mean, /* swapRB = */true, /* crop = */false, /* ddepth = */ CV_32F);
    neuralNetwork.setInput(blob);
    vector<cv::Mat> layerOutputs;
    neuralNetwork.forward(layerOutputs, outLayerNames);

    // Extract objects
    vector<DetectedObject> detectedObjects;
    for (cv::Mat layerOutput : layerOutputs) {
        for (int rowIndex = 0; rowIndex < layerOutput.rows; rowIndex++) {
            int probStartIndex = 5;
            int probSize = layerOutput.cols - probStartIndex;

            // Find the detected class and confidence
            int classId = -1;
            float confidence = -1.0f;
            for (int probIndex = probStartIndex; probIndex < probStartIndex + probSize; probIndex++) {
                float probability = layerOutput.at<float>(rowIndex, probIndex);
                if (probability > confidence) {
                    confidence = probability;
                    classId = probIndex - probStartIndex;
                }
            }

            // If the confidence is high enough, 
            if (confidence >= minConfidence) {
                DetectedObjectType objectType = objectTypesByClassId[classId];
            
                float minConfidenceForType = minConfidence;
                switch (objectType) {
                    case DetectedObjectType::SMALL_TIP:
                    case DetectedObjectType::BIG_TIP:
                        minConfidenceForType = minTipConfidence;
                        break;
                    case DetectedObjectType::ARM:
                        minConfidenceForType = minArmConfidence;
                        break;
                    case DetectedObjectType::CHOPSTICK:
                    default:
                        minConfidenceForType = minChopstickConfidence;
                        break;
                }

                if (confidence >= minConfidenceForType) {
                    float centerX = layerOutput.at<float>(rowIndex, 0) * videoProperties.frameWidth;
                    float centerY = layerOutput.at<float>(rowIndex, 1) * videoProperties.frameHeight;
                    float width = layerOutput.at<float>(rowIndex, 2) * videoProperties.frameWidth;
                    float height = layerOutput.at<float>(rowIndex, 3) * videoProperties.frameHeight;
                    float x = centerX - (width / 2);
                    float y = centerY - (height / 2);

                    const DetectedObject detectedObject(
                        x, y,
                        width, height,
                        objectTypesByClassId[classId], 
                        confidence);
                    detectedObjects.push_back(detectedObject);
                }
            }
        }
    }
    return detectedObjects;
}