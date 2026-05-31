// =====================================================
// MatchEngine.cpp
// Referee Watch App - Core Match Logic
// =====================================================

#include "MatchEngine.h"

// =====================================================
// SINGLETON
// =====================================================

MatchEngine& MatchEngine::getInstance() {
    static MatchEngine instance;
    return instance;
}

// =====================================================
// CONSTRUCTOR / DESTRUCTOR
// =====================================================

MatchEngine::MatchEngine() {
    db = nullptr;
    hapticManager = nullptr;

    currentState = MatchState::IDLE;
    previousState = MatchState::IDLE;
    lastStateBeforePause = MatchState::IDLE;

    timerMode = TimerMode::COUNTING_UP;

    elapsedSeconds = 0;
    stoppageSeconds = 0;
    halftimeSecondsRemaining = 0;

    lastTimerUpdateMs = 0;
    lastAutoSaveMs = 0;

    autoSaveIntervalSeconds = 30;
    timerTickIntervalMs = 1000;

    eventCount = 0;
    currentMatchId = "";

    autoSaveEnabled = true;
    matchModifiedSinceLastSave = false;
    vibrationEnabled = true;
}

MatchEngine::~MatchEngine() {}

// =====================================================
// INITIALIZATION
// =====================================================

void MatchEngine::begin(DatabaseManager* databaseManager) {
    db = databaseManager;
    currentState = MatchState::IDLE;
}

void MatchEngine::setHapticManager(HapticManager* haptics) {
    hapticManager = haptics;
}

// =====================================================
// CONFIGURATION
// =====================================================

void MatchEngine::setTimerMode(TimerMode mode) {
    timerMode = mode;
}

void MatchEngine::setTimerTickInterval(int ms) {
    if (ms < 250) ms = 250;
    timerTickIntervalMs = ms;
}

// =====================================================
// MATCH LIFECYCLE
// =====================================================

bool MatchEngine::newMatch(
    const MatchConfig& config,
    const Team& home,
    const Team& away
) {
    matchConfig = config;
    homeTeam = home;
    awayTeam = away;

    elapsedSeconds = 0;
    stoppageSeconds = 0;
    eventCount = 0;

    matchTimeline = Timeline();

    if (db) {
        currentMatchId = db->getNextMatchId();
    } else {
        currentMatchId = "match_unsynced";
    }

    changeState(MatchState::READY);

    matchModifiedSinceLastSave = true;
    forceSave();

    playHaptic(HapticPattern::CONFIRM);

    return true;
}

bool MatchEngine::loadExistingMatch(const String& matchId) {
    if (!db) return false;

    String status;

    bool loaded = db->loadMatch(
        matchId,
        matchConfig,
        homeTeam,
        awayTeam,
        events,
        eventCount,
        matchTimeline,
        status
    );

    if (!loaded) {
        playHaptic(HapticPattern::ERROR);
        return false;
    }

    currentMatchId = matchId;

    if (status == "completed") {
        changeState(MatchState::FULLTIME);
    } else if (status == "in_progress") {
        changeState(MatchState::PAUSED);
    } else {
        changeState(MatchState::READY);
    }

    matchModifiedSinceLastSave = false;

    playHaptic(HapticPattern::CONFIRM);

    return true;
}

bool MatchEngine::resumeInProgressMatch() {
    if (!db) return false;
    if (!db->hasInProgressMatch()) return false;

    bool loaded = db->loadInProgressMatch(
        matchConfig,
        homeTeam,
        awayTeam,
        events,
        eventCount,
        matchTimeline
    );

    if (!loaded) {
        playHaptic(HapticPattern::ERROR);
        return false;
    }

    currentMatchId = "in_progress";

    changeState(MatchState::PAUSED);

    playHaptic(HapticPattern::CONFIRM);

    return true;
}

