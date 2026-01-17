#pragma once
#include <algorithm>
#include <any>
#include <functional>
#include <iostream>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "ecs/Entity.hpp"
#include "ecs/SparseArray.hpp"

class Registry {
 public:
  template <class Component>
  SparseArray<Component>& register_component() {
    std::type_index type_idx(typeid(Component));

    if (m_components_arrays.find(type_idx) != m_components_arrays.end()) {
      std::cerr << "Warning: Component already registered: "
                << typeid(Component).name() << std::endl;
      return std::any_cast<SparseArray<Component>&>(
          m_components_arrays[type_idx]);
    }

    m_components_arrays[type_idx] = SparseArray<Component>();

    m_erase_functions.emplace_back([type_idx](Registry& reg, const Entity& e) {
      try {
        auto& arr = std::any_cast<SparseArray<Component>&>(
            reg.m_components_arrays[type_idx]);
        arr.erase(e);
      } catch (const std::exception& ex) {
        std::cerr << "Error erasing component: " << ex.what() << std::endl;
      }
    });

    std::cout << "Registered component: " << typeid(Component).name()
              << std::endl;

    return std::any_cast<SparseArray<Component>&>(
        m_components_arrays[type_idx]);
  }

  template <class Component>
  SparseArray<Component>& get_components() {
    std::type_index type_idx(typeid(Component));

    auto it = m_components_arrays.find(type_idx);
    if (it == m_components_arrays.end()) {
      std::cerr << "ERROR: Component not registered: "
                << typeid(Component).name() << std::endl;
      std::cerr << "Did you forget to call register_component<"
                << typeid(Component).name() << ">()?" << std::endl;
      throw std::runtime_error("Component not registered");
    }

    try {
      return std::any_cast<SparseArray<Component>&>(it->second);
    } catch (const std::bad_any_cast& e) {
      std::cerr << "ERROR: Bad component cast for: " << typeid(Component).name()
                << std::endl;
      throw;
    }
  }

  template <class Component>
  const SparseArray<Component>& get_components() const {
    std::type_index type_idx(typeid(Component));

    auto it = m_components_arrays.find(type_idx);
    if (it == m_components_arrays.end()) {
      std::cerr << "ERROR: Component not registered: "
                << typeid(Component).name() << std::endl;
      throw std::runtime_error("Component not registered");
    }

    try {
      return std::any_cast<const SparseArray<Component>&>(it->second);
    } catch (const std::bad_any_cast& e) {
      std::cerr << "ERROR: Bad component cast for: " << typeid(Component).name()
                << std::endl;
      throw;
    }
  }

  Entity spawn_entity() {
    Entity e(m_next_entity_id++);
    m_entities.push_back(e);
    m_valid_entities.insert(e);


    return e;
  }

  Entity entity_from_index(size_t idx) const { return Entity(idx); }

  bool is_entity_valid(const Entity& e) const {
    return m_valid_entities.find(e) != m_valid_entities.end();
  }

