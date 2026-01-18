#pragma once
#include <cstddef>

class Registry;

class Entity {
 public:
  explicit Entity(size_t id = 0) : m_id(id) {}

  operator size_t() const { return m_id; }

  size_t id() const { return m_id; }

 private:
  size_t m_id;

  friend class Registry;
};
