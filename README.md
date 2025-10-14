# vaqt: High-precision date/time in C

The `vaqt` package offers data types and functions for handling time and duration in C, with nanosecond precision.

[Concepts](#concepts) •
[Creating values](#creating-time-values) •
[Extracting fields](#extracting-time-fields) •
[Unix time](#unix-time) •
[Calendar time](#calendar-time) •
[Time comparison](#time-comparison) •
[Time arithmetic](#time-arithmetic) •
[Rounding](#rounding) •
[Formatting](#formatting) •
[Marshaling](#marshaling) •
[Duration](#duration) •
[Duration rounding](#duration-rounding) •
[Getting started](#getting-started) •
[Building from source](#building-from-source) •
[Credits](#credits)

## Concepts

This package works with two types of values: `Time` and `Duration`.

```text
  Time

  since     within
  0-time    second
┌─────────┬─────────────┐
│ seconds │ nanoseconds │
└─────────┴─────────────┘
  64 bit    32 bit
```

Time is a pair (seconds, nanoseconds), where `seconds` is the 64-bit number of seconds since zero time (0001-01-01 00:00:00 UTC) and `nanoseconds` is the number of nanoseconds within the current second (0-999999999). Time can represent dates billions of years in the past or future with nanosecond precision.

Time is always operated in UTC, but you can convert it from/to a specific timezone.

```text
  Duration
┌─────────────┐
│ nanoseconds │
└─────────────┘
  64 bit
```

Duration is a 64-bit number of nanoseconds. It can represent values up to about 290 years.

The calendrical calculations always assume a Gregorian calendar, with no leap seconds.

## Creating time values

There are two basic constructors — one for the current time and one for a specific date/time.

### time_now

```c
Time time_now(void);
```

Returns the current time in UTC.

```c
Time t = time_now();
char buf[64];
time_fmt_iso(buf, sizeof(buf), t, 0);
// 2025-10-13T19:46:07.726485000Z
```

### time_date

```c
Time time_date(int year, enum Month month, int day, int hour, int min, int sec, int nsec, int offset_sec);
```

Returns the Time corresponding to a given date/time.

The `month`, `day`, `hour`, `min`, `sec`, and `nsec` values may be outside their usual ranges and will be normalized during the conversion. For example, October 32 converts to November 1.

If `offset_sec` is not 0, the source time is treated as being in a given timezone (with an offset in seconds east of UTC) and converted back to UTC.

```c
Time t1 = time_date(2011, TIME_NOVEMBER, 18, 0, 0, 0, 0, 0);
char buf1[64];
time_fmt_iso(buf1, sizeof(buf1), t1, 0);
// 2011-11-18T00:00:00Z

Time t2 = time_date(2011, TIME_NOVEMBER, 18, 15, 56, 35, 0, 0);
char buf2[64];
time_fmt_iso(buf2, sizeof(buf2), t2, 0);
// 2011-11-18T15:56:35Z

Time t3 = time_date(2011, TIME_NOVEMBER, 18, 15, 56, 35, 666777888, 0);
char buf3[64];
time_fmt_iso(buf3, sizeof(buf3), t3, 0);
// 2011-11-18T15:56:35.666777888Z

Time t4 = time_date(2011, TIME_NOVEMBER, 18, 15, 56, 35, 0, -5 * 3600);
char buf4[64];
time_fmt_iso(buf4, sizeof(buf4), t4, 0);
// 2011-11-18T20:56:35Z

Time t5 = time_date(2011, TIME_NOVEMBER, 18, 15, 56, 35, 666777888, -5 * 3600);
char buf5[64];
time_fmt_iso(buf5, sizeof(buf5), t5, 0);
// 2011-11-18T20:56:35.666777888Z
```

## Extracting time fields

There are a number of functions for extracting different time fields.

### time_get_year

```c
int time_get_year(Time t);
```

Returns the year in which t occurs.

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 431295000, 0);
int year = time_get_year(t);
// 2024
```

### time_get_month

```c
enum Month time_get_month(Time t);
```

Returns the month of the year specified by t (1-12).

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 431295000, 0);
enum Month month = time_get_month(t);
// TIME_AUGUST (8)
```

### time_get_day

```c
int time_get_day(Time t);
```

Returns the day of the month specified by t (1-31).

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 431295000, 0);
int day = time_get_day(t);
// 6
```

### time_get_hour

```c
int time_get_hour(Time t);
```

Returns the hour within the day specified by t (0-23).

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 431295000, 0);
int hour = time_get_hour(t);
// 21
```

### time_get_minute

```c
int time_get_minute(Time t);
```

Returns the minute offset within the hour specified by t.

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 431295000, 0);
int minute = time_get_minute(t);
// 22
```

### time_get_second

```c
int time_get_second(Time t);
```

Returns the second offset within the minute specified by t (0-59).

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 431295000, 0);
int second = time_get_second(t);
// 15
```

### time_get_nano

```c
int time_get_nano(Time t);
```

Returns the nanosecond offset within the second specified by t (0-999999999).

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 431295000, 0);
int nano = time_get_nano(t);
// 431295000
```

### time_get_weekday

```c
enum Weekday time_get_weekday(Time t);
```

Returns the day of the week specified by t (0-6, Sunday = 0).

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 431295000, 0);
enum Weekday weekday = time_get_weekday(t);
// Tuesday (2)
```

### time_get_yearday

```c
int time_get_yearday(Time t);
```

Returns the day of the year specified by t (1-366).

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 431295000, 0);
int yearday = time_get_yearday(t);
// 219
```

### time_get_isoweek

```c
void time_get_isoweek(Time t, int* year, int* week);
```

Returns the ISO 8601 year and week number in which t occurs.

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 431295000, 0);
int iso_year, iso_week;
time_get_isoweek(t, &iso_year, &iso_week);
// iso_year = 2024, iso_week = 32
```

### time_get_date

```c
void time_get_date(Time t, int* year, enum Month* month, int* day);
```

Returns the year, month, and day in which t occurs.

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 431295000, 0);
int year, day;
enum Month month;
time_get_date(t, &year, &month, &day);
// year = 2024, month = TIME_AUGUST, day = 6
```

### time_get_clock

```c
void time_get_clock(Time t, int* hour, int* min, int* sec);
```

Returns the hour, minute, and second within the day specified by t.

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 431295000, 0);
int hour, min, sec;
time_get_clock(t, &hour, &min, &sec);
// hour = 21, min = 22, sec = 15
```

## Unix time

Functions for converting time values to/from Unix time (time since the Unix epoch - January 1, 1970 UTC).

### time_unix

```c
Time time_unix(int64_t sec, int64_t nsec);
```

Returns the Time corresponding to the given Unix time, `sec` seconds and `nsec` nanoseconds since January 1, 1970 UTC.

```c
Time t = time_unix(1321631795, 666777888);
char buf[64];
time_fmt_iso(buf, sizeof(buf), t, 0);
// "2011-11-18T15:56:35.666777888Z"
```

### time_milli

```c
Time time_milli(int64_t msec);
```

Returns the Time corresponding to the given Unix time, `msec` milliseconds since January 1, 1970 UTC.

```c
Time t = time_milli(1321631795666);
char buf[64];
time_fmt_iso(buf, sizeof(buf), t, 0);
// "2011-11-18T15:56:35.666000000Z"
```

### time_micro

```c
Time time_micro(int64_t usec);
```

Returns the Time corresponding to the given Unix time, `usec` microseconds since January 1, 1970 UTC.

```c
Time t = time_micro(1321631795666777);
char buf[64];
time_fmt_iso(buf, sizeof(buf), t, 0);
// "2011-11-18T15:56:35.666777000Z"
```

### time_nano

```c
Time time_nano(int64_t nsec);
```

Returns the Time corresponding to the given Unix time, `nsec` nanoseconds since January 1, 1970 UTC.

```c
Time t = time_nano(1321631795666777888);
char buf[64];
time_fmt_iso(buf, sizeof(buf), t, 0);
// "2011-11-18T15:56:35.666777888Z"
```

### time_to_unix

```c
int64_t time_to_unix(Time t);
```

Returns t as a Unix time, the number of seconds elapsed since January 1, 1970 UTC.

Unix-like operating systems often record time as a 32-bit number of seconds, but since `time_to_unix` returns a 64-bit value, it is valid for billions of years into the past or future.

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 431295000, 0);
int64_t unix_sec = time_to_unix(t);
// 1722979335
```

### time_to_milli

```c
int64_t time_to_milli(Time t);
```

Returns t as a Unix time, the number of milliseconds elapsed since January 1, 1970 UTC.

The result is undefined if the Unix time in milliseconds cannot be represented by a 64-bit integer (a date more than 292 million years before or after 1970).

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 431295000, 0);
int64_t unix_msec = time_to_milli(t);
// 1722979335431
```

### time_to_micro

```c
int64_t time_to_micro(Time t);
```

Returns t as a Unix time, the number of microseconds elapsed since January 1, 1970 UTC.

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 431295000, 0);
int64_t unix_usec = time_to_micro(t);
// 1722979335431295
```

### time_to_nano

```c
int64_t time_to_nano(Time t);
```

Returns t as a Unix time, the number of nanoseconds elapsed since January 1, 1970 UTC.

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 431295000, 0);
int64_t unix_nsec = time_to_nano(t);
// 1722979335431295000
```

## Calendar time

Functions for converting time values to/from calendar time.

### time_tm

```c
Time time_tm(struct tm tm, int offset_sec);
```

Returns the Time corresponding to the given calendar time at the given timezone offset.

```c
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
// "2011-11-18T15:56:35Z"
```

### time_to_tm

```c
struct tm time_to_tm(Time t, int offset_sec);
```

Returns t in the given timezone offset as a calendar time.

```c
Time t = time_date(2011, TIME_NOVEMBER, 18, 15, 56, 35, 0, 0);
struct tm tm = time_to_tm(t, 0);
// tm.tm_year = 111, tm.tm_mon = 10, tm.tm_mday = 18,
// tm.tm_hour = 15, tm.tm_min = 56, tm.tm_sec = 35
```

## Time comparison

Functions for comparing time values.

### time_after

```c
bool time_after(Time t, Time u);
```

Reports whether the time instant t is after u.

```c
Time t1 = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 0, 0);
Time t2 = time_date(2024, TIME_AUGUST, 6, 21, 22, 16, 0, 0);
bool after = time_after(t2, t1);
// true
```

### time_before

```c
bool time_before(Time t, Time u);
```

Reports whether the time instant t is before u.

```c
Time t1 = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 0, 0);
Time t2 = time_date(2024, TIME_AUGUST, 6, 21, 22, 16, 0, 0);
bool before = time_before(t1, t2);
// true
```

### time_compare

```c
int time_compare(Time t, Time u);
```

Compares the time instant t with u. Returns -1 if t < u, 0 if t == u, +1 if t > u.

```c
Time t1 = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 0, 0);
Time t2 = time_date(2024, TIME_AUGUST, 6, 21, 22, 16, 0, 0);
int cmp = time_compare(t1, t2);
// -1
```

### time_equal

```c
bool time_equal(Time t, Time u);
```

Reports whether t and u represent the same time instant.

```c
Time t1 = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 0, 0);
Time t2 = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 0, 0);
bool equal = time_equal(t1, t2);
// true
```

### time_is_zero

```c
bool time_is_zero(Time t);
```

Reports whether t represents the zero time instant, January 1, year 1, 00:00:00 UTC.

```c
Time t = {0, 0};
bool is_zero = time_is_zero(t);
// true
```

## Time arithmetic

Functions for adding and subtracting time values.

### time_add

```c
Time time_add(Time t, Duration d);
```

Returns the time t+d.

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 0, 0);
Duration d = 30 * TIME_SECOND;
Time result = time_add(t, d);
char buf[64];
time_fmt_iso(buf, sizeof(buf), result, 0);
// "2024-08-06T21:22:45Z"
```

### time_sub

```c
Duration time_sub(Time t, Time u);
```

Returns the duration t-u.

```c
Time t1 = time_date(2024, TIME_AUGUST, 6, 21, 22, 45, 0, 0);
Time t2 = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 0, 0);
Duration d = time_sub(t1, t2);
// d == 30 * TIME_SECOND
```

### time_since

```c
Duration time_since(Time t);
```

Returns the time elapsed since t.

```c
Time past = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 0, 0);
Duration elapsed = time_since(past);
// elapsed is the duration since past
```

### time_until

```c
Duration time_until(Time t);
```

Returns the duration until t.

```c
Time future = time_date(2034, TIME_AUGUST, 6, 21, 22, 45, 0, 0);
Duration remaining = time_until(future);
// remaining is the duration until future
```

### time_add_date

```c
Time time_add_date(Time t, int years, int months, int days);
```

Returns the time corresponding to adding the given number of years, months, and days to t.

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 0, 0);
Time result = time_add_date(t, 0, 0, 1);
char buf[64];
time_fmt_iso(buf, sizeof(buf), result, 0);
// "2024-08-07T21:22:15Z"
```

## Rounding

Functions for rounding and truncating time values.

### time_truncate

```c
Time time_truncate(Time t, Duration d);
```

Returns the result of rounding t down to a multiple of d (since the zero time).

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 500000000, 0);
Duration d = 10 * TIME_SECOND;
Time result = time_truncate(t, d);
char buf[64];
time_fmt_iso(buf, sizeof(buf), result, 0);
// "2024-08-06T21:22:10Z"
```

### time_round

```c
Time time_round(Time t, Duration d);
```

Returns the result of rounding t to the nearest multiple of d.

```c
Time t = time_date(2024, TIME_AUGUST, 6, 21, 22, 15, 500000000, 0);
Duration d = 10 * TIME_SECOND;
Time result = time_round(t, d);
char buf[64];
time_fmt_iso(buf, sizeof(buf), result, 0);
// "2024-08-06T21:22:20Z"
```

## Formatting

Functions for formatting and parsing time values.

### time_fmt_iso

```c
size_t time_fmt_iso(char* buf, size_t size, Time t, int offset_sec);
```

Returns an ISO 8601 time string for the given time value. Converts the time value to the given timezone offset before formatting.

Chooses the most compact representation:

```text
2006-01-02T15:04:05.999999999+07:00
2006-01-02T15:04:05.999999999Z
2006-01-02T15:04:05+07:00
2006-01-02T15:04:05Z
```

```c
Time t = time_date(2011, TIME_NOVEMBER, 18, 15, 56, 35, 666777888, 0);
char buf[64];
size_t n = time_fmt_iso(buf, sizeof(buf), t, 0);
// buf = "2011-11-18T15:56:35.666777888Z"
```

### time_fmt_datetime

```c
size_t time_fmt_datetime(char* buf, size_t size, Time t, int offset_sec);
```

Returns a datetime string (2006-01-02 15:04:05) for the given time value. Converts the time value to the given timezone offset before formatting.

```c
Time t = time_date(2011, TIME_NOVEMBER, 18, 15, 56, 35, 0, 0);
char buf[64];
size_t n = time_fmt_datetime(buf, sizeof(buf), t, 0);
// buf = "2011-11-18 15:56:35"
```

### time_fmt_date

```c
size_t time_fmt_date(char* buf, size_t size, Time t, int offset_sec);
```

Returns a date string (2006-01-02) for the given time value. Converts the time value to the given timezone offset before formatting.

```c
Time t = time_date(2011, TIME_NOVEMBER, 18, 15, 56, 35, 0, 0);
char buf[64];
size_t n = time_fmt_date(buf, sizeof(buf), t, 0);
// buf = "2011-11-18"
```

### time_fmt_time

```c
size_t time_fmt_time(char* buf, size_t size, Time t, int offset_sec);
```

Returns a time string (15:04:05) for the given time value. Converts the time value to the given timezone offset before formatting.

```c
Time t = time_date(2011, TIME_NOVEMBER, 18, 15, 56, 35, 0, 0);
char buf[64];
size_t n = time_fmt_time(buf, sizeof(buf), t, 0);
// buf = "15:56:35"
```

### time_parse

```c
Time time_parse(const char* value);
```

Parses a formatted string and returns the time value it represents.

Supports a limited set of layouts:

-   `2006-01-02T15:04:05.999999999+07:00` (ISO 8601 with nanoseconds and timezone)
-   `2006-01-02T15:04:05.999999999Z` (ISO 8601 with nanoseconds, UTC)
-   `2006-01-02T15:04:05+07:00` (ISO 8601 with timezone)
-   `2006-01-02T15:04:05Z` (ISO 8601, UTC)
-   `2006-01-02 15:04:05` (date and time, UTC)
-   `2006-01-02` (date only, UTC)
-   `15:04:05` (time only, UTC)

```c
Time t = time_parse("2011-11-18T15:56:35.666777888Z");
char buf[64];
time_fmt_iso(buf, sizeof(buf), t, 0);
// buf = "2011-11-18T15:56:35.666777888Z"
```

## Marshaling

Functions for converting time values to and from binary data.

### time_marshal_binary

```c
void time_marshal_binary(Time t, uint8_t* buf);
```

Marshals the time value into a binary blob.

```c
Time t = time_now();
uint8_t buf[TIME_BINARY_SIZE];
time_marshal_binary(t, buf);
// buf contains serialized t value
```

### time_unmarshal_binary

```c
Time time_unmarshal_binary(const uint8_t* buf);
```

Unmarshals a time value from a binary blob.

```c
Time t1 = time_date(2011, TIME_NOVEMBER, 18, 15, 56, 35, 666777888, 0);
uint8_t buf[TIME_BINARY_SIZE];
time_marshal_binary(t1, buf);
Time t2 = time_unmarshal_binary(buf);
// t2 equals t1
```

## Duration

Functions for converting duration values between different units.

### duration_to_micro

```c
int64_t duration_to_micro(Duration d);
```

Returns the duration as an integer microsecond count.

```c
Duration d = 5 * TIME_SECOND + 10 * TIME_MILLI;
int64_t micro = duration_to_micro(d);
// 5010000
```

### duration_to_milli

```c
int64_t duration_to_milli(Duration d);
```

Returns the duration as an integer millisecond count.

```c
Duration d = 5 * TIME_SECOND + 10 * TIME_MILLI;
int64_t milli = duration_to_milli(d);
// 5010
```

### duration_to_seconds

```c
double duration_to_seconds(Duration d);
```

Returns the duration as a floating point number of seconds.

```c
Duration d = 5 * TIME_SECOND + 500 * TIME_MILLI;
double sec = duration_to_seconds(d);
// 5.5
```

### duration_to_minutes

```c
double duration_to_minutes(Duration d);
```

Returns the duration as a floating point number of minutes.

```c
Duration d = 2 * TIME_MINUTE + 30 * TIME_SECOND;
double min = duration_to_minutes(d);
// 2.5
```

### duration_to_hours

```c
double duration_to_hours(Duration d);
```

Returns the duration as a floating point number of hours.

```c
Duration d = 1 * TIME_HOUR + 30 * TIME_MINUTE;
double hours = duration_to_hours(d);
// 1.5
```

## Duration rounding

Functions for rounding and truncating duration values.

### duration_truncate

```c
Duration duration_truncate(Duration d, Duration m);
```

Returns the result of rounding d toward zero to a multiple of m.

```c
Duration d = 25 * TIME_SECOND + 500 * TIME_MILLI;
Duration m = 10 * TIME_SECOND;
Duration result = duration_truncate(d, m);
// 20 * TIME_SECOND
```

### duration_round

```c
Duration duration_round(Duration d, Duration m);
```

Returns the result of rounding d to the nearest multiple of m.

```c
Duration d = 25 * TIME_SECOND + 500 * TIME_MILLI;
Duration m = 10 * TIME_SECOND;
Duration result = duration_round(d, m);
// 30 * TIME_SECOND
```

### duration_abs

```c
Duration duration_abs(Duration d);
```

Returns the absolute value of d.

```c
Duration d = -5 * TIME_SECOND;
Duration result = duration_abs(d);
// 5 * TIME_SECOND
```

## Getting started

To use the vaqt package in your C project:

1. Copy the `src/vaqt.h` header file and the source files (`src/*.c`) into your project's `vaqt` source folder.

2. Include the header in your source files:

```c
#include "vaqt/vaqt.h"
```

3. Use the functions as described in the documentation above.

4. Compile your project along with the vaqt source files. For example:

```bash
gcc -o main main.c vaqt/*.c
```

## Building from source

Build and test the package:

```
make test-all
```

Run examples:

```
make example
```

## License

Created by [Anton Zhiyanov](https://antonz.org/).

Based on Go's stdlib [time](https://github.com/golang/go/tree/master/src/time) package, available under the [BSD 3-Clause License](LICENSE).
