#include "DatabaseManager.h"
#include <time.h>
#include <esp_task_wdt.h>

bool DatabaseManager::initSD(
    int csPin,
    int sckPin,
    int mosiPin,
    int misoPin
) {
    Serial.println("[DB] Initializing SD card...");

    SPI.begin(sckPin, misoPin, mosiPin, csPin);

    if (!SD.begin(csPin)) {
        Serial.println("[DB] SD Card Mount Failed");
        return false;
    }

    if (SD.cardType() == CARD_NONE) {
        Serial.println("[DB] No SD Card Attached");
        return false;
    }

    if (!SD.exists("/matches")) {
        SD.mkdir("/matches");
    }

    if (!SD.exists("/config")) {
        SD.mkdir("/config");
    }

    Serial.printf(
        "[DB] SD Ready | Total: %llu MB\n",
        SD.totalBytes() / (1024 * 1024)
    );

    return true;
}

String DatabaseManager::getCurrentTimestamp() {
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo)) {
        return "";
    }

    char buffer[30];

    strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%dT%H:%M:%SZ",
        &timeinfo
    );

    return String(buffer);
}

String DatabaseManager::getNextMatchId() {
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo)) {
        return "match_unsynced";
    }

    char buffer[24];

    strftime(
        buffer,
        sizeof(buffer),
        "%Y%m%d_%H%M%S",
        &timeinfo
    );

    return String(buffer);
}

bool DatabaseManager::isValidJerseyColor(const String &color) {
    static const char* validColors[] = {
        "blue",
        "red",
        "yellow",
        "black",
        "white",
        "green",
        "orange",
        "purple"
    };

    for (const char* validColor : validColors) {
        if (color.equalsIgnoreCase(validColor)) {
            return true;
        }
    }

    return false;
}

uint32_t DatabaseManager::jerseyColorToHex(const String &color) {
    if (color.equalsIgnoreCase("blue")) return 0x0A84FF;
    if (color.equalsIgnoreCase("red")) return 0xFF3B30;
    if (color.equalsIgnoreCase("yellow")) return 0xFFD60A;
    if (color.equalsIgnoreCase("black")) return 0x000000;
    if (color.equalsIgnoreCase("white")) return 0xFFFFFF;
    if (color.equalsIgnoreCase("green")) return 0x32D74B;
    if (color.equalsIgnoreCase("orange")) return 0xFF9500;
    if (color.equalsIgnoreCase("purple")) return 0xBF5AF2;

    return 0x8E8E93;
}

bool DatabaseManager::isValidHalfPreset(int preset) {
    return
        preset == MatchDurationPreset::MIN_20 ||
        preset == MatchDurationPreset::MIN_25 ||
        preset == MatchDurationPreset::MIN_30 ||
        preset == MatchDurationPreset::MIN_35 ||
        preset == MatchDurationPreset::MIN_40 ||
        preset == MatchDurationPreset::MIN_45 ||
        preset == MatchDurationPreset::CUSTOM;
}

MatchConfig DatabaseManager::createConfigFromHalfPreset(
    int halfPreset,
    int halftimeBreakMinutes
) {
    MatchConfig config;

    if (!isValidHalfPreset(halfPreset) ||
        halfPreset == MatchDurationPreset::CUSTOM) {
        halfPreset = MatchDurationPreset::MIN_45;
    }

    config.halfMinutes = halfPreset;
    config.totalMinutes = halfPreset * 2;
    config.selectedHalfPreset = halfPreset;
    config.customHalfDuration = false;
    config.halfTimeBreakMinutes = halftimeBreakMinutes;
    config.stoppageTimeHalf1 = 0;
    config.stoppageTimeHalf2 = 0;

    return config;
}

