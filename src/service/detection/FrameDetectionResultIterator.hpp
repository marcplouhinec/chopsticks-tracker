#ifndef SERVICE_FRAME_DETECTION_RESULT_ITERATOR
#define SERVICE_FRAME_DETECTION_RESULT_ITERATOR

#include <vector>
#include <functional>
#include <boost/iterator/iterator_facade.hpp>
#include <opencv2/opencv.hpp>
#include "../../model/detection/FrameDetectionResult.hpp"
#include "../../model/detection/DetectedObject.hpp"

namespace service {

    /**
     * Iterator that provides {@link FrameDetectionResult}s.
     * 
     * This class allows object detection to be performed in a lazy way, when the {@link #increment()}
     * method is called.
     * 
     * @author Marc Plouhinec
     */
    class FrameDetectionResultIterator : public boost::iterator_facade<
        FrameDetectionResultIterator, model::FrameDetectionResult const, boost::forward_traversal_tag> {

        private:
            int frameIndex;
            std::function<std::vector<model::DetectedObject>(int)> detectedObjectsProvider;
            std::function<cv::Mat(int)> frameImageProvider;
            model::FrameDetectionResult nextFrameDetectionResult;
        
        public:
            /**
             * @param detectedObjectsProvider
             *     Lambda that provides the detected objects for the given frame index.
             * @param frameImageProvider
             *     Lambda that provides the frame image for the given frame index.
             */
            FrameDetectionResultIterator(
                std::function<std::vector<model::DetectedObject>(int)> detectedObjectsProvider,
                std::function<cv::Mat(int)> frameImageProvider
            ) : FrameDetectionResultIterator(0, detectedObjectsProvider, frameImageProvider) {}

            /**
             * @param frameIndex
             *     Index of the current frame.
             * @param detectedObjectsProvider
             *     Lambda that provides the detected objects for the given frame index.
             * @param frameImageProvider
             *     Lambda that provides the frame image for the given frame index.
             */
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
                std::function<cv::Mat(int)> localFrameImageProvider = frameImageProvider;
                int localFrameIndex = frameIndex;

                nextFrameDetectionResult = model::FrameDetectionResult(
                    frameIndex,
                    detectedObjectsProvider(frameIndex),
                    [localFrameImageProvider, localFrameIndex]() {
                        return localFrameImageProvider(localFrameIndex);
                    });
            }
    };
}

#endif // SERVICE_FRAME_DETECTION_RESULT_ITERATOR