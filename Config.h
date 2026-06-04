#pragma once
#include <filesystem>
#include <string>
namespace fs = std::filesystem;
struct Config
{
    int maxThreads = 4;
    int maxFileSizeMB = 10;
    std::string keyword = "ERROR";
};
struct FileStats 
{
    int lines = 0;
    int words = 0;
    int keywordCount = 0;
    int objects;
    std::string timestamp;
};
struct LogEntry
{
    std::string date;
    std::string time;
    std::string level;
    std::string action;
    int id = -1;
};

Config loadConfig(const std::string& filename);