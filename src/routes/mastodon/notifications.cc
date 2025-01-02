#include "mshelper.h"
#include "router.h"
#include "querybuilder.h"

GET(notifications, "/api/v1/notifications") {
  MSAUTH
  
  QueryBuilder query;
  query = query
    .select({"*"})
    .from("notification")
    .where(EQ("notifieeId", authuser.id))
    .orderBy("createdAt", "DESC");


  PAGINATE(query, Notification, createdAt);
}

GET(notification_policy, "/api//v1/notifications/policy") {
  MSAUTH

  json j = {
    {"for_not_following", "accept"},
    {"for_not_followers", "accept"},
    {"for_new_accounts", "accept"},
    {"for_private_mentions", "accept"},
    {"for_limited_accounts", "accept"},
    {"summary", {
      {"pending_requests_count", 0},
      {"pending_notifications_count", 0},
    }},
  };

  OK(j, MIMEJSON);
}

GET(announcements, "/api/v1/announcements") {
  OK(json::array(), MIMEJSON);
}

POST(pleroma_notifications_read, "/api/v1/pleroma/notifications/read") {
  MSAUTH


  if (!mp.isValid()) ERROR(400, "invalid multipart");
  std::pair<std::string_view, std::string_view> headers[20];

  string maxId;
  while (true) {
    std::optional<std::string_view> optionalPart = mp.getNextPart(headers);
    if (!optionalPart.has_value()) {
      break;
    }

    for (int i = 0; headers[i].first.length(); i++) {
      if (headers[i].first == "content-disposition") {
        uWS::ParameterParser pp(headers[i].second);
        while (true) {
          auto [key, value] = pp.getKeyValue();
          if (key.empty()) break;
          if (key == "name") {
            if (value == "max_id") {
              maxId = optionalPart.value();
            }
          }
        }
      }
    }
  }
  if (maxId.empty()) ERROR(400, "max_id is required");

  QueryBuilder qb;
  qb.update()
    .from("notification")
    .set("isread", true)
    .where(LTE("id", maxId))
    .where(EQ("notifieeId", authuser.id))
    .build()
    .exec();


  qb = qb
    .select()
    .from("notification")
    .where(EQ("notifieeId", authuser.id))
    .orderBy("createdAt", "DESC");


  PAGINATE(qb, Notification, createdAt);
}

GET(timeline_markers, "/api/v1/markers") {
  MSAUTH

  auto response = json::object();
  string timeline (req->getQuery("timeline[]"));

  if (timeline == "notifications") {
    QueryBuilder query;
    auto s = query
      .select({"id"})
      .from("notification")
      .where(EQ("notifieeId", authuser.id))
      .where(EQ("isread", true))
      .orderBy("createdAt", "DESC")
      .limit(1)
      .build();
    ASSERT(s.executeStep());
    string last_read_id = s.getColumn("id");

    response["notifications"] = {
      {"last_read_id", last_read_id},
      {"version", 11},
      {"updated_at", "2025-01-01T17:06:00.913Z"}
    };
  }

  OK(response, MIMEJSON);
}
