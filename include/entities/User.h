#pragma once
#include "database.h"

struct User {
  string uri;
  string id;
  int local;
  string host;

  string publicKey;
  string privateKey;

  string username;
  string displayname;
  string summary;
  string friendlyUrl;

  std::time_t lastUpdatedAt;
  int remoteFollowersCount;
  int remoteFollowingCount;
  int remoteNotesCount;

  string avatar;
  string banner;

int isBot;
  int isCat;
  int speakAsCat;

  string inbox;
  string sharedInbox;
  string featured;

  ORM(user, uri,
    F(uri)
    F(id)
    F(local)
    F(host)

    F(publicKey)
    F(privateKey)

    F(username)
    F(displayname)
    F(summary)
    F(friendlyUrl)

    F(lastUpdatedAt)
    F(remoteFollowersCount);
    F(remoteFollowingCount);
    F(remoteNotesCount);

    F(avatar)
    F(banner)

    F(isBot)
    F(isCat)
    F(speakAsCat)

    F(inbox)
    F(sharedInbox)
    F(featured)
  )

  LOOKUPKEY(User, user, id);
  LOOKUPKEY(User, user, uri);

  static User ingest(const json object);
  json renderAP() {
    ASSERT(local);

    string userurl = USERPAGE(id);
    json j = {
      {"@context", context},
      {"type", "Person"},
      {"id", userurl},
      {"inbox", userurl + "/inbox"},
      {"outbox", userurl + "/outbox"},
      {"followers", userurl + "/followers"},
      {"following", userurl + "/following"},
      {"featured", userurl + "/featured"},
      {"sharedinbox", API("inbox")},
      {"endpoints", { 
        {"sharedInbox", API("inbox")} 
      } },
      {"isCat", (bool)isCat},
      {"speakAsCat", (bool)speakAsCat},
      {"publicKey", {
        {"id", userurl},
        {"owner", userurl},
        {"publicKeyPem", publicKey},
      }},
      {"url", userurl},
      {"preferredUsername", username},
      {"name", displayname},
      {"summary", summary},
      {"icon", {
        {"type", "Image"},
        {"url", avatar},
        {"sensitive", false},
        // {"name", nullptr}
      }},
      {"discoverable", true},
      {"noindex", true},
      {"attachment", json::array()},
      {"alsoKnownAs", json::array()},
      {"tag", ARR}
    };
    return j;
  }

  string acct(bool omitLocalHost) {
    if (omitLocalHost && host == cfg.domain) {
      return username;
    }

    return FMT("{}@{}", username, host);
  }
  
  json renderMS();
};
