#include "services/notification.h"

#include "entities/Notification.h"
#include "utils.h"
#include "services/user.h"
#include "services/note.h"

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
  }
}

json Notification::renderMS(User &requester){
    json notif = {
      {"id", id},
      {"created_at", utils::millisToIso(createdAt)},
    };

    if (type == NOTIFICATION_Follow) {
      auto follower = UserService::lookup(notifierId.value()).value();
      notif["type"] = "follow";
      notif["account"] = follower.renderMS();
    }

    if (type == NOTIFICATION_Favorite) {
      auto favoriter = UserService::lookup(notifierId.value()).value();
      auto note = NoteService::lookup(statusId.value()).value();
      notif["type"] = "favorite";
      notif["account"] = favoriter.renderMS();
      notif["status"] = note.renderMS(requester);
    }

    return notif;
  }
