#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_DRV2605.h>
#include "BoardConfig.h"

// =====================================================
// HapticManager.h
// Referee Watch App - Haptic Feedback System
// =====================================================
//
// Uses DRV2605 haptic driver.
//
// Provides consistent vibration patterns for:
// - tap feedback
// - confirmations
// - errors
// - goals
// - cards
// - substitutions
// - halftime
// - fulltime
//
// =====================================================

enum class HapticPattern {
    TAP,
    CONFIRM,
    ERROR,
    GOAL,
    YELLOW_CARD,
    RED_CARD,
    SUBSTITUTION,
    HALFTIME,
    FULLTIME,
    WARNING
};

class HapticManager {
public:
    static HapticManager& getInstance();

    HapticManager(const HapticManager&) = delete;
    HapticManager& operator=(const HapticManager&) = delete;

    bool begin();

    void enable(bool enabled);
    bool isEnabled() const;
    bool isReady() const;

    void play(HapticPattern pattern);

    void tap();
    void confirm();
    void error();
    void goal();
    void yellowCard();
    void redCard();
    void substitution();
    void halftime();
    void fulltime();
    void warning();

private:
    HapticManager();

    void playEffect(uint8_t effectId);
    void playSequence(
        const uint8_t* effects,
        const uint16_t* pausesMs,
        size_t count
    );

    Adafruit_DRV2605 drv;

    bool initialized;
    bool enabled;
};