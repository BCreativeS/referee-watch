# Copilot Instructions — Referee Watch

PlatformIO + Arduino C++17 project for an ESP32-S3 soccer referee smartwatch.

---

## Project Summary

This is a **logic-first embedded application**. Core match logic, storage, and hardware
drivers are complete. LVGL UI work has not started. Do not generate UI code unless
explicitly asked.

**Primary target board:** LILYGO T-Watch Ultra (ESP32-S3)
**Framework:** Arduino (via PlatformIO)
**C++ standard:** C++17 (`-std=gnu++17` in `platformio.ini`)

---

## What Is Already Implemented

Do not re-implement or stub these — they exist and compile:

- `AppController` — top-level orchestration singleton
- `MatchEngine` — match state machine, timer, events, auto-save
- `DatabaseManager` — SD card JSON storage (matches + settings)
- `SettingsManager` — persistent app defaults
- `EventFormatter` — static text formatting helpers
- `HapticManager` — DRV2605 vibration driver (wired into MatchEngine)
- `BatteryManager` — AXP2101 PMU driver (wired into AppController)
- `SoundManager` — MAX98357A I2S audio driver (implemented, **not yet wired into AppController**)
- `LogicTestSuite` (in `test/`) — repeatable pre-LVGL logic tests

## What Is Not Yet Implemented

Do not generate stubs or placeholders for these unless explicitly asked:

- `UIManager` and all LVGL screens / components / styles
- `GPSManager`, `BLEManager`, `LoRaManager`
- Utility helpers (`Logger`, `TimeUtils`, `StringUtils`)

---

## Architecture Rules

### Folder responsibilities

| Folder | Contains |
| --- | --- |
| `include/` | Shared hardware config only (`BoardConfig.h`) |
| `src/core/` | App logic — no hardware drivers, no LVGL |
| `src/hardware/` | Hardware driver managers only |
| `src/ui/` | LVGL screens and components only |
| `src/connectivity/` | BLE, WiFi, LoRa — future |
| `test/` | Logic test suite only |

### Hard rules

- **Never put LVGL code inside `src/core/`.** Core logic must run without a display.
- **Never hardcode GPIO pin numbers outside `include/BoardConfig.h`.** Always use `BoardConfig::PIN_NAME`.
- **Never instantiate a manager directly.** Always use `Manager::getInstance()`.
- **Never call `MatchEngine` or `DatabaseManager` directly from UI code.** Always go through `AppController`.
- **Never use `DynamicJsonDocument`.** This project uses ArduinoJson **v7** (`^7.0.4`). Use `JsonDocument` with no capacity argument.
- **Never use `createNestedObject()` or `createNestedArray()`.** Use `doc["key"].to<JsonObject>()` and `doc["key"].to<JsonArray>()`. For array elements use `arr.add<JsonObject>()`.
- **Never leave a declared method without an implementation.** If a method is added to a `.h`, it must exist in the `.cpp`.
- **Never add hardware pin TODOs inside source files.** Pin verification notes belong only in `include/BoardConfig.h`.
- **Never use magic strings for event types, team sides, or card reasons.** Use the defined namespaces: `EventType::GOAL`, `TeamSide::HOME`, `YellowCardReason::DISSENT`, etc.

---

## Singleton Pattern

Every manager follows this exact pattern. Do not deviate:

```cpp
// In .h
class FooManager {
public:
    static FooManager& getInstance();
    FooManager(const FooManager&) = delete;
    FooManager& operator=(const FooManager&) = delete;
private:
    FooManager();
};

// In .cpp
FooManager& FooManager::getInstance() {
    static FooManager instance;
    return instance;
}
```

---

## Data Models

Use these exact types — do not redefine them:

```cpp
MatchConfig   // halfMinutes, totalMinutes, halfTimeBreakMinutes, stoppageTime...
Team          // name, jerseyColor, score, yellowCards, redCards, substitutions
Event         // id, half, minute, type, team, jerseyNumber, playerOut, playerIn, reason
Timeline      // matchStarted, half1Ended, half2Started, matchEnded (ISO 8601 strings)
MatchState    // IDLE, SETUP, READY, FIRST_HALF, HALFTIME, SECOND_HALF, PAUSED, FULLTIME
```

