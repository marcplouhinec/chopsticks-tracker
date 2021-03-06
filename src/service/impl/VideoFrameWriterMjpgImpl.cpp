#include <stdexcept>
#include "VideoFrameWriterMjpgImpl.hpp"

using namespace service;
using std::make_unique;
using std::runtime_error;
using std::string;
using std::to_string;
using std::unique_ptr;
namespace fs = boost::filesystem;

VideoFrameWriterMjpgImpl::~VideoFrameWriterMjpgImpl() {
    if (pVideoWriter) {
        pVideoWriter.release();
    }
}

cv::Mat VideoFrameWriterMjpgImpl::buildOutputFrame() {
    return cv::Mat(outputFrameHeight, outputFrameWidth, CV_8UC3);
}

void VideoFrameWriterMjpgImpl::writeFrameAt(int frameIndex, cv::Mat& frame) {
    int expectedFrameIndex = lastWrittenFrameIndex + 1;
    if (expectedFrameIndex != frameIndex) {
        throw runtime_error("Only sequential write is supported (expected frameIndex = "
            + to_string(expectedFrameIndex) + ", actual = " + to_string(frameIndex) + ")");
    }

    if (!pVideoWriter) {
        LOG_INFO(logger) << "Initialize the output video file...";

        string outputVideoFilename = inputVideoPath.stem().string() + ".avi";
        fs::path outputVideoPath(configuration.renderingOutputPath / outputVideoFilename);

        if (fs::is_directory(outputVideoPath) || fs::exists(outputVideoPath)) {
            if (!fs::remove(outputVideoPath)) {
                throw runtime_error("Unable to delete the file: " + outputVideoPath.string());
            }
        }
        fs::path parentPath = outputVideoPath.parent_path();
        if (!fs::is_directory(parentPath)) {
            fs::create_directories(parentPath);
        }

        pVideoWriter = make_unique<cv::VideoWriter>(cv::VideoWriter(
            outputVideoPath.string(),
            cv::VideoWriter::fourcc('M','J','P','G'),
            outputFps,
            cv::Size(outputFrameWidth, outputFrameHeight),
            /* isColor= */true));
    }

    pVideoWriter->write(frame);
    lastWrittenFrameIndex = frameIndex;
}