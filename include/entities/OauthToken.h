#pragma once
#include "database.h"

struct OauthToken {
  string id;
  string userId;

  bool isPleroma;

  ORM(oauthtoken, id,
    F(id)
    F(userId)

    BOOL(isPleroma)
  )

  LOOKUPKEY(OauthToken, oauthtoken, id)
};
