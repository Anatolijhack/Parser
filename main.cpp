#include "args.h"
#include "Config.h"
#include "converter.h"
#include "csv_untils.h"
#include "file_proesor.h"
#include "global.h"
#include "json_untils.h"
#include "progress.h"
#include "report.h"
#include "untils.h"
#include "Windows.h"
#include <iostream>
#include "ThreadPool.h"
int toSeconds(const std::string& t)
{
    int h = 0, m = 0, s = 0;
    sscanf(t.c_str(), "%d:%d:%d", &h, &m, &s);
    return h * 3600 + m * 60 + s;
}

bool isInRange(const std::string& time,
    const std::string& from,
    const std::string& to)
{
    if (from.empty() && to.empty())
        return true;

    int t = toSeconds(time);

    if (!from.empty() && t < toSeconds(from))
        return false;

    if (!to.empty() && t > toSeconds(to))
        return false;

    return true;
}
int main(int argc, char* argv[])
{
    // ===== UTF-8 =====
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // ===== Парсинг аргументов =====
    ProgramOptions options = parseArguments(argc, argv);

    // ===== Конфиг =====
    Config config = loadConfig("config.ini");

    ThreadPool pool(config.maxThreads);
    FileWriter writer;
    FileStatistic stat;
    FileProgres proces;

    // fallback значения (если не заданы через CLI)
    if (options.keyword.empty())
        options.keyword = config.keyword;

    if (options.outputFile.empty())
        options.outputFile = "report";

    if (options.format.empty())
        options.format = "txt";

    // ===== Проверка пути =====
    if (!fs::exists(options.path))
    {
        std::cout << "Invalid path: " << options.path << "\n";
        return 1;
    }

    // ===== Открытие отчётов =====

    if (options.format == "txt")
        writer.txtFile.open(options.outputFile + ".txt", std::ios::out);

    if (!writer.csVFile.is_open())
    {
        writer.csVFile.open(options.outputFile + ".csv", std::ios::out);
        writer.csvHeaderWritten = false;
    }

    writer.jsonFile.open(options.outputFile + ".json", std::ios::out);
    if (writer.jsonFile.is_open())
        writer.jsonFile << "{\n  \"files\": [\n";

    // ===== Обработка =====
    if (fs::is_directory(options.path))
    {
        parseDirectory(options.path, config, options, pool, proces, stat, writer);
        pool.waitFinished();
    }
    else if (fs::is_regular_file(options.path))
    {
        // если у тебя есть лог-время — оно должно извлекаться внутри analyzeByType
        FileStats stats = analyzeByType(options.path, options, stat, writer);

        // пример: если у тебя есть timestamp результата
        if (!options.startTime.empty() || !options.endTime.empty())
        {
            if (!isInRange(stats.timestamp, options.startTime, options.endTime))
            {
                std::cout << "Skipped by time filter: " << options.path << "\n";
                return 0;
            }
        }

        printStatsRow(options.path, stats, proces);
        WriteOutput(options, options.path, stats, writer);
    }
    else
    {
        std::cout << "Invalid path type\n";
        return 1;
    }

    // ===== Финализация =====
    finalizeReports(options, writer, stat);

    // ===== Закрытие файлов =====
    if (writer.txtFile.is_open()) writer.txtFile.close();
    if (writer.csVFile.is_open()) writer.csVFile.close();
    if (writer.jsonFile.is_open()) writer.jsonFile.close();

    std::cout << "\nDone!\n";
    return 0;
}