#include "entities/Note.h"
#include "entities/Emoji.h"
#include "entities/EmojiReact.h"
#include "services/FetchService.h"
#include "services/NotificationService.h"


Note Note::ingestAnnounce(const json data) {
  string object = data["object"];
  auto note = get<Note>(FetchService::fetch(object));

  User uRenoter = FetchService::fetch<User>(data["actor"]);

  time_t published = utils::isoToMillis(data["published"]);
  Note rn;
  rn.uri = data["id"];
  rn.id = utils::genid();
  rn.local = false;
  rn.host = uRenoter.host;

  rn.renoteUri = note.uri;
    
  rn.owner = uRenoter.uri;
  rn.published = published;
  rn.publishedClamped = utils::clampmillis(published);
  rn.recievedAt = utils::millis();
  rn.insert();

  if (note.local) {
    User uRenotee = User::lookupuri(note.owner).value();
    NotificationService::createRenote(note, rn, uRenotee, uRenoter);
  }

  return rn;
}

Note Note::ingest(const json note) {
  URL url(note["id"]);

  std::time_t published = utils::isoToMillis(note["published"]);
  Note n;
  n.uri = note["id"];
  n.local = false;
  n.host = url.host;

  n.replyToUri = nullopt;
  n.conversation = utils::genid();

  n.content = note["content"];
  n.sensitive = false;
  n.owner = note["attributedTo"];
  n.published = published;
  n.publishedClamped = utils::clampmillis(published);
  n.recievedAt = utils::millis();

  if (
      note["to"].is_string() && note["to"] == AS_PUBLIC ||
      note["to"].is_array() && note["to"].contains(AS_PUBLIC)) {
    n.visibility = NOTEVISIBILITY_Public;
  } else if (
      note["cc"].is_string() && note["cc"] == AS_PUBLIC ||
      note["cc"].is_array() && note["cc"].contains(AS_PUBLIC)) {
    n.visibility = NOTEVISIBILITY_Home;
  } else {
    n.visibility = NOTEVISIBILITY_Followers;
    // how is direct going to work?
  }


  n.lastUpdatedAt = utils::millis();

  if (note.contains("_misskey_quote") && note["_misskey_quote"].is_string())
    n.quoteUri = note["_misskey_quote"];
  else if (note.contains("quoteUri") && note["quoteUri"].is_string())
    n.quoteUri = note["quoteUri"];
  else
    n.quoteUri = nullopt;

  if (n.quoteUri.has_value()) {
    FetchService::fetch(n.quoteUri.value());
  }
    

  if (note.at("tag").is_array()) {
    for (const auto tag : note["tag"]) {
      if (tag["type"] == "Mention") {
        NoteMention m;
        auto mentionee = get<User>(FetchService::fetch(tag["href"]));
        m.friendlyUrl = mentionee.friendlyUrl;
        m.id = mentionee.id;
        m.fqn = mentionee.acct(false);
        m.uri = mentionee.uri;

        n.mentions.push_back(m);
      } else if (tag["type"] == "Hashtag") {
        NoteHashtag h;
        h.href = tag["href"];
        h.name = tag["name"];
        h.name.erase(0, 1); // remove hashtag
        
        n.hashtags.push_back(h);
      } else if (tag["type"] == "Emoji") {
        Emoji em = Emoji::ingestAPTag(tag, n.host);
        NoteEmoji nem;
        nem.id = em.id;
        nem.shortcode = em.shortcode;
        n.emojis.push_back(nem);
      } else {
        error("unkown Tag type {}", (string)tag["type"]);
      }
    }
  }

  if (note.contains("attachment") && note["attachment"].is_array()) {
    for (const auto attachment : (std::vector<json>)note["attachment"]) {
      if (attachment["type"] == "Document") {
        NoteAttachment a;
        a.url = attachment["url"];

        if (attachment.contains("name") && attachment["name"].is_string()) {
          a.description = attachment["name"];
        }

        if (attachment.contains("blurhash") && attachment["blurhash"].is_string()) {
          a.blurhash = attachment["blurhash"];
        }

        a.sensitive = false;
        if (attachment.contains("sensitive") && attachment["sensitive"].is_boolean()) {
          a.sensitive = attachment["sensitive"];
        }
        n.mediaattachments.push_back(a);
      }
    }
  }

  if (note.contains("sensitive") && note["sensitive"].is_boolean()) {
    n.sensitive = note["sensitive"];
  }
  if (note.contains("inReplyTo") && !note["inReplyTo"].is_null()) {
    n.replyToUri = std::make_optional(note["inReplyTo"]);
  }

  FetchService::fetch(note["attributedTo"]);

  if (n.replyToUri.has_value())
    // TODO: if one ingest fails they will all fail, maybe not good
    FetchService::fetch(n.replyToUri.value());

  // recursively fetch until the initial post in the thread
  Note topmost = n;
  while (topmost.replyToUri.has_value()) {
    topmost = Note::lookupuri(topmost.replyToUri.value()).value();
  }

  // now we can safely take the conversation id
  n.conversation = topmost.conversation;
  INSERT_OR_UPDATE(n, uri, id, utils::genid());

  return n;
}


