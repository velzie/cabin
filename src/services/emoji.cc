#include "services/emoji.h"
#include "database.h"
#include "utils.h"


namespace EmojiService {
  Emoji parse(const json tag, const string host) {
    Emoji e;
    string shortcode = tag["name"];
    ASSERT(shortcode.length() > 2);

    // :name: -> name
    if (shortcode[0] == ':') {
      shortcode.erase(0, 1);
      shortcode.pop_back();
    }

    e.shortcode = shortcode;
    e.address = shortcode + "@" + host;
    e.host = host;
    e.local = false;
    e.imageurl = tag["icon"]["url"];
    

    INSERT_OR_UPDATE(e, address, id, utils::genid());
    return e;
  }
}
