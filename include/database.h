#pragma once

#include <common.h>
#include "SQLiteCpp/Database.h"
#include <SQLiteCpp/Statement.h>
#include <migration.h>

namespace Database {
  extern SQLite::Database *conn;
  void Init();
}

#define STATEMENT(string) SQLite::Statement(*Database::conn, string);

#define LOOKUPKEY(entity, table, key)\
  static optional<entity> lookup##key(const string key) {\
    auto q = STATEMENT("SELECT * FROM "#table" WHERE "#key" = ?");\
    q.bind(1, key);\
    if (!q.executeStep()) return nullopt;\
    entity e;\
    e.load(q);\
    return e;\
  }

#define ORM(table, key, fields)\
    void load(SQLite::Statement &statement) {\
      auto _statement = &statement;\
      SQLite::Statement *_query;\
      int __flag = 1;\
      int _counter = 0;\
      std::vector<string> names;\
      fields\
    }\
    int insert() {\
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
    }\
    int update() {\
      int __flag = 0;\
      std::vector<string> names;\
      SQLite::Statement *_statement;\
      SQLite::Statement *_query;\
      int _counter = 0;\
      fields\
      string set;\
      for (auto name : names) {\
        set += name + " = ?,";\
      }\
      set.pop_back();\
      auto __q = STATEMENT(FMT("UPDATE {} SET {} WHERE {} = ?", #table, set, #key));\
      _query = &__q;\
      _counter = 1;\
      __flag = 2;\
      fields\
      __q.bind(_counter, key);\
      return _query->exec();\
    }\
    string __table = #table;\


#define INSERT_OR_UPDATE(object, pkey, key, value)\
  auto _query = STATEMENT(FMT("SELECT * FROM {} where {} = ? LIMIT 1", object.__table, #pkey));\
  bool isNew = false;\
  _query.bind(1, object.pkey);\
  int _r;\
  if (_query.executeStep()) {\
    object.key = (string)_query.getColumn(#key);\
    _r = object.update();\
    isNew = false;\
  } else {\
    object.key = value;\
    _r = object.insert();\
    isNew = true;\
  }\

#define F(name)\
  names.push_back(#name);\
  if (__flag == 1)\
    name = (typeof(name))_statement->getColumn(#name);\
  if (__flag == 2)\
      _query->bind(_counter, name);\
  \
  _counter++;

#define BOOL(name)\
  names.push_back(#name);\
  if (__flag == 1)\
    name = (bool)(int)_statement->getColumn(#name);\
  if (__flag == 2)\
      _query->bind((int)_counter, name);\
  \
  _counter++;

#define OPT(name)\
  names.push_back(#name);\
  if (__flag == 1) {\
    if (_statement->getColumn(#name).isNull()) {\
      name = nullopt;\
    } else {\
      name = (typeof(name))_statement->getColumn(#name);\
    }\
  }\
  if (__flag == 2) {\
      if (name.has_value()){\
        _query->bind(_counter, name.value());\
      } else {\
        _query->bind(_counter);\
      }\
  }\
  _counter++;

#define JSON(name)\
  names.push_back(#name);\
  if (__flag == 1) {\
    json j = json::parse((string)_statement->getColumn(#name));\
    name = j.template get<decltype(name)>();\
  }\
  if (__flag == 2) {\
      _query->bind(_counter, json(name).dump());\
  }\
  _counter++;


