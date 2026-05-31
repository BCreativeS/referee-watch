#pragma once

#include <Arduino.h>

#include "AppController.h"
#include "MatchEngine.h"
#include "DatabaseManager.h"
#include "EventFormatter.h"

// =====================================================
// LogicTestSuite.h
// Referee Watch App - Repeatable Logic Tests
// =====================================================
//
// Use this before LVGL integration.
//
// Tests:
// - Match setup presets
// - Custom half duration
// - Team validation
// - Goals
// - Yellow cards
// - Red cards
// - Substitutions
// - Stoppage time
// - Event formatting
//
// =====================================================

class LogicTestSuite {
public:
    static LogicTestSuite& getInstance();

    LogicTestSuite(const LogicTestSuite&) = delete;
    LogicTestSuite& operator=(const LogicTestSuite&) = delete;

    void begin();

    bool runAll();

    bool testMatchPresetCreation();
    bool testCustomDurationCreation();
    bool testGoalLogging();
    bool testYellowCardLogging();
    bool testRedCardLogging();
    bool testSubstitutionLogging();
    bool testStoppageTime();
    bool testInvalidInputs();
    bool testEventFormatting();

private:
    LogicTestSuite();

    void printResult(
        const String& testName,
        bool passed
    );

    bool assertTrue(
        const String& testName,
        bool condition
    );

    AppController* app;
    MatchEngine* engine;
};