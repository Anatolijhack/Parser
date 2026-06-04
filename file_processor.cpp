#include "file_proesor.h"
#include <fstream>
#include "converter.h"
#include <iostream>
FileStats analyzeJSONFile(const std::string& filename, const std::string& keyword,FileStatistic& stat) {
    FileStats stats;

    std::ifstream file(filename);
    if (!file.is_open()) return stats;

    std::string lowerKeyword = toLower(keyword);
    std::string buffer;
    buffer.reserve(64 * 1024); // 64KB буфер
    char ch;

    int braceLevel = 0;
    bool inString = false;
    bool escape = false;

    std::function<void(const json&)> traverse = [&](const json& obj) {
        if (obj.is_primitive()) {
            stats.lines++;

            std::string s = obj.dump();
            std::vector<std::string> words = smartSplit(s);
            stats.words += words.size();

            if (containsKeyword(s, keyword)) {
                stats.keywordCount++;
                std::lock_guard<std::mutex> lock(stat.errorMutex);
                stat.keywordStats[s]++;
            }
        }
        else if (obj.is_array()) {
            for (auto& el : obj)
                traverse(el);
        }
        else if (obj.is_object()) {
            std::string levelValue;
            std::string messageValue;

            for (auto& el : obj.items()) {
                std::string key = toLower(el.key());
                std::string valStr = el.value().is_string() ? el.value().get<std::string>() : el.value().dump();

                if (containsKeyword(valStr, keyword)) {
                    stats.keywordCount++;
                    std::lock_guard<std::mutex> lock(stat.errorMutex);
                    stat.keywordStats[valStr]++;
                }

                if (key == "level") levelValue = valStr;
                if (key == "message") messageValue = valStr;

                traverse(el.value());
            }

            if (!levelValue.empty()) {
                std::lock_guard<std::mutex> lock(stat.levelMutex);
                stat.levelStats[levelValue]++;
            }

            if (toLower(levelValue).find(lowerKeyword) != std::string::npos && !messageValue.empty()) {
                std::lock_guard<std::mutex> lock(stat.errorMutex);
                stat.errorStats[messageValue]++;
            }
        }
        };

    // 🔥 Потоковое чтение по символам
    while (file.get(ch)) {
        buffer += ch;

        if (ch == '"' && !escape) inString = !inString;
        if (ch == '\\' && !escape) { escape = true; continue; }
        escape = false;

        if (!inString) {
            if (ch == '{' || ch == '[') braceLevel++;
            if (ch == '}' || ch == ']') braceLevel--;

            if (braceLevel == 0 && !buffer.empty()) {
                try {
                    json j = json::parse(buffer);
                    traverse(j);
                    buffer.clear(); // очистка после успешного парсинга
                }
                catch (...) {
                    // буфер не полный — продолжаем читать
                }
            }
        }
    }

    // 🔹 Обработка последнего объекта в буфере
    if (!buffer.empty()) {
        try {
            json j = json::parse(buffer);
            traverse(j);
        }
        catch (...) {}
    }

    return stats;
}
FileStats analyzeFile(const std::string& filename, const std::string& keyword, std::string& startTime , std::string& endTime,FileStatistic& stat,FileWriter& writer)
{
    std::ifstream file(filename);
    FileStats stats;

    if (!file.is_open())
    {
        std::lock_guard<std::mutex> lock(stat.errorMutex);
        std::string err = "Failed to open file: " + filename;
        logMessage(err,writer);
        stat.errorStats[err]++; // 🔥 важно
        return stats;
    }

    std::string line;
    std::string lowerKeyword = toLower(keyword);

    while (getline(file, line))
    {
        if (line.empty()) continue;

        // ===== Lines & Words =====
        stats.lines++;
        std::vector<std::string> words = smartSplit(line);
        stats.words += words.size();

        // ===== Keyword =====
        bool hasKeyword = toLower(line).find(lowerKeyword) != std::string::npos;
        if (hasKeyword)
            stats.keywordCount++;

        // 🔹 Keyword Stats (как errorStats)
        if (hasKeyword)
        {
            std::lock_guard<std::mutex> lock(stat.errorMutex);
            stat.keywordStats[line]++; // собираем строки с ключевым словом
        }

        // ===== Parse log =====
        LogEntry entry;
        if (parseLogLine(line, entry) && isTimeInRange(entry.time, startTime, endTime))
        {
            // LEVEL
            {
                std::lock_guard<std::mutex> lock(stat.levelMutex);
                stat.levelStats[entry.level]++;
            }

            // HOUR
            int h = parseHour(entry.time);
            if (h != -1)
            {
                std::string hourStr = (h < 10 ? "0" : "") + std::to_string(h);

                {
                    std::lock_guard<std::mutex> lock(stat.hourMutex);
                    stat.hourStats[hourStr]++;
                }

                // ERRORS
                std::string lvlLower = toLower(entry.level);
                std::string actionLower = toLower(entry.action);

                if (lvlLower.find(lowerKeyword) != std::string::npos || actionLower.find(lowerKeyword) != std::string::npos)
                {
                    std::string actionKey = entry.action.empty() ? "(empty)" : entry.action;

                    std::lock_guard<std::mutex> lock(stat.errorMutex);
                    stat.errorStats[actionKey]++;
                    stat.hourErrorDetails[hourStr][actionKey]++;
                }
            }
        }
    }

    return stats;
}

