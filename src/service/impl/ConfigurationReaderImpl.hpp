#ifndef SERVICE_CONFIGURATION_READER_IMPL
#define SERVICE_CONFIGURATION_READER_IMPL

#include "../../utils/logging.hpp"
#include "../ConfigurationReader.hpp"

namespace service {

    class ConfigurationReaderImpl : public ConfigurationReader {
        public:
            virtual ~ConfigurationReaderImpl() {}

            virtual model::Configuration read(boost::filesystem::path configurationPath) const;
    };

}

#endif // SERVICE_CONFIGURATION_READER_IMPL