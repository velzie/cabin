#include "services/NotificationService.h"

#include "entities/Emoji.h"
#include "entities/Notification.h"
#include "utils.h"

namespace NotificationService {
  Notification create(User &notifiee, int type) {
    Notification n;
    n.id = utils::genid();
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

json Notification::renderMS(User &requester){
    json notif = {
      {"id", id},
      {"created_at", utils::millisToIso(createdAt)},
      {"pleroma", json::object()}
    };

    if (type == NOTIFICATION_Follow) {
      auto follower = User::lookupid(notifierId.value()).value();
      notif["type"] = "follow";
      notif["account"] = follower.renderMS();
    }

    if (type == NOTIFICATION_Favorite) {
      auto favoriter = User::lookupid(notifierId.value()).value();
      auto note = Note::lookupid(statusId.value()).value();
      notif["type"] = "favourite";
      notif["account"] = favoriter.renderMS();
      notif["status"] = note.renderMS(requester);
    }

    if (type == NOTIFICATION_Renote) {
      auto renoter = User::lookupid(notifierId.value()).value();
      auto note = Note::lookupid(statusId.value()).value();
      notif["type"] = "reblog";
      notif["account"] = renoter.renderMS();
      notif["status"] = note.renderMS(requester);
    }

    if (type == NOTIFICATION_React) {
      auto favoriter = User::lookupid(notifierId.value()).value();
      auto note = Note::lookupid(statusId.value()).value();
      notif["type"] = "pleroma:emoji_reaction";
      notif["account"] = favoriter.renderMS();
      notif["status"] = note.renderMS(requester);

      if (emojiUrl.has_value()) {
        notif["emoji"] = FMT(":{}:", emojiText.value());
        notif["emoji_url"] = emojiUrl.value();
      } else {
        notif["emoji"] = emojiText.value();
      }
    }

    return notif;
  }
