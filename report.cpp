#include "report.h"
#include <iostream>


void writeCSVReport(const std::string & filename, const FileStats& stats,FileWriter& stat)
{
    std::lock_guard<std::mutex> lock(stat.csvMutex);
    
    if (!stat.csVFile.is_open()) return;

    if (!stat.csvHeaderWritten)
    {
        stat.csVFile << "File,Lines,Words,KeywordMatches\n";
        stat.csvHeaderWritten = true;
    }

    stat.csVFile << filename << ","
        << stats.lines << ","
        << stats.words << ","
        << stats.keywordCount << "\n";

    stat.csVFile.flush(); // обязательно сброс потока
}
void writeJSONReportSafe(const std::string& filename, const FileStats& stats,FileWriter& stat)
{
    std::lock_guard<std::mutex> lock(stat.jsonMutex);

    if (!stat.jsonFile.is_open()) return;

    // добавляем запятую, если это не первый файл
    if (!stat.firstJsonEntry) stat.jsonFile << ",\n";

    stat.jsonFile << "    {\n";
    stat.jsonFile << "      \"file\": \"" << escapeJson(filename) << "\",\n";
    stat.jsonFile << "      \"lines\": " << stats.lines << ",\n";
    stat.jsonFile << "      \"words\": " << stats.words << ",\n";
    stat.jsonFile << "      \"keyword_matches\": " << stats.keywordCount << "\n";
    stat.jsonFile << "    }";

    stat.firstJsonEntry = false;
}
void WriteOutput(const ProgramOptions& options, const std::string& filename, const FileStats& stats,FileWriter& writer)
{
    if (options.format == "txt" && writer.txtFile.is_open())
        writeReport(writer.txtFile, filename, stats);

    if (writer.csVFile.is_open()) // используем глобальный поток
        writeCSVReport(filename, stats,writer);


    if (writer.jsonFile.is_open()) // JSON всегда пишем
        writeJSONReportSafe(filename, stats,writer);
}
void printStatsHeader(FileProgres& writer)
{
    
    std::lock_guard<std::mutex> lock(writer.coutMutex);
    std::cout << std::left << std::setw(50) << "File"
        << std::setw(10) << "Lines"
        << std::setw(10) << "Words"
        << std::setw(15) << "Keyword matches"
        << "\n";
    std::cout << std::string(85, '-') << "\n";
}
void printStatsRow(const std::string& filename, const FileStats& stats,FileProgres& proces) {
    std::lock_guard<std::mutex> lock(proces.coutMutex);
    std::cout << std::left << std::setw(50) << filename
        << std::setw(10) << stats.lines
        << std::setw(10) << stats.words
        << std::setw(15) << stats.keywordCount
        << "\n";
}
void writeReport(std::ofstream& report, const std::string& filename, const FileStats& stats) {
    report << std::left << std::setw(30) << filename
        << std::setw(10) << stats.lines
        << std::setw(10) << stats.words
        << std::setw(10) << stats.keywordCount
        << "\n";
}
void writeINIReport(const ProgramOptions& options)
{
    std::string iniName = options.outputFile + ".ini";
    std::ofstream iniFile(iniName);
    if (!iniFile.is_open()) return;

    iniFile << "[Settings]\n";
    iniFile << "path=" << options.path << "\n";
    iniFile << "format=" << options.format << "\n";
    iniFile << "keyword=" << options.keyword << "\n";
    iniFile << "toJson=" << (options.toJson ? "true" : "false") << "\n";

    iniFile.close();
    std::cout << "INI report saved: " << iniName << "\n";
}
void writeCSVSummaryReport(const std::string& filename,FileStatistic& stat)
{
    std::ofstream csVFile(filename);
    if (!csVFile.is_open()) return;

    // Level Stats
    csVFile << "Level,Count\n";
    for (auto& p : stat.levelStats)
        csVFile << (p.first.empty() ? "(empty)" : p.first) << "," << p.second << "\n";

    // Top Errors
    csVFile << "\nError,Count\n";
    auto top = getTopErrors(10,stat);
    for (auto& p : top)
        csVFile << p.first << "," << p.second << "\n";

    // Errors by hour
    csVFile << "\nHour,Error,Count\n";
    for (auto& h : stat.hourErrorDetails)
        for (auto& e : h.second)
            csVFile << h.first << "," << e.first << "," << e.second << "\n";

    csVFile.close();

}
void writeErrorReportTXT(const std::string& fileName,FileWriter& writer,FileStatistic& stat) {
    std::lock_guard<std::mutex> lock(writer.jsonMutex);

    std::ofstream txtFile(fileName);
    if (!txtFile.is_open()) return;

    // Заголовок
    txtFile << "==== Ошибки проекта ====\n\n";

    // Топ ошибок
    txtFile << "=== Топ ошибок (10) ===\n";
    auto top = getTopErrors(10,stat);
    for (auto& [msg, count] : top) {
        txtFile << msg << " : " << count << "\n";
    }

    // Количество уникальных ошибок
    txtFile << "\n=== Уникальные ошибки ===\n";
    txtFile << "Всего уникальных ошибок: " << stat.errorStats.size() << "\n";

    txtFile.close();
}
void writeErrorReportCSV(const std::string& fileName,FileWriter& wirter,FileStatistic& stat)
{
    std::lock_guard<std::mutex> lock(wirter.jsonMutex);

    std::ofstream csvFile(fileName);
    if (!csvFile.is_open()) return;

    // Заголовки CSV
    csvFile << "Type,Error,Count\n";

    // Топ ошибок
    auto top = getTopErrors(10,stat);
    for (auto& [msg, count] : top) {
        csvFile << "Top,\"" << escapeJson(msg) << "\"," << count << "\n";
    }

    // Количество уникальных ошибок
    csvFile << "Summary,Unique Errors," << stat.errorStats.size() << "\n";

    csvFile.close();
}
void generateHtmlReport(const std::string& filename,FileStatistic& stat)
{
    std::ofstream html(filename);

    if (!html.is_open()) {
        std::cout << "Failed to create HTML report\n";
        return;
    }

    html << "<!DOCTYPE html>";
    html << "<html><head><meta charset='UTF-8'>";
    html << "<title>Log Report</title>";

    // ===== СТИЛИ =====
    html << "<style>";
    html << "body { font-family: Arial; background:#f5f5f5; padding:20px; }";
    html << "h1 { color:#333; }";
    html << "h2 { margin-top:30px; }";
    html << "table { border-collapse: collapse; width: 70%; background:white; }";
    html << "th, td { border: 1px solid #ccc; padding: 8px; text-align:left; }";
    html << "th { background:#eee; }";

    // Цвета уровней
    html << ".ERROR { color:red; font-weight:bold; }";
    html << ".WARN { color:orange; font-weight:bold; }";
    html << ".INFO { color:green; }";
    html << "</style>";

    html << "</head><body>";

    html << "<h1>Log Analysis Report</h1>";

    // ===== LEVEL STATS =====
    html << "<h2>Level Statistics</h2>";
    html << "<table>";
    html << "<tr><th>Level</th><th>Count</th></tr>";

    for (auto& p : stat.levelStats)
    {
        std::string level = p.first.empty() ? "(empty)" : p.first;

        html << "<tr>";
        html << "<td class='" << level << "'>" << level << "</td>";
        html << "<td>" << p.second << "</td>";
        html << "</tr>";
    }

    html << "</table>";

    // ===== TOP ERRORS =====
    html << "<h2>Top Errors</h2>";
    html << "<table>";
    html << "<tr><th>Error</th><th>Count</th></tr>";

    auto top = getTopErrors(10,stat);
    if (top.empty())
    {
        html << "<tr><td colspan='2'>No errors found</td></tr>";
    }
    else
    {
        for (auto& p : top)
        {
            html << "<tr>";
            html << "<td>" << p.first << "</td>";
            html << "<td>" << p.second << "</td>";
            html << "</tr>";
        }
    }

    html << "</table>";

    // ===== ERRORS BY HOUR =====
    html << "<h2>Errors by Hour</h2>";

    for (auto& h : stat.hourErrorDetails)
    {
        int total = 0;
        for (auto& e : h.second)
            total += e.second;

        html << "<h3>" << h.first << ":00 (" << total << " errors)</h3>";
        html << "<ul>";

        for (auto& e : h.second)
        {
            html << "<li>" << e.first << " -> " << e.second << "</li>";
        }

        html << "</ul>";
    }

    html << "</body></html>";
    html.close();

    std::cout << "HTML report generated: " << filename << "\n";
}
void finalizeReports(const ProgramOptions& options,FileWriter& writer,FileStatistic& stat)
{
    // ===== JSON =====
    if (!writer.jsonFile.is_open()) return;

    // закрываем массив файлов
    writer.jsonFile << "\n  ],\n";

    // summary
    int totalErrors = 0;
    for (auto& [msg, count] : stat.errorStats)
        totalErrors += count;

    writer.jsonFile << "  \"summary\": {\n";
    writer.jsonFile << "    \"total_levels\": " << stat.levelStats.size() << ",\n";
    writer.jsonFile << "    \"total_errors\": " << totalErrors << ",\n";
    writer.jsonFile << "    \"unique_errors\": " << stat.errorStats.size() << "\n";
    writer.jsonFile << "  },\n";

    // top_errors
    writer.jsonFile << "  \"top_errors\": [\n";
    auto top = getTopErrors(10,stat);
    for (size_t i = 0; i < top.size(); i++) {
        writer.jsonFile << "    {\"error\": \"" << escapeJson(top[i].first)
            << "\", \"count\": " << top[i].second << "}";
        if (i + 1 < top.size()) writer.jsonFile << ",";
        writer.jsonFile << "\n";
    }
    writer.jsonFile << "  ]\n";

    // закрываем основной объект
    writer.jsonFile << "}\n";

    writer.jsonFile.close();

    // ===== 🔥 ДОБАВЛЯЕМ ОТЧЁТЫ ОШИБОК =====
    writeErrorReportTXT(options.outputFile + "_errors.txt",writer,stat);
    writeErrorReportCSV(options.outputFile + "_errors.csv",writer,stat);

    // ===== HTML =====
    generateHtmlReport(options.outputFile + ".html",stat);

    // ===== INI =====
    writeINIReport(options);

}