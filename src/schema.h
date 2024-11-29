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
  string publicKey;
  string privateKey;

  string username;
  string displayname;
  string summary;

  ORM(user,
    F(uri)
    F(id)

    F(local)
    F(publicKey)
    F(privateKey)

    F(username)
    F(displayname)
    F(summary)
  )
};

struct Note {
  string uri;
  string id;
  int local;

  string content;
  string owner;
  std::time_t published;

  int sensitive;

  ORM(note,
    F(uri)
    F(id)
    F(local)

    F(content)
    F(owner)
    F(published)

    F(sensitive)
  )
};


struct Like {
  string uri;
  string id;
  int local;

  string owner;
  string object;

  ORM(like,
      F(uri)
      F(id)
      F(local)

      F(owner)
      F(object)
  )
};
