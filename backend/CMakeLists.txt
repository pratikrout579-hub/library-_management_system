include(FetchContent)

find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# Crow uses the standalone Asio package installed by the toolchain. Keep its
# optional examples, tests, and install rules out of this application build.
set(CROW_USE_BOOST OFF CACHE BOOL "Use Boost.Asio for Crow" FORCE)
set(CROW_BUILD_EXAMPLES OFF CACHE BOOL "Build Crow examples" FORCE)
set(CROW_BUILD_TESTS OFF CACHE BOOL "Build Crow tests" FORCE)
set(CROW_INSTALL OFF CACHE BOOL "Install Crow" FORCE)

FetchContent_Declare(
  Crow
  GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
  GIT_TAG v1.3.3
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Crow)

add_executable(smart-library
  src/main.cpp
  src/repositories/Database.cpp
  src/repositories/LibraryRepositories.cpp
  src/services/AuthService.cpp
  src/services/LibraryService.cpp
  src/services/ReportService.cpp
  src/controllers/Routes.cpp
  src/utils/ActivityLogger.cpp
)

target_include_directories(smart-library PRIVATE include)
target_compile_features(smart-library PRIVATE cxx_std_20)
target_link_libraries(smart-library PRIVATE Crow::Crow SQLite3::SQLite3 OpenSSL::Crypto)
target_compile_definitions(smart-library PRIVATE
  PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
  CROW_STATIC_DIRECTORY="${CMAKE_SOURCE_DIR}/frontend"
  CROW_STATIC_ENDPOINT="/<path>"
)