void MatchEngine::startMatch() {
    if (currentState != MatchState::READY &&
        currentState != MatchState::SETUP &&
        currentState != MatchState::IDLE) {
        return;
    }

    elapsedSeconds = 0;
    stoppageSeconds = 0;

    matchTimeline.matchStarted = db ? db->getCurrentTimestamp() : "";

    lastTimerUpdateMs = millis();

    changeState(MatchState::FIRST_HALF);

    matchModifiedSinceLastSave = true;
    forceSave();

    playHaptic(HapticPattern::CONFIRM);
}

void MatchEngine::pauseMatch() {
    if (!isMatchActive()) return;
    if (currentState == MatchState::PAUSED) return;

    lastStateBeforePause = currentState;

    changeState(MatchState::PAUSED);

    matchModifiedSinceLastSave = true;
    forceSave();

    playHaptic(HapticPattern::TAP);
}

void MatchEngine::resumeMatch() {
    if (currentState != MatchState::PAUSED) return;

    lastTimerUpdateMs = millis();

    if (lastStateBeforePause == MatchState::SECOND_HALF) {
        changeState(MatchState::SECOND_HALF);
    } else {
        changeState(MatchState::FIRST_HALF);
    }

    matchModifiedSinceLastSave = true;

    playHaptic(HapticPattern::CONFIRM);
}

void MatchEngine::endHalf() {
    if (currentState != MatchState::FIRST_HALF) return;

    matchTimeline.half1Ended = db ? db->getCurrentTimestamp() : "";

    halftimeSecondsRemaining = matchConfig.halfTimeBreakMinutes * 60;
    elapsedSeconds = 0;
    stoppageSeconds = 0;

    lastTimerUpdateMs = millis();

    changeState(MatchState::HALFTIME);

    matchModifiedSinceLastSave = true;
    forceSave();

    playHaptic(HapticPattern::HALFTIME);
}

void MatchEngine::startSecondHalf() {
    if (currentState != MatchState::HALFTIME) return;

    matchTimeline.half2Started = db ? db->getCurrentTimestamp() : "";

    elapsedSeconds = 0;
    stoppageSeconds = 0;
    halftimeSecondsRemaining = 0;

    lastTimerUpdateMs = millis();

    changeState(MatchState::SECOND_HALF);

    matchModifiedSinceLastSave = true;
    forceSave();

    playHaptic(HapticPattern::CONFIRM);
}

void MatchEngine::endMatch() {
    if (currentState == MatchState::FULLTIME) return;

    matchTimeline.matchEnded = db ? db->getCurrentTimestamp() : "";

    changeState(MatchState::FULLTIME);

    matchModifiedSinceLastSave = true;

    saveCurrentState("completed");

    if (db) {
        db->clearInProgressMatch();
    }

    playHaptic(HapticPattern::FULLTIME);

    fireMatchEndCallbacks();
}

void MatchEngine::resetMatch() {
    elapsedSeconds = 0;
    stoppageSeconds = 0;
    halftimeSecondsRemaining = 0;

    eventCount = 0;
    currentMatchId = "";

    homeTeam = Team();
    awayTeam = Team();
    matchConfig = MatchConfig();
    matchTimeline = Timeline();

    matchModifiedSinceLastSave = false;

    changeState(MatchState::IDLE);

    playHaptic(HapticPattern::WARNING);
}

// =====================================================
// TIMER CONTROLS
// =====================================================

void MatchEngine::addStoppageTime(int seconds) {
    if (seconds <= 0) return;

    stoppageSeconds += seconds;

    if (currentState == MatchState::FIRST_HALF) {
        matchConfig.stoppageTimeHalf1 = stoppageSeconds;
    }

    if (currentState == MatchState::SECOND_HALF) {
        matchConfig.stoppageTimeHalf2 = stoppageSeconds;
    }

    matchModifiedSinceLastSave = true;
    playHaptic(HapticPattern::TAP);
}

