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
        {"name", nullptr}
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
  
  json renderMS() {
   return {
    {"id", id},
    {"username", username},
    {"acct", acct(true)},
    {"display_name", displayname},
    {"locked", false},
    {"bot", false},
    {"created_at", "2016-03-16T14:34:26.392Z"},
    {"note", summary},
    {"url", USERPAGE(id)},
    {"avatar", avatar},
    {"avatar_static", avatar},
    {"header", banner},
    {"header_static", banner},
    {"followers_count", 320472},
    {"following_count", 453},
    {"statuses_count", 61163},
    {"last_status_at", "2019-12-05T03:03:02.595Z"},
    {"emojis", json::array()},
    {"fields", json::array()},
    {"pleroma",{
      {"background_image", nullptr},
      {"skip_thread_containment", false},
      {"is_moderator", false},
      {"is_admin", true},
      {"ap_id", uri},
      {"tags", ARR},
      {"also_known_as", {}},
      {"hide_follows", true},
      {"hide_follows_count", false},
      {"hide_followers", true},
      {"hide_followers_count", false},
      {"is_confirmed", true},
      {"is_suggested", false},
      {"hide_favorites", true},
      {"favicon", FMT("https://{}/favicon.png", host)},
      {"relationship", json::object()},
    }}
  };
}
};
