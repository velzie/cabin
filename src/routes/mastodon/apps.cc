#include "Multipart.h"
#include "entities/Emoji.h"
#include "entities/OauthToken.h"
#include "querybuilder.h"
#include "entities/Notification.h"
#include "entities/UserSettings.h"
#include <router.h>
#include <common.h>
#include <QueryParser.h>
#include "mshelper.h"

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

  res->writeHeader("Content-Type", "text/html");
  res->end(R"(
    <form method="post">
      <div>
          <input type="checkbox" name="isPleroma" id="isPleroma" value="1">
          <label for="isPleroma">This client supports the Pleroma API</label>
      </div>
      <input type="text" placeholder="Username" name="username" required>
      <input type="password" placeholder="password" name="password" required>
      <button type="submit">submit</button>
    </form>
  )");
}

std::map<string, OauthToken> tokens;
POST(authorize_post, "/oauth/authorize") {
  ASSERT(req->getQuery("response_type") == "code");

  string client_id(req->getQuery("client_id"));
  string redirect_uri (req->getQuery("redirect_uri"));

  string scope(req->getQuery("scope"));
  if (scope.empty()) scope = "read";

  string query("?" + bodyraw);
  string username(uWS::getDecodedQueryValue("username", query));
  string password(uWS::getDecodedQueryValue("password", query));
  bool isPleroma = uWS::getDecodedQueryValue("isPleroma", query) == "1";

  QueryBuilder qb;
  auto user = qb.select().from("user").where(EQ("local", true)).where(EQ("username", username)).getOne<User>();
  if (!user.has_value()) ERROR(401, "invalid username or password");
  auto settings = UserSettings::lookupuserId(user->id).value();
  if (settings.password != password) ERROR(401, "invalid username or password");

  OauthToken oauth;
  oauth.id = utils::genid();
  oauth.userId = user->id;

  oauth.scopes = {};
  while (!scope.empty()) {
    size_t pos = scope.find(" ");
    if (pos == string::npos) {
      oauth.scopes.push_back(scope);
      break;
    }
    oauth.scopes.push_back(scope.substr(0, pos));
    scope = scope.substr(pos + 1);
  }
  oauth.isPleroma = isPleroma;
  oauth.clientId = client_id;
  oauth.token = utils::genid(); // FIXME: ...
  oauth.insert();

  string code = utils::genid();
  tokens[code] = oauth;

  REDIRECT(redirect_uri + "?code=" + code);
}

POST(token, "/oauth/token") {
  string code;
  string scopes;


  if (req->getHeader("content-type") == "application/json") {
    code = body.at("code");
    scopes = body.at("scope");
  } else if (mp.isValid()) {
    std::pair<std::string_view, std::string_view> headers[20];
    while (true) {
      optional<std::string_view> optionalPart = mp.getNextPart(headers);
      if (!optionalPart.has_value()) break;

      for (int i = 0; headers[i].first.length(); i++) {
        if (headers[i].first == "content-disposition") {
          uWS::ParameterParser pp(headers[i].second);
          while (true) {
            auto [key, value] = pp.getKeyValue();
            if (key.empty()) break;
            if (key == "name") {
              if (value == "code") {
                code = optionalPart.value();
              } else if (value == "scope") {
                scopes = optionalPart.value();
              }
            }
          }
        }
      }
    }
  } else {
    string query("?" + bodyraw);
    code = uWS::getDecodedQueryValue("code", query);
    scopes = uWS::getDecodedQueryValue("scope", query);
  }


  std::time_t current_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());


  if (code.empty()) ERROR(400, "missing code");
  if (!tokens.count(code)) ERROR(400, "invalid code");
  auto token = tokens[code];
  tokens.erase(code);

  json r = {
    {"access_token", token.token},
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
    {"version", "2.7.2 (compatible; Pleroma 2.4.50+akkoma)"},
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
          "bubble_timeline"
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
