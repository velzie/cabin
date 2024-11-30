#define USE_DB
#include <common.h>
#include "ingest.h"
#include "follow.h"
#include "user.h"
#include "note.h"

#include "../utils.h"
#include "../entities/Like.h"
#include <cpptrace/from_current.hpp>

namespace IngestService {


  void Ingest(json body) {
    string type = body["type"];
    info("starting ingest of {}", type);

    body["@context"] = "";
    std::cout << body.dump() << "\n";


    auto object = body["object"];
    if (type == "Create") {
      if (object["type"] != "Note") return;
      URL noteuri(object["id"]);
      // todo verify id is right
      
      NoteService::ingest(object["id"], object);
    } else if (type == "Announce") {
      string object = body["object"];
      Note note = NoteService::fetchRemote(object);
    } else if (type == "Like") {
      UserService::fetchRemote(body["actor"]);
      NoteService::fetchRemote(body["object"]);

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
    } else if (type == "Follow"){
      string uri = body["id"];
      FollowService::ingest(uri, body);
    } else {
      error("unimplemented activity {}", type);
    }
  }

  void QueueIngest(json activity) {
    std::thread t([activity](){
      CPPTRACE_TRY {
        Ingest(activity);    
      } CPPTRACE_CATCH(const std::exception &e) {
        error("ingest {} failed: {}", (string)activity["type"], e.what());
        utils::getStackTrace();
      }
    });
    t.detach();
  }
}
