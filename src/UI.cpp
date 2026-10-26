#include "UI.h"

#include <cstring>

#include "Translation.h"

void UI::Register() {
    if (!SKSEMenuFramework::IsInstalled()) {
        logger::error("SKSE Menu Framework is not installed!");
        return;
    }

    SKSEMenuFramework::SetSection("Input Hotkey Suppression");
    SKSEMenuFramework::AddSectionItem("Settings", RenderSettings);

    // Create the Add Key window (starts closed)
    AddKeyWindow = SKSEMenuFramework::AddWindow(RenderAddWindow, true);
    if (AddKeyWindow) {
        AddKeyWindow->IsOpen = false;
    }

    SKSEMenuFramework::AddHudElement(RenderOverlay);
    SKSEMenuFramework::AddInputEvent(OnInput);

    logger::info("UI registered");
}

// =========================================================
// MAIN SETTINGS PAGE
// =========================================================
void __stdcall UI::RenderSettings() {
    ImGuiMCP::Checkbox(Translation::Get("ENABLE_MOD"), &Configuration::Enabled);
    ImGuiMCP::Checkbox(Translation::Get("SHOW_STATUS_OVERLAY"), &Configuration::ShowStatusOverlay);
    ImGuiMCP::Checkbox(Translation::Get("DEBUG"), &Configuration::Debug);

    ImGuiMCP::Separator();
    ImGuiMCP::Text(Translation::Get("HOLD_HOTKEY"));

    char hotkeyText[64];
    sprintf_s(hotkeyText, Translation::Get("CURRENT_HOTKEY"), Configuration::Hotkey);
    ImGuiMCP::Text(hotkeyText);

    if (Configuration::IsListeningForHotkey) {
        ImGuiMCP::TextColored(ImGuiMCP::ImVec4(1.0f, 1.0f, 0.0f, 1.0f), Translation::Get("LISTENING_HOTKEY"));
    } else {
        if (ImGuiMCP::Button(Translation::Get("CHANGE_HOTKEY"))) {
            Configuration::IsListeningForHotkey = true;
        }
    }

    ImGuiMCP::Separator();
    ImGuiMCP::Text(Translation::Get("SUPPRESSED_KEYS"));

    // List
    for (size_t i = 0; i < Configuration::SuppressedList.size(); ++i) {
        auto& entry = Configuration::SuppressedList[i];

        ImGuiMCP::PushID(static_cast<int>(i));

        char label[128];
        sprintf_s(label, "%s  (0x%X)", entry.name.c_str(), entry.keyCode);
        ImGuiMCP::Text(label);

        ImGuiMCP::SameLine();
        if (ImGuiMCP::Button(Translation::Get("REMOVE"))) {
            Configuration::SuppressedList.erase(Configuration::SuppressedList.begin() + i);
            Configuration::Save();
            ImGuiMCP::PopID();
            break;
        }

        ImGuiMCP::PopID();
    }

    ImGuiMCP::Spacing();
    if (ImGuiMCP::Button(Translation::Get("ADD_KEY_BUTTON"))) {
        // Reset temporary data and open window
        Configuration::TempNewName.clear();
        Configuration::TempNewKeyCode = 0;
        Configuration::IsListeningForNewKey = false;
        if (AddKeyWindow) {
            AddKeyWindow->IsOpen = true;
        }
    }

    ImGuiMCP::Separator();
    if (ImGuiMCP::Button(Translation::Get("SAVE_SETTINGS"))) {
        Configuration::Save();
    }
    ImGuiMCP::SameLine();
    if (ImGuiMCP::Button(Translation::Get("RELOAD_SETTINGS"))) {
        Configuration::Load();
        Translation::Load();
    }
}

