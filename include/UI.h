#pragma once
#include "Configuration.h"
#include "SKSEMenuFramework.h"

namespace UI {
    void Register();

    void __stdcall RenderSettings();
    void __stdcall RenderAddWindow();  // NEW
    void __stdcall RenderOverlay();
    bool __stdcall OnInput(RE::InputEvent* event);

    // Window handle
    inline MENU_WINDOW AddKeyWindow = nullptr;
}