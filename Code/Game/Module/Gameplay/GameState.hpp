#pragma once

enum class EGameState
{
    NONE,
    LOAD_RESOURCE,
    ATTRACT,
    LOBBY,
    MATCH,
    SETTLEMENT
};

inline const char* to_string(EGameState e)
{
    switch (e)
    {
    case EGameState::LOAD_RESOURCE: return "LOAD_RESOURCE";
    case EGameState::ATTRACT: return "ATTRACT";
    case EGameState::LOBBY: return "LOBBY";
    case EGameState::NONE: return "NONE";
    case EGameState::MATCH: return "MATCH";
    case EGameState::SETTLEMENT: return "SETTLEMENT";
    default: return "UNKNOWN";
    }
}
