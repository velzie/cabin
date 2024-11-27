#pragma once
#include "SQLiteCpp/Statement.h"
#include <SQLiteCpp/Database.h>
using MigrationHandler = void (*)(SQLite::Database*);
#include "../src/database.h"


#define MIGRATION_UP(name, ver)\
  static void migrate_up_##name(SQLite::Database *db);\
  static void *reg_migration_up_##name = register_migration_up(ver, &migrate_up_##name);\
  static void migrate_up_##name(SQLite::Database *db)


#define MIGRATION_DOWN(name, ver)\
  static void migrate_down_##name(SQLite::Database *db);\
  static void *reg_migration_down_##name = register_migration_down(ver, &migrate_down_##name);\
  static void migrate_down_##name(SQLite::Database *db)


void *register_migration_up(int ver, MigrationHandler);
void *register_migration_down(int ver, MigrationHandler);


inline void setVersion(SQLite::Database *db, int ver) {
  db->exec("DELETE FROM schema_version");
  SQLite::Statement q(*db, R"(INSERT INTO schema_version (version) VALUES (?))");
  q.bind(1, ver);
  q.exec();
}
