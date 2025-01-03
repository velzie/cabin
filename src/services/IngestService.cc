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
