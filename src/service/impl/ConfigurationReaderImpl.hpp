#ifndef SERVICE_CONFIGURATION_READER_IMPL
#define SERVICE_CONFIGURATION_READER_IMPL

#include "../../utils/logging.hpp"
#include "../ConfigurationReader.hpp"

namespace service {

    class ConfigurationReaderImpl : public ConfigurationReader {
        private:
            boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger;

            boost::filesystem::path configurationPath;
            bool configurationLoaded = false;

            std::vector<std::string> yoloModelClassNames;
            boost::filesystem::path yoloModelCfgPath;
            boost::filesystem::path yoloModelWeightsPath;

            float objectDetectionMinConfidence;
            float objectDetectionNmsThreshold;
            std::string objectDetectionImplementation;
            boost::filesystem::path objectDetectionCacheFolderPath;

            boost::filesystem::path renderingOutputPath;

        public:
            ConfigurationReaderImpl(boost::filesystem::path configurationPath) :
                configurationPath(configurationPath) {}

            virtual ~ConfigurationReaderImpl() {}

            virtual std::vector<std::string> getYoloModelClassNames();
            virtual std::vector<model::DetectedObjectType> getYoloModelClassEnums();
            virtual boost::filesystem::path getYoloModelCfgPath();
            virtual boost::filesystem::path getYoloModelWeightsPath();

            virtual float getObjectDetectionMinConfidence();
            virtual float getObjectDetectionNmsThreshold();
            virtual std::string getObjectDetectionImplementation();
            virtual boost::filesystem::path getObjectDetectionCacheFolderPath();

            virtual boost::filesystem::path getRenderingOutputPath();
        
        private:
            void loadConfiguration();
    };

}

#endif // SERVICE_CONFIGURATION_READER_IMPL