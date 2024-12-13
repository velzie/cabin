#include "database.h"
#include "entities/Emoji.h"
#include <router.h>
#include <common.h>

GET(pleroma_emojis, "/api/v1/pleroma/emoji") {
  json emojis = json::object();

  auto q = STATEMENT("SELECT * FROM emoji");
  while (q.executeStep()) {
    Emoji e;
    e.load(q);
    emojis[e.shortcode + "@" + e.host] = {
        {"tags", {FMT("pack:{}", e.host)}},
        {"image_url", "/302?url="+e.imageurl}
    };
  }

  OK(emojis, MIMEJSON);
}

// jank but whatever
GET(redirect302, "/302") {
  string url(req->getQuery("url"));

  res->writeStatus("302");
  res->writeHeader("Location", url);
  res->endWithoutBody();
}

GET(pleroma_stickers, "/static/stickers.json") {
  res->endWithoutBody();
}
