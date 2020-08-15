/*
Copyright 2020. Siwei Wang.

Unit testing for stopwatch class.
*/
#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <numeric>
#include <random>
#include <thread>
#include <type_traits>
#include "framework.h"
#include "stopwatch.h"
using std::array;
using std::bind;
using std::cout;
using std::default_random_engine;
using std::distance;
using std::endl;
using std::equal;
using std::includes;
using std::ios_base;
using std::is_integral_v;
using std::is_sorted;
using std::partial_sum;
using std::uniform_int_distribution;
using std::vector;
using std::chrono::duration_cast;
using std::chrono::system_clock;
using std::placeholders::_1;
using std::placeholders::_2;
using std::this_thread::sleep_for;

// WARNING: nanoseconds is too fine grained for sleep_for.
using time_unit = std::chrono::milliseconds;

// The margin of error we are willing to accept.
static constexpr time_unit::rep epsilon = 2;

/**
 * Generate a random sample of N integers
 * in the inclusive range [a, b].
 */
template <typename T, size_t N = 20>
array<T, N> randint_sample(T a, T b);

/**
 * Checks that experimental is within error
 * of the actual measurement.
 */
bool approx(decltype(epsilon) actual, decltype(epsilon) experimental,
            decltype(epsilon) error);

/**
 * Print the contents of container to stdout.
 * REQUIRES: Container is for-range iterable.
 */
template <typename Container>
void print_range(const Container&);

/**
 * Return a stop watch with record called
 * at the given time intervals in the given mode.
 */
template <typename Times>
Stopwatch<time_unit> recorded(const Times&,
                              bool mode = Stopwatch<>::SPLIT_MODE);

// Unit tests for stopwatch.
namespace Test {
void test_sizemode();
void test_split();
void test_elapsed();
void test_iterate();
void test_compare();
void test_arithmetic();
void test_data();
void test_interleave();
}  // namespace Test

int main() {
  ios_base::sync_with_stdio(false);

  Framework fr;
  fr.emplace("size mode", Test::test_sizemode);
  fr.emplace("split", Test::test_split);
  fr.emplace("elapsed", Test::test_elapsed);
  fr.emplace("iterate", Test::test_iterate);
  fr.emplace("compare", Test::test_compare);
  fr.emplace("arithmetic", Test::test_arithmetic);
  fr.emplace("data", Test::test_data);
  fr.emplace("interleave", Test::test_interleave);

  fr.run_all();
  cout << fr << "Passed " << fr.passed() << " out of " << fr.executed_size()
       << " tests.\n";
}

template <typename T, size_t N>
array<T, N> randint_sample(T a, T b) {
  static_assert(is_integral_v<T>, "Integer type required.");
  static const auto seed = system_clock::now().time_since_epoch().count();
  static default_random_engine gen(seed);

  uniform_int_distribution<T> distr(a, b);
  array<T, N> arr;
  for (unsigned i = 0; i < N; ++i) arr[i] = distr(gen);
  return arr;
}

bool approx(decltype(epsilon) actual, decltype(epsilon) experimental,
            decltype(epsilon) error) {
  return actual - error <= experimental && experimental <= actual + error;
}

template <typename Container>
void print_range(const Container& items) {
  for (const auto& x : items) cout << x << ' ';
  cout << '\n';
}

template <typename Times>
Stopwatch<time_unit> recorded(const Times& times, bool mode) {
  Stopwatch<time_unit> sw(times.size(), mode);
  sw.record();
  for (const auto t : times) {
    sleep_for(time_unit(t));
    sw.record();
  }
  return sw;
}

void Test::test_sizemode() {
  const auto times = randint_sample<unsigned, 5>(10, 20);
  auto sw = recorded(times);
  assert_false(sw.empty(), "Stopwatch is not empty.");
  assert_eq(sw.size(), times.size(), "Incorrect stopwatch size.");

  const auto one_time = randint_sample<unsigned, 1>(10, 20);
  auto one_sw = recorded(one_time);
  assert_false(one_sw.empty(), "One stopwatch is not empty.");
  assert_eq(one_sw.size(), one_time.size(), "Stopwatch size is non-zero.");

  const auto zero_time = randint_sample<unsigned, 0>(10, 20);
  auto zero_sw = recorded(zero_time, Stopwatch<>::ELAPSE_MODE);
  assert_true(zero_sw.empty(), "Zero stopwatch should be empty.");
  assert_eq(zero_sw.size(), zero_time.size(), "Stopwatch size should be 0.");
  assert_eq(zero_sw.mode(), Stopwatch<>::ELAPSE_MODE,
            "Stopwatch should be in elapse mode.");

  sw.clear();
  assert_true(sw.empty(), "Nonempty stopwatch after clear.");
  assert_eq(sw.size(), static_cast<unsigned>(0),
            "Non zero stopwatch size after clear.");

  assert_eq(one_sw.mode(), Stopwatch<>::SPLIT_MODE,
            "Default mode should be split.");
  one_sw.mode(Stopwatch<>::ELAPSE_MODE);
  assert_eq(one_sw.mode(), Stopwatch<>::ELAPSE_MODE,
            "Stopwatch mode did not switch to elapse.");
}

