#pragma once
#include "Game/Core/Render/BakedModel.hpp"

class BakeModelChessBoard : public BakedModel
{
public:
    BakeModelChessBoard();
    ~BakeModelChessBoard() override;

    void Build() override;
};
