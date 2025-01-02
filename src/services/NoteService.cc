#include <common.h>
#include <httplib.h>
#include "entities/Note.h"
#include "entities/Emoji.h"
#include "entities/EmojiReact.h"
#include <stdexcept>

#include "database.h"
#include "utils.h"
#include "http.h"
#include "querybuilder.h"

#include "services/DeliveryService.h"


namespace NoteService {

  Note _create(User &owner) {
    string id = utils::genid();
    time_t published = utils::millis();
    Note n;
    n.uri = NOTE(id);
    n.id = id;
    n.local = 1;
    n.host = cfg.domain;
    n.visibility = NOTEVISIBILITY_Public;

    n.owner = USERPAGE(owner.id);
    n.published = published;
    n.publishedClamped = published;
    n.recievedAt = published;
    n.conversation = utils::genid();
    return n;
  }

  Note create(User &owner, string content, optional<string> contentWarning, optional<Note> replyTo, optional<Note> quote, bool preview, int visibility, vector<string> mediaIds) {
    Note note = _create(owner);
    note.content = content;
    note.cw = contentWarning;
    note.sensitive = false;
    note.visibility = visibility;
    note.mediaIds = mediaIds;
    

    // find every emoji inside content (identified by :name:)
    size_t pos = 0;
    while ((pos = content.find(":", pos)) != string::npos) {
      size_t end = content.find(":", pos + 1);
      if (end == string::npos) {
        break;
      }
      string emojiName = content.substr(pos + 1, end - pos - 1);
      auto emoji = Emoji::lookupaddress(emojiName);
      if (emoji.has_value()) {
        note.emojis.push_back(emoji->renderNoteEmoji());
      }
      pos = end + 1;
    }


    vector<string> extramentions;
    // find every @user@instance.tld
    pos = 0;
    while ((pos = content.find("@", pos)) != string::npos) {
      size_t end = content.find(" ", pos + 1);
      if (end == string::npos) {
        end = content.length();
      }
      string mention = content.substr(pos + 1, end - pos - 1);

      std::stringstream ss(mention);
      std::string user;
      std::getline(ss, user, '@');
      std::string instance;
      std::getline(ss, instance, '@');
      dbg(user, instance);

      QueryBuilder q;
      auto mentionee = q.select({"*"})
            .from("user")
            .where(EQ("username", user))
            .where(EQ("host", instance))
            .getOne<User>();

      if (mentionee.has_value()) {
        note.mentions.push_back(NoteMention::fromUser(mentionee.value()));
        extramentions.push_back(mentionee.value().uri);
      }
      pos = end + 1;
    }


    if (replyTo.has_value()) {
      note.replyToUri = replyTo->uri;
      extramentions.push_back(replyTo->owner);
    }

    if (quote.has_value()) {
      note.quoteUri = quote->uri;
      extramentions.push_back(quote->owner);
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

      activity["to"] = activity["object"]["to"];
      activity["cc"] = activity["object"]["cc"];

      DeliveryService::Audience au = {
        .actor = owner,
        .mentions = extramentions,
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
    note.visibility = NOTEVISIBILITY_Public;

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
}
