#define USE_DB
#include "ingest.h"

#include "user.h"
#include "note.h"
#include "../utils.h"
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
      UserService::fetchRemote(object["actor"]);
      NoteService::fetchRemote(object["object"]);

      URL likeuri(object["id"]);
      Like l = {
        .uri = object["id"],
        .id = utils::genid(),
        .local = 0,
        .host = likeuri.host,

        .owner = object["actor"],
        .object = object["object"]
      };

      l.insert();
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
          auto t = cpptrace::from_current_exception();

          t.frames.erase(std::remove_if(t.frames.begin(), t.frames.end(), [](auto f) {
            return f.filename.find(".third-party") != std::string::npos || f.filename.find("/usr") != std::string::npos;
          }), t.frames.end());

          t.print_with_snippets();
      }
    });
    t.detach();
  }
}