void MatchEngine::resetStoppageTime() {
    stoppageSeconds = 0;

    if (currentState == MatchState::FIRST_HALF) {
        matchConfig.stoppageTimeHalf1 = 0;
    }

    if (currentState == MatchState::SECOND_HALF) {
        matchConfig.stoppageTimeHalf2 = 0;
    }

    matchModifiedSinceLastSave = true;
    playHaptic(HapticPattern::WARNING);
}

int MatchEngine::getCurrentMatchMinute() {
    return (elapsedSeconds / 60) + 1;
}

int MatchEngine::getElapsedSeconds() {
    return elapsedSeconds;
}

int MatchEngine::getStoppageSeconds() {
    return stoppageSeconds;
}

int MatchEngine::getDisplayMinutes() {
    if (timerMode == TimerMode::COUNTING_DOWN) {
        int remaining = (matchConfig.halfMinutes * 60) - elapsedSeconds;
        if (remaining < 0) remaining = 0;
        return remaining / 60;
    }

    return elapsedSeconds / 60;
}

int MatchEngine::getDisplaySeconds() {
    if (timerMode == TimerMode::COUNTING_DOWN) {
        int remaining = (matchConfig.halfMinutes * 60) - elapsedSeconds;
        if (remaining < 0) remaining = 0;
        return remaining % 60;
    }

    return elapsedSeconds % 60;
}

String MatchEngine::getFormattedTime() {
    char buffer[20];

    int minutes = getDisplayMinutes();
    int seconds = getDisplaySeconds();

    if (stoppageSeconds > 0) {
        snprintf(
            buffer,
            sizeof(buffer),
            "%02d:%02d +%d",
            minutes,
            seconds,
            stoppageSeconds / 60
        );
    } else {
        snprintf(
            buffer,
            sizeof(buffer),
            "%02d:%02d",
            minutes,
            seconds
        );
    }

    return String(buffer);
}

// =====================================================
// EVENT LOGGING
// =====================================================

bool MatchEngine::logGoal(
    const String& team,
    int jerseyNumber
) {
    if (!isValidTeamSide(team)) {
        playHaptic(HapticPattern::ERROR);
        return false;
    }

    if (!isValidJerseyNumber(jerseyNumber)) {
        playHaptic(HapticPattern::ERROR);
        return false;
    }

    Event event;
    event.id = getNextEventId();
    event.half = getCurrentHalf();
    event.minute = getCurrentMatchMinute();
    event.type = EventType::GOAL;
    event.team = team;
    event.jerseyNumber = jerseyNumber;

    addEvent(event);
    return true;
}

bool MatchEngine::logYellowCard(
    const String& team,
    int jerseyNumber,
    const String& reason
) {
    if (!isValidTeamSide(team)) {
        playHaptic(HapticPattern::ERROR);
        return false;
    }

    if (!isValidJerseyNumber(jerseyNumber)) {
        playHaptic(HapticPattern::ERROR);
        return false;
    }

    if (db && !db->isValidYellowReason(reason)) {
        playHaptic(HapticPattern::ERROR);
        return false;
    }

    Event event;
    event.id = getNextEventId();
    event.half = getCurrentHalf();
    event.minute = getCurrentMatchMinute();
    event.type = EventType::YELLOW;
    event.team = team;
    event.jerseyNumber = jerseyNumber;
    event.reason = reason;

    addEvent(event);
    return true;
}

bool MatchEngine::logRedCard(
    const String& team,
    int jerseyNumber,
    const String& reason
) {
    if (!isValidTeamSide(team)) {
        playHaptic(HapticPattern::ERROR);
        return false;
    }

    if (!isValidJerseyNumber(jerseyNumber)) {
        playHaptic(HapticPattern::ERROR);
        return false;
    }

    if (db && !db->isValidRedReason(reason)) {
        playHaptic(HapticPattern::ERROR);
        return false;
    }

    Event event;
    event.id = getNextEventId();
    event.half = getCurrentHalf();
    event.minute = getCurrentMatchMinute();
    event.type = EventType::RED;
    event.team = team;
    event.jerseyNumber = jerseyNumber;
    event.reason = reason;

    addEvent(event);
    return true;
}

