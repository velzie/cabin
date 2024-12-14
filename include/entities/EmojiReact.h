#pragma once
#include "database.h"

struct EmojiReact {
  string uri;
  string id;
  int local;
  string host;

  string owner;
  string object;

  optional<string> emojiId;
  optional<string> emojiText;



  ORM(emojireact, uri,
      F(uri)
      F(id)
      F(local)
      F(host)

      F(owner)
      F(object)

      OPT(emojiId)
      OPT(emojiText)
  )
};
