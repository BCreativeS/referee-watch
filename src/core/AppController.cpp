#include "AppController.h"

AppController& AppController::getInstance() {
    static AppController instance;
    return instance;
}

AppController::AppController() {
    settingsManager = &SettingsManager::getInstance();
    haptics = &HapticManager::getInstance();
    battery = &BatteryManager::getInstance();
    engine = &MatchEngine::getInstance();

    appReady = false;
}

AppStartupResult AppController::begin() {
    Serial.println("[APP] Starting Referee Watch App...");

    bool storageReady = database.initSD(
        BoardConfig::SD_CS,
        BoardConfig::SD_SCK,
        BoardConfig::SD_MOSI,
        BoardConfig::SD_MISO
    );

    if (!storageReady) {
        appReady = false;
        return AppStartupResult::STORAGE_FAILED;
    }

    settingsManager->begin(&database);

    bool hapticsReady = haptics->begin();
    bool batteryReady = battery->begin(&database);

    engine->begin(&database);
    engine->setHapticManager(haptics);
    engine->enableVibration(settingsManager->isVibrationEnabled());
    engine->setAutoSaveInterval(settingsManager->getAutoSaveIntervalSeconds());

    if (engine->hasInProgressMatch()) {
        bool resumed = engine->resumeInProgressMatch();

        if (resumed) {
            appReady = true;

            if (!batteryReady) {
                return AppStartupResult::BATTERY_FAILED;
            }

            if (!hapticsReady) {
                return AppStartupResult::HAPTICS_FAILED;
            }

            return AppStartupResult::RESUMED_IN_PROGRESS;
        }
    }

    appReady = true;

    if (!batteryReady) {
        return AppStartupResult::BATTERY_FAILED;
    }

    if (!hapticsReady) {
        return AppStartupResult::HAPTICS_FAILED;
    }

    return AppStartupResult::READY_NEW_MATCH;
}

void AppController::update() {
    if (!appReady) return;

    engine->update();
    battery->update();

    handleBatteryEmergencySave();
}

bool AppController::createNewMatch(
    int halfPreset,
    int halftimeBreakMinutes,
    const String& homeName,
    const String& homeColor,
    const String& awayName,
    const String& awayColor
) {
    if (!appReady) return false;

    if (!database.isValidHalfPreset(halfPreset)) {
        haptics->error();
        return false;
    }

    if (halfPreset == MatchDurationPreset::CUSTOM) {
        haptics->error();
        return false;
    }

    if (!validateTeamSetup(homeName, homeColor, awayName, awayColor)) {
        haptics->error();
        return false;
    }

    MatchConfig config = database.createConfigFromHalfPreset(
        halfPreset,
        halftimeBreakMinutes
    );

    Team home = makeTeam(homeName, homeColor);
    Team away = makeTeam(awayName, awayColor);

    bool created = engine->newMatch(config, home, away);

    if (created) {
        settingsManager->setLastMatchId(engine->getCurrentMatchId());
        settingsManager->save();
    }

    return created;
}

bool AppController::createCustomDurationMatch(
    int customHalfMinutes,
    int halftimeBreakMinutes,
    const String& homeName,
    const String& homeColor,
    const String& awayName,
    const String& awayColor
) {
    if (!appReady) return false;

    if (!validateTeamSetup(homeName, homeColor, awayName, awayColor)) {
        haptics->error();
        return false;
    }

    MatchConfig config = database.createCustomConfig(
        customHalfMinutes,
        halftimeBreakMinutes
    );

    Team home = makeTeam(homeName, homeColor);
    Team away = makeTeam(awayName, awayColor);

    bool created = engine->newMatch(config, home, away);

    if (created) {
        settingsManager->setLastMatchId(engine->getCurrentMatchId());
        settingsManager->save();
    }

    return created;
}

void AppController::startMatch() {
    if (!appReady) return;
    engine->startMatch();
}

void AppController::pauseMatch() {
    if (!appReady) return;
    engine->pauseMatch();
}

void AppController::resumeMatch() {
    if (!appReady) return;
    engine->resumeMatch();
}

void AppController::endHalf() {
    if (!appReady) return;
    engine->endHalf();
}

void AppController::startSecondHalf() {
    if (!appReady) return;
    engine->startSecondHalf();
}

void AppController::endMatch() {
    if (!appReady) return;
    engine->endMatch();
}

void AppController::resetMatch() {
    if (!appReady) return;
    engine->resetMatch();
}

bool AppController::addGoal(const String& team, int jerseyNumber) {
    if (!appReady) return false;
    return engine->logGoal(team, jerseyNumber);
}

bool AppController::addYellowCard(
    const String& team,
    int jerseyNumber,
    const String& reason
) {
    if (!appReady) return false;
    return engine->logYellowCard(team, jerseyNumber, reason);
}

bool AppController::addRedCard(
    const String& team,
    int jerseyNumber,
    const String& reason
) {
    if (!appReady) return false;
    return engine->logRedCard(team, jerseyNumber, reason);
}

bool AppController::addSubstitution(
    const String& team,
    int playerOut,
    int playerIn
) {
    if (!appReady) return false;
    return engine->logSubstitution(team, playerOut, playerIn);
}

