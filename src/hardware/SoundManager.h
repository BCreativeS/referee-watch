#pragma once

#include <Arduino.h>
#include "BoardConfig.h"

enum class SoundPattern {
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

class SoundManager {
public:
    static SoundManager& getInstance();

    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    bool begin();

    void enable(bool enabled);
    bool isEnabled() const;
    bool isReady() const;

    void play(SoundPattern pattern);

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
    SoundManager();

    void playTone(int frequencyHz, int durationMs, int volume);
    void silence(int durationMs);

    bool initialized;
    bool enabled;
};