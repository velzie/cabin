#include "entities/EmojiReact.h"
#include "services/emoji.h"
#include "services/notification.h"
#include <error.h>
#define USE_DB
#include <common.h>
#include "utils.h"

#include "services/follow.h"
#include "services/user.h"
#include "services/note.h"
#include "services/ingest.h"
#include "entities/Like.h"

#include <cpptrace/from_current.hpp>

namespace IngestService {


  void Ingest(json body) {
    string type = body["type"];
    trace("starting ingest of {}", type);

    auto object = body["object"];
    if (type == "Create") {
      if (object["type"] != "Note") return;
      URL noteuri(object["id"]);
      // todo verify id is right
      
      NoteService::ingest(object["id"], object);
    } else if (type == "Announce") {
      string object = body["object"];
      Note note = NoteService::fetchRemote(object);

      User uRenoter = UserService::fetchRemote(body["actor"]);

      time_t published = utils::isoToMillis(body["published"]);
      Note renote = {
        .uri = body["id"],
        .id = utils::genid(),
        .local = false,
        .host = uRenoter.host,

        .renoteUri = note.uri,
        
        .owner = uRenoter.uri,
        .published = published,
        .publishedClamped = utils::clampmillis(published),
        .recievedAt = utils::millis(),
      };
      renote.insert();

      if (note.local) {
        User uRenotee = UserService::lookup_ap(note.owner).value();
        NotificationService::createRenote(note, renote, uRenotee, uRenoter);
      }

    } else if (type == "Like") {
      User favoriter = UserService::fetchRemote(body["actor"]);
      Note note = NoteService::fetchRemote(body["object"]);

      URL likeuri(body["id"]);
      Like l = {
        .uri = body["id"],
        .id = utils::genid(),
        .local = 0,
        .host = likeuri.host,

        .owner = body["actor"],
        .object = body["object"]
      };

      l.insert();

      if (note.local == true) {
        User favoritee = UserService::lookup_ap(note.owner).value();
        NotificationService::createFavorite(note, favoritee, favoriter);
      }

    } else if (type == "EmojiReact") {
      User reacter = UserService::fetchRemote(body["actor"]);
      Note note = NoteService::fetchRemote(body["object"]);

      URL reacturi(body["id"]);

      EmojiReact e = {
        .uri = body["id"],
        .id = utils::genid(),
        .local = 0,
        .host = reacturi.host,

        .owner = body["actor"],
        .object = body["object"]
      };

      if (body["tag"].is_array() && body["tag"][0].is_object()) {
        Emoji em = EmojiService::parse(body["tag"][0], reacturi.host);
        e.emojiText = nullopt;
        e.emojiId = em.id;
      } else {
        e.emojiId = nullopt;
        e.emojiText = body["content"];
      }

      e.insert();
      // std::cout << body.dump() << "\n";
    } else if (type == "Follow"){
      string uri = body["id"];
      FollowService::ingest(uri, body);
    } else {
      error("unimplemented activity {}", type);
      body["@context"] = nullptr;
      std::cout << body.dump() << "\n";
    }
  }

  void QueueIngest(json activity) {
    std::thread t([activity](){
      CPPTRACE_TRY {
        Ingest(activity);    
      } CPPTRACE_CATCH(const InvalidActivityError &e) { 
        trace(e.what());
      } catch(const std::exception &e) {
        error("ingest {} failed: {}", (string)activity["type"], e.what());
        utils::getStackTrace();
      }
    });
    t.detach();
  }
}
