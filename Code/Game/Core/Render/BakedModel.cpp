#include "BakedModel.hpp"

#include "Engine/Core/StringUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Core/LoggerSubsystem.hpp"
#include "Game/Module/Model/BakedModelBishop.hpp"
#include "Game/Module/Model/BakedModelKnight.hpp"
#include "Game/Module/Model/BakeModelChessBoard.hpp"
#include "Game/Module/Model/BakeModelKing.hpp"
#include "Game/Module/Model/BakeModelPawn.hpp"
#include "Game/Module/Model/BakeModelQueen.hpp"
#include "Game/Module/Model/BakeModelRook.hpp"
std::vector<BakedModel*> BakedModel::s_models = {};

BakedModel::BakedModel()
{
}

BakedModel::~BakedModel()
{
}

void BakedModel::Build()
{
}

BakedModel* BakedModel::GetModelByName(std::string name)
{
    for (BakedModel* model : s_models)
    {
        if (model->name == name)
            return model;
    }
    return nullptr;
}

void BakedModel::RegisterModels()
{
    RegisterModel(new BakeModelQueen());
    RegisterModel(new BakeModelKing());
    RegisterModel(new BakeModelRook());
    RegisterModel(new BakedModelBishop());
    RegisterModel(new BakedModelKnight());
    RegisterModel(new BakeModelPawn());
    RegisterModel(new BakeModelChessBoard());
}

BakedModel* BakedModel::RegisterModel(BakedModel* model)
{
    LOG(LogResource, Info, ((Stringf("Register baked model with name: %s",model->name.c_str())).c_str()));
    s_models.push_back(model);
    model->Build();
    return model;
}

void BakedModel::ReleaseResources()
{
    for (BakedModel* model : s_models)
    {
        delete model;
        model = nullptr;
    }
}

BakedModel& BakedModel::SetVerticesByID(std::vector<Vertex_PCUTBN>& verts, int id)
{
    m_vertices[id] = verts;
    return *this;
}

BakedModel& BakedModel::SetIndicesByID(std::vector<unsigned int>& indices, int id)
{
    m_indices[id] = indices;
    return *this;
}

std::vector<Vertex_PCUTBN>& BakedModel::GetVerticesByID(int id)
{
    return m_vertices[id];
}

std::vector<unsigned int>& BakedModel::GetIndicesByID(int id)
{
    return m_indices[id];
}
