#pragma once
#include <string>
struct ProgramOptions
{
    std::string path;
    std::string keyword = "ERROR";
    std::string startTime = "00:00:00";
    std::string endTime = "23:59:59";
    std::string outputFile = "report";
    std::string format = "txt";
    bool toJson = false;
   
};
ProgramOptions parseArguments(int argc, char* argv[]);
void  printHelp();