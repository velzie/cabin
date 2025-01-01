#include "entities/Notification.h"
#include "entities/Note.h"

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

    if (type == NOTIFICATION_Mention) {
      auto mentioner = User::lookupid(notifierId.value()).value();
      auto note = Note::lookupid(statusId.value()).value();
      notif["type"] = "mention";
      notif["account"] = mentioner.renderMS();
      notif["status"] = note.renderMS(requester);
    }

    if (type == NOTIFICATION_Bite) {
      auto biter = User::lookupid(notifierId.value()).value();
      notif["type"] = "bite";
      notif["account"] = biter.renderMS();
    }

    return notif;
  }
