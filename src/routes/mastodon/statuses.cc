#include "SQLiteCpp/Statement.h"
#include "entities/Emoji.h"
#include "entities/EmojiReact.h"
#include "entities/Note.h"
#include <router.h>
#include <common.h>
#include "database.h"
#include "querybuilder.h"
#include "utils.h"
#include "http.h"
#include "entities/Like.h"
#include "services/NoteService.h"
#include "mshelper.h"

POST(post_status, "/api/v1/statuses") {
  MSAUTH

  optional<Note> replyTo;
  optional<Note> quote;
  string status;

  bool isPreview = false;

  if (mp.isValid()) {
    // pleroma style
    
    std::pair<std::string_view, std::string_view> headers[20];
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
            if (!key.length()) {
                break;
            }
            if (key != "name") continue;
            auto data = optionalPart.value();


            if (value == "status") {
              status = data;
            } else if (value == "in_reply_to_id") {
              replyTo = Note::lookupid(string(data));
            } else if (value == "quote_id") {
              quote = Note::lookupid(string(data));
            } else if (value == "preview" && string(data) == "true") {
              isPreview = true;
            }
            
          }
        }
      }
    }
  } else {
    // mastodon style
    if (body["in_reply_to_id"].is_string()) {
      replyTo = Note::lookupid(body["in_reply_to_id"]);
    }
    status = body["status"];
  }

  
  Note note = NoteService::create(authuser, status, replyTo, quote, isPreview);

  OK(note.renderMS(authuser), MIMEJSON);
}

GET(status, "/api/v1/statuses/:id") {
  MSAUTH
  string id (req->getParameter("id"));

  auto n = Note::lookupid(id);
  if (!n.has_value()) {
    ERROR(404, "no note");
  }

  json j = n.value().renderMS(authuser);
  OK(j, MIMEJSON);
}

POST(status_like, "/api/v1/statuses/:id/favourite") {
  MSAUTH
  string id (req->getParameter("id"));

  auto note = Note::lookupid(id);
  if (!note.has_value()) {
    ERROR(404, "no note");
  }

  string likeid = utils::genid();
  Like like = {
    .uri = LIKE(likeid),
    .id = likeid,
    .local = true,
    .host = cfg.domain,

    .owner = authuser.uri,
    .object = note->uri
  };
  like.insert();

  URL url(note->uri);
  APClient cli(authuser, url.host);

  json activity = {
    {"actor", authuser.uri},
    {"id", like.uri},
    {"object", note->uri},
    {"content", "❤"},
    {"_misskey_reaction", "❤"},
    {"type", "Like"}
  };

  auto resp = cli.Post("/inbox", activity);

  note = Note::lookupid(id);
  OK(note->renderMS(authuser), MIMEJSON);
}

PUT(status_addreaction, "/api/v1/pleroma/statuses/:id/reactions/:emoji") {
  MSAUTH
  string id (req->getParameter("id"));
  string emojiname (req->getParameter("emoji"));
  emojiname = httplib::detail::decode_url(emojiname, false);
  ASSERT(emojiname.length() > 2);
  if (emojiname[0] == ':') {
    emojiname.erase(0, 1);
    emojiname.pop_back();
  }


  auto note = Note::lookupid(id);
  if (!note.has_value()) {
    ERROR(404, "no note");
  }

  string reactid = utils::genid();
  EmojiReact react = {
    .uri = REACT(reactid),
    .id = reactid,
    .local = true,
    .host = cfg.domain,

    .owner = authuser.uri,
    .object = note->uri
  };

  auto emoji = Emoji::lookupaddress(emojiname);
  if (emoji.has_value()) {
    react.emojiText = nullopt;
    react.emojiId = emoji->id;
  } else {
    react.emojiId = nullopt;
    react.emojiText = emojiname;
  }

  react.insert();

  URL url(note->uri);
  APClient cli(authuser, url.host);

  json activity = {
    {"actor", authuser.uri},
    {"id", react.uri},
    {"object", note->uri},
    {"type", "EmojiReact"}
  };

  if (emoji.has_value()) {
    activity["content"] = FMT(":{}:", emoji->address);
    activity["tag"] = {
      emoji->renderAPTag()
    };
  } else {
    activity["content"] = react.emojiText;
  }

  auto resp = cli.Post("/inbox", activity);

  note = Note::lookupid(id);
  OK(note->renderMS(authuser), MIMEJSON);
}

POST(status_renote, "/api/v1/statuses/:id/reblog") {
  MSAUTH

  string id(req->getParameter("id"));
  auto note = Note::lookupid(id);
  if (!note.has_value()) {
    ERROR(404, "no note");
  }

  Note n = NoteService::createRenote(authuser, note.value());

  OK(n.renderMS(authuser), MIMEJSON);
}

