#pragma once
#include "Game/Core/Render/BakedModel.hpp"

class BakeModelKing : public BakedModel
{
public:
    BakeModelKing();
    ~BakeModelKing() override;
    void Build() override;
};
