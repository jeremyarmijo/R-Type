#pragma once

#include "db/IDatabase.hpp"
#include <string>
#include <iostream>
#include <sqlite3.h>

class Database : public IDatabase
{
public:
  Database();
  ~Database() override;

  bool Open(const std::string &db_name) override;
  void Close() override;
  bool ExecuteQuery(const std::string &query) override;
  bool PrepareStatement(const std::string &query,
                        void **stmt) override;
  void FinalizeStatement(void *stmt) override;

  bool GetUser(const std::string &username, std::string &password, int &score);
  bool AddUser(const std::string &username, const std::string &password, int score);
private:
  sqlite3 *db_;
};

// extern "C" {
//   Database* CreateDatabase() {
//     return new Database();
//   }
// }