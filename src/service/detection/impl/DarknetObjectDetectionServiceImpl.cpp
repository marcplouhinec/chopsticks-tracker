#include "DarknetObjectDetectionServiceImpl.hpp"

boost::iterator_range<service::FrameDetectionResultIterator>
    service::DarknetObjectDetectionServiceImpl::detectObjectsInVideo(std::string videoFilePath) {

    // TODO

    std::function<std::vector<model::DetectedObject>(int)> detectedObjectsProvider =
        [](int frameIndex) { return std::vector<model::DetectedObject>(); };
    std::function<cv::Mat(int)> frameImageProvider =
        [](int frameIndex) { return cv::Mat(1, frameIndex, CV_8UC1, cv::Scalar(70)); };

    service::FrameDetectionResultIterator first(detectedObjectsProvider, frameImageProvider);
    service::FrameDetectionResultIterator last(10, detectedObjectsProvider, frameImageProvider);

    return boost::iterator_range<service::FrameDetectionResultIterator>(first, last);
}
