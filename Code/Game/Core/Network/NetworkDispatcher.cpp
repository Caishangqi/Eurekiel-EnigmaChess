#include "NetworkDispatcher.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Network/NetworkSubsystem.hpp"
#include "Game/GameCommon.hpp"


NetworkDispatcher::NetworkDispatcher(NetworkSubsystem* networkSubsystem)
    : m_networkSubsystem(networkSubsystem)
{
    // 预分配客户端消息缓冲区
    m_clientMessageBuffers.resize(20); // 支持更多客户端
}

NetworkDispatcher::~NetworkDispatcher()
{
}

bool NetworkDispatcher::ExecuteRemoteCmd()
{
    bool processedAnyMessage = false;

    // 处理从服务端接收的消息
    if (ProcessServerMessages())
    {
        processedAnyMessage = true;
    }

    // 处理从客户端接收的消息
    if (ProcessClientMessages())
    {
        processedAnyMessage = true;
    }

    return processedAnyMessage;
}

bool NetworkDispatcher::ProcessServerMessages()
{
    if (!m_networkSubsystem->HasDataFromServer())
    {
        return false;
    }

    // 获取新数据
    std::vector<uint8_t> serverData = m_networkSubsystem->ReceiveFromServer();

    // 根据消息边界模式处理数据
    std::vector<std::string> completeMessages = ProcessMessageData(m_serverMessageBuffer, serverData);

    // 执行完整的消息
    for (const std::string& message : completeMessages)
    {
        if (!message.empty())
        {
            ExecuteCommand(message);
        }
    }

    return !completeMessages.empty();
}

bool NetworkDispatcher::ProcessClientMessages()
{
    if (m_networkSubsystem->GetServerState() != ServerState::LISTENING)
    {
        return false;
    }

    bool   processedAny = false;
    size_t clientCount  = m_networkSubsystem->GetConnectedClientCount();

    // 确保客户端缓冲区足够大
    if (m_clientMessageBuffers.size() < clientCount)
    {
        m_clientMessageBuffers.resize(clientCount);
    }

    for (size_t clientIndex = 0; clientIndex < clientCount; ++clientIndex)
    {
        if (m_networkSubsystem->HasDataFromClient(clientIndex))
        {
            // 获取该客户端的新数据
            std::vector<uint8_t> clientData = m_networkSubsystem->ReceiveFromClient(clientIndex);

            // 根据消息边界模式处理数据
            std::vector<std::string> completeMessages = ProcessMessageData(
                m_clientMessageBuffers[clientIndex], clientData);

            // 执行完整的消息
            for (const std::string& message : completeMessages)
            {
                if (!message.empty())
                {
                    ExecuteCommand(message);
                    processedAny = true;
                }
            }
        }
    }

    return processedAny;
}

std::vector<std::string> NetworkDispatcher::ProcessMessageData(std::string& buffer, const std::vector<uint8_t>& newData)
{
    MessageBoundaryMode mode = m_networkSubsystem->GetMessageBoundaryMode();

    switch (mode)
    {
    case MessageBoundaryMode::NULL_TERMINATED:
        return ExtractCompleteMessages(buffer, newData);

    case MessageBoundaryMode::RAW_BYTES:
        // RAW模式：每次接收到的数据都当作一个完整消息
        buffer.clear(); // 清空缓冲区，因为不需要累积
        return ExtractRawMessages(newData);

    case MessageBoundaryMode::LENGTH_PREFIXED:
        // 未来实现：长度前缀模式
        // 目前回退到NULL_TERMINATED
        return ExtractCompleteMessages(buffer, newData);

    default:
        return ExtractCompleteMessages(buffer, newData);
    }
}

std::vector<std::string> NetworkDispatcher::ExtractCompleteMessages(
    std::string& buffer, const std::vector<uint8_t>& newData)
{
    std::vector<std::string> completeMessages;

    // 将新数据添加到缓冲区
    for (uint8_t byte : newData)
    {
        buffer.push_back(static_cast<char>(byte));
    }

    // 获取当前的消息分隔符
    char delimiter = m_networkSubsystem->GetConfig().messageDelimiter;

    // 查找完整的消息（以分隔符结尾）
    size_t startPos     = 0;
    size_t delimiterPos = 0;

    while ((delimiterPos = buffer.find(delimiter, startPos)) != std::string::npos)
    {
        // 找到一个完整的消息
        std::string completeMessage = buffer.substr(startPos, delimiterPos - startPos);

        if (!completeMessage.empty())
        {
            completeMessages.push_back(completeMessage);
        }

        startPos = delimiterPos + 1; // 跳过分隔符
    }

    // 移除已处理的消息，保留未完成的部分
    if (startPos > 0)
    {
        buffer = buffer.substr(startPos);
    }

    return completeMessages;
}

std::vector<std::string> NetworkDispatcher::ExtractRawMessages(const std::vector<uint8_t>& data)
{
    std::vector<std::string> messages;

    if (!data.empty())
    {
        // 移除空字符（为了兼容性）
        std::vector<uint8_t> cleanedData;
        for (uint8_t byte : data)
        {
            if (byte != 0) // 移除 \0 字符
            {
                cleanedData.push_back(byte);
            }
        }

        if (!cleanedData.empty())
        {
            std::string message(cleanedData.begin(), cleanedData.end());
            messages.push_back(message);
        }
    }

    return messages;
}

void NetworkDispatcher::ExecuteCommand(const std::string& command)
{
    // 添加 remote=true 标记并执行
    std::string commandWithRemote = command + " remote=true";
    g_theDevConsole->Execute(commandWithRemote);

    // 可选：添加调试日志
#ifdef NETWORK_DEBUG
    std::cout << "[NetworkDispatcher] Executing: " << command << std::endl;
#endif
}

bool NetworkDispatcher::IsConnectedAsClient() const
{
    return m_networkSubsystem->GetClientState() == ClientState::CONNECTED;
}

bool NetworkDispatcher::IsRunningAsServer() const
{
    return m_networkSubsystem->GetServerState() == ServerState::LISTENING;
}

size_t NetworkDispatcher::GetConnectedClientCount() const
{
    return m_networkSubsystem->GetConnectedClientCount();
}
