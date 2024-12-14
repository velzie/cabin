#include "services/BiteService.h"
#include "services/DeliveryService.h"
#include "services/FetchService.h"
#include "services/NotificationService.h"
#include "utils.h"
#include <variant>

namespace BiteService {
  Bite create(User &user, User &biteee) {
    Bite b;
    string id = utils::genid();
    b.uri = API("bite/"+id);
    b.id = id;
    b.local = true;
    b.host = cfg.domain;

    b.bitUser = biteee.uri;

    DeliveryService::Audience au = {
      .actor = {user},
      .mentions = {biteee.uri},
      .aspublic = true,
      .followers = true,
    };
    DeliveryService::QueueDelivery(b.renderAP(), au);

    b.insert();
    return b;
  }

  Bite ingest(const json data) {
    User biter = FetchService::fetch<User>(data["actor"]);
    AnyEntity target = FetchService::fetch(data["object"]);

    URL biteuri(data["id"]);
    Bite b;
    b.uri = data["id"];
    b.id = utils::genid();
    b.local = 0;
    b.host = biteuri.host;

    b.owner = data["actor"];

    if (std::holds_alternative<Note>(target)) {
      auto note = get<Note>(target);
      b.bitNote = note.uri;

      if (note.local == true) {
        User biteee = User::lookupuri(note.owner).value();
        NotificationService::createBite(note, biteee, biter);
      }
    } else if (std::holds_alternative<User>(target)) { 
      auto biteee = get<User>(target);
      b.bitUser = biteee.uri;

      if (biteee.local == true) {
        NotificationService::createBite(nullopt, biteee, biter);
      }
    } else {
      ASSERT(0);
    }

    b.insert();

    return b;
  }
}
