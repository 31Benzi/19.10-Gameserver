#include "pch.h"
#include "Creative.h"

#include "Inventory.h"
#include "Options.h"
#include "Utils.h"

void Creative::MakeNewCreativePlot(UObject* Context, FFrame& Stack)
{
    UFortCreativeRealEstatePlotItemDefinition PlotType;
    FString Locale;
    FString Title;

    Stack.StepCompiledIn(&PlotType);
    Stack.StepCompiledIn(&Locale);
    Stack.StepCompiledIn(&Title);
    Stack.IncrementCode();
    
    auto Controller = (AFortPlayerControllerAthena*)Context;
    if (!Controller) return;
    Controller->ClientBroadcastOnMakeNewCreativePlotFinished(true, UKismetTextLibrary::Conv_StringToText(L""));
}

void Creative::DestroyCreativePlot(AFortPlayerControllerAthena* Controller, FString& IslandId)
{
    if (!Controller) return;
    auto CreativeIslands = Controller->CreativeIslands;

    CreativeIslands[0].bIsDeleted = true;
    Controller->ClientBroadcastOnDestroyCreativePlotFinished(true, UKismetTextLibrary::Conv_StringToText(L""));
}

void Creative::DuplicateCreativePlot(AFortPlayerControllerAthena* Controller, FString& IslandId, FString& Locale, FString& Title)
{
    if (!Controller) return;
    //auto CreativeIslands = Controller->CreativeIslands;
    //auto Name = CreativeIslands[0].IslandName;

    //MakeNewIsland(Controller, UKismetTextLibrary::Conv_TextToString(Name), false);
    Controller->ClientBroadcastOnDuplicateCreativePlotFinished(true, UKismetTextLibrary::Conv_StringToText(L""));
}

void Creative::RestoreCreativePlot(AFortPlayerControllerAthena* Controller, FString& IslandId)
{
    if (!Controller) return;
    auto CreativeIslands = Controller->CreativeIslands;
    CreativeIslands[0].bIsDeleted = false; // scuffed....
    Controller->ClientBroadcastOnRestoreCreativePlotFinished(true, UKismetTextLibrary::Conv_StringToText(L""));
}

static void(*OnStopCallbackOG)(AFortProjectileBase* Base, FHitResult& Hit);
void Creative::OnStopCallback(AFortProjectileBase* Base, FHitResult& Hit)
{
    if (!Base) return OnStopCallbackOG(Base, Hit);

    std::cout << Base->GetFullName() << std::endl;

    return OnStopCallbackOG(Base, Hit);
}

void Creative::UpdateCreativePlotData(AFortPlayerControllerAthena* Controller, AFortVolume* VolumeToPublish, FCreativeIslandInfo& MyInfo)
{
    if (!Controller || !VolumeToPublish) return;
}

extern inline __int64 (*ShowPlaysetOG)(class UPlaysetLevelStreamComponent*) = decltype(ShowPlaysetOG)(Sarah::ImageBase + 0x67c1cac);
void Creative::ShowPlayset(UFortPlaysetItemDefinition* Playset, AFortVolume* Volume)
{
    auto LevelStreamComponent = (UPlaysetLevelStreamComponent*)Volume->GetComponentByClass(UPlaysetLevelStreamComponent::StaticClass());
    if (!LevelStreamComponent) return;
    LevelStreamComponent->SetPlayset(Playset);
    
    ShowPlaysetOG(LevelStreamComponent);
}

void Creative::ClientBroadcastOnUpdateCreativePlotName(AFortPlayerControllerAthena* Controller, bool bSuccess, FText& Reason)
{
    if (!Controller) return;
}

void Creative::ServerTeleportToPlaygroundLobbyIsland(AFortPlayerControllerAthena* Controller)
{
    if (!Controller || !bCreative) return;

    auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
    auto GameState = (AFortGameStateAthena*)GameMode->GameState;

    if (Controller->WarmupPlayerStart) Controller->GetPlayerPawn()->K2_TeleportTo(Controller->WarmupPlayerStart->K2_GetActorLocation(), Controller->GetPlayerPawn()->K2_GetActorRotation());
    else
    {
        AActor* Actor = GameMode->ChoosePlayerStart(Controller);
        Controller->GetPlayerPawn()->K2_TeleportTo(Actor->K2_GetActorLocation(), Actor->K2_GetActorRotation());
    }

    Controller->bIsCreativeModeEnabled = false;
    Controller->OnRep_IsCreativeModeEnabled();
    Controller->bBuildFree = false;
    Controller->bIsCreativeQuickbarEnabled = false;
    Controller->OnRep_IsCreativeQuickbarEnabled(false);
    Controller->CreativePlotLinkedVolume = nullptr;
    Controller->OnRep_CreativePlotLinkedVolume();

    // SCUFFED
    auto Num = Controller->WorldInventory->Inventory.ReplicatedEntries.Num();
    if (Num != 0) {
        Controller->WorldInventory->Inventory.ReplicatedEntries.ResetNum();
        Controller->WorldInventory->Inventory.ItemInstances.ResetNum();
    }
 
    Inventory::GiveItem(Controller, Controller->CosmeticLoadoutPC.Pickaxe->WeaponDefinition);
    for (auto& StartingItem : ((AFortGameModeAthena*)GameMode)->StartingItems)
    {
        if (StartingItem.Count && !StartingItem.Item->IsA<UFortSmartBuildingItemDefinition>())
        {
            Inventory::GiveItem(Controller, StartingItem.Item, StartingItem.Count);
        }
    }
}

