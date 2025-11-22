#pragma once
#include <tuple>
#include <utility>
#include <algorithm>

#include "engine/ecs/Zipper.hpp"

template <class... Containers>
class Zipper;

template <class... Containers>
class ZipperIterator {
 private:
  template <class Container>
  using iterator_t = decltype(std::declval<Container&>().begin());

  template <class Container>
  using it_reference_t = typename iterator_t<Container>::reference;

  template <class Container>
  using component_t = typename std::remove_reference_t<
      decltype(std::declval<it_reference_t<Container>>().value())>;

 public:
  using value_type = std::tuple<component_t<Containers>&...>;
  using reference = value_type;
  using pointer = void;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;
  using iterator_tuple = std::tuple<iterator_t<Containers>...>;

  friend Zipper<Containers...>;

 private:
  ZipperIterator(iterator_tuple const& it_tuple, size_t max)
      : m_current(it_tuple), m_max(max), m_idx(0) {
    if (!all_set(m_seq)) {
      advance_to_next_valid();
    }
  }

 public:
  ZipperIterator(const ZipperIterator&) = default;
  ZipperIterator& operator=(const ZipperIterator&) = default;

  // Pre-increment: ++it
  ZipperIterator& operator++() {
    incr_all(m_seq);
    advance_to_next_valid();
    return *this;
  }

  // Post-increment: it++
  ZipperIterator operator++(int) {
    ZipperIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  value_type operator*() { return to_value(m_seq); }

  value_type operator->() { return to_value(m_seq); }

  friend bool operator==(const ZipperIterator& lhs, const ZipperIterator& rhs) {
    return lhs.m_idx == rhs.m_idx;
  }

  friend bool operator!=(const ZipperIterator& lhs, const ZipperIterator& rhs) {
    return !(lhs == rhs);
  }

 private:
  template <size_t... Is>
  void incr_all(std::index_sequence<Is...>) {
    m_idx++;
    (++std::get<Is>(m_current), ...);
  }

  template <size_t... Is>
  bool all_set(std::index_sequence<Is...>) {
    if (m_idx >= m_max) return false;
    return (std::get<Is>(m_current)->has_value() && ...);
  }

  template <size_t... Is>
  value_type to_value(std::index_sequence<Is...>) {
    return std::tie(std::get<Is>(m_current)->value()...);
  }

  void advance_to_next_valid() {
    while (m_idx < m_max && !all_set(m_seq)) {
      incr_all(m_seq);
    }
  }

 private:
  iterator_tuple m_current;
  size_t m_max;
  size_t m_idx;
  static constexpr std::index_sequence_for<Containers...> m_seq{};
};

template <class... Containers>
class Zipper {
 public:
  using iterator = ZipperIterator<Containers...>;
  using iterator_tuple = typename iterator::iterator_tuple;

  explicit Zipper(Containers&... cs)
      : m_begin(compute_begin(cs...)),
        m_end(compute_end(cs...)),
        m_size(compute_size(cs...)) {}

  iterator begin() { return iterator(m_begin, m_size); }

  iterator end() { return iterator(m_end, m_size); }

 private:
  static size_t compute_size(Containers&... containers) {
    return std::max({containers.size()...});
  }

  static iterator_tuple compute_begin(Containers&... containers) {
    return std::make_tuple(containers.begin()...);
  }

  static iterator_tuple compute_end(Containers&... containers) {
    return std::make_tuple(containers.end()...);
  }

 private:
  iterator_tuple m_begin;
  iterator_tuple m_end;
  size_t m_size;
};

template <class... Containers>
class IndexedZipperIterator {
 private:
  template <class Container>
  using iterator_t = decltype(std::declval<Container&>().begin());

  template <class Container>
  using it_reference_t = typename iterator_t<Container>::reference;

  template <class Container>
  using component_t = typename std::remove_reference_t<
      decltype(std::declval<it_reference_t<Container>>().value())>;

 public:
  using value_type = std::tuple<size_t, component_t<Containers>&...>;
  using reference = value_type;
  using pointer = void;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;
  using iterator_tuple = std::tuple<iterator_t<Containers>...>;

  template <class... Cs>
  friend class IndexedZipper;

 private:
  IndexedZipperIterator(iterator_tuple const& it_tuple, size_t max)
      : m_current(it_tuple), m_max(max), m_idx(0) {
    if (!all_set(m_seq)) {
      advance_to_next_valid();
    }
  }

 public:
  IndexedZipperIterator(const IndexedZipperIterator&) = default;
  IndexedZipperIterator& operator=(const IndexedZipperIterator&) = default;

  IndexedZipperIterator& operator++() {
    incr_all(m_seq);
    advance_to_next_valid();
    return *this;
  }

  IndexedZipperIterator operator++(int) {
    IndexedZipperIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  value_type operator*() { return to_value(m_seq); }

  value_type operator->() { return to_value(m_seq); }

  friend bool operator==(const IndexedZipperIterator& lhs,
                         const IndexedZipperIterator& rhs) {
    return lhs.m_idx == rhs.m_idx;
  }

  friend bool operator!=(const IndexedZipperIterator& lhs,
                         const IndexedZipperIterator& rhs) {
    return !(lhs == rhs);
  }

 private:
  template <size_t... Is>
  void incr_all(std::index_sequence<Is...>) {
    m_idx++;
    (++std::get<Is>(m_current), ...);
  }

  template <size_t... Is>
  bool all_set(std::index_sequence<Is...>) {
    if (m_idx >= m_max) return false;
    return (std::get<Is>(m_current)->has_value() && ...);
  }

  template <size_t... Is>
  value_type to_value(std::index_sequence<Is...>) {
    return std::make_tuple(m_idx,
                           std::ref(std::get<Is>(m_current)->value())...);
  }

  void advance_to_next_valid() {
    while (m_idx < m_max && !all_set(m_seq)) {
      incr_all(m_seq);
    }
  }

 private:
  iterator_tuple m_current;
  size_t m_max;
  size_t m_idx;
  static constexpr std::index_sequence_for<Containers...> m_seq{};
};

template <class... Containers>
class IndexedZipper {
 public:
  using iterator = IndexedZipperIterator<Containers...>;
  using iterator_tuple = typename iterator::iterator_tuple;

  explicit IndexedZipper(Containers&... cs)
      : m_begin(compute_begin(cs...)),
        m_end(compute_end(cs...)),
        m_size(compute_size(cs...)) {}

  iterator begin() { return iterator(m_begin, m_size); }

  iterator end() { return iterator(m_end, m_size); }

 private:
  static size_t compute_size(Containers&... containers) {
    return std::max({containers.size()...});
  }

  static iterator_tuple compute_begin(Containers&... containers) {
    return std::make_tuple(containers.begin()...);
  }

  static iterator_tuple compute_end(Containers&... containers) {
    return std::make_tuple(containers.end()...);
  }

 private:
  iterator_tuple m_begin;
  iterator_tuple m_end;
  size_t m_size;
};