json Note::renderReactionsMS(User &requester, bool fullAccounts) {
  std::map<string, json> emojireacts;
  auto qReacted = STATEMENT("SELECT * FROM emojireact WHERE object = ?");
  qReacted.bind(1, uri);
  while (qReacted.executeStep()) {
    EmojiReact emreact;
    emreact.load(qReacted);

    string emname;
    if (emreact.emojiId.has_value()) {
      Emoji em = Emoji::lookupid(emreact.emojiId.value()).value();
      emname = em.address;
    } else {
      emname = emreact.emojiText.value();
    }

    if (emojireacts.count(emname)) {
      json r = emojireacts[emname];
      r["count"] = r["count"].get<int>() + 1;

      if (fullAccounts) {
        auto acc = User::lookupid(emreact.ownerId).value();
        r["accounts"].push_back(acc.renderMS());
      } else {
        r["account_ids"].push_back(emreact.ownerId);
      }

      emojireacts[emname] = r;
    } else {
      json r = {
        {"count", 1},
        {"name", emname},
        {"me", false}, // TODO:
      };

      if (fullAccounts) {
        auto acc = User::lookupid(emreact.ownerId).value();
        r["accounts"] = {acc.renderMS()};
      } else {
        r["account_ids"] = {emreact.ownerId};
      }

      if (emreact.emojiId.has_value()) {
        Emoji em = Emoji::lookupid(emreact.emojiId.value()).value();
        r["url"] = em.imageurl;
        r["static_url"] = em.imageurl;
      }

      emojireacts.insert({emname, r});
    }
  }

  vector<json> emojireactsarr;
  for (const auto [key, value] : emojireacts) {
    emojireactsarr.push_back(value);
  }

  return emojireactsarr;
}

json Note::renderMS(User &requester) {
  User uOwner = User::lookupuri(owner).value();

  Note nReplyTo;
  if (replyToUri.has_value()) {
    nReplyTo = Note::lookupuri(replyToUri.value()).value();
  }

  auto favs = STATEMENT("SELECT COUNT(*) FROM like WHERE object = ?");
  favs.bind(1, uri);
  favs.executeStep();
  int favouriteCount = favs.getColumn(0);

  auto qFavourited = STATEMENT("SELECT COUNT(*) FROM like WHERE object = ? AND owner = ?");
  qFavourited.bind(1, uri);
  qFavourited.bind(2, requester.uri);
  qFavourited.executeStep();
  bool favourited = (int)qFavourited.getColumn(0) > 0;

  auto qRenoted = STATEMENT("SELECT COUNT(*) from note WHERE renoteUri = ?");
  qRenoted.bind(1, uri);
  qRenoted.executeStep();
  int renoteCount = qRenoted.getColumn(0);

  auto qReplied = STATEMENT("SELECT COUNT(*) from note WHERE replyToUri = ?");
  qReplied.bind(1, uri);
  qReplied.executeStep();
  int replyCount = qReplied.getColumn(0);

  vector<json> msemojis;
  for (const auto noteemoji : emojis) {
    auto qEmoji = STATEMENT("SELECT * FROM emoji WHERE id = ?");
    qEmoji.bind(1, noteemoji.id);
    qEmoji.executeStep();
    Emoji emoji;
    emoji.load(qEmoji);

    msemojis.push_back({
      {"url", emoji.imageurl},
      {"shortcode", emoji.address},
      {"static_url", emoji.imageurl},
      {"visible_in_picker", false}
    });
  }

  auto emojireactsarr = renderReactionsMS(requester, false);

  json j = {
    {"id", id},
    {"created_at", utils::millisToIso(published)},
    {"in_reply_to_id", nReplyTo.id},
    {"in_reply_to_account_id", nullptr},
    {"quote_id", nullptr},
    {"quote", nullptr},
    {"sensitive", sensitive},
    {"spoiler_text", ""},
    {"visibility", "public"},
    {"language", "en"},
    {"uri", uri},
    {"url", uri},
    {"replies_count", replyCount},
    {"reblogs_count", renoteCount},
    {"favourites_count", favouriteCount},
    {"favourited", favourited},
    {"reblogged", false},
    {"muted", false},
    {"pinned", false},
    {"bookmarked", false},
    {"content", content},
    {"reblog", nullptr},
    {"application", nullptr},
    {"account", uOwner.renderMS()},
    {"emojis", msemojis},
    {"emoji_reactions", emojireactsarr},
    {"reactions", emojireactsarr},
    {"card", nullptr},
    {"poll", nullptr},

    {"pleroma", {
      {"local", (bool)local},
      {"context", ""},
      {"content", {
        {"text/plain", ""}
      }},
      {"expires_at", nullptr},
      {"direct_conversation_id", nullptr},
      {"emoji_reactions", emojireactsarr},
      {"spoiler_text", {
        {"text/plain", ""}
      }},
      {"pinned_at", nullptr},
      {"conversation_id", conversation},
      {"in_reply_to_account_acct", nullptr},
      {"parent_visible", true},
      {"thread_muted", false},
    }},
    {"akkoma", {
      {"source", {
        {"content", "A"},
        {"mediaType", "text/plain"}
      }}
    }},
    {"edited_at", nullptr},
    {"text", nullptr}
  };

  if (quoteUri.has_value()) {
    auto nQuote = Note::lookupuri(quoteUri.value()).value();
    j["quote_id"] = nQuote.id;
    j["quote"] = nQuote.renderMS(requester);
  }

  std::vector<json> respmentions;
  for (const auto mention : mentions) {
    std::stringstream acct(mention.fqn);
    string user, host;
    std::getline(acct, user, '@');
    std::getline(acct, host);

    respmentions.push_back({
        {"id", mention.id},
        {"url", mention.friendlyUrl},
        {"username", user},
        {"acct", mention.fqn}
    });
  }
  j["mentions"] = respmentions;


  std::vector<json> resptags;
  for (const auto hashtag : hashtags) {
    resptags.push_back({
      {"url", hashtag.href},
      {"name", hashtag.name}
    });
  }
  j["tags"] = resptags;

  std::vector<json> respmedia_attachments;
  for (const auto attachment : mediaattachments) {
    respmedia_attachments.push_back({
      {"id", utils::genid()},
      {"type", "image"},
      {"url", attachment.url},
      {"preview_url", attachment.url},
      {"remote_url", attachment.url},
      {"blurhash", attachment.blurhash},
      {"description", attachment.description}
    });
  }
  j["media_attachments"] = respmedia_attachments;

  if (visibility == NOTEVISIBILITY_Public) {
    j["visibility"] = "public";
  } else if (visibility == NOTEVISIBILITY_Home) {
    j["visibility"] = "unlisted";
  } else if (visibility == NOTEVISIBILITY_Followers) {
    j["visibility"] = "private";
  } else if (visibility == NOTEVISIBILITY_Direct) {
    j["visibility"] = "direct";
  } else {
    ASSERT(0);
  }


  if (replyToUri.has_value())
    j["in_reply_to_account_id"] = nReplyTo.owner;

  if (renoteUri.has_value()) {
    auto n = Note::lookupuri(renoteUri.value());
    j["reblog"] = n.value().renderMS(requester);
  }

  return j;
}

