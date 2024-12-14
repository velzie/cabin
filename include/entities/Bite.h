#pragma once
#include "database.h"

struct Bite {
  string uri;
  string id;
  int local;
  string host;

  string owner;
  string object;

  ORM(bite, uri,
      F(uri)
      F(id)
      F(local)
      F(host)

      F(owner)
      F(object)
  )
};
