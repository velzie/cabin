#include "router.h"
#include "common.h"

json NodeMeta(std::string version) {
  return {
    {"version", "2.0"},
    {"software", {
      {"name", SOFTWARE},
      {"version", VERSION_LONG},
      {"codename", "gyat"},
      {"edition", "enterprise"},
      {"homepage", HOMEPAGE},
    }},
    {"protocols", {
      "activitypub",
    }},
    {"services", {
      {"inbound", {}},
      {"outbound", {
        "atom1.0",
        "rss2.0",
      }},
    }},
    {"usage", {
      {"users", {
        {"total", 1},
        {"activeHalfyear", 1},
        {"activeMonth", 1},
      }},
      {"localPosts", 6514},
      {"localComments", 0},
    }},
    {"metadata", {
      {"nodeName", "cabin"},
      {"nodeDescription", "A cabin instance"},
      {"maintainer", {
        {"name", "todo"},
        {"email", "todo"},
      }},
      {"langs", ARR},
      {"tosUrl", "todo"},
      {"repositoryUrl", "https://iceshrimp.dev/iceshrimp/Iceshrimp.NET"},
      {"feedbackUrl", "https://issues.iceshrimp.dev"},
      {"themeColor", "#000000"},
      {"disableRegistration", true},
      {"disableLocalTimeline", false},
      {"disableRecommendedTimeline", false},
      {"disableGlobalTimeline", false},
      {"emailRequiredForSignup", false},
      {"postEditing", false},
      {"postImports", false},
      {"enableHcaptcha", false},
      {"enableRecaptcha", false},
      {"maxNoteTextLength", 0},
      {"maxCaptionTextLength", 0},
      {"enableGithubIntegration", false},
      {"enableDiscordIntegration", false},
      {"enableEmail", false},
      {"post_formats", {
        "text/plain",
        "text/x.misskeymarkdown",
      }},
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
      {"staffAccounts", ARR},
      {"publicTimelineVisibility", {
        {"bubble", false},
        {"federated", false},
        {"local", false},
      }},
      {"uploadLimits", {
        {"general", 104857600},
        {"avatar", 104857600},
        {"background", 104857600},
        {"banner", 104857600},
      }},
      {"suggestions", {
        {"enabled", false},
      }},
      {"federation", {
        {"enabled", true},
      }},
    }},
    {"openRegistrations", false},
    {"operations", {
      {"com.shinolabs.api.bite", {
        "1.0.0",
      }},
      {"jetzt.mia.ns.activitypub.accept.bite", {
        "1.0.0",
      }},
    }},
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

