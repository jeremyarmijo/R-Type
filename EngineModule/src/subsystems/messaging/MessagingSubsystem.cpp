#include "messaging/MessagingSubsystem.hpp"

#include <iostream>
#include <string>
#include <utility>

MessagingSubsystem::MessagingSubsystem() : m_messagesProcessedPerFrame(100) {}

bool MessagingSubsystem::Initialize() {
  std::cout << "Initializing Messaging Subsystem..." << std::endl;
  std::cout << "Messages per frame: " << m_messagesProcessedPerFrame
            << std::endl;
  return true;
}

void MessagingSubsystem::Shutdown() {
  std::cout << "Shutting down Messaging Subsystem..." << std::endl;
  m_subscribers.clear();

  // Clear remaining messages
  while (!m_messageQueue.empty()) {
    m_messageQueue.pop();
  }
}

void MessagingSubsystem::Update(float deltaTime) { ProcessMessages(); }

void MessagingSubsystem::PostMessage(const Message& message) {
  if (message.synchronous) {
    DispatchMessage(message);
  } else {
    m_messageQueue.push(message);
  }
}

void MessagingSubsystem::PostMessage(const std::string& type, std::any data,
                                     MessagePriority priority) {
  m_messageQueue.push(Message(type, std::move(data), priority, false));
}

void MessagingSubsystem::SendSyncMessage(const std::string& type,
                                         std::any data) {
  DispatchMessage(
      Message(type, std::move(data), MessagePriority::CRITICAL, true));
}

void MessagingSubsystem::Subscribe(const std::string& messageType,
                                   MessageCallback callback) {
  m_subscribers[messageType].push_back(callback);
  std::cout << "Subscribed to message type: " << messageType << std::endl;
}

void MessagingSubsystem::Unsubscribe(const std::string& messageType) {
  m_subscribers.erase(messageType);
}

void MessagingSubsystem::ProcessMessages() {
  int processed = 0;

  while (!m_messageQueue.empty() && processed < m_messagesProcessedPerFrame) {
    Message message = m_messageQueue.top();
    m_messageQueue.pop();

    DispatchMessage(message);
    ++processed;
  }
}

void MessagingSubsystem::DispatchMessage(const Message& message) {
  auto it = m_subscribers.find(message.type);
  if (it != m_subscribers.end()) {
    for (const auto& callback : it->second) {
      try {
        callback(message);
      } catch (const std::exception& e) {
        std::cerr << "Error dispatching message '" << message.type
                  << "': " << e.what() << std::endl;
      }
    }
  }
}
#ifdef _WIN32
__declspec(dllexport) ISubsystem* CreateSubsystem() {
    return new MessagingSubsystem();
}
#else
extern "C" {
ISubsystem* CreateSubsystem() { return new MessagingSubsystem(); }
}
#endif