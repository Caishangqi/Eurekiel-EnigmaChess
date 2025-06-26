#pragma once

enum class EGameState
{
    NONE,
    LOAD_RESOURCE,
    ATTRACT,
    LOBBY,
    MATCH,
    TURN_PLAYER_ONE,
    TURN_PLAYER_TWO
};

inline const char* to_string(EGameState e)
{
    switch (e)
    {
    case EGameState::LOAD_RESOURCE: return "LOAD_RESOURCE";
    case EGameState::ATTRACT: return "ATTRACT";
    case EGameState::LOBBY: return "LOBBY";
    case EGameState::TURN_PLAYER_ONE: return "TURN_PLAYER_ONE";
    case EGameState::TURN_PLAYER_TWO: return "TURN_PLAYER_TWO";
    case EGameState::NONE: return "NONE";
    case EGameState::MATCH: return "MATCH";
    default: return "UNKNOWN";
    }
}
