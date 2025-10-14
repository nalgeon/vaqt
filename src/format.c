// Copyright 2025 Anton Zhiyanov, BSD 3-Clause License
// https://github.com/nalgeon/vaqt

// Copyright 2009 The Go Authors, BSD 3-Clause License
// https://github.com/golang/go

// Time formatting.

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "vaqt.h"

// parse_timezone_offset parses a timezone offset string in format ±HH:MM
// and returns the offset in seconds. Returns true on success, false on failure.
static bool parse_timezone_offset(const char* tz, int* offset_sec) {
    if (tz[0] == '\0') {
        *offset_sec = 0;
        return true;
    }
    // Validate the timezone string format before parsing
    if (tz[0] != '+' && tz[0] != '-') {
        return false;  // Invalid sign
    }
    if (!isdigit((unsigned char)tz[1]) || !isdigit((unsigned char)tz[2])) {
        return false;  // Invalid hours digits
    }
    if (tz[3] != ':') {
        return false;  // Missing colon separator
    }
    if (!isdigit((unsigned char)tz[4]) || !isdigit((unsigned char)tz[5])) {
        return false;  // Invalid minutes digits
    }

    // Parse timezone offset.
    // + 0 7 : 0 0
    // ⁰ ¹ ² ³ ⁴ ⁵
    // tz[0] is the sign.
    int sign = (tz[0] == '-') ? -1 : 1;
    // tz[1] and tz[2] are hours.
    *offset_sec = ((tz[1] - '0') * 10 + (tz[2] - '0')) * 3600 * sign;
    // tz[4] and tz[5] are minutes.
    *offset_sec += ((tz[4] - '0') * 10 + (tz[5] - '0')) * 60 * sign;
    return true;
}

// time_fmt_iso returns an ISO 8601 time string for the given time value.
// Converts the time value to the given timezone offset before formatting.
// Chooses the most compact representation:
//  - 2006-01-02T15:04:05.999999999+07:00
//  - 2006-01-02T15:04:05.999999999Z
//  - 2006-01-02T15:04:05+07:00
//  - 2006-01-02T15:04:05Z
size_t time_fmt_iso(Time t, int offset_sec, char* buf, size_t size) {
    int year, day, hour, min, sec;
    enum Month month;
    const char* layout;
    size_t n = 0;

    if (offset_sec == 0) {
        time_get_date(t, &year, &month, &day);
        time_get_clock(t, &hour, &min, &sec);
        if (t.nsec == 0) {
            layout = "%04d-%02d-%02dT%02d:%02d:%02dZ";
            n = snprintf(buf, size, layout, year, month, day, hour, min, sec);
        } else {
            layout = "%04d-%02d-%02dT%02d:%02d:%02d.%09dZ";
            n = snprintf(buf, size, layout, year, month, day, hour, min, sec, t.nsec);
        }
    } else {
        Time loc_t = time_add(t, offset_sec * TIME_SECOND);
        time_get_date(loc_t, &year, &month, &day);
        time_get_clock(loc_t, &hour, &min, &sec);
        int ofhour = offset_sec / 3600;
        int ofmin = (offset_sec % 3600) / 60;
        if (ofmin < 0) {
            ofmin = -ofmin;
        }
        if (loc_t.nsec == 0) {
            layout = "%04d-%02d-%02dT%02d:%02d:%02d%+03d:%02d";
            n = snprintf(buf, size, layout, year, month, day, hour, min, sec, ofhour, ofmin);
        } else {
            layout = "%04d-%02d-%02dT%02d:%02d:%02d.%09d%+03d:%02d";
            n = snprintf(buf, size, layout, year, month, day, hour, min, sec, loc_t.nsec, ofhour,
                         ofmin);
        }
    }
    return n;
}

// time_fmt_datetime returns a datetime string
// (2006-01-02 15:04:05) for the given time value.
// Converts the time value to the given timezone offset before formatting.
size_t time_fmt_datetime(Time t, int offset_sec, char* buf, size_t size) {
    int year, day, hour, min, sec;
    enum Month month;
    if (offset_sec == 0) {
        time_get_date(t, &year, &month, &day);
        time_get_clock(t, &hour, &min, &sec);
    } else {
        Time loc_t = time_add(t, offset_sec * TIME_SECOND);
        time_get_date(loc_t, &year, &month, &day);
        time_get_clock(loc_t, &hour, &min, &sec);
    }
    return snprintf(buf, size, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, min, sec);
}

