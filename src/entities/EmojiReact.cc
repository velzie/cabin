#include "entities/EmojiReact.h"
#include "entities/Emoji.h"
#include "entities/Note.h"
#include "entities/User.h"
#include "services/FetchService.h"
#include "services/NotificationService.h"

EmojiReact EmojiReact::ingest(const json body) {
  User reacter = FetchService::fetch<User>(body["actor"]);
  Note note = FetchService::fetch<Note>(body["object"]);

  URL reacturi(body["id"]);

  EmojiReact e;
  e.uri = body["id"];
  e.id = utils::genid();
  e.local = 0;
  e.host = reacturi.host;

  e.owner = body["actor"];
  e.ownerId = reacter.id;
  e.object = body["object"];

  optional<Emoji> oem;
  if (body["tag"].is_array() && body["tag"][0].is_object()) {
    Emoji em = Emoji::ingestAPTag(body["tag"][0], reacturi.host);
    oem = em;
    e.emojiText = nullopt;
    e.emojiId = em.id;
  } else {
    e.emojiId = nullopt;
    e.emojiText = body["content"];
  }

  e.insert();

  if (note.local == true) {
    User reactee = User::lookupuri(note.owner).value();
    NotificationService::createReact(note, reactee, reacter, oem, e.emojiText);
  }

  return e;
}
