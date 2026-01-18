#pragma once
#include <any>
#include <functional>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "engine/ISubsystem.hpp"

enum class MessagePriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};

struct Message {
    std::string type;
    std::any data;
    MessagePriority priority;
    bool synchronous;
    
    Message(const std::string& t, std::any d = {}, 
            MessagePriority p = MessagePriority::NORMAL, bool sync = false)
        : type(t), data(std::move(d)), priority(p), synchronous(sync) {}
};

struct MessageComparator {
    bool operator()(const Message& a, const Message& b) const {
        return static_cast<int>(a.priority) < static_cast<int>(b.priority);
    }
};

using MessageCallback = std::function<void(const Message&)>;

class MessagingSubsystem : public ISubsystem {
private:
    std::priority_queue<Message, std::vector<Message>, MessageComparator> m_messageQueue;
    std::unordered_map<std::string, std::vector<MessageCallback>> m_subscribers;
    
    int m_messagesProcessedPerFrame;

public:
    MessagingSubsystem();
    ~MessagingSubsystem() override = default;
    
    bool Initialize() override;
    void Shutdown() override;
    void Update(float deltaTime) override;
    void SetRegistry(Registry* registry) override {}
    void ProcessEvent(SDL_Event event) override {}
    
    const char* GetName() const override { return "Messaging"; }
    SubsystemType GetType() const override { return SubsystemType::MESSAGING; }
    const char* GetVersion() const override { return "1.0.0"; }
    
    // Messaging API
    void PostMessage(const Message& message);
    void PostMessage(const std::string& type, std::any data = {}, 
                    MessagePriority priority = MessagePriority::NORMAL);
    void SendSyncMessage(const std::string& type, std::any data = {});
    
    void Subscribe(const std::string& messageType, MessageCallback callback);
    void Unsubscribe(const std::string& messageType);
    
    void SetMessagesPerFrame(int count) { m_messagesProcessedPerFrame = count; }

private:
    void ProcessMessages();
    void DispatchMessage(const Message& message);
};

extern "C" {
    ISubsystem* CreateSubsystem();
    void DestroySubsystem(ISubsystem* subsystem);
}
