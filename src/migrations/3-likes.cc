#include <migration.h>
#include <common.h>

MIGRATION_UP(likes, 3) {
  db->exec(R"(
    CREATE TABLE like(
      uri TEXT PRIMARY KEY,
      id TEXT NOT NULL,
      local INTEGER NOT NULL,

      owner TEXT NOT NULL,
      object TEXT NOT NULL
    )
  )");

  setVersion(db, 3);
}

MIGRATION_DOWN(likes, 3) {
  db->exec(R"(
    DROP TABLE like
  )");

  setVersion(db, 2);
}
