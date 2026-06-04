#pragma once
#include "global.h"
#include <iomanip>
#include <string>
#include <vector>
#include "Config.h"
#include <map>

std::string toLower(std::string s);
bool containsKeyword(const std::string& line, const std::string& keyword);
std::string trim(const std::string& s);
std::vector<std::string> smartSplit(const std::string& line);
int parseHour(const std::string& time);
bool isValidTime(const std::string& t);
void logMessage(const std::string& message, FileWriter& writer);
bool parseLogLine(const std::string& line, LogEntry& entry);
bool isTimeInRange(const std::string& logTime, const std::string& start, const std::string& end);
std::string getExtension(const std::string& name);
void logError(const std::string& msg, FileStatistic& stat, FileWriter& writer);
std::vector<std::pair<std::string, int>> getTopErrors(int topN, FileStatistic& stat);

