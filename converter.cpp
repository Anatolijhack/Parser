#include "converter.h"
#include <iostream>
#include <fstream>
#include <set>
void exportTXTtoJSON(const std::string& txtFile, const std::string& jsonFileName) {
    std::ifstream file(txtFile);
    if (!file.is_open()) {
        std::cerr << "Cannot open TXT file: " << txtFile << std::endl;
        return;
    }

    std::ofstream jsonFile(jsonFileName);
    if (!jsonFile.is_open()) {
        std::cerr << "Cannot create JSON file: " << jsonFileName << std::endl;
        return;
    }

    json j = json::array();
    std::string line;
    int processedRows = 0;

    while (getline(file, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string ip, level;
        std::string word;
        std::vector<std::string> words;

        // Читаем IP и уровень
        iss >> ip >> level;

        // Остальные слова сохраняем в вектор
        while (iss >> word) {
            words.push_back(word);
        }

        // Проверка, что есть хотя бы дата и время
        if (words.size() < 3) continue;

        // Сообщение = все слова, кроме последних двух (дата и время)
        std::string message;
        for (size_t i = 0; i < words.size() - 2; ++i) {
            if (i > 0) message += " ";
            message += words[i];
        }

        // Дата и время
        std::string timestamp = words[words.size() - 2] + " " + words[words.size() - 1];

        // Формируем объект JSON
        json obj;
        obj["ip"] = ip;
        obj["level"] = level;
        obj["message"] = message;
        obj["timestamp"] = timestamp;

        j.push_back(obj);
        processedRows++;
    }

    jsonFile << j.dump(4); // красиво с отступами
    std::cout << "TXT to JSON complete: " << jsonFileName
        << " | Processed: " << processedRows << std::endl;
}
void jsonToCSV(const std::string& jsonFile, const std::string& csvFile) {
    std::ifstream in(jsonFile);
    if (!in.is_open()) {
        std::cerr << "Cannot open JSON file: " << jsonFile << std::endl;
        return;
    }

    std::ofstream out(csvFile);
    if (!out.is_open()) {
        std::cerr << "Cannot open CSV file: " << csvFile << std::endl;
        return;
    }

    json j;
    in >> j;

    std::string systemName = j["system"]["name"];

    // 1?? Сначала собираем все ключи из details для заголовка
    std::set<std::string> detailsKeys;
    for (auto& log : j["system"]["logs"]) {
        if (log.contains("details")) {
            for (auto it = log["details"].begin(); it != log["details"].end(); ++it) {
                detailsKeys.insert(it.key());
            }
        }
    }

    // Заголовки CSV
    out << "system_name,level,message,timestamp";
    for (auto& key : detailsKeys) {
        out << "," << key;
    }
    out << "\n";

    // 2?? Заполняем строки CSV
    for (auto& log : j["system"]["logs"]) {
        std::string level = log.value("level", "");
        std::string message = log.value("message", "");
        std::string timestamp = log.value("timestamp", "");

        out << escapeCSV(systemName) << ","
            << escapeCSV(level) << ","
            << escapeCSV(message) << ","
            << escapeCSV(timestamp);

        // Для каждой колонки details вставляем значение или пустое поле
        for (auto& key : detailsKeys) {
            std::string value = "";
            if (log.contains("details") && log["details"].contains(key)) {
                auto v = log["details"][key];
                if (v.is_string()) value = v.get<std::string>();
                else if (v.is_boolean()) value = v.get<bool>() ? "true" : "false";
                else value = v.dump(); // числа и другие типы
            }
            out << "," << escapeCSV(value);
        }
        out << "\n";
    }

    std::cout << "Conversion complete: " << csvFile << std::endl;
}
void txtToCSV(const std::string& txtFile, const std::string& csvFile)
{
    std::ifstream in(txtFile);
    std::ofstream out(csvFile);

    out << "line\n";

    std::string line;
    while (getline(in, line))
    {
        if (!line.empty())
            out << "\"" << line << "\"\n";
    }
}
void csvToJSON(const std::string& csvFile, const std::string& jsonFile)
{
    auto rows = parseCSVWithHeader(csvFile);

    json j = json::array();

    for (auto& row : rows)
    {
        json obj;

        for (auto& [k, v] : row)
            obj[k] = v;

        j.push_back(obj);
    }

    std::ofstream out(jsonFile);
    out << j.dump(4);
}
void CSVtoTXT(const std::string& csvFile, const std::string& txtFile) {
    std::ifstream in(csvFile);
    std::ofstream out(txtFile);

    if (!in.is_open() || !out.is_open()) {
        std::cerr << "Cannot open CSV or TXT file." << std::endl;
        return;
    }

    std::string line;
    std::vector<std::string> headers;

    // Читаем заголовки
    if (getline(in, line)) {
        headers = splitCSVLine(line);
    }
    else {
        std::cerr << "Empty CSV file." << std::endl;
        return;
    }

    while (getline(in, line)) {
        std::vector<std::string> values = splitCSVLine(line);
        std::string ip, level, message, timestamp;

        for (size_t i = 0; i < headers.size(); ++i) {
            if (i >= values.size()) continue;
            std::string key = headers[i];
            std::string val = values[i];

            if (key == "ip") ip = val;
            else if (key == "level") level = val;
            else if (key == "message") message = val;
            else if (key == "timestamp") timestamp = val;
        }

        out << ip << " " << level << " " << message << " " << timestamp << "\n";
    }

    std::cout << "CSV to TXT done: " << txtFile << std::endl;
}
void JSONtoTXT(const std::string& jsonFile, const std::string& txtFile) {
    std::ifstream in(jsonFile);
    std::ofstream out(txtFile);

    if (!in.is_open() || !out.is_open()) {
        std::cerr << "Cannot open JSON or TXT file." << std::endl;
        return;
    }

    json j;
    in >> j;

    for (auto& obj : j) {
        std::string ip = obj.value("ip", "");
        std::string level = obj.value("level", "");
        std::string message = obj.value("message", "");
        std::string timestamp = obj.value("timestamp", "");

        out << ip << " " << level << " " << message << " " << timestamp << "\n";
    }

    std::cout << "JSON to TXT done: " << txtFile << std::endl;
}