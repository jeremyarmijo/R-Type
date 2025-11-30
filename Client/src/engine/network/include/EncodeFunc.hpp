#include <cstring>
#include <vector>

#include "Encode.hpp"

void Player_Up(const Action& a, std::vector<uint8_t>& out,
               uint32_t sequenceNum) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;

  out.resize(11);
  size_t offset = 0;

  memcpy(out.data() + offset, &sequenceNum, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  uint32_t tick = input->tick;
  memcpy(out.data() + offset, &tick, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  int8_t mx = 0;
  memcpy(out.data() + offset, &mx, sizeof(int8_t));
  offset += sizeof(int8_t);

  int8_t my = -1;
  memcpy(out.data() + offset, &my, sizeof(int8_t));
  offset += sizeof(int8_t);

  uint8_t actions = 0;
  memcpy(out.data() + offset, &actions, sizeof(uint8_t));
}

void Player_Down(const Action& a, std::vector<uint8_t>& out,
                 uint32_t sequenceNum) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;

  out.resize(11);
  size_t offset = 0;

  memcpy(out.data() + offset, &sequenceNum, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  uint32_t tick = input->tick;
  memcpy(out.data() + offset, &tick, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  int8_t mx = 0;
  memcpy(out.data() + offset, &mx, sizeof(int8_t));
  offset += sizeof(int8_t);

  int8_t my = 1;  // bas
  memcpy(out.data() + offset, &my, sizeof(int8_t));
  offset += sizeof(int8_t);

  uint8_t actions = 0;
  memcpy(out.data() + offset, &actions, sizeof(uint8_t));
}

void Player_Left(const Action& a, std::vector<uint8_t>& out,
                 uint32_t sequenceNum) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;

  out.resize(11);
  size_t offset = 0;

  memcpy(out.data() + offset, &sequenceNum, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  uint32_t tick = input->tick;
  memcpy(out.data() + offset, &tick, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  int8_t mx = -1;  // gauche
  memcpy(out.data() + offset, &mx, sizeof(int8_t));
  offset += sizeof(int8_t);

  int8_t my = 0;
  memcpy(out.data() + offset, &my, sizeof(int8_t));
  offset += sizeof(int8_t);

  uint8_t actions = 0;
  memcpy(out.data() + offset, &actions, sizeof(uint8_t));
}

void Player_Right(const Action& a, std::vector<uint8_t>& out,
                  uint32_t sequenceNum) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;

  out.resize(11);
  size_t offset = 0;

  memcpy(out.data() + offset, &sequenceNum, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  uint32_t tick = input->tick;
  memcpy(out.data() + offset, &tick, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  int8_t mx = 1;  // droite
  memcpy(out.data() + offset, &mx, sizeof(int8_t));
  offset += sizeof(int8_t);

  int8_t my = 0;
  memcpy(out.data() + offset, &my, sizeof(int8_t));
  offset += sizeof(int8_t);

  uint8_t actions = 0;
  memcpy(out.data() + offset, &actions, sizeof(uint8_t));
}

void Player_Shoot(const Action& a, std::vector<uint8_t>& out,
                  uint32_t sequenceNum) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;

  out.resize(11);
  size_t offset = 0;

  memcpy(out.data() + offset, &sequenceNum, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  uint32_t tick = input->tick;
  memcpy(out.data() + offset, &tick, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  int8_t mx = 0;
  memcpy(out.data() + offset, &mx, sizeof(int8_t));
  offset += sizeof(int8_t);

  int8_t my = 0;
  memcpy(out.data() + offset, &my, sizeof(int8_t));
  offset += sizeof(int8_t);

  uint8_t actions = 0x01;  // Tir primaire
  memcpy(out.data() + offset, &actions, sizeof(uint8_t));
}

void Player_ForceAttach(const Action& a, std::vector<uint8_t>& out,
                        uint32_t sequenceNum) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;

  out.resize(11);
  size_t offset = 0;

  memcpy(out.data() + offset, &sequenceNum, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  uint32_t tick = input->tick;
  memcpy(out.data() + offset, &tick, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  int8_t mx = 0;
  memcpy(out.data() + offset, &mx, sizeof(int8_t));
  offset += sizeof(int8_t);

  int8_t my = 0;
  memcpy(out.data() + offset, &my, sizeof(int8_t));
  offset += sizeof(int8_t);

  uint8_t actions = 0x08;  // Rappeler la Force
  memcpy(out.data() + offset, &actions, sizeof(uint8_t));
}

void Player_ForceDetach(const Action& a, std::vector<uint8_t>& out,
                        uint32_t sequenceNum) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;

  out.resize(11);
  size_t offset = 0;

  memcpy(out.data() + offset, &sequenceNum, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  uint32_t tick = input->tick;
  memcpy(out.data() + offset, &tick, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  int8_t mx = 0;
  memcpy(out.data() + offset, &mx, sizeof(int8_t));
  offset += sizeof(int8_t);

  int8_t my = 0;
  memcpy(out.data() + offset, &my, sizeof(int8_t));
  offset += sizeof(int8_t);

  uint8_t actions = 0x04;
  memcpy(out.data() + offset, &actions, sizeof(uint8_t));
}

void Player_UsePowerup(const Action& a, std::vector<uint8_t>& out,
                       uint32_t sequenceNum) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;

  out.resize(11);
  size_t offset = 0;

  memcpy(out.data() + offset, &sequenceNum, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  uint32_t tick = input->tick;
  memcpy(out.data() + offset, &tick, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  int8_t mx = 0;
  memcpy(out.data() + offset, &mx, sizeof(int8_t));
  offset += sizeof(int8_t);

  int8_t my = 0;
  memcpy(out.data() + offset, &my, sizeof(int8_t));
  offset += sizeof(int8_t);

  uint8_t actions = 0x02;
  memcpy(out.data() + offset, &actions, sizeof(uint8_t));
}

void SetupEncoder(Encoder& encoder) {
  encoder.registerHandler(ActionType::UP, Player_Up);
  encoder.registerHandler(ActionType::DOWN, Player_Down);
  encoder.registerHandler(ActionType::LEFT, Player_Left);
  encoder.registerHandler(ActionType::RIGHT, Player_Right);
  encoder.registerHandler(ActionType::SHOOT, Player_Shoot);
  encoder.registerHandler(ActionType::FORCE_ATTACH, Player_ForceAttach);
  encoder.registerHandler(ActionType::FORCE_DETACH, Player_ForceDetach);
  encoder.registerHandler(ActionType::USE_POWERUP, Player_UsePowerup);
}
