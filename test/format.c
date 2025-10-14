// Copyright 2025 Anton Zhiyanov, BSD 3-Clause License
// https://github.com/nalgeon/vaqt

// Copyright 2009 The Go Authors, BSD 3-Clause License
// https://github.com/golang/go

// Time formatting tests.

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "vaqt.h"

typedef struct {
    int year, month, day, hour, min, sec, nsec;
    const char* want;
    int offset_sec;
} FormatTest;

FormatTest fmt_iso_tests[] = {
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18T15:56:35Z", 0},
    {2011, 11, 18, 15, 56, 35, 666777888, "2011-11-18T15:56:35.666777888Z", 0},
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18T20:56:35+05:00", 5 * 3600},
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18T21:26:35+05:30", 5 * 3600 + 30 * 60},
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18T10:56:35-05:00", -5 * 3600},
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18T10:26:35-05:30", -5 * 3600 - 30 * 60},
    {2011, 11, 18, 15, 56, 35, 666777888, "2011-11-18T20:56:35.666777888+05:00", 5 * 3600},
    {2011, 11, 18, 15, 56, 35, 666777888, "2011-11-18T10:56:35.666777888-05:00", -5 * 3600},
};

static void test_fmt_iso(void) {
    printf("test_fmt_iso...");
    for (size_t i = 0; i < sizeof(fmt_iso_tests) / sizeof(fmt_iso_tests[0]); i++) {
        FormatTest test = fmt_iso_tests[i];
        Time t =
            time_date(test.year, test.month, test.day, test.hour, test.min, test.sec, test.nsec, 0);
        char got[64];
        time_fmt_iso(got, sizeof(got), t, test.offset_sec);
        // printf("want: %s, got: %s\n", test.want, got);
        assert(strcmp(got, test.want) == 0);
    }
    printf("OK\n");
}

FormatTest fmt_dt_tests[] = {
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18 15:56:35", 0},
    {2011, 11, 18, 15, 56, 35, 666777888, "2011-11-18 15:56:35", 0},
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18 20:56:35", 5 * 3600},
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18 21:26:35", 5 * 3600 + 30 * 60},
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18 10:56:35", -5 * 3600},
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18 10:26:35", -5 * 3600 - 30 * 60},
    {2011, 11, 18, 15, 56, 35, 666777888, "2011-11-18 20:56:35", 5 * 3600},
    {2011, 11, 18, 15, 56, 35, 666777888, "2011-11-18 10:56:35", -5 * 3600},
};

static void test_fmt_datetime(void) {
    printf("test_fmt_datetime...");
    for (size_t i = 0; i < sizeof(fmt_dt_tests) / sizeof(fmt_dt_tests[0]); i++) {
        FormatTest test = fmt_dt_tests[i];
        Time t =
            time_date(test.year, test.month, test.day, test.hour, test.min, test.sec, test.nsec, 0);
        char got[64];
        time_fmt_datetime(got, sizeof(got), t, test.offset_sec);
        // printf("want: %s, got: %s\n", test.want, got);
        assert(strcmp(got, test.want) == 0);
    }
    printf("OK\n");
}

FormatTest fmt_date_tests[] = {
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18", 0},
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18", 5 * 3600},
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-19", 12 * 3600},
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18", -5 * 3600},
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-17", -20 * 3600},
};

static void test_fmt_date(void) {
    printf("test_fmt_date...");
    for (size_t i = 0; i < sizeof(fmt_date_tests) / sizeof(fmt_date_tests[0]); i++) {
        FormatTest test = fmt_date_tests[i];
        Time t =
            time_date(test.year, test.month, test.day, test.hour, test.min, test.sec, test.nsec, 0);
        char got[64];
        time_fmt_date(got, sizeof(got), t, test.offset_sec);
        // printf("want: %s, got: %s\n", test.want, got);
        assert(strcmp(got, test.want) == 0);
    }
    printf("OK\n");
}

