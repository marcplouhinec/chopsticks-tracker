#include <fstream>
#include <streambuf>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "ObjectDetectorOpenCvDnnImpl.hpp"

using namespace model;
using namespace service;
using std::ifstream;
using std::istreambuf_iterator;
using std::stringstream;
using std::string;
using std::vector;
namespace pt = boost::property_tree;

vector<DetectedObject> ObjectDetectorOpenCvDnnImpl::detectObjectsAt(int frameIndex) {
    cv::Mat frame = videoFrameReader.getFrameAt(frameIndex);
    int frameWidth = videoFrameReader.getFrameWidth();
    int frameHeight = videoFrameReader.getFrameHeight();

    if (!neuralNetworkInitialized) {
        LOG_INFO(logger) << "Loading the YOLO neural network model...";
        string yoloModelCfgPath = configurationReader.getYoloModelCfgPath().string();
        string yoloModelWeights = configurationReader.getYoloModelWeightsPath().string();
        neuralNetwork = cv::dnn::readNetFromDarknet(yoloModelCfgPath, yoloModelWeights);
        
        outLayerNames = neuralNetwork.getUnconnectedOutLayersNames();
        objectTypesByClassId = configurationReader.getYoloModelClassEnums();

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
        
        neuralNetworkInitialized = true;
    }

    // Detect objects
    cv::Mat blob = cv::dnn::blobFromImage(
        frame, 1 / 255.0, blobSize, mean, /* swapRB = */true, /* crop = */false, /* ddepth = */ CV_32F);
    neuralNetwork.setInput(blob);
    vector<cv::Mat> layerOutputs;
    neuralNetwork.forward(layerOutputs, outLayerNames);

    // Extract objects
    float minConfidence = configurationReader.getObjectDetectionMinConfidence();
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
                float centerX = layerOutput.at<float>(rowIndex, 0) * frameWidth;
                float centerY = layerOutput.at<float>(rowIndex, 1) * frameHeight;
                float width = layerOutput.at<float>(rowIndex, 2) * frameWidth;
                float height = layerOutput.at<float>(rowIndex, 3) * frameHeight;
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