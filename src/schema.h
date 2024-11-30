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
  _counter++;\

#include "entities/Note.h"
#include "entities/Instance.h"
#include "entities/User.h"
#include "entities/Like.h"
