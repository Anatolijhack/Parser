#pragma once
#include "args.h"
#include "Config.h"
#include "json_untils.h"
#include "untils.h"
#include <string>



void writeReport(std::ofstream& report, const std::string& filename, const FileStats& stats);
void writeCSVReport(const std::string& filename, const FileStats& stats, FileWriter& stat);
void writeJSONReportSafe(const std::string& filename, const FileStats& stats, FileWriter& stat);
void writeCSVSummaryReport(const std::string& filename, FileStatistic& stat);
void WriteOutput(const ProgramOptions& options, const std::string& filename, const FileStats& stats, FileWriter& writer);
void writeErrorReportTXT(const std::string& fileName, FileWriter& writer, FileStatistic& stat);
void writeErrorReportCSV(const std::string& fileName, FileWriter& wirter, FileStatistic& stat);
void printStatsHeader(FileProgres& writer);
void printStatsRow(const std::string& filename, const FileStats& stats, FileProgres& proces);
void finalizeReports(const ProgramOptions& options, FileWriter& writer, FileStatistic& stat);
void generateHtmlReport(const std::string& filename, FileStatistic& stat);