// time_fmt_date returns a date string
// (2006-01-02) for the given time value.
// Converts the time value to the given timezone offset before formatting.
size_t time_fmt_date(Time t, int offset_sec, char* buf, size_t size) {
    int year, day;
    enum Month month;
    if (offset_sec == 0) {
        time_get_date(t, &year, &month, &day);
    } else {
        Time loc_t = time_add(t, offset_sec * TIME_SECOND);
        time_get_date(loc_t, &year, &month, &day);
    }
    return snprintf(buf, size, "%04d-%02d-%02d", year, month, day);
}

// time_fmt_time returns a time string
// (15:04:05) for the given time value.
// Converts the time value to the given timezone offset before formatting.
size_t time_fmt_time(Time t, int offset_sec, char* buf, size_t size) {
    int hour, min, sec;
    if (offset_sec == 0) {
        time_get_clock(t, &hour, &min, &sec);
    } else {
        Time loc_t = time_add(t, offset_sec * TIME_SECOND);
        time_get_clock(loc_t, &hour, &min, &sec);
    }
    return snprintf(buf, size, "%02d:%02d:%02d", hour, min, sec);
}

// time_parse parses a formatted string and returns the time value it represents.
// Supports a limited set of layouts:
// - "2006-01-02T15:04:05.999999999+07:00" (ISO 8601 with nanoseconds and timezone)
// - "2006-01-02T15:04:05.999999999Z" (ISO 8601 with nanoseconds, UTC)
// - "2006-01-02T15:04:05+07:00" (ISO 8601 with timezone)
// - "2006-01-02T15:04:05Z" (ISO 8601, UTC)
// - "2006-01-02 15:04:05" (date and time, UTC)
// - "2006-01-02" (date only, UTC)
// - "15:04:05" (time only, UTC)
Time time_parse(const char* value) {
    Time zero = {0, 0};
    size_t len = strlen(value);
    if (len < 8 || len > 35) {
        return zero;
    }

    int year = 1, month = 1, day = 1, hour = 0, min = 0, sec = 0, nsec = 0, offset_sec = 0;
    char tz[7] = "";

    if (len == 35) {
        // "2006-01-02T15:04:05.999999999+07:00"
        int n = sscanf(value, "%d-%d-%dT%d:%d:%d.%d%6s", &year, &month, &day, &hour, &min, &sec,
                       &nsec, tz);
        if (n != 8) {
            return zero;
        }
    }

    if (len == 30) {
        // "2006-01-02T15:04:05.999999999Z"
        int n =
            sscanf(value, "%d-%d-%dT%d:%d:%d.%dZ", &year, &month, &day, &hour, &min, &sec, &nsec);
        if (n != 7) {
            return zero;
        }
    }

    if (len == 25) {
        // "2006-01-02T15:04:05+07:00"
        int n = sscanf(value, "%d-%d-%dT%d:%d:%d%6s", &year, &month, &day, &hour, &min, &sec, tz);
        if (n != 7) {
            return zero;
        }
    }

    if (len == 19 || len == 20) {
        // "2006-01-02T15:04:05Z"
        // "2006-01-02 15:04:05"
        int n = sscanf(value, "%d-%d-%d%*c%d:%d:%d", &year, &month, &day, &hour, &min, &sec);
        if (n != 6) {
            return zero;
        }
    }

    if (len == 10) {
        // "2006-01-02"
        int n = sscanf(value, "%d-%d-%d", &year, &month, &day);
        if (n != 3) {
            return zero;
        }
    }

    if (len == 8) {
        // "15:04:05"
        int n = sscanf(value, "%d:%d:%d", &hour, &min, &sec);
        if (n != 3) {
            return zero;
        }
    }

    if (!parse_timezone_offset(tz, &offset_sec)) {
        // Invalid timezone offset.
        return zero;
    }

    return time_date(year, (enum Month)month, day, hour, min, sec, nsec, offset_sec);
}
