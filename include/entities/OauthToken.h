#pragma once
#include "database.h"

struct OauthToken {
  string id;
  string userId;

  vector<string> scopes;
  string clientId;
  bool isPleroma;

  string token;


  ORM(oauthtoken, id,
    F(id)
    F(userId)

    JSON(scopes)
    F(clientId)
    BOOL(isPleroma)

    F(token)
  )

  LOOKUPKEY(OauthToken, oauthtoken, id)
  LOOKUPKEY(OauthToken, oauthtoken, token)
};