// =========================================================
// ADD KEY WINDOW
// =========================================================
void __stdcall UI::RenderAddWindow() {
    if (!AddKeyWindow || !AddKeyWindow->IsOpen) {
        return;
    }

    auto viewport = ImGuiMCP::GetMainViewport();
    ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{420.0f, 280.0f}, ImGuiMCP::ImGuiCond_Appearing);
    ImGuiMCP::SetNextWindowPos(ImGuiMCP::ImVec2{viewport->Size.x * 0.5f, viewport->Size.y * 0.5f}, ImGuiMCP::ImGuiCond_Appearing, ImGuiMCP::ImVec2{0.5f, 0.5f});

    ImGuiMCP::Begin(Translation::Get("ADD_WINDOW_TITLE"), nullptr, ImGuiMCP::ImGuiWindowFlags_NoCollapse);

    // Name
    static char nameBuffer[64] = "";
    ImGuiMCP::InputText(Translation::Get("NAME"), nameBuffer, sizeof(nameBuffer));

    ImGuiMCP::Spacing();

    // Listen button
    if (Configuration::IsListeningForNewKey) {
        ImGuiMCP::TextColored(ImGuiMCP::ImVec4(1.0f, 1.0f, 0.0f, 1.0f), Translation::Get("LISTENING_NEW_KEY"));
    } else {
        if (ImGuiMCP::Button(Translation::Get("LISTEN_FOR_KEY"))) {
            Configuration::IsListeningForNewKey = true;
            Configuration::TempNewKeyCode = 0;
        }
    }

    ImGuiMCP::Spacing();

    // Show detected key
    if (Configuration::TempNewKeyCode != 0) {
        char detected[64];
        sprintf_s(detected, Translation::Get("DETECTED_KEY"), Configuration::TempNewKeyCode);
        ImGuiMCP::TextColored(ImGuiMCP::ImVec4(0.3f, 1.0f, 0.3f, 1.0f), detected);
    } else {
        ImGuiMCP::TextDisabled(Translation::Get("NO_KEY_YET"));
    }

    ImGuiMCP::Spacing();
    ImGuiMCP::Separator();
    ImGuiMCP::Spacing();

    // Save / Cancel
    if (ImGuiMCP::Button(Translation::Get("SAVE"))) {
        if (strlen(nameBuffer) > 0 && Configuration::TempNewKeyCode != 0) {
            if (!Configuration::IsKeySuppressed(Configuration::TempNewKeyCode)) {
                Configuration::SuppressedEntry entry;
                entry.name = nameBuffer;
                entry.keyCode = Configuration::TempNewKeyCode;
                Configuration::SuppressedList.push_back(entry);
                Configuration::Save();

                if (Configuration::Debug) {
                    logger::info("Added suppressed key: {} (0x{:X})", entry.name, entry.keyCode);
                }
            }
        }

        // Close and reset
        nameBuffer[0] = '\0';
        Configuration::TempNewName.clear();
        Configuration::TempNewKeyCode = 0;
        Configuration::IsListeningForNewKey = false;
        AddKeyWindow->IsOpen = false;
    }

    ImGuiMCP::SameLine();

    if (ImGuiMCP::Button(Translation::Get("CANCEL"))) {
        nameBuffer[0] = '\0';
        Configuration::TempNewName.clear();
        Configuration::TempNewKeyCode = 0;
        Configuration::IsListeningForNewKey = false;
        AddKeyWindow->IsOpen = false;
    }

    ImGuiMCP::End();
}

// =========================================================
// STATUS OVERLAY
// =========================================================
void __stdcall UI::RenderOverlay() {
    if (!Configuration::ShowStatusOverlay || !Configuration::Enabled) {
        return;
    }

    if (!Configuration::IsBlocking) {
        return;
    }

    if (SKSEMenuFramework::IsAnyBlockingWindowOpened()) {
        return;
    }

    auto drawList = ImGuiMCP::GetForegroundDrawList();
    const char* text = Translation::Get("SUPPRESSION_ACTIVE");

    ImGuiMCP::ImVec2 textSize = ImGuiMCP::CalcTextSize(text, 0, false, 0);

    ImGuiMCP::ImVec2 pos{ImGuiMCP::GetIO()->DisplaySize.x - textSize.x - 20.0f, 20.0f};

    ImGuiMCP::ImDrawListManager::AddText(drawList, pos, IM_COL32(255, 60, 60, 230), text);
}

// =========================================================
// INPUT HANDLER
// =========================================================
bool __stdcall UI::OnInput(RE::InputEvent* event) {
    if (!Configuration::Enabled || !event) {
        return false;
    }

    if (event->device != RE::INPUT_DEVICE::kKeyboard) {
        return false;
    }

    auto* button = event->AsButtonEvent();
    if (!button) {
        return false;
    }

    const std::uint32_t keyCode = button->GetIDCode();

    // Change main Hold Hotkey
    if (Configuration::IsListeningForHotkey && button->IsDown()) {
        Configuration::Hotkey = keyCode;
        Configuration::IsListeningForHotkey = false;
        Configuration::Save();
        if (Configuration::Debug) {
            logger::info("Hold Hotkey changed to 0x{:X}", keyCode);
        }
        return true;
    }

    // Listening inside Add Key window
    if (Configuration::IsListeningForNewKey && button->IsDown()) {
        Configuration::TempNewKeyCode = keyCode;
        Configuration::IsListeningForNewKey = false;
        if (Configuration::Debug) {
            logger::info("Detected new key for suppression: 0x{:X}", keyCode);
        }
        return true;
    }

    // Hold Hotkey logic
    if (keyCode == Configuration::Hotkey) {
        if (button->IsDown()) {
            Configuration::IsBlocking = true;
            if (Configuration::Debug) {
                logger::debug("Blocking mode ON");
            }
        } else if (button->IsUp()) {
            Configuration::IsBlocking = false;
            if (Configuration::Debug) {
                logger::debug("Blocking mode OFF");
            }
        }
        return false;
    }

    // Suppress keys while holding
    if (Configuration::IsBlocking && button->IsDown()) {
        if (Configuration::IsKeySuppressed(keyCode)) {
            if (Configuration::Debug) {
                logger::debug("Suppressed key 0x{:X}", keyCode);
            }
            return true;
        }
    }

    return false;
}