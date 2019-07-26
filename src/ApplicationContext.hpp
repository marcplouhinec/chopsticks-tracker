#ifndef APPLICATION_CONTEXT
#define APPLICATION_CONTEXT

#include <memory>
#include <boost/filesystem.hpp>
#include "service/impl/ConfigurationReaderImpl.hpp"
#include "service/impl/ObjectDetectorCacheImpl.hpp"
#include "service/impl/ObjectDetectorDarknetImpl.hpp"
#include "service/impl/ObjectDetectorOpenCvDnnImpl.hpp"
#include "service/impl/TrackerTipImpl.hpp"
#include "service/impl/TrackerChopstickImpl.hpp"
#include "service/impl/VideoFramePainterImageImpl.hpp"
#include "service/impl/VideoFramePainterDetectedObjectsImpl.hpp"
#include "service/impl/VideoFramePainterTrackedObjectsImpl.hpp"
#include "service/impl/VideoFrameReaderImpl.hpp"
#include "service/impl/VideoFrameWriterMjpgImpl.hpp"
#include "service/impl/VideoFrameWriterMultiJpegImpl.hpp"

class ApplicationContext {
    private:
        model::Configuration configuration;
        model::VideoProperties videoProperties;

        std::unique_ptr<service::ConfigurationReader> pConfigurationReaderImpl;
        std::unique_ptr<service::VideoFrameReader> pVideoFrameReaderImpl;
        std::unique_ptr<service::ObjectDetector> pInnerObjectDetector;
        std::unique_ptr<service::ObjectDetector> pObjectDetectorCacheImpl;
        std::unique_ptr<service::TrackerTip> pTrackerTipImpl;
        std::unique_ptr<service::TrackerChopstick> pTrackerChopstickImpl;
        std::unique_ptr<service::VideoFrameWriter> pVideoFrameWriter;
        std::unique_ptr<service::VideoFramePainterImage> pVideoFramePainterImageImpl;
        std::unique_ptr<service::VideoFramePainterDetectedObjects> pVideoFramePainterDetectedObjectsImpl;
        std::unique_ptr<service::VideoFramePainterTrackedObjects> pVideoFramePainterTrackedObjectsImpl;

    public:
        ApplicationContext(boost::filesystem::path& configurationPath, boost::filesystem::path& videoPath) {
            // Configuration
            pConfigurationReaderImpl.reset(new service::ConfigurationReaderImpl());
            configuration = pConfigurationReaderImpl->read(configurationPath);

            // Video reader
            pVideoFrameReaderImpl.reset(new service::VideoFrameReaderImpl(videoPath));
            videoProperties = pVideoFrameReaderImpl->getVideoProperties();

            // Objects detection
            if (configuration.objectDetectionImplementation == "darknet") {
                pInnerObjectDetector.reset(new service::ObjectDetectorDarknetImpl(
                    configuration, *pVideoFrameReaderImpl, videoProperties));
            } else if (configuration.objectDetectionImplementation == "opencvdnn") {
                pInnerObjectDetector.reset(new service::ObjectDetectorOpenCvDnnImpl(
                    configuration, *pVideoFrameReaderImpl, videoProperties));
            }
            pObjectDetectorCacheImpl.reset(new service::ObjectDetectorCacheImpl(
                configuration, *pInnerObjectDetector, videoPath));

            // Objects tracking
            pTrackerTipImpl.reset(new service::TrackerTipImpl(configuration));
            pTrackerChopstickImpl.reset(new service::TrackerChopstickImpl(configuration));

            // Video writer
            if (configuration.renderingWriterImplementation == "mjpeg") {
                pVideoFrameWriter.reset(new service::VideoFrameWriterMjpgImpl(
                    configuration, videoPath, videoProperties));
            } else if (configuration.renderingWriterImplementation == "multijpeg") {
                pVideoFrameWriter.reset(new service::VideoFrameWriterMultiJpegImpl(
                    configuration, videoPath, videoProperties));
            }

            // Video painters
            pVideoFramePainterImageImpl.reset(
                new service::VideoFramePainterImageImpl(configuration));
            pVideoFramePainterDetectedObjectsImpl.reset(
                new service::VideoFramePainterDetectedObjectsImpl(configuration));
            pVideoFramePainterTrackedObjectsImpl.reset(
                new service::VideoFramePainterTrackedObjectsImpl(configuration));
        }

        const model::Configuration& getConfiguration() const {
            return configuration;
        }

        const model::VideoProperties& getVideoProperties() const {
            return videoProperties;
        }

        const service::ConfigurationReader& getConfigurationReader() const {
            return *pConfigurationReaderImpl;
        }

        service::VideoFrameReader& getVideoFrameReader() const {
            return *pVideoFrameReaderImpl;
        }

        service::ObjectDetector& getObjectDetector() const {
            return *pObjectDetectorCacheImpl;
        }

        const service::TrackerTip& getTrackerTip() const {
            return *pTrackerTipImpl;
        }

        const service::TrackerChopstick& getTrackerChopstick() const {
            return *pTrackerChopstickImpl;
        }

        service::VideoFrameWriter& getVideoFrameWriter() const {
            return *pVideoFrameWriter;
        }

        const service::VideoFramePainterImage& getVideoFramePainterImage() const {
            return *pVideoFramePainterImageImpl;
        }

        const service::VideoFramePainterDetectedObjects& getVideoFramePainterDetectedObjects() const {
            return *pVideoFramePainterDetectedObjectsImpl;
        }

        const service::VideoFramePainterTrackedObjects& getVideoFramePainterTrackedObjects() const {
            return *pVideoFramePainterTrackedObjectsImpl;
        }
};

#endif // APPLICATION_CONTEXT