#include "SettingsManager.h"

// =====================================================
// SINGLETON
// =====================================================

SettingsManager& SettingsManager::getInstance() {
    static SettingsManager instance;
    return instance;
}

// =====================================================
// CONSTRUCTOR
// =====================================================

SettingsManager::SettingsManager() {
    db = nullptr;
    resetToDefaults();
}

// =====================================================
// INITIALIZATION
// =====================================================

bool SettingsManager::begin(DatabaseManager* databaseManager) {
    db = databaseManager;
    return load();
}

// =====================================================
// LOAD / SAVE
// =====================================================

bool SettingsManager::load() {
    if (!db) {
        resetToDefaults();
        return false;
    }

    // FIXED: was calling db->loadSettings() with 6 args instead of 7.
    // soundEnabled was missing, causing all subsequent args to be misaligned
    // with wrong types (int passed as bool, String passed as int, etc.).
    bool loaded = db->loadSettings(
        vibrationEnabled,
        soundEnabled,
        defaultHalfMinutes,
        defaultHalftimeBreakMinutes,
        gpsEnabled,
        autoSaveIntervalSeconds,
        lastMatchId
    );

    if (!loaded) {
        resetToDefaults();
        return false;
    }

    if (!db->isValidHalfPreset(defaultHalfMinutes)) {
        defaultHalfMinutes = MatchDurationPreset::MIN_45;
    }

    if (defaultHalftimeBreakMinutes < 1) {
        defaultHalftimeBreakMinutes = 10;
    }

    if (defaultHalftimeBreakMinutes > 30) {
        defaultHalftimeBreakMinutes = 30;
    }

    if (autoSaveIntervalSeconds < 5) {
        autoSaveIntervalSeconds = 30;
    }

    if (autoSaveIntervalSeconds > 300) {
        autoSaveIntervalSeconds = 300;
    }

    return true;
}

bool SettingsManager::save() {
    if (!db) {
        return false;
    }

    // FIXED: was calling db->saveSettings() with 6 args instead of 7.
    // soundEnabled was missing, causing all subsequent args to be misaligned.
    return db->saveSettings(
        vibrationEnabled,
        soundEnabled,
        defaultHalfMinutes,
        defaultHalftimeBreakMinutes,
        gpsEnabled,
        autoSaveIntervalSeconds,
        lastMatchId
    );
}

// =====================================================
// DEFAULTS
// =====================================================

void SettingsManager::resetToDefaults() {
    vibrationEnabled = true;
    soundEnabled = true;   // FIXED: was missing — soundEnabled not initialized
    defaultHalfMinutes = MatchDurationPreset::MIN_45;
    defaultHalftimeBreakMinutes = 10;
    gpsEnabled = false;
    autoSaveIntervalSeconds = 30;
    lastMatchId = "";
}

// =====================================================
// VIBRATION
// =====================================================

bool SettingsManager::isVibrationEnabled() const {
    return vibrationEnabled;
}

void SettingsManager::setVibrationEnabled(bool enabled) {
    vibrationEnabled = enabled;
}

// FIXED: added isSoundEnabled/setSoundEnabled — soundEnabled existed in DB schema
// but was never exposed through SettingsManager, causing the 7-arg mismatch.
bool SettingsManager::isSoundEnabled() const {
    return soundEnabled;
}

void SettingsManager::setSoundEnabled(bool enabled) {
    soundEnabled = enabled;
}

// =====================================================
// MATCH DURATION DEFAULT
// =====================================================

int SettingsManager::getDefaultHalfMinutes() const {
    return defaultHalfMinutes;
}

void SettingsManager::setDefaultHalfMinutes(int minutes) {
    if (db && db->isValidHalfPreset(minutes) && minutes != MatchDurationPreset::CUSTOM) {
        defaultHalfMinutes = minutes;
        return;
    }

    defaultHalfMinutes = MatchDurationPreset::MIN_45;
}

// =====================================================
// HALFTIME BREAK DEFAULT
// =====================================================

int SettingsManager::getDefaultHalftimeBreakMinutes() const {
    return defaultHalftimeBreakMinutes;
}

void SettingsManager::setDefaultHalftimeBreakMinutes(int minutes) {
    if (minutes < 1) {
        minutes = 1;
    }

    if (minutes > 30) {
        minutes = 30;
    }

    defaultHalftimeBreakMinutes = minutes;
}

// =====================================================
// GPS
// =====================================================

bool SettingsManager::isGpsEnabled() const {
    return gpsEnabled;
}

void SettingsManager::setGpsEnabled(bool enabled) {
    gpsEnabled = enabled;
}

// =====================================================
// AUTO SAVE
// =====================================================

int SettingsManager::getAutoSaveIntervalSeconds() const {
    return autoSaveIntervalSeconds;
}

void SettingsManager::setAutoSaveIntervalSeconds(int seconds) {
    if (seconds < 5) {
        seconds = 5;
    }

    if (seconds > 300) {
        seconds = 300;
    }

    autoSaveIntervalSeconds = seconds;
}

// =====================================================
// LAST MATCH
// =====================================================

String SettingsManager::getLastMatchId() const {
    return lastMatchId;
}

void SettingsManager::setLastMatchId(const String& matchId) {
    lastMatchId = matchId;
}

// =====================================================
// MATCH CONFIG FACTORY
// =====================================================

MatchConfig SettingsManager::createDefaultMatchConfig() {
    if (!db) {
        MatchConfig config;
        config.halfMinutes = defaultHalfMinutes;
        config.totalMinutes = defaultHalfMinutes * 2;
        config.selectedHalfPreset = defaultHalfMinutes;
        config.customHalfDuration = false;
        config.halfTimeBreakMinutes = defaultHalftimeBreakMinutes;
        return config;
    }

    return db->createConfigFromHalfPreset(
        defaultHalfMinutes,
        defaultHalftimeBreakMinutes
    );
}