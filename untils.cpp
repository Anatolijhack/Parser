#include "untils.h"
#include <iostream>
std::string toLower(std::string s) 
{
    for (char& c : s)
    {
        c = tolower(c);
    }
    return s;
}
bool containsKeyword(const std::string& line, const std::string& keyword)
{
    return toLower(line).find(toLower(keyword)) != std::string::npos;
}
std::vector<std::string> smartSplit(const std::string& line)
{
    std::vector<std::string> result;
    std::string current;
    bool inQuotes = false;

    for (char c : line)
    {
        if (c == '"')
        {
            inQuotes = !inQuotes;
            current += c;
        }
        else if (c == ' ' && !inQuotes)
        {
            if (!current.empty())
            {
                result.push_back(current);
                current.clear();
            }
        }
        else
        {
            current += c;
        }
    }

    if (!current.empty())
        result.push_back(current);

    return result;
}
std::string trim(const std::string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
    {
        return "";
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}
bool isValidTime(const std::string& t)
{
    if (t.size() != 8)
    {
        return false;
    }
    return isdigit(t[0]) && isdigit(t[1]) && t[2] == ':' && isdigit(t[3]) && isdigit(t[4]) && t[5] == ':' && isdigit(t[6]) && isdigit(t[7]);
}
int parseHour(const std::string& time)
{
    if (!isValidTime(time))
    {
        return -1;
    }
    int h = stoi(time.substr(0, 2));
    if (h < 0 || h > 23)
    {
        return -1;
    }
    return h;
}
void logMessage(const std::string& message,FileWriter& writer)
{
    std::lock_guard<std::mutex> lock(writer.logMutex);

    std::ofstream log("app.log", std::ios::app);
    if (!log.is_open()) return;

    // время
    auto now = std::chrono::system_clock::now();
    time_t t = std::chrono::system_clock::to_time_t(now);

    tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &t);
#else
    localtime_r(&t, &localTime);
#endif

    log << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S")
        << " | " << message << "\n";
}
bool parseLogLine(const std::string& line, LogEntry& entry)
{
    if (line.empty()) return false;

    // 1️⃣ Дата и время
    size_t pos1 = line.find(' ');
    if (pos1 == std::string::npos) return false;

    size_t pos2 = line.find(' ', pos1 + 1);
    if (pos2 == std::string::npos) return false;

    size_t pos3 = line.find(' ', pos2 + 1);
    if (pos3 == std::string::npos) return false;

    entry.date = line.substr(0, pos1);
    entry.time = line.substr(pos1 + 1, pos2 - pos1 - 1);
    entry.level = line.substr(pos2 + 1, pos3 - pos2 - 1);

    // 2️⃣ Ищем id= с конца строки
    size_t idPos = line.rfind(" id=");
    if (idPos != std::string::npos)
    {
        // Преобразуем id в число
        std::string idStr = line.substr(idPos + 4); // после "id="
        try { entry.id = std::stoi(idStr); }
        catch (...) { entry.id = -1; }

        // Всё между level и id= — это действие
        entry.action = line.substr(pos3 + 1, idPos - pos3 - 1);
    }
    else
    {
        entry.id = -1;
        entry.action = line.substr(pos3 + 1); // до конца строки
    }

    return true;
}
bool isTimeInRange(const std::string& logTime, const std::string& start, const std::string& end)
{
    return logTime >= start && logTime <= end;
}
std::string getExtension(const std::string& name) {
    size_t pos = name.find_last_of('.');
    if (pos == std::string::npos) return "";
    return name.substr(pos + 1);
}
void logError(const std::string& msg, FileStatistic& stat,FileWriter& writer)
{
    std::lock_guard<std::mutex> lock(stat.errorMutex);
    if (writer.errorLog.is_open())
        writer.errorLog << msg << "\n";
    else
        std::cerr << "ErrorLog not open! Message: " << msg << "\n";
}
std::vector<std::pair<std::string, int>> getTopErrors(int topN,FileStatistic& stat)
{
    std::vector<std::pair<std::string, int>> vec(stat.errorStats.begin(), stat.errorStats.end());
    sort(vec.begin(), vec.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) { return a.second > b.second; });
    if (vec.size() > topN)
    {
        vec.resize(topN);
    }
    return vec;
}