#include "entities/EmojiReact.h"
#include "entities/Note.h"
#include "services/BiteService.h"
#include "services/FetchService.h"
#include "services/NotificationService.h"
#include <error.h>
#define USE_DB
#include <common.h>
#include "utils.h"

#include "services/FollowService.h"
#include "services/NoteService.h"
#include "entities/Like.h"

#include <cpptrace/from_current.hpp>

namespace IngestService {


  void Ingest(json body) {
    string type = body["type"];
    trace("starting ingest of {}", type);

    // TODO: verify id is right

    if (type == "Create") {
      auto object = body["object"];
      if (object["type"] != "Note") return;
      Note::ingest(object);
    } else if (type == "Announce") {
      Note::ingestAnnounce(body);
    } else if (type == "Like") {
      Like::ingest(body);
    } else if (type == "Bite") {
      BiteService::ingest(body);
    } else if (type == "EmojiReact") {
      EmojiReact::ingest(body);
    } else if (type == "Follow") {
      FollowService::ingest(body["id"], body);
    } else if (type == "Reject") {
      FollowService::ingestReject(body["id"], body);
    } else if (type == "Accept") {
      FollowService::ingestAccept(body["id"], body);
    } else if (type == "Listen") {
     // {"@context":null,"actor":"https://pl.noob.quest/users/iris","cc":[],"context":"https://pl.noob.quest/contexts/872f7b3d-81b2-47c5-9d0d-09f50b616f15","directMessage":false,"id":"https://pl.noob.quest/activities/b1085785-00c2-4170-b0d0-e63c71f0762c","object":{"actor":"https://pl.noob.quest/users/iris","album":"Flex Musix","artist":"OsamaSon","attachment":[],"attributedTo":"https://pl.noob.quest/users/iris","cc":["https://pl.noob.quest/users/iris/followers"],"context":"https://pl.noob.quest/contexts/872f7b3d-81b2-47c5-9d0d-09f50b616f15","conversation":"https://pl.noob.quest/contexts/872f7b3d-81b2-47c5-9d0d-09f50b616f15","id":"https://pl.noob.quest/objects/8e2185fa-5019-4acb-8075-f03abdd283e2","length":144601,"published":"2025-01-04T01:17:37.997368Z","tag":[],"title":"Blonde","to":["https://www.w3.org/ns/activitystreams#Public"],"type":"Audio"},"published":"2025-01-04T01:17:37.997312Z","to":["https://www.w3.org/ns/activitystreams#Public"],"type":"Listen"} 
      auto user = User::lookupuri(body.at("actor"));
      ASSERT(body.contains("object") && body["object"].at("type") == "Audio");
      string text = FMT("scrobbled {} - {} {}", body["object"].at("artist").get<string>(), body["object"].at("album").get<string>(), body["object"].at("title").get<string>());
      NoteService::create(user.value(), text, nullopt, nullopt, nullopt, false, NOTEVISIBILITY_Followers, {});
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
