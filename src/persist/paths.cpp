#include "persist/paths.h"

#include <cstdlib>
#include <filesystem>

namespace kobo_2048::persist {

namespace fs = std::filesystem;

Paths resolveDataDir(const char* cliOverride) {
    std::string dir;
    if (cliOverride && *cliOverride) {
        dir = cliOverride;
    } else if (const char* env = std::getenv("KOBO_2048_DATA_DIR"); env && *env) {
        dir = env;
    } else {
        std::error_code ec;
        const char* kobo = "/mnt/onboard/.adds/kobo_2048";
        dir = fs::is_directory(kobo, ec) ? kobo : "kobo_2048-data";
    }
    std::error_code ec;
    fs::create_directories(dir, ec);  // best effort; saves will fail loudly if unusable

    Paths p;
    p.dataDir = dir;
    p.save = (fs::path(dir) / "save.json").string();
    p.best = (fs::path(dir) / "best.json").string();
    p.settings = (fs::path(dir) / "settings.json").string();
    return p;
}

}  // namespace kobo_2048::persist
