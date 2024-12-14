#include <common.h>
#include <httplib.h>
#include "entities/Note.h"
#include "entities/Emoji.h"
#include "entities/EmojiReact.h"
#include <stdexcept>

#include "database.h"
#include "utils.h"
#include "http.h"

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

    n.owner = USERPAGE(owner.id);
    n.published = published;
    n.publishedClamped = published;
    n.recievedAt = published;
    return n;
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
      auto emoji = Emoji::lookupaddress(emojiName);
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
}
