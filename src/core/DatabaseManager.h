#pragma once

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>

#define CURRENT_SCHEMA_VERSION 1
#define MAX_EVENTS 100
// FIXED: JSON_CAPACITY removed — ArduinoJson v7 uses JsonDocument which
// allocates dynamically from the heap; a fixed capacity is no longer needed.

namespace MatchDurationPreset {
    constexpr int CUSTOM = 0;
    constexpr int MIN_20 = 20;
    constexpr int MIN_25 = 25;
    constexpr int MIN_30 = 30;
    constexpr int MIN_35 = 35;
    constexpr int MIN_40 = 40;
    constexpr int MIN_45 = 45;
}

namespace EventType {
    constexpr const char* GOAL = "goal";
    constexpr const char* YELLOW = "yellow";
    constexpr const char* RED = "red";
    constexpr const char* SUBSTITUTION = "substitution";
}

namespace TeamSide {
    constexpr const char* HOME = "home";
    constexpr const char* AWAY = "away";
}

namespace YellowCardReason {
    constexpr const char* UNSPORTING_BEHAVIOUR = "unsporting_behaviour";
    constexpr const char* DISSENT = "dissent";
    constexpr const char* PERSISTENT_OFFENCES = "persistent_offences";
    constexpr const char* DELAYING_RESTART = "delaying_restart";
    constexpr const char* REQUIRED_DISTANCE = "required_distance";
    constexpr const char* ENTER_LEAVE_WITHOUT_PERMISSION = "enter_leave_without_permission";
    constexpr const char* ENTERING_RRA = "entering_rra";
    constexpr const char* EXCESSIVE_TV_SIGNAL = "excessive_tv_signal";
    constexpr const char* OTHER = "other";
}

namespace RedCardReason {
    constexpr const char* SERIOUS_FOUL_PLAY = "serious_foul_play";
    constexpr const char* VIOLENT_CONDUCT = "violent_conduct";
    constexpr const char* BITING_OR_SPITTING = "biting_or_spitting";
    constexpr const char* DOGSO_HANDBALL = "dogso_handball";
    constexpr const char* DOGSO_FOUL = "dogso_foul";
    constexpr const char* OFFENSIVE_LANGUAGE = "offensive_language";
    constexpr const char* SECOND_CAUTION = "second_caution";
    constexpr const char* OTHER = "other";
}

struct MatchConfig {
    int totalMinutes = 90;
    int halfMinutes = 45;
    int selectedHalfPreset = MatchDurationPreset::MIN_45;
    bool customHalfDuration = false;
    int halfTimeBreakMinutes = 10;
    int stoppageTimeHalf1 = 0;
    int stoppageTimeHalf2 = 0;
};

struct Team {
    String name = "";
    String jerseyColor = "blue";
    int score = 0;
    int yellowCards = 0;
    int redCards = 0;
    int substitutions = 0;
};

struct Event {
    int id = 0;
    int half = 1;
    int minute = 0;
    String type = "";
    String team = "";
    int jerseyNumber = 0;
    int playerOut = 0;
    int playerIn = 0;
    String reason = "";
};

struct Timeline {
    String matchStarted = "";
    String half1Ended = "";
    String half2Started = "";
    String matchEnded = "";
};

class DatabaseManager {
public:
    bool initSD(
        int csPin,
        int sckPin = 18,
        int mosiPin = 19,
        int misoPin = 23
    );

    String getCurrentTimestamp();
    String getNextMatchId();

    bool isValidJerseyColor(const String &color);
    uint32_t jerseyColorToHex(const String &color);

    bool isValidHalfPreset(int preset);

    MatchConfig createConfigFromHalfPreset(
        int halfPreset,
        int halftimeBreakMinutes
    );

    MatchConfig createCustomConfig(
        int customHalfMinutes,
        int halftimeBreakMinutes
    );

    bool isValidYellowReason(const String &reason);
    bool isValidRedReason(const String &reason);

    const char* yellowReasonLabel(const String &reason);
    const char* redReasonLabel(const String &reason);

    bool saveMatch(
        String matchId,
        MatchConfig config,
        Team home,
        Team away,
        Event* events,
        int eventCount,
        Timeline timeline,
        String status
    );

    bool loadMatch(
        String matchId,
        MatchConfig &config,
        Team &home,
        Team &away,
        Event* events,
        int &eventCount,
        Timeline &timeline,
        String &status
    );

    bool deleteMatch(String matchId);
    bool matchExists(String matchId);
    void listAllMatches();

    bool saveInProgressMatch(
        MatchConfig config,
        Team home,
        Team away,
        Event* events,
        int eventCount,
        Timeline timeline
    );

    bool loadInProgressMatch(
        MatchConfig &config,
        Team &home,
        Team &away,
        Event* events,
        int &eventCount,
        Timeline &timeline
    );

    bool clearInProgressMatch();
    bool hasInProgressMatch();

    bool loadSettings(
        bool &vibrationEnabled,
        bool &soundEnabled,
        int &defaultMatchMinutes,
        int &defaultHalfTimeMinutes,
        bool &gpsEnabled,
        int &autoSaveInterval,
        String &lastMatchId
    );

    bool saveSettings(
        bool vibrationEnabled,
        bool soundEnabled,
        int defaultMatchMinutes,
        int defaultHalfTimeMinutes,
        bool gpsEnabled,
        int autoSaveInterval,
        String lastMatchId
    );

    bool batteryBackupSave(
        MatchConfig config,
        Team home,
        Team away,
        Event* events,
        int eventCount,
        Timeline timeline
    );

private:
    // FIXED: DynamicJsonDocument (v6 API) → JsonDocument (v7 API).
    // ArduinoJson v7 removed DynamicJsonDocument; JsonDocument auto-manages memory.
    bool writeJsonSafely(
        const String &path,
        JsonDocument &doc
    );
};