#include "progress.h"
#include <iostream>
void showProgress(FileProgres& proces) {
    const int barWidth = 30;
    float progress = 0.0;

    int done = proces.processed.load();
    int total = proces.totalFiles > 0 ? proces.totalFiles : 1; // избегаем деления на 0
    progress = static_cast<float>(done) / total;

    int pos = static_cast<int>(barWidth * progress);

    std::lock_guard<std::mutex> lock(proces.coutMutex);
    std::cout << "\r[";
    for (int i = 0; i < barWidth; ++i) std::cout << (i < pos ? '#' : '-');
    std::cout << "] " << int(progress * 100) << "% (" << done << "/" << total << " files)";
    std::cout.flush();
}