#pragma once
#include "Game/Core/Render/BakedModel.hpp"

class BakeModelPawn : public BakedModel
{
public:
    BakeModelPawn();
    ~BakeModelPawn() override;
    void Build() override;
};
