#include <migration.h>
#include <common.h>

MIGRATION_UP(init, 1) {
  db->exec(R"(
    CREATE TABLE note(
      uri VARCHAR PRIMARY KEY,
      id TEXT NOT NULL UNIQUE,
      local BOOLEAN NOT NULL,
      host TEXT NOT NULL,

      visibility INTEGER,
      replyToUri TEXT,
      renoteUri TEXT,
      
      content TEXT,
      cw TEXT,
      sensitive BOOLEAN NOT NULL,

      owner TEXT NOT NULL,
      published INTEGER NOT NULL,


      lastUpdatedAt INTEGER,
      remoteRenoteCount INTEGER,
      remoteReplyCount INTEGER,
      remoteLikeCount INTEGER
    )
  )");

  db->exec(R"(
    CREATE TABLE instance(
      host TEXT PRIMARY KEY,
      id TEXT NOT NULL UNIQUE,

      lastUpdatedAt INTEGER,
      remoteUsersCount INTEGER,
      remoteNotesCount INTEGER,

      description TEXT,
      name TEXT
    )
  )");

  db->exec(R"(
    CREATE TABLE user(
      uri TEXT PRIMARY KEY,
      id TEXT NOT NULL UNIQUE,
      local INTEGER NOT NULL,
      host TEXT NOT NULL,

      publicKey TEXT,
      privateKey TEXT,

      username TEXT NOT NULL,
      displayname TEXT,
      summary TEXT,

      lastUpdatedAt INTEGER,
      remoteFollowersCount INTEGER,
      remoteFollowingCount INTEGER,
      remoteNotesCount INTEGER,

      avatar TEXT,
      banner TEXT,

      isBot BOOLEAN NOT NULL,
      isCat BOOLEAN,
      speakAsCat BOOLEAN,

      inbox TEXT,
      sharedInbox TEXT,
      featured TEXT
    )
  )");

  db->exec(R"(
    CREATE TABLE like(
      uri TEXT PRIMARY KEY,
      id TEXT NOT NULL UNIQUE,
      local INTEGER NOT NULL,
      host TEXT NOT NULL,

      owner TEXT NOT NULL,
      object TEXT NOT NULL
    )
  )");

  db->exec(R"(
    CREATE TABLE follow(
      uri TEXT PRIMARY KEY,
      id TEXT NOT NULL UNIQUE,
      local INTEGER NOT NULL,
      host TEXT NOT NULL,

      follower TEXT NOT NULL,
      followee TEXT NOT NULL,

      pending BOOLEAN,
      createdAt INTEGER
    )
  )");

  setVersion(db, 1);
}

MIGRATION_DOWN(init, 1) {
  db->exec(R"(
    DROP TABLE note
    DROP TABLE instance
    DROP TABLE user
    DROP TABLE like
    DROP TABLE follow
  )");

  setVersion(db, 0);
}
