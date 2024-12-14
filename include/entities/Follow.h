#pragma once
#include "database.h"

struct Follow {
  string uri;
  string id;
  int local;
  string host;

  string follower;
  string followee;

  int pending;
  std::time_t createdAt;

  ORM(follow, uri,
      F(uri)
      F(id)
      F(local)
      F(host)

      F(follower)
      F(followee)

      F(pending)
      F(createdAt)
  )

  LOOKUPKEY(Follow, follow, uri);
  LOOKUPKEY(Follow, follow, id);

  static Follow ingest(const json data);

  json renderAP() {
    return {
      {"id", FOLLOW(id)},
      {"type", "Follow"},
      {"actor", follower},
      {"object", followee}
    };
  }
};