MatchConfig DatabaseManager::createCustomConfig(
    int customHalfMinutes,
    int halftimeBreakMinutes
) {
    MatchConfig config;

    if (customHalfMinutes < 1) {
        customHalfMinutes = 45;
    }

    if (customHalfMinutes > 60) {
        customHalfMinutes = 60;
    }

    config.halfMinutes = customHalfMinutes;
    config.totalMinutes = customHalfMinutes * 2;
    config.selectedHalfPreset = MatchDurationPreset::CUSTOM;
    config.customHalfDuration = true;
    config.halfTimeBreakMinutes = halftimeBreakMinutes;
    config.stoppageTimeHalf1 = 0;
    config.stoppageTimeHalf2 = 0;

    return config;
}

bool DatabaseManager::isValidYellowReason(const String &reason) {
    static const char* valid[] = {
        YellowCardReason::UNSPORTING_BEHAVIOUR,
        YellowCardReason::DISSENT,
        YellowCardReason::PERSISTENT_OFFENCES,
        YellowCardReason::DELAYING_RESTART,
        YellowCardReason::REQUIRED_DISTANCE,
        YellowCardReason::ENTER_LEAVE_WITHOUT_PERMISSION,
        YellowCardReason::ENTERING_RRA,
        YellowCardReason::EXCESSIVE_TV_SIGNAL,
        YellowCardReason::OTHER
    };

    for (const char* r : valid) {
        if (reason.equalsIgnoreCase(r)) {
            return true;
        }
    }

    return false;
}

bool DatabaseManager::isValidRedReason(const String &reason) {
    static const char* valid[] = {
        RedCardReason::SERIOUS_FOUL_PLAY,
        RedCardReason::VIOLENT_CONDUCT,
        RedCardReason::BITING_OR_SPITTING,
        RedCardReason::DOGSO_HANDBALL,
        RedCardReason::DOGSO_FOUL,
        RedCardReason::OFFENSIVE_LANGUAGE,
        RedCardReason::SECOND_CAUTION,
        RedCardReason::OTHER
    };

    for (const char* r : valid) {
        if (reason.equalsIgnoreCase(r)) {
            return true;
        }
    }

    return false;
}

const char* DatabaseManager::yellowReasonLabel(const String &reason) {
    if (reason == YellowCardReason::UNSPORTING_BEHAVIOUR) return "Unsporting";
    if (reason == YellowCardReason::DISSENT) return "Dissent";
    if (reason == YellowCardReason::PERSISTENT_OFFENCES) return "Persistent";
    if (reason == YellowCardReason::DELAYING_RESTART) return "Delay Restart";
    if (reason == YellowCardReason::REQUIRED_DISTANCE) return "Required Dist.";
    if (reason == YellowCardReason::ENTER_LEAVE_WITHOUT_PERMISSION) return "No Permission";
    if (reason == YellowCardReason::ENTERING_RRA) return "Entering RRA";
    if (reason == YellowCardReason::EXCESSIVE_TV_SIGNAL) return "TV Signal";

    return "Other";
}

const char* DatabaseManager::redReasonLabel(const String &reason) {
    if (reason == RedCardReason::SERIOUS_FOUL_PLAY) return "Serious Foul";
    if (reason == RedCardReason::VIOLENT_CONDUCT) return "Violent";
    if (reason == RedCardReason::BITING_OR_SPITTING) return "Bite/Spit";
    if (reason == RedCardReason::DOGSO_HANDBALL) return "DOGSO Hand";
    if (reason == RedCardReason::DOGSO_FOUL) return "DOGSO Foul";
    if (reason == RedCardReason::OFFENSIVE_LANGUAGE) return "Offensive";
    if (reason == RedCardReason::SECOND_CAUTION) return "2nd Yellow";

    return "Other";
}

