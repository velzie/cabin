cmake_minimum_required(VERSION 3.10)
project(cabin C CXX)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(.third-party/json)
add_subdirectory(.third-party/sqlitecpp)
add_subdirectory(.third-party/spdlog)
add_subdirectory(.third-party/dbg-macro)
add_subdirectory(.third-party/fmt)
add_subdirectory(wrapusockets)
add_subdirectory(wrapcpptrace)

include_directories(include)
include_directories(.third-party/uWebSockets/src)

set(SOURCES 
  .third-party/httplib.cc

  src/main.cc
  src/http.cc
  src/server.cc
  src/database.cc
  src/util.cc

  src/entities/Emoji.cc
  src/entities/EmojiReact.cc
  src/entities/Follow.cc
  src/entities/Like.cc
  src/entities/Note.cc
  src/entities/User.cc
  src/entities/Notification.cc
  src/entities/Media.cc

  src/services/UserService.cc
  src/services/NoteService.cc
  src/services/IngestService.cc
  src/services/DeliveryService.cc
  src/services/FollowService.cc
  src/services/InstanceService.cc
  src/services/NotificationService.cc
  src/services/FetchService.cc
  src/services/BiteService.cc
  src/services/MediaService.cc

  src/workers/BubbleFetcher.cc

  src/routes/federation/webfinger.cc
  src/routes/federation/meta.cc
  src/routes/federation/notes.cc
  src/routes/federation/instance.cc

  src/routes/web/main.cc

  src/routes/mastodon/apps.cc
  src/routes/mastodon/accounts.cc
  src/routes/mastodon/statuses.cc
  src/routes/mastodon/notifications.cc
  src/routes/mastodon/timeline.cc

  src/routes/pleroma/frontend.cc
  src/routes/pleroma/emojis.cc

  src/migrations/1-init.cc
)

find_package(OpenSSL REQUIRED)
# set(OPENSSL_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/openssl-src)
# set(OPENSSL_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/openssl)
# set(OPENSSL_INCLUDE_DIR ${OPENSSL_INSTALL_DIR}/include)
# set(OPENSSL_CONFIGURE_COMMAND ${OPENSSL_SOURCE_DIR}/config)
# ExternalProject_Add(
#   OpenSSL
#   SOURCE_DIR ${OPENSSL_SOURCE_DIR}
#   GIT_REPOSITORY https://github.com/openssl/openssl.git
#   GIT_TAG openssl-3.4.0
#   USES_TERMINAL_DOWNLOAD TRUE
#   CONFIGURE_COMMAND
#     ${OPENSSL_CONFIGURE_COMMAND}
#     --prefix=${OPENSSL_INSTALL_DIR}
#     --openssldir=${OPENSSL_INSTALL_DIR}
#     --api=1.1.0
#   BUILD_COMMAND make
#   TEST_COMMAND ""
#   INSTALL_COMMAND make install
#   INSTALL_DIR ${OPENSSL_INSTALL_DIR}
# )
# file(MAKE_DIRECTORY ${OPENSSL_INCLUDE_DIR})
#
# add_library(OpenSSL::SSL STATIC IMPORTED GLOBAL)
# set_property(TARGET OpenSSL::SSL PROPERTY IMPORTED_LOCATION ${OPENSSL_INSTALL_DIR}/lib64/libssl.a)
# set_property(TARGET OpenSSL::SSL PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${OPENSSL_INCLUDE_DIR})
# add_dependencies(OpenSSL::SSL OpenSSL)
#
# add_library(OpenSSL::Crypto STATIC IMPORTED GLOBAL)
# set_property(TARGET OpenSSL::Crypto PROPERTY IMPORTED_LOCATION ${OPENSSL_INSTALL_DIR}/lib64/libcrypto.a)
# set_property(TARGET OpenSSL::Crypto PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${OPENSSL_INCLUDE_DIR})
# add_dependencies(OpenSSL::Crypto OpenSSL)
#
# set(OPENSSL_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)
# set(OPENSSL_MSVC_STATIC_RT ON CACHE BOOL "" FORCE)
# set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# include_directories(openssl/include)
# FetchContent_Declare(
#     zlib
#     GIT_REPOSITORY https://github.com/madler/zlib.git
#     GIT_TAG        v1.2.13
# )
#
# FetchContent_MakeAvailable(zlib)

# add_link_options(-static)
add_link_options(-static-libstdc++)
add_compile_options(-fsanitize=address -rdynamic)
add_link_options(-fsanitize=address -rdynamic)
add_executable(cabin ${SOURCES})

set(OPENSSL_USE_STATIC_LIBS TRUE)
target_link_libraries(cabin PUBLIC nlohmann_json::nlohmann_json SQLiteCpp dbg_macro fmt uSockets OpenSSL::SSL OpenSSL::Crypto z)
target_link_libraries(cabin PRIVATE spdlog cpptrace::cpptrace dwarf)
