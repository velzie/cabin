#include "services/FetchService.h"
#include "entities/User.h"
#include "http.h"
#include <error.h>

namespace FetchService {
  AnyEntity fetch(const string uri) {
    URL url(uri);

    auto ia = INSTANCEACTOR;
    APClient cli(ia, url.host);

    auto response = cli.Get(url.path);

    json object = json::parse(response->body);

    // TODO: make sure it's not serving us somethinf from a different instance

    if (object["type"] == "Note") {
      return Note::ingest(object);
    } else if (object["type"] == "User") {
      return User::ingest(object);
    } else {
      throw InvalidActivityError(FMT("unknown entity type {}", (string)object["type"]));
    }
  }
}