// FIXED: DynamicJsonDocument (ArduinoJson v6) → JsonDocument (ArduinoJson v7).
// v7 removed DynamicJsonDocument; JsonDocument allocates on the heap automatically.
bool DatabaseManager::writeJsonSafely(
    const String &path,
    JsonDocument &doc
) {
    String tempPath = path + ".tmp";

    File tempFile = SD.open(tempPath, FILE_WRITE);

    if (!tempFile) {
        Serial.println("[DB] Failed opening temp file");
        return false;
    }

    size_t written = serializeJson(doc, tempFile);
    tempFile.close();

    if (written == 0) {
        Serial.println("[DB] JSON write failed");
        SD.remove(tempPath);
        return false;
    }

    if (SD.exists(path)) {
        SD.remove(path);
    }

    if (!SD.rename(tempPath, path)) {
        Serial.println("[DB] Temp rename failed");
        SD.remove(tempPath);
        return false;
    }

    return true;
}

bool DatabaseManager::saveMatch(
    String matchId,
    MatchConfig config,
    Team home,
    Team away,
    Event* events,
    int eventCount,
    Timeline timeline,
    String status
) {
    if (eventCount < 0 || eventCount > MAX_EVENTS) {
        Serial.println("[DB] Invalid event count");
        return false;
    }

    if (!isValidHalfPreset(config.selectedHalfPreset)) {
        config = createConfigFromHalfPreset(
            MatchDurationPreset::MIN_45,
            config.halfTimeBreakMinutes
        );
    }

    if (config.customHalfDuration) {
        if (config.halfMinutes < 1) config.halfMinutes = 45;
        if (config.halfMinutes > 60) config.halfMinutes = 60;

        config.totalMinutes = config.halfMinutes * 2;
        config.selectedHalfPreset = MatchDurationPreset::CUSTOM;
    } else {
        config.halfMinutes = config.selectedHalfPreset;
        config.totalMinutes = config.halfMinutes * 2;
    }

    if (!isValidJerseyColor(home.jerseyColor)) {
        home.jerseyColor = "blue";
    }

    if (!isValidJerseyColor(away.jerseyColor)) {
        away.jerseyColor = "red";
    }

    // FIXED: DynamicJsonDocument doc(JSON_CAPACITY) → JsonDocument doc (ArduinoJson v7 API)
    JsonDocument doc;
    String now = getCurrentTimestamp();

    doc["schemaVersion"] = CURRENT_SCHEMA_VERSION;
    doc["matchId"] = matchId;
    doc["createdAt"] = now;
    doc["lastModified"] = now;
    doc["timeSynced"] = now != "";
    doc["status"] = status;

    // FIXED: createNestedObject() → doc["key"].to<JsonObject>() (ArduinoJson v7 API)
    JsonObject cfg = doc["matchConfig"].to<JsonObject>();

    cfg["totalMinutes"] = config.totalMinutes;
    cfg["halfMinutes"] = config.halfMinutes;
    cfg["selectedHalfPreset"] = config.selectedHalfPreset;
    cfg["customHalfDuration"] = config.customHalfDuration;
    cfg["halfTimeBreakMinutes"] = config.halfTimeBreakMinutes;
    cfg["stoppageTimeHalf1"] = config.stoppageTimeHalf1;
    cfg["stoppageTimeHalf2"] = config.stoppageTimeHalf2;

    JsonObject teams = doc["teams"].to<JsonObject>();

    JsonObject homeObj = teams["home"].to<JsonObject>();
    homeObj["name"] = home.name;
    homeObj["jerseyColor"] = home.jerseyColor;
    homeObj["score"] = home.score;
    homeObj["yellowCards"] = home.yellowCards;
    homeObj["redCards"] = home.redCards;
    homeObj["substitutions"] = home.substitutions;

    JsonObject awayObj = teams["away"].to<JsonObject>();
    awayObj["name"] = away.name;
    awayObj["jerseyColor"] = away.jerseyColor;
    awayObj["score"] = away.score;
    awayObj["yellowCards"] = away.yellowCards;
    awayObj["redCards"] = away.redCards;
    awayObj["substitutions"] = away.substitutions;

    // FIXED: createNestedArray() → doc["key"].to<JsonArray>() (ArduinoJson v7 API)
    JsonArray eventArray = doc["events"].to<JsonArray>();

    for (int i = 0; i < eventCount; i++) {
        // FIXED: eventArray.createNestedObject() → eventArray.add<JsonObject>() (ArduinoJson v7 API)
        JsonObject eventObj = eventArray.add<JsonObject>();

        eventObj["id"] = events[i].id;
        eventObj["half"] = events[i].half;
        eventObj["minute"] = events[i].minute;
        eventObj["type"] = events[i].type;
        eventObj["team"] = events[i].team;
        eventObj["reason"] = events[i].reason;

        if (events[i].type == EventType::SUBSTITUTION) {
            eventObj["playerOut"] = events[i].playerOut;
            eventObj["playerIn"] = events[i].playerIn;
        } else {
            eventObj["jerseyNumber"] = events[i].jerseyNumber;
        }
    }

    // FIXED: createNestedObject() → doc["key"].to<JsonObject>() (ArduinoJson v7 API)
    JsonObject timelineObj = doc["timeline"].to<JsonObject>();

    timelineObj["matchStarted"] = timeline.matchStarted;
    timelineObj["half1Ended"] = timeline.half1Ended;
    timelineObj["half2Started"] = timeline.half2Started;
    timelineObj["matchEnded"] = timeline.matchEnded;

    String path = "/matches/" + matchId + ".json";

    if (!writeJsonSafely(path, doc)) {
        Serial.println("[DB] Match save failed");
        return false;
    }

    Serial.printf(
        "[DB] Match saved: %s | Events: %d\n",
        matchId.c_str(),
        eventCount
    );

    return true;
}

