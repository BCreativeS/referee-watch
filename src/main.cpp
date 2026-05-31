#include <Arduino.h>

#include "BoardConfig.h"
#include "DatabaseManager.h"
#include "SettingsManager.h"
#include "HapticManager.h"
#include "MatchEngine.h"
#include "EventFormatter.h"
#include "AppController.h"

// =====================================================
// MAIN BOOTSTRAP TEST
// Referee Watch App Logic Stack Test
// =====================================================
//
// This file is for validating logic BEFORE LVGL.
//
// It tests:
// - App startup
// - Storage
// - Haptics
// - Match setup
// - Match start
// - Event logging
// - Timer callback
// - Event formatting
//
// =====================================================

AppController& app = AppController::getInstance();

bool testMatchCreated = false;
bool testEventsLogged = false;
unsigned long testStartMs = 0;

// =====================================================
// CALLBACKS
// =====================================================

void registerEngineCallbacks() {
    MatchEngine& engine = MatchEngine::getInstance();

    engine.onStateChange([&engine](MatchState oldState, MatchState newState) {
        Serial.printf(
            "[CALLBACK] State: %s -> %s\n",
            engine.getStateName(oldState).c_str(),
            engine.getStateName(newState).c_str()
        );
    });

    engine.onTimerTick([](int minute, int second, int half, int stoppageSeconds) {
        Serial.printf(
            "[CALLBACK] Timer | Half: %d | Time: %02d:%02d | Stoppage: %d sec\n",
            half,
            minute,
            second,
            stoppageSeconds
        );
    });

    engine.onEventLogged([](const Event& event) {
        String text = EventFormatter::formatEvent(event);
        Serial.printf("[CALLBACK] Event Logged: %s\n", text.c_str());
    });

    engine.onHalftimeTick([](int secondsLeft) {
        Serial.printf(
            "[CALLBACK] Halftime remaining: %d sec\n",
            secondsLeft
        );
    });

    engine.onMatchEnd([]() {
        Serial.println("[CALLBACK] Match ended");
    });
}

// =====================================================
// TEST MATCH CREATION
// =====================================================

void createTestMatch() {
    Serial.println("[TEST] Creating sample youth match...");

    bool created = app.createNewMatch(
        MatchDurationPreset::MIN_20,   // 20 min halves
        5,                             // 5 min halftime
        "HOME",
        "blue",
        "AWAY",
        "red"
    );

    if (!created) {
        Serial.println("[TEST] Failed to create match");
        return;
    }

    Serial.println("[TEST] Match created");

    Serial.print("[TEST] Score: ");
    Serial.println(app.getScoreText());

    Serial.print("[TEST] Half: ");
    Serial.println(app.getHalfText());

    testMatchCreated = true;
}

// =====================================================
// TEST EVENTS
// =====================================================

void logTestEvents() {
    Serial.println("[TEST] Logging sample events...");

    app.addGoal(
        TeamSide::HOME,
        10
    );

    app.addYellowCard(
        TeamSide::AWAY,
        7,
        YellowCardReason::DISSENT
    );

    app.addSubstitution(
        TeamSide::HOME,
        10,
        17
    );

    Serial.print("[TEST] Score after events: ");
    Serial.println(app.getScoreText());

    int count = app.getEventCount();

    Serial.printf("[TEST] Event count: %d\n", count);

    for (int i = 0; i < count; i++) {
        Serial.printf(
            "[TEST] Event %d: %s\n",
            i,
            app.getEventText(i).c_str()
        );
    }

    testEventsLogged = true;
}

// =====================================================
// SETUP
// =====================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("======================================");
    Serial.println(" Referee Watch App - Logic Test Boot  ");
    Serial.println("======================================");

    AppStartupResult result = app.begin();

    switch (result) {
        case AppStartupResult::READY_NEW_MATCH:
            Serial.println("[APP] Ready for new match");
            break;

        case AppStartupResult::RESUMED_IN_PROGRESS:
            Serial.println("[APP] Resumed in-progress match");
            break;

        case AppStartupResult::STORAGE_FAILED:
            Serial.println("[APP] Storage failed. Cannot continue.");
            return;

        case AppStartupResult::HAPTICS_FAILED:
            Serial.println("[APP] Haptics failed, continuing without vibration.");
            break;
    }

    registerEngineCallbacks();

    if (!app.hasInProgressMatch()) {
        createTestMatch();
        app.startMatch();
        testStartMs = millis();
    } else {
        Serial.println("[TEST] In-progress match found. Using existing state.");
        testMatchCreated = true;
        testStartMs = millis();
    }
}

// =====================================================
// LOOP
// =====================================================

void loop() {
    app.update();

    if (testMatchCreated && !testEventsLogged) {
        if (millis() - testStartMs > 3000) {
            logTestEvents();
        }
    }

    delay(10);
}
// FIXED: Removed duplicate setup()/loop() block that was pasted below the first
// entry point. Arduino only allows one setup() and one loop() per compilation unit.