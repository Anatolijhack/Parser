#include "args.h"
#include <iostream>
ProgramOptions parseArguments(int argc, char* argv[])
{
    ProgramOptions options;

    if (argc < 2)
    {
        printHelp();
        exit(0);
    }
    std::string  firstArg = argv[1];
    if (firstArg == "--help" || firstArg == "-h")
    {
        printHelp();
        exit(0);
    }
    options.path = argv[1];
    for (int i = 2; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "--keyword" && i + 1 < argc)
            options.keyword = argv[++i];
        else if (arg == "--from" && i + 1 < argc)
            options.startTime = argv[++i];
        else if (arg == "--to" && i + 1 < argc)
            options.endTime = argv[++i];
        else if (arg == "--output" && i + 1 < argc)
            options.outputFile = argv[++i];
        else if (arg == "--format" && i + 1 < argc)
        {
            options.format = argv[++i];

            if (options.format != "txt" && options.format != "csv" && options.format != "json")
            {
                std::cout << "Invalid format: " << options.format << "\n";
                exit(1);
            }
        }
        else if (arg == "--to-json")
        {
            options.toJson = true;
        }
        else
        {
            std::cout << "Unknown option: " << arg << "\n";
            exit(1);
        }
    }

    return options;
}
void  printHelp()
{
    std::cout << "===== Log Parser CLI === \n\n";
    std::cout << "Usage:\n";
    std::cout << "parser.exe < path> [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "--keyword <word> Fillter by keyword (default: ERROR)\n";
    std::cout << "--from <HH:MM:SS> End time filter\n";
    std::cout << "--output <name> Output file name (default: report)\n";
    std::cout << "--format<type> txt/csv/json (default : txt)\n";
    std::cout << "--help Show this help mesage\n\n";
    std::cout << "Examples:\n";
    std::cout << "parser exe logs/\n";
    std::cout << "parser.exe logs/ --keyword FAIL\n";
    std::cout << "parser.exe logs/ --from 10:00:00 -- to 18:00:00\n";
}