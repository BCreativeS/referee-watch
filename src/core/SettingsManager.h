#pragma once

#include <Arduino.h>
#include "DatabaseManager.h"

// =====================================================
// SettingsManager.h
// Referee Watch App - Settings Logic
// =====================================================
//
// Handles app-level defaults:
// - vibration enabled
// - default half duration
// - halftime break duration
// - GPS enabled
// - auto-save interval
// - last match id
//
// =====================================================

class SettingsManager {
public:
    static SettingsManager& getInstance();

    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    bool begin(DatabaseManager* databaseManager);

    bool load();
    bool save();

    void resetToDefaults();

    bool isVibrationEnabled() const;
    void setVibrationEnabled(bool enabled);

    bool isSoundEnabled() const;          // FIXED: added — soundEnabled was in DB schema but missing here
    void setSoundEnabled(bool enabled);   // FIXED: added

    int getDefaultHalfMinutes() const;
    void setDefaultHalfMinutes(int minutes);

    int getDefaultHalftimeBreakMinutes() const;
    void setDefaultHalftimeBreakMinutes(int minutes);

    bool isGpsEnabled() const;
    void setGpsEnabled(bool enabled);

    int getAutoSaveIntervalSeconds() const;
    void setAutoSaveIntervalSeconds(int seconds);

    String getLastMatchId() const;
    void setLastMatchId(const String& matchId);

    MatchConfig createDefaultMatchConfig();

private:
    SettingsManager();

    DatabaseManager* db;

    bool vibrationEnabled;
    bool soundEnabled;        // FIXED: was missing — caused loadSettings/saveSettings arg count mismatch
    int defaultHalfMinutes;
    int defaultHalftimeBreakMinutes;
    bool gpsEnabled;
    int autoSaveIntervalSeconds;
    String lastMatchId;
};