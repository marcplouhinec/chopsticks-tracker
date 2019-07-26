#include <stdexcept>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include "ObjectDetectorCacheImpl.hpp"

using namespace model;
using namespace service;
using std::ifstream;
using std::ofstream;
using std::string;
using std::istreambuf_iterator;
using std::to_string;
using std::vector;
using std::runtime_error;
namespace fs = boost::filesystem;

vector<DetectedObject> ObjectDetectorCacheImpl::detectObjectsAt(int frameIndex) {
    fs::path rootCacheFolderPath = configuration.objectDetectionCacheFolderPath;
    fs::path cacheFolderPath(rootCacheFolderPath / videoPath.filename());

    // Create the cache folder if it doesn't exist
    if (!cacheFolderInitialized) {
        LOG_INFO(logger) << "Initialize the cache folder: " << cacheFolderPath.string();

        if (!fs::is_directory(cacheFolderPath)) {
            if (fs::exists(cacheFolderPath)) {
                if (!fs::remove(cacheFolderPath)) {
                    throw runtime_error("Unable to delete the file: " + cacheFolderPath.string());
                }
            } else {
                if (!fs::create_directories(cacheFolderPath)) {
                    throw runtime_error("Unable to create the directory: " + cacheFolderPath.string());
                }
            }
        }

        cacheFolderInitialized = true;
    }

    // Check if the detected objects are cached already
    string fileName = to_string(frameIndex) + ".json";
    fs::path objectsPath(cacheFolderPath / fileName);
    
    if (fs::exists(objectsPath)) {
        // Unserialize the objects
        ifstream objectsFile(objectsPath.string());
        string detectedObjectsJson(
            (istreambuf_iterator<char>(objectsFile)), istreambuf_iterator<char>());
        objectsFile.close();

        return convertFromJson(detectedObjectsJson);
    } else {
        // Use the wrapped object detector to get the objects and serialize it
        vector<DetectedObject> detectedObjects = wrappedObjectDetector.detectObjectsAt(frameIndex);
        
        ofstream objectsFile(objectsPath.string());
        objectsFile << convertToJson(detectedObjects);
        objectsFile.close();
 
        return detectedObjects;
    }
}

std::string ObjectDetectorCacheImpl::convertToJson(std::vector<DetectedObject> detectedObjects) {
    rapidjson::Document jsonDoc;
    jsonDoc.SetArray();
    rapidjson::Document::AllocatorType& allocator = jsonDoc.GetAllocator();
    
    for (const DetectedObject& detectedObject : detectedObjects) {
        rapidjson::Value detectedObjectValue;
        detectedObjectValue.SetObject();
        detectedObjectValue.AddMember("x", detectedObject.x, allocator);
        detectedObjectValue.AddMember("y", detectedObject.y, allocator);
        detectedObjectValue.AddMember("width", detectedObject.width, allocator);
        detectedObjectValue.AddMember("height", detectedObject.height, allocator);
        detectedObjectValue.AddMember("confidence", detectedObject.confidence, allocator);

        string objectTypeString = DetectedObjectTypeHelper::enumToString(detectedObject.objectType);
        rapidjson::Value objectTypeValue;
        objectTypeValue.SetString(
            objectTypeString.c_str(),
            static_cast<rapidjson::SizeType>(objectTypeString.length()),
            allocator);
        detectedObjectValue.AddMember("objectType", objectTypeValue, allocator);

        jsonDoc.PushBack(detectedObjectValue, allocator);
    }

    rapidjson::StringBuffer jsonStringBuffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(jsonStringBuffer);
    jsonDoc.Accept(writer);
    return jsonStringBuffer.GetString();
}

std::vector<DetectedObject> ObjectDetectorCacheImpl::convertFromJson(std::string detectedObjectsJson) {
    rapidjson::Document jsonDoc;
    jsonDoc.Parse(detectedObjectsJson.c_str());

    std::vector<DetectedObject> detectedObjects;
    for (rapidjson::Value::ConstValueIterator itr = jsonDoc.Begin(); itr != jsonDoc.End(); ++itr) {
        const rapidjson::Value& detectedObjectValue = *itr;

        float x = detectedObjectValue["x"].GetInt();
        float y = detectedObjectValue["y"].GetInt();
        float width = detectedObjectValue["width"].GetInt();
        float height = detectedObjectValue["height"].GetInt();
        float confidence = detectedObjectValue["confidence"].GetFloat();
        string objectTypeString(detectedObjectValue["objectType"].GetString());
        DetectedObjectType objectType = DetectedObjectTypeHelper::stringToEnum(objectTypeString);
        
        const DetectedObject detectedObject(x, y, width, height, objectType, confidence);
        detectedObjects.push_back(detectedObject);
    }
    
    return detectedObjects;
}