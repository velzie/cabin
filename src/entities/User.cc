#include "common.h"
#include "utils.h"
#include "entities/User.h"

User User::ingest(const json user) {
  URL url(user["id"]);

  User u;
  u.uri = user["id"];
  u.local = false;
  u.host = url.host;
    
  u.username = user["preferredUsername"];
  u.summary = JstringOrEmpty(user, "summary");
  u.friendlyUrl = user["url"];

  u.lastUpdatedAt = utils::millis();
  u.isCat = JboolOrFalse(user, "isCat");
  u.speakAsCat = JboolOrFalse(user, "speakAsCat");

  u.inbox = user["inbox"];
  u.featured = user["featured"];

  if (user.contains("sharedInbox") && user["sharedInbox"].is_string())
    u.sharedInbox = user["sharedInbox"];
  else
    u.sharedInbox = user["inbox"];

  if (user.contains("name") && user["name"].is_string())
    u.displayname = user["name"];
  else
    u.displayname = user["preferredUsername"];

  if (user.contains("icon") && user["icon"].is_object())
    u.avatar = user["icon"]["url"];

  if (user.contains("image") && user["image"].is_string())
    u.banner = user["image"]["url"];

  INSERT_OR_UPDATE(u, uri, id, utils::genid());
  return u;
}