bool MatchEngine::logSubstitution(
    const String& team,
    int playerOut,
    int playerIn
) {
    if (!isValidTeamSide(team)) {
        playHaptic(HapticPattern::ERROR);
        return false;
    }

    if (!isValidJerseyNumber(playerOut) ||
        !isValidJerseyNumber(playerIn)) {
        playHaptic(HapticPattern::ERROR);
        return false;
    }

    Event event;
    event.id = getNextEventId();
    event.half = getCurrentHalf();
    event.minute = getCurrentMatchMinute();
    event.type = EventType::SUBSTITUTION;
    event.team = team;
    event.playerOut = playerOut;
    event.playerIn = playerIn;

    addEvent(event);
    return true;
}

int MatchEngine::getEventCount() {
    return eventCount;
}

Event* MatchEngine::getEvents() {
    return events;
}

// =====================================================
// TEAM DATA
// =====================================================

Team& MatchEngine::getHomeTeam() {
    return homeTeam;
}

Team& MatchEngine::getAwayTeam() {
    return awayTeam;
}

void MatchEngine::incrementScore(const String& team) {
    if (team == TeamSide::HOME) {
        homeTeam.score++;
    } else if (team == TeamSide::AWAY) {
        awayTeam.score++;
    }
}

void MatchEngine::incrementYellowCard(const String& team) {
    if (team == TeamSide::HOME) {
        homeTeam.yellowCards++;
    } else if (team == TeamSide::AWAY) {
        awayTeam.yellowCards++;
    }
}

void MatchEngine::incrementRedCard(const String& team) {
    if (team == TeamSide::HOME) {
        homeTeam.redCards++;
    } else if (team == TeamSide::AWAY) {
        awayTeam.redCards++;
    }
}

void MatchEngine::incrementSubstitution(const String& team) {
    if (team == TeamSide::HOME) {
        homeTeam.substitutions++;
    } else if (team == TeamSide::AWAY) {
        awayTeam.substitutions++;
    }
}

// =====================================================
// MATCH STATE QUERIES
// =====================================================

MatchState MatchEngine::getCurrentState() {
    return currentState;
}

MatchState MatchEngine::getPreviousState() {
    return previousState;
}

int MatchEngine::getCurrentHalf() {
    if (currentState == MatchState::SECOND_HALF) return 2;
    if (lastStateBeforePause == MatchState::SECOND_HALF) return 2;
    return 1;
}

int MatchEngine::getHalftimeRemainingSeconds() {
    return halftimeSecondsRemaining;
}

bool MatchEngine::isFirstHalf() {
    return currentState == MatchState::FIRST_HALF;
}

bool MatchEngine::isSecondHalf() {
    return currentState == MatchState::SECOND_HALF;
}

bool MatchEngine::isHalftime() {
    return currentState == MatchState::HALFTIME;
}

bool MatchEngine::isPaused() {
    return currentState == MatchState::PAUSED;
}

bool MatchEngine::isMatchActive() {
    return currentState == MatchState::FIRST_HALF ||
           currentState == MatchState::SECOND_HALF ||
           currentState == MatchState::PAUSED;
}

bool MatchEngine::isMatchComplete() {
    return currentState == MatchState::FULLTIME;
}

MatchConfig MatchEngine::getMatchConfig() {
    return matchConfig;
}

String MatchEngine::getCurrentMatchId() {
    return currentMatchId;
}

// =====================================================
// AUTO SAVE
// =====================================================

void MatchEngine::enableAutoSave(bool enable) {
    autoSaveEnabled = enable;
}

