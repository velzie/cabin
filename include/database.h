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


#define ORM(table, key, fields)\
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
    }\
    inline int update() {\
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
    inline int insertOrUpdate(string keyv) {\
      auto query = STATEMENT(FMT("SELECT 1 FROM {} WHERE {} = ? LIMIT 1", #table, #key));\
      query.bind(1, key);\
      if (query.executeStep()) {\
        return update();\
      }else {\
        key = keyv;\
        return insert();\
      }\
    }\


#define F(name)\
  names.push_back(#name);\
  if (__flag == 1)\
    name = (typeof(name))_statement->getColumn(#name);\
  if (__flag == 2)\
      _query->bind(_counter, name);\
  \
  _counter++;

#define OPT(name)\
  names.push_back(#name);\
  if (__flag == 1)\
    if (_statement->getColumn(#name).isNull())\
      name = nullopt;\
    else\
      name = (typeof(name))_statement->getColumn(#name);\
  if (__flag == 2)\
      if (name.has_value())\
        _query->bind(_counter, name.value());\
      else\
        _query->bind(_counter);\
  \
  _counter++;
