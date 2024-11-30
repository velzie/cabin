#include "follow.h"
#include "../utils.h"
#include "delivery.h"


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

    DeliveryService::Audience au = {
      .mentions = {followee}
    };

    DeliveryService::QueueDelivery(f.renderAP(), au);


    return f;
  }

  // Follow ingest(const string uri, const json follow) {
  //   string id = utils::genid();
  //   Follow f = {
  //     .uri = uri,
  //     .id = id,
  //     .local = false,
  //     .follower
  //   }
  // }
}
