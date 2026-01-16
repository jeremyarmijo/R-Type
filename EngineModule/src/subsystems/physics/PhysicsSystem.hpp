#pragma once
#include "engine/ISubsystem.hpp"
#include "ecs/Registry.hpp"
#include "physics/Physics2D.hpp"
#include <functional>

class PhysicsSubsystem : public ISubsystem {
private:
    Registry* m_registry;
    Vector2 m_gravity;
    float m_timeAccumulator;
    float m_fixedTimeStep;

    std::vector<std::function<void(const CollisionEvent&)>> m_collisionCallbacks;

    class MessagingSubsystem* m_messagingSubsystem;
    
public:
    PhysicsSubsystem();
    ~PhysicsSubsystem() override = default;
    
    bool Initialize() override;
    void Shutdown() override;
    void Update(float deltaTime) override;
    
    const char* GetName() const override { return "Physics"; }
    SubsystemType GetType() const override { return SubsystemType::PHYSICS; }
    const char* GetVersion() const override { return "1.0.0"; }
    
    void SetMessagingSubsystem(MessagingSubsystem* messaging) { 
        m_messagingSubsystem = messaging; 
    }
    void ProcessEvent(SDL_Event event) override {}

    void SetRegistry(Registry* registry) override;
    void SetGravity(Vector2 gravity) { m_gravity = gravity; }
    Vector2 GetGravity() const { return m_gravity; }
    void SetFixedTimeStep(float step) { m_fixedTimeStep = step; }

    void RegisterCollisionCallback(std::function<void(const CollisionEvent&)> callback) {
        m_collisionCallbacks.push_back(callback);
    }

private:
    void FixedUpdate(float fixedDeltaTime);
    void UpdatePhysics(float deltaTime);
    void CheckCollisions();
    bool CheckAABBCollision(const BoxCollider::Bounds& a, const BoxCollider::Bounds& b);
    void ResolveCollision(Entity a, Entity b, const BoxCollider::Bounds& boundsA, 
                         const BoxCollider::Bounds& boundsB);
    bool ShouldCollide(uint32_t layerA, uint32_t maskA, uint32_t layerB, uint32_t maskB);
};

extern "C" {
    ISubsystem* CreateSubsystem();
}
