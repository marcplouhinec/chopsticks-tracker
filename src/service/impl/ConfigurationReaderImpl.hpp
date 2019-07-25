#ifndef SERVICE_CONFIGURATION_READER_IMPL
#define SERVICE_CONFIGURATION_READER_IMPL

#include "../../utils/logging.hpp"
#include "../ConfigurationReader.hpp"

namespace service {

    class ConfigurationReaderImpl : public ConfigurationReader {
        private:
            boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger;

        public:
            virtual ~ConfigurationReaderImpl() {}

            virtual model::Configuration read(boost::filesystem::path configurationPath);
    };

}

#endif // SERVICE_CONFIGURATION_READER_IMPL