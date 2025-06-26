#pragma once
#include "Game/Core/Render/BakedModel.hpp"

class BakedModelKnight : public BakedModel
{
public:
    BakedModelKnight();
    ~BakedModelKnight() override;
    void Build() override;
};
