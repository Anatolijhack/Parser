#include "csv_untils.h"
#include <fstream>
#include <iostream>
std::vector<std::string> CSVsplit(const std::string& line, char delimiter)
{
    std::vector<std::string> result;
    std::string current;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i)
    {
        char c = line[i];

        if (c == '"')
        {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"')
            {
                current += '"'; // escaped quote
                ++i;
            }
            else
            {
                inQuotes = !inQuotes;
            }
        }
        else if (c == delimiter && !inQuotes)
        {
            result.push_back(current);
            current.clear();
        }
        else
        {
            current += c;
        }
    }

    result.push_back(current);
    return result;
}
void removeBOM(std::string& line)
{
    if (line.size() >= 3 &&
        (unsigned char)line[0] == 0xEF &&
        (unsigned char)line[1] == 0xBB &&
        (unsigned char)line[2] == 0xBF)
    {
        line = line.substr(3);
    }
}
char detectDelimiter(const std::string& line)
{
    if (line.find(';') != std::string::npos) return ';';
    if (line.find(',') != std::string::npos) return ',';
    if (line.find(';') != std::string::npos) return ';';
    if (line.find('\t') != std::string::npos) return '\t';
    return ','; // fallback
}
std::vector<std::map<std::string, std::string>> parseCSVWithHeader(const std::string& filename)
{
    std::ifstream file(filename);
    std::vector<std::map<std::string, std::string>> result;

    if (!file)
    {
        std::cout << "Cannot open file: " << filename << std::endl;
        return result;
    }

    std::string line;

    // Читаем заголовки
    if (!getline(file, line)) return result;

    removeBOM(line); // убираем BOM, если есть

    char delimiter = detectDelimiter(line);
    std::vector<std::string> headers = CSVsplit(line, delimiter);

    auto trim = [](std::string& s) {
        s.erase(0, s.find_first_not_of(" \t\r\n"));
        s.erase(s.find_last_not_of(" \t\r\n") + 1);
        };

    // Читаем строки
    while (getline(file, line))
    {
        if (line.empty()) continue;

        std::vector<std::string> values = CSVsplit(line, delimiter);
        std::map<std::string, std::string> row;

        for (size_t i = 0; i < headers.size(); i++)
        {
            std::string key = headers[i];
            std::string value = (i < values.size()) ? values[i] : "";

            trim(key);
            trim(value);

            // убираем кавычки вокруг значения, если есть
            if (!value.empty() && value.front() == '"' && value.back() == '"')
                value = value.substr(1, value.size() - 2);

            // экранированные двойные кавычки в значении
            size_t pos = 0;
            while ((pos = value.find("\"\"", pos)) != std::string::npos)
            {
                value.replace(pos, 2, "\"");
                pos++;
            }

            row[key] = value;
        }

        result.push_back(row);
    }

    return result;
}
std::string escapeCSV(const std::string& s) {
    std::string escaped = "\"";
    for (char c : s) {
        if (c == '"') escaped += "\"\""; // двойные кавычки внутри строки
        else escaped += c;
    }
    escaped += "\"";
    return escaped;
}
std::vector<std::string> splitCSVLine(const std::string& line) {
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string field;

    while (getline(ss, field, ',')) {
        // Убираем кавычки вокруг поля, если есть
        if (!field.empty() && field.front() == '"' && field.back() == '"') {
            field = field.substr(1, field.size() - 2);
        }
        result.push_back(field);
    }
    return result;
}