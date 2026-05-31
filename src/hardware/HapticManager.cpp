#include "HapticManager.h"

// =====================================================
// SINGLETON
// =====================================================

HapticManager& HapticManager::getInstance() {
    static HapticManager instance;
    return instance;
}

// =====================================================
// CONSTRUCTOR
// =====================================================

HapticManager::HapticManager() {
    initialized = false;
    enabled = true;
}

// =====================================================
// INITIALIZATION
// =====================================================

bool HapticManager::begin() {
    Wire.begin(
        BoardConfig::I2C_SDA,
        BoardConfig::I2C_SCL,
        BoardConfig::I2C_FREQUENCY
    );

    if (!drv.begin(&Wire)) {
        Serial.println("[HAPTIC] DRV2605 not detected");
        initialized = false;
        return false;
    }

    drv.selectLibrary(1);
    drv.setMode(DRV2605_MODE_INTTRIG);

    initialized = true;

    Serial.println("[HAPTIC] DRV2605 initialized");

    confirm();

    return true;
}

// =====================================================
// ENABLE / STATUS
// =====================================================

void HapticManager::enable(bool value) {
    enabled = value;
}

bool HapticManager::isEnabled() const {
    return enabled;
}

bool HapticManager::isReady() const {
    return initialized;
}

// =====================================================
// PATTERN ROUTER
// =====================================================

void HapticManager::play(HapticPattern pattern) {
    if (!enabled || !initialized) {
        return;
    }

    switch (pattern) {
        case HapticPattern::TAP:
            tap();
            break;

        case HapticPattern::CONFIRM:
            confirm();
            break;

        case HapticPattern::ERROR:
            error();
            break;

        case HapticPattern::GOAL:
            goal();
            break;

        case HapticPattern::YELLOW_CARD:
            yellowCard();
            break;

        case HapticPattern::RED_CARD:
            redCard();
            break;

        case HapticPattern::SUBSTITUTION:
            substitution();
            break;

        case HapticPattern::HALFTIME:
            halftime();
            break;

        case HapticPattern::FULLTIME:
            fulltime();
            break;

        case HapticPattern::WARNING:
            warning();
            break;
    }
}

// =====================================================
// PUBLIC PATTERN METHODS
// =====================================================

void HapticManager::tap() {
    playEffect(1);
}

void HapticManager::confirm() {
    playEffect(14);
}

void HapticManager::error() {
    playEffect(47);
}

void HapticManager::goal() {
    const uint8_t effects[] = {15, 15};
    const uint16_t pauses[] = {90, 0};

    playSequence(effects, pauses, 2);
}

void HapticManager::yellowCard() {
    const uint8_t effects[] = {21};
    const uint16_t pauses[] = {0};

    playSequence(effects, pauses, 1);
}

void HapticManager::redCard() {
    const uint8_t effects[] = {47, 47};
    const uint16_t pauses[] = {120, 0};

    playSequence(effects, pauses, 2);
}

void HapticManager::substitution() {
    playEffect(10);
}

void HapticManager::halftime() {
    const uint8_t effects[] = {52, 52};
    const uint16_t pauses[] = {150, 0};

    playSequence(effects, pauses, 2);
}

void HapticManager::fulltime() {
    const uint8_t effects[] = {52, 52, 52};
    const uint16_t pauses[] = {150, 150, 0};

    playSequence(effects, pauses, 3);
}

void HapticManager::warning() {
    playEffect(30);
}

// =====================================================
// LOW LEVEL PLAYBACK
// =====================================================

void HapticManager::playEffect(uint8_t effectId) {
    if (!enabled || !initialized) {
        return;
    }

    drv.setWaveform(0, effectId);
    drv.setWaveform(1, 0);
    drv.go();
}

void HapticManager::playSequence(
    const uint8_t* effects,
    const uint16_t* pausesMs,
    size_t count
) {
    if (!enabled || !initialized) {
        return;
    }

    for (size_t i = 0; i < count; i++) {
        playEffect(effects[i]);

        if (pausesMs[i] > 0) {
            delay(pausesMs[i]);
        }
    }
}