#include "entities/Emoji.h"

Emoji Emoji::ingestAPTag(const json tag, const string host) {
  Emoji e;
  string shortcode = tag["name"];
  ASSERT(shortcode.length() > 2);

  // :name: -> name
  if (shortcode[0] == ':') {
    shortcode.erase(0, 1);
    shortcode.pop_back();
  }

  e.shortcode = shortcode;
  string sanitizedhost;
  for (auto c : host) {
    if (isalnum(c) || c == '_') {
      sanitizedhost += c;
    } else {
      sanitizedhost += '_';
    }
  }

  
  e.address = shortcode + "_" + sanitizedhost;
  e.host = host;
  e.local = false;
  e.imageurl = tag.at("icon").at("url");
  

  INSERT_OR_UPDATE(e, address, id, utils::genid());
  return e;
}

json Emoji::renderAPTag() {
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

