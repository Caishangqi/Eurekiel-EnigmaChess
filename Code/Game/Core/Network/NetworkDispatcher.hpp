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

    bool   IsConnectedAsClient() const;
    bool   IsRunningAsServer() const;
    size_t GetConnectedClientCount() const;

private:
    NetworkSubsystem* m_networkSubsystem = nullptr;

    // Message buffer: store incomplete messages
    std::string              m_serverMessageBuffer; // Incomplete message received from the server
    std::vector<std::string> m_clientMessageBuffers; // Incomplete messages received from various clients

    // Message processing
    bool ProcessServerMessages();
    bool ProcessClientMessages();

    // Message boundary processing
    std::vector<std::string> ExtractCompleteMessages(std::string& buffer, const std::vector<uint8_t>& newData);
    std::vector<std::string> ExtractRawMessages(const std::vector<uint8_t>& data); // for RAW_BYTES mode

    void ExecuteCommand(const std::string& command);

    // Select the processing method according to the current boundary mode
    std::vector<std::string> ProcessMessageData(std::string& buffer, const std::vector<uint8_t>& newData);
};
