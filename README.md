# Referee Watch
Professional Soccer Referee Smartwatch Application for ESP32-S3 AMOLED Watches

## Overview

Referee Watch is a custom-built smartwatch operating system and match management application designed specifically for soccer referees.

The project targets ESP32-S3 AMOLED wearable hardware such as the:

- LILYGO T-Watch Ultra ESP32-S3 LoRa
- Waveshare ESP32-S3 AMOLED devices
- Future custom referee smartwatch hardware

The application focuses on:

- fast one-handed operation
- outdoor sunlight visibility
- minimal cognitive load
- instant match event logging
- reliable offline operation
- vibration + audio feedback
- youth and adult soccer support

---

# Core Features

## Match Management

- First Half / Second Half tracking
- Halftime countdown
- Match timer
- Stoppage time
- Pause / Resume support
- Match auto-save
- Resume interrupted matches

---

## Event Logging

Quick event entry using jersey numbers only:

- Goals
- Yellow Cards
- Red Cards
- Substitutions

No player roster required for MVP.

---

## Card Reason Tracking

### Yellow Card Reasons

- Unsporting Behaviour
- Dissent
- Persistent Offences
- Delaying Restart
- Failure to Respect Distance
- Entering/Leaving Without Permission
- Entering Referee Review Area
- Excessive TV Signal

### Red Card Reasons

- Serious Foul Play
- Violent Conduct
- Biting or Spitting
- DOGSO Handball
- DOGSO Foul
- Offensive/Abusive Language
- Second Yellow

---

## Smartwatch UX Design

Designed with:

- AMOLED-first UI
- Apple Watch Ultra inspiration
- Garmin sports watch workflow
- large touch targets
- high contrast
- minimal taps
- dark mode optimized interface

---

## Hardware Features

### Supported Hardware

- ESP32-S3
- AMOLED display
- DRV2605 haptic motor
- MAX98357A audio amplifier
- SD card storage
- AXP2101 battery management
- LoRa (future)
- GPS (future)

---

# Current Architecture

## Core Systems

### MatchEngine

Handles:

- timer state machine
- match lifecycle
- event tracking
- auto-save
- stoppage time
- halftime logic

---

### DatabaseManager

Handles:

- SD card persistence
- JSON match storage
- settings storage
- backup recovery
- schema versioning

---

### HapticManager

DRV2605-based vibration system:

- goals
- yellow cards
- red cards
- halftime/fulltime
- confirmation feedback

---

### SoundManager

I2S audio alerts through MAX98357A:

- goal tones
- warnings
- confirmations
- referee alerts

---

### BatteryManager

AXP2101 PMU integration:

- battery percentage
- charging state
- critical battery detection
- emergency match backup

---

### AppController

Main orchestration layer connecting:

- database
- match engine
- haptics
- audio
- settings
- battery management

---

# Match Workflow

1. Referee opens application
2. Creates new match
3. Selects:
   - half duration preset
   - halftime break
4. Enters:
   - home team name
   - away team name
   - jersey colors
5. Match begins
6. Referee logs events during play
7. Match summary generated at fulltime

---

# Match Duration Presets

Quick-select presets:

- 20 min
- 25 min
- 30 min
- 35 min
- 40 min
- 45 min

Custom duration also supported.

---

# Planned Features

## Phase 1
- Core match logic
- SD storage
- Haptics
- Audio alerts
- AMOLED UI
- LVGL integration

## Phase 2
- GPS analytics
- Distance tracking
- Heatmaps
- Sprint metrics

## Phase 3
- BLE phone sync
- Apple companion app
- Match import/export
- Cloud sync

## Phase 4
- LoRa assistant referee communication
- Multi-watch synchronization
- Advanced referee analytics

---

# Project Structure

```text
referee-watch/
│
├── platformio.ini
├── README.md
├── .gitignore
│
├── include/
│   ├── BoardConfig.h
│   ├── AppConfig.h
│   ├── Theme.h
│   └── Constants.h
│
├── src/
│   │
│   ├── main.cpp
│   │
│   ├── core/
│   │   ├── AppController.h
│   │   ├── AppController.cpp
│   │   ├── MatchEngine.h
│   │   ├── MatchEngine.cpp
│   │   ├── DatabaseManager.h
│   │   ├── DatabaseManager.cpp
│   │   ├── SettingsManager.h
│   │   ├── SettingsManager.cpp
│   │   ├── EventFormatter.h
│   │   ├── EventFormatter.cpp
│   │   ├── LogicTestSuite.h
│   │   └── LogicTestSuite.cpp
│   │
│   ├── hardware/
│   │   ├── HapticManager.h
│   │   ├── HapticManager.cpp
│   │   ├── BatteryManager.h
│   │   ├── BatteryManager.cpp
│   │   ├── SoundManager.h
│   │   ├── SoundManager.cpp
│   │   ├── GPSManager.h
│   │   ├── GPSManager.cpp
│   │   ├── DisplayManager.h
│   │   └── DisplayManager.cpp
│   │
│   ├── ui/
│   │   ├── UIManager.h
│   │   ├── UIManager.cpp
│   │   │
│   │   ├── screens/
│   │   │   ├── SplashScreen.cpp
│   │   │   ├── HomeScreen.cpp
│   │   │   ├── MatchSetupScreen.cpp
│   │   │   ├── TeamSetupScreen.cpp
│   │   │   ├── GoalScreen.cpp
│   │   │   ├── YellowCardScreen.cpp
│   │   │   ├── RedCardScreen.cpp
│   │   │   ├── SubstitutionScreen.cpp
│   │   │   ├── EventsScreen.cpp
│   │   │   ├── MatchMenuScreen.cpp
│   │   │   ├── SettingsScreen.cpp
│   │   │   └── MatchSummaryScreen.cpp
│   │   │
│   │   ├── components/
│   │   │   ├── ScoreBar.cpp
│   │   │   ├── TimerWidget.cpp
│   │   │   ├── TeamColorBadge.cpp
│   │   │   ├── CardEventTile.cpp
│   │   │   └── BatteryWidget.cpp
│   │   │
│   │   └── styles/
│   │       ├── Colors.h
│   │       ├── Fonts.h
│   │       └── Styles.cpp
│   │
│   ├── connectivity/
│   │   ├── BLEManager.h
│   │   ├── BLEManager.cpp
│   │   ├── WifiManager.h
│   │   ├── WifiManager.cpp
│   │   ├── LoRaManager.h
│   │   └── LoRaManager.cpp
│   │
│   └── utils/
│       ├── Logger.h
│       ├── Logger.cpp
│       ├── TimeUtils.h
│       ├── TimeUtils.cpp
│       ├── StringUtils.h
│       └── StringUtils.cpp
│
├── data/
│   └── sample_matches/
│
├── test/
│   ├── test_match_engine/
│   ├── test_database/
│   └── test_events/
│
└── lib/
