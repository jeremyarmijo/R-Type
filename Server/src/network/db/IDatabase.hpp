#pragma once

#include <string>

class IDatabase {
 public:
  virtual ~IDatabase() = default;

  virtual bool Open(const std::string &db_name) = 0;
  virtual void Close() = 0;
  virtual bool ExecuteQuery(const std::string &query) = 0;
  virtual bool PrepareStatement(const std::string &query, void **stmt) = 0;
  virtual void FinalizeStatement(void *stmt) = 0;
  virtual bool GetUser(const std::string &username, std::string &password,
                       int &score) = 0;
  virtual bool AddUser(const std::string &username, const std::string &password,
                       int score) = 0;
};
