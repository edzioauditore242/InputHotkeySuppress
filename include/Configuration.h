#pragma once
#include <algorithm>
#include <cstdio>
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
    inline std::uint32_t Hotkey = 0xC5;  // Default: Pause
    inline bool ShowStatusOverlay = true;
    inline bool Debug = false;

    inline std::vector<SuppressedEntry> SuppressedList;

    // ====================== RUNTIME ONLY ======================
    inline bool IsBlocking = false;
    inline bool IsListeningForHotkey = false;

    // Add window
    inline bool IsListeningForNewKey = false;
    inline std::string TempNewName;
    inline std::uint32_t TempNewKeyCode = 0;

    // Edit window
    inline bool IsListeningForEditKey = false;
    inline size_t EditIndex = 0;
    inline std::string TempEditName;
    inline std::uint32_t TempEditKeyCode = 0;

    // Delete confirmation
    inline size_t DeleteIndex = 0;

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
            // Default list
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
            // Trim
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

                // Trim
                key.erase(key.find_last_not_of(" \t") + 1);
                val.erase(0, val.find_first_not_of(" \t"));

                if (key == "Enabled") {
                    Enabled = (val == "true" || val == "1");
                } else if (key == "Hotkey") {
                    try {
                        Hotkey = static_cast<std::uint32_t>(std::stoul(val, nullptr, 0));
                    } catch (...) {
                    }
                } else if (key == "ShowStatusOverlay") {
                    ShowStatusOverlay = (val == "true" || val == "1");
                } else if (key == "Debug") {
                    Debug = (val == "true" || val == "1");
                }
            }
        }

        file.close();
        logger::info("Settings loaded ({} suppressed keys)", SuppressedList.size());
    }

    // ====================== HELPERS ======================
    inline bool IsKeySuppressed(std::uint32_t keyCode) {
        return std::any_of(SuppressedList.begin(), SuppressedList.end(), [keyCode](const SuppressedEntry& e) { return e.keyCode == keyCode; });
    }

    // ====================== KEY NAME LOOKUP ======================
    inline std::string GetKeyName(std::uint32_t code) {
        switch (code) {
            case 0x01:
                return "Escape";
            case 0x0E:
                return "Backspace";
            case 0x0F:
                return "Tab";
            case 0x1C:
                return "Enter";
            case 0x39:
                return "Space";
            case 0xC5:
                return "Pause";
            case 0xC7:
                return "Home";
            case 0xC8:
                return "Up";
            case 0xC9:
                return "PageUp";
            case 0xCB:
                return "Left";
            case 0xCD:
                return "Right";
            case 0xCF:
                return "End";
            case 0xD0:
                return "Down";
            case 0xD1:
                return "PageDown";
            case 0xD2:
                return "Insert";
            case 0xD3:
                return "Delete";

            case 0x10:
                return "Q";
            case 0x11:
                return "W";
            case 0x12:
                return "E";
            case 0x13:
                return "R";
            case 0x14:
                return "T";
            case 0x15:
                return "Y";
            case 0x16:
                return "U";
            case 0x17:
                return "I";
            case 0x18:
                return "O";
            case 0x19:
                return "P";
            case 0x1E:
                return "A";
            case 0x1F:
                return "S";
            case 0x20:
                return "D";
            case 0x21:
                return "F";
            case 0x22:
                return "G";
            case 0x23:
                return "H";
            case 0x24:
                return "J";
            case 0x25:
                return "K";
            case 0x26:
                return "L";
            case 0x2C:
                return "Z";
            case 0x2D:
                return "X";
            case 0x2E:
                return "C";
            case 0x2F:
                return "V";
            case 0x30:
                return "B";
            case 0x31:
                return "N";
            case 0x32:
                return "M";

            case 0x02:
                return "1";
            case 0x03:
                return "2";
            case 0x04:
                return "3";
            case 0x05:
                return "4";
            case 0x06:
                return "5";
            case 0x07:
                return "6";
            case 0x08:
                return "7";
            case 0x09:
                return "8";
            case 0x0A:
                return "9";
            case 0x0B:
                return "0";

            case 0x3B:
                return "F1";
            case 0x3C:
                return "F2";
            case 0x3D:
                return "F3";
            case 0x3E:
                return "F4";
            case 0x3F:
                return "F5";
            case 0x40:
                return "F6";
            case 0x41:
                return "F7";
            case 0x42:
                return "F8";
            case 0x43:
                return "F9";
            case 0x44:
                return "F10";
            case 0x57:
                return "F11";
            case 0x58:
                return "F12";

            default: {
                char buf[16];
                sprintf_s(buf, "0x%X", code);
                return std::string(buf);
            }
        }
    }
}