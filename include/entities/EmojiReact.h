#pragma once
#include "database.h"

struct EmojiReact {
  string uri;
  string id;
  int local;
  string host;

  string owner;
  string ownerId;
  string object;

  optional<string> emojiId;
  optional<string> emojiText;



  ORM(emojireact, uri,
      F(uri)
      F(id)
      F(local)
      F(host)

      F(owner)
      F(ownerId)
      F(object)

      OPT(emojiId)
      OPT(emojiText)
  )

  LOOKUPKEY(EmojiReact, emojireact, uri);
  LOOKUPKEY(EmojiReact, emojireact, id);

  static EmojiReact ingest(const json data);
};
