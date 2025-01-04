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
      quoteUri TEXT,
      conversation TEXT NOT NULL,
      
      content TEXT,
      cw TEXT,
      sensitive BOOLEAN NOT NULL,

      owner TEXT NOT NULL,

      mentions JSON NOT NULL,
      hashtags JSON NOT NULL,
      emojis JSON NOT NULL,
      mediaIds JSON NOT NULL,

      published INTEGER NOT NULL,
      publishedClamped INTEGER,
      recievedAt INTEGER,

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

      lastUpdatedAt INTEGER NOT NULL,
      remoteUsersCount INTEGER NOT NULL,
      remoteNotesCount INTEGER NOT NULL,

      themecolor TEXT NOT NULL,
      faviconurl TEXT NOT NULL,
      iconurl TEXT NOT NULL,

      description TEXT NOT NULL,
      name TEXT NOT NULL
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
      friendlyUrl TEXT NOT NULL,

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
    CREATE TABLE notification(
      id TEXT PRIMARY KEY,
      type INTEGER NOT NULL,

      createdAt INTEGER NOT NULL,

      notifierUri TEXT,
      notifierId TEXT,
      notifieeUri TEXT NOT NULL,
      notifieeId TEXT NOT NULL,

      statusId TEXT,
      statusUri TEXT,

      emojiText TEXT,
      emojiUrl TEXT,

      isread BOOLEAN NOT NULL
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
    CREATE TABLE bite(
      uri TEXT PRIMARY KEY,
      id TEXT NOT NULL UNIQUE,
      local INTEGER NOT NULL,
      host TEXT NOT NULL,

      owner TEXT NOT NULL,
      bitUser TEXT,
      bitNote TEXT
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

  db->exec(R"(
    CREATE TABLE emoji(
      address TEXT NOT NULL PRIMARY KEY,
      id TEXT NOT NULL,
      shortcode TEXT NOT NULL,
      local INTEGER NOT NULL,
      host TEXT NOT NULL,

      imageurl TEXT NOT NULL
    )
  )");

  db->exec(R"(
    CREATE TABLE emojireact(
      uri TEXT PRIMARY KEY,
      id TEXT NOT NULL UNIQUE,
      local INTEGER NOT NULL,
      host TEXT NOT NULL,

      owner TEXT NOT NULL,
      ownerId TEXT NOT NULL,
      object TEXT NOT NULL,

      emojiId TEXT,
      emojiText TEXT
    )
  )");

  db->exec(R"(
    CREATE TABLE media(
      id PRIMARY KEY NOT NULL,
      local INTEGER NOT NULL,

      owner TEXT,

      mimeType TEXT NOT NULL,
      url TEXT NOT NULL,
      description TEXT NOT NULL,
      blurhash JSON,
      sensitive BOOLEAN NOT NULL,

      createdAt INTEGER NOT NULL
    )
  )");

  db->exec(R"(
    CREATE TABLE usersettings(
      userId TEXT PRIMARY KEY NOT NULL,
      email TEXT NOT NULL,

      frontendSettings JSON NOT NULL,

      password TEXT NOT NULL
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