json Note::renderAP() {
    ASSERT(local);
    vector<json> tags;

    auto uOwner = User::lookupuri(owner).value();

    for (const auto hashtag : hashtags) {
      tags.push_back({
        {"type", "Hashtag"},
        {"href", hashtag.href},
        {"name", hashtag.name}
      });
    }

    for (const auto mention : mentions) {
      tags.push_back({
        {"type", "Mention"},
        {"href", mention.uri},
        {"name", mention.fqn}
      });
    }

    for (const auto emoji : emojis) {
      Emoji em = Emoji::lookupid(emoji.id).value();
      tags.push_back(em.renderAPTag());
    }

    


    json j = {
      // {"@context", context},
      {"id", NOTE(id)},
      {"type", "Note"},
      {"inReplyTo", replyToUri},
      {"published", utils::millisToIso(published)},
      {"url", NOTE(id)},
      {"attributedTo", owner},
      {"sensitive", false},
      {"content", content},
      {"attachment", ARR},
      {"tag", tags}
    };

    if (cw.has_value()) {
      j["cw"] = cw.value();
    }

    if (visibility == NOTEVISIBILITY_Public) {
      j["to"] = {AS_PUBLIC};
      j["cc"] = {FOLLOWERS(uOwner.id)};
    } else if (visibility == NOTEVISIBILITY_Home) {
      j["to"] = {FOLLOWERS(uOwner.id)};
      j["cc"] = {AS_PUBLIC};
    } else if (visibility == NOTEVISIBILITY_Followers) {
      j["to"] = {FOLLOWERS(uOwner.id)};
      j["cc"] = {FOLLOWERS(uOwner.id)};
    } else if (visibility == NOTEVISIBILITY_Direct) {
      j["to"] = ARR;
      j["cc"] = ARR;
    } else {
      ASSERT(0);
    }

    for (const auto mention : mentions) {
      j["to"].push_back(mention.uri);
    }

    if (quoteUri.has_value()) {
      j["_misskey_quote"] = *quoteUri;
      j["quoteUri"] = *quoteUri;
    }

    return j;
  }
