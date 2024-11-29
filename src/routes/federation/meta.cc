#include "router.h"
#include "common.h"

json NodeMeta(std::string version) {
  return {
      {"version", version},
      {"software", {
        {"name", SOFTWARE},
        {"version", VERSION_LONG},
        {"homepage", HOMEPAGE}
      }},
      {"protocols", {"activitypub"}},
      {"services", {
        {"inbound", {}},
        {"outbound", {}}
      }},
      {"openRegistrations", false},
      {"usage", {
        {"users", {
          {"total", 1}, // ;)
          {"activeHalfyear", nullptr},
          {"activeMonth", nullptr},
        }},
        {"localPosts", 0},
        {"localComments", 0}
      }},
      {"metadata", {
        {"nodeName", "land of rizz"},
        {"nodeDescription", "placeholder"},
        {"nodeAdmins", {}},
        {"maintainer", {
          {"name", "..."},
          {"email", "..."},
        }},
        {"langs", {}},
        {"tosUrl", {}},
        {"privacyPolicyUrl", {}},
          // "inquiryUrl": "https://synth.download",
          //    "impressumUrl": "",
          //    "donationUrl": "https://ko-fi.com/sneexy",
          //    "repositoryUrl": "https://activitypub.software/TransFem-org/Sharkey/",
          //    "feedbackUrl": "https://activitypub.software/TransFem-org/Sharkey/-/issues/new",
        {"features", {
          "pleroma_api",
          "akkoma_api",
          "mastodon_api",
          "mastodon_api_streaming",
          "polls",
          "quote_posting",
          "editing",
          "pleroma_emoji_reactions",
          "exposable_reactions",
          "custom_emoji_reactions",
          "pleroma:bites",
        }},
        {"localBubbleInstances", ARR},
        {"disableRegistration", true},
        {"disableLocalTimeline", true},
        {"disableGlobalTimeline", true},
        {"emailRequiredForSignup", false},
        {"maxNoteTextLength", 100000},
        {"maxRemoteNoteTextLength", 100000},
        {"maxCwLength", 5000},
        {"maxRemoteCwLength", 5000},
        {"maxAltTextLength", 100000},
        {"maxRemoteAltTextLength", 100000},
        {"enableEmail", false},
        {"themeColor", "#f9e2af"}
      }}
    };
}



GET(hostmeta, "/.well-known/host-meta") {
  res->writeStatus("200");
  res->writeHeader("Content-Type", "application/xml");

  res->write(FMT(R"(
    <XRD xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns="http://docs.oasis-open.org/ns/xri/xrd-1.0">
      <Link rel="lrdd" type="application/jrd+json" template="{}" />
    </XRD>
    )", API(".well-known/webfinger?resource={uri}")));

  res->end();
}

GET(nodeinfo, "/.well-known/nodeinfo") {
  json j = {
    {"links", {
      {
        {"rel", "http://nodeinfo.diaspora.software/ns/schema/2.1"},
        {"href", API("nodeinfo/2.1")}
      },
      {
        {"rel", "http://nodeinfo.diaspora.software/ns/schema/2.0"},
        {"href", API("nodeinfo/2.0")}
      }
    }}
  };

  OK(j, MIMEJSON);
}


GET(diaspora_nodeinfo, "/nodeinfo/:version") {
  string version (req->getParameter("version"));

  if (version.find(".json") != string::npos) {
    version.erase(version.size() - 5, 5);

    REDIRECT(API(FMT("nodeinfo/{}", version)));
  }


  OK(
      NodeMeta("2.0"),
      FMT("application/json; profile=\"http://nodeinfo.diaspora.software/ns/schema/{}#\"; charset=utf-8", version)
  );
}

