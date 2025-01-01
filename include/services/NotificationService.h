#pragma once
#include "entities/Emoji.h"
#include "entities/User.h"
#include "entities/Note.h"

namespace NotificationService {
  void createMention(Note &note, User &mentioner, User &mentionee);
  void createFollow(User &followee, User &follower);
  void createBite(optional<Note> note, User &biteee, User &biter);
  void createRenote(Note &note, Note &renote, User &renotee, User &renoter);
  void createFavorite(Note &note, User &favoritee, User &favoriter);
  void createReact(Note &note, User &reactee, User &reacter, optional<Emoji> &emoji, optional<string> emojiText);
}
