/*
Copyright 2020. Siwei Wang.

Interface and implementation of stopwatch.
*/
#pragma once
#include <algorithm>
#include <chrono>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <vector>

/**
 * A stopwatch that is template parameterized
 * by the unit of time to use in measuring and
 * the underlying clock.
 */
template <typename Duration = std::chrono::milliseconds,
          typename Clock = std::chrono::steady_clock>
class Stopwatch {
 private:
  /* --- MEMBER VARIABLES --- */

  // A list of recorded time point measurements.
  std::vector<typename Clock::time_point> measurements;

  // Determines iterator mode created by begin and end.
  bool sw_mode;

 public:
  /* --- PUBLIC INTERFACE --- */

  // Work with splits between recorded times.
  static inline constexpr bool SPLIT_MODE = true;

  // Work with total elapsed time periods.
  static inline constexpr bool ELAPSE_MODE = false;

  /**
   * Basic default constructor. Optional argument
   * to specify the stopwatch mode. Defaults to
   * split mode.
   */
  explicit Stopwatch(bool mode_in = SPLIT_MODE);

  /**
   * Reserve the internal buffer to the given
   * number of durations. This can improve
   * accuracy by reducing internal runtime.
   * Optional argument to specify the stopwatch
   * mode. Defaults to split mode.
   */
  explicit Stopwatch(size_t res, bool = SPLIT_MODE);

  /**
   * Returns whether or not there are recorded
   * durations. This is not the same as the
   * number of recorded time points.
   */
  bool empty() const noexcept;

  /**
   * Get the number of recorded durations.
   * This is not the same as the number of
   * recorded time points.
   */
  size_t size() const noexcept;

  /**
   * Returns the stopwatch mode.
   */
  bool mode() const noexcept;

  /**
   * Sets the stopwatch mode.
   */
  void mode(bool mode_in) noexcept;

  /**
   * Records the current time measurement.
   * Note that there is no distinction between
   * start, split, and stop.
   * WARNING: invalidates iterators and data reference.
   */
  void record();

  /**
   * Delete all recorded time points.
   * WARNING: invalidates iterators and data reference.
   */
  void clear() noexcept;

  /**
   * Index-checked access into durations.
   * Based on stopwatch mode, determines either
   * the indexed split or elapsed time.
   */
  template <typename Integer>
  typename Duration::rep operator[](Integer index) const;

  /**
   * Yields a const reference to the underlying
   * time_point measurements made by the stopwatch.
   * WARNING: Reference is invalidated and record and clear.
   */
  const std::vector<typename Clock::time_point>& data() const noexcept;

  /**
   * Index-checked access into the
   * underlying time_point measurements.
   */
  template <typename Integer>
  typename Clock::time_point data(Integer index) const;

  /**
   * Returns the number of underlying data measurements.
   */
  size_t data_size() const noexcept;

  /**
   * A random access const iterator type that
   * gives indexed access into time splits.
   * Can be toggled between split and elapse modes.
   */
  class iterator
      : public std::iterator<
            std::random_access_iterator_tag, typename Duration::rep, ptrdiff_t,
            const typename Duration::rep*, const typename Duration::rep&> {
    friend class Stopwatch;

   private:
    // Tracks the initial time point of the underlying container.
    const typename Clock::time_point* base;
    // Tracks the underlying time point of this iterator.
    const typename Clock::time_point* ptr;
    // The mode of this iterator, determines whether it uses split or elapse.
    bool iter_mode;

    // Constructor that gives the iterator all its member variables.
    explicit iterator(const typename Clock::time_point* const,
                      const typename Clock::time_point* const, bool) noexcept;

   public:
    // Should not be able to default construct stopwatch iterators.
    iterator() = delete;

    // Returns the iterator mode.
    bool mode() const noexcept;

    // Sets the iterator mode.
    void mode(bool mode_in) noexcept;

    // Increment and decrement operators.

    iterator& operator++() noexcept;
    iterator operator++(int) noexcept;
    iterator& operator--() noexcept;
    iterator operator--(int) noexcept;

    // Gives the split pointed to by this iterator.
    typename Duration::rep operator*() const;
    typename Duration::rep operator[](ptrdiff_t dist) const;
    // This iterator has no arrow operator.

    // Comparison operators.

    bool operator==(const iterator& other) const noexcept;
    bool operator!=(const iterator& other) const noexcept;
    bool operator<(const iterator& other) const noexcept;
    bool operator<=(const iterator& other) const noexcept;
    bool operator>(const iterator& other) const noexcept;
    bool operator>=(const iterator& other) const noexcept;

    // Arithmetic operators.

    iterator& operator+=(ptrdiff_t dist) noexcept;
    iterator& operator-=(ptrdiff_t dist) noexcept;
    iterator operator+(ptrdiff_t dist) const noexcept;
    iterator operator-(ptrdiff_t dist) const noexcept;
    ptrdiff_t operator-(const iterator& other) const;

    /*
    Note: friend functions are not used because mixing
    friends and templates is absolutely the worst!
    */
  };

