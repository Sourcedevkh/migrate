#pragma once
#include <iostream>
#include <filesystem>
#include "config.h"
#include "database.h"
#include "migration_runner.h"

namespace fs = std::filesystem;

class CLI
{
public:
    CLI()
    {
        cfg_ = ConfigLoader::load("cpp-migrate.conf");
    }

    // ./cpp-migrate up
    bool runUp()
    {
        printBanner();
        std::cout << "[CMD] Running: up\n\n";

        if (!db_.connect(cfg_))
            return false;

        MigrationRunner runner(db_);
        bool ok = runner.runUp(cfg_.migrations_dir);

        db_.disconnect();
        return ok;
    }

    // ./cpp-migrate status
    bool runStatus()
    {
        printBanner();
        std::cout << "[CMD] Running: status\n";

        if (!db_.connect(cfg_))
            return false;

        MigrationRunner runner(db_);
        bool ok = runner.printStatus(cfg_.migrations_dir);

        db_.disconnect();
        return ok;
    }

    // ./cpp-migrate init
    bool runInit()
    {
        printBanner();
        std::cout << "[CMD] Running: init\n\n";

        // Create config
        if (!fs::exists("cpp-migrate.conf"))
        {
            ConfigLoader::createTemplate("cpp-migrate.conf");
        }
        else
        {
            std::cout << "[SKIP] cpp-migrate.conf already exists\n";
        }

        // Create migrations directory
        if (!fs::exists("migrations"))
        {
            fs::create_directory("migrations");
            std::cout << "[OK] Created directory: ./migrations\n";

            // Create example migration
            std::ofstream example("migrations/001_create_users.sql");
            example << "-- Migration 001: Create users table\n"
                       "-- Created by cpp-migrate\n\n"
                       "CREATE TABLE IF NOT EXISTS users (\n"
                       "    id         INT AUTO_INCREMENT PRIMARY KEY,\n"
                       "    username   VARCHAR(100) NOT NULL UNIQUE,\n"
                       "    email      VARCHAR(255) NOT NULL,\n"
                       "    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP\n"
                       ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n";

            std::cout << "[OK] Created example: migrations/001_create_users.sql\n";
        }
        else
        {
            std::cout << "[SKIP] ./migrations directory already exists\n";
        }

        std::cout << "\n[DONE] Project initialized!\n";
        std::cout << "  1. Edit cpp-migrate.conf with your database credentials\n";
        std::cout << "  2. Add .sql files to ./migrations/\n";
        std::cout << "  3. Run: ./cpp-migrate up\n\n";
        return true;
    }

    static void printUsage()
    {
        std::cout << "\n"
                     "  cpp-migrate - MySQL Schema Migration Tool\n"
                     "  ------------------------------------------\n"
                     "  Usage: ./cpp-migrate <command>\n\n"
                     "  Commands:\n"
                     "    init     Create config file and migrations/ directory\n"
                     "    up       Run all pending migrations\n"
                     "    status   Show migration status (applied vs pending)\n"
                     "    help     Show this help message\n\n"
                     "  Config file: cpp-migrate.conf\n"
                     "  Migrations:  ./migrations/001_name.sql, 002_name.sql, ...\n\n";
    }

private:
    Config cfg_;
    Database db_;

    static void printBanner()
    {
        std::cout << "\n"
                     "  ╔═══════════════════════════════╗\n"
                     "  ║   cpp-migrate v1.0.0          ║\n"
                     "  ║   MySQL Migration Tool        ║\n"
                     "  ╚═══════════════════════════════╝\n\n";
    }
};