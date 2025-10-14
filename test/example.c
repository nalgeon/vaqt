// Copyright 2025 Anton Zhiyanov, BSD 3-Clause License
// https://github.com/nalgeon/vaqt

// Usage examples for the vaqt package.

#include <stdio.h>
#include "vaqt.h"

static void example_time_now(void) {
    printf("---\ntime_now:\n");

    Time t = time_now();
    char buf[64];
    time_fmt_iso(buf, sizeof(buf), t, 0);
    printf("%s\n", buf);
    // 2025-10-13T19:46:07.726485000Z
}

static void example_time_date(void) {
    printf("---\ntime_date:\n");

    Time t1 = time_date(2011, November, 18, 0, 0, 0, 0, 0);
    char buf1[64];
    time_fmt_iso(buf1, sizeof(buf1), t1, 0);
    printf("%s\n", buf1);
    // 2011-11-18T00:00:00Z

    Time t2 = time_date(2011, November, 18, 15, 56, 35, 0, 0);
    char buf2[64];
    time_fmt_iso(buf2, sizeof(buf2), t2, 0);
    printf("%s\n", buf2);
    // 2011-11-18T15:56:35Z

    Time t3 = time_date(2011, November, 18, 15, 56, 35, 666777888, 0);
    char buf3[64];
    time_fmt_iso(buf3, sizeof(buf3), t3, 0);
    printf("%s\n", buf3);
    // 2011-11-18T15:56:35.666777888Z

    Time t4 = time_date(2011, November, 18, 15, 56, 35, 0, -5 * 3600);
    char buf4[64];
    time_fmt_iso(buf4, sizeof(buf4), t4, 0);
    printf("%s\n", buf4);
    // 2011-11-18T20:56:35Z

    Time t5 = time_date(2011, November, 18, 15, 56, 35, 666777888, -5 * 3600);
    char buf5[64];
    time_fmt_iso(buf5, sizeof(buf5), t5, 0);
    printf("%s\n", buf5);
    // 2011-11-18T20:56:35.666777888Z
}

