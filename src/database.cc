#include "database.h"
#include "SQLiteCpp/Database.h"

std::map<int, MigrationHandler> migrations_up;
std::map<int, MigrationHandler> migrations_down;

void *register_migration_up(int ver, MigrationHandler h) {
  migrations_up[ver] = h;
  return nullptr;
}

void *register_migration_down(int ver, MigrationHandler h) {
  migrations_down[ver] = h;
  return nullptr;
}


namespace Database {
  SQLite::Database *conn;
  void Init() {
    conn = new SQLite::Database(SOFTWARE".db3", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE | SQLite::OPEN_FULLMUTEX);

    conn->exec(R"(
      CREATE TABLE IF NOT EXISTS schema_version (
          version INTEGER PRIMARY KEY
      );
    )");

    auto query = STATEMENT("SELECT version FROM schema_version ORDER BY version DESC LIMIT 1");
    int version;
    if (query.executeStep()) {
      version = query.getColumn("version").getInt();
    } else {
      version = 0;
    }
    info("DB version {}", version);

    auto migration = migrations_up.rbegin();
    if (migration->first > version) {
      for (int i = version+1; i <= migration->first; i++ ) {
        info("Attempting migration to schema {}", i);
        migrations_up[i](conn);
      }
    }
  }
}
