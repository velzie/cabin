#pragma once
#include "database.h"
#include "utils.h"

struct Bite {
  string uri;
  string id;
  int local;
  string host;

  string owner;

  optional<string> bitUser;
  optional<string> bitNote;

  ORM(bite, uri,
      F(uri)
      F(id)
      F(local)
      F(host)

      F(owner)

      OPT(bitUser)
      OPT(bitNote)
  )

  LOOKUPKEY(Bite, bite, id);
  LOOKUPKEY(Bite, bite, uri);

  json renderAP() {
    return {
      {"id", uri},
      {"type", "Bite"},
      {"actor", owner},
      {"target", bitUser.value()},
      {"to", bitUser.value()},
      {"published", utils::dateISO()},
    };
  }
};
