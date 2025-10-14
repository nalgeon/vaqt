// Copyright 2025 Anton Zhiyanov, BSD 3-Clause License
// https://github.com/nalgeon/vaqt

// Copyright 2009 The Go Authors, BSD 3-Clause License
// https://github.com/golang/go

// Time functions and methods.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "vaqt.h"

// Some platforms do not support timespec_get() from time.h.
#if defined(_WIN32)
#include <sys/timeb.h>
#elif !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L || \
    (!defined(TIME_UTC) && (!defined(_POSIX_TIMERS) || _POSIX_TIMERS <= 0))
#include <sys/time.h>
#endif

// ## Private

static const int64_t seconds_per_minute = 60;
static const int64_t seconds_per_hour = 60 * seconds_per_minute;
static const int64_t seconds_per_day = 24 * seconds_per_hour;
static const int64_t seconds_per_week = 7 * seconds_per_day;

// Gregorian calendar constants:
// - 400 years = 303 regular years + 97 leap years
// - 100 years = 76 regular years + 24 leap years
// - 4 years = 3 regular years + 1 leap year
static const int64_t days_per_400_years = 365 * 400 + 97;
static const int64_t days_per_100_years = 365 * 100 + 24;
static const int64_t days_per_4_years = 365 * 4 + 1;

// The absolute zero year for internal calculations.
// This is year 1 in the proleptic Gregorian calendar.
// Must be 1 mod 400 for proper leap year calculations.
// Times before this year will not compute correctly.
static const int64_t absolute_zero_year = -292277022399LL;

// Offset constants for time representation conversion:
// absolute_to_internal = (absolute_zero_year - internal_zero_year) * 365.2425 * seconds_per_day
// Where internal_zero_year is the year 1 in internal representation.
// These values are pre-calculated to avoid runtime computation.
static const int64_t absolute_to_internal = -9223371966579724800LL;
static const int64_t internal_to_absolute = -absolute_to_internal;

// Unix epoch (1970-01-01) offset from internal zero:
// Calculated as: (1969 * 365 + 1969/4 - 1969/100 + 1969/400) * seconds_per_day
// The 1969 comes from Unix epoch being 1970, but we calculate days up to end of 1969.
static const int64_t unix_to_internal =
    (1969 * 365 + 1969 / 4 - 1969 / 100 + 1969 / 400) * seconds_per_day;
static const int64_t internal_to_unix = -unix_to_internal;

// days_before[m] counts the number of days in a non-leap year
// before month m begins. There is an entry for m=12, counting
// the number of days before January of next year (365).
static const int days_before[] = {
    0,
    31,
    31 + 28,
    31 + 28 + 31,
    31 + 28 + 31 + 30,
    31 + 28 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31,
};

// norm returns nhi, nlo such that
//
//	hi * base + lo == nhi * base + nlo
//	0 <= nlo < base
static void norm(int hi, int lo, int base, int* nhi, int* nlo) {
    if (lo < 0) {
        int n = (-lo - 1) / base + 1;
        hi -= n;
        lo += n * base;
    }
    if (lo >= base) {
        int n = lo / base;
        hi += n;
        lo -= n * base;
    }
    *nhi = hi;
    *nlo = lo;
}

// days_since_epoch takes a year and returns the number of days from
// the absolute epoch (year 1) to the start of that year.
// This accounts for the Gregorian calendar's leap year rules:
// - Years divisible by 4 are leap years
// - Except years divisible by 100 are not leap years
// - Except years divisible by 400 are leap years
// The calculation breaks down the year into 400-year, 100-year, 4-year, and 1-year cycles
// for efficient computation.
static uint64_t days_since_epoch(int year) {
    uint64_t y = year - absolute_zero_year;

    // Add in days from complete 400-year cycles.
    // Each 400-year cycle has 97 leap years (400/4 - 400/100 + 400/400 = 100 - 4 + 1 = 97)
    uint64_t n = y / 400;
    y -= 400 * n;
    uint64_t d = days_per_400_years * n;

    // Add in days from complete 100-year cycles within remaining years.
    // Each 100-year cycle has 24 leap years (100/4 - 100/100 = 25 - 1 = 24)
    n = y / 100;
    y -= 100 * n;
    d += days_per_100_years * n;

    // Add in days from complete 4-year cycles within remaining years.
    // Each 4-year cycle has 1 leap year
    n = y / 4;
    y -= 4 * n;
    d += days_per_4_years * n;

    // Add in days from remaining non-leap years.
    n = y;
    d += 365 * n;

    return d;
}

