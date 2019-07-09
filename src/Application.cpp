#include <iostream>
#include <boost/move/utility_core.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include "service/detection/impl/DarknetObjectDetectionServiceImpl.hpp"

int main(int argc, char* argv[]) {
    boost::log::add_common_attributes();
    boost::log::sources::logger logger;

    BOOST_LOG(logger) << "Building the application context...";
    service::DarknetObjectDetectionServiceImpl objectDetectionService;

    BOOST_LOG(logger) << "Detect objects in video...";
    std::string videoFilePath = "";
    auto frameDetectionResultRange = objectDetectionService.detectObjectsInVideo(videoFilePath);

    for (auto frameDetectionResult : frameDetectionResultRange) {
        BOOST_LOG(logger) << "Process frame " << frameDetectionResult.frameIndex << "...";
        // TODO
    }

    BOOST_LOG(logger) << "Application executed with success!";
    return 0;
}