#pragma once
#include "../schema.h"
#include "../utils.h"

#include "User.h"

#define NOTEVISIBILITY_Public 0
#define NOTEVISIBILITY_Home 1
#define NOTEVISIBILITY_Followers 2
#define NOTEVISIBILITY_Direct 3
struct Note {
  string uri;
  string id;
  int local;
  string host;

  int visibility;
  optional<string> replyToUri;
  optional<string> renoteUri;

  string content;
  optional<string> cw;
  int sensitive;

  string owner;
  std::time_t published;

  std::time_t lastUpdatedAt;
  int remoteRenoteCount;
  int remoteReplyCount;
  int remoteLikeCount;

  ORM(note,
    F(uri)
    F(id)
    F(local)
    F(host)

    F(visibility)
    OPT(replyToUri)
    OPT(renoteUri)

    F(content)
    OPT(cw)
    F(sensitive)

    F(owner)
    F(published)

    F(lastUpdatedAt)
    F(remoteRenoteCount)
    F(remoteReplyCount)
    F(remoteLikeCount)
  )


  json renderAP() {
    ASSERT(local);
    json j = {
      {"@context", ct->context},
      {"id", NOTE(id)},
      {"type", "Note"},
      {"inReplyTo", replyToUri},
      {"published", utils::millisToIso(published)},
      {"url", NOTE(id)},
      {"attributedTo", owner},
      {"to", {"https://www.w3.org/ns/activitystreams#Public"}},
      {"cc", {owner+ "/followers"}},
      {"sensitive", false},
      {"content", content},
      {"attachment", ARR},
      {"tag", ARR}
    };

    return j;
  }

  json renderMS(User &requester);
};
