# Stopwatch

An industrial grade, template parameterized, lightweight stopwatch utility for time measurement in modern C++.

## Usage

To use the stopwatch, simply `#include "stopwatch.h"` somewhere in your source code. The implementation is defined in the header so no additional linking steps are necessary. The `Stopwatch` class takes two template parameters.

- `Duration`: A `std::chrono` duration type for the unit of time in which to report results. By default, this is `std::chrono::milliseconds`.
- `Clock`: A `std::chrono` clock type that is used to keep record time points. By default, this is `std::chrono::steady_clock`.

To get the most out of `Stopwatch`, use the reserve constructor and pass it the number of time durations you expect to measure. This will preallocate memory space for the underlying time point vector. The goal is to minimize the runtime impact of `record`, which emplaces the current time point onto the back of the vector.

## Modes

The stopwatch can be in one of two distinct modes.

- Split mode: Reports the number of `Duration` ticks between consecutive snapshots.
- Elapse mode: Reports the number of `Duration` ticks counting from the first snapshot.

The stopwatch mode can be configured as a constructor argument. It can also be read and modified on the fly using the `mode` function. Stopwatch iterators also have modes independent of the underlying stopwatch mode. They can be read and modified using the `iterator::mode` function. The only relationship between stopwatch and iterator modes is that `begin` and `end` will return range iterators in the same mode as the underlying stopwatch.

## Measurement

The single `record` function serves the role of start, split, and stop in a conventional stopwatch. Given n snapshots in time, there are n - 1 durations between snapshots. This latter number is the one given by the `size` function, which only starts incrementing after the second call to `record`. To clear out the measurements in the stopwatch, simply call `clear`. Note that both `record` and `clear` invalidate references to the underlying vector as well as iterators in the stopwatch.

Use `Stopwatch<>::SPLIT_MODE` or `Stopwatch<>::ELAPSE_MODE` to set the mode of the stopwatch. Then use `operator[]` to index into the stopwatch. So indexing into `i` in split mode will get the duration of time between snapshots `i` and `i + 1` (with 0-indexing). In elapse mode, it would get the duration of time between snapshots 0 and `i + 1`.

To access the raw time point data stored in the `Stopwatch`, use one of the two overloads for the `data` function. Without any parameters, it returns a const reference to its own internal vector. Given an index, it makes an index-checked access into the time point vector. Iterating over this second overload is possible using `data_size` and the idiomatic C++ for loop. Note that either `data_size` and `size` are both 0, or `data_size` is 1 larger than `size`.

## Iteration

The `Stopwatch::iterator` is a random access iterator into the *durations* measured by the time points. That is, given n snapshots, the begin and end iterators into the valid range have a distance of n - 1. Of course, when n = 0 or n = 1, the distance is 0 in both cases. Being random access, it can be incremented and decremented. It can move forward or backward by some integer number of steps in constant time. Two iterators can be compared, taking their base `Stopwatch` into account. It also defines `operator[]` that indexes as expected.

Two iterators can also be subtracted to get obtain their signed distance. The `Stopwatch` class will check that the iterators refer to the same underlying stopwatch and throw a `std::runtime_error` if this is not the case. As described above, `Stopwatch::iterator` has an additional `mode` function overload that allows reading and modifying the iterator mode. Dereferencing the iterator will give the appropriate result, consistent with its mode.

## Interleaving

Given multiple stopwatches, use `operator+=` and `operator+` to perform a sorted set union operation on the underlying measured time points. For example, given stopwatches `A` and `B`, the interleaved stopwatch `C := A + B` satisfies the triangle inequality `|A + B| <= |A| + |B|` since it discards common time points. The resulting `C` appears as if each call to snapshot on `A` or `B` concurrently induces a snapshot on `C`.

## Testing

All test cases are housed in `test.cpp`. It uses my personal unit testing framework, defined and implemented in `framework.h` and `framework.cpp`. The exact contents of the framework are not particularly relevant. To compile and run tests, simply call `make` using the included `Makefile` and execute all unit tests with `./test`.

The 8 test cases included in `test.cpp` are comprehensive in that they cover every single function declared in `stopwatch.h`. Since a stopwatch is inherently designed to measure elapsed time, the test cases may take a few seconds to execute on a modern processor. Each run of `test` will be different since it uses the current time to seed a pseudo-random number generator used to determine snapshot intervals. In addition, the `sleep_for` function from `std::this_thread` (defined in `<thread>`) is used to create time between snapshots.

Since `sleep_for` is not precise at the millisecond level, an error of 2 milliseconds is granted to the stopwatch. Consequently, it's possible that on occasion, certain runs may exceed this wiggle room. However, I have not encountered such issues at the current error bound. The time unit and epsilon value (wiggle room) can be changed at the top of `test.cpp`.
