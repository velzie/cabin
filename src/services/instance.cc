
#include "entities/Instance.h"
#include "services/user.h"

#include "http.h"
#include "utils.h"

#include <httplib.h>
namespace InstanceService {
  optional<Instance> lookup(const string host) {
    auto q = STATEMENT("SELECT * FROM instance WHERE host = ?");
    q.bind(1, host);
    if (!q.executeStep()) {
      return nullopt;
    }
    Instance i;
    i.load(q);
    return i;
  }

  Instance fetchRemote(const string host) {
    auto cached = lookup(host);
    if (cached.has_value()) {
      return cached.value();
    }

    auto ia = UserService::lookup(cfg.instanceactor);
    httplib::Client cli("https://"+host);
    cli.set_follow_location(true);

    auto nodeinfoMetaReq = cli.Get("/.well-known/nodeinfo");
    ASSERT(nodeinfoMetaReq);
    ASSERT(nodeinfoMetaReq->status == 200);
    json nodeinfoMeta = json::parse(nodeinfoMetaReq->body);


    string nodeinfouri;
    for (const auto link : (vector<json>)nodeinfoMeta["links"]) {
      // TODO: get 2.0
      nodeinfouri = link["href"];
    }

    URL nodeinfourl(nodeinfouri);
    ASSERT(nodeinfourl.host == host);

    auto nodeinfoReq = cli.Get(nodeinfourl.path);
    ASSERT(nodeinfoReq);
    ASSERT(nodeinfoReq->status == 200);
    json nodeinfo = json::parse(nodeinfoReq->body);

    Instance i;
    i.host = host;
    if (nodeinfo["metadata"].contains("nodeName"))
      i.name = nodeinfo["metadata"]["nodeName"];
    if (nodeinfo["metadata"].contains("nodeDescription"))
      i.description = nodeinfo["metadata"]["nodeDescription"];
    i.lastUpdatedAt = utils::millis();

    INSERT_OR_UPDATE(i, host, id, utils::genid());

    return i;
  }
}
