#pragma once
#include "pch.h"
#include "Utils.h"
#include "Looting.h"

extern bool bReady;
extern bool bFirstDied;
extern bool bFirstPLayer;

class GameMode {
private:
	static inline TArray<UFortAbilitySet*> AbilitySets;
	static inline uint8_t CurrentTeam = 3;
	static inline uint8_t PlayersOnCurTeam = 0;

	DefUHookOgRet(bool, ReadyToStartMatch);
	static APawn* SpawnDefaultPawnFor(UObject*, FFrame&, APawn**);
	DefHookOg(EFortTeam, PickTeam, AFortGameModeAthena*, uint8_t, AFortPlayerControllerAthena*);
	DefUHookOg(HandleStartingNewPlayer);
	DefUHookOg(OnAircraftExitedDropZone);
	DefUHookOg(OnAircraftEnteredDropZone);
	DefUHookOg(HandleMatchHasEnded);

public:
	static bool IsFirstPlayer;
	static void ResetState();

	InitHooks;
};