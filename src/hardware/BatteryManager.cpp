#include "BatteryManager.h"

BatteryManager& BatteryManager::getInstance() {
    static BatteryManager instance;
    return instance;
}

BatteryManager::BatteryManager() {
    db = nullptr;

    initialized = false;
    emergencySaveTriggered = false;

    batteryVoltage = 0.0f;
    batteryPercent = 0;

    charging = false;
    batteryConnected = false;

    lowPercentThreshold = 20;
    criticalPercentThreshold = 8;

    state = BatteryState::UNKNOWN;

    lastUpdateMs = 0;
    updateIntervalMs = 5000;
}

bool BatteryManager::begin(DatabaseManager* databaseManager) {
    db = databaseManager;

    bool ok = pmu.begin(
        Wire,
        AXP2101_SLAVE_ADDRESS,
        BoardConfig::I2C_SDA,
        BoardConfig::I2C_SCL
    );

    if (!ok) {
        Serial.println("[BATTERY] AXP2101 PMU not detected");
        initialized = false;
        return false;
    }

    initialized = true;

    updateState();

    Serial.println("[BATTERY] PMU initialized");

    return true;
}

void BatteryManager::update() {
    if (!initialized) {
        return;
    }

    unsigned long now = millis();

    if (now - lastUpdateMs < (unsigned long)updateIntervalMs) {
        return;
    }

    lastUpdateMs = now;

    updateState();
}

void BatteryManager::updateState() {
    batteryConnected = pmu.isBatteryConnect();
    charging = pmu.isCharging();

    if (!batteryConnected) {
        batteryVoltage = 0.0f;
        batteryPercent = 0;
        state = BatteryState::UNKNOWN;
        return;
    }

    batteryVoltage = pmu.getBattVoltage() / 1000.0f;
    batteryPercent = pmu.getBatteryPercent();

    if (batteryPercent < 0 || batteryPercent > 100) {
        batteryPercent = estimatePercentFromVoltage(batteryVoltage);
    }

    if (charging) {
        state = BatteryState::CHARGING;
        return;
    }

    if (batteryPercent >= 95) {
        state = BatteryState::FULL;
        return;
    }

    if (batteryPercent <= criticalPercentThreshold) {
        state = BatteryState::CRITICAL_BATTERY;

        if (!emergencySaveTriggered) {
            emergencySaveTriggered = true;
        }

        return;
    }

    if (batteryPercent <= lowPercentThreshold) {
        state = BatteryState::LOW_BATTERY;
        return;
    }

    state = BatteryState::NORMAL;
}

float BatteryManager::getVoltage() {
    return batteryVoltage;
}

int BatteryManager::getPercent() {
    return batteryPercent;
}

bool BatteryManager::isCharging() {
    return charging;
}

bool BatteryManager::isBatteryConnected() {
    return batteryConnected;
}

BatteryState BatteryManager::getState() {
    return state;
}

String BatteryManager::getStateText() {
    switch (state) {
        case BatteryState::NORMAL:
            return "NORMAL";

        case BatteryState::LOW_BATTERY:
            return "LOW";

        case BatteryState::CRITICAL_BATTERY:
            return "CRITICAL";

        case BatteryState::CHARGING:
            return "CHARGING";

        case BatteryState::FULL:
            return "FULL";

        case BatteryState::UNKNOWN:
        default:
            return "UNKNOWN";
    }
}

void BatteryManager::setLowPercentThreshold(int percent) {
    if (percent < 5) percent = 5;
    if (percent > 50) percent = 50;

    lowPercentThreshold = percent;
}

void BatteryManager::setCriticalPercentThreshold(int percent) {
    if (percent < 1) percent = 1;
    if (percent > 20) percent = 20;

    criticalPercentThreshold = percent;
}

bool BatteryManager::shouldTriggerEmergencySave() {
    return emergencySaveTriggered;
}

void BatteryManager::clearEmergencySaveFlag() {
    emergencySaveTriggered = false;
}

int BatteryManager::estimatePercentFromVoltage(float voltage) {
    if (voltage >= 4.20f) return 100;
    if (voltage >= 4.10f) return 90;
    if (voltage >= 4.00f) return 80;
    if (voltage >= 3.90f) return 70;
    if (voltage >= 3.80f) return 60;
    if (voltage >= 3.75f) return 50;
    if (voltage >= 3.70f) return 40;
    if (voltage >= 3.65f) return 30;
    if (voltage >= 3.55f) return 20;
    if (voltage >= 3.45f) return 10;

    return 5;
}