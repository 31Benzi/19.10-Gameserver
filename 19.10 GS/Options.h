#pragma once
#include <string>
#include <atomic>

inline bool bDuos = false;
inline bool bLateGame = false;
inline bool bTournament = true;
inline bool bCreative = false;
inline bool bGameSessions = false;
inline bool bDev = true;

inline std::string IP = "26.25.203.160";
inline std::string bRegion = "EU";
inline std::string eventId = "";

inline int StartingCount = 0;

inline std::atomic<bool> bServerRestarting{ false };