void MatchEngine::setAutoSaveInterval(int seconds) {
    if (seconds < 5) seconds = 5;
    autoSaveIntervalSeconds = seconds;
}

void MatchEngine::forceSave() {
    saveCurrentState(
        currentState == MatchState::FULLTIME ? "completed" : "in_progress"
    );
}

// =====================================================
// HAPTICS
// =====================================================

void MatchEngine::enableVibration(bool enable) {
    vibrationEnabled = enable;

    if (hapticManager) {
        hapticManager->enable(enable);
    }
}

// =====================================================
// CALLBACK REGISTRATION
// =====================================================

void MatchEngine::onStateChange(StateChangeCallback callback) {
    stateChangeCallbacks.push_back(callback);
}

void MatchEngine::onTimerTick(TimerTickCallback callback) {
    timerTickCallbacks.push_back(callback);
}

void MatchEngine::onEventLogged(EventLoggedCallback callback) {
    eventLoggedCallbacks.push_back(callback);
}

void MatchEngine::onHalftimeTick(HalftimeTickCallback callback) {
    halftimeTickCallbacks.push_back(callback);
}

void MatchEngine::onMatchEnd(MatchEndCallback callback) {
    matchEndCallbacks.push_back(callback);
}

// =====================================================
// MAIN UPDATE LOOP
// =====================================================

void MatchEngine::update() {
    updateTimer();
    updateHalftimeTimer();
    updateAutoSave();
}

// =====================================================
// UTILITY
// =====================================================

String MatchEngine::getStateName(MatchState state) {
    switch (state) {
        case MatchState::IDLE: return "IDLE";
        case MatchState::SETUP: return "SETUP";
        case MatchState::READY: return "READY";
        case MatchState::FIRST_HALF: return "FIRST_HALF";
        case MatchState::HALFTIME: return "HALFTIME";
        case MatchState::SECOND_HALF: return "SECOND_HALF";
        case MatchState::PAUSED: return "PAUSED";
        case MatchState::FULLTIME: return "FULLTIME";
        default: return "UNKNOWN";
    }
}

bool MatchEngine::hasInProgressMatch() {
    return db && db->hasInProgressMatch();
}

// =====================================================
// INTERNAL STATE HELPERS
// =====================================================

void MatchEngine::changeState(MatchState newState) {
    if (newState == currentState) return;

    MatchState oldState = currentState;

    previousState = currentState;
    currentState = newState;

    fireStateChangeCallbacks(oldState, newState);
}

void MatchEngine::updateTimer() {
    if (currentState != MatchState::FIRST_HALF &&
        currentState != MatchState::SECOND_HALF) {
        return;
    }

    unsigned long now = millis();

    if (now - lastTimerUpdateMs < (unsigned long)timerTickIntervalMs) {
        return;
    }

    lastTimerUpdateMs = now;
    elapsedSeconds++;

    int halfLimitSeconds = matchConfig.halfMinutes * 60;

    if (elapsedSeconds >= halfLimitSeconds + stoppageSeconds) {
        if (currentState == MatchState::FIRST_HALF) {
            endHalf();
            return;
        }

        if (currentState == MatchState::SECOND_HALF) {
            endMatch();
            return;
        }
    }

    fireTimerTickCallbacks();
}

void MatchEngine::updateHalftimeTimer() {
    if (currentState != MatchState::HALFTIME) return;

    unsigned long now = millis();

    if (now - lastTimerUpdateMs < (unsigned long)timerTickIntervalMs) {
        return;
    }

    lastTimerUpdateMs = now;

    if (halftimeSecondsRemaining > 0) {
        halftimeSecondsRemaining--;
        fireHalftimeTickCallbacks();
    }

    if (halftimeSecondsRemaining <= 0) {
        playHaptic(HapticPattern::HALFTIME);
    }
}

