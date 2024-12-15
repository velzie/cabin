#include "entities/Emoji.h"
#include "entities/Notification.h"
#include <router.h>
#include <common.h>
#include <QueryParser.h>


std::string join(const std::vector<std::string>& vec, const std::string& delimiter) {
    if (vec.empty()) return "";
    return std::accumulate(std::next(vec.begin()), vec.end(), vec[0], 
    [&delimiter](const std::string& a, const std::string& b) {
        return a + delimiter + b;
    });
}


json stored_app;
POST(apps, "/api/v1/apps") {
  string client_name;
  string website;
  string scopes;
  string redirect_uris;

  if (req->getHeader("content-type") == "application/json") {
    client_name = body["client_name"];
    redirect_uris = body["redirect_uris"];
    scopes = body["scopes"];
    website = body["website"];
  } else {
    string query("?" + bodyraw);
    client_name = (uWS::getDecodedQueryValue("client_name", query));
    website = (uWS::getDecodedQueryValue("website", query));
    scopes = (uWS::getDecodedQueryValue("scopes", query));
    redirect_uris = req->getParameter("redirect_uris");
  }



  json r = {
    {"id", "0"},
    {"name", client_name},
    {"website", website},
    {"scopes", scopes},
    {"redirect_uri", redirect_uris},
    {"redirect_uris", redirect_uris},
    {"client_id", "placeholder"},
    {"client_secret", "placeholder_secret"}
  };
  stored_app = r;

  OK(r, MIMEJSON);
}

