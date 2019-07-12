#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "ObjectDetectorOpenCvDnnImpl.hpp"

using namespace model;
using namespace service;
using std::string;
using std::vector;
namespace pt = boost::property_tree;

vector<DetectedObject> ObjectDetectorOpenCvDnnImpl::detectObjectsAt(int frameIndex) {
    cv::Mat frame = pVideoFrameReader->getFrameAt(frameIndex);
    int frameWidth = pVideoFrameReader->getFrameWidth();
    int frameHeight = pVideoFrameReader->getFrameHeight();

    if (!neuralNetworkInitialized) {
        LOG_INFO(logger) << "Loading the YOLO neural network model...";
        string yoloModelCfgPath = pConfigurationReader->getYoloModelCfgPath().string();
        string yoloModelWeights = pConfigurationReader->getYoloModelWeightsPath().string();
        neuralNetwork = cv::dnn::readNetFromDarknet(yoloModelCfgPath, yoloModelWeights);
        
        outLayerNames = neuralNetwork.getUnconnectedOutLayersNames();
        objectTypesByClassId = pConfigurationReader->getYoloModelClassEnums();

        pt::ptree propTree;
        pt::ini_parser::read_ini(yoloModelCfgPath, propTree);
        int netWidth = propTree.get<int>("net.width");
        int netHeight = propTree.get<int>("net.height");
        blobSize = cv::Size(netWidth, netHeight);
        mean = cv::Scalar(0, 0, 0, 0);
        
        LOG_INFO(logger) << "YOLO model initialized: outLayerNames = " << "outLayerNames"
            << ", netWidth = " << netWidth << ", netHeight = " << netHeight;
        
        neuralNetworkInitialized = true;
    }

    // Detect objects
    cv::Mat blob = cv::dnn::blobFromImage(
        frame, 1 / 255.0, blobSize, mean, /* swapRB = */true, /* crop = */false, /* ddepth = */ CV_32F);
    neuralNetwork.setInput(blob);
    vector<cv::Mat> layerOutputs;
    neuralNetwork.forward(layerOutputs, outLayerNames);

    // Extract objects
    float minConfidence = pConfigurationReader->getObjectDetectionMinConfidence();
    vector<DetectedObject> detectedObjects;
    for (cv::Mat layerOutput : layerOutputs) {
        cv::Size layerOutputSize = layerOutput.size();
        for (int rowIndex = 0; rowIndex < layerOutputSize.height; rowIndex++) {
            vector<float> detections;
            for (int colIndex = 0; colIndex < layerOutputSize.width; colIndex++) {
                auto detection = layerOutput.at<float>(rowIndex, colIndex);
                detections.push_back(detection);
            }

            // Find the detected class and confidence
            int classId = -1;
            float confidence = -1.0f;
            for (int i = objectTypesByClassId.size(); i < detections.size(); i++) {
                if (detections[i] >= confidence) {
                    classId = i;
                    confidence = detections[i];
                }
            }

            // If the confidence is high enough, 
            if (confidence >= minConfidence) {
                float centerX = detections[0] * frameWidth;
                float centerY = detections[1] * frameHeight;
                float width = detections[2] * frameWidth;
                float height = detections[3] * frameHeight;
                float x = centerX - (width / 2);
                float y = centerY - (height / 2);

                DetectedObject detectedObject(
                    round(x), round(y),
                    round(width), round(height),
                    objectTypesByClassId[classId], 
                    confidence);
                detectedObjects.push_back(detectedObject);
            }
        }
    }
    return detectedObjects;
}