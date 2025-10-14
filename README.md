# vaqt: High-precision date/time in C

The `vaqt` package offers data types and functions for handling time and duration in C, with nanosecond precision.

[Concepts](#concepts) •
[Features](#features) •
[Getting started](#getting-started) •
[Building from source](#building-from-source) •
[API reference](doc/api.md)

## Concepts

This package works with two types of values: `Time` and `Duration`.

Time is a pair (seconds, nanoseconds), where `seconds` is the 64-bit number of seconds since zero time (0001-01-01 00:00:00 UTC) and `nanoseconds` is the number of nanoseconds within the current second (0-999999999). Time can represent dates billions of years in the past or future with nanosecond precision.

```text
  since     within
  0-time    second
┌─────────┬─────────────┐
│ seconds │ nanoseconds │
└─────────┴─────────────┘
  64 bit    32 bit
```

Time is always operated in UTC, but you can convert it from/to a specific timezone.

Duration is a 64-bit number of nanoseconds. It can represent values up to about 290 years.

```text
┌─────────────┐
│ nanoseconds │
└─────────────┘
  64 bit
```

## Features

This package provides functions for common date and time operations.

Creating time values:

```text
time_now()
time_date(year, month, day, hour, min, sec, nsec, offset_sec)
```

Extracting time fields:

```text
time_get_year(t)
time_get_month(t)
time_get_day(t)
time_get_clock(t)
time_get_hour(t)
time_get_minute(t)
time_get_second(t)
time_get_nano(t)
time_get_weekday(t)
time_get_yearday(t)
time_get_isoweek(t)
```

Unix time:

```text
time_unix(sec, nsec)
time_unix_milli(msec)
time_unix_micro(usec)
time_unix_nano(nsec)
time_to_unix(t)
time_to_unix_milli(t)
time_to_unix_micro(t)
time_to_unix_nano(t)
```

Calendar time:

```text
time_tm(tm, offset_sec)
time_to_tm(t, offset_sec)
```

Time comparison:

```text
time_after(t, u)
time_before(t, u)
time_compare(t, u)
time_equal(t, u)
time_is_zero(t)
```

Time arithmetic:

```text
time_add(t, d)
time_add_date(t, years, months, days)
time_sub(t, u)
time_since(t)
time_until(t)
```

Rounding:

```text
time_truncate(t, d)
time_round(t, d)
```

Formatting:

```text
time_fmt_iso(t, offset_sec)
time_fmt_datetime(t, offset_sec)
time_fmt_date(t, offset_sec)
time_fmt_time(t, offset_sec)
time_parse(s)
```

Marshaling:

```text
time_unmarshal_binary(b);
time_marshal_binary(t)
```

Check the [API reference](doc/api.md) for more details.

## Getting started

To use `vaqt` in your C project:

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
