#include "entities/User.h"
#include "services/NotificationService.h"
#include "services/FetchService.h"
#include "services/FollowService.h"
#include "services/DeliveryService.h"
#include "utils.h"
#include <stdexcept>


namespace FollowService {
  Follow create(User &user, string followee) {
    string id = utils::genid();
    Follow f = {
      .uri = FOLLOW(id),
      .id = id,
      .local = true,
      .host = user.host,

      .follower = user.uri,
      .followee = followee,

      .pending = true,
      .createdAt = utils::millis(),
    };
    f.insert();

    DeliveryService::Audience au = {
      .mentions = {followee}
    };

    DeliveryService::QueueDelivery(f.renderAP(), au);


    return f;
  }

  Follow ingest(const string uri, const json follow) {
    string followee = follow["object"];
    string follower = follow["actor"];

    auto uFollowee = User::lookupuri(followee);
    if (!uFollowee.has_value()) {
      throw std::runtime_error("follow request that doesn't exist..");
    }

    auto uFollower = FetchService::fetch<User>(follower);

    string id = utils::genid();
    URL url(uri);
    Follow f = {
      .uri = uri,
      .id = id,
      .local = false,
      .host = url.host, 

      .follower = follower,
      .followee = followee,
      .pending = false, // TODO: this should only update after delivery succeeds

      .createdAt = utils::millis(),
    };

    NotificationService::createFollow(uFollowee.value(), uFollower);

    json accept = {
      {"actor", followee},
      {"id", API("accepts/"+utils::genid())},
      {"object", f.renderAP()},
      {"type", "Accept"}
    };

    DeliveryService::Audience au = {
      .actor = uFollowee,
      .mentions = {follower},
    };
    DeliveryService::QueueDelivery(accept, au);


    f.insert();
    return f;
  }
}
