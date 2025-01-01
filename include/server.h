#pragma once
#include "entities/Note.h"
#include "entities/Notification.h"
#include <variant>
#pragma once
#define HEADERLIST "Authorization, Content-Type, Link"

using ServerEvent = std::variant<Notification, Note>;
namespace Server {
  void Listen();
  void PublishEvent(ServerEvent e);
}
