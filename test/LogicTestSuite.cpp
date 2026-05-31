#include "LogicTestSuite.h"

// =====================================================
// SINGLETON
// =====================================================

LogicTestSuite& LogicTestSuite::getInstance() {
    static LogicTestSuite instance;
    return instance;
}

// =====================================================
// CONSTRUCTOR
// =====================================================

LogicTestSuite::LogicTestSuite() {
    app = &AppController::getInstance();
    engine = &MatchEngine::getInstance();
}

// =====================================================
// INIT
// =====================================================

void LogicTestSuite::begin() {
    Serial.println("[TEST SUITE] Ready");
}

// =====================================================
// RUN ALL TESTS
// =====================================================

bool LogicTestSuite::runAll() {
    Serial.println();
    Serial.println("======================================");
    Serial.println(" Running Referee Watch Logic Tests     ");
    Serial.println("======================================");

    bool passed = true;

    passed &= testMatchPresetCreation();
    passed &= testCustomDurationCreation();
    passed &= testGoalLogging();
    passed &= testYellowCardLogging();
    passed &= testRedCardLogging();
    passed &= testSubstitutionLogging();
    passed &= testStoppageTime();
    passed &= testInvalidInputs();
    passed &= testEventFormatting();

    Serial.println("======================================");

    if (passed) {
        Serial.println("[TEST SUITE] ALL TESTS PASSED");
    } else {
        Serial.println("[TEST SUITE] SOME TESTS FAILED");
    }

    Serial.println("======================================");
    Serial.println();

    return passed;
}

// =====================================================
// TEST: MATCH PRESET CREATION
// =====================================================

bool LogicTestSuite::testMatchPresetCreation() {
    const String testName = "Match preset creation";

    engine->resetMatch();

    bool created = app->createNewMatch(
        MatchDurationPreset::MIN_25,
        5,
        "Home FC",
        "blue",
        "Away FC",
        "red"
    );

    MatchConfig config = app->getMatchConfig();

    bool passed =
        created &&
        config.halfMinutes == 25 &&
        config.totalMinutes == 50 &&
        config.selectedHalfPreset == MatchDurationPreset::MIN_25 &&
        config.customHalfDuration == false &&
        config.halfTimeBreakMinutes == 5;

    printResult(testName, passed);
    return passed;
}

// =====================================================
// TEST: CUSTOM DURATION CREATION
// =====================================================

bool LogicTestSuite::testCustomDurationCreation() {
    const String testName = "Custom duration creation";

    engine->resetMatch();

    bool created = app->createCustomDurationMatch(
        18,
        4,
        "Blue Team",
        "blue",
        "Red Team",
        "red"
    );

    MatchConfig config = app->getMatchConfig();

    bool passed =
        created &&
        config.halfMinutes == 18 &&
        config.totalMinutes == 36 &&
        config.selectedHalfPreset == MatchDurationPreset::CUSTOM &&
        config.customHalfDuration == true &&
        config.halfTimeBreakMinutes == 4;

    printResult(testName, passed);
    return passed;
}

// =====================================================
// TEST: GOAL LOGGING
// =====================================================

bool LogicTestSuite::testGoalLogging() {
    const String testName = "Goal logging";

    engine->resetMatch();

    app->createNewMatch(
        MatchDurationPreset::MIN_20,
        5,
        "Home",
        "blue",
        "Away",
        "red"
    );

    app->startMatch();

    bool logged = app->addGoal(
        TeamSide::HOME,
        10
    );

    Team home = app->getHomeTeam();

    bool passed =
        logged &&
        home.score == 1 &&
        app->getEventCount() == 1;

    printResult(testName, passed);
    return passed;
}

// =====================================================
// TEST: YELLOW CARD LOGGING
// =====================================================

bool LogicTestSuite::testYellowCardLogging() {
    const String testName = "Yellow card logging";

    engine->resetMatch();

    app->createNewMatch(
        MatchDurationPreset::MIN_20,
        5,
        "Home",
        "blue",
        "Away",
        "red"
    );

    app->startMatch();

    bool logged = app->addYellowCard(
        TeamSide::AWAY,
        7,
        YellowCardReason::DISSENT
    );

    Team away = app->getAwayTeam();

    bool passed =
        logged &&
        away.yellowCards == 1 &&
        app->getEventCount() == 1;

    printResult(testName, passed);
    return passed;
}

