#include "EventFormatter.h"

// =====================================================
// FORMAT FULL EVENT
// =====================================================

String EventFormatter::formatEvent(
    const Event& event,
    DatabaseManager* db
) {
    String text = "";

    text += formatMinute(event.minute);
    text += " ";

    if (event.type == EventType::GOAL) {
        text += "GOAL ";
        text += "#";
        text += String(event.jerseyNumber);
        text += " ";
        text += formatTeamLabel(event.team);
        return text;
    }

    if (event.type == EventType::YELLOW) {
        text += "YC ";
        text += "#";
        text += String(event.jerseyNumber);
        text += " ";
        text += formatTeamLabel(event.team);

        String reason = formatReasonLabel(event, db);
        if (reason.length() > 0) {
            text += " - ";
            text += reason;
        }

        return text;
    }

    if (event.type == EventType::RED) {
        text += "RC ";
        text += "#";
        text += String(event.jerseyNumber);
        text += " ";
        text += formatTeamLabel(event.team);

        String reason = formatReasonLabel(event, db);
        if (reason.length() > 0) {
            text += " - ";
            text += reason;
        }

        return text;
    }

    if (event.type == EventType::SUBSTITUTION) {
        text += "SUB ";
        text += "#";
        text += String(event.playerOut);
        text += "→#";
        text += String(event.playerIn);
        text += " ";
        text += formatTeamLabel(event.team);
        return text;
    }

    text += "EVENT ";
    text += formatTeamLabel(event.team);

    return text;
}

// =====================================================
// FORMAT SHORT EVENT
// =====================================================
//
// Used when space is tight on the watch.
//

String EventFormatter::formatEventShort(
    const Event& event
) {
    String text = "";

    text += formatMinute(event.minute);
    text += " ";

    if (event.type == EventType::GOAL) {
        text += "G #";
        text += String(event.jerseyNumber);
        return text;
    }

    if (event.type == EventType::YELLOW) {
        text += "YC #";
        text += String(event.jerseyNumber);
        return text;
    }

    if (event.type == EventType::RED) {
        text += "RC #";
        text += String(event.jerseyNumber);
        return text;
    }

    if (event.type == EventType::SUBSTITUTION) {
        text += "SUB ";
        text += String(event.playerOut);
        text += ">";
        text += String(event.playerIn);
        return text;
    }

    text += "EVT";
    return text;
}

// =====================================================
// FORMAT TEAM LABEL
// =====================================================

String EventFormatter::formatTeamLabel(
    const String& team
) {
    if (team == TeamSide::HOME) {
        return "HOME";
    }

    if (team == TeamSide::AWAY) {
        return "AWAY";
    }

    return "TEAM";
}

// =====================================================
// FORMAT MINUTE
// =====================================================

String EventFormatter::formatMinute(
    int minute
) {
    if (minute < 0) {
        minute = 0;
    }

    return String(minute) + "'";
}

// =====================================================
// FORMAT SCORE
// =====================================================

String EventFormatter::formatScore(
    const Team& home,
    const Team& away
) {
    return String(home.score) + " - " + String(away.score);
}

// =====================================================
// FORMAT MATCH SUMMARY
// =====================================================

String EventFormatter::formatMatchSummary(
    const Team& home,
    const Team& away
) {
    String summary = "";

    summary += home.name.length() > 0 ? home.name : "HOME";
    summary += " ";
    summary += String(home.score);
    summary += " - ";
    summary += String(away.score);
    summary += " ";
    summary += away.name.length() > 0 ? away.name : "AWAY";

    return summary;
}

// =====================================================
// FORMAT HALF LABEL
// =====================================================

String EventFormatter::formatHalfLabel(
    int half
) {
    if (half == 1) {
        return "1ST HALF";
    }

    if (half == 2) {
        return "2ND HALF";
    }

    return "MATCH";
}

// =====================================================
// FORMAT REASON LABEL
// =====================================================

String EventFormatter::formatReasonLabel(
    const Event& event,
    DatabaseManager* db
) {
    if (event.reason.length() == 0) {
        return "";
    }

    if (!db) {
        return event.reason;
    }

    if (event.type == EventType::YELLOW) {
        return String(db->yellowReasonLabel(event.reason));
    }

    if (event.type == EventType::RED) {
        return String(db->redReasonLabel(event.reason));
    }

    return event.reason;
}