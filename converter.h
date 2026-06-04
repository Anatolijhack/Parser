#pragma once
#include <string>
#include "global.h"
#include "nlohmann/json.hpp"
#include "json_untils.h"
#include "csv_untils.h"
#include <sstream>
using json = nlohmann::json;
void exportTXTtoJSON(const std::string& txtFile, const std::string& jsonFileName);
void jsonToCSV(const std::string& jsonFile, const std::string& csvFile);
void txtToCSV(const std::string& txtFile, const std::string& csvFile);
void csvToJSON(const std::string& csvFile, const std::string& jsonFile);
void CSVtoTXT(const std::string& csvFile, const std::string& txtFile);
void JSONtoTXT(const std::string& jsonFile, const std::string& txtFile);