void AppController::addStoppageSeconds(int seconds) {
    if (!appReady) return;
    engine->addStoppageTime(seconds);
}

void AppController::addStoppageMinutes(int minutes) {
    if (!appReady) return;
    if (minutes <= 0) return;
    engine->addStoppageTime(minutes * 60);
}

MatchState AppController::getMatchState() {
    return engine->getCurrentState();
}

String AppController::getMatchStateName() {
    return engine->getStateName(engine->getCurrentState());
}

String AppController::getTimerText() {
    if (engine->isHalftime()) {
        return formatHalftimeClock(engine->getHalftimeRemainingSeconds());
    }

    return engine->getFormattedTime();
}

String AppController::getScoreText() {
    return EventFormatter::formatScore(
        engine->getHomeTeam(),
        engine->getAwayTeam()
    );
}

String AppController::getHalfText() {
    MatchState state = engine->getCurrentState();

    if (state == MatchState::FIRST_HALF) return "1ST HALF";
    if (state == MatchState::SECOND_HALF) return "2ND HALF";
    if (state == MatchState::HALFTIME) return "HALFTIME";
    if (state == MatchState::FULLTIME) return "FULLTIME";
    if (state == MatchState::PAUSED) return "PAUSED";
    if (state == MatchState::READY) return "READY";

    return "SETUP";
}

Team AppController::getHomeTeam() {
    return engine->getHomeTeam();
}

Team AppController::getAwayTeam() {
    return engine->getAwayTeam();
}

MatchConfig AppController::getMatchConfig() {
    return engine->getMatchConfig();
}

int AppController::getEventCount() {
    return engine->getEventCount();
}

Event* AppController::getEvents() {
    return engine->getEvents();
}

String AppController::getEventText(int index) {
    if (index < 0 || index >= engine->getEventCount()) {
        return "";
    }

    Event* events = engine->getEvents();

    return EventFormatter::formatEvent(
        events[index],
        &database
    );
}

String AppController::getEventShortText(int index) {
    if (index < 0 || index >= engine->getEventCount()) {
        return "";
    }

    Event* events = engine->getEvents();

    return EventFormatter::formatEventShort(
        events[index]
    );
}

int AppController::getHalftimeRemainingSeconds() {
    return engine->getHalftimeRemainingSeconds();
}

int AppController::getBatteryPercent() {
    return battery->getPercent();
}

float AppController::getBatteryVoltage() {
    return battery->getVoltage();
}

String AppController::getBatteryStateText() {
    return battery->getStateText();
}

bool AppController::isCharging() {
    return battery->isCharging();
}

bool AppController::hasInProgressMatch() {
    return engine->hasInProgressMatch();
}

bool AppController::isReady() {
    return appReady;
}

SettingsManager& AppController::settings() {
    return *settingsManager;
}

bool AppController::setVibrationEnabled(bool enabled) {
    settingsManager->setVibrationEnabled(enabled);
    engine->enableVibration(enabled);
    haptics->enable(enabled);

    return settingsManager->save();
}

bool AppController::setDefaultHalfPreset(int preset) {
    settingsManager->setDefaultHalfMinutes(preset);
    return settingsManager->save();
}

bool AppController::setDefaultHalftimeBreak(int minutes) {
    settingsManager->setDefaultHalftimeBreakMinutes(minutes);
    return settingsManager->save();
}

bool AppController::setAutoSaveInterval(int seconds) {
    settingsManager->setAutoSaveIntervalSeconds(seconds);
    engine->setAutoSaveInterval(seconds);

    return settingsManager->save();
}

bool AppController::validateTeamSetup(
    const String& homeName,
    const String& homeColor,
    const String& awayName,
    const String& awayColor
) {
    if (homeName.length() == 0) return false;
    if (awayName.length() == 0) return false;

    if (!database.isValidJerseyColor(homeColor)) return false;
    if (!database.isValidJerseyColor(awayColor)) return false;

    return true;
}

Team AppController::makeTeam(
    const String& name,
    const String& color
) {
    Team team;

    team.name = name;
    team.jerseyColor = color;

    team.score = 0;
    team.yellowCards = 0;
    team.redCards = 0;
    team.substitutions = 0;

    return team;
}

String AppController::formatHalftimeClock(int seconds) {
    if (seconds < 0) {
        seconds = 0;
    }

    int minutesPart = seconds / 60;
    int secondsPart = seconds % 60;

    char buffer[12];

    snprintf(
        buffer,
        sizeof(buffer),
        "%02d:%02d",
        minutesPart,
        secondsPart
    );

    return String(buffer);
}

void AppController::handleBatteryEmergencySave() {
    if (!battery->shouldTriggerEmergencySave()) {
        return;
    }

    MatchState state = engine->getCurrentState();

    bool matchNeedsBackup =
        state == MatchState::FIRST_HALF ||
        state == MatchState::SECOND_HALF ||
        state == MatchState::PAUSED ||
        state == MatchState::HALFTIME;

    if (matchNeedsBackup) {
        engine->forceSave();
        haptics->warning();
        Serial.println("[APP] Emergency battery save completed");
    }

    battery->clearEmergencySaveFlag();
}