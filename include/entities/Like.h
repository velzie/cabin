#pragma once
#include "database.h"

struct Like {
  string uri;
  string id;
  int local;
  string host;

  string owner;
  string object;

  ORM(like, uri,
      F(uri)
      F(id)
      F(local)
      F(host)

      F(owner)
      F(object)
  )

  LOOKUPKEY(Like, like, id);
  LOOKUPKEY(Like, like, uri);

  static Like ingest(const json object);
};
