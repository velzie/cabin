#include "services/EmojiService.h"
#include "entities/EmojiReact.h"
#include "services/DeliveryService.h"

namespace EmojiService {
  EmojiReact CreateReact(User &reacter, Note &note, optional<Emoji> &emoji, optional<string> emojiText) { 
    string reactid = utils::genid();
    EmojiReact react = {
      .uri = REACT(reactid),
      .id = reactid,
      .local = true,
      .host = cfg.domain,

      .owner = reacter.uri,
      .object = note.uri
    };

    if (emoji.has_value()) {
      react.emojiText = nullopt;
      react.emojiId = emoji->id;
    } else {
      react.emojiId = nullopt;
      react.emojiText = emojiText;
    }

    react.insert();

    json activity = {
      {"actor", reacter.uri},
      {"id", react.uri},
      {"object", note.uri},
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

    DeliveryService::Audience au = {
      .actor = reacter,
      .mentions = {note.owner},
      .aspublic = true,
      .followers = true,
    };
    DeliveryService::QueueDelivery(activity, au);

    return react;
  }
}
