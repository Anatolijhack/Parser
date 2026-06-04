#pragma once
#include <map>
#include <string>
#include <mutex>
#include <atomic>
#include <string>
#include <fstream>

struct FileStatistic
{
	std::map<std::string, int> levelStats;
	std::map<std::string, int> errorStats;
	std::map<std::string, int> hourStats;
	std::map<std::string, std::map<std::string, int>> hourErrorDetails;
	std::map<std::string, int> keywordStats;
	
	std::mutex reportMutex;
	std::mutex statusMutex;
	std::mutex levelMutex;
    std::mutex hourMutex;
	std::mutex coutMutex;
	std::mutex errorMutex;
};

struct FileWriter
{
	bool firstJsonEntry = false;
	bool jsonStarted = false;
    bool csvHeaderWritten = false;
	std::mutex csvMutex;
	std::mutex jsonMutex;
	std::mutex logMutex;
	std::mutex htmlMutex;
	std::mutex iniMutex;

	std::ofstream jsonFile;
	std::ofstream csVFile;
	std::ofstream txtFile;
	std::ofstream htmlFile;
	std::ofstream iniFile;
	std::ofstream errorLog;
};


struct FileProgres
{
	std::mutex coutMutex;
	std::mutex errorMutex;
	std::atomic<int> processed{ 0 };
	int totalFiles = 0;
};