  void kill_entity(const Entity& e) {
    if (!is_entity_valid(e)) {
      std::cerr << "Warning: Trying to kill invalid entity: "
                << static_cast<size_t>(e) << std::endl;
      return;
    }


    for (auto& erase_fn : m_erase_functions) {
      try {
        erase_fn(*this, e);
      } catch (const std::exception& ex) {
        std::cerr << "Error in erase function: " << ex.what() << std::endl;
      }
    }

    m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(),
                                    [&e](const Entity& entity) {
                                      return entity.id() == e.id();
                                    }),
                     m_entities.end());

    m_valid_entities.erase(e);
  }

  template <typename Component>
  typename SparseArray<Component>::reference_type add_component(
      const Entity& to, Component&& c) {
    if (!is_entity_valid(to)) {
      std::cerr << "ERROR: Trying to add component to invalid entity: "
                << static_cast<size_t>(to) << std::endl;
      throw std::runtime_error("Invalid entity");
    }

    try {
      auto& arr = get_components<Component>();
      auto& result = arr.insert_at(to, std::forward<Component>(c));

      // std::cout << "Added component " << typeid(Component).name()
      //           << " to entity " << static_cast<size_t>(to) << std::endl;

      return result;
    } catch (const std::exception& e) {
      std::cerr << "ERROR adding component: " << e.what() << std::endl;
      throw;
    }
  }

  template <typename Component, typename... Params>
  typename SparseArray<Component>::reference_type emplace_component(
      const Entity& to, Params&&... params) {
    if (!is_entity_valid(to)) {
      std::cerr << "ERROR: Trying to emplace component to invalid entity: "
                << static_cast<size_t>(to) << std::endl;
      throw std::runtime_error("Invalid entity");
    }

    try {
      auto& arr = get_components<Component>();
      auto& result = arr.emplace_at(to, std::forward<Params>(params)...);

      // std::cout << "Emplaced component " << typeid(Component).name()
      //           << " to entity " << static_cast<size_t>(to) << std::endl;

      return result;
    } catch (const std::exception& e) {
      std::cerr << "ERROR emplacing component: " << e.what() << std::endl;
      throw;
    }
  }

  template <typename Component>
  void remove_component(const Entity& from) {
    if (!is_entity_valid(from)) {
      std::cerr << "Warning: Trying to remove component from invalid entity: "
                << static_cast<size_t>(from) << std::endl;
      return;
    }

    try {
      auto& arr = get_components<Component>();
      arr.erase(from);

      std::cout << "Removed component " << typeid(Component).name()
                << " from entity " << static_cast<size_t>(from) << std::endl;
    } catch (const std::exception& e) {
      std::cerr << "ERROR removing component: " << e.what() << std::endl;
    }
  }

  template <class... Components, typename Function>
  void add_system(Function&& f) {
    m_systems.emplace_back([f = std::forward<Function>(f)](Registry& reg) {
      try {
        f(reg, reg.get_components<Components>()...);
      } catch (const std::exception& e) {
        std::cerr << "ERROR in system: " << e.what() << std::endl;
      }
    });
  }

  void run_systems() {
    for (auto& system : m_systems) {
      try {
        system(*this);
      } catch (const std::exception& e) {
        std::cerr << "ERROR running system: " << e.what() << std::endl;
      }
    }
  }

  const std::vector<Entity>& get_entities() const { return m_entities; }

  size_t entity_count() const { return m_valid_entities.size(); }

  void clear_all_entities() {
    std::cout << "Clearing all entities (" << m_entities.size() << " entities)"
              << std::endl;

    // Create a copy to avoid iterator invalidation
    std::vector<Entity> entities_to_kill = m_entities;

    for (const auto& entity : entities_to_kill) {
      kill_entity(entity);
    }

    m_entities.clear();
    m_valid_entities.clear();

    std::cout << "All entities cleared" << std::endl;
  }
  template <typename Component>
  bool has_component(const Entity& e) const {
    std::type_index type_idx(typeid(Component));
    auto it = m_components_arrays.find(type_idx);
    if (it == m_components_arrays.end()) {
      return false;  // le composant n'est même pas enregistré
    }

    try {
      const auto& arr =
          std::any_cast<const SparseArray<Component>&>(it->second);
      return arr[e].has_value();  // renvoie vrai si l'entité a ce composant
    } catch (const std::bad_any_cast& ex) {
      std::cerr << "ERROR: Bad component cast for has_component: "
                << typeid(Component).name() << std::endl;
      return false;
    }
  }

  void print_debug_info() const {
    std::cout << "\n=== Registry Debug Info ===" << std::endl;
    std::cout << "Total entities: " << m_entities.size() << std::endl;
    std::cout << "Valid entities: " << m_valid_entities.size() << std::endl;
    std::cout << "Registered component types: " << m_components_arrays.size()
              << std::endl;
    std::cout << "Registered systems: " << m_systems.size() << std::endl;
    std::cout << "=========================\n" << std::endl;
  }

 private:
  std::unordered_map<std::type_index, std::any> m_components_arrays;
  std::vector<std::function<void(Registry&, const Entity&)>> m_erase_functions;
  std::vector<std::function<void(Registry&)>> m_systems;
  std::vector<Entity> m_entities;
  size_t m_next_entity_id = 0;
  std::unordered_set<size_t> m_valid_entities;
};
