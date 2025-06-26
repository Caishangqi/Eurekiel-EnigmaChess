#pragma once
#include "Game/Core/Render/BakedModel.hpp"

class BakedModelBishop : public BakedModel
{
public:
    BakedModelBishop();
    ~BakedModelBishop() override;
    void Build() override;
};
