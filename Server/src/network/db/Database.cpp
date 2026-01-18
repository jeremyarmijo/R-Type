#include "Database.hpp"

Database::Database() {
    db_ = nullptr;
}

Database::~Database() {
  if (db_ != nullptr) {
    Close();
  }
}

bool Database::Open(const std::string &db_name) {
  return sqlite3_open(db_name.c_str(), &db_) == SQLITE_OK;
}

void Database::Close() {
  if (db_ != nullptr) {
    sqlite3_close(db_);
    db_ = nullptr;
  }
}

bool Database::ExecuteQuery(const std::string &query) {
  char *errMsg = nullptr;
  int response = sqlite3_exec(db_, query.c_str(), nullptr, nullptr, &errMsg);
  if (response != SQLITE_OK) {
    sqlite3_free(errMsg);
    return false;
  }
  return true;
}

bool Database::PrepareStatement(const std::string &query,
                               void **stmt) {
  return sqlite3_prepare_v2(db_, query.c_str(), -1, reinterpret_cast<sqlite3_stmt**>(stmt), nullptr) == SQLITE_OK;
}

void Database::FinalizeStatement(void *stmt) {
  sqlite3_finalize(reinterpret_cast<sqlite3_stmt*>(stmt));
}

bool Database::GetUser(const std::string &username, std::string &password, int &score) {
    sqlite3_stmt *stmt;
    std::string query = "SELECT password, score FROM users WHERE username = ?";
    if (!PrepareStatement(query, reinterpret_cast<void**>(&stmt)))
        return false;
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        score = sqlite3_column_int(stmt, 1);
        found = true;
    }
    FinalizeStatement(stmt);
    return found;
}

bool Database::AddUser(const std::string &username, const std::string &password, int score) {
    sqlite3_stmt *stmt;
    std::string query = "INSERT INTO users (username, password, score) VALUES (?, ?, ?) "
              "ON CONFLICT(username) DO UPDATE SET password=excluded.password, score=excluded.score;";
    if (!PrepareStatement(query, reinterpret_cast<void**>(&stmt))) {
        std::cerr << "Failed to prepare statement for adding user: " << username << std::endl;
        return false;
    }
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, score);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success) {
        std::cerr << "Failed to add user: " << username << std::endl;
    }
    FinalizeStatement(stmt);
    return success;
}
