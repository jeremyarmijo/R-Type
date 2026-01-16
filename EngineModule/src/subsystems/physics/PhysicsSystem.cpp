#include "physics/PhysicsSystem.hpp"
#include <iostream>

PhysicsSubsystem::PhysicsSubsystem()
    : m_registry(nullptr),
      m_gravity{0, 0},
      m_timeAccumulator(0.0f),
      m_fixedTimeStep(1.0f / 60.0f) {}

bool PhysicsSubsystem::Initialize() {
    std::cout << "Initializing Physics Subsystem..." << std::endl;
    std::cout << "Fixed timestep: " << m_fixedTimeStep << "s" << std::endl;
    std::cout << "Gravity: (" << m_gravity.x << ", " << m_gravity.y << ")" << std::endl;
    return true;
}

void PhysicsSubsystem::Shutdown() {
    std::cout << "Shutting down Physics Subsystem..." << std::endl;
}

void PhysicsSubsystem::Update(float deltaTime) {
    if (!m_registry) return;

    m_timeAccumulator += deltaTime;
    
    while (m_timeAccumulator >= m_fixedTimeStep) {
        FixedUpdate(m_fixedTimeStep);
        m_timeAccumulator -= m_fixedTimeStep;
    }
}

void PhysicsSubsystem::FixedUpdate(float fixedDeltaTime) {
    UpdatePhysics(fixedDeltaTime);
    CheckCollisions();
}

void PhysicsSubsystem::UpdatePhysics(float deltaTime) {
    auto& transforms = m_registry->get_components<Transform>();
    auto& rigidbodies = m_registry->get_components<RigidBody>();
    
    for (size_t i = 0; i < std::min(transforms.size(), rigidbodies.size()); ++i) {
        if (!transforms[i].has_value() || !rigidbodies[i].has_value()) {
            continue;
        }
        
        auto& transform = transforms[i].value();
        auto& rigidbody = rigidbodies[i].value();
        
        if (rigidbody.isStatic) {
            continue;
        }

        rigidbody.acceleration += m_gravity;
        rigidbody.velocity += rigidbody.acceleration * deltaTime;
        transform.position += rigidbody.velocity * deltaTime;
        rigidbody.acceleration = {0, 0};
    }
}

void PhysicsSubsystem::CheckCollisions() {
    auto& transforms = m_registry->get_components<Transform>();
    auto& colliders = m_registry->get_components<BoxCollider>();
    auto& rigidbodies = m_registry->get_components<RigidBody>();
    
    size_t maxSize = std::min({transforms.size(), colliders.size()});
    
    for (size_t i = 0; i < maxSize; ++i) {
        if (!transforms[i].has_value() || !colliders[i].has_value()) {
            continue;
        }
        
        Entity entityA(i);
        auto& transformA = transforms[i].value();
        auto& colliderA = colliders[i].value();
        auto boundsA = colliderA.GetBounds(transformA.position);
        
        for (size_t j = i + 1; j < maxSize; ++j) {
            if (!transforms[j].has_value() || !colliders[j].has_value()) {
                continue;
            }
            
            Entity entityB(j);
            auto& transformB = transforms[j].value();
            auto& colliderB = colliders[j].value();
            auto boundsB = colliderB.GetBounds(transformB.position);
            
            // Check collision layers
            if (!ShouldCollide(colliderA.layer, colliderA.mask, 
                             colliderB.layer, colliderB.mask)) {
                continue;
            }
            
            // AABB collision test
            if (CheckAABBCollision(boundsA, boundsB)) {
                // Create collision event
                CollisionEvent event;
                event.entityA = entityA;
                event.entityB = entityB;
                event.point = {
                    (boundsA.left + boundsA.right + boundsB.left + boundsB.right) / 4.0f,
                    (boundsA.top + boundsA.bottom + boundsB.top + boundsB.bottom) / 4.0f
                };
                event.normal = Vector2{
                    transformB.position.x - transformA.position.x,
                    transformB.position.y - transformA.position.y
                }.Normalized();
                
                // Send collision message via messaging system
                // if (m_messagingSubsystem) {
                //     m_messagingSubsystem->PostMessage(
                //         "collision",
                //         event,
                //         MessagePriority::HIGH
                //     );
                // }

                for (auto& callback : m_collisionCallbacks) {
                    callback(event);
                }

                if (!colliderA.isTrigger && !colliderB.isTrigger) {
                    ResolveCollision(entityA, entityB, boundsA, boundsB);
                }
            }
        }
    }
}

bool PhysicsSubsystem::ShouldCollide(uint32_t layerA, uint32_t maskA, 
                                     uint32_t layerB, uint32_t maskB) {
    return (layerA & maskB) && (layerB & maskA);
}

void PhysicsSubsystem::ResolveCollision(Entity a, Entity b, 
                                       const BoxCollider::Bounds& boundsA,
                                       const BoxCollider::Bounds& boundsB) {
    auto& rigidbodies = m_registry->get_components<RigidBody>();
    
    if (a >= rigidbodies.size() || b >= rigidbodies.size()) return;
    if (!rigidbodies[a].has_value() || !rigidbodies[b].has_value()) return;

    auto& rbA = rigidbodies[a].value();
    auto& rbB = rigidbodies[b].value();

    if (!rbA.isStatic && !rbB.isStatic) {
        Vector2 temp = rbA.velocity;
        rbA.velocity = rbB.velocity * rbA.restitution;
        rbB.velocity = temp * rbB.restitution;
    }
}

bool PhysicsSubsystem::CheckAABBCollision(const BoxCollider::Bounds& a, 
                                         const BoxCollider::Bounds& b) {
    return a.left < b.right &&
           a.right > b.left &&
           a.top < b.bottom &&
           a.bottom > b.top;
}

void PhysicsSubsystem::SetRegistry(Registry* registry) {
    m_registry = registry;
    m_registry->register_component<Transform>();
    m_registry->register_component<RigidBody>();
    m_registry->register_component<BoxCollider>();
}

extern "C" {
    ISubsystem* CreateSubsystem() {
        return new PhysicsSubsystem();
    }
}
