#pragma once

enum class ECameraState
{
    NONE = -1,
    PER_PLAYER = 0,
    CONFIGURED = 1
};

enum class ECameraMode
{
    NONE = -1,
    AUTO = 0,
    FREE = 1
};

inline const char* to_string(ECameraMode e)
{
    switch (e)
    {
    case ECameraMode::NONE: return "NONE";
    case ECameraMode::AUTO: return "AUTO";
    case ECameraMode::FREE: return "FREE";
    }
    return "UNKNOWN";
}
