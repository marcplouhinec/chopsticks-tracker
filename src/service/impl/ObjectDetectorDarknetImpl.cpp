#include <algorithm>
#include <math.h>
#include "../../model/VideoProperties.hpp"
#include "ObjectDetectorDarknetImpl.hpp"

using namespace model;
using namespace service;
using std::map;
using std::min;
using std::string;
using std::vector;

vector<DetectedObject> ObjectDetectorDarknetImpl::detectObjectsAt(int frameIndex) {
    cv::Mat frame = videoFrameReader.readFrameAt(frameIndex);

    if (!pNeuralNetwork) {
        LOG_INFO(logger) << "Loading the YOLO neural network model...";
        pNeuralNetwork = std::unique_ptr<network>(load_network_custom(
            (char*) configuration.yoloModelCfgPath.string().c_str(), 
            (char*) configuration.yoloModelWeightsPath.string().c_str(), 
            /*clear = */ 0,
            /*batch = */ 1));
        
        lastLayer = pNeuralNetwork->layers[pNeuralNetwork->n - 1];
        objectTypesByClassId = DetectedObjectTypeHelper::stringsToEnums(configuration.yoloModelClassNames);

        minTipConfidence = configuration.objectDetectionMinTipConfidence;
        minChopstickConfidence = configuration.objectDetectionMinChopstickConfidence;
        minArmConfidence = configuration.objectDetectionMinArmConfidence;
        minConfidence = min(minTipConfidence, min(minChopstickConfidence, minArmConfidence));
    }

    // Convert and resize the image for YOLO on Darknet
    image frameImage = matToImage(frame);
    image resizedImage = resize_image(frameImage, pNeuralNetwork->w, pNeuralNetwork->h);
    // TODO crop before resize (can be done with a VideoFrameReaderCropImpl)

    // Detect objects
    network_predict_image(pNeuralNetwork.get(), resizedImage);
    int nbDetections = 0;
    detection* detections = get_network_boxes(
        pNeuralNetwork.get(),
        pNeuralNetwork->w,
        pNeuralNetwork->h,
        minConfidence,
        minConfidence,
        /* map */0,
        /* relative */1,
        &nbDetections,
        /* letter */0);
    float nmsThreshold = configuration.objectDetectionNmsThreshold;
    do_nms_sort(detections, nbDetections, lastLayer.classes, nmsThreshold);
    
    // Release image resources
    free_image(resizedImage);
    free_image(frameImage);

    // Extract objects
    vector<DetectedObject> detectedObjects;
    for (int detectionIndex = 0; detectionIndex < nbDetections; detectionIndex++) {
        detection detection = detections[detectionIndex];

        // Find the detected class and confidence
        int classId = -1;
        float confidence = -1.0f;
        for (int i = 0; i < detection.classes; i++) {
            if (detection.prob[i] >= confidence) {
                classId = i;
                confidence = detection.prob[i];
            }
        }

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
                float centerX = detection.bbox.x * videoProperties.frameWidth;
                float centerY = detection.bbox.y * videoProperties.frameHeight;
                float width = detection.bbox.w * videoProperties.frameWidth;
                float height = detection.bbox.h * videoProperties.frameHeight;
                float x = centerX - (width / 2);
                float y = centerY - (height / 2);

                const DetectedObject detectedObject(
                    x, y,
                    width, height,
                    objectType, 
                    confidence);
                detectedObjects.push_back(detectedObject);
            }
        }
    }

    // Release the remaining resources
    free_detections(detections, nbDetections);

    return detectedObjects;
}

image ObjectDetectorDarknetImpl::matToImage(cv::Mat& src) {
    int width = src.cols;
    int height = src.rows;
    int nbChannels = src.channels();
    int step = src.step;

    image dest = make_image(width, height, nbChannels);

    for (int c = 0; c < nbChannels; c++) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                float pixelValue = src.data[y * step + x * nbChannels + c] / 255.0f;
                int offset = c * width * height + y * width + x;
                dest.data[offset] = pixelValue;
            }
        }
    }

    rgbgr_image(dest);

    return dest;
}