#include "services/MediaService.h"
#include "entities/Media.h"
#include <fstream>

namespace MediaService {
  Media uploadMedia(User &uploader, string body, string mimeType, string description, bool sensitive) {
    auto m = Media::create(mimeType);
    m.owner = uploader.uri;
    m.sensitive = sensitive;
    m.description = description;
    
    std::ofstream file(cfg.mediapath + m.id, std::ios::binary);
    file.write(body.c_str(), body.size());
    file.close();

    m.insert();
    return m;
  }
}
