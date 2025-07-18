﻿#pragma once
#include "Component.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Resource/Loader/ModelLoader/ModelLoader.hpp"
#include "Game/Core/Render/Renderable.hpp"

struct FMesh;
class BakedModel;
class IndexBuffer;
class VertexBuffer;
class Shader;
class Texture;

class MeshComponent final : public IComponent, public IRenderable
{
public:
    COMPONENT_CLASS(MeshComponent)

    MeshComponent();
    ~MeshComponent() override;


    void        OnInit() override;
    void        OnAttach(Actor& owner) override;
    void        OnDetach() override;
    void        OnDestroy() override;
    void        Tick(float deltaTime) override;
    IComponent* FromXML(const XmlElement& xmlElement) override;
    XmlElement* ToXML() const override;

    void Render(const RenderContext& ctx) override;

    /// Vertices Manipulation
    MeshComponent* AppendVertices(std::vector<Vertex_PCUTBN> vertices, std::vector<unsigned int>& indices);
    MeshComponent* AppendVertices(std::vector<Vertex_PCU> vertices, std::vector<unsigned int>& indices);
    MeshComponent* AppendVertices(std::vector<Vertex_PCU> vertices);
    MeshComponent* AppendVertices(std::vector<Vertex_PCUTBN> vertices);
    MeshComponent* AppendIndices(std::vector<unsigned int>& indices);
    MeshComponent* Build(BakedModel* model); // Build Vertices and Indices from BakedModel
    MeshComponent* SetModel(BakedModel* model);
    MeshComponent* SetMesh(std::shared_ptr<FMesh> mesh);
    void           UploadIfDirty();

    std::vector<Vertex_PCUTBN> m_vertexesPCUTBN;
    std::vector<Vertex_PCU>    m_vertexesPCU;
    std::vector<unsigned int>  m_indices;
    Texture*                   m_diffuseTexture       = nullptr;
    Texture*                   m_normalTexture        = nullptr;
    Texture*                   m_specGlossEmitTexture = nullptr;
    Shader*                    m_shader               = nullptr;
    VertexBuffer*              m_vertexBufferPCUTBN   = nullptr;
    VertexBuffer*              m_vertexBufferPCU      = nullptr;
    IndexBuffer*               m_indexBuffer          = nullptr;
    Rgba8                      m_color                = Rgba8::WHITE;
    BakedModel*                m_model                = nullptr;

    // Material Base
    MeshComponent* ApplyMaterial(const FMaterial& material);
    MeshComponent* ApplyMaterialByIndex(size_t materialIndex);

    // Material Texture
    const FMaterial* GetCurrentMaterial() const;
    size_t           GetMaterialCount() const;

    // Material Texture Settings
    MeshComponent* SetDiffuseTexture(std::unique_ptr<Texture> texture);
    MeshComponent* SetNormalTexture(std::unique_ptr<Texture> texture);
    MeshComponent* SetSpecularTexture(std::unique_ptr<Texture> texture);

    bool  IsMaterialValid(size_t materialIndex) const;

protected:
    void CreateBuffers();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CopyBufferData();
    void EnsureCapacity();

private:
    bool m_dirty = false;


    std::shared_ptr<FMesh> m_mesh = nullptr;

    size_t m_currentMaterialIndex = 0;
    // If Mesh component want their own texture copy
    std::unique_ptr<Texture> m_ownedDiffuseTexture  = nullptr;
    std::unique_ptr<Texture> m_ownedNormalTexture   = nullptr;
    std::unique_ptr<Texture> m_ownedSpecularTexture = nullptr;
};
