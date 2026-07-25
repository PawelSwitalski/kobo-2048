#include "doctest/doctest.h"

#include <filesystem>

#include "persist/store.h"

namespace fs = std::filesystem;

TEST_CASE("atomic write: correct contents, tmp cleaned up, overwrite works") {
    fs::path dir = fs::temp_directory_path() / "kobo-app-test";
    fs::create_directories(dir);
    std::string path = (dir / "test.json").string();

    CHECK(kobo_2048::persist::saveFileAtomic(path, "first"));
    auto r1 = kobo_2048::persist::loadFile(path);
    REQUIRE(r1.has_value());
    CHECK(*r1 == "first");
    CHECK(!fs::exists(path + ".tmp"));

    CHECK(kobo_2048::persist::saveFileAtomic(path, "second, longer contents"));
    auto r2 = kobo_2048::persist::loadFile(path);
    REQUIRE(r2.has_value());
    CHECK(*r2 == "second, longer contents");
    CHECK(!fs::exists(path + ".tmp"));

    kobo_2048::persist::removeFile(path);
    CHECK(!fs::exists(path));
    CHECK(!kobo_2048::persist::loadFile(path).has_value());  // missing => nullopt
    kobo_2048::persist::removeFile(path);                    // idempotent
    fs::remove_all(dir);
}
