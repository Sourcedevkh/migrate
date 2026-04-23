#pragma once
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include "database.h"

namespace fs = std::filesystem;

struct MigrationFile
{
    std::string filename;
    std::string filepath;
    int version;
};

class MigrationRunner
{
public:
    explicit MigrationRunner(Database &db) : db_(db) {}

    // Create schema_history table
    bool ensureHistoryTable()
    {
        const std::string sql = R"(
            CREATE TABLE IF NOT EXISTS schema_history (
                id           INT AUTO_INCREMENT PRIMARY KEY,
                version      INT NOT NULL UNIQUE,
                filename     VARCHAR(255) NOT NULL,
                applied_at   DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
                success      TINYINT(1) NOT NULL DEFAULT 1
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
        )";

        if(!db_.execute(sql)){
            std::cerr << "[ERROR] could not create schema_history table\n";
            return false;
        }
        return true;
    }

    // Get a sorted list of migration
    std::vector<MigrationFile> discoverFile(const std::string& dir){
        std::vector<MigrationFile> files;

        if(!fs::exists(dir)){
            std::cerr << "[ERROR] migration directory not found: "<< dir<<"\n";
            return files;
        }

        // Regex matches files 
        std::regex pattern(R"(^(\d+)_.*\.sql$)");

        for(const auto& entry : fs::directory_iterator(dir)){
            if(!entry.is_regular_file()) continue;

            std::string fname = entry.path().filename().string();
            std::smatch match;

            if(std::regex_match(fname, match, pattern)){
                MigrationFile mf;
                mf.filename = fname;
                mf.filepath = entry.path().string();
                mf.version = std::stoi(match[1].str());
                files.push_back(mf);

            }
        }

        // Sort by version number
        std::sort(files.begin(), files.end(), [](const MigrationFile& a, const MigrationFile& b) {
            return a.version < b.version;
        });
        return files;
    }

    // Get set of applied migration version
     std::set<int> getAppliedVersions() {
        std::set<int> applied;
        auto rows = db_.query("SELECT version FROM schema_history WHERE success = 1");
        for (auto& row : rows) {
            applied.insert(std::stoi(row[0]));
        }
        return applied;
    }

    // Rull all pending migration
    bool runUp(const std::string& migrationsDir){
        if(!ensureHistoryTable())
            return false;
        
        auto files = discoverFile(migrationsDir);
        auto applied = getAppliedVersions();

        if(files.empty()){
            std::cout<< "[INFO] No migration files found"<< migrationsDir<<"\n";
            return true;
        }

        std::vector<MigrationFile> pending;
        for(auto& f : files){
            if(applied.find(f.version) == applied.end()){
                pending.push_back(f);
            }
        }

        if(pending.empty()){
            std::cout<<"[OK] Nothing to migrate. Database is up to date!\n";
            return true;
        }
        std::cout<<"[INFO] Found"<<pending.size()<<"pending migration(s).\n\n";
        
        int success_count = 0;
        for(auto& mf : pending){
            std::cout << " ---> Applying: "<<mf.filename<<" ... ";
            std::string sql = readFile(mf.filepath);

            if(sql.empty()){
                std::cout << "SKIP (empty file)\n";
                continue;
            }

            // Split by semicolon(;) to handle multiple statements
            auto statements = splitStatements(sql);
            bool ok = true;

            for(auto& stmt : statements){
                if(!db_.execute(stmt)){
                    ok = false;
                    break;
                }
            }
            
            if(ok){
                // Record success in history
               std::string record =
                    "INSERT INTO schema_history (version, filename, success) VALUES ("
                    + std::to_string(mf.version) + ", '"
                    + escapeSql(mf.filename) + "', 1)";
                db_.execute(record);
                std::cout<<"Ok\n";
                success_count++;
            }else{
                // Record failed
                std::string record = "INSERT INTO schema_history (version, filename, success) VALUES ("
                    + std::to_string(mf.version) + ", '"
                    + escapeSql(mf.filename) + "', 0)"
                    " ON DUPLICATE KEY UPDATE success = 0";
                db_.execute(record);
                std::cout<<"Failed\n";
                std::cerr << "[ERROR] Migration failed. stopping.\n";
                return false;
            }
        }
        std::cout<<"\n[OK] Applied"<<success_count<<"migration(s) success.\n";
        return true;
    }

    // Print status of all migration
    bool printStatus(const std::string& migrationsDir){
        if(!ensureHistoryTable()) return false;

        auto files = discoverFile(migrationsDir);
        auto applied = getAppliedVersions();

        std::cout<<"\n";
        std::cout<<"Version | Status | Filename\n";
        std::cout<<"--------|--------|-------------------------\n";

        if(files.empty()){
            std::cout<<"(no migration files found in "<< migrationsDir<<")\n";
            return true;
        }
        
        for(auto & f : files){
            bool isApplied = applied.count(f.version) > 0;
            std::string status = isApplied ? "Applied" : "Pending";
            std::string marker = isApplied ? " " : ">";
            printf("  %s%03d     | %s | %s\n",
                   marker.c_str(), f.version, status.c_str(), f.filename.c_str());
        }

        std::cout<<"\n";
        std::cout << "Total: " << files.size() << " file(s), "
                  << applied.size() << " applied, "
                  << (files.size() - applied.size()) << " pending.\n\n";
        return true;
    }


private:
    Database& db_;

    std::string readFile(const std::string& path){
        std::ifstream file(path);
        if(!file.is_open()){
            std::cerr << "[ERROR] Cannot open file:"<<path<<"\n";
            return "";
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    // Split SQL content by semicolon, skip empty statements
     std::vector<std::string> splitStatements(const std::string& sql) {
        std::vector<std::string> stmts;
        std::string current;

        for(char c : sql){
            if(c == ';'){

                // Trim whitespace
                size_t start = current.find_first_not_of(" \t\r\n");
                if(start != std::string::npos){
                    stmts.push_back(current.substr(start));
                }
                current.clear();
            }else{
                current += c;
            }
        }

        // Catch last statement if no trailing semicolon
        size_t start = current.find_first_not_of(" \t\r\n");
        if(start != std::string::npos){
            stmts.push_back(current.substr(start));
        }
        return stmts;
    }

    std::string escapeSql(const std::string& s) {
        std::string out;
        for (char c : s) {
            if (c == '\'') out += "\\'";
            else out += c;
        }
        return out;
    }
};