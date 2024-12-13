#pragma once
#include "entities/Emoji.h"

namespace EmojiService {
  Emoji parse(const json tag, const string host);
  optional<Emoji> lookup(const string name);
  optional<Emoji> lookupAddress(const string address);
}
