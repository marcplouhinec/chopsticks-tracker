#ifndef UTILS_PROGRAM_ARGUMENT_PARSER
#define UTILS_PROGRAM_ARGUMENT_PARSER

#include "../model/ProgramArguments.hpp"

namespace utils {

    class ProgramArgumentsParser {
        public:
            model::ProgramArguments parse(int argc, char* argv[]) const;
    };

}

#endif // UTILS_PROGRAM_ARGUMENT_PARSER