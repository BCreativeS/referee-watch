#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <XPowersLib.h>

#include "BoardConfig.h"
#include "DatabaseManager.h"

enum class BatteryState {
    UNKNOWN,
    NORMAL,
    LOW_BATTERY,
    CRITICAL_BATTERY,
    CHARGING,
    FULL
};

class BatteryManager {
public:
    static BatteryManager& getInstance();

    BatteryManager(const BatteryManager&) = delete;
    BatteryManager& operator=(const BatteryManager&) = delete;

    bool begin(DatabaseManager* databaseManager);

    void update();

    float getVoltage();
    int getPercent();

    bool isCharging();
    bool isBatteryConnected();

    BatteryState getState();
    String getStateText();

    void setLowPercentThreshold(int percent);
    void setCriticalPercentThreshold(int percent);

    bool shouldTriggerEmergencySave();
    void clearEmergencySaveFlag();

private:
    BatteryManager();

    void updateState();
    int estimatePercentFromVoltage(float voltage);

    XPowersAXP2101 pmu;
    DatabaseManager* db;

    bool initialized;
    bool emergencySaveTriggered;

    float batteryVoltage;
    int batteryPercent;

    bool charging;
    bool batteryConnected;

    int lowPercentThreshold;
    int criticalPercentThreshold;

    BatteryState state;

    unsigned long lastUpdateMs;
    int updateIntervalMs;
};