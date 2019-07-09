#ifndef SERVICE_FRAME_DETECTION_RESULT_ITERATOR
#define SERVICE_FRAME_DETECTION_RESULT_ITERATOR

#include <vector>
#include <functional>
#include <boost/iterator/iterator_facade.hpp>
#include <opencv2/opencv.hpp>
#include "../../model/detection/FrameDetectionResult.hpp"
#include "../../model/detection/DetectedObject.hpp"

namespace service {

    class FrameDetectionResultIterator : public boost::iterator_facade<
        FrameDetectionResultIterator, model::FrameDetectionResult const, boost::forward_traversal_tag> {

        private:
            int frameIndex;
            std::function<std::vector<model::DetectedObject>(int)> detectedObjectsProvider;
            std::function<cv::Mat(int)> frameImageProvider;
            model::FrameDetectionResult nextFrameDetectionResult;
        
        public:
            FrameDetectionResultIterator(
                std::function<std::vector<model::DetectedObject>(int)> detectedObjectsProvider,
                std::function<cv::Mat(int)> frameImageProvider
            ) : FrameDetectionResultIterator(0, detectedObjectsProvider, frameImageProvider) {}

            explicit FrameDetectionResultIterator(
                int frameIndex,
                std::function<std::vector<model::DetectedObject>(int)> detectedObjectsProvider,
                std::function<cv::Mat(int)> frameImageProvider
            ) : frameIndex(frameIndex),
                detectedObjectsProvider(detectedObjectsProvider),
                frameImageProvider(frameImageProvider) {
                computeNextFrameDetectionResult();
            }

        private:
            friend class boost::iterator_core_access;

            void increment() {
                frameIndex++;

                computeNextFrameDetectionResult();
            }

            bool equal(FrameDetectionResultIterator const& other) const {
                return this->frameIndex == other.frameIndex;
            }

            model::FrameDetectionResult const& dereference() const {
                return nextFrameDetectionResult;
            }

            void computeNextFrameDetectionResult() {
                std::vector<model::DetectedObject> detectedObjects = detectedObjectsProvider(frameIndex);

                std::function<cv::Mat(int)> localFrameImageProvider = frameImageProvider;
                int localFrameIndex = frameIndex;

                nextFrameDetectionResult = model::FrameDetectionResult(frameIndex, detectedObjects,
                    [localFrameImageProvider, localFrameIndex]() {
                        return localFrameImageProvider(localFrameIndex);
                    });
            }
    };
}

#endif // SERVICE_FRAME_DETECTION_RESULT_ITERATOR