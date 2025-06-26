#pragma once
#include "Game/Core/Render/BakedModel.hpp"

class BakeModelQueen : public BakedModel
{
public:
    BakeModelQueen();
    ~BakeModelQueen() override;
    void Build() override;
};
