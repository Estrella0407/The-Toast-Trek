#pragma once
#include <vector>

// Plain-text persistence
// Everything the game needs to resume is a handful of ints (savegame.txt)
// plus the audio settings (settings.txt), both written next to the executable
namespace save {

    struct Progress {
        bool valid = false;             // false = no usable save on disk
        int  mapId = 0;                 // MapId as int
        float px = 0.0f, py = 0.0f;     // Pochi's position on that map
        int  level = 1;
        int  potions = 0, bones = 0, toast = 0;
        std::vector<int> clearedMaps;    // MapId ints (all bosses beaten)
        std::vector<int> collectedItems; // encoded mapId*16 + item index
        std::vector<int> clearedBosses;  // encoded mapId*16 + boss index
    };

    struct Settings {
        float master = 1.0f;
        float music = 0.8f;
        float sfx = 1.0f;
        bool  muted = false;
    };

    // --- Progress ---
    bool HasProgress();               // true if savegame.txt exists and parses
    Progress LoadProgress();          // .valid == false on any failure
    void SaveProgress(const Progress& p);
    void ClearProgress();             // Delete the file (new game / game finished)

    // --- Settings ---
    Settings LoadSettings();          // Defaults on any failure
    void SaveSettings(const Settings& s);
}
