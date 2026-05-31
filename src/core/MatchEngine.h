// =====================================================
// MatchEngine.h
// Referee Watch App - Core Match Logic
// =====================================================
//
// Handles:
// - Match state machine
// - Timer logic
// - Halftime logic
// - Event logging
// - Team stats
// - Auto-save
// - Database integration
// - UI callbacks
//
// MVP:
// - No roster API
// - Jersey numbers only
// - Manual match setup
// - Half duration presets
// - Optional custom half duration
//
// =====================================================

#pragma once

#include "DatabaseManager.h"
#include "HapticManager.h"
#include <Arduino.h>
#include <functional>
#include <vector>

// =====================================================
// MATCH STATE MACHINE
// =====================================================

enum class MatchState {
    IDLE,           // No match loaded
    SETUP,          // Match configuration screen
    READY,          // Match configured, ready to start
    FIRST_HALF,     // First half running
    HALFTIME,       // Halftime break timer
    SECOND_HALF,    // Second half running
    PAUSED,         // Match paused
    FULLTIME        // Match complete
};

// =====================================================
// TIMER MODE
// =====================================================

enum class TimerMode {
    COUNTING_UP,    // 00:00 upward
    COUNTING_DOWN   // Half duration downward
};

// =====================================================
// CALLBACK TYPES
// =====================================================

typedef std::function<void(MatchState oldState, MatchState newState)> StateChangeCallback;
typedef std::function<void(int minute, int second, int half, int stoppageSeconds)> TimerTickCallback;
typedef std::function<void(const Event& event)> EventLoggedCallback;
typedef std::function<void(int secondsLeft)> HalftimeTickCallback;
typedef std::function<void()> MatchEndCallback;

// =====================================================
// MATCH ENGINE
// =====================================================

class MatchEngine {
public:
    // Singleton
    static MatchEngine& getInstance();

    MatchEngine(const MatchEngine&) = delete;
    MatchEngine& operator=(const MatchEngine&) = delete;

    // =================================================
    // INITIALIZATION
    // =================================================

    void begin(DatabaseManager* databaseManager);
    void setHapticManager(HapticManager* haptics);  // FIXED: was missing — used in MatchEngine.cpp and AppController.cpp

    void setTimerMode(TimerMode mode);
    void setTimerTickInterval(int ms);

    // =================================================
    // MATCH LIFECYCLE
    // =================================================

    bool newMatch(
        const MatchConfig& config,
        const Team& home,
        const Team& away
    );

    bool loadExistingMatch(const String& matchId);
    bool resumeInProgressMatch();

    void startMatch();
    void pauseMatch();
    void resumeMatch();

    void endHalf();
    void startSecondHalf();
    void endMatch();
    void resetMatch();

    // =================================================
    // TIMER CONTROLS
    // =================================================

    void addStoppageTime(int seconds);
    void resetStoppageTime();

    int getCurrentMatchMinute();
    int getElapsedSeconds();
    int getStoppageSeconds();

    int getDisplayMinutes();
    int getDisplaySeconds();

    String getFormattedTime();

    // =================================================
    // EVENT LOGGING
    // =================================================

    bool logGoal(
        const String& team,
        int jerseyNumber
    );

    bool logYellowCard(
        const String& team,
        int jerseyNumber,
        const String& reason
    );

    bool logRedCard(
        const String& team,
        int jerseyNumber,
        const String& reason
    );

    bool logSubstitution(
        const String& team,
        int playerOut,
        int playerIn
    );

    int getEventCount();
    Event* getEvents();

    // =================================================
    // TEAM DATA
    // =================================================

    Team& getHomeTeam();
    Team& getAwayTeam();

    void incrementScore(const String& team);
    void incrementYellowCard(const String& team);
    void incrementRedCard(const String& team);
    void incrementSubstitution(const String& team);

    // =================================================
    // MATCH STATE QUERIES
    // =================================================

