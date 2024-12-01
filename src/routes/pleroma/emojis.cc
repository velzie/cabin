#include <router.h>
#include <common.h>

GET(pleroma_emojis, "/api/v1/pleroma/emoji") {
  OK(json::object(), MIMEJSON);
}

GET(pleroma_stickers, "/static/stickers.json") {
  res->endWithoutBody();
}