GET(status_context, "/api/v1/statuses/:id/context") {
  MSAUTH

  string id (req->getParameter("id"));
  auto originalnote = Note::lookupid(id);
  if (!originalnote.has_value()) {
    ERROR(404, "no note");
  }

  auto q = STATEMENT("SELECT * FROM note WHERE conversation = ?");
  q.bind(1, originalnote->conversation);

  std::vector<Note> bag;
  while (q.executeStep()) {
    Note note;
    note.load(q);
    if (note.id != id)
      bag.push_back(note);
  }


  json ancestors = json::array();
  json descendants = json::array();

  Note topmost = originalnote.value();
  while (topmost.replyToUri.has_value()) {
    auto s = std::find_if(bag.begin(), bag.end(), [topmost](Note n){ return n.uri == topmost.replyToUri.value(); });
    ASSERT(s != bag.end());
    topmost = *s;
    bag.erase(s);

    ancestors.push_back(topmost.renderMS(authuser));
  }

  for (Note n : bag) {
    descendants.push_back(n.renderMS(authuser));
  }





  json j = {
    {"ancestors", ancestors},
    {"descendants", descendants},
  };

  OK(j, MIMEJSON);
}

GET(status_favourited_by, "/api/v1/statuses/:id/favourited_by") {
  string id (req->getParameter("id"));
  auto n = Note::lookupid(id);
  if (!n.has_value()) {
    ERROR(404, "no note");
  }

  json accounts = json::array();

  auto qFavourited = STATEMENT("SELECT owner FROM like WHERE object = ?");
  qFavourited.bind(1, n->uri);

  while (qFavourited.executeStep()) {
    User u = User::lookupuri(qFavourited.getColumn(0)).value();
    accounts.push_back(u.renderMS());
  }

  OK(accounts, MIMEJSON);
}

GET(status_reblogged_by, "/api/v1/statuses/:id/reblogged_by") {
  string id (req->getParameter("id"));
  auto n = Note::lookupid(id);
  if (!n.has_value()) {
    ERROR(404, "no note");
  }

  json accounts = json::array();

  auto qRenoted = STATEMENT("SELECT owner FROM note WHERE renoteUri = ?");
  qRenoted.bind(1, n->uri);

  while (qRenoted.executeStep()) {
    User u = User::lookupuri(qRenoted.getColumn(0)).value();
    accounts.push_back(u.renderMS());
  }

  OK(accounts, MIMEJSON);
}

GET(status_reactions, "/api/v1/statuses/:id/reactions") {
  MSAUTH

  string id (req->getParameter("id"));
  auto n = Note::lookupid(id);
  if (!n.has_value()) {
    ERROR(404, "no note");
  }


  json reactions = n.value().renderReactionsMS(authuser, true);
  OK(reactions, MIMEJSON);
}

GET(status_reactions_pleroma, "/api/v1/pleroma/statuses/:id/reactions") {
  MSAUTH

  string id (req->getParameter("id"));
  auto n = Note::lookupid(id);
  if (!n.has_value()) {
    ERROR(404, "no note");
  }


  json reactions = n.value().renderReactionsMS(authuser, true);
  OK(reactions, MIMEJSON);
}

GET(timelines, "/api/v1/timelines/home") {
  MSAUTH

  QueryBuilder query;
  query = query
    .select({"*"})
    .from("note")
    .orderBy("publishedClamped", "DESC");


  QueryBuilder followers;
  query = query.where(
      IN("owner", 
        followers
        .select({"followee"})
        .from("follow")
        .where(EQ("follower", authuser.uri))
      )
  );

  PAGINATE(query, Note, publishedClamped);
}

GET(timeline_federated, "/api/v1/timelines/public") {
  MSAUTH

  QueryBuilder query;
  query = query
    .select({"*"})
    .from("note")
    .orderBy("publishedClamped", "DESC");

  if (req->getQuery("local") == "true") {
    query = query.where(EQ("host", cfg.domain));
  }

  if (req->getQuery("only_media") == "true") {
    query = query.where(GT("json_array_length(attachments)", 0));
  }

  PAGINATE(query, Note, publishedClamped);
}

GET(timeline_bubble, "/api/v1/timelines/bubble") {
  MSAUTH

  QueryBuilder query;
  query = query
    .select({"*"})
    .from("note")
    .orderBy("publishedClamped", "DESC");

  
  vector<string> bubbledHosts = cfg.bubbledHosts;
  QueryConstraint bubble = EQ("host", bubbledHosts.back());
  bubbledHosts.pop_back();
  for (const string host : bubbledHosts) {
    bubble = OR(bubble, EQ("host", host));
  }
  query = query.where(bubble);

  PAGINATE(query, Note, publishedClamped);
}

