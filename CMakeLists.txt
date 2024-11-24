cmake_minimum_required(VERSION 3.10)
project(cabin C CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenSSL REQUIRED)

add_subdirectory(third-party/json)
add_subdirectory(third-party/sqlitecpp)
add_subdirectory(third-party/spdlog)
add_subdirectory(third-party/dbg-macro)
add_subdirectory(third-party/backward-cpp)

include_directories(include)

set(SOURCES 
  third-party/httplib.cc

  src/main.cc
  src/http.cc
  src/server.cc

  src/routes/federation/webfinger.cc
  src/routes/federation/meta.cc
  src/routes/federation/notes.cc
  src/routes/federation/instance.cc

  src/routes/web/main.cc
)
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

add_executable(cabin ${SOURCES})

target_link_libraries(cabin nlohmann_json::nlohmann_json OpenSSL::SSL SQLiteCpp spdlog Backward::Interface dbg_macro)