// is_leap reports whether the year is a leap year.
static bool is_leap(int year) {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

static int64_t unix_sec(Time t) {
    return t.sec + internal_to_unix;
}

static Time unix_time(int64_t sec, int32_t nsec) {
    return (Time){sec + unix_to_internal, nsec};
}

// abs_time returns the time t as an absolute time, adjusted by the zone offset.
// It is called when computing a presentation property like Month or Hour.
static uint64_t abs_time(Time t) {
    return t.sec + internal_to_absolute;
}

// abs_weekday is like Weekday but operates on an absolute time.
static enum Weekday abs_weekday(uint64_t abs) {
    // January 1 of the absolute year, like January 1 of 2001, was a Monday.
    uint64_t sec = (abs + TIME_MONDAY * seconds_per_day) % seconds_per_week;
    return sec / seconds_per_day;
}

// abs_date converts an absolute time in seconds to the corresponding
// year and day-of-year (1-366).
static void abs_date(uint64_t abs, int* year, int* yday) {
    // Split into time and day.
    uint64_t d = abs / seconds_per_day;

    // Account for 400 year cycles.
    uint64_t n = d / days_per_400_years;
    uint64_t y = 400 * n;
    d -= days_per_400_years * n;

    // Cut off 100-year cycles.
    // The last cycle has one extra leap year, so on the last day
    // of that year, day / days_per_100_years will be 4 instead of 3.
    // Cut it back down to 3 by subtracting n>>2.
    n = d / days_per_100_years;
    n -= n >> 2;
    y += 100 * n;
    d -= days_per_100_years * n;

    // Cut off 4-year cycles.
    // The last cycle has a missing leap year, which does not
    // affect the computation.
    n = d / days_per_4_years;
    y += 4 * n;
    d -= days_per_4_years * n;

    // Cut off years within a 4-year cycle.
    // The last year is a leap year, so on the last day of that year,
    // day / 365 will be 4 instead of 3. Cut it back down to 3
    // by subtracting n>>2.
    n = d / 365;
    n -= n >> 2;
    y += n;
    d -= 365 * n;

    *year = y + absolute_zero_year;
    *yday = d;
}

// abs_date_full converts an absolute time in seconds to the corresponding
// year, month, day, and day-of-year (1-366).
static void abs_date_full(uint64_t abs, int* year, enum Month* month, int* day, int* yday) {
    abs_date(abs, year, yday);

    *day = *yday;
    if (is_leap(*year)) {
        // In leap years, February has 29 days instead of 28.
        // The day-of-year (*yday) includes the leap day, but we need to handle it specially
        // when converting to month/day format.
        if (*day > 31 + 29 - 1) {
            // After February 29 (day 60 in leap year), subtract 1 to account for the extra day
            // that was already included in the day-of-year count.
            *day -= 1;
        }
        if (*day == 31 + 29 - 1) {
            // This is February 29 (the leap day) - day 60 in a leap year.
            *month = TIME_FEBRUARY;
            *day = 29;
            return;
        }
    }

    // Estimate month by assuming every month has 31 days.
    // This gives an initial guess that may be off by at most one month.
    *month = *day / 31;
    int end = days_before[(int)(*month) + 1];
    int begin;
    if (*day >= end) {
        *month += 1;
        begin = end;
    } else {
        begin = days_before[(int)(*month)];
    }

    *month += 1;  // Convert from 0-based to 1-based month (January = 1)
    *day = *day - begin + 1;
}

// abs_clock converts an absolute time in seconds to the corresponding
// hour (0-23), minute (0-59), and second (0-59) within the day.
void abs_clock(uint64_t abs, int* hour, int* min, int* sec) {
    *sec = abs % seconds_per_day;
    *hour = *sec / seconds_per_hour;
    *sec -= *hour * seconds_per_hour;
    *min = *sec / seconds_per_minute;
    *sec -= *min * seconds_per_minute;
}

// tless_than_half reports whether x+x < y but avoids overflow,
// assuming x and y are both positive (Duration is signed).
static bool tless_than_half(Duration x, Duration y) {
    return (uint64_t)x + (uint64_t)x < (uint64_t)y;
}

// time_div divides t by d and returns the remainder.
// Only supports d which is a multiple of 1 second.
static Duration time_div(Time t, Duration d) {
    if (d % TIME_SECOND != 0) {
        return 0;
    }

    bool neg = false;
    int64_t sec = t.sec;
    int64_t nsec = t.nsec;
    if (sec < 0) {
        // Operate on absolute value.
        neg = true;
        sec = -sec;
        nsec = -nsec;
        if (nsec < 0) {
            nsec += 1e9;
            sec--;  // sec >= 1 before the -- so safe
        }
    }

    // d is a multiple of 1 second.
    int64_t d1 = d / TIME_SECOND;
    Duration r = (sec % d1) * TIME_SECOND + nsec;

    if (neg && r != 0) {
        r = d - r;
    }
    return r;
}

// timespec_now returns the current time with nanosecond precision.
static struct timespec timespec_now(void) {
    struct timespec ts;
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && defined(TIME_UTC) && \
    !defined(__ANDROID__)
    // C11.
    timespec_get(&ts, TIME_UTC);
#elif defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)
    // POSIX.
    clock_gettime(CLOCK_REALTIME, &ts);
#elif defined(_WIN32)
    // Windows.
    struct __timeb64 tb;
    _ftime64(&tb);
    ts.tv_sec = (time_t)tb.time;
    ts.tv_nsec = tb.millitm * 1000000;
#else
    // Fallback for older systems.
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ts.tv_sec = tv.tv_sec;
    ts.tv_nsec = tv.tv_usec * 1000;
#endif
    return ts;
}

// ## Constructors

// time_now returns the current time in UTC.
Time time_now(void) {
    struct timespec ts = timespec_now();
    return unix_time(ts.tv_sec, ts.tv_nsec);
}

// time_date returns the Time corresponding to
// yyyy-mm-dd hh:mm:ss + nsec nanoseconds
//
// The month, day, hour, min, sec, and nsec values may be outside
// their usual ranges and will be normalized during the conversion.
// For example, October 32 converts to November 1.
//
// The time is converted to UTC using offset_sec in seconds east of UTC.
Time time_date(int year,
               enum Month month,
               int day,
               int hour,
               int min,
               int sec,
               int nsec,
               int offset_sec) {
    // Normalize month, overflowing into year.
    int m = month - 1;
    norm(year, m, 12, &year, &m);
    month = m + 1;

    // Normalize nsec, sec, min, hour, overflowing into day.
    norm(sec, nsec, 1000000000, &sec, &nsec);
    norm(min, sec, 60, &min, &sec);
    norm(hour, min, 60, &hour, &min);
    norm(day, hour, 24, &day, &hour);

    // Compute days since the absolute epoch.
    uint64_t d = days_since_epoch(year);

    // Add in days before this month.
    d += days_before[month - 1];
    if (is_leap(year) && month >= TIME_MARCH) {
        d++;  // February 29
    }

    // Add in days before today.
    d += day - 1;

    // Add in time elapsed today.
    uint64_t abs = d * seconds_per_day;
    abs += hour * seconds_per_hour + min * seconds_per_minute + sec;

    // Convert to UTC.
    abs -= offset_sec;

    return (Time){abs + absolute_to_internal, nsec};
}

// ## Time parts

// time_get_date returns the year, month, and day in which t occurs.
void time_get_date(Time t, int* year, enum Month* month, int* day) {
    uint64_t abs = abs_time(t);
    int yday;
    abs_date_full(abs, year, month, day, &yday);
}

// time_get_year returns the year in which t occurs.
int time_get_year(Time t) {
    uint64_t abs = abs_time(t);
    int year, yday;
    abs_date(abs, &year, &yday);
    return year;
}

// time_get_month returns the month of the year specified by t.
enum Month time_get_month(Time t) {
    uint64_t abs = abs_time(t);
    int year, day, yday;
    enum Month month;
    abs_date_full(abs, &year, &month, &day, &yday);
    return month;
}

// time_get_day returns the day of the month specified by t.
int time_get_day(Time t) {
    uint64_t abs = abs_time(t);
    int year, day, yday;
    enum Month month;
    abs_date_full(abs, &year, &month, &day, &yday);
    return day;
}

// time_get_clock returns the hour, minute, and second within the day specified by t.
void time_get_clock(Time t, int* hour, int* min, int* sec) {
    uint64_t abs = abs_time(t);
    abs_clock(abs, hour, min, sec);
}

// time_get_hour returns the hour within the day specified by t, in the range
// [0, 23].
int time_get_hour(Time t) {
    uint64_t abs = abs_time(t);
    return (abs % seconds_per_day) / seconds_per_hour;
}

// time_get_minute returns the minute offset within the hour specified by t, in
// the range [0, 59].
int time_get_minute(Time t) {
    uint64_t abs = abs_time(t);
    return (abs % seconds_per_hour) / seconds_per_minute;
}

// time_get_second returns the second offset within the minute specified by t,
// in the range [0, 59].
int time_get_second(Time t) {
    uint64_t abs = abs_time(t);
    return abs % seconds_per_minute;
}

// time_get_nano returns the nanosecond offset within the second specified by t,
// in the range [0, 999999999].
int time_get_nano(Time t) {
    return t.nsec;
}

// time_get_weekday returns the day of the week specified by t.
enum Weekday time_get_weekday(Time t) {
    uint64_t abs = abs_time(t);
    return abs_weekday(abs);
}

// time_get_yearday returns the day of the year specified by t, in the range
// [1,365] for non-leap years, and [1,366] in leap years.
int time_get_yearday(Time t) {
    uint64_t abs = abs_time(t);
    int year, yday;
    abs_date(abs, &year, &yday);
    return yday + 1;
}

// time_get_isoweek returns the ISO 8601 year and week number in which t occurs.
// Week ranges from 1 to 53. Jan 01 to Jan 03 of year n might belong to
// week 52 or 53 of year n-1, and Dec 29 to Dec 31 might belong to week 1 of
// year n+1.
void time_get_isoweek(Time t, int* year, int* week) {
    // According to the rule that the first calendar week of a calendar year is
    // the week including the first Thursday of that year, and that the last one
    // is the week immediately preceding the first calendar week of the next
    // calendar year. See
    // https://www.iso.org/obp/ui#iso:std:iso:8601:-1:ed-1:v1:en:term:3.1.1.23 for
    // details.

    // weeks start with Monday
    // Monday Tuesday Wednesday Thursday Friday Saturday Sunday
    // 1      2       3         4        5      6        7
    // +3     +2      +1        0        -1     -2       -3
    // the offset to Thursday
    uint64_t abs = abs_time(t);
    int d = (TIME_THURSDAY - abs_weekday(abs));
    // handle Sunday
    if (d == 4) {
        d = -3;
    }
    // find the Thursday of the calendar week
    int yday;
    abs += d * seconds_per_day;
    abs_date(abs, year, &yday);
    *week = yday / 7 + 1;
}

// ## Unix time

// time_unix returns the Time corresponding to the given Unix time,
// sec seconds and nsec nanoseconds since January 1, 1970 UTC.
// It is valid to pass nsec outside the range [0, 999999999].
// Not all sec values have a corresponding time value. One such
// value is 1<<63-1 (the largest int64 value).
Time time_unix(int64_t sec, int64_t nsec) {
    if (nsec < 0 || nsec >= 1000000000) {
        int64_t n = nsec / 1000000000;
        sec += n;
        nsec -= n * 1000000000;
        if (nsec < 0) {
            nsec += 1000000000;
            sec--;
        }
    }
    return unix_time(sec, nsec);
}

// time_unix_milli returns the Time corresponding to the given Unix time,
// msec milliseconds since January 1, 1970 UTC.
Time time_unix_milli(int64_t msec) {
    return time_unix(msec / 1000, (msec % 1000) * 1000000);
}

// time_unix_micro returns the Time corresponding to the given Unix time,
// usec microseconds since January 1, 1970 UTC.
Time time_unix_micro(int64_t usec) {
    return time_unix(usec / 1000000, (usec % 1000000) * 1000);
}

// time_unix_nano returns the Time corresponding to the given Unix time,
// nsec nanoseconds since January 1, 1970 UTC.
Time time_unix_nano(int64_t nsec) {
    return time_unix(0, nsec);
}

// time_to_unix returns t as a Unix time, the number of seconds elapsed
// since January 1, 1970 UTC.
// Unix-like operating systems often record time as a 32-bit
// count of seconds, but since the method here returns a 64-bit
// value it is valid for billions of years into the past or future.
int64_t time_to_unix(Time t) {
    return unix_sec(t);
}

// time_to_unix_milli returns t as a Unix time, the number of milliseconds elapsed
// since January 1, 1970 UTC. The result is undefined if the Unix time in
// milliseconds cannot be represented by an int64 (a date more than 292 million
// years before or after 1970).
int64_t time_to_unix_milli(Time t) {
    return unix_sec(t) * 1000 + t.nsec / 1000000;
}

// time_to_unix_micro returns t as a Unix time, the number of microseconds elapsed
// since January 1, 1970 UTC. The result is undefined if the Unix time in
// microseconds cannot be represented by an int64 (a date before year -290307 or
// after year 294246).
int64_t time_to_unix_micro(Time t) {
    return unix_sec(t) * 1000000 + t.nsec / 1000;
}

// time_to_unix_nano returns t as a Unix time, the number of nanoseconds elapsed
// since January 1, 1970 UTC. The result is undefined if the Unix time
// in nanoseconds cannot be represented by an int64 (a date before the year
// 1678 or after 2262). Note that this means the result of calling UnixNano
// on the zero Time is undefined.
int64_t time_to_unix_nano(Time t) {
    return unix_sec(t) * 1000000000 + t.nsec;
}

// ## Calendar time

// time_tm returns the Time corresponding to the given calendar time
// at the given timezone offset.
Time time_tm(struct tm tm, int offset_sec) {
    int year = tm.tm_year + 1900;
    int month = tm.tm_mon + 1;
    int day = tm.tm_mday;
    int hour = tm.tm_hour;
    int min = tm.tm_min;
    int sec = tm.tm_sec;
    return time_date(year, month, day, hour, min, sec, 0, offset_sec);
}

// time_to_tm returns t in the given timezone offset as a calendar time.
struct tm time_to_tm(Time t, int offset_sec) {
    Time loc_t = time_add(t, offset_sec * TIME_SECOND);
    int year, day, hour, min, sec;
    enum Month month;
    time_get_date(loc_t, &year, &month, &day);
    time_get_clock(loc_t, &hour, &min, &sec);
    struct tm tm = {
        .tm_year = year - 1900,
        .tm_mon = month - 1,
        .tm_mday = day,
        .tm_hour = hour,
        .tm_min = min,
        .tm_sec = sec,
        .tm_isdst = -1,
    };
    return tm;
}

// ## Comparison

// time_after reports whether the time instant t is after u.
bool time_after(Time t, Time u) {
    return t.sec > u.sec || (t.sec == u.sec && t.nsec > u.nsec);
}

// time_before reports whether the time instant t is before u.
bool time_before(Time t, Time u) {
    return t.sec < u.sec || (t.sec == u.sec && t.nsec < u.nsec);
}

// time_compare compares the time instant t with u. If t is before u, it returns
// -1; if t is after u, it returns +1; if they're the same, it returns 0.
int time_compare(Time t, Time u) {
    if (time_before(t, u)) {
        return -1;
    }
    if (time_after(t, u)) {
        return +1;
    }
    return 0;
}

// time_equal reports whether t and u represent the same time instant.
bool time_equal(Time t, Time u) {
    return t.sec == u.sec && t.nsec == u.nsec;
}

// time_is_zero reports whether t represents the zero time instant,
// January 1, year 1, 00:00:00 UTC.
bool time_is_zero(Time t) {
    return t.sec == 0 && t.nsec == 0;
}

// ## Arithmetic

// time_add returns the time t+d.
Time time_add(Time t, Duration d) {
    int64_t dsec = d / TIME_SECOND;
    int64_t nsec = t.nsec + d % 1000000000;
    if (nsec >= 1e9) {
        dsec++;
        nsec -= 1e9;
    } else if (nsec < 0) {
        dsec--;
        nsec += 1e9;
    }
    return (Time){t.sec + dsec, nsec};
}

// time_sub returns the duration t-u. If the result exceeds the maximum (or
// minimum) value that can be stored in a Duration, the maximum (or minimum)
// duration will be returned.
Duration time_sub(Time t, Time u) {
    int64_t d = (t.sec - u.sec) * TIME_SECOND + (t.nsec - u.nsec);
    if (time_equal(time_add(u, d), t)) {
        return d;  // d is correct
    }
    if (time_before(t, u)) {
        return DURATION_MIN;  // t - u is negative out of range
    }
    return DURATION_MAX;  // t - u is positive out of range
}

// time_since returns the time elapsed since t.
// It is shorthand for time_sub(time_now(), t).
Duration time_since(Time t) {
    return time_sub(time_now(), t);
}

// time_until returns the duration until t.
// It is shorthand for time_sub(t, time_now()).
Duration time_until(Time t) {
    return time_sub(t, time_now());
}

// time_add_date returns the time corresponding to adding the
// given number of years, months, and days to t.
// For example, time_add_date(-1, 2, 3) applied to January 1, 2011
// returns March 4, 2010.
//
// time_add_date normalizes its result in the same way that Date does,
// so, for example, adding one month to October 31 yields
// December 1, the normalized form for November 31.
Time time_add_date(Time t, int years, int months, int days) {
    int year, day;
    enum Month month;
    time_get_date(t, &year, &month, &day);
    int hour, min, sec;
    time_get_clock(t, &hour, &min, &sec);
    return time_date(year + years, month + months, day + days, hour, min, sec, t.nsec, 0);
}

// ## Rounding

// time_truncate returns the result of rounding t down to a multiple of d (since
// the zero time). Only supports d which is a multiple of 1 second. If d <= 0,
// returns t unchanged.
Time time_truncate(Time t, Duration d) {
    if (d <= 0) {
        return t;
    }
    Duration r = time_div(t, d);
    return time_add(t, -r);
}

// time_round returns the result of rounding t to the nearest multiple of d
// (since the zero time). The rounding behavior for halfway values is to round
// up. If d <= 0, returns t unchanged.
Time time_round(Time t, Duration d) {
    if (d <= 0) {
        return t;
    }
    Duration r = time_div(t, d);
    if (tless_than_half(r, d)) {
        return time_add(t, -r);
    }
    return time_add(t, d - r);
}

// ## Marshaling

// time_unmarshal_binary returns the time instant represented by the binary data.
// The blob must have been created by time_marshal_binary and be at least 13 bytes long.
Time time_unmarshal_binary(const uint8_t* buf) {
    const uint8_t version = buf[0];
    if (version != 1) {
        return (Time){0, 0};
    }

    int64_t sec = (int64_t)buf[8] | (int64_t)buf[7] << 8 | (int64_t)buf[6] << 16 |
                  (int64_t)buf[5] << 24 | (int64_t)buf[4] << 32 | (int64_t)buf[3] << 40 |
                  (int64_t)buf[2] << 48 | (int64_t)buf[1] << 56;

    int32_t nsec =
        (int32_t)buf[12] | (int32_t)buf[11] << 8 | (int32_t)buf[10] << 16 | (int32_t)buf[9] << 24;

    return (Time){sec, nsec};
}

// time_marshal_binary returns the binary representation of the time instant t.
// buf must be at least 13 bytes long.
// The result is a byte slice with the following layout:
// 0: version (currently 1)
// 1-8: seconds
// 9-12: nanoseconds
void time_marshal_binary(Time t, uint8_t* buf) {
    const uint8_t version = 1;
    buf[0] = version;
    buf[1] = t.sec >> 56;  // bytes 1-8: seconds
    buf[2] = t.sec >> 48;
    buf[3] = t.sec >> 40;
    buf[4] = t.sec >> 32;
    buf[5] = t.sec >> 24;
    buf[6] = t.sec >> 16;
    buf[7] = t.sec >> 8;
    buf[8] = t.sec;
    buf[9] = t.nsec >> 24;  // bytes 9-12: nanoseconds
    buf[10] = t.nsec >> 16;
    buf[11] = t.nsec >> 8;
    buf[12] = t.nsec;
}
