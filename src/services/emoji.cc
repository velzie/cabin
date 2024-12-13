#include "services/emoji.h"
#include "database.h"
#include "utils.h"


namespace EmojiService {
  Emoji Parse(const json tag, const string host) {
    Emoji e;
    e.shortcode = tag["name"];
    e.address = e.shortcode + "@" + host;
    e.host = host;
    e.local = false;
    e.imageurl = tag["icon"]["url"];
    

    INSERT_OR_UPDATE(e, address, id, utils::genid());
    return e;
  }
}
