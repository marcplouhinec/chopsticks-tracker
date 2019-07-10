#ifndef UTILS_LOGGING
#define UTILS_LOGGING

#include <boost/move/utility_core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

// Define the __SHORT_FILE__ macro (thanks to https://blog.galowicz.de/2016/02/20/short_file_macro/)
static constexpr const char * const past_last_slash(const char * const str, const char * const last_slash) {
    return
        *str == '\0' ? last_slash :
        *str == '/'  ? past_last_slash(str + 1, str + 1) :
                       past_last_slash(str + 1, last_slash);
}

static constexpr const char * const past_last_slash(const char * const str)  { 
    return past_last_slash(str, str);
}

#define __SHORT_FILE__ ({constexpr const char * const sf__ {past_last_slash(__FILE__)}; sf__;})

// Define log macros that add additional informations
#define LOG_INFO(logger) \
    BOOST_LOG_SEV(logger, boost::log::trivial::severity_level::info) << "(" << __SHORT_FILE__ << ", " << __func__ << ", " << __LINE__ << ") "

#define LOG_ERROR(logger) \
    BOOST_LOG_SEV(logger, boost::log::trivial::severity_level::error) << "(" << __SHORT_FILE__ << ", " << __func__ << ", " << __LINE__ << ") "

#endif // UTILS_LOGGING