    MatchState getCurrentState();
    MatchState getPreviousState();

    int getCurrentHalf();

    int getHalftimeRemainingSeconds();

    bool isFirstHalf();
    bool isSecondHalf();
    bool isHalftime();
    bool isPaused();
    bool isMatchActive();
    bool isMatchComplete();

    MatchConfig getMatchConfig();
    String getCurrentMatchId();

    // =================================================
    // AUTO SAVE
    // =================================================

    void enableAutoSave(bool enable);
    void setAutoSaveInterval(int seconds);
    void forceSave();

    // =================================================
    // VIBRATION / HAPTICS
    // =================================================

    void enableVibration(bool enable);
    // FIXED: removed void vibrate(int durationMs, int intensity) — declared but never implemented

    // =================================================
    // CALLBACK REGISTRATION
    // =================================================

    void onStateChange(StateChangeCallback callback);
    void onTimerTick(TimerTickCallback callback);
    void onEventLogged(EventLoggedCallback callback);
    void onHalftimeTick(HalftimeTickCallback callback);
    void onMatchEnd(MatchEndCallback callback);

    // =================================================
    // MAIN LOOP UPDATE
    // =================================================

    void update();

    // =================================================
    // UTILITY
    // =================================================

    String getStateName(MatchState state);
    bool hasInProgressMatch();

private:
    MatchEngine();
    ~MatchEngine();

    // =================================================
    // INTERNAL STATE HELPERS
    // =================================================

    void changeState(MatchState newState);

    void updateTimer();
    void updateHalftimeTimer();
    void updateAutoSave();

    void saveCurrentState(const String& status);

    void addEvent(const Event& event);
    int getNextEventId();

    bool isValidTeamSide(const String& team);
    bool isValidJerseyNumber(int jerseyNumber);

    void updateTeamStatsFromEvent(const Event& event);
    void playHapticForEvent(const Event& event);  // FIXED: was playVibrationForEvent — name mismatch with .cpp implementation
    void playHaptic(HapticPattern pattern);       // FIXED: was missing — called internally throughout .cpp

    void fireStateChangeCallbacks(
        MatchState oldState,
        MatchState newState
    );

    void fireTimerTickCallbacks();
    void fireEventLoggedCallbacks(const Event& event);
    void fireHalftimeTickCallbacks();
    void fireMatchEndCallbacks();

    // =================================================
    // TIMER DATA
    // =================================================

    unsigned long lastTimerUpdateMs;
    unsigned long lastAutoSaveMs;

    int elapsedSeconds;
    int stoppageSeconds;
    int halftimeSecondsRemaining;

    int autoSaveIntervalSeconds;
    int timerTickIntervalMs;

    // =================================================
    // MATCH DATA
    // =================================================

    MatchState currentState;
    MatchState previousState;
    MatchState lastStateBeforePause;

    TimerMode timerMode;

    MatchConfig matchConfig;

    Team homeTeam;
    Team awayTeam;

    Timeline matchTimeline;

    Event events[MAX_EVENTS];
    int eventCount;

    String currentMatchId;

    // =================================================
    // FLAGS
    // =================================================

    bool autoSaveEnabled;
    bool matchModifiedSinceLastSave;
    bool vibrationEnabled;

    // =================================================
    // CALLBACK STORAGE
    // =================================================

    std::vector<StateChangeCallback> stateChangeCallbacks;
    std::vector<TimerTickCallback> timerTickCallbacks;
    std::vector<EventLoggedCallback> eventLoggedCallbacks;
    std::vector<HalftimeTickCallback> halftimeTickCallbacks;
    std::vector<MatchEndCallback> matchEndCallbacks;

    // =================================================
    // EXTERNAL DEPENDENCIES
    // =================================================

    DatabaseManager* db;
    HapticManager* hapticManager;  // FIXED: was missing — set in constructor and setHapticManager()
};