void Test::test_split() {
  const auto times = randint_sample<unsigned, 40>(10, 30);
  auto sw = recorded(times);

  assert_eq(sw.mode(), Stopwatch<>::SPLIT_MODE,
            "Stopwatch should be in split mode.");
  assert_eq(sw.size(), times.size(), "Stopwatch is missing measurements.");

  assert_true(equal(times.begin(), times.end(), sw.begin(),
                    bind(approx, _1, _2, epsilon)),
              "Stopwatch splits are inaccurate.");

  for (size_t i = 0; i < sw.size(); ++i) {
    assert_true(approx(sw[i], times[i], epsilon),
                "Stopwatch splits don't match iteration.");
  }
}

void Test::test_elapsed() {
  auto times = randint_sample<unsigned, 40>(10, 30);
  auto sw = recorded(times, Stopwatch<>::ELAPSE_MODE);

  auto partials(times);
  partial_sum(times.begin(), times.end(), partials.begin());

  assert_eq(sw.mode(), Stopwatch<>::ELAPSE_MODE,
            "Stopwatch should be in elapse mode.");
  assert_eq(sw.size(), times.size(), "Stopwatch is missing measurements.");

  // Error bounds grow as we increase.
  for (size_t i = 0; i < sw.size(); ++i) {
    assert_true(approx(sw[i], partials[i], epsilon * (i + 1)),
                "Stopwatch elapses are inaccurate.");
  }
}

void Test::test_iterate() {
  const auto times = randint_sample<unsigned, 10>(10, 30);
  auto sw = recorded(times);
  assert_eq(sw.size(), times.size(), "Stopwatch is missing measurements.");

  unsigned i = 0;
  for (auto iter = sw.begin(); iter != sw.end(); iter++) {
    assert_eq(sw.mode(), Stopwatch<>::SPLIT_MODE,
              "Stopwatch should be in split mode.");
    assert_eq(iter.mode(), Stopwatch<>::SPLIT_MODE,
              "Iterator should be in split mode.");
    assert_eq(*iter, sw[i], "Split iterator should match stopwatch.");

    sw.mode(Stopwatch<>::ELAPSE_MODE);
    iter.mode(Stopwatch<>::ELAPSE_MODE);

    assert_eq(sw.mode(), Stopwatch<>::ELAPSE_MODE,
              "Stopwatch should be in elapse mode.");
    assert_eq(iter.mode(), Stopwatch<>::ELAPSE_MODE,
              "Iterator should be in elapse mode.");
    assert_eq(*iter, sw[i], "Elapse iterator should match stopwatch.");

    sw.mode(Stopwatch<>::SPLIT_MODE);
    iter.mode(Stopwatch<>::SPLIT_MODE);

    ++i;
  }
  auto end = sw.end();
  for (unsigned j = 0; j < sw.size(); ++j) {
    end--;
  }
  assert_eq(sw.begin(), end, "End should be decremented to begin.");

  Stopwatch edge;
  assert_eq(edge.begin(), edge.end(), "Empty stopwatch has no range.");
  assert_eq(distance(edge.begin(), edge.end()), 0,
            "Empty stopwatch has no range.");
  edge.record();
  assert_eq(edge.begin(), edge.end(),
            "Stopwatch with one snapshot has no range.");
  assert_eq(distance(edge.begin(), edge.end()), 0,
            "Stopwatch with one snapshot has no range.");
  for (unsigned j = 0; j < 5; ++j) {
    edge.record();
    assert_less(edge.begin(), edge.end(), "Begin should be less than end.");
    assert_eq(distance(edge.begin(), edge.end()), j + 1,
              "Stopwatch with j snapshots has j - 1 range.");
  }
}

