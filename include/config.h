#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

struct Config
{
    std::string host = "localhost";
    std::string port = "3306";
    std::string user = "root";
    std::string password = "";
    std::string database = "";
    std::string migrations_dir = "./migrations";
};

class ConfigManager
{
public:
    static Config load(const std::string &filepath = "cpp-migrate.conf")
    {
        Config cfg;
        std::ifstream file(filepath);

        if (!file.is_open())
        {
            std::cerr << "Error: Could not open config file: " << filepath << std::endl;
            std::cerr << "Run './cpp-migrate init' to create a config file.\n";
            return cfg;
        }

        std::unordered_map<std::string, std::string> values;
        std::string line;

        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#')
                continue; // Skip empty lines and comments
            auto eq = line.find('=');
            if (eq == std::string::npos)
                continue; // Skip malformed lines

            std::string key = trim(line.substr(0, eq));
            std::string value = trim(line.substr(eq + 1));
            values[key] = value;
        }

        if (values.count("DB_HOST"))
            cfg.host = values["DB_HOST"];
        if (values.count("DB_PORT"))
            cfg.port = values["DB_PORT"];
        if (values.count("DB_USER"))
            cfg.user = values["DB_USER"];
        if (values.count("DB_PASSWORD"))
            cfg.password = values["DB_PASSWORD"];
        if (values.count("DB_NAME"))
            cfg.database = values["DB_NAME"];
        if (values.count("MIGRATIONS_DIR"))
            cfg.migrations_dir = values["MIGRATIONS_DIR"];
        return cfg;
    }

    static void createTemplate(const std::string &filepath = "cpp-migrate.conf")
    {
        std::ofstream file(filepath);

        file << "# cpp-migrate configuration\n"
                "# Edit these values to match your MySQL setup\n\n"
                "DB_HOST=127.0.0.1\n"
                "DB_PORT=3306\n"
                "DB_USER=root\n"
                "DB_PASSWORD=your_password_here\n"
                "DB_NAME=your_database_here\n"
                "MIGRATIONS_DIR=./migrations\n";
        std::cout << "[OK] Created config file: " << filepath << "\n";
    }

private:
    static std::string trim(const std::string& s){
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end   = s.find_last_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        return s.substr(start, end - start + 1);
    }
};
