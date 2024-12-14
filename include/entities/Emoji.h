#pragma once
#include "database.h"
#include "entities/Note.h"

struct Emoji {
  string address;
  string id;
  string shortcode;
  int local;
  string host;

  string imageurl;

  ORM(emoji, address,
      F(address)
      F(id)
      F(shortcode)
      F(local)
      F(host)

      F(imageurl)
  )

  string canonUri() {
    return API("emojis/" + shortcode);
  }

  NoteEmoji renderNoteEmoji() {
    NoteEmoji nem;
    nem.id = id;
    nem.shortcode = shortcode;
    return nem;
  }

  json renderTag() {
    return {
      {"icon", {
        {"type", "Image"},
        {"url", imageurl},
      }},
      {"id", canonUri()},
      {"name", FMT(":{}:", address)},
      {"type", "Emoji"},
    };
  }
};
