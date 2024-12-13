#pragma once
#include "entities/Emoji.h"

namespace EmojiService {
  Emoji parse(const json tag, const string host);
}
