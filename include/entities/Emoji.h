#pragma once
#include "database.h"

struct Emoji {
  string id;
  string shortcode;
  int local;
  string host;

  string imageurl;

  ORM(emoji, id,
      F(id)
      F(shortcode)
      F(local)
      F(host)

      F(imageurl)
  )

  string canonUri() {
    return API("emojis/" + shortcode);
  }

  json renderTag() {
    return {
      {"icon", {
        {"type", "Image"},
        {"url", imageurl},
      }},
      {"id", canonUri()},
      {"name", shortcode},
      {"type", "Emoji"},
    };
  }
};
