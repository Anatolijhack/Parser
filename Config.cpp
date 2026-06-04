#include "Config.h"
#include <fstream>
#include <iostream>


Config loadConfig(const std::string& filename) {
    Config config;

    // Если файла нет — создаём с дефолтными значениями
    if (!fs::exists(filename)) {
        std::ofstream createFile(filename);
        if (createFile.is_open()) {
            createFile << "; Configuration file for parser\n";
            createFile << "max_threads=4\n";
            createFile << "max_file_size=10\n";
            createFile << "keyword=ERROR\n";
            createFile.close();
            std::cout << "Config file not found. Created default: " << filename << "\n";
        }
        return config; // возвращаем дефолтный config
    }

    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Cannot open config file: " << filename << "\n";
        return config;
    }

    std::string line;
    while (getline(file, line)) {
        // Убираем пробелы в начале и конце
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        // Пропускаем пустые строки и комментарии
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;

        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        // Убираем пробелы
        key.erase(0, key.find_first_not_of(" \t\r\n"));
        key.erase(key.find_last_not_of(" \t\r\n") + 1);
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);

        if (key == "max_threads") {
            try {
                int v = stoi(value);
                if (v > 0) config.maxThreads = v;
            }
            catch (...) {  }
        }
        else if (key == "max_file_size") {
            try {
                int v = stoi(value);
                if (v > 0) config.maxFileSizeMB = v;
            }
            catch (...) {  }
        }
        else if (key == "keyword") {
            if (!value.empty()) config.keyword = value;
        }
    }

    return config;
}