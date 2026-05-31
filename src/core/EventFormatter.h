#pragma once

#include <Arduino.h>
#include "DatabaseManager.h"

// =====================================================
// EventFormatter.h
// Referee Watch App - Event Display Formatter
// =====================================================
//
// Converts stored match events into short UI-friendly text.
//
// Example output:
// 43' GOAL #10 HOME
// 52' YC #7 AWAY - Dissent
// 61' RC #4 HOME - DOGSO Foul
// 70' SUB #10→#17 AWAY
//
// =====================================================

class EventFormatter {
public:
    static String formatEvent(
        const Event& event,
        DatabaseManager* db = nullptr
    );

    static String formatEventShort(
        const Event& event
    );

    static String formatTeamLabel(
        const String& team
    );

    static String formatMinute(
        int minute
    );

    static String formatScore(
        const Team& home,
        const Team& away
    );

    static String formatMatchSummary(
        const Team& home,
        const Team& away
    );

    static String formatHalfLabel(
        int half
    );

private:
    static String formatReasonLabel(
        const Event& event,
        DatabaseManager* db
    );
};