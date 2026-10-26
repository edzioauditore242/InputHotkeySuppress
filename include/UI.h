#pragma once
#include "Configuration.h"
#include "SKSEMenuFramework.h"

namespace UI {
    void Register();

    void __stdcall RenderSettings();
    void __stdcall RenderAddWindow();
    void __stdcall RenderEditWindow();
    void __stdcall RenderDeleteConfirm();
    void __stdcall RenderOverlay();
    bool __stdcall OnInput(RE::InputEvent* event);

    inline MENU_WINDOW AddKeyWindow = nullptr;
    inline MENU_WINDOW EditKeyWindow = nullptr;
    inline MENU_WINDOW DeleteConfirmWindow = nullptr;
}