FormatTest fmt_time_tests[] = {
    {2011, 11, 18, 15, 56, 35, 0, "15:56:35", 0},
    {2011, 11, 18, 15, 56, 35, 0, "20:56:35", 5 * 3600},
    {2011, 11, 18, 15, 56, 35, 0, "03:56:35", 12 * 3600},
    {2011, 11, 18, 15, 56, 35, 0, "10:56:35", -5 * 3600},
    {2011, 11, 18, 15, 56, 35, 0, "19:56:35", -20 * 3600},
};

static void test_fmt_time(void) {
    printf("test_fmt_time...");
    for (size_t i = 0; i < sizeof(fmt_time_tests) / sizeof(fmt_time_tests[0]); i++) {
        FormatTest test = fmt_time_tests[i];
        Time t =
            time_date(test.year, test.month, test.day, test.hour, test.min, test.sec, test.nsec, 0);
        char got[64];
        time_fmt_time(got, sizeof(got), t, test.offset_sec);
        // printf("want: %s, got: %s\n", test.want, got);
        assert(strcmp(got, test.want) == 0);
    }
    printf("OK\n");
}

FormatTest parse_tests[] = {
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18T15:56:35Z", 0},
    {2011, 11, 18, 15, 56, 35, 666777888, "2011-11-18T15:56:35.666777888Z", 0},
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18T20:56:35+05:00", 5 * 3600},
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18T21:26:35+05:30", 5 * 3600 + 30 * 60},
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18T10:56:35-05:00", -5 * 3600},
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18T10:26:35-05:30", -5 * 3600 - 30 * 60},
    {2011, 11, 18, 15, 56, 35, 666777888, "2011-11-18T20:56:35.666777888+05:00", 5 * 3600},
    {2011, 11, 18, 15, 56, 35, 666777888, "2011-11-18T10:56:35.666777888-05:00", -5 * 3600},
    {2011, 11, 18, 15, 56, 35, 0, "2011-11-18 15:56:35", 0},
    {2011, 11, 18, 0, 0, 0, 0, "2011-11-18", 0},
    {1, 1, 1, 15, 56, 35, 0, "15:56:35", 0},
    {1, 1, 1, 0, 0, 0, 0, "2011-11-18 10:56", 0},
};

static void test_parse(void) {
    printf("test_parse...");
    for (size_t i = 0; i < sizeof(parse_tests) / sizeof(parse_tests[0]); i++) {
        FormatTest test = parse_tests[i];
        Time want =
            time_date(test.year, test.month, test.day, test.hour, test.min, test.sec, test.nsec, 0);
        Time got = time_parse(test.want);
        // printf("want: {%lld %d}, got: {%lld %d}\n", time_to_unix(want),
        // want.nsec, time_to_unix(got), got.nsec);
        assert(time_equal(got, want));
    }
    printf("OK\n");
}

static void test_parse_invalid(void) {
    printf("test_parse_invalid...");
    // Test invalid timezone strings that should return zero time
    const char* invalid_cases[] = {
        "2011-11-18T15:56:35+0500",   // missing colon
        "2011-11-18T15:56:35+0X:00",  // non-digit in hours
        "2011-11-18T15:56:35+00:0X",  // non-digit in minutes
        "2011-11-18T15:56:35*05:00",  // invalid sign
        "2011-11-18T15:56:35+05:0",   // too short minutes
        "2011-11-18T15:56:35+5:00",   // too short hours
    };

    for (size_t i = 0; i < sizeof(invalid_cases) / sizeof(invalid_cases[0]); i++) {
        Time got = time_parse(invalid_cases[i]);
        assert(time_is_zero(got));  // Should return zero time for invalid input
    }
    printf("OK\n");
}

int main(void) {
    test_fmt_iso();
    test_fmt_datetime();
    test_fmt_date();
    test_fmt_time();
    test_parse();
    test_parse_invalid();
}