---

## Match Duration Presets

Support these exact values (defined in `MatchDurationPreset` namespace):

```
20, 25, 30, 35, 40, 45 minutes (and CUSTOM = 0 for arbitrary duration)
```

Custom duration range: 1–60 minutes per half.

---

## Event Types and Reasons

Event types (`EventType` namespace): `goal` · `yellow` · `red` · `substitution`

Team sides (`TeamSide` namespace): `home` · `away`

Yellow card reasons (`YellowCardReason` namespace):
`unsporting_behaviour` · `dissent` · `persistent_offences` · `delaying_restart` ·
`required_distance` · `enter_leave_without_permission` · `entering_rra` ·
`excessive_tv_signal` · `other`

Red card reasons (`RedCardReason` namespace):
`serious_foul_play` · `violent_conduct` · `biting_or_spitting` · `dogso_handball` ·
`dogso_foul` · `offensive_language` · `second_caution` · `other`

---

## Storage Rules

- Matches saved to `/matches/<matchId>.json`
- In-progress match saved to `/matches/in_progress.json`
- Settings saved to `/config/settings.json`
- Every write uses the temp-file-then-rename pattern via `writeJsonSafely()`
- Every JSON file includes a `schemaVersion` field (`CURRENT_SCHEMA_VERSION = 1`)
- Match saves after every logged event (`forceSave()` called from `addEvent()`)

---

## ArduinoJson v7 API

This project uses `bblanchon/ArduinoJson @ ^7.0.4`. The v6 API was removed in v7.

| v6 (do NOT use) | v7 (correct) |
| --- | --- |
| `DynamicJsonDocument doc(size)` | `JsonDocument doc` |
| `doc.createNestedObject("key")` | `doc["key"].to<JsonObject>()` |
| `doc.createNestedArray("key")` | `doc["key"].to<JsonArray>()` |
| `arr.createNestedObject()` | `arr.add<JsonObject>()` |
| `StaticJsonDocument<N> doc` | `JsonDocument doc` |

---

## EventFormatter Usage

`EventFormatter` is a **static utility class** — never instantiate it:

```cpp
String text     = EventFormatter::formatEvent(event);
String shortTxt = EventFormatter::formatEventShort(event);
String score    = EventFormatter::formatScore(home, away);     // "2 - 1"
String summary  = EventFormatter::formatMatchSummary(home, away);
```

---

## HapticManager Patterns

Use `HapticPattern` enum values. Never call with a raw integer:

```cpp
hapticManager->play(HapticPattern::GOAL);
hapticManager->play(HapticPattern::YELLOW_CARD);
hapticManager->play(HapticPattern::CONFIRM);
hapticManager->play(HapticPattern::ERROR);
```

---

## AppController Public API

`main.cpp` and future UI code must only call `AppController`. Never bypass it
to call `MatchEngine` or `DatabaseManager` directly (except inside `LogicTestSuite`
during the pre-LVGL test phase):

```cpp
AppController& app = AppController::getInstance();

app.begin();
app.createNewMatch(halfPreset, halftimeMinutes, homeName, homeColor, awayName, awayColor);
app.startMatch();
app.addGoal(TeamSide::HOME, jerseyNumber);
app.addYellowCard(TeamSide::AWAY, jersey, YellowCardReason::DISSENT);
app.addRedCard(TeamSide::HOME, jersey, RedCardReason::DOGSO_FOUL);
app.addSubstitution(TeamSide::AWAY, playerOut, playerIn);
app.addStoppageMinutes(2);
app.update();  // call every loop iteration
```

---

## Coding Style

- Arduino C++17. Prefer readable code over clever code.
- Validate all inputs at the boundary (in `AppController` and `MatchEngine`).
- Return `bool` for all operations that can fail.
- Use `const String&` for string parameters. Use `String` for return values.
- Use `constexpr` for constants. Use namespaces instead of `#define` for string constants.
- Section dividers in `.cpp` files:

```cpp
// =====================================================
// SECTION NAME
// =====================================================
```

- Always include the matching `.h` as the first include in a `.cpp` file.
- Place `#pragma once` at the top of every header.
