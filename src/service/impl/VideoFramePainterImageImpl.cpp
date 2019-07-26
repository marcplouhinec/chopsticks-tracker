#include "VideoFramePainterImageImpl.hpp"

using namespace model;
using namespace service;

void VideoFramePainterImageImpl::paintOnFrame(
    cv::Mat& frame, const cv::Mat& image, const FrameOffset accumulatedFrameOffset) const {
    
    frame.setTo(blackColor);
    int marginLeft = round(configuration.renderingVideoFrameMarginsInPixels - accumulatedFrameOffset.dx);
    int marginTop = round(configuration.renderingVideoFrameMarginsInPixels - accumulatedFrameOffset.dy);
    image.copyTo(frame(cv::Rect(marginLeft, marginTop, image.cols, image.rows)));
}