static void example_time_get_year(void) {
    printf("---\ntime_get_year:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 431295000, 0);
    int year = time_get_year(t);
    printf("%d\n", year);
    // 2024
}

static void example_time_get_month(void) {
    printf("---\ntime_get_month:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 431295000, 0);
    enum Month month = time_get_month(t);
    printf("%d\n", month);
    // 8
}

static void example_time_get_day(void) {
    printf("---\ntime_get_day:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 431295000, 0);
    int day = time_get_day(t);
    printf("%d\n", day);
    // 6
}

static void example_time_get_hour(void) {
    printf("---\ntime_get_hour:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 431295000, 0);
    int hour = time_get_hour(t);
    printf("%d\n", hour);
    // 21
}

static void example_time_get_minute(void) {
    printf("---\ntime_get_minute:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 431295000, 0);
    int minute = time_get_minute(t);
    printf("%d\n", minute);
    // 22
}

static void example_time_get_second(void) {
    printf("---\ntime_get_second:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 431295000, 0);
    int second = time_get_second(t);
    printf("%d\n", second);
    // 15
}

static void example_time_get_nano(void) {
    printf("---\ntime_get_nano:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 431295000, 0);
    int nano = time_get_nano(t);
    printf("%d\n", nano);
    // 431295000
}

static void example_time_get_weekday(void) {
    printf("---\ntime_get_weekday:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 431295000, 0);
    enum Weekday weekday = time_get_weekday(t);
    printf("%d\n", weekday);
    // 2
}

static void example_time_get_yearday(void) {
    printf("---\ntime_get_yearday:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 431295000, 0);
    int yearday = time_get_yearday(t);
    printf("%d\n", yearday);
    // 219
}

static void example_time_get_isoweek(void) {
    printf("---\ntime_get_isoweek:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 431295000, 0);
    int iso_year, iso_week;
    time_get_isoweek(t, &iso_year, &iso_week);
    printf("%d, %d\n", iso_year, iso_week);
    // 2024, 32
}

static void example_time_get_date(void) {
    printf("---\ntime_get_date:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 431295000, 0);
    int year, day;
    enum Month month;
    time_get_date(t, &year, &month, &day);
    printf("%d, %d, %d\n", year, month, day);
    // 2024, 8, 6
}

static void example_time_get_clock(void) {
    printf("---\ntime_get_clock:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 431295000, 0);
    int hour, min, sec;
    time_get_clock(t, &hour, &min, &sec);
    printf("%d, %d, %d\n", hour, min, sec);
    // 21, 22, 15
}

static void example_time_unix(void) {
    printf("---\ntime_unix:\n");

    Time t = time_unix(1321631795, 666777888);
    char buf[64];
    time_fmt_iso(buf, sizeof(buf), t, 0);
    printf("%s\n", buf);
    // "2011-11-18T15:56:35.666777888Z"
}

static void example_time_milli(void) {
    printf("---\ntime_milli:\n");

    Time t = time_milli(1321631795666);
    char buf[64];
    time_fmt_iso(buf, sizeof(buf), t, 0);
    printf("%s\n", buf);
    // "2011-11-18T15:56:35.666000000Z"
}

static void example_time_micro(void) {
    printf("---\ntime_micro:\n");

    Time t = time_micro(1321631795666777);
    char buf[64];
    time_fmt_iso(buf, sizeof(buf), t, 0);
    printf("%s\n", buf);
    // "2011-11-18T15:56:35.666777000Z"
}

static void example_time_nano(void) {
    printf("---\ntime_nano:\n");

    Time t = time_nano(1321631795666777888);
    char buf[64];
    time_fmt_iso(buf, sizeof(buf), t, 0);
    printf("%s\n", buf);
    // "2011-11-18T15:56:35.666777888Z"
}

static void example_time_to_unix(void) {
    printf("---\ntime_to_unix:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 431295000, 0);
    int64_t unix_sec = time_to_unix(t);
    printf("%lld\n", unix_sec);
    // 1722979335
}

static void example_time_to_milli(void) {
    printf("---\ntime_to_milli:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 431295000, 0);
    int64_t unix_msec = time_to_milli(t);
    printf("%lld\n", unix_msec);
    // 1722979335431
}

static void example_time_to_micro(void) {
    printf("---\ntime_to_micro:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 431295000, 0);
    int64_t unix_usec = time_to_micro(t);
    printf("%lld\n", unix_usec);
    // 1722979335431295
}

static void example_time_to_nano(void) {
    printf("---\ntime_to_nano:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 431295000, 0);
    int64_t unix_nsec = time_to_nano(t);
    printf("%lld\n", unix_nsec);
    // 1722979335431295000
}

static void example_time_tm(void) {
    printf("---\ntime_tm:\n");

    struct tm tm = {
        .tm_year = 111,  // 2011 - 1900
        .tm_mon = 10,    // November (0-based)
        .tm_mday = 18,
        .tm_hour = 15,
        .tm_min = 56,
        .tm_sec = 35,
    };
    Time t = time_tm(tm, 0);
    char buf[64];
    time_fmt_iso(buf, sizeof(buf), t, 0);
    printf("%s\n", buf);
    // "2011-11-18T15:56:35Z"
}

static void example_time_to_tm(void) {
    printf("---\ntime_to_tm:\n");

    Time t = time_date(2011, November, 18, 15, 56, 35, 0, 0);
    struct tm tm = time_to_tm(t, 0);
    printf("tm_year = %d, tm_mon = %d, tm_mday = %d, tm_hour = %d, tm_min = %d, tm_sec = %d\n",
           tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    // tm.tm_year = 111, tm.tm_mon = 10, tm.tm_mday = 18,
    // tm.tm_hour = 15, tm.tm_min = 56, tm.tm_sec = 35
}

static void example_time_after(void) {
    printf("---\ntime_after:\n");

    Time t1 = time_date(2024, August, 6, 21, 22, 15, 0, 0);
    Time t2 = time_date(2024, August, 6, 21, 22, 16, 0, 0);
    bool after = time_after(t2, t1);
    printf("%s\n", after ? "true" : "false");
    // true
}

static void example_time_before(void) {
    printf("---\ntime_before:\n");

    Time t1 = time_date(2024, August, 6, 21, 22, 15, 0, 0);
    Time t2 = time_date(2024, August, 6, 21, 22, 16, 0, 0);
    bool before = time_before(t1, t2);
    printf("%s\n", before ? "true" : "false");
    // true
}

static void example_time_compare(void) {
    printf("---\ntime_compare:\n");

    Time t1 = time_date(2024, August, 6, 21, 22, 15, 0, 0);
    Time t2 = time_date(2024, August, 6, 21, 22, 16, 0, 0);
    int cmp = time_compare(t1, t2);
    printf("%d\n", cmp);
    // -1
}

static void example_time_equal(void) {
    printf("---\ntime_equal:\n");

    Time t1 = time_date(2024, August, 6, 21, 22, 15, 0, 0);
    Time t2 = time_date(2024, August, 6, 21, 22, 15, 0, 0);
    bool equal = time_equal(t1, t2);
    printf("%s\n", equal ? "true" : "false");
    // true
}

static void example_time_is_zero(void) {
    printf("---\ntime_is_zero:\n");

    Time t = {0, 0};
    bool is_zero = time_is_zero(t);
    printf("%s\n", is_zero ? "true" : "false");
    // true
}

static void example_time_add(void) {
    printf("---\ntime_add:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 0, 0);
    Duration d = Second * 30;
    Time result = time_add(t, d);
    char buf[64];
    time_fmt_iso(buf, sizeof(buf), result, 0);
    printf("%s\n", buf);
    // "2024-08-06T21:22:45Z"
}

static void example_time_sub(void) {
    printf("---\ntime_sub:\n");

    Time t1 = time_date(2024, August, 6, 21, 22, 45, 0, 0);
    Time t2 = time_date(2024, August, 6, 21, 22, 15, 0, 0);
    Duration d = time_sub(t1, t2);
    printf("%lld\n", d);
    // 30000000000
}

static void example_time_since(void) {
    printf("---\ntime_since:\n");

    Time past = time_date(2024, August, 6, 21, 22, 15, 0, 0);
    Duration elapsed = time_since(past);
    printf("%.3f seconds\n", duration_to_seconds(elapsed));
    // elapsed is the duration since past
}

static void example_time_until(void) {
    printf("---\ntime_until:\n");

    Time future = time_date(2034, August, 6, 21, 22, 45, 0, 0);
    Duration remaining = time_until(future);
    printf("%.3f seconds\n", duration_to_seconds(remaining));
    // remaining is the duration until future
}

static void example_time_add_date(void) {
    printf("---\ntime_add_date:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 0, 0);
    Time result = time_add_date(t, 0, 0, 1);
    char buf[64];
    time_fmt_iso(buf, sizeof(buf), result, 0);
    printf("%s\n", buf);
    // "2024-08-07T21:22:15Z"
}

static void example_time_truncate(void) {
    printf("---\ntime_truncate:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 500000000, 0);
    Duration d = Second * 10;
    Time result = time_truncate(t, d);
    char buf[64];
    time_fmt_iso(buf, sizeof(buf), result, 0);
    printf("%s\n", buf);
    // "2024-08-06T21:22:10Z"
}

static void example_time_round(void) {
    printf("---\ntime_round:\n");

    Time t = time_date(2024, August, 6, 21, 22, 15, 500000000, 0);
    Duration d = Second * 10;
    Time result = time_round(t, d);
    char buf[64];
    time_fmt_iso(buf, sizeof(buf), result, 0);
    printf("%s\n", buf);
    // "2024-08-06T21:22:20Z"
}

static void example_time_fmt_iso(void) {
    printf("---\ntime_fmt_iso:\n");

    Time t = time_date(2011, November, 18, 15, 56, 35, 666777888, 0);
    char buf[64];
    size_t n = time_fmt_iso(buf, sizeof(buf), t, 0);
    printf("%s\n", buf);
    // "2011-11-18T15:56:35.666777888Z"
}

static void example_time_fmt_datetime(void) {
    printf("---\ntime_fmt_datetime:\n");

    Time t = time_date(2011, November, 18, 15, 56, 35, 0, 0);
    char buf[64];
    size_t n = time_fmt_datetime(buf, sizeof(buf), t, 0);
    printf("%s\n", buf);
    // "2011-11-18 15:56:35"
}

static void example_time_fmt_date(void) {
    printf("---\ntime_fmt_date:\n");

    Time t = time_date(2011, November, 18, 15, 56, 35, 0, 0);
    char buf[64];
    size_t n = time_fmt_date(buf, sizeof(buf), t, 0);
    printf("%s\n", buf);
    // "2011-11-18"
}

static void example_time_fmt_time(void) {
    printf("---\ntime_fmt_time:\n");

    Time t = time_date(2011, November, 18, 15, 56, 35, 0, 0);
    char buf[64];
    size_t n = time_fmt_time(buf, sizeof(buf), t, 0);
    printf("%s\n", buf);
    // "15:56:35"
}

static void example_time_parse(void) {
    printf("---\ntime_parse:\n");

    Time t = time_parse("2011-11-18T15:56:35.666777888Z");
    char buf[64];
    time_fmt_iso(buf, sizeof(buf), t, 0);
    printf("%s\n", buf);
    // "2011-11-18T15:56:35.666777888Z"
}

static void example_time_marshal_binary(void) {
    printf("---\ntime_marshal_binary:\n");

    Time t = time_now();
    uint8_t buf[TIME_BINARY_SIZE];
    time_marshal_binary(t, buf);
    printf("buf contains serialized t value\n");
    // buf contains serialized t value
}

static void example_time_unmarshal_binary(void) {
    printf("---\ntime_unmarshal_binary:\n");

    Time t1 = time_date(2011, November, 18, 15, 56, 35, 666777888, 0);
    uint8_t buf[TIME_BINARY_SIZE];
    time_marshal_binary(t1, buf);
    Time t2 = time_unmarshal_binary(buf);
    bool equal = time_equal(t1, t2);
    printf("%s\n", equal ? "true" : "false");
    // true
}

static void example_duration_to_micro(void) {
    printf("---\nduration_to_micro:\n");

    Duration d = 5 * Second + 10 * Millisecond;
    int64_t micro = duration_to_micro(d);
    printf("%lld\n", micro);
    // 5010000
}

static void example_duration_to_milli(void) {
    printf("---\nduration_to_milli:\n");

    Duration d = 5 * Second + 10 * Millisecond;
    int64_t milli = duration_to_milli(d);
    printf("%lld\n", milli);
    // 5010
}

static void example_duration_to_seconds(void) {
    printf("---\nduration_to_seconds:\n");

    Duration d = 5 * Second + 500 * Millisecond;
    double sec = duration_to_seconds(d);
    printf("%.1f\n", sec);
    // 5.5
}

static void example_duration_to_minutes(void) {
    printf("---\nduration_to_minutes:\n");

    Duration d = 2 * Minute + 30 * Second;
    double min = duration_to_minutes(d);
    printf("%.1f\n", min);
    // 2.5
}

static void example_duration_to_hours(void) {
    printf("---\nduration_to_hours:\n");

    Duration d = 1 * Hour + 30 * Minute;
    double hours = duration_to_hours(d);
    printf("%.1f\n", hours);
    // 1.5
}

static void example_duration_truncate(void) {
    printf("---\nduration_truncate:\n");

    Duration d = 25 * Second + 500 * Millisecond;
    Duration m = 10 * Second;
    Duration result = duration_truncate(d, m);
    printf("%lld\n", result);
    // 20000000000
}

static void example_duration_round(void) {
    printf("---\nduration_round:\n");

    Duration d = 25 * Second + 500 * Millisecond;
    Duration m = 10 * Second;
    Duration result = duration_round(d, m);
    printf("%lld\n", result);
    // 30000000000
}

static void example_duration_abs(void) {
    printf("---\nduration_abs:\n");

    Duration d = -5 * Second;
    Duration result = duration_abs(d);
    printf("%lld\n", result);
    // 5000000000
}

int main(void) {
    example_time_now();
    example_time_date();
    example_time_get_year();
    example_time_get_month();
    example_time_get_day();
    example_time_get_hour();
    example_time_get_minute();
    example_time_get_second();
    example_time_get_nano();
    example_time_get_weekday();
    example_time_get_yearday();
    example_time_get_isoweek();
    example_time_get_date();
    example_time_get_clock();
    example_time_unix();
    example_time_milli();
    example_time_micro();
    example_time_nano();
    example_time_to_unix();
    example_time_to_milli();
    example_time_to_micro();
    example_time_to_nano();
    example_time_tm();
    example_time_to_tm();
    example_time_after();
    example_time_before();
    example_time_compare();
    example_time_equal();
    example_time_is_zero();
    example_time_add();
    example_time_sub();
    example_time_since();
    example_time_until();
    example_time_add_date();
    example_time_truncate();
    example_time_round();
    example_time_fmt_iso();
    example_time_fmt_datetime();
    example_time_fmt_date();
    example_time_fmt_time();
    example_time_parse();
    example_time_marshal_binary();
    example_time_unmarshal_binary();
    example_duration_to_micro();
    example_duration_to_milli();
    example_duration_to_seconds();
    example_duration_to_minutes();
    example_duration_to_hours();
    example_duration_truncate();
    example_duration_round();
    example_duration_abs();
}