  /**
   * Iterator to first split. The iterator
   * is given the stopwatch mode.
   */
  iterator begin() const noexcept;

  /**
   * Iterator to first split. The iterator
   * is given the stopwatch mode.
   */
  iterator end() const noexcept;

  /**
   * Addition operator interleaves the result of other into this.
   */
  Stopwatch& operator+=(const Stopwatch&);

  /**
   * Returns a new Stopwatch with the iterleaving of times.
   */
  Stopwatch operator+(const Stopwatch&);
};

/* --- TEMPLATE IMPLEMENTATION --- */

template <typename Duration, typename Clock>
inline Stopwatch<Duration, Clock>::Stopwatch(bool mode_in) : sw_mode(mode_in) {
  measurements.reserve(2);
}

template <typename Duration, typename Clock>
inline Stopwatch<Duration, Clock>::Stopwatch(size_t res, bool mode_in)
    : sw_mode(mode_in) {
  measurements.reserve(res + 1);
}

template <typename Duration, typename Clock>
inline bool Stopwatch<Duration, Clock>::empty() const noexcept {
  return measurements.size() < 2;
}

template <typename Duration, typename Clock>
inline size_t Stopwatch<Duration, Clock>::size() const noexcept {
  const auto sz = measurements.size();
  // Elapsed durations is one less than measurements.
  return sz > 1 ? sz - 1 : 0;
}

template <typename Duration, typename Clock>
inline bool Stopwatch<Duration, Clock>::mode() const noexcept {
  return sw_mode;
}

template <typename Duration, typename Clock>
inline void Stopwatch<Duration, Clock>::mode(bool mode) noexcept {
  sw_mode = mode;
}

template <typename Duration, typename Clock>
inline void Stopwatch<Duration, Clock>::record() {
  measurements.emplace_back(Clock::now());
}

template <typename Duration, typename Clock>
inline void Stopwatch<Duration, Clock>::clear() noexcept {
  measurements.clear();
}

template <typename Duration, typename Clock>
template <typename Integer>
typename Duration::rep Stopwatch<Duration, Clock>::operator[](
    Integer index) const {
  static_assert(std::is_integral_v<Integer>, "Parameter must be integer type.");
  const auto end = measurements.at(index + 1);
  const auto begin =
      (sw_mode == SPLIT_MODE) ? measurements[index] : measurements.front();
  auto dur = std::chrono::duration_cast<Duration>(end - begin);
  return dur.count();
}

template <typename Duration, typename Clock>
inline const std::vector<typename Clock::time_point>&
Stopwatch<Duration, Clock>::data() const noexcept {
  return measurements;
}

template <typename Duration, typename Clock>
template <typename Integer>
inline typename Clock::time_point Stopwatch<Duration, Clock>::data(
    Integer index) const {
  static_assert(std::is_integral_v<Integer>, "Parameter must be integer type.");
  return measurements.at(index);
}

template <typename Duration, typename Clock>
inline size_t Stopwatch<Duration, Clock>::data_size() const noexcept {
  return measurements.size();
}

template <typename Duration, typename Clock>
inline typename Stopwatch<Duration, Clock>::iterator
Stopwatch<Duration, Clock>::begin() const noexcept {
  const auto base = measurements.data();
  return iterator(base, base, sw_mode);
}

template <typename Duration, typename Clock>
inline typename Stopwatch<Duration, Clock>::iterator
Stopwatch<Duration, Clock>::end() const noexcept {
  const auto base = measurements.data();
  const auto fin = measurements.empty() ? measurements.begin()
                                        : std::prev(measurements.end());
  return iterator(base, &(*fin), sw_mode);
}

template <typename Duration, typename Clock>
Stopwatch<Duration, Clock>& Stopwatch<Duration, Clock>::operator+=(
    const Stopwatch<Duration, Clock>& other) {
  decltype(measurements) new_measures;
  new_measures.reserve(measurements.size() + other.measurements.size());
  std::set_union(measurements.begin(), measurements.end(),
                 other.measurements.begin(), other.measurements.end(),
                 std::back_inserter(new_measures));
  measurements.swap(new_measures);
  return *this;
}

template <typename Duration, typename Clock>
Stopwatch<Duration, Clock> Stopwatch<Duration, Clock>::operator+(
    const Stopwatch<Duration, Clock>& other) {
  auto temp(*this);
  return temp += other;
}

template <typename Duration, typename Clock>
inline bool Stopwatch<Duration, Clock>::iterator::mode() const noexcept {
  return iter_mode;
}

