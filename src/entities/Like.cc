#include "entities/Like.h"
#include "entities/User.h"
#include "services/FetchService.h"
#include "services/NotificationService.h"

Like Like::ingest(const json object) {
  User favoriter = FetchService::fetch<User>(object["actor"]);
  Note note = FetchService::fetch<Note>(object["object"]);

  URL likeuri(object["id"]);
  Like l = {
    .uri = object["id"],
    .id = utils::genid(),
    .local = 0,
    .host = likeuri.host,

    .owner = object["actor"],
    .object = object["object"]
  };

  l.insert();

  if (note.local == true) {
    User favoritee = User::lookupuri(note.owner).value();
    NotificationService::createFavorite(note, favoritee, favoriter);
  }

  return l;
}