// =====================================================
// TEST: RED CARD LOGGING
// =====================================================

bool LogicTestSuite::testRedCardLogging() {
    const String testName = "Red card logging";

    engine->resetMatch();

    app->createNewMatch(
        MatchDurationPreset::MIN_30,
        10,
        "Home",
        "blue",
        "Away",
        "red"
    );

    app->startMatch();

    bool logged = app->addRedCard(
        TeamSide::HOME,
        4,
        RedCardReason::DOGSO_FOUL
    );

    Team home = app->getHomeTeam();

    bool passed =
        logged &&
        home.redCards == 1 &&
        app->getEventCount() == 1;

    printResult(testName, passed);
    return passed;
}

// =====================================================
// TEST: SUBSTITUTION LOGGING
// =====================================================

bool LogicTestSuite::testSubstitutionLogging() {
    const String testName = "Substitution logging";

    engine->resetMatch();

    app->createNewMatch(
        MatchDurationPreset::MIN_35,
        10,
        "Home",
        "blue",
        "Away",
        "red"
    );

    app->startMatch();

    bool logged = app->addSubstitution(
        TeamSide::AWAY,
        10,
        17
    );

    Team away = app->getAwayTeam();

    bool passed =
        logged &&
        away.substitutions == 1 &&
        app->getEventCount() == 1;

    printResult(testName, passed);
    return passed;
}

// =====================================================
// TEST: STOPPAGE TIME
// =====================================================

bool LogicTestSuite::testStoppageTime() {
    const String testName = "Stoppage time";

    engine->resetMatch();

    app->createNewMatch(
        MatchDurationPreset::MIN_20,
        5,
        "Home",
        "blue",
        "Away",
        "red"
    );

    app->startMatch();

    app->addStoppageSeconds(30);
    app->addStoppageMinutes(1);

    bool passed =
        engine->getStoppageSeconds() == 90;

    printResult(testName, passed);
    return passed;
}

// =====================================================
// TEST: INVALID INPUTS
// =====================================================

bool LogicTestSuite::testInvalidInputs() {
    const String testName = "Invalid inputs";

    engine->resetMatch();

    bool badTeamSetup = app->createNewMatch(
        MatchDurationPreset::MIN_20,
        5,
        "",
        "blue",
        "Away",
        "red"
    );

    app->createNewMatch(
        MatchDurationPreset::MIN_20,
        5,
        "Home",
        "blue",
        "Away",
        "red"
    );

    app->startMatch();

    bool badGoal = app->addGoal(
        "bad_team",
        10
    );

    bool badYellow = app->addYellowCard(
        TeamSide::HOME,
        7,
        "not_a_reason"
    );

    bool badJersey = app->addGoal(
        TeamSide::HOME,
        101
    );

    bool passed =
        badTeamSetup == false &&
        badGoal == false &&
        badYellow == false &&
        badJersey == false;

    printResult(testName, passed);
    return passed;
}

// =====================================================
// TEST: EVENT FORMATTING
// =====================================================

bool LogicTestSuite::testEventFormatting() {
    const String testName = "Event formatting";

    Event goal;
    goal.minute = 12;
    goal.type = EventType::GOAL;
    goal.team = TeamSide::HOME;
    goal.jerseyNumber = 9;

    Event sub;
    sub.minute = 55;
    sub.type = EventType::SUBSTITUTION;
    sub.team = TeamSide::AWAY;
    sub.playerOut = 10;
    sub.playerIn = 17;

    String goalText =
        EventFormatter::formatEvent(goal);

    String subText =
        EventFormatter::formatEvent(sub);

    bool passed =
        goalText == "12' GOAL #9 HOME" &&
        subText == "55' SUB #10→#17 AWAY";

    printResult(testName, passed);
    return passed;
}

// =====================================================
// HELPERS
// =====================================================

void LogicTestSuite::printResult(
    const String& testName,
    bool passed
) {
    Serial.print("[TEST] ");
    Serial.print(testName);
    Serial.print(": ");

    if (passed) {
        Serial.println("PASS");
    } else {
        Serial.println("FAIL");
    }
}

bool LogicTestSuite::assertTrue(
    const String& testName,
    bool condition
) {
    printResult(testName, condition);
    return condition;
}