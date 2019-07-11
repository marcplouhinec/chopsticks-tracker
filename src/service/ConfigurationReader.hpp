#ifndef SERVICE_CONFIGURATION_READER
#define SERVICE_CONFIGURATION_READER

#include <string>
#include <vector>
#include <boost/filesystem.hpp>

namespace service {

    class ConfigurationReader {
        public:
            virtual ~ConfigurationReader() {}

            virtual std::vector<std::string> getYoloModelClassNames() = 0;
            virtual boost::filesystem::path getYoloModelCfgPath() = 0;
            virtual boost::filesystem::path getYoloModelWeightsPath() = 0;

            virtual float getObjectDetectionMinConfidence() = 0;
            virtual float getObjectDetectionNmsThreshold() = 0;
    };

}

#endif // SERVICE_CONFIGURATION_READER