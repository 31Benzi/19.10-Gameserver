#include "pch.h"

#include "Options.h"
#include "Utils.h"
#include "UI.h"
#include "GameMode.h"
#include "Misc.h"

DefHookOg(void, Test, UAthenaNavSystem*, UWorld*, char);

void Test(UAthenaNavSystem* NavSys, UWorld* World, char a3)
{
    for (auto& Agent : NavSys->SupportedAgents)
    {
        if (Agent.NavDataClass.Get())
        {
            Log(L"NavCl: %s", Agent.NavDataClass.Get()->Name.GetRawWString().c_str());
            Log(L"NavClPath: %s", Agent.NavDataClass.ObjectID.AssetPathName.GetRawWString().c_str());
        }
        else
        {
            Log(L"[null]");
        }
        if (Agent.NavigationDataClass.Get())
            Log(L"NavCl2: %s", Agent.NavigationDataClass.Get()->Name.GetRawWString().c_str());
    }
    Log(L"Enabled: %d", NavSys->SupportedAgentsMask.bSupportsAgent0);
    Log(L"Enabled: %d", NavSys->SupportedAgentsMask.bSupportsAgent1);
    Log(L"Enabled: %d", NavSys->SupportedAgentsMask.bSupportsAgent2);
    Log(L"Enabled: %d", NavSys->SupportedAgentsMask.bSupportsAgent3);
    Log(L"Enabled: %d", NavSys->SupportedAgentsMask.bSupportsAgent4);
    Log(L"Enabled: %d", NavSys->SupportedAgentsMask.bSupportsAgent5);
    Log(L"Enabled: %d", NavSys->SupportedAgentsMask.bSupportsAgent6);
    Log(L"Enabled: %d", NavSys->SupportedAgentsMask.bSupportsAgent7);
    while (true) {}
    return TestOG(NavSys, World, a3);
}

static void RestartServer() {
    UI::AddLog(L"Closing map...");
    UI::SetStatus(L"\u25cf  RESTARTING...");

    Sleep(1500);
    Misc::bPendingRestart = true;

    Sleep(8000);

    bServerRestarting = false;
    UI::SetStatus(L"\u25cf  READY");
    UI::AddLog(L"Server map reloaded successfully.");
}

void Main() {
    UI::Create();

    LogCategory = FName(L"LogGameserver");

    UI::SetRestartCallback(RestartServer);

    Sleep(5000);

    MH_Initialize();
    for (auto& HookFunc : _HookFuncs) HookFunc();
    MH_EnableHook(MH_ALL_HOOKS);
    srand((uint32_t)time(0));

    *(bool*)(ImageBase + Sarah::Offsets::GIsClient) = false;

    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), bCreative ? L"open Creative_NoApollo_Terrain" : L"open Artemis_Terrain", nullptr);
}

BOOL APIENTRY DllMain(HMODULE Module, DWORD Reason, LPVOID Reserved)
{
    switch (Reason)
    {
    case 1: std::thread(Main).detach();
    case 0: break;
    }
    return 1;
}
