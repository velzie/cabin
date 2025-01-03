#include "common.h"
#include "entities/User.h"
#include "services/NoteService.h"
#include "services/NotificationService.h"
#include "services/FetchService.h"
#include "services/FollowService.h"
#include "services/DeliveryService.h"
#include "utils.h"
#include <stdexcept>
#include "querybuilder.h"


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

  void undo(User &user, Follow &follow) {
    QueryBuilder qb;

    auto s = qb
      .deleteFrom("follow")
      .where(EQ("id", follow.id))
      .build();
    ASSERT(s.exec());

    json activity = {
      {"id", API("undos/"+utils::genid())},
      {"type", "Undo"},
      {"actor", user.uri},
      {"object", follow.renderAP()},
    };

    DeliveryService::Audience au = {
      .actor = user,
      .mentions = {follow.followee},
    };
    DeliveryService::QueueDelivery(activity, au);
  }

  Follow ingest(const string uri, const json follow) {
    string followee = follow.at("object");
    string follower = follow.at("actor");

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
      .local = uFollowee.value().local,
      .host = url.host, 

      .follower = follower,
      .followee = followee,
      .pending = false, // TODO: this should only update after delivery succeeds

      .createdAt = utils::millis(),
    };

    INSERT_OR_UPDATE(f, uri, id, utils::genid());

    if (isNew && f.local) {

      NotificationService::createFollow(uFollowee.value(), uFollower);

      json accept = {
        {"actor", followee},
        {"id", API("accepts/"+utils::genid())},
        {"object", f.uri},
        {"type", "Accept"}
      };

      DeliveryService::Audience au = {
        .actor = uFollowee,
        .mentions = {follower},
      };
      DeliveryService::QueueDelivery(accept, au);
    }

    return f;
  }
  // {"@context":null,"actor":"https://booping.synth.download/users/a005c9wl4pw
  // j0arp","id":"https://booping.synth.download/49dd80f3-d717-4dea-819d-60af0f
  // 3c4787","object":{"actor":"https://staging.velzie.rip/users/test","id":"ht
  // tps://booping.synth.download/follows/a293x1zkk1280gdn/a005c9wl4pwj0arp","o
  // bject":"https://booping.synth.download/users/a005c9wl4pwj0arp","type":"Fol
  // low"},"type":"Reject"}

  void ingestReject(const string id, const json activity) {
    optional<Follow> follow;
    ASSERT(activity.contains("object"));
    if (activity["object"].is_string()) {
      follow = Follow::lookupuri(activity["object"]);
    } else {
      follow = Follow::lookupuri(activity["object"].at("id"));
    }
    ASSERT(follow.has_value());

    QueryBuilder qb;
    auto s = qb
      .deleteFrom("follow")
      .where(EQ("id", follow->id))
      .build();
    ASSERT(s.exec());

    User u = User::lookupuri(follow->follower).value();
    NotificationService::createMisc(u, FMT("Notification(follow_request): Your follow request to {} was rejected", follow->followee));
  }

  // "{"@context":"http
  // s://www.w3.org/ns/activitystreams","actor":"https://mastodon.social/users/daletr
  // aita","id":"https://mastodon.social/users/daletraita#accepts/follows/48010540","
  // object":{"actor":"https://staging.velzie.rip/users/test","id":"https://staging.v
  // elzie.rip/follows/18174fa485f00da1uLILa0Lz","object":"https://mastodon.social/us
  // ers/daletraita","type":"Follow"},"type":"Accept"}
  void ingestAccept(const string id, const json activity) {
    optional<Follow> follow;
    ASSERT(activity.contains("object"));
    if (activity["object"].is_string()) {
      follow = Follow::lookupuri(activity["object"]);
    } else {
      follow = Follow::lookupuri(activity["object"].at("id"));
    }
    ASSERT(follow.has_value());

    if (!follow.value().pending) {
      return; // already accepted
    }

    User u = User::lookupuri(follow->follower).value();
    follow.value().pending = false;
    follow.value().update();

    NotificationService::createMisc(u, FMT("Notification(follow_request): You are now following {}", follow->followee));
  }
}
