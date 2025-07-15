#include "NetworkDispatcher.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Network/NetworkSubsystem.hpp"
#include "Game/GameCommon.hpp"


NetworkDispatcher::NetworkDispatcher(NetworkSubsystem* networkSubsystem)
    : m_networkSubsystem(networkSubsystem)
{
    // Preallocate client message buffer
    m_clientMessageBuffers.resize(20); // 支持更多客户端
}

NetworkDispatcher::~NetworkDispatcher()
{
}

bool NetworkDispatcher::ExecuteRemoteCmd()
{
    bool processedAnyMessage = false;

    // Process the message received from the server
    if (ProcessServerMessages())
    {
        processedAnyMessage = true;
    }

    // Process the message received from the client
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

    // Get new data
    std::vector<uint8_t> serverData = m_networkSubsystem->ReceiveFromServer();

    // Process data according to the message boundary mode
    std::vector<std::string> completeMessages = ProcessMessageData(m_serverMessageBuffer, serverData);

    // Execute the complete message
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

    // Make sure the client buffer is large enough
    if (m_clientMessageBuffers.size() < clientCount)
    {
        m_clientMessageBuffers.resize(clientCount);
    }

    for (size_t clientIndex = 0; clientIndex < clientCount; ++clientIndex)
    {
        if (m_networkSubsystem->HasDataFromClient(clientIndex))
        {
            // Get the new data of the client
            std::vector<uint8_t> clientData = m_networkSubsystem->ReceiveFromClient(clientIndex);

            // Process data according to the message boundary mode
            std::vector<std::string> completeMessages = ProcessMessageData(
                m_clientMessageBuffers[clientIndex], clientData);

            // Execute the complete message
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
        // RAW mode: each received data is treated as a complete message
        buffer.clear(); // Clear the buffer because no accumulation is needed
        return ExtractRawMessages(newData);

    case MessageBoundaryMode::LENGTH_PREFIXED:
        // TODO: length prefix mode
        // Currently falls back to NULL_TERMINATED
        return ExtractCompleteMessages(buffer, newData);
    default:
        return ExtractCompleteMessages(buffer, newData);
    }
}

std::vector<std::string> NetworkDispatcher::ExtractCompleteMessages(
    std::string& buffer, const std::vector<uint8_t>& newData)
{
    std::vector<std::string> completeMessages;

    // Add new data to the buffer
    for (uint8_t byte : newData)
    {
        buffer.push_back(static_cast<char>(byte));
    }

    // Get the current message separator
    char delimiter = m_networkSubsystem->GetConfig().messageDelimiter;

    // Find the complete message (ending with the delimiter)
    size_t startPos     = 0;
    size_t delimiterPos = 0;

    while ((delimiterPos = buffer.find(delimiter, startPos)) != std::string::npos)
    {
        // Find a complete message
        std::string completeMessage = buffer.substr(startPos, delimiterPos - startPos);

        if (!completeMessage.empty())
        {
            completeMessages.push_back(completeMessage);
        }

        startPos = delimiterPos + 1; // Skip the delimiter
    }

    // Remove the processed messages and keep the unfinished ones
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
        // Remove null characters (for compatibility)
        std::vector<uint8_t> cleanedData;
        for (uint8_t byte : data)
        {
            if (byte != 0) // Remove the \0 character
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
    // Add remote=true flag and execute
    std::string commandWithRemote = command + " remote=true";
    g_theDevConsole->Execute(commandWithRemote);

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
