# Referee Watch

Professional Soccer Referee Smartwatch Application for ESP32-S3 AMOLED Devices

---

## Overview

Referee Watch is a custom-built smartwatch operating system and soccer match management application designed specifically for referees.

The project targets ESP32-S3 AMOLED wearable hardware including:

* LILYGO T-Watch Ultra ESP32-S3 LoRa
* Waveshare ESP32-S3 AMOLED devices
* Future custom referee smartwatch hardware

The application prioritizes:

* Fast one-handed operation
* Outdoor sunlight visibility
* Minimal cognitive load
* Instant event logging
* Reliable offline operation
* Haptic and audio feedback
* Youth and adult soccer support

---

## MVP Philosophy

The MVP intentionally avoids complex dependencies such as:

* Online APIs
* Team roster synchronization
* Cloud-only workflows
* Internet requirements during matches

All event logging uses jersey numbers only for maximum speed and reliability.

---

## Core Features

## Match Management

* First Half / Second Half tracking
* Halftime countdown
* Match timer
* Stoppage time
* Pause / Resume support
* Match auto-save
* Resume interrupted matches
* Fulltime match summary

---

## Event Logging

Quick event entry using jersey numbers only:

* Goals
* Yellow Cards
* Red Cards
* Substitutions

No player roster required for MVP.

---

## Card Reason Tracking

## Yellow Card Reasons

* Unsporting Behaviour
* Dissent
* Persistent Offences
* Delaying Restart
* Failure to Respect Required Distance
* Entering/Leaving Without Permission
* Entering Referee Review Area
* Excessive TV Signal
* Other

## Red Card Reasons

* Serious Foul Play
* Violent Conduct
* Biting or Spitting
* DOGSO Handball
* DOGSO Foul
* Offensive or Abusive Language
* Second Yellow
* Other

---

## Smartwatch UX Design

The interface is inspired by:

* Apple Watch Ultra
* Garmin sports watches
* AMOLED-first smartwatch UI systems

Design goals:

* Large touch targets
* Minimal taps
* High contrast
* Dark AMOLED optimized interface
* Fast operation while running
* Glanceable information under 1 second

---

## Hardware Features

## Current Hardware Support

* ESP32-S3
* AMOLED display
* DRV2605 haptic motor
* MAX98357A audio amplifier
* SD card storage
* AXP2101 battery management

## Planned Hardware Support

* GPS analytics
* LoRa communication
* BLE synchronization
* WiFi sync/export

---

## Architecture

The project is intentionally separated into layers.

## Core Layer (`src/core/`)

Pure application logic only.

No:

* LVGL
* display code
* hardware dependencies

Contains:

* MatchEngine
* DatabaseManager
* SettingsManager
* AppController
* EventFormatter

---

## Hardware Layer (`src/hardware/`)

Hardware abstraction for:

* Battery
* Haptics
* Audio
* GPS
* Display

All hardware-specific logic belongs here.

---

## UI Layer (`src/ui/`)

Contains all LVGL smartwatch UI code.

Includes:

* Screens
* Components
* Themes
* Styles

The UI layer must never contain match logic.

---

## Connectivity Layer (`src/connectivity/`)

Future networking systems:

* BLE
* WiFi
* LoRa
* Apple companion sync

---

## Utilities Layer (`src/utils/`)

Reusable helper systems:

* Logging
* Time formatting
* String utilities

---

## Current Core Systems

## MatchEngine

Handles:

* Match state machine
* Timer lifecycle
* Halftime logic
* Stoppage time
* Event tracking
* Auto-save
* Match progression

---

## DatabaseManager

Handles:

* SD card persistence
* JSON match storage
* Settings storage
* Backup recovery
* Schema versioning
* Safe temp-file writes

---

## HapticManager

DRV2605-based vibration system:

* Goals
* Cards
* Match alerts
* Confirmation feedback

---

## SoundManager

MAX98357A I2S audio alerts:

* Goal tones
* Warning tones
* Match alerts
* Confirmation sounds

---

## BatteryManager

AXP2101 PMU integration:

* Battery percentage
* Charging state
* Critical battery detection
* Emergency backup saves

---

## AppController

Central orchestration layer connecting:

