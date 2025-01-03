#include "entities/EmojiReact.h"
#include "entities/Emoji.h"
#include "entities/Note.h"
#include "entities/User.h"
#include "services/FetchService.h"
#include "services/NotificationService.h"

EmojiReact EmojiReact::ingest(const json body) {
  User reacter = FetchService::fetch<User>(body.at("actor"));
  Note note = FetchService::fetch<Note>(body.at("object"));

  URL reacturi(body.at("id"));

  EmojiReact e;
  e.uri = body.at("id");
  e.local = 0;
  e.host = reacturi.host;

  e.owner = body.at("actor");
  e.ownerId = reacter.id;
  e.object = body.at("object");

  optional<Emoji> oem;
  if (body.contains("tag") && body["tag"].is_array() && body["tag"].at(0).is_object()) {
    Emoji em = Emoji::ingestAPTag(body["tag"].at(0), reacturi.host);
    oem = em;
    e.emojiText = nullopt;
    e.emojiId = em.id;
  } else {
    e.emojiId = nullopt;
    e.emojiText = body["content"];
  }

  INSERT_OR_UPDATE(e, uri, id, utils::genid());

  if (isNew && note.local == true) {
    User reactee = User::lookupuri(note.owner).value();
    NotificationService::createReact(note, reactee, reacter, oem, e.emojiText);
  }

  return e;
}
