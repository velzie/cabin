#include "services/NotificationService.h"

#include "entities/Emoji.h"
#include "entities/Notification.h"
#include "utils.h"
#include <random>

namespace NotificationService {
  Notification create(User &notifiee, int type) {
    Notification n;
    // id cannot be a regular id for pleroma reasons. do millis + random int
    thread_local std::mt19937 rng(std::random_device{}());
    n.id = std::to_string(utils::millis() * 100 + rng() % 100);
    n.createdAt = utils::millis();
    n.type = type;
    n.notifieeId = notifiee.id;
    n.notifieeUri = notifiee.uri;
    return n;
  }

  void createFollow(User &followee, User &follower) {
    auto n = create(followee, NOTIFICATION_Follow);
    n.notifierId = follower.id;
    n.notifierUri = follower.uri;

    n.insert();
  }

  void createBite(optional<Note> note, User &biteee, User &biter) {
    auto n = create(biteee, NOTIFICATION_Bite);
    n.notifierId = biter.id;
    n.notifierUri = biter.uri;
    if (note.has_value()) {
      n.statusId = note->id;
      n.statusUri = note->uri;
    }
    n.insert();
  }

  void createFavorite(Note &note, User &favoritee, User &favoriter) {
    auto n = create(favoritee, NOTIFICATION_Favorite);
    n.notifierId = favoriter.id;
    n.notifierUri = favoriter.uri;
    n.statusId = note.id;
    n.statusUri = note.uri;

    n.insert();
  }

  void createReact(Note &note, User &reactee, User &reacter, optional<Emoji> &emoji, optional<string> emojiText) {
    auto n = create(reactee, NOTIFICATION_React);
    n.notifierId = reacter.id;
    n.notifierUri = reacter.uri;
    n.statusId = note.id;
    n.statusUri = note.uri;

    if (emoji.has_value()) {
      n.emojiText = emoji->address;
      n.emojiUrl = emoji->imageurl;
    } else {
      n.emojiText = emojiText.value();
    }

    n.insert();
  }

  void createRenote(Note &note, Note &renote, User &renotee, User &renoter) {
    auto n = create(renotee, NOTIFICATION_Renote);
    n.notifierId = renoter.id;
    n.notifierUri = renoter.uri;
    n.statusId = note.id;
    n.statusUri = note.uri;

    n.insert();
  }
}
