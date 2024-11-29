#include <migration.h>
#include <common.h>

MIGRATION_UP(init, 1) {
  db->exec(R"(
    CREATE TABLE note(
      uri VARCHAR PRIMARY KEY,
      id TEXT,

      content TEXT NOT NULL,
      owner VARCHAR NOT NULL,
      published INTEGER NOT NULL,
      local BOOLEAN NOT NULL,
      sensitive BOOLEAN NOT NULL
    )
  )");

  setVersion(db, 1);
}

MIGRATION_DOWN(init, 1) {
  db->exec(R"(
    DROP TABLE note
  )");

  setVersion(db, 0);
}