bool DatabaseManager::loadMatch(
    String matchId,
    MatchConfig &config,
    Team &home,
    Team &away,
    Event* events,
    int &eventCount,
    Timeline &timeline,
    String &status
) {
    String path = "/matches/" + matchId + ".json";

    if (!SD.exists(path)) {
        Serial.println("[DB] Match file not found");
        return false;
    }

    File file = SD.open(path, FILE_READ);

    if (!file) {
        Serial.println("[DB] Failed opening match file");
        return false;
    }

    // FIXED: DynamicJsonDocument doc(JSON_CAPACITY) → JsonDocument doc (ArduinoJson v7 API)
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.printf(
            "[DB] JSON parse failed: %s\n",
            error.c_str()
        );
        return false;
    }

    int schemaVersion = doc["schemaVersion"] | 0;

    if (schemaVersion > CURRENT_SCHEMA_VERSION) {
        Serial.println("[DB] Unsupported schema version");
        return false;
    }

    status = doc["status"] | "unknown";

    JsonObject cfg = doc["matchConfig"];

    config.totalMinutes = cfg["totalMinutes"] | 90;
    config.halfMinutes = cfg["halfMinutes"] | 45;
    config.selectedHalfPreset = cfg["selectedHalfPreset"] | MatchDurationPreset::MIN_45;
    config.customHalfDuration = cfg["customHalfDuration"] | false;
    config.halfTimeBreakMinutes = cfg["halfTimeBreakMinutes"] | 10;
    config.stoppageTimeHalf1 = cfg["stoppageTimeHalf1"] | 0;
    config.stoppageTimeHalf2 = cfg["stoppageTimeHalf2"] | 0;

    if (!isValidHalfPreset(config.selectedHalfPreset)) {
        config.selectedHalfPreset = MatchDurationPreset::MIN_45;
        config.customHalfDuration = false;
    }

    if (config.customHalfDuration) {
        if (config.halfMinutes < 1) config.halfMinutes = 45;
        if (config.halfMinutes > 60) config.halfMinutes = 60;
        config.totalMinutes = config.halfMinutes * 2;
        config.selectedHalfPreset = MatchDurationPreset::CUSTOM;
    } else {
        config.halfMinutes = config.selectedHalfPreset;
        config.totalMinutes = config.halfMinutes * 2;
    }

    JsonObject homeObj = doc["teams"]["home"];

    home.name = homeObj["name"].as<String>();
    home.jerseyColor = homeObj["jerseyColor"].as<String>();
    home.score = homeObj["score"] | 0;
    home.yellowCards = homeObj["yellowCards"] | 0;
    home.redCards = homeObj["redCards"] | 0;
    home.substitutions = homeObj["substitutions"] | 0;

    if (!isValidJerseyColor(home.jerseyColor)) {
        home.jerseyColor = "blue";
    }

    JsonObject awayObj = doc["teams"]["away"];

    away.name = awayObj["name"].as<String>();
    away.jerseyColor = awayObj["jerseyColor"].as<String>();
    away.score = awayObj["score"] | 0;
    away.yellowCards = awayObj["yellowCards"] | 0;
    away.redCards = awayObj["redCards"] | 0;
    away.substitutions = awayObj["substitutions"] | 0;

    if (!isValidJerseyColor(away.jerseyColor)) {
        away.jerseyColor = "red";
    }

    eventCount = 0;

    JsonArray eventArray = doc["events"].as<JsonArray>();

    for (JsonObject eventObj : eventArray) {
        if (eventCount >= MAX_EVENTS) {
            break;
        }

        events[eventCount].id = eventObj["id"] | 0;
        events[eventCount].half = eventObj["half"] | 1;
        events[eventCount].minute = eventObj["minute"] | 0;
        events[eventCount].type = eventObj["type"].as<String>();
        events[eventCount].team = eventObj["team"].as<String>();
        events[eventCount].reason = eventObj["reason"].as<String>();

        if (events[eventCount].type == EventType::SUBSTITUTION) {
            events[eventCount].playerOut = eventObj["playerOut"] | 0;
            events[eventCount].playerIn = eventObj["playerIn"] | 0;
            events[eventCount].jerseyNumber = 0;
        } else {
            events[eventCount].jerseyNumber = eventObj["jerseyNumber"] | 0;
            events[eventCount].playerOut = 0;
            events[eventCount].playerIn = 0;
        }

        eventCount++;
    }

    JsonObject timelineObj = doc["timeline"];

    timeline.matchStarted = timelineObj["matchStarted"].as<String>();
    timeline.half1Ended = timelineObj["half1Ended"].as<String>();
    timeline.half2Started = timelineObj["half2Started"].as<String>();
    timeline.matchEnded = timelineObj["matchEnded"].as<String>();

    Serial.printf(
        "[DB] Match loaded: %s | Events: %d\n",
        matchId.c_str(),
        eventCount
    );

    return true;
}

