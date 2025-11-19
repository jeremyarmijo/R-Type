#pragma once
#include <algorithm>
#include <any>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "Entity.hpp"
#include "SparseArray.hpp"

class Registry {
 public:
  template <class Component>
  SparseArray<Component>& register_component() {
    std::type_index type_idx(typeid(Component));

    m_components_arrays[type_idx] = SparseArray<Component>();

    m_erase_functions.emplace_back([type_idx](Registry& reg, const Entity& e) {
      auto& arr = std::any_cast<SparseArray<Component>&>(
          reg.m_components_arrays[type_idx]);
      arr.erase(e);
    });

    return std::any_cast<SparseArray<Component>&>(
        m_components_arrays[type_idx]);
  }

  template <class Component>
  SparseArray<Component>& get_components() {
    std::type_index type_idx(typeid(Component));
    return std::any_cast<SparseArray<Component>&>(
        m_components_arrays[type_idx]);
  }

  template <class Component>
  const SparseArray<Component>& get_components() const {
    std::type_index type_idx(typeid(Component));
    return std::any_cast<const SparseArray<Component>&>(
        m_components_arrays.at(type_idx));
  }

  Entity spawn_entity() {
    Entity e(m_next_entity_id++);
    m_entities.push_back(e);
    return e;
  }

  Entity entity_from_index(size_t idx) const { return Entity(idx); }

  void kill_entity(const Entity& e) {
    for (auto& erase_fn : m_erase_functions) {
      erase_fn(*this, e);
    }

    m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(),
                                    [&e](const Entity& entity) {
                                      return entity.id() == e.id();
                                    }),
                     m_entities.end());
  }

  template <typename Component>
  typename SparseArray<Component>::reference_type add_component(
      const Entity& to, Component&& c) {
    auto& arr = get_components<Component>();
    return arr.insert_at(to, std::forward<Component>(c));
  }

  template <typename Component, typename... Params>
  typename SparseArray<Component>::reference_type emplace_component(
      const Entity& to, Params&&... params) {
    auto& arr = get_components<Component>();
    return arr.emplace_at(to, std::forward<Params>(params)...);
  }

  template <typename Component>
  void remove_component(const Entity& from) {
    auto& arr = get_components<Component>();
    arr.erase(from);
  }

  template <class... Components, typename Function>
  void add_system(Function&& f) {
    m_systems.emplace_back([f = std::forward<Function>(f)](Registry& reg) {
      f(reg, reg.get_components<Components>()...);
    });
  }

  void run_systems() {
    for (auto& system : m_systems) {
      system(*this);
    }
  }

  const std::vector<Entity>& get_entities() const { return m_entities; }

 private:
  std::unordered_map<std::type_index, std::any> m_components_arrays;
  std::vector<std::function<void(Registry&, const Entity&)>> m_erase_functions;
  std::vector<std::function<void(Registry&)>> m_systems;
  std::vector<Entity> m_entities;
  size_t m_next_entity_id = 0;
};
