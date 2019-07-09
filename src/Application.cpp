#include <iostream>
#include "service/detection/impl/DarknetObjectDetectionServiceImpl.hpp"

int main(int argc, char* argv[]) {
    std::cout << "start" << std::endl;
    service::DarknetObjectDetectionServiceImpl objectDetectionService;

    std::string videoFilePath = "";
    auto frameDetectionResultRange = objectDetectionService.detectObjectsInVideo(videoFilePath);

    for (auto frameDetectionResult : frameDetectionResultRange) {
        std::cout << "in: frameIndex = " << frameDetectionResult.frameImageProvider().size() << std::endl;
    }

    std::cout << "end" << std::endl;
    return 0;
}