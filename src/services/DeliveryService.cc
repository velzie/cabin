#include "utils.h"
#include "http.h"

#include "services/DeliveryService.h"

namespace DeliveryService {

  void Deliver(string instance, json activity) {
    URL url(instance);

    User ia = INSTANCEACTOR;
    APClient cli(ia, url.host);

    auto resp = cli.Post("/inbox", activity);
    error("request: {} {}", resp->status, resp->body);
  }

  void QueueDelivery(json activity, Audience audience) {
    std::vector<string> inboxes;

    if (audience.actor.has_value() && audience.followers) {
      auto q = STATEMENT("SELECT * FROM follow WHERE followee = ? AND pending = 0");
      q.bind(1, audience.actor->uri);

      while (q.executeStep()) {
        string follower = q.getColumn("follower");
        auto uFollower = User::lookupuri(follower);
        inboxes.push_back(uFollower->inbox);
      }
    }

    // add extras (mentions)
    for (const string instance : audience.mentions) {
      if (std::find(inboxes.begin(), inboxes.end(), instance) == inboxes.end()) {
        inboxes.push_back(instance);
      }
    }

    if (audience.forcefederate) {
      // todo
    }

    // add json-ld context 
    activity["@context"] = context;

    // if (audience.showcc) {
    //   std::vector<string> to;
    //   if (audience.aspublic) {
    //     to.push_back("https://www.w3.org/ns/activitystreams#Public");
    //   } else {
    //     if (audience.followers) {
    //       
    //     }
    //
    //   }
    //   activity["to"] = to;
    // }

    for (const string instance : inboxes) {
      std::thread t([activity, audience, instance](){
        CPPTRACE_TRY {
          trace("delivering to {}", instance);
          Deliver(instance, activity);    
        } CPPTRACE_CATCH(const std::exception &e) {
          error("delivery {} to {} failed: {}", (string)activity["type"], instance, e.what());
          utils::getStackTrace();
        }
      });
      t.detach();
    }
  }
}
