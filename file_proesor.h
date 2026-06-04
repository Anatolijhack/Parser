#pragma once
#include "global.h"
#include "csv_untils.h"
#include "json_untils.h"
#include <functional>
#include "converter.h"
#include "Config.h"
#include "report.h"
#include  "untils.h"
#include "progress.h"
#include "ThreadPool.h"
FileStats analyzeINIFile(const std::string& filename, const std::string& keyword, FileWriter& writer);
FileStats analyzeJSONFile(const std::string& filename, const std::string& keyword, FileStatistic& stat);
FileStats analyzeFile(const std::string& filename, const std::string& keyword, std::string& startTime, std::string& endTime, FileStatistic& stat, FileWriter& writer);
FileStats analyzeCSVFile(const std::string& filename, const std::string& keyword, FileStatistic& stat, FileWriter& writer);
FileStats analyzeByType(const std::string& filename, ProgramOptions& options, FileStatistic& stat, FileWriter& writer);
void parseDirectory(const std::string& directory, const Config& config, ProgramOptions& options, ThreadPool& obj, FileProgres& progres, FileStatistic& stat, FileWriter& writer);