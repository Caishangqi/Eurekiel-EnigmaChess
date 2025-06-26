#pragma once
#include "Game/Core/Render/BakedModel.hpp"

class BakeModelRook : public BakedModel
{
public:
    BakeModelRook();
    ~BakeModelRook() override;
    void Build() override;
};
