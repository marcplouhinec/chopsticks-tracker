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

        public:
            ConfigurationReaderImpl(boost::filesystem::path configurationPath) :
                configurationPath(configurationPath) {}

            virtual ~ConfigurationReaderImpl() {}

            virtual std::vector<std::string> getYoloModelClassNames();
            virtual boost::filesystem::path getYoloModelCfgPath();
            virtual boost::filesystem::path getYoloModelWeightsPath();

            virtual float getObjectDetectionMinConfidence();
            virtual float getObjectDetectionNmsThreshold();
        
        private:
            void loadConfiguration();
    };

}

#endif // SERVICE_CONFIGURATION_READER_IMPL