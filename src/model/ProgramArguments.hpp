#ifndef MODEL_PROGRAM_ARGUMENTS
#define MODEL_PROGRAM_ARGUMENTS

#include <boost/filesystem.hpp>

namespace model {

    class ProgramArguments {
        public:
            boost::filesystem::path configurationPath;
            boost::filesystem::path videoPath;
        
        public:
            ProgramArguments() {}

            ProgramArguments(boost::filesystem::path configurationPath, boost::filesystem::path videoPath) :
                configurationPath(configurationPath), videoPath(videoPath) {}
    };
}

#endif // MODEL_PROGRAM_ARGUMENTS