* Match engine
* Database
* Settings
* Haptics
* Audio
* Battery systems

---

## Match Workflow

1. Referee opens application
2. Creates a new match
3. Selects:

   * Half duration
   * Halftime break duration
4. Enters:

   * Home team name
   * Away team name
   * Team jersey colors
5. Match begins
6. Referee logs events during play
7. Match summary generated at fulltime

---

## Match Duration Presets

Quick-select presets:

* 20 minute halves
* 25 minute halves
* 30 minute halves
* 35 minute halves
* 40 minute halves
* 45 minute halves

Custom half duration is also supported.

---

## Project Structure

```text
# Project Structure

```text
referee-watch/
├── platformio.ini
├── README.md
├── .gitignore
├── include/
│   ├── BoardConfig.h
│   ├── AppConfig.h
│   ├── Theme.h
│   └── Constants.h
├── src/
│   ├── main.cpp
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
│   ├── ui/
│   │   ├── UIManager.h
│   │   ├── UIManager.cpp
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
│   │   ├── components/
│   │   │   ├── ScoreBar.cpp
│   │   │   ├── TimerWidget.cpp
│   │   │   ├── TeamColorBadge.cpp
│   │   │   ├── CardEventTile.cpp
│   │   │   └── BatteryWidget.cpp
│   │   └── styles/
│   │       ├── Colors.h
│   │       ├── Fonts.h
│   │       └── Styles.cpp
│   ├── connectivity/
│   │   ├── BLEManager.h
│   │   ├── BLEManager.cpp
│   │   ├── WifiManager.h
│   │   ├── WifiManager.cpp
│   │   ├── LoRaManager.h
│   │   └── LoRaManager.cpp
│   └── utils/
│       ├── Logger.h
│       ├── Logger.cpp
│       ├── TimeUtils.h
│       ├── TimeUtils.cpp
│       ├── StringUtils.h
│       └── StringUtils.cpp
├── data/
│   └── sample_matches/
├── test/
│   ├── test_match_engine/
│   ├── test_database/
│   └── test_events/
└── lib/
```

## Development Environment

## Recommended Tools

* VSCode
* PlatformIO
* Git

---

## Required VSCode Extensions

* PlatformIO IDE
* C/C++
* Error Lens
* GitLens

---

## PlatformIO Configuration

## Example `platformio.ini`

```ini
[env:twatch_ultra]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino

monitor_speed = 115200
upload_speed = 921600

build_flags =
    -DCORE_DEBUG_LEVEL=3
    -DBOARD_HAS_PSRAM
    -DARDUINO_USB_CDC_ON_BOOT=1

board_build.partitions = huge_app.csv

lib_deps =
    bblanchon/ArduinoJson
    adafruit/Adafruit DRV2605 Library
    lewisxhe/XPowersLib
```

---

## Building the Project

## Initialize PlatformIO

```bash
pio run
```

## Upload Firmware

```bash
pio run --target upload
```

## Open Serial Monitor

```bash
pio device monitor
```

---

## Git Setup

## Initialize Git

```bash
git init
git add .
git commit -m "Initial project setup"
```

---

## Current Status

## Completed

* MatchEngine
* DatabaseManager
* SettingsManager
* HapticManager
* BatteryManager
* SoundManager
* AppController
* Utility systems

## In Progress

* LVGL UI integration
* Display driver
* Screen workflow implementation

## Planned

* BLE synchronization
* Apple companion app
* GPS analytics
* LoRa referee communication
* Match cloud sync

---

## Development Priorities

Current priority order:

1. Complete core logic
2. Complete hardware abstraction
3. Verify SD persistence
4. Verify haptics/audio/battery systems
5. Implement LVGL UI
6. Add connectivity systems
7. Add companion app support

---

## Design Philosophy

This project prioritizes:

* Reliability over complexity
* Speed under pressure
* Offline-first operation
* Minimal user interaction
* Referee-focused workflows
* Clear separation of concerns

The goal is to create a professional-grade smartwatch referee tool purpose-built for real match environments.

---

## License

MIT License

---

## Author

AJ Carbajal

Professional soccer referee, mortgage professional, electrician, and embedded systems developer.
