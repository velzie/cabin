#pragma once
#include "database.h"
#include "utils.h"
#include "entities/User.h"

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
  string conversation;

  string content;
  optional<string> cw;
  int sensitive;

  string owner;

  time_t published;
  time_t publishedClamped;
  time_t recievedAt;

  time_t lastUpdatedAt;
  int remoteRenoteCount;
  int remoteReplyCount;
  int remoteLikeCount;

  ORM(note, uri,
    F(uri)
    F(id)
    F(local)
    F(host)

    F(visibility)
    OPT(replyToUri)
    OPT(renoteUri)
    F(conversation)

    F(content)
    OPT(cw)
    F(sensitive)

    F(owner)
    F(published)
    F(publishedClamped)
    F(recievedAt)

    F(lastUpdatedAt)
    F(remoteRenoteCount)
    F(remoteReplyCount)
    F(remoteLikeCount)
  )


  json renderAP() {
    ASSERT(local);
    json j = {
      {"@context", context},
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