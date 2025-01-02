#pragma once
#include "database.h"
#include "entity.h"
#include "utils.h"
#include "entities/User.h"

#define NOTEVISIBILITY_Public 0
#define NOTEVISIBILITY_Home 1
#define NOTEVISIBILITY_Followers 2
#define NOTEVISIBILITY_Direct 3


struct NoteMention {
  string uri;
  string friendlyUrl;
  string fqn;
  string id;

  static NoteMention fromUser(User &u) {
    NoteMention nm;
    nm.uri = u.uri;
    nm.friendlyUrl = u.friendlyUrl;
    nm.fqn = u.acct(false);
    nm.id = u.id;
    return nm;
  }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(NoteMention, uri, fqn, id, friendlyUrl);

struct NoteHashtag {
  string name;
  string href;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(NoteHashtag, name, href);

struct NoteEmoji {
  string shortcode; // note that this won't change even if the db's entry does
  string id; // our id's not the ap one
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(NoteEmoji, shortcode, id);


struct Note : Entity {
  string uri;
  string id;
  int local;
  string host;

  int visibility;
  optional<string> replyToUri;
  optional<string> renoteUri;
  optional<string> quoteUri;
  string conversation;

  string content;
  optional<string> cw;
  int sensitive;

  string owner;

  std::vector<NoteMention> mentions;
  std::vector<NoteHashtag> hashtags;
  std::vector<NoteEmoji> emojis;
  std::vector<string> mediaIds;

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
    OPT(quoteUri)
    F(conversation)

    F(content)
    OPT(cw)
    F(sensitive)

    F(owner)

    JSON(mentions)
    JSON(hashtags)
    JSON(emojis)
    JSON(mediaIds)

    F(published)
    F(publishedClamped)
    F(recievedAt)

    F(lastUpdatedAt)
    F(remoteRenoteCount)
    F(remoteReplyCount)
    F(remoteLikeCount)
  )

  LOOKUPKEY(Note, note, id);
  LOOKUPKEY(Note, note, uri);

  static Note ingest(const json note);
  static Note ingestAnnounce(const json data);
  json renderAP();
  json renderReactionsMS(User &requester, bool fullAccounts);
  json renderMS(User &requester);
};
