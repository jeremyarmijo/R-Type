#pragma once
#include <algorithm>
#include <functional>
#include <tuple>
#include <utility>

template <class... Containers>
class Zipper;

template <class... Containers>
class ZipperIterator {
 private:
  template <class Container>
  using component_t = typename std::remove_reference_t<
      decltype(std::declval<Container&>()[0].value())>;

 public:
  using value_type = std::tuple<component_t<Containers>&...>;
  using reference = value_type;
  using pointer = void;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;
  using container_tuple = std::tuple<std::reference_wrapper<Containers>...>;

  friend Zipper<Containers...>;

 private:
  ZipperIterator(container_tuple const& containers, size_t max, size_t idx = 0)
      : m_containers(containers), m_max(max), m_idx(idx) {
    if (m_idx < m_max && !all_set(m_seq)) {
      advance_to_next_valid();
    }
  }

 public:
  ZipperIterator(const ZipperIterator&) = default;
  ZipperIterator& operator=(const ZipperIterator&) = default;

  ZipperIterator& operator++() {
    ++m_idx;
    if (m_idx < m_max) {
      advance_to_next_valid();
    }
    return *this;
  }

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
  bool all_set(std::index_sequence<Is...>) {
    if (m_idx >= m_max) return false;
    auto& containers_tuple = m_containers;
    return ((m_idx < std::get<Is>(containers_tuple).get().size() &&
             std::get<Is>(containers_tuple).get()[m_idx].has_value()) &&
            ...);
  }

  template <size_t... Is>
  value_type to_value(std::index_sequence<Is...>) {
    auto& containers_tuple = m_containers;
    return std::tie(std::get<Is>(containers_tuple).get()[m_idx].value()...);
  }

  void advance_to_next_valid() {
    while (m_idx < m_max && !all_set(m_seq)) {
      ++m_idx;
    }
  }

 private:
  container_tuple m_containers;
  size_t m_max;
  size_t m_idx;
  static constexpr std::index_sequence_for<Containers...> m_seq{};
};

template <class... Containers>
class Zipper {
 public:
  using iterator = ZipperIterator<Containers...>;
  using container_tuple = typename iterator::container_tuple;

  explicit Zipper(Containers&... cs)
      : m_containers(std::ref(cs)...), m_size(compute_size(cs...)) {}

  iterator begin() { return iterator(m_containers, m_size, 0); }

  iterator end() { return iterator(m_containers, m_size, m_size); }

 private:
  static size_t compute_size(Containers&... containers) {
    return std::max({containers.size()...});
  }

 private:
  container_tuple m_containers;
  size_t m_size;
};

template <class... Containers>
class IndexedZipperIterator {
 private:
  template <class Container>
  using component_t = typename std::remove_reference_t<
      decltype(std::declval<Container&>()[0].value())>;

 public:
  using value_type = std::tuple<size_t, component_t<Containers>&...>;
  using reference = value_type;
  using pointer = void;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;
  using container_tuple = std::tuple<std::reference_wrapper<Containers>...>;

  template <class... Cs>
  friend class IndexedZipper;

 private:
  IndexedZipperIterator(container_tuple const& containers, size_t max,
                        size_t idx = 0)
      : m_containers(containers), m_max(max), m_idx(idx) {
    if (m_idx < m_max && !all_set(m_seq)) {
      advance_to_next_valid();
    }
  }

 public:
  IndexedZipperIterator(const IndexedZipperIterator&) = default;
  IndexedZipperIterator& operator=(const IndexedZipperIterator&) = default;

  IndexedZipperIterator& operator++() {
    ++m_idx;
    if (m_idx < m_max) {
      advance_to_next_valid();
    }
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
  bool all_set(std::index_sequence<Is...>) {
    if (m_idx >= m_max) return false;
    auto& containers_tuple = m_containers;
    return ((m_idx < std::get<Is>(containers_tuple).get().size() &&
             std::get<Is>(containers_tuple).get()[m_idx].has_value()) &&
            ...);
  }

  template <size_t... Is>
  value_type to_value(std::index_sequence<Is...>) {
    auto& containers_tuple = m_containers;
    return std::make_tuple(
        m_idx,
        std::ref(std::get<Is>(containers_tuple).get()[m_idx].value())...);
  }

  void advance_to_next_valid() {
    while (m_idx < m_max && !all_set(m_seq)) {
      ++m_idx;
    }
  }

 private:
  container_tuple m_containers;
  size_t m_max;
  size_t m_idx;
  static constexpr std::index_sequence_for<Containers...> m_seq{};
};

template <class... Containers>
class IndexedZipper {
 public:
  using iterator = IndexedZipperIterator<Containers...>;
  using container_tuple = typename iterator::container_tuple;

  explicit IndexedZipper(Containers&... cs)
      : m_containers(std::ref(cs)...), m_size(compute_size(cs...)) {}

  iterator begin() { return iterator(m_containers, m_size, 0); }

  iterator end() { return iterator(m_containers, m_size, m_size); }

 private:
  static size_t compute_size(Containers&... containers) {
    return std::max({containers.size()...});
  }

 private:
  container_tuple m_containers;
  size_t m_size;
};
