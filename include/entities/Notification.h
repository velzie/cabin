#pragma once
#include "database.h"
#include "entities/User.h"

#define NOTIFICATION_Follow 0
#define NOTIFICATION_Mention 1
#define NOTIFICATION_Renote 2
#define NOTIFICATION_Favorite 3
#define NOTIFICATION_Poll 4
#define NOTIFICATION_Update 5
#define NOTIFICATION_Bite 6
#define NOTIFICATION_React 7

struct Notification {
  string id;
  int type;

  time_t createdAt;

  optional<string> notifierUri;
  optional<string> notifierId;
  string notifieeUri;
  string notifieeId;

  optional<string> statusUri;
  optional<string> statusId;

  optional<string> emojiText;
  optional<string> emojiUrl;

  bool isread;

  ORM(notification, id,
    F(id)
    F(type)

    F(createdAt)

    OPT(notifierUri)
    OPT(notifierId)
    F(notifieeUri)
    F(notifieeId)

    OPT(statusUri)
    OPT(statusId)

    OPT(emojiText)
    OPT(emojiUrl)

    BOOL(isread)
  )

  LOOKUPKEY(Notification, notification, id);
  json renderMS(User &requester);
};
