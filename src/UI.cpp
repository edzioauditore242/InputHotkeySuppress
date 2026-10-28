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

    // Create windows (start closed)
    AddKeyWindow = SKSEMenuFramework::AddWindow(RenderAddWindow, true);
    if (AddKeyWindow) AddKeyWindow->IsOpen = false;

    EditKeyWindow = SKSEMenuFramework::AddWindow(RenderEditWindow, true);
    if (EditKeyWindow) EditKeyWindow->IsOpen = false;

    DeleteConfirmWindow = SKSEMenuFramework::AddWindow(RenderDeleteConfirm, true);
    if (DeleteConfirmWindow) DeleteConfirmWindow->IsOpen = false;

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

    // Show both hex and name
    char hotkeyText[128];
    sprintf_s(hotkeyText, "Current Hotkey: 0x%X  (%s)", Configuration::Hotkey, Configuration::GetKeyName(Configuration::Hotkey).c_str());
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

    // ===== TABLE =====
    if (ImGuiMCP::BeginTable("SuppressedTable", 4, ImGuiMCP::ImGuiTableFlags_Borders | ImGuiMCP::ImGuiTableFlags_RowBg | ImGuiMCP::ImGuiTableFlags_Resizable)) {
        ImGuiMCP::TableSetupColumn("Name", ImGuiMCP::ImGuiTableColumnFlags_WidthStretch);
        ImGuiMCP::TableSetupColumn("Hex", ImGuiMCP::ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGuiMCP::TableSetupColumn("Key", ImGuiMCP::ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGuiMCP::TableSetupColumn("Action", ImGuiMCP::ImGuiTableColumnFlags_WidthFixed, 140.0f);
        ImGuiMCP::TableHeadersRow();

        for (size_t i = 0; i < Configuration::SuppressedList.size(); ++i) {
            auto& entry = Configuration::SuppressedList[i];
            ImGuiMCP::PushID(static_cast<int>(i));

            ImGuiMCP::TableNextRow();

            // Name
            ImGuiMCP::TableSetColumnIndex(0);
            ImGuiMCP::Text("%s", entry.name.c_str());

            // Hex
            ImGuiMCP::TableSetColumnIndex(1);
            ImGuiMCP::Text("0x%X", entry.keyCode);

            // Key Name
            ImGuiMCP::TableSetColumnIndex(2);
            ImGuiMCP::Text("%s", Configuration::GetKeyName(entry.keyCode).c_str());

            // Actions
            ImGuiMCP::TableSetColumnIndex(3);

            if (ImGuiMCP::Button("Edit")) {
                Configuration::EditIndex = i;
                Configuration::TempEditName = entry.name;
                Configuration::TempEditKeyCode = entry.keyCode;
                Configuration::IsListeningForEditKey = false;
                if (EditKeyWindow) EditKeyWindow->IsOpen = true;
            }

            ImGuiMCP::SameLine();

            if (ImGuiMCP::Button("Remove")) {
                Configuration::DeleteIndex = i;
                if (DeleteConfirmWindow) DeleteConfirmWindow->IsOpen = true;
            }

            ImGuiMCP::PopID();
        }

        ImGuiMCP::EndTable();
    }

    ImGuiMCP::Spacing();
    if (ImGuiMCP::Button(Translation::Get("ADD_KEY_BUTTON"))) {
        Configuration::TempNewName.clear();
        Configuration::TempNewKeyCode = 0;
        Configuration::IsListeningForNewKey = false;
        if (AddKeyWindow) AddKeyWindow->IsOpen = true;
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
    if (!AddKeyWindow || !AddKeyWindow->IsOpen) return;

    auto viewport = ImGuiMCP::GetMainViewport();
    ImGuiMCP::SetNextWindowPos(ImGuiMCP::ImVec2{viewport->Size.x * 0.5f, viewport->Size.y * 0.5f}, ImGuiMCP::ImGuiCond_Appearing, ImGuiMCP::ImVec2{0.5f, 0.5f});

    ImGuiMCP::Begin(Translation::Get("ADD_WINDOW_TITLE"), nullptr, ImGuiMCP::ImGuiWindowFlags_NoCollapse | ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize);

    static char nameBuffer[64] = "";
    ImGuiMCP::InputText(Translation::Get("NAME"), nameBuffer, sizeof(nameBuffer));

    ImGuiMCP::Spacing();

    if (Configuration::IsListeningForNewKey) {
        ImGuiMCP::TextColored(ImGuiMCP::ImVec4(1, 1, 0, 1), Translation::Get("LISTENING_NEW_KEY"));
    } else {
        if (ImGuiMCP::Button(Translation::Get("LISTEN_FOR_KEY"))) {
            Configuration::IsListeningForNewKey = true;
            Configuration::TempNewKeyCode = 0;
        }
    }

    ImGuiMCP::Spacing();

    if (Configuration::TempNewKeyCode != 0) {
        char buf[64];
        sprintf_s(buf, "Detected: 0x%X  (%s)", Configuration::TempNewKeyCode, Configuration::GetKeyName(Configuration::TempNewKeyCode).c_str());
        ImGuiMCP::TextColored(ImGuiMCP::ImVec4(0.3f, 1.0f, 0.3f, 1.0f), buf);
    } else {
        ImGuiMCP::TextDisabled(Translation::Get("NO_KEY_YET"));
    }

    ImGuiMCP::Spacing();
    ImGuiMCP::Separator();
    ImGuiMCP::Spacing();

    if (ImGuiMCP::Button(Translation::Get("SAVE"))) {
        if (strlen(nameBuffer) > 0 && Configuration::TempNewKeyCode != 0) {
            if (!Configuration::IsKeySuppressed(Configuration::TempNewKeyCode)) {
                Configuration::SuppressedEntry entry;
                entry.name = nameBuffer;
                entry.keyCode = Configuration::TempNewKeyCode;
                Configuration::SuppressedList.push_back(entry);
                Configuration::Save();
            }
        }
        nameBuffer[0] = '\0';
        Configuration::TempNewKeyCode = 0;
        Configuration::IsListeningForNewKey = false;
        AddKeyWindow->IsOpen = false;
    }

    ImGuiMCP::SameLine();
    if (ImGuiMCP::Button(Translation::Get("CANCEL"))) {
        nameBuffer[0] = '\0';
        Configuration::TempNewKeyCode = 0;
        Configuration::IsListeningForNewKey = false;
        AddKeyWindow->IsOpen = false;
    }

    ImGuiMCP::End();
}

// =========================================================
// EDIT KEY WINDOW
// =========================================================
void __stdcall UI::RenderEditWindow() {
    if (!EditKeyWindow || !EditKeyWindow->IsOpen) return;

    auto viewport = ImGuiMCP::GetMainViewport();
    ImGuiMCP::SetNextWindowPos(ImGuiMCP::ImVec2{viewport->Size.x * 0.5f, viewport->Size.y * 0.5f}, ImGuiMCP::ImGuiCond_Appearing, ImGuiMCP::ImVec2{0.5f, 0.5f});

    ImGuiMCP::Begin("Edit Suppression Key", nullptr, ImGuiMCP::ImGuiWindowFlags_NoCollapse | ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize);

    static char nameBuffer[64] = "";
    static bool initialized = false;
    if (!initialized) {
        strcpy_s(nameBuffer, Configuration::TempEditName.c_str());
        initialized = true;
    }

    ImGuiMCP::InputText(Translation::Get("NAME"), nameBuffer, sizeof(nameBuffer));

    ImGuiMCP::Spacing();

    if (Configuration::IsListeningForEditKey) {
        ImGuiMCP::TextColored(ImGuiMCP::ImVec4(1, 1, 0, 1), Translation::Get("LISTENING_NEW_KEY"));
    } else {
        if (ImGuiMCP::Button(Translation::Get("LISTEN_FOR_KEY"))) {
            Configuration::IsListeningForEditKey = true;
        }
    }

    ImGuiMCP::Spacing();

    char buf[64];
    sprintf_s(buf, "Current: 0x%X  (%s)", Configuration::TempEditKeyCode, Configuration::GetKeyName(Configuration::TempEditKeyCode).c_str());
    ImGuiMCP::Text(buf);

    ImGuiMCP::Spacing();
    ImGuiMCP::Separator();
    ImGuiMCP::Spacing();

    if (ImGuiMCP::Button(Translation::Get("SAVE"))) {
        if (strlen(nameBuffer) > 0 && Configuration::EditIndex < Configuration::SuppressedList.size()) {
            auto& entry = Configuration::SuppressedList[Configuration::EditIndex];
            entry.name = nameBuffer;
            entry.keyCode = Configuration::TempEditKeyCode;
            Configuration::Save();
        }
        initialized = false;
        Configuration::IsListeningForEditKey = false;
        EditKeyWindow->IsOpen = false;
    }

    ImGuiMCP::SameLine();
    if (ImGuiMCP::Button(Translation::Get("CANCEL"))) {
        initialized = false;
        Configuration::IsListeningForEditKey = false;
        EditKeyWindow->IsOpen = false;
    }

    ImGuiMCP::End();
}

// =========================================================
// DELETE CONFIRMATION WINDOW
// =========================================================
void __stdcall UI::RenderDeleteConfirm() {
    if (!DeleteConfirmWindow || !DeleteConfirmWindow->IsOpen) return;

    auto viewport = ImGuiMCP::GetMainViewport();
    ImGuiMCP::SetNextWindowPos(ImGuiMCP::ImVec2{viewport->Size.x * 0.5f, viewport->Size.y * 0.5f}, ImGuiMCP::ImGuiCond_Appearing, ImGuiMCP::ImVec2{0.5f, 0.5f});

    ImGuiMCP::Begin("Confirm Delete", nullptr, ImGuiMCP::ImGuiWindowFlags_NoCollapse | ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize);

    if (Configuration::DeleteIndex < Configuration::SuppressedList.size()) {
        auto& entry = Configuration::SuppressedList[Configuration::DeleteIndex];
        ImGuiMCP::Text("Delete this key?");
        ImGuiMCP::Spacing();
        ImGuiMCP::Text("%s  (0x%X - %s)", entry.name.c_str(), entry.keyCode, Configuration::GetKeyName(entry.keyCode).c_str());
    }

    ImGuiMCP::Spacing();
    ImGuiMCP::Separator();
    ImGuiMCP::Spacing();

    if (ImGuiMCP::Button("Yes, Delete")) {
        if (Configuration::DeleteIndex < Configuration::SuppressedList.size()) {
            Configuration::SuppressedList.erase(Configuration::SuppressedList.begin() + Configuration::DeleteIndex);
            Configuration::Save();
        }
        DeleteConfirmWindow->IsOpen = false;
    }

    ImGuiMCP::SameLine();
    if (ImGuiMCP::Button("Cancel")) {
        DeleteConfirmWindow->IsOpen = false;
    }

    ImGuiMCP::End();
}

// =========================================================
// STATUS OVERLAY
// =========================================================
void __stdcall UI::RenderOverlay() {
    if (!Configuration::ShowStatusOverlay || !Configuration::Enabled) return;
    if (!Configuration::IsBlocking) return;
    if (SKSEMenuFramework::IsAnyBlockingWindowOpened()) return;

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
    if (!Configuration::Enabled || !event) return false;
    if (event->device != RE::INPUT_DEVICE::kKeyboard) return false;

    auto* button = event->AsButtonEvent();
    if (!button) return false;

    const std::uint32_t keyCode = button->GetIDCode();

    // Change main Hold Hotkey
    if (Configuration::IsListeningForHotkey && button->IsDown()) {
        Configuration::Hotkey = keyCode;
        Configuration::IsListeningForHotkey = false;
        Configuration::Save();
        return true;
    }

    // Listening in Add window
    if (Configuration::IsListeningForNewKey && button->IsDown()) {
        Configuration::TempNewKeyCode = keyCode;
        Configuration::IsListeningForNewKey = false;
        return true;
    }

    // Listening in Edit window
    if (Configuration::IsListeningForEditKey && button->IsDown()) {
        Configuration::TempEditKeyCode = keyCode;
        Configuration::IsListeningForEditKey = false;
        return true;
    }

    // Hold Hotkey logic
    if (keyCode == Configuration::Hotkey) {
        if (button->IsDown()) {
            Configuration::IsBlocking = true;
        } else if (button->IsUp()) {
            Configuration::IsBlocking = false;
        }
        return false;
    }

    // Suppress keys while holding
    if (Configuration::IsBlocking && button->IsDown()) {
        if (Configuration::IsKeySuppressed(keyCode)) {
            return true;
        }
    }

    return false;
}