#pragma once
#include "entities/Emoji.h"
#include "entities/EmojiReact.h"
#include "entities/Note.h"


namespace EmojiService {
  EmojiReact CreateReact(User &reacter, Note &note, optional<Emoji> &emoji, optional<string> emojiText);
}
