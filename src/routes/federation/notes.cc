#include "common.h"
#include "router.h"

std::string dateUTC() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm utc_tm = *std::gmtime(&now_c);
    std::ostringstream oss;
    oss << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

GET(notes, "/notes/:id") {
  std::string id = "0";
  std::string idurl = API("notes/"+id);
  json j = {
    {"@context", ct->context},
    {"id", idurl},
    {"type", "Note"},
    {"summary", nullptr},
    {"inReplyTo", nullptr},
    {"published", dateUTC()},
    {"url", idurl},
    {"attributedTo", USERPAGE(ct->userid)},
    {"to", {"https://www.w3.org/ns/activitystreams#Public"}},
    {"cc", {USERPAGE(ct->userid)+"/followers"}},
    {"sensitive", false},
    {"content", "i cant think of anything funny to put here sorry"},
    {"attachment", {}},
    {"tag", {}}
  };

  res.set_content(j.dump(), "application/activity+json");
}
