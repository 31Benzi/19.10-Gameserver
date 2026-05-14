## PLS STAR THE REPO

# 19.10 Gameserver Feature Comparison

Comparison between **Crystal Old** (older branch) and **Crystal New** (newer branch).

---

# Features Present in Crystal New / Missing in Crystal Old

---

## 1. GUI / UI System

Crystal New introduces a complete Win32 GUI system with:

- Server status display
- Runtime feature toggles
- Network configuration controls
- Launch and restart controls
- Live log viewer
- Persistent INI configuration saving/loading

Crystal Old only uses a standard console window.

---

## 2. Server Restart System

Crystal New adds a full restart infrastructure including:

- Automatic restart handling
- Restart callbacks
- Restart state tracking
- Match restart flow integration
- Graceful restart sequencing

Crystal Old has no restart system.

---

## 3. Automatic Match-End Handling

Crystal New supports automatic shutdown/restart handling when matches end.

Crystal Old does not implement any match-end restart logic.

---

## 4. Global State Reset System

Crystal New introduces proper runtime state cleanup for:

- Ability sets
- Team state
- Match flags
- Looting data
- Cached player destruction lists

Crystal Old lacks a reset pipeline, making restarts unsafe.

---

## 5. Thread-Safe HTTP / curl Initialization

Crystal New uses a thread-safe global curl initialization system.

Benefits include:

- safer multithreaded networking
- reduced initialization overhead
- improved runtime stability

Crystal Old initializes curl unsafely inside individual calls.

---

## 6. Non-Blocking Async API Requests

Crystal New adds asynchronous backend API requests.

Benefits:

- prevents game thread stalls
- improves matchmaking responsiveness
- avoids gameplay hitching during HTTP calls

Crystal Old uses blocking API requests.

---

## 7. Anti-Cheat Stability Improvements

Crystal New improves anti-cheat handling with:

- controller validation
- null safety checks
- safer disconnect handling
- reduced crash risk

Crystal Old performs unsafe direct access and forced kicks.

---

## 8. Hook Re-Entry Protection

Crystal New prevents duplicate hook registration during restarts.

Benefits:

- avoids trampoline corruption
- prevents recursive detours
- improves restart stability

Crystal Old can re-register hooks repeatedly.

---

## 9. Full Creative Mode Support

Crystal New significantly expands creative functionality with:

- plot permission handling
- creative inventory support
- creative phone support
- resource granting
- quickbar replication
- quickmenu replication
- minigame initialization
- island creation handling
- linked volume replication

Crystal Old only implements partial creative teleport behavior.

---

## 10. Creative Lobby Cleanup

Crystal New properly resets creative state when leaving islands.

This includes:

- disabling creative mode
- clearing build permissions
- resetting quickbars
- unlinking creative volumes
- replication cleanup

Crystal Old skips most cleanup logic.

---

## 11. Restart-Safe First Player Tracking

Crystal New moves first-player tracking into managed game state.

Benefits:

- restart-safe behavior
- proper state resetting
- cleaner player initialization

Crystal Old uses a static local variable that cannot reset correctly.

---

## 12. Graceful Empty-Server Shutdown

Crystal New adds proper shutdown sequencing when all players leave.

Benefits:

- cleaner resource cleanup
- proper logging
- safer process termination

Crystal Old forcefully terminates the process immediately.

---

# Summary Table

| Feature | Crystal Old | Crystal New |
|---|:---:|:---:|
| Win32 GUI Panel | No | Yes |
| Settings INI Save/Load | No | Yes |
| Automatic Restart System | No | Yes |
| Match-End Restart Handling | No | Yes |
| Runtime State Reset System | No | Yes |
| Thread-Safe curl Initialization | No | Yes |
| Async API Requests | No | Yes |
| Safe Anti-Cheat Validation | No | Yes |
| Hook Re-Entry Protection | No | Yes |
| Full Creative Support | Partial | Complete |
| Creative Lobby Cleanup | No | Yes |
| Restart-Safe First Player Tracking | No | Yes |
| Graceful Empty-Server Shutdown | No | Yes |

---

# Overall Assessment

Crystal Old functions as a basic single-session server implementation.

Crystal New expands the architecture with:

- restart support
- runtime management
- async networking
- safer hook systems
- full creative lifecycle support
- state cleanup systems
- operational GUI controls

The largest improvement is the transition from a one-time process model into a restart-capable managed server runtime.
