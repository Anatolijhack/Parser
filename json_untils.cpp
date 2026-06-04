#include "json_untils.h"

void WriteSummaryJson(std::ofstream& jsonFile,FileStatistic& stat,FileWriter& writer) {
    std::lock_guard<std::mutex> lock(writer.jsonMutex);

    if (!jsonFile.is_open()) return;

    jsonFile << "{\n";

    int totalErrors = 0;
    for (auto& [msg, count] : stat.errorStats)
        totalErrors += count;

    jsonFile << "  \"summary\": {\n";
    jsonFile << "    \"total_levels\": " << stat.levelStats.size() << ",\n";
    jsonFile << "    \"total_errors\": " << totalErrors << ",\n";
    jsonFile << "    \"unique_errors\": " << stat.errorStats.size() << "\n";
    jsonFile << "  },\n";

    jsonFile << "  \"top_errors\": [\n";
    auto top = getTopErrors(10,stat);
    for (size_t i = 0; i < top.size(); i++) {
        jsonFile << "    {\"error\": \"" << escapeJson(top[i].first)
            << "\", \"count\": " << top[i].second << "}";
        if (i + 1 < top.size()) jsonFile << ",";
        jsonFile << "\n";
    }
    jsonFile << "  ]\n";

    jsonFile << "}\n";
}
std::string escapeJson(const std::string& s)
{
    std::string result;
    for (char c : s)
    {
        if (c == '\"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else if (c == '\r') result += "\\r";
        else if (c == '\t') result += "\\t";
        else result += c;
    }
    return result;
}