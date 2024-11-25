#include <migration.h>
#include <common.h>

MIGRATION_UP(init, 1) {
  db->exec(R"(
    CREATE TABLE note(
      id VARCHAR PRIMARY KEY,
      content TEXT NOT NULL,
      owner VARCHAR NOT NULL,
      published INTEGER
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