FileStats analyzeINIFile(const std::string& filename, const std::string& keyword,FileWriter& writer)
{
    std::ifstream file(filename);
    FileStats stats;
    std::mutex errorMutex;
    std::map<std::string, int> errorStats;

    if (!file.is_open()) {
        logMessage("Failed to open INI: " + filename,writer);
        return stats;
    }

    std::string line;
    std::string lowerKeyword = toLower(keyword);

    while (getline(file, line))
    {
        if (line.empty() || line[0] == ';') continue;

        stats.lines++;

        std::vector<std::string> words = smartSplit(line);
        stats.words += words.size();

        if (toLower(line).find(lowerKeyword) != std::string::npos)
            stats.keywordCount++;
    }

    return stats;
}
FileStats analyzeCSVFile(const std::string& filename, const std::string & keyword,FileStatistic& stat,FileWriter& writer)
{
    std::ifstream file(filename);
    FileStats stats;
    std::map<std::string, int> localKeywordStats;

    if (!file.is_open()) {
        logMessage("Failed to open CSV: " + filename,writer);
        return stats;
    }

    std::string line;
    std::string lowerKeyword = toLower(keyword);

    // Читаем заголовок
    if (!getline(file, line)) return stats;
    removeBOM(line);
    char delimiter = detectDelimiter(line);
    std::vector<std::string> headers = CSVsplit(line, delimiter);

    int levelIndex = -1, messageIndex = -1;
    for (int i = 0; i < headers.size(); i++) {
        std::string h = toLower(headers[i]);
        if (h == "level") levelIndex = i;
        if (h == "message") messageIndex = i;
    }

    // Читаем строки
    while (getline(file, line)) {
        if (line.empty()) continue;

        std::vector<std::string> cols = CSVsplit(line, delimiter);
        stats.lines++;
        stats.words += cols.size();

        bool lineHasKeyword = false;
        for (auto& col : cols) {
            if (toLower(col).find(lowerKeyword) != std::string::npos) {
                stats.keywordCount++;
                lineHasKeyword = true;
            }
        }

        // 🔹 собираем статистику по строкам с ключевым словом
        if (lineHasKeyword) {
            localKeywordStats[line]++;
        }

        // 🔥 обработка ERROR
        if (levelIndex != -1 && levelIndex < cols.size()) {
            std::string level = toLower(cols[levelIndex]);
            if (level.find(lowerKeyword) != std::string::npos) {
                std::string message = (messageIndex != -1 && messageIndex < cols.size())
                    ? cols[messageIndex]
                    : "UNKNOWN_ERROR";
                std::lock_guard<std::mutex> lock(stat.errorMutex);
                stat.errorStats[message]++;
            }
        }
    }

    // 🔹 объединяем локальную статистику ключевого слова с глобальной
    if (!localKeywordStats.empty()) {
        std::lock_guard<std::mutex> lock(stat.errorMutex);
        for (auto& [lineText, count] : localKeywordStats)
            stat.keywordStats[lineText] += count;
    }

    return stats;
}
FileStats analyzeByType(const std::string& filename, ProgramOptions& options,FileStatistic& stat,FileWriter& writer)
{
    std::string ext = toLower(getExtension(filename));
    if (ext == "log" || ext == "txt")
        return analyzeFile(filename, options.keyword, options.startTime, options.endTime,stat,writer);

    if (ext == "csv")
        return analyzeCSVFile(filename, options.keyword,stat,writer);

    if (ext == "json")
        return analyzeJSONFile(filename, options.keyword,stat);

    if (ext == "ini")
        return analyzeINIFile(filename, options.keyword,writer);

    return {};
}
//void parseDirectory(const std::string& directory, const Config& config,ProgramOptions& options)
//{
//    std::vector<std::string> filesToProcess;
//
//    // Собираем все файлы рекурсивно
//    for (const auto& entry : fs::recursive_directory_iterator(directory))
//        if (fs::is_regular_file(entry.path()))
//            filesToProcess.push_back(entry.path().string());
//
//    totalFiles = static_cast<int>(filesToProcess.size());
//    processed = 0;
//
//    // Заголовок таблицы в консоль
//    printStatsHeader();
//
//    std::vector<std::thread> threads;
//
//    for (const auto& filename : filesToProcess)
//    {
//        threads.emplace_back([&, filename]()
//            {
//                try
//                {
//                    FileStats stats = analyzeByType(filename, options);
//
//                    // ===== Вывод в консоль =====
//                    printStatsRow(filename, stats);
//
//                    // ===== Запись в отчёты =====
//                    WriteOutput(options, filename, stats);
//                }
//                catch (const std::exception& e)
//                {
//                    logError("[ERROR] File failed: " + filename + " | " + e.what());
//                }
//                catch (...)
//                {
//                    logError("[ERROR] Unknown error with file: " + filename);
//                }
//
//                // ===== Прогресс =====
//                processed.fetch_add(1);
//                showProgress();
//            });
//
//        // Ждём, если достигнут лимит потоков
//        if (threads.size() >= config.maxThreads)
//        {
//            for (auto& t : threads) t.join();
//            threads.clear();
//        }
//    }
//
//    // Ждём завершения оставшихся потоков
//    for (auto& t : threads) t.join();
//
//    // Перевод строки после прогресса
//    std::cout << "\nProcessing complete for directory: " << directory << "\n";
//}

