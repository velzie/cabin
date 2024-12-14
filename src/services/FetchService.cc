#include "services/FetchService.h"
#include "services/BiteService.h"
#include "entities/User.h"
#include "entities/Bite.h"
#include "http.h"
#include <error.h>

namespace FetchService {
  AnyEntity fetch(const string uri) {

    auto u = User::lookupuri(uri);
    if (u.has_value()) return u.value();
    auto n = Note::lookupuri(uri);
    if (n.has_value()) return n.value();
    auto b = Bite::lookupuri(uri);
    if (b.has_value()) return b.value();

    URL url(uri);

    auto ia = INSTANCEACTOR;
    APClient cli(ia, url.host);

    auto response = cli.Get(url.path);

    json object = json::parse(response->body);

    // TODO: make sure it's not serving us somethinf from a different instance

    if (object["type"] == "Note") {
      return Note::ingest(object);
    } else if (object["type"] == "Person") {
      return User::ingest(object);
    } else if (object["type"] == "Bite") { 
      return BiteService::ingest(object);
    } else {
      throw InvalidActivityError(FMT("unknown entity type {}", (string)object["type"]));
    }
  }
}
