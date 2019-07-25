#include <stdexcept>
#include "VideoFrameWriterMultiJpegImpl.hpp"

using namespace service;
using std::runtime_error;
using std::string;
using std::to_string;
namespace fs = boost::filesystem;

void VideoFrameWriterMultiJpegImpl::writeFrameAt(int frameIndex, cv::Mat& frame) {
    if (!folderInitialized) {
        LOG_INFO(logger) << "Initialize the output folder...";

        string outputFolderName = inputVideoPath.stem().string();
        outputFolderPath = fs::path(configuration.renderingOutputPath / outputFolderName);

        if (fs::is_directory(outputFolderPath) || fs::exists(outputFolderPath)) {
            if (!fs::remove_all(outputFolderPath)) {
                throw runtime_error("Unable to delete the folder: " + outputFolderPath.string());
            }
        }
        fs::create_directories(outputFolderPath);

        folderInitialized = true;
    }

    string imageName = to_string(frameIndex) + ".jpg";
    fs::path imagePath(outputFolderPath / imageName);
    cv::imwrite(imagePath.string(), frame);
}