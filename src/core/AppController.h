#pragma once

#include <Arduino.h>

#include "BoardConfig.h"
#include "DatabaseManager.h"
#include "SettingsManager.h"
#include "HapticManager.h"
#include "BatteryManager.h"
#include "MatchEngine.h"
#include "EventFormatter.h"

enum class AppStartupResult {
    READY_NEW_MATCH,
    RESUMED_IN_PROGRESS,
    STORAGE_FAILED,
    HAPTICS_FAILED,
    BATTERY_FAILED
};

class AppController {
public:
    static AppController& getInstance();

    AppController(const AppController&) = delete;
    AppController& operator=(const AppController&) = delete;

    AppStartupResult begin();
    void update();

    bool createNewMatch(
        int halfPreset,
        int halftimeBreakMinutes,
        const String& homeName,
        const String& homeColor,
        const String& awayName,
        const String& awayColor
    );

    bool createCustomDurationMatch(
        int customHalfMinutes,
        int halftimeBreakMinutes,
        const String& homeName,
        const String& homeColor,
        const String& awayName,
        const String& awayColor
    );

    void startMatch();
    void pauseMatch();
    void resumeMatch();
    void endHalf();
    void startSecondHalf();
    void endMatch();
    void resetMatch();

    bool addGoal(const String& team, int jerseyNumber);
    bool addYellowCard(const String& team, int jerseyNumber, const String& reason);
    bool addRedCard(const String& team, int jerseyNumber, const String& reason);
    bool addSubstitution(const String& team, int playerOut, int playerIn);

    void addStoppageSeconds(int seconds);
    void addStoppageMinutes(int minutes);

    MatchState getMatchState();
    String getMatchStateName();

    String getTimerText();
    String getScoreText();
    String getHalfText();

    Team getHomeTeam();
    Team getAwayTeam();

    MatchConfig getMatchConfig();

    int getEventCount();
    Event* getEvents();

    String getEventText(int index);
    String getEventShortText(int index);

    int getHalftimeRemainingSeconds();

    int getBatteryPercent();
    float getBatteryVoltage();
    String getBatteryStateText();
    bool isCharging();

    bool hasInProgressMatch();
    bool isReady();

    SettingsManager& settings();

    bool setVibrationEnabled(bool enabled);
    bool setDefaultHalfPreset(int preset);
    bool setDefaultHalftimeBreak(int minutes);
    bool setAutoSaveInterval(int seconds);

private:
    AppController();

    bool validateTeamSetup(
        const String& homeName,
        const String& homeColor,
        const String& awayName,
        const String& awayColor
    );

    Team makeTeam(const String& name, const String& color);

    String formatHalftimeClock(int seconds);

    void handleBatteryEmergencySave();

    DatabaseManager database;
    SettingsManager* settingsManager;
    HapticManager* haptics;
    BatteryManager* battery;
    MatchEngine* engine;

    bool appReady;
};