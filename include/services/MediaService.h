#pragma once

#include "entities/Media.h"
namespace MediaService {
  Media uploadMedia(User &uploader, string body, string mimeType, string description, bool sensitive);
}
