#pragma once
#include "Player/PlayerEntity.hpp"
#include "physics/Physics2D.hpp"
#include "input/InputSubsystem.hpp"
#include "ecs/SparseArray.hpp"

class NetworkSubsystem;

void player_input_system(Registry& registry,
                         InputSubsystem* inputManager,
                         NetworkSubsystem* networkManager = nullptr);