bool DatabaseManager::deleteMatch(String matchId) {
    String path = "/matches/" + matchId + ".json";

    if (!SD.exists(path)) {
        return false;
    }

    bool removed = SD.remove(path);

    if (removed) {
        Serial.printf(
            "[DB] Match deleted: %s\n",
            matchId.c_str()
        );
    }

    return removed;
}

bool DatabaseManager::matchExists(String matchId) {
    return SD.exists(
        "/matches/" +
        matchId +
        ".json"
    );
}

void DatabaseManager::listAllMatches() {
    File root = SD.open("/matches");

    if (!root) {
        Serial.println("[DB] Failed to open /matches");
        return;
    }

    File file = root.openNextFile();

    while (file) {
        if (!file.isDirectory()) {
            Serial.println(file.name());
        }

        File nextFile = root.openNextFile();

        file.close();

        file = nextFile;
    }

    root.close();
}

bool DatabaseManager::saveInProgressMatch(
    MatchConfig config,
    Team home,
    Team away,
    Event* events,
    int eventCount,
    Timeline timeline
) {
    return saveMatch(
        "in_progress",
        config,
        home,
        away,
        events,
        eventCount,
        timeline,
        "in_progress"
    );
}

bool DatabaseManager::loadInProgressMatch(
    MatchConfig &config,
    Team &home,
    Team &away,
    Event* events,
    int &eventCount,
    Timeline &timeline
) {
    String status;

    return loadMatch(
        "in_progress",
        config,
        home,
        away,
        events,
        eventCount,
        timeline,
        status
    );
}

