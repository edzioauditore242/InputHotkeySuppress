#pragma once
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace Configuration {
    struct SuppressedEntry {
        std::string name;
        std::uint32_t keyCode = 0;
    };

    // ====================== SETTINGS ======================
    inline bool Enabled = true;
    inline std::uint32_t Hotkey = 0xC5;  // Hold this key
    inline bool ShowStatusOverlay = true;
    inline bool Debug = false;  // NEW

    inline std::vector<SuppressedEntry> SuppressedList;

    // Runtime only
    inline bool IsBlocking = false;
    inline bool IsListeningForHotkey = false;

    // For the Add Key window
    inline bool IsAddWindowOpen = false;
    inline bool IsListeningForNewKey = false;
    inline std::string TempNewName;
    inline std::uint32_t TempNewKeyCode = 0;

    // ====================== INI PATH ======================
    inline std::filesystem::path GetIniPath() {
        auto path = SKSE::log::log_directory();
        if (path) {
            return *path / "InputHotkeySuppression.ini";
        }
        return "Data/SKSE/Plugins/InputHotkeySuppression.ini";
    }

    // ====================== SAVE ======================
    inline void Save() {
        std::ofstream file(GetIniPath());
        if (!file.is_open()) {
            logger::error("Failed to open INI for writing");
            return;
        }

        file << "[General]\n";
        file << "Enabled=" << (Enabled ? "true" : "false") << "\n";
        file << "Hotkey=0x" << std::hex << Hotkey << std::dec << "\n";
        file << "ShowStatusOverlay=" << (ShowStatusOverlay ? "true" : "false") << "\n";
        file << "Debug=" << (Debug ? "true" : "false") << "\n\n";

        file << "[Suppressed]\n";
        for (const auto& entry : SuppressedList) {
            file << entry.name << "|0x" << std::hex << entry.keyCode << std::dec << "\n";
        }

        file.close();
        logger::info("Settings saved");
    }

    // ====================== LOAD ======================
    inline void Load() {
        auto path = GetIniPath();
        if (!std::filesystem::exists(path)) {
            SuppressedList = {{"Inventory", 0x17}, {"Magic", 0x24}, {"Map", 0x32}, {"Tween Menu", 0x0F}, {"Favorites", 0x21}};
            Save();
            return;
        }

        std::ifstream file(path);
        if (!file.is_open()) return;

        SuppressedList.clear();
        std::string line;
        bool inSuppressed = false;

        while (std::getline(file, line)) {
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line.empty() || line[0] == ';' || line[0] == '#') continue;

            if (line == "[Suppressed]") {
                inSuppressed = true;
                continue;
            }
            if (line[0] == '[') {
                inSuppressed = false;
                continue;
            }

            if (inSuppressed) {
                auto pos = line.find('|');
                if (pos == std::string::npos) continue;

                SuppressedEntry entry;
                entry.name = line.substr(0, pos);
                std::string codeStr = line.substr(pos + 1);

                try {
                    entry.keyCode = static_cast<std::uint32_t>(std::stoul(codeStr, nullptr, 0));
                    SuppressedList.push_back(entry);
                } catch (...) {
                }
            } else {
                auto eq = line.find('=');
                if (eq == std::string::npos) continue;

                std::string key = line.substr(0, eq);
                std::string val = line.substr(eq + 1);

                key.erase(key.find_last_not_of(" \t") + 1);
                val.erase(0, val.find_first_not_of(" \t"));

                if (key == "Enabled")
                    Enabled = (val == "true" || val == "1");
                else if (key == "Hotkey") {
                    try {
                        Hotkey = static_cast<std::uint32_t>(std::stoul(val, nullptr, 0));
                    } catch (...) {
                    }
                } else if (key == "ShowStatusOverlay")
                    ShowStatusOverlay = (val == "true" || val == "1");
                else if (key == "Debug")
                    Debug = (val == "true" || val == "1");
            }
        }

        file.close();
        logger::info("Settings loaded ({} suppressed keys)", SuppressedList.size());
    }

    inline bool IsKeySuppressed(std::uint32_t keyCode) {
        return std::any_of(SuppressedList.begin(), SuppressedList.end(), [keyCode](const SuppressedEntry& e) { return e.keyCode == keyCode; });
    }
}