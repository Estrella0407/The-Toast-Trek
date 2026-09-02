#include "SaveGame.h"
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>

namespace {
    const char* kProgressFile = "savegame.txt";
    const char* kSettingsFile = "settings.txt";
}

namespace save {

    bool HasProgress() {
        std::ifstream f(kProgressFile);
        return f.good();
    }

    Progress LoadProgress() {
        Progress p;
        std::ifstream f(kProgressFile);
        if (!f.good()) return p;   // Valid stays false

        // A tag whose value is a space-separated int list to end of line.
        auto readList = [&f](std::vector<int>& out) {
            std::string rest;
            std::getline(f, rest);
            std::istringstream ss(rest);
            int v;
            while (ss >> v) out.push_back(v);
        };

        std::string tag;
        while (f >> tag) {
            if (tag == "map")        f >> p.mapId;
            else if (tag == "pos")   f >> p.px >> p.py;
            else if (tag == "level") f >> p.level;
            else if (tag == "inv")   f >> p.potions >> p.bones >> p.toast;
            else if (tag == "cleared") readList(p.clearedMaps);
            else if (tag == "items")   readList(p.collectedItems);
            else if (tag == "bosses")  readList(p.clearedBosses);
        }
        p.valid = true;
        return p;
    }

    void SaveProgress(const Progress& p) {
        std::ofstream f(kProgressFile, std::ios::trunc);
        if (!f.good()) return;
        f << "map " << p.mapId << "\n";
        f << "pos " << p.px << " " << p.py << "\n";
        f << "level " << p.level << "\n";
        f << "inv " << p.potions << " " << p.bones << " " << p.toast << "\n";
        f << "cleared";
        for (int id : p.clearedMaps) f << " " << id;
        f << "\n";
        f << "items";
        for (int k : p.collectedItems) f << " " << k;
        f << "\n";
        f << "bosses";
        for (int k : p.clearedBosses) f << " " << k;
        f << "\n";
    }

    void ClearProgress() {
        std::remove(kProgressFile);
    }

    Settings LoadSettings() {
        Settings s;   // Defaults
        std::ifstream f(kSettingsFile);
        if (!f.good()) return s;

        std::string tag;
        while (f >> tag) {
            if (tag == "master")     f >> s.master;
            else if (tag == "music") f >> s.music;
            else if (tag == "sfx")   f >> s.sfx;
            else if (tag == "muted") { int m = 0; f >> m; s.muted = (m != 0); }
        }
        return s;
    }

    void SaveSettings(const Settings& s) {
        std::ofstream f(kSettingsFile, std::ios::trunc);
        if (!f.good()) return;
        f << "master " << s.master << "\n";
        f << "music " << s.music << "\n";
        f << "sfx " << s.sfx << "\n";
        f << "muted " << (s.muted ? 1 : 0) << "\n";
    }
}