void Creative::TeleportPlayerToLinkedVolume(AFortAthenaCreativePortal* Portal, FFrame& Stack)
{
    AFortPlayerPawnAthena* PlayerPawn = nullptr;
    bool bUseSpawnTags;

    Stack.StepCompiledIn(&PlayerPawn);
    Stack.StepCompiledIn(&bUseSpawnTags);
    Stack.IncrementCode();
    if (!PlayerPawn) return;

    auto Controller = (AFortPlayerControllerAthena*)PlayerPawn->Controller;
    if (!Controller) return;

    auto Volume = Portal->LinkedVolume;
    if (!Volume) return;

    auto PlayerState = (AFortPlayerStateAthena*)Controller->PlayerState;
    if (!PlayerState) return;

    // Permissions & Ownership
    auto LevelSaveComponent = (UFortLevelSaveComponent*)Volume->GetComponentByClass(UFortLevelSaveComponent::StaticClass());
    if (LevelSaveComponent)
    {
        LevelSaveComponent->AccountIdOfOwner = Portal->OwningPlayer;
        LevelSaveComponent->bIsLoaded = true;
        
        bool bIsOwner = Portal->OwningPlayer == PlayerState->UniqueId;
        LevelSaveComponent->PlotPermissions.Permission = bIsOwner ? EFortCreativePlotPermission::Public : EFortCreativePlotPermission::Private; 
        
        LevelSaveComponent->OnRep_PlotPermissionsChanged();
    }

    // Teleportation
    auto Location = Volume->K2_GetActorLocation();
    Location.Z = 10000;
    PlayerPawn->K2_TeleportTo(Location, FRotator());
    PlayerPawn->BeginSkydiving(false);

    // Tools & Resources
    static auto CreativePhone = Utils::FindObject<UFortWeaponItemDefinition>(L"/Game/Athena/Items/Weapons/Prototype/WID_CreativeTool.WID_CreativeTool");
    Inventory::GiveItem(Controller, CreativePhone, 1, 0, 1, true, false);
    Controller->ClientCreativePhoneCreated();

    auto Wood = UFortKismetLibrary::K2_GetResourceItemDefinition(EFortResourceType::Wood);
    auto Stone = UFortKismetLibrary::K2_GetResourceItemDefinition(EFortResourceType::Stone);
    auto Metal = UFortKismetLibrary::K2_GetResourceItemDefinition(EFortResourceType::Metal);

    Inventory::GiveItem(Controller, Wood, 999, 0, 1, false, false);
    Inventory::GiveItem(Controller, Stone, 999, 0, 1, false, false);
    Inventory::GiveItem(Controller, Metal, 999, 0, 1, false, true);

    // Player State
    Controller->bIsCreativeModeEnabled = true;
    Controller->OnRep_IsCreativeModeEnabled();

    Controller->bIsCreativeQuickbarEnabled = true;
    Controller->OnRep_IsCreativeQuickbarEnabled(false);

    Controller->bIsCreativeQuickmenuEnabled = true;
    Controller->bBuildFree = true;

    // Volume State
    Volume->VolumeState = ESpatialLoadingState::Ready; // 3
    Volume->OnRep_VolumeState();

    Controller->CreativePlotLinkedVolume = Volume;
    Controller->OnRep_CreativePlotLinkedVolume();

    Controller->OwnedPortal = Portal;
    
    // Ensure island info is set up without triggering multiple splash screens
    if (Controller->CreativeIslands.Num() == 0)
    {
        MakeNewIsland(Controller, L"My Creative Island", false);
    }

    SpawnSettingsMachine(Volume);

    auto Minigame = Volume->GetMinigame();
    if (Minigame)
    {
        Minigame->AddMinigamePlayer(PlayerState, true);
        Minigame->CurrentState = EFortMinigameState::InProgress;
        Minigame->OnRep_CurrentState();
        Minigame->StartMinigame(Controller);
    }
}