void MatchEngine::updateAutoSave() {
    if (!autoSaveEnabled) return;
    if (!matchModifiedSinceLastSave) return;
    if (!db) return;

    unsigned long now = millis();

    if (now - lastAutoSaveMs < (unsigned long)(autoSaveIntervalSeconds * 1000)) {
        return;
    }

    forceSave();
}

void MatchEngine::saveCurrentState(const String& status) {
    if (!db) return;

    db->saveInProgressMatch(
        matchConfig,
        homeTeam,
        awayTeam,
        events,
        eventCount,
        matchTimeline
    );

    if (status == "completed" && currentMatchId != "") {
        db->saveMatch(
            currentMatchId,
            matchConfig,
            homeTeam,
            awayTeam,
            events,
            eventCount,
            matchTimeline,
            "completed"
        );
    }

    lastAutoSaveMs = millis();
    matchModifiedSinceLastSave = false;
}

void MatchEngine::addEvent(const Event& event) {
    if (eventCount >= MAX_EVENTS) {
        playHaptic(HapticPattern::ERROR);
        return;
    }

    events[eventCount] = event;
    eventCount++;

    updateTeamStatsFromEvent(event);
    playHapticForEvent(event);

    matchModifiedSinceLastSave = true;

    fireEventLoggedCallbacks(event);

    forceSave();
}

int MatchEngine::getNextEventId() {
    int highest = 0;

    for (int i = 0; i < eventCount; i++) {
        if (events[i].id > highest) {
            highest = events[i].id;
        }
    }

    return highest + 1;
}

bool MatchEngine::isValidTeamSide(const String& team) {
    return team == TeamSide::HOME || team == TeamSide::AWAY;
}

bool MatchEngine::isValidJerseyNumber(int jerseyNumber) {
    return jerseyNumber >= 0 && jerseyNumber <= 99;
}

void MatchEngine::updateTeamStatsFromEvent(const Event& event) {
    if (event.type == EventType::GOAL) {
        incrementScore(event.team);
    } else if (event.type == EventType::YELLOW) {
        incrementYellowCard(event.team);
    } else if (event.type == EventType::RED) {
        incrementRedCard(event.team);
    } else if (event.type == EventType::SUBSTITUTION) {
        incrementSubstitution(event.team);
    }
}

void MatchEngine::playHapticForEvent(const Event& event) {
    if (event.type == EventType::GOAL) {
        playHaptic(HapticPattern::GOAL);
    } else if (event.type == EventType::YELLOW) {
        playHaptic(HapticPattern::YELLOW_CARD);
    } else if (event.type == EventType::RED) {
        playHaptic(HapticPattern::RED_CARD);
    } else if (event.type == EventType::SUBSTITUTION) {
        playHaptic(HapticPattern::SUBSTITUTION);
    }
}

void MatchEngine::playHaptic(HapticPattern pattern) {
    if (!vibrationEnabled) return;
    if (!hapticManager) return;

    hapticManager->play(pattern);
}

// =====================================================
// CALLBACK FIRING
// =====================================================

void MatchEngine::fireStateChangeCallbacks(
    MatchState oldState,
    MatchState newState
) {
    for (auto& callback : stateChangeCallbacks) {
        callback(oldState, newState);
    }
}

void MatchEngine::fireTimerTickCallbacks() {
    int minute = getDisplayMinutes();
    int second = getDisplaySeconds();
    int half = getCurrentHalf();

    for (auto& callback : timerTickCallbacks) {
        callback(minute, second, half, stoppageSeconds);
    }
}

void MatchEngine::fireEventLoggedCallbacks(const Event& event) {
    for (auto& callback : eventLoggedCallbacks) {
        callback(event);
    }
}

void MatchEngine::fireHalftimeTickCallbacks() {
    for (auto& callback : halftimeTickCallbacks) {
        callback(halftimeSecondsRemaining);
    }
}

void MatchEngine::fireMatchEndCallbacks() {
    for (auto& callback : matchEndCallbacks) {
        callback();
    }
}