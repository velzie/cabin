#include "entities/Media.h"
#include "utils.h"

json Media::renderAP() {
  return {
    {"type", "Document"},
    {"mediaType", mimeType},
    {"url", url},
    {"name", description},
    {"blurhash", blurhash},
  };
}

json Media::renderMS(User &requester) {
  return {
    {"id", id},
    {"type", "image"},
    {"url", url},
    {"preview_url", url},
    {"remote_url", url},
    {"blurhash", blurhash},
    {"description", description}
  };
}

Media Media::ingest(const json attachment) {
  ASSERT(attachment["type"] == "Document");
  Media m;
  m.url = attachment["url"];

  if (attachment.contains("name") && attachment["name"].is_string()) {
    m.description = attachment["name"];
  }

  if (attachment.contains("blurhash") && attachment["blurhash"].is_string()) {
    m.blurhash = attachment["blurhash"];
  }

  m.sensitive = false;
  if (attachment.contains("sensitive") && attachment["sensitive"].is_boolean()) {
    m.sensitive = attachment["sensitive"];
  }

  m.id = utils::genid();
  m.local = false;
  m.mimeType = attachment["mediaType"];
  m.owner = nullopt;
  m.insert();

  return m;
}

Media Media::create(string mimeType) {
  Media m;
  m.id = utils::genid();
  m.url = API("media/" + m.id);
  m.mimeType = mimeType;
  m.local = true;
  m.createdAt = utils::millis();
  return m;
}
