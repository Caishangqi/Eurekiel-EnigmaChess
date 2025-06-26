#pragma once
#include <string>
#include <vector>

#include "Engine/Core/Vertex_PCU.hpp"

/// Internal Baked Model that will discard in the future? I guess.
class BakedModel
{
public:
    BakedModel();
    virtual ~BakedModel();

    BakedModel& SetVerticesByID(std::vector<Vertex_PCUTBN>& verts, int id = 0);
    BakedModel& SetIndicesByID(std::vector<unsigned int>& indices, int id = 0);

    std::vector<Vertex_PCUTBN>& GetVerticesByID(int id = 0);
    std::vector<unsigned int>&  GetIndicesByID(int id = 0);

    virtual void Build();

    /// Query
    static BakedModel* GetModelByName(std::string name);
    static void        RegisterModels();
    static BakedModel* RegisterModel(BakedModel* model);
    static void        ReleaseResources();

    std::string                     name;
    static std::vector<BakedModel*> s_models;

protected:
    int defaultGroup = 0;
    int group        = 0; // Group ID

    std::vector<Vertex_PCUTBN> m_vertices[10];
    std::vector<unsigned int>  m_indices[10];
};
