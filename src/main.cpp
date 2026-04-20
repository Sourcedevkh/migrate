#include <iostream>
#include <string>
#include "cli.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        CLI::printUsage();
        return 1;
    }

    std::string command = argv[1];

    CLI cli;

    if (command == "up") {
        return cli.runUp() ? 0 : 1;
    } else if (command == "status") {
        return cli.runStatus() ? 0 : 1;
    } else if (command == "init") {
        return cli.runInit() ? 0 : 1;
    } else if (command == "help" || command == "--help" || command == "-h") {
        CLI::printUsage();
        return 0;
    } else {
        std::cerr << "[ERROR] Unknown command: " << command << "\n";
        CLI::printUsage();
        return 1;
    }
}