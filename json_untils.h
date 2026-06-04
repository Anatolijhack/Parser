#pragma once
#include <string>
#include "global.h"
#include "untils.h"
#include <vector>
#include <algorithm>

std::string escapeJson(const std::string& s);

void WriteSummaryJson(std::ofstream& jsonFile, FileStatistic& stat, FileWriter& writer);