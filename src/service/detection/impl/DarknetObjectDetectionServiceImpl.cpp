#include "../../../utils/logging.hpp"
#include "DarknetObjectDetectionServiceImpl.hpp"

using namespace service;
using namespace model;
namespace lg = boost::log;

lg::sources::severity_logger<lg::trivial::severity_level> logger;

std::vector<DetectedObject> DarknetObjectDetectionServiceImpl::detectObjectsInImage(cv::Mat image) {
    LOG_INFO(logger) << "Detecting objects...";
    // TODO

    return std::vector<DetectedObject>();
}