bool DatabaseManager::clearInProgressMatch() {
    if (SD.exists("/matches/in_progress.json")) {
        return SD.remove("/matches/in_progress.json");
    }

    return true;
}

bool DatabaseManager::hasInProgressMatch() {
    return SD.exists("/matches/in_progress.json");
}

bool DatabaseManager::loadSettings(
    bool &vibrationEnabled,
    bool &soundEnabled,
    int &defaultMatchMinutes,
    int &defaultHalfTimeMinutes,
    bool &gpsEnabled,
    int &autoSaveInterval,
    String &lastMatchId
) {
    String path = "/config/settings.json";

    if (!SD.exists(path)) {
        vibrationEnabled = true;
        soundEnabled = true;
        defaultMatchMinutes = MatchDurationPreset::MIN_45;
        defaultHalfTimeMinutes = 10;
        gpsEnabled = false;
        autoSaveInterval = 30;
        lastMatchId = "";
        return true;
    }

    File file = SD.open(path, FILE_READ);

    if (!file) {
        Serial.println("[DB] Failed opening settings");
        return false;
    }

    // FIXED: DynamicJsonDocument doc(2048) → JsonDocument doc (ArduinoJson v7 API)
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.printf(
            "[DB] Settings parse failed: %s\n",
            error.c_str()
        );
        return false;
    }

    vibrationEnabled = doc["vibrationEnabled"] | true;
    soundEnabled = doc["soundEnabled"] | true;
    defaultMatchMinutes = doc["defaultMatchMinutes"] | MatchDurationPreset::MIN_45;
    defaultHalfTimeMinutes = doc["defaultHalfTimeMinutes"] | 10;
    gpsEnabled = doc["gpsEnabled"] | false;
    autoSaveInterval = doc["autoSaveInterval"] | 30;
    lastMatchId = doc["lastMatchId"].as<String>();

    return true;
}

bool DatabaseManager::saveSettings(
    bool vibrationEnabled,
    bool soundEnabled,
    int defaultMatchMinutes,
    int defaultHalfTimeMinutes,
    bool gpsEnabled,
    int autoSaveInterval,
    String lastMatchId
) {
    // FIXED: DynamicJsonDocument doc(2048) → JsonDocument doc (ArduinoJson v7 API)
    JsonDocument doc;
    String now = getCurrentTimestamp();

    doc["schemaVersion"] = CURRENT_SCHEMA_VERSION;
    doc["vibrationEnabled"] = vibrationEnabled;
    doc["soundEnabled"] = soundEnabled;
    doc["defaultMatchMinutes"] = defaultMatchMinutes;
    doc["defaultHalfTimeMinutes"] = defaultHalfTimeMinutes;
    doc["gpsEnabled"] = gpsEnabled;
    doc["autoSaveInterval"] = autoSaveInterval;
    doc["lastMatchId"] = lastMatchId;
    doc["lastUpdated"] = now;
    doc["timeSynced"] = now != "";

    return writeJsonSafely(
        "/config/settings.json",
        doc
    );
}

bool DatabaseManager::batteryBackupSave(
    MatchConfig config,
    Team home,
    Team away,
    Event* events,
    int eventCount,
    Timeline timeline
) {
    Serial.println("[DB] Battery critical! Saving...");

    esp_task_wdt_reset();

    bool saveMain = saveInProgressMatch(
        config,
        home,
        away,
        events,
        eventCount,
        timeline
    );

    String backupId =
        "battery_backup_" +
        getNextMatchId();

    bool saveBackup = saveMatch(
        backupId,
        config,
        home,
        away,
        events,
        eventCount,
        timeline,
        "critical_backup"
    );

    if (saveMain && saveBackup) {
        Serial.println("[DB] Battery backup successful");
        return true;
    }

    Serial.println("[DB] Battery backup FAILED");
    return false;
}