#pragma once
#define USE_DB
#include <common.h>
#include <SQLiteCpp/Statement.h>

#define ORM(table, fields)\
    inline void load(SQLite::Statement &statement) {\
      auto _statement = &statement;\
      SQLite::Statement *_query;\
      int __flag = 1;\
      int _counter = 0;\
      std::vector<string> names;\
      fields\
    }\
    inline int insert() {\
      int __flag = 0;\
      std::vector<string> names;\
      SQLite::Statement *_statement;\
      SQLite::Statement *_query;\
      int _counter = 0;\
      fields\
      string keys;\
      string values;\
      for (auto name : names) {\
        keys += name+",";\
        values +="?,";\
      }\
      keys.pop_back();\
      values.pop_back();\
      auto __q = STATEMENT(FMT("INSERT INTO {} ({}) VALUES ({})", #table, keys, values))\
      _query = &__q;\
      _counter = 1;\
      __flag = 2;\
      fields\
      return _query->exec();\
    }

#define F(name)\
  names.push_back(#name);\
  if (__flag == 1)\
    name = (typeof(name))_statement->getColumn(#name);\
  if (__flag == 2)\
      _query->bind(_counter, name);\
  _counter++;\
    
struct User {
  string uri;
  string id;
  int local;
  string host;

  string publicKey;
  string privateKey;

  string username;
  string displayname;
  string summary;

  std::time_t lastUpdatedAt;
  int remoteFollowersCount;
  int remoteFollowingCount;
  int remoteNotesCount;

  string avatar;
  string banner;

  int isBot;
  int isCat;
  int speakAsCat;

  string inbox;
  string sharedInbox;
  string featured;

  ORM(user,
    F(uri)
    F(id)
    F(local)
    F(host)

    F(publicKey)
    F(privateKey)

    F(username)
    F(displayname)
    F(summary)

    F(lastUpdatedAt)
    F(remoteFollowersCount);
    F(remoteFollowingCount);
    F(remoteNotesCount);

    F(avatar)
    F(banner)

    F(isBot)
    F(isCat)
    F(speakAsCat)

    F(inbox)
    F(sharedInbox)
    F(featured)
  )
};

struct Instance {
  string host;

  std::time_t lastUpdatedAt;
  int remoteUsersCount;
  int remoteNotesCount;

  string description;
  string name;

  ORM(instance,
    F(host)

    F(lastUpdatedAt)
    F(remoteUsersCount)
    F(remoteNotesCount)

    F(description)
    F(name)
  )
};

#define NOTEVISIBILITY_Public 0
#define NOTEVISIBILITY_Home 1
#define NOTEVISIBILITY_Followers 2
#define NOTEVISIBILITY_Direct 3
struct Note {
  string uri;
  string id;
  int local;
  string host;

  int visibility;
  string replyToUri;
  string renoteUri;

  string content;
  string cw;
  int sensitive;

  string owner;
  std::time_t published;

  std::time_t lastUpdatedAt;
  int remoteRenoteCount;
  int remoteReplyCount;
  int remoteLikeCount;

  ORM(note,
    F(uri)
    F(id)
    F(local)
    F(host)

    F(visibility)
    F(replyToUri)
    F(renoteUri)

    F(content)
    F(cw)
    F(sensitive)

    F(owner)
    F(published)

    F(lastUpdatedAt)
    F(remoteRenoteCount)
    F(remoteReplyCount)
    F(remoteLikeCount)
  )
};


struct Like {
  string uri;
  string id;
  int local;
  string host;

  string owner;
  string object;

  ORM(like,
      F(uri)
      F(id)
      F(local)
      F(host)

      F(owner)
      F(object)
  )
};
