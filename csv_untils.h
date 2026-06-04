#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <map>

std::vector<std::string> CSVsplit(const std::string& line, char delimiter);
char detectDelimiter(const std::string& line);
std::vector<std::map<std::string, std::string>> parseCSVWithHeader(const std::string& filename);
std::string escapeCSV(const std::string& s);
void removeBOM(std::string& line);
std::vector<std::string> splitCSVLine(const std::string& line);
