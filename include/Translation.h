#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

namespace Translation {
    inline std::unordered_map<std::string, std::string> Strings;

    inline std::filesystem::path GetTranslationPath() {
        auto path = SKSE::log::log_directory();
        if (path) {
            return *path / "InputHotkeySuppression_translation.txt";
        }
        return "Data/SKSE/Plugins/InputHotkeySuppression_translation.txt";
    }

    inline void Load() {
        Strings.clear();

        // Default English
        Strings["SUPPRESSION_ACTIVE"] = "SUPPRESSION ACTIVE";
        Strings["ENABLE_MOD"] = "Enable Mod";
        Strings["SHOW_STATUS_OVERLAY"] = "Show Status Overlay";
        Strings["DEBUG"] = "Debug Logging";
        Strings["HOLD_HOTKEY"] = "Hold Hotkey (press and hold this key to suppress the list below)";
        Strings["CURRENT_HOTKEY"] = "Current Hotkey: 0x%X";
        Strings["CHANGE_HOTKEY"] = "Change Hold Hotkey";
        Strings["LISTENING_HOTKEY"] = ">>> Press any key now to set as Hold Hotkey...";
        Strings["SUPPRESSED_KEYS"] = "Suppressed Keys";
        Strings["REMOVE"] = "Remove";
        Strings["ADD_KEY_BUTTON"] = "Add Key for Suppression";
        Strings["ADD_WINDOW_TITLE"] = "Add Suppression Key";
        Strings["NAME"] = "Name";
        Strings["LISTEN_FOR_KEY"] = "Listen for Key";
        Strings["LISTENING_NEW_KEY"] = ">>> Press the key you want to suppress...";
        Strings["DETECTED_KEY"] = "Detected Key: 0x%X";
        Strings["NO_KEY_YET"] = "No key detected yet";
        Strings["SAVE"] = "Save";
        Strings["CANCEL"] = "Cancel";
        Strings["SAVE_SETTINGS"] = "Save Settings";
        Strings["RELOAD_SETTINGS"] = "Reload Settings";

        auto path = GetTranslationPath();
        if (!std::filesystem::exists(path)) {
            std::ofstream out(path);
            if (out.is_open()) {
                out << "; Input Hotkey Suppression - Translation File\n";
                out << "; Change only the text after the = sign\n\n";
                for (const auto& [key, value] : Strings) {
                    out << key << "=" << value << "\n";
                }
                out.close();
            }
            return;
        }

        std::ifstream file(path);
        if (!file.is_open()) return;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == ';' || line[0] == '#') continue;

            auto pos = line.find('=');
            if (pos == std::string::npos) continue;

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            key.erase(0, key.find_first_not_of(" \t\r\n"));
            key.erase(key.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);

            if (!key.empty()) {
                Strings[key] = value;
            }
        }
        file.close();
    }

    inline const char* Get(const std::string& key) {
        auto it = Strings.find(key);
        if (it != Strings.end()) {
            return it->second.c_str();
        }
        return key.c_str();
    }
}