#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <mysql.h>
#include <config.h>

class Database{
    public:
        Database(): conn_(nullptr){}

        ~Database(){
            disconnect();
        }

    // Connect to Mysql
    bool connect(const Config& cfg){
        conn_ = mysql_init(nullptr);
        if(!conn_){
            std::cerr << "Error: mysql_init failed\n";
            return false;
        }

        unsigned int port = std::stoi(cfg.port);
        if(!mysql_real_connect(
            conn_,
            cfg.host.c_str(),
            cfg.user.c_str(),
            cfg.password.c_str(),
            cfg.database.c_str(),
            port,
            nullptr, 0)){
                std::cerr << "Error: Mysql connection failed: "<< mysql_error(conn_) << std::endl;
                return false;
            }
    
            // Use UTF-8 encoding
            mysql_set_character_set(conn_, "utf8mb4");
            std::cout <<"[OK] connected to database:"<<cfg.database<<"@"<<cfg.host<<":"<<cfg.port<<"\n";
            return true;
    }

    void disconnect(){
        if(conn_){
            mysql_close(conn_);
            conn_ = nullptr;
        }
    }

    // Execute a SQL statement
    bool execute(const std::string& sql){

        // failed to connect
        if(!conn_) return false;
        
        if(mysql_query(conn_, sql.c_str()) != 0){
            std::cerr << "[ERORR] Sql fialed: "<<mysql_error(conn_)<<"\n";
            std::cerr<<"Query: "<<sql.substr(0, 120)<<"\n";
            return false;
        }
        return true;
    }

    // Execute and return rows as vector of vector<string>
    std::vector<std::vector<std::string>> query(const std::string& sql){
        std::vector<std::vector<std::string>> result;

        if(!conn_) return result;
        if(mysql_query(conn_, sql.c_str()) != 0){
            std::cerr <<"[ERORR] Query failed: "<<mysql_error(conn_)<<"\n";
            return result;
        }

        MYSQL_RES* res = mysql_store_result(conn_);
        if(!res) return result;

        int num_fields = mysql_num_fields(res);
        MYSQL_ROW row;

        while ((row = mysql_fetch_row(res)))
        {
            std::vector<std::string> rowData;
            for(int i=0; i<num_fields; i++){
                rowData.push_back(row[i] ? std::string(row[i]): "NULL");
            }
            result.push_back(rowData);
        }
        mysql_free_result(res);
        return result;
    }

    // Check if a table exists in the current database
    bool tableExists(const std::string& tableName){
        std::string sql =
            "SELECT COUNT(*) FROM information_schema.tables "
            "WHERE table_schema = DATABASE() AND table_name = '" + tableName + "'";
        auto rows = query(sql);
        return !rows.empty() && rows[0][0] == "1";
    }

    bool isConnected() const{return conn_ != nullptr;}

    private:
        MYSQL* conn_;
};