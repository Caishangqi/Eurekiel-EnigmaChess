#pragma once
#include <string>
#include <vector>

class NetworkSubsystem;

class NetworkDispatcher
{
public:
    NetworkDispatcher(NetworkSubsystem* networkSubsystem);
    ~NetworkDispatcher();

    bool ExecuteRemoteCmd();

    // 便利方法
    bool   IsConnectedAsClient() const;
    bool   IsRunningAsServer() const;
    size_t GetConnectedClientCount() const;

private:
    NetworkSubsystem* m_networkSubsystem = nullptr;

    // 消息缓冲：存储不完整的消息
    std::string              m_serverMessageBuffer; // 从服务端接收的不完整消息
    std::vector<std::string> m_clientMessageBuffers; // 从各个客户端接收的不完整消息

    // 消息处理
    bool ProcessServerMessages();
    bool ProcessClientMessages();

    // 消息边界处理
    std::vector<std::string> ExtractCompleteMessages(std::string& buffer, const std::vector<uint8_t>& newData);
    std::vector<std::string> ExtractRawMessages(const std::vector<uint8_t>& data); // 用于RAW_BYTES模式

    void ExecuteCommand(const std::string& command);

    // 根据当前边界模式选择处理方式
    std::vector<std::string> ProcessMessageData(std::string& buffer, const std::vector<uint8_t>& newData);
};
