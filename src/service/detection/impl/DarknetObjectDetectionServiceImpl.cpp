#include "DarknetObjectDetectionServiceImpl.hpp"

boost::iterator_range<service::FrameDetectionResultIterator>
    service::DarknetObjectDetectionServiceImpl::detectObjectsInVideo(std::string videoFilePath) {

    // TODO

    service::FrameDetectionResultIterator first(
        [](int frameIndex) { return std::vector<model::DetectedObject>(); },
        [](int frameIndex) { return cv::Mat(1, frameIndex, CV_8UC1, cv::Scalar(70)); }
    );
    
    service::FrameDetectionResultIterator last(
        10,
        [](int frameIndex) { return std::vector<model::DetectedObject>(); },
        [](int frameIndex) { return cv::Mat(1, frameIndex, CV_8UC1, cv::Scalar(70)); }
    );

    return boost::iterator_range<service::FrameDetectionResultIterator>(first, last);
}