void Test::test_compare() {
  const auto times = randint_sample<unsigned, 2>(10, 30);
  auto sw = recorded(times);
  assert_eq(sw.size(), times.size(), "Stopwatch is missing measurements.");
  auto begin = sw.begin();
  auto end = sw.end();

  assert_eq(begin, begin, "Begin is equal to itself.");
  assert_leq(begin, begin, "Begin is less than or equal to itself.");
  assert_geq(begin, begin, "Begin is greater than or equal to itself.");

  assert_eq(end, end, "End is equal to itself.");
  assert_leq(end, end, "End is less than or equal to itself.");
  assert_geq(end, end, "End is greater than or equal to itself.");

  assert_less(begin, end, "Begin is less than end.");
  assert_leq(begin, end, "Begin is less than or equal to end.");
  assert_greater(end, begin, "End is greater than begin.");
  assert_geq(end, begin, "End is greater than or equal to begin.");
}

void Test::test_arithmetic() {
  const auto times = randint_sample<unsigned, 10>(10, 30);
  auto sw = recorded(times);
  assert_eq(sw.size(), times.size(), "Stopwatch is missing measurements.");

  auto forward = sw.begin();
  auto backward = sw.end();
  for (int i = 0; i < static_cast<int>(sw.size()); ++i) {
    assert_eq(forward - sw.begin(), i, "Iterator subtraction failed.");
    assert_eq(sw.begin() - forward, -i,
              "Iterator negative subtraction failed.");
    assert_eq(sw.begin() + i, forward++, "Iterator numerical addition failed.");
    assert_eq(sw.end() - backward, i, "Iterator subtraction failed.");
    assert_eq(backward - sw.end(), -i, "Iterator negative subtraction failed.");
    assert_eq(sw.end() - i, backward--,
              "Iterator numerical subtraction failed.");
  }

  auto begin = sw.begin();
  for (unsigned i = 0; i < sw.size(); ++i) {
    assert_eq(begin[i], sw[i], "Iterator and stopwatch index do not agree.");
  }

  auto other = recorded(times);
  bool caught = false;
  try {
    sw.end() - other.begin();
  } catch (const std::runtime_error& err) {
    caught = true;
  }
  assert_true(caught, "Iterator base exception not thrown.");
}

void Test::test_data() {
  auto times = randint_sample<unsigned>(10, 30);
  auto sw = recorded(times);

  const auto& data = sw.data();
  assert_eq(data.size(), times.size() + 1,
            "Data size should be one greater than times.");
  assert_eq(data.size(), sw.data_size(),
            "Data size does not match returned vector size.");
  for (unsigned i = 0; i < sw.data_size(); ++i) {
    assert_eq(data[i], sw.data(i),
              "Returned data and stopwatch data do not match.");
  }
  // Check the computation was correct.
  vector<time_unit::rep> sw_splits(sw.begin(), sw.end());
  vector<time_unit::rep> comp(data.size() - 1);
  for (unsigned i = 0; i < data.size() - 1; ++i) {
    comp[i] = duration_cast<time_unit>(data[i + 1] - data[i]).count();
  }
  assert_eq(sw_splits, comp, "Computation does not match data.");
}

void Test::test_interleave() {
  const auto times_a = randint_sample<unsigned, 15>(10, 30);
  const auto times_b = randint_sample<unsigned, 25>(10, 30);
  auto sw_a = recorded(times_a);
  auto sw_b = recorded(times_b);
  const auto sw_union = sw_a + sw_b;

  assert_true(is_sorted(sw_a.data().begin(), sw_a.data().end()),
              "Stopwatch data is not sorted.");
  assert_true(is_sorted(sw_b.data().begin(), sw_b.data().end()),
              "Stopwatch data is not sorted.");
  assert_true(is_sorted(sw_union.data().begin(), sw_union.data().end()),
              "Interleaved stopwatch data is not sorted.");

  assert_true(includes(sw_union.data().begin(), sw_union.data().end(),
                       sw_a.data().begin(), sw_a.data().end()),
              "Interleaved stopwatch must be union of first argument.");
  assert_true(includes(sw_union.data().begin(), sw_union.data().end(),
                       sw_b.data().begin(), sw_b.data().end()),
              "Interleaved stopwatch must be union of second argument.");
  sw_a += sw_b;
  sw_b += sw_a;
  assert_true(
      equal(sw_a.data().begin(), sw_a.data().end(), sw_b.data().begin()),
      "Two stopwatches must be equal after mutual interleaving.");
  assert_true(
      equal(sw_b.data().begin(), sw_b.data().end(), sw_union.data().begin()),
      "Stopwatches must be equal to union after interleaving.");

  assert_true(is_sorted(sw_a.data().begin(), sw_a.data().end()),
              "Stopwatch data is not sorted.");
  assert_true(is_sorted(sw_b.data().begin(), sw_b.data().end()),
              "Stopwatch data is not sorted.");
}