void parseDirectory(const std::string& directory, const Config& config, ProgramOptions& options,ThreadPool& obj,FileProgres& progres,FileStatistic& stat,FileWriter& writer)
{
    std::vector<std::string> filesToProcess;

    // Собираем все файлы рекурсивно
    for (const auto& entry : fs::recursive_directory_iterator(directory))
        if (fs::is_regular_file(entry.path()))
            filesToProcess.push_back(entry.path().string());

    progres.totalFiles = static_cast<int>(filesToProcess.size());
    progres.processed = 0;

    // Заголовок таблицы в консоль
    printStatsHeader(progres);

   

    for (const auto& filename : filesToProcess)
    {
        obj.enqueue([&, filename]()
            {
                try
                {
                    FileStats stats = analyzeByType(filename, options,stat,writer);

                    // ===== Вывод в консоль =====
                    printStatsRow(filename, stats,progres);

                    // ===== Запись в отчёты =====
                    WriteOutput(options, filename, stats,writer);
                }
                catch (const std::exception& e)
                {
                    logError("[ERROR] File failed: " + filename + " | " + e.what(),stat,writer);
                }
                catch (...)
                {
                    logError("[ERROR] Unknown error with file: " + filename,stat,writer);
                }

                // ===== Прогресс =====
                progres.processed.fetch_add(1);
                showProgress(progres);
        });
    }
    // Перевод строки после прогресса
    std::cout << "\nProcessing complete for directory: " << directory << "\n";
}
