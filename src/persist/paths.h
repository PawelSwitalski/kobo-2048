#pragma once
#include <string>

namespace kobo_2048::persist {

struct Paths {
    std::string dataDir;
    std::string save;      // dataDir/save.json — in-progress GameSession (FR-012)
    std::string best;      // dataDir/best.json — BestScore (FR-008)
    std::string settings;  // dataDir/settings.json — DisplaySettings (FR-020)
};

// Resolution order: cliOverride > $KOBO_2048_DATA_DIR > /mnt/onboard/.adds/kobo_2048
// (when it exists, i.e. on a Kobo) > ./kobo_2048-data. Creates the directory.
Paths resolveDataDir(const char* cliOverride);

}  // namespace kobo_2048::persist
