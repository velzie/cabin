#include "SQLiteCpp/Statement.h"
#include "entities/Note.h"
#include <type_traits>
#define USE_DB
#include "router.h"
#include "common.h"

#include "utils.h"
GET(notes, "/notes/:id") {
  std::string id(req->getParameter("id"));
  std::string idurl = API("notes/"+id);



  auto note = Note::lookupid(id);

  if (!note.has_value()) {
    ERROR(404, "no such note");
  }
  
  auto r = note->renderAP();
  r["@context"] = context;

  OK(r, MIMEAP);
}
