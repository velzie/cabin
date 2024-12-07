#include "SQLiteCpp/Statement.h"
#define USE_DB
#include <router.h>
#include <common.h>
#include "database.h"
#include "utils.h"
#include "services/note.h"
#include "services/user.h"
#include "http.h"
#include "entities/Like.h"

POST(post_status, "/api/v1/statuses") {
  MSAUTH

  optional<Note> replyTo;
  string status;

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
              replyTo = NoteService::lookup(string(data));
            }

            
          }
        }
      }
    }
  } else {
    // mastodon style
    if (body["in_reply_to_id"].is_string()) {
      replyTo = NoteService::lookup(body["in_reply_to_id"]);
    }
    status = body["status"];
  }

  
  Note note = NoteService::create(authuser, status, replyTo);

  OK(note.renderMS(authuser), MIMEJSON);
}

GET(status, "/api/v1/statuses/:id") {
  MSAUTH
  string id (req->getParameter("id"));

  auto n = NoteService::lookup(id);
  if (!n.has_value()) {
    ERROR(404, "no note");
  }

  json j = n.value().renderMS(authuser);
  OK(j, MIMEJSON);
}

POST(status_like, "/api/v1/statuses/:id/favourite") {
  MSAUTH
  string id (req->getParameter("id"));

  auto note = NoteService::lookup(id);
  if (!note.has_value()) {
    ERROR(404, "no note");
  }

  string likeid = utils::genid();
  Like like = {
    .uri = LIKE(likeid),
    .id = likeid,
    .local = true,

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

  note = NoteService::lookup(id);
  OK(note->renderMS(authuser), MIMEJSON);
}

POST(status_renote, "/api/v1/statuses/:id/reblog") {
  MSAUTH

  string id(req->getParameter("id"));
  auto note = NoteService::lookup(id);
  if (!note.has_value()) {
    ERROR(404, "no note");
  }

  Note n = NoteService::createRenote(authuser, note.value());

  OK(n.renderMS(authuser), MIMEJSON);
}

GET(status_context, "/api/v1/statuses/:id/context") {
  MSAUTH

  string id (req->getParameter("id"));
  auto originalnote = NoteService::lookup(id);
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
  auto n = NoteService::lookup(id);
  if (!n.has_value()) {
    ERROR(404, "no note");
  }

  json accounts = json::array();

  auto qFavourited = STATEMENT("SELECT owner FROM like WHERE object = ?");
  qFavourited.bind(1, n->uri);

  while (qFavourited.executeStep()) {
    User u = UserService::lookup_ap(qFavourited.getColumn(0)).value();
    accounts.push_back(u.renderMS());
  }

  OK(accounts, MIMEJSON);
}

GET(status_reblogged_by, "/api/v1/statuses/:id/reblogged_by") {
  string id (req->getParameter("id"));
  auto n = NoteService::lookup(id);
  if (!n.has_value()) {
    ERROR(404, "no note");
  }

  json accounts = json::array();

  auto qRenoted = STATEMENT("SELECT owner FROM note WHERE renoteUri = ?");
  qRenoted.bind(1, n->uri);

  while (qRenoted.executeStep()) {
    User u = UserService::lookup_ap(qRenoted.getColumn(0)).value();
    accounts.push_back(u.renderMS());
  }

  OK(accounts, MIMEJSON);
}

GET(status_reactions, "/api/v1/statuses/:id/reactions") {
  string id (req->getParameter("id"));
  auto n = NoteService::lookup(id);
  if (!n.has_value()) {
    ERROR(404, "no note");
  }

  json reactions = json::array();
  OK(reactions, MIMEJSON);
}

GET(timelines, "/api/v1/timelines/:id") {
  MSAUTH

  string tlname (req->getParameter("id"));

  std::stringstream ss(string(req->getQuery("limit")));
  int limit = 0;
  ss >> limit;
  if (!limit) limit = 20;
  if (limit > 20) limit = 20;


  // top ryri (180e61a5efb46fa1lkQrLikM)
  // knizer boston
  // arab
  // trump

  string max_id (req->getQuery("max_id"));
  string min_id (req->getQuery("min_id"));
  string since_id (req->getQuery("since_id"));

  // default, no pagination
  SQLite::Statement q = STATEMENT("SELECT * FROM note ORDER BY publishedClamped DESC LIMIT ?");
  q.bind(1, limit);

  if (!max_id.empty()) {
    // start at max_id and paginated down
    Note upperNote = NoteService::lookup(max_id).value();
    q = STATEMENT("SELECT * FROM note WHERE publishedClamped < ? ORDER BY publishedClamped DESC LIMIT ?");
    q.bind(1, upperNote.publishedClamped);
    q.bind(2, limit);
  } else if (!min_id.empty()) {
    // start at low id, paginate up
    Note lowerNote = NoteService::lookup(min_id).value();
    q = STATEMENT("SELECT * FROM note WHERE publishedClamped > ? ORDER BY publishedClamped DESC LIMIT ?");
    q.bind(1, lowerNote.publishedClamped);
    q.bind(2, limit);
  } else if (!since_id.empty()) {
    // start at most recent date, paginate down but don't go further than since_id
    Note lowerNote = NoteService::lookup(since_id).value();
    q = STATEMENT(R"SQL(
        SELECT *
        FROM (
          SELECT * FROM note
          WHERE publishedClamped > ?
          ORDER BY publishedClamped DESC
          LIMIT ?
        ) AS subquery
        ORDER BY publishedClamped
    )SQL");
    q.bind(1, lowerNote.publishedClamped);
    q.bind(2, limit);
  }


  json response = json::array();

  string ret_max_id;
  string ret_min_id;


  while (q.executeStep()) {
    Note n;
    n.load(q);

    if (ret_min_id.empty()) ret_min_id = n.id;
    ret_max_id = n.id;

    response.push_back(n.renderMS(authuser));
  }

  res->writeHeader("Link",
      FMT("<{}>; rel=\"next\",<{}>; rel=\"prev\"", 
        MAPI(FMT("timelines/{}?max_id={}", tlname, ret_max_id)),
        MAPI(FMT("timelines/{}?min_id={}", tlname, ret_min_id))
      )
  );

  OK(response, MIMEJSON);
}