void Creative::UpdateCreativePlotName(UObject* Context, FFrame& Stack)
{
    FString Locale;
    FString Title;
    FString IslandId;

    Stack.StepCompiledIn(&IslandId);
    Stack.StepCompiledIn(&Locale);
    Stack.StepCompiledIn(&Title);
    Stack.IncrementCode();
    
    auto Controller = (AFortPlayerControllerAthena*)Context;
    if (!Controller) return;

    Controller->CreativeIslands[0].IslandName = UKismetTextLibrary::Conv_StringToText(Title);
    Controller->ClientBroadcastOnUpdateCreativePlotName(true, UKismetTextLibrary::Conv_StringToText(L""));
}

void Creative::UpdateCreativeIslandDescriptionTags(AFortPlayerControllerAthena* Controller, FString& IslandId, FString& Locale, TArray<class FString>& DescriptionTags)
{
    if (!Controller) return;
}

void Creative::ServerLoadPlotForPortal(AFortPlayerControllerAthena* Controller, AFortAthenaCreativePortal* Portal, FString& PlotItemId)
{
    if (!Controller) return;
    
    auto GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;
    if (!GameState) return;
}


void Creative::LoadIslands(AFortPlayerControllerAthena* Controller, FString Title)
{
    if (!Controller) return;
    MakeNewIsland(Controller, Title, false);
}

void Creative::MakeNewIsland(AFortPlayerControllerAthena* Controller, FString Title, bool Deleted)
{
    if (!Controller) return;
    FCreativeIslandData Data;
    Data.bIsDeleted = Deleted;
    Data.PublishedIslandVersion = 1;
    Data.PublishedIslandCode = L""; 
    Data.IslandName = UKismetTextLibrary::Conv_StringToText(Title);
    Data.McpId = L"creative";
    Data.LastLoadedDate = UKismetMathLibrary::UtcNow();
    
    // Ensure CreatorName is not being set to anything that triggers the splash screen
    // Data.CreatorName = L""; 
    
    Controller->CreativeIslands.Add(Data);
    Controller->OnRep_CreativeIslands();
}

void Creative::SpawnSettingsMachine(AFortVolume* Volume)
{
    if (!Volume) return;

    if (AFortVolumeManager::GetSettingsActorForVolume(Volume)) return;

    auto Location = Volume->K2_GetActorLocation();
    auto Rotation = Volume->K2_GetActorRotation();

    auto SettingsMachineClass = AMinigameSettingsMachine_C::StaticClass();
    auto SettingsMachine = (AMinigameSettingsMachine_C*)Utils::SpawnActor(SettingsMachineClass, Location, Rotation);
    if (SettingsMachine)
    {
        SettingsMachine->SettingsVolume = Volume;
        SettingsMachine->OnRep_SettingsVolume();

        auto MinigameVolumeComponent = (UFortMinigameVolumeComponent*)Volume->GetComponentByClass(UFortMinigameVolumeComponent::StaticClass());
        if (MinigameVolumeComponent)
        {
            MinigameVolumeComponent->CurrentMinigameSettingsMachine = SettingsMachine;
        }
    }
}


void Creative::Hook()
{
    if (bCreative)
    {
        Utils::Hook<AFortProjectileBase>(uint32(0xed), OnStopCallback, OnStopCallbackOG);
        Utils::Patch<uint8>(ImageBase + 0x5F9CA4F, 0xeb);
        Utils::Patch<uint16>(ImageBase + 0x5F9888C, 0xe990);
    }
    Utils::ExecHook(L"/Script/FortniteGame.FortAthenaCreativePortal.TeleportPlayerToLinkedVolume", TeleportPlayerToLinkedVolume);
    Utils::ExecHook(L"/Script/FortniteGame.FortPlayerControllerAthena.ServerTeleportToPlaygroundLobbyIsland", ServerTeleportToPlaygroundLobbyIsland);
    Utils::ExecHook(L"/Script/FortniteGame.FortPlayerControllerAthena.MakeNewCreativePlot", MakeNewCreativePlot);
    Utils::ExecHook(L"/Script/FortniteGame.FortPlayerControllerAthena.UpdateCreativePlotName", UpdateCreativePlotName);
    Utils::ExecHook(L"/Script/FortniteGame.FortPlayerControllerAthena.DestroyCreativePlot", DestroyCreativePlot);
    Utils::ExecHook(L"/Script/FortniteGame.FortPlayerControllerAthena.DuplicateCreativePlot", DuplicateCreativePlot);
    Utils::ExecHook(L"/Script/FortniteGame.FortPlayerControllerAthena.RestoreCreativePlot", RestoreCreativePlot);
}
