#include "Plugin.h"

#include "Translation.h"

void OnMessage(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        Configuration::Load();
        Translation::Load();
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);

    SetupLog();
    logger::info("Input Hotkey Suppression loaded");

    UI::Register();
    return true;
}