template <typename Duration, typename Clock>
inline void Stopwatch<Duration, Clock>::iterator::mode(bool mode) noexcept {
  iter_mode = mode;
}

template <typename Duration, typename Clock>
inline Stopwatch<Duration, Clock>::iterator::iterator(
    const typename Clock::time_point* const base_in,
    const typename Clock::time_point* const ptr_in, bool mode_in) noexcept
    : base(base_in), ptr(ptr_in), iter_mode(mode_in) {}

template <typename Duration, typename Clock>
inline typename Stopwatch<Duration, Clock>::iterator&
Stopwatch<Duration, Clock>::iterator::operator++() noexcept {
  ++ptr;
  return *this;
}

template <typename Duration, typename Clock>
inline typename Stopwatch<Duration, Clock>::iterator
Stopwatch<Duration, Clock>::iterator::operator++(int) noexcept {
  auto temp(*this);
  ++ptr;
  return temp;
}

template <typename Duration, typename Clock>
inline typename Stopwatch<Duration, Clock>::iterator&
Stopwatch<Duration, Clock>::iterator::operator--() noexcept {
  --ptr;
  return *this;
}

template <typename Duration, typename Clock>
inline typename Stopwatch<Duration, Clock>::iterator
Stopwatch<Duration, Clock>::iterator::operator--(int) noexcept {
  auto temp(*this);
  --ptr;
  return temp;
}

template <typename Duration, typename Clock>
typename Duration::rep Stopwatch<Duration, Clock>::iterator::operator*() const {
  const auto end = *std::next(ptr);
  const auto begin = (iter_mode == SPLIT_MODE) ? *ptr : *base;
  auto dur = std::chrono::duration_cast<Duration>(end - begin);
  return dur.count();
}

template <typename Duration, typename Clock>
inline typename Duration::rep Stopwatch<Duration, Clock>::iterator::operator[](
    ptrdiff_t dist) const {
  return *(*this + dist);
}

template <typename Duration, typename Clock>
inline bool Stopwatch<Duration, Clock>::iterator::operator==(
    const typename Stopwatch<Duration, Clock>::iterator& other) const noexcept {
  return ptr == other.ptr && base == other.base;
}

template <typename Duration, typename Clock>
inline bool Stopwatch<Duration, Clock>::iterator::operator!=(
    const typename Stopwatch<Duration, Clock>::iterator& other) const noexcept {
  return ptr != other.ptr || base != other.base;
}

template <typename Duration, typename Clock>
inline bool Stopwatch<Duration, Clock>::iterator::operator<(
    const typename Stopwatch<Duration, Clock>::iterator& other) const noexcept {
  return other.ptr - ptr > 0 && base == other.base;
}

template <typename Duration, typename Clock>
inline bool Stopwatch<Duration, Clock>::iterator::operator<=(
    const typename Stopwatch<Duration, Clock>::iterator& other) const noexcept {
  return other.ptr - ptr >= 0 && base == other.base;
}

template <typename Duration, typename Clock>
inline bool Stopwatch<Duration, Clock>::iterator::operator>(
    const typename Stopwatch<Duration, Clock>::iterator& other) const noexcept {
  return ptr - other.ptr > 0 && base == other.base;
}

template <typename Duration, typename Clock>
inline bool Stopwatch<Duration, Clock>::iterator::operator>=(
    const typename Stopwatch<Duration, Clock>::iterator& other) const noexcept {
  return ptr - other.base >= 0 && base == other.base;
}

template <typename Duration, typename Clock>
inline typename Stopwatch<Duration, Clock>::iterator&
Stopwatch<Duration, Clock>::iterator::operator+=(ptrdiff_t dist) noexcept {
  ptr += dist;
  return (*this);
}

template <typename Duration, typename Clock>
inline typename Stopwatch<Duration, Clock>::iterator&
Stopwatch<Duration, Clock>::iterator::operator-=(ptrdiff_t dist) noexcept {
  ptr -= dist;
  return (*this);
}

template <typename Duration, typename Clock>
inline typename Stopwatch<Duration, Clock>::iterator
Stopwatch<Duration, Clock>::iterator::operator+(ptrdiff_t dist) const noexcept {
  auto temp(*this);
  return temp += dist;
}

template <typename Duration, typename Clock>
inline typename Stopwatch<Duration, Clock>::iterator
Stopwatch<Duration, Clock>::iterator::operator-(ptrdiff_t dist) const noexcept {
  auto temp(*this);
  return temp -= dist;
}

template <typename Duration, typename Clock>
inline ptrdiff_t Stopwatch<Duration, Clock>::iterator::operator-(
    const typename Stopwatch<Duration, Clock>::iterator& other) const {
  if (base != other.base) {
    throw std::runtime_error("Iterator base mismatch.");
  }
  return ptr - other.ptr;
}
