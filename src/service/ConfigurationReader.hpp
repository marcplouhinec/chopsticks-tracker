#ifndef SERVICE_CONFIGURATION_READER
#define SERVICE_CONFIGURATION_READER

#include <boost/filesystem.hpp>
#include "../model/Configuration.hpp"

namespace service {

    class ConfigurationReader {
        public:
            virtual ~ConfigurationReader() {}

            virtual model::Configuration read(boost::filesystem::path configurationPath) = 0;
    };

}

#endif // SERVICE_CONFIGURATION_READER