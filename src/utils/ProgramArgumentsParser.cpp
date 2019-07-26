#include <iostream>
#include <stdexcept>
#include <boost/program_options.hpp>
#include "ProgramArgumentsParser.hpp"

using namespace model;
using namespace utils;
using std::cerr;
using std::cout;
using std::runtime_error;
using std::string;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

model::ProgramArguments ProgramArgumentsParser::parse(int argc, char* argv[]) const {
    po::options_description programDesc("Allowed options");
    programDesc.add_options()
        ("help", "produce help message")
        ("config-path", po::value<string>(), "path to the config.ini file")
        ("video-path", po::value<string>(), "path to the video file");
    
    po::variables_map varsMap;
    po::store(po::parse_command_line(argc, argv, programDesc), varsMap);
    po::notify(varsMap);

    if (varsMap.count("help")) {
        cout << programDesc << "\n";
        throw runtime_error("Missing arguments.");
    }

    fs::path configurationPath;
    if (varsMap.count("config-path")) {
        string relativeConfigurationPath = varsMap["config-path"].as<string>();
        configurationPath = fs::canonical(fs::path(relativeConfigurationPath));
    } else {
        cerr << "--config-path not set. See --help for more info.\n";
        throw runtime_error("Missing argument: --config-path");
    }

    fs::path videoPath;
    if (varsMap.count("video-path")) {
        string relativeVideoPath = varsMap["video-path"].as<string>();
        videoPath = fs::canonical(fs::path(relativeVideoPath));
    } else {
        cerr << "--video-path not set. See --help for more info.\n";
        throw runtime_error("Missing argument: --video-path");
    }

    return ProgramArguments(configurationPath, videoPath);
}