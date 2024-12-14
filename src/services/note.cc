#include <common.h>
#include <httplib.h>
#include "entities/Note.h"
#include "services/emoji.h"
#include <stdexcept>

#include "database.h"
#include "utils.h"
#include "http.h"

#include "services/delivery.h"
#include "services/note.h"
#include "services/user.h"


namespace NoteService {

  Note _create(User &owner) {
    string id = utils::genid();
    time_t published = utils::millis();
    Note note = {
      .uri = NOTE(id),
      .id = id,
      .local = 1,
      .host = cfg.domain,

      .owner = USERPAGE(owner.id),
      .published = published,
      .publishedClamped = published,
      .recievedAt = published
    };
    return note;
  }

  Note create(User &owner, string content, optional<Note> replyTo, optional<Note> quote, bool preview) {
    Note note = _create(owner);
    note.content = content;
    note.cw = "";
    note.sensitive = false;
    

    // find every emoji inside content (identified by :name:)
    size_t pos = 0;
    while ((pos = content.find(":", pos)) != string::npos) {
      size_t end = content.find(":", pos + 1);
      if (end == string::npos) {
        break;
      }
      string emojiName = content.substr(pos + 1, end - pos - 1);
      auto emoji = EmojiService::lookupAddress(emojiName);
      if (emoji.has_value()) {
        note.emojis.push_back(emoji->renderNoteEmoji());
      }
      pos = end + 1;
    }


    std::vector<string> mentions;
    if (replyTo.has_value()) {
      note.replyToUri = replyTo->uri;
      mentions.push_back(replyTo->owner);
    }

    if (quote.has_value()) {
      note.quoteUri = quote->uri;
      mentions.push_back(quote->owner);
    }

    if (!preview) {
      note.insert();

      json activity = {
        {"type", "Create"},
        {"actor", note.owner},
        {"id", NOTE(note.id)+"/activity"},
        {"published", utils::millisToIso(note.published)},
        {"object", note.renderAP()},
      };

      DeliveryService::Audience au = {
        .actor = owner,
        .mentions = mentions,
        .aspublic = true,
        .followers = true,
      };
      DeliveryService::QueueDelivery(activity, au);
    }

    return note;
  }

  Note createRenote(User &owner, Note &renotee) {
    Note note = _create(owner);
    note.renoteUri = renotee.uri;
    note.insert();

    json activity = {
      {"type", "Announce"},
      {"actor", owner.uri},
      {"id", NOTE(note.id)+"/activity"},
      {"published", utils::millisToIso(note.published)},
      {"to", {"https://www.w3.org/ns/activitystreams#Public"}},
      {"cc", {"https://www.w3.org/ns/activitystreams#Public", "https://wetdry.world/users/ce", "https://booping.synth.download/users/a005c9wl4pwj0arp"}},
      {"object", renotee.uri},
    };

    DeliveryService::Audience au = {
      .actor = owner,
      .mentions = {renotee.owner},
      .aspublic = true,
      .followers = true,
    };
    DeliveryService::QueueDelivery(activity, au);

    return note;
  }

  Note ingest(const string uri, const json note) {
    URL url(uri);

    std::time_t published = utils::isoToMillis(note["published"]);
    Note n = {
      .uri = uri,
      .local = false,
      .host = url.host,

      .replyToUri = nullopt,
      .conversation = utils::genid(),

      .content = note["content"],
      .sensitive = false,
      .owner = note["attributedTo"],
      .published = published,
      .publishedClamped = utils::clampmillis(published),
      .recievedAt = utils::millis(),


      .lastUpdatedAt = utils::millis()
    };

    if (note.contains("_misskey_quote") && note["_misskey_quote"].is_string())
      n.quoteUri = note["_misskey_quote"];
    else if (note.contains("quoteUri") && note["quoteUri"].is_string())
      n.quoteUri = note["quoteUri"];
    else
      n.quoteUri = nullopt;

    if (n.quoteUri.has_value()) {
      fetchRemote(n.quoteUri.value());
    }
      

    if (note.contains("tag") && note["tag"].is_array()) {
      for (const auto tag : (std::vector<json>)note["tag"]) {
        if (tag["type"] == "Mention") {
          NoteMention m;
          auto mentionee = UserService::fetchRemote(tag["href"]);
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
          Emoji em = EmojiService::parse(tag, n.host);
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

    UserService::fetchRemote(note["attributedTo"]);

    if (n.replyToUri.has_value())
      // TODO: if one ingest fails they will all fail, maybe not good
      fetchRemote(n.replyToUri.value());

    // recursively fetch until the initial post in the thread
    Note topmost = n;
    while (topmost.replyToUri.has_value()) {
      topmost = NoteService::lookup_ap(topmost.replyToUri.value()).value();
    }

    // now we can safely take the conversation id
    n.conversation = topmost.conversation;
    INSERT_OR_UPDATE(n, uri, id, utils::genid());

    return n;
  }

  Note fetchRemote(const string uri) {
    auto cached = lookup_ap(uri);
    if (cached.has_value()) {
      // TODO: don't skip if it's been a while
      return cached.value();
    }

    trace("fetching note {}", uri);
    auto u = UserService::lookup(cfg.instanceactor);
    URL url(uri);
    APClient cli(u.value(), url.host);

    auto response = cli.Get(url.path);

    json note = json::parse(response->body);

    Note n = ingest(uri, note);
    return n;
  }

  optional<Note> lookup(const string id) {
    auto q = STATEMENT("SELECT * FROM note WHERE id = ?");
    q.bind(1, id);

    if (!q.executeStep()) {
      return nullopt;
    }
  
    Note n;
    n.load(q);
    return n;
  }

  optional<Note> lookup_ap(const string uri) {
    auto q = STATEMENT("SELECT * FROM note WHERE uri = ?");
    q.bind(1, uri);

    if (!q.executeStep()) {
      return nullopt;
    }
  
    Note n;
    n.load(q);
    return n;
  }
}


json Note::renderMS(User &requester) {
  User uOwner = UserService::lookup_ap(owner).value();

  Note nReplyTo;
  if (replyToUri.has_value()) {
    nReplyTo = NoteService::lookup_ap(replyToUri.value()).value();
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
    {"url", NOTE(id)},
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
    {"emoji_reactions", json::array()},
    {"reactions", json::array()},
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
      {"emoji_reactions", ARR},
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
    auto nQuote = NoteService::lookup_ap(quoteUri.value()).value();
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


  if (replyToUri.has_value())
    j["in_reply_to_account_id"] = nReplyTo.owner;

  if (renoteUri.has_value()) {
    auto n = NoteService::lookup_ap(renoteUri.value());
    j["reblog"] = n.value().renderMS(requester);
  }

  return j;
}

json Note::renderAP() {
    ASSERT(local);
    vector<json> tags;

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
      Emoji em = EmojiService::lookup(emoji.id).value();
      tags.push_back(em.renderTag());
    }

    


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
      {"tag", tags}
    };

    if (quoteUri.has_value()) {
      j["_misskey_quote"] = *quoteUri;
      j["quoteUri"] = *quoteUri;
    }

    return j;
  }