GET(authorize, "/oauth/authorize") {
  ASSERT(req->getQuery("response_type") == "code");


  string client_id(req->getQuery("client_id"));
  string redirect_uri (req->getQuery("redirect_uri"));

  string scope(req->getQuery("scope"));
  if (scope.empty()) scope = "read";

  json sanitized = redirect_uri;


  res->writeHeader("Content-Type", "text/html");
  res->end(FMT(R"(
    <button id="redirect">click to oauth</button>
    <script>
      let url = {}
      redirect.onclick = () => window.location = url + "?code=placeholder_code"
    </script>
  )", sanitized.dump()));
}

POST(token, "/oauth/token") {
  string query("?" + bodyraw);
  string scopes(uWS::getDecodedQueryValue("scopes", query));
  std::time_t current_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  json r = {
    {"access_token", "placeholder_token"},
    {"token_type", "Bearer"},
    {"scope", scopes},
    {"created_at", current_time},
  };

  OK(r, MIMEJSON);
}

GET(verify_credentials, "/api/v1/apps/verify_credentials") {
  OK(stored_app, MIMEJSON);
}

GET(instance, "/api/:v/instance") {
  json j = {
    {"domain", cfg.domain},
    {"title", "Cabin"},
    {"version", "4.0.0rc1 (compatible; Cabin 0.0.0)"},
    {"source_url", "https://github.com/mastodon/mastodon"},
    {"description", "The original server operated by the Mastodon gGmbH non-profit"},
    {"usage", {
      {"users", {
        {"active_month", 123122},
      }},
    }},
    {"thumbnail", {
      {"url", "https://files.mastodon.social/site_uploads/files/000/000/001/@1x/57c12f441d083cde.png"},
      {"blurhash", "UeKUpFxuo~R%0nW;WCnhF6RjaJt757oJodS$"},
      {"versions", {
        {"@1x", "https://files.mastodon.social/site_uploads/files/000/000/001/@1x/57c12f441d083cde.png"},
        {"@2x", "https://files.mastodon.social/site_uploads/files/000/000/001/@2x/57c12f441d083cde.png"},
      }},
    }},
    {"icon", {
      {
        {"src", "https://files.mastodon.social/site_uploads/files/000/000/003/36/accf17b0104f18e5.png"},
        {"size", "36x36"},
      },
      {
        {"src", "https://files.mastodon.social/site_uploads/files/000/000/003/72/accf17b0104f18e5.png"},
        {"size", "72x72"},
      },
      {
        {"src", "https://files.mastodon.social/site_uploads/files/000/000/003/192/accf17b0104f18e5.png"},
        {"size", "192x192"},
      },
      {
        {"src", "https://files.mastodon.social/site_uploads/files/000/000/003/512/accf17b0104f18e5.png"},
        {"size", "512x512"},
      },
    }},
    {"languages", {
      "en",
    }},
    {"configuration", {
      {"urls", {
        {"streaming", "wss://mastodon.social"},
      }},
      {"vuri", {
        {"public_key", "BCkMmVdKDnKYwzVCDC99Iuc9GvId-x7-kKtuHnLgfF98ENiZp_aj-UNthbCdI70DqN1zUVis-x0Wrot2sBagkMc="},
      }},
      {"accounts", {
        {"max_featured_tags", 10},
        {"max_pinned_statuses", 4},
      }},
      {"statuses", {
        {"max_characters", 500},
        {"max_media_attachments", 4},
        {"characters_reserved_per_url", 23},
      }},
      {"media_attachments", {
        {"supported_mime_types", {
          "image/jpeg",
          "image/png",
          "image/gif",
          "image/heic",
          "image/heif",
          "image/webp",
          "video/webm",
          "video/mp4",
          "video/quicktime",
          "video/ogg",
          "audio/wave",
          "audio/wav",
          "audio/x-wav",
          "audio/x-pn-wave",
          "audio/vnd.wave",
          "audio/ogg",
          "audio/vorbis",
          "audio/mpeg",
          "audio/mp3",
          "audio/webm",
          "audio/flac",
          "audio/aac",
          "audio/m4a",
          "audio/x-m4a",
          "audio/mp4",
          "audio/3gpp",
          "video/x-ms-asf",
        }},
        {"image_size_limit", 10485760},
        {"image_matrix_limit", 16777216},
        {"video_size_limit", 41943040},
        {"video_frame_rate_limit", 60},
        {"video_matrix_limit", 2304000},
      }},
      {"polls", {
        {"max_options", 4},
        {"max_characters_per_option", 50},
        {"min_expiration", 300},
        {"max_expiration", 2629746},
      }},
      {"translation", {
        {"enabled", false},
      }},
    }},
    {"pleroma", {
      {"vuri_public_key", "BBUPV8y0hPC4rixOhrnFIElqD4MZ57fng9sD36v3Keau3ta2ZkaWRHTBIDGhp4V04TEwJaXbbrCkjH_bpZxio0k"},
      {"metadata",{
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
        {"fields_limits", {
          {"max_fields", 2147483647},
          {"max_remote_fields", 2147483647},
          {"name_length", 2147483647},
          {"value_length", 2147483647},
        }},
      }
    }}},
    {"registrations", {
      {"enabled", false},
      {"approval_required", false},
      {"message", nullptr},
    }},
    {"contact", {
      {"email", "staff@mastodon.social"},
      {"account", {
        {"id", "1"},
        {"username", "Gargron"},
        {"acct", "Gargron"},
        {"display_name", "Eugen 💀"},
        {"locked", false},
        {"bot", false},
        {"discoverable", true},
        {"group", false},
        {"created_at", "2016-03-16T00:00:00.000Z"},
        {"note", "<p>Founder, CEO and lead developer <span class=\"h-card\"><a href=\"https://mastodon.social/@Mastodon\" class=\"u-url mention\">@<span>Mastodon</span></a></span>, Germany.</p>"},
        {"url", "https://mastodon.social/@Gargron"},
        {"avatar", "https://files.mastodon.social/accounts/avatars/000/000/001/original/dc4286ceb8fab734.jpg"},
        {"avatar_static", "https://files.mastodon.social/accounts/avatars/000/000/001/original/dc4286ceb8fab734.jpg"},
        {"header", "https://files.mastodon.social/accounts/headers/000/000/001/original/3b91c9965d00888b.jpeg"},
        {"header_static", "https://files.mastodon.social/accounts/headers/000/000/001/original/3b91c9965d00888b.jpeg"},
        {"followers_count", 133026},
        {"following_count", 311},
        {"statuses_count", 72605},
        {"last_status_at", "2022-10-31"},
        {"noindex", false},
        {"emojis", json::array()},
        {"fields", {
          {
            {"name", "Patreon"},
            {"value", "<a href=\"https://www.patreon.com/mastodon\" target=\"_blank\" rel=\"nofollow noopener noreferrer me\"><span class=\"invisible\">https://www.</span><span class=\"\">patreon.com/mastodon</span><span class=\"invisible\"></span></a>"},
            {"verified_at", nullptr},
          },
        }},
      }},
    }},
    {"rules", {
      {
        {"id", "1"},
        {"text", "Sexually explicit or violent media must be marked as sensitive when posting"},
      },
      {
        {"id", "2"},
        {"text", "No racism, sexism, homophobia, transphobia, xenophobia, or casteism"},
      },
      {
        {"id", "3"},
        {"text", "No incitement of violence or promotion of violent ideologies"},
      },
      {
        {"id", "4"},
        {"text", "No harassment, dogpiling or doxxing of other users"},
      },
      {
        {"id", "5"},
        {"text", "No content illegal in Germany"},
      },
      {
        {"id", "7"},
        {"text", "Do not share intentionally false or misleading information"},
      },
    }},
  };

  OK(j, MIMEJSON);
}

GET(mastodon_custom_emojis, "/api/v1/custom_emojis") {
  json emojis = json::array();

  auto q = STATEMENT("SELECT * FROM emoji");
  while (q.executeStep()) {
    Emoji e;
    e.load(q);
    emojis.push_back({
        {"shortcode", e.address},
        {"url", e.imageurl},
        {"static_url", e.imageurl},
        {"visible_in_picker", true},
        {"category", e.host},
    });
  }

  OK(emojis, MIMEJSON);
}


GET(notifications, "/api/v1/notifications") {
  MSAUTH

  auto notifications = ARR;

  auto q = STATEMENT("SELECT * FROM notification WHERE notifieeId = ?");
  q.bind(1, authuser.id);
  
  while (q.executeStep()) {
    Notification n;
    n.load(q);
    notifications.push_back(n.renderMS(authuser));
  }

  OK(notifications, MIMEJSON);
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
