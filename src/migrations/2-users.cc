#include <migration.h>
#include <common.h>

MIGRATION_UP(init, 2) {
  db->exec(R"(
    CREATE TABLE user(
      uri TEXT PRIMARY KEY,
      id TEXT,

      local INTEGER NOT NULL,
      publicKey TEXT,
      privateKey TEXT,

      username TEXT,
      displayname TEXT,
      summary TEXT
    )
  )");

  setVersion(db, 2);
}

MIGRATION_DOWN(init, 2) {
  db->exec(R"(
    DROP TABLE user
  )");

  setVersion(db, 1);
}
