#pragma once
#include "database.h"
#include "entities/User.h"

struct Media {
  string id;
  int local;

  optional<string> owner;

  string mimeType;
  string url;
  string description;
  string blurhash;
  bool sensitive;

  std::time_t createdAt;

  ORM(media, id, 
    F(id)
    F(local)

    OPT(owner)

    F(mimeType)
    F(url)
    F(description)
    F(blurhash)
    BOOL(sensitive)

    F(createdAt)
  )

  LOOKUPKEY(Media, media, id);

  json renderAP();
  json renderMS(User &requester);
  static Media ingest(const json attachment);
  static Media create(string mimeType);
};
