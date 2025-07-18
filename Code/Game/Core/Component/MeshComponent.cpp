﻿#include "MeshComponent.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Core/LoggerSubsystem.hpp"
#include "Game/Core/Actor/Actor.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Game/Core/Render/BakedModel.hpp"
#include "Game/Core/Render/RenderContext.hpp"
#include "Game/Core/Render/RenderSubsystem.hpp"

MeshComponent::MeshComponent()
{
    m_vertexesPCUTBN.reserve(1024);
    m_vertexesPCU.reserve(1024);
    m_indices.reserve(1024);
}

MeshComponent::~MeshComponent()
{
    POINTER_SAFE_DELETE(m_vertexBufferPCUTBN)
    POINTER_SAFE_DELETE(m_indexBuffer)
    POINTER_SAFE_DELETE(m_vertexBufferPCU)
}

void MeshComponent::OnInit()
{
    IComponent::OnInit();
    if (!m_vertexesPCUTBN.empty() || !m_vertexesPCU.empty())
        CreateBuffers();
    else
        CopyBufferData();
}

void MeshComponent::OnAttach(Actor& owner)
{
    IComponent::OnAttach(owner);
    g_theRenderSubsystem->Register(this);
}

void MeshComponent::OnDetach()
{
    IComponent::OnDetach();
    g_theRenderSubsystem->Unregister(this);
}

void MeshComponent::OnDestroy()
{
    IComponent::OnDestroy();
    g_theRenderSubsystem->Unregister(this);
}

void MeshComponent::Tick(float deltaTime)
{
    IComponent::Tick(deltaTime);
}

IComponent* MeshComponent::FromXML(const XmlElement& xmlElement)
{
    UNUSED(xmlElement)
    return this;
}

XmlElement* MeshComponent::ToXML() const
{
    return nullptr;
}

void MeshComponent::Render(const RenderContext& ctx)
{
    UploadIfDirty();
    if (m_vertexesPCUTBN.empty() && m_vertexesPCU.empty()) return;

    if (IsMaterialValid(0))
    {
        m_mesh->EnsureGPUBuffers(&ctx.renderer);
        m_diffuseTexture = m_mesh->GetMaterial(0)->GetTexture(EMaterialChannel::Specular);
        m_color          = Rgba8(m_mesh->GetMaterial(0)->baseColorFactor);
        ctx.renderer.BindTexture(m_diffuseTexture, 0);
    }

    Mat44 matrix;
    matrix = GetOwner()->GetModelToWorldTransform();
    matrix.AppendTranslation3D(m_position);
    ctx.SetModel(matrix, m_color); // TODO: Consider to Move into Transform Component (additional matrix mul)
    ctx.renderer.BindTexture(m_diffuseTexture, 0);
    ctx.renderer.BindTexture(m_normalTexture, 1);
    ctx.renderer.BindTexture(m_specGlossEmitTexture, 2);
    ctx.renderer.BindShader(m_shader);


    ctx.renderer.SetLightConstants(ctx.lightingConstants);
    if (!m_vertexesPCUTBN.empty() && m_indexBuffer != nullptr)
        ctx.renderer.DrawVertexIndexed(m_vertexBufferPCUTBN, m_indexBuffer, static_cast<int>(m_indices.size()));
    if (!m_vertexesPCUTBN.empty() && m_indexBuffer == nullptr)
        ctx.renderer.DrawVertexBuffer(m_vertexBufferPCUTBN, static_cast<int>(m_vertexesPCUTBN.size()));
    if (!m_vertexesPCU.empty())
        ctx.renderer.DrawVertexBuffer(m_vertexBufferPCU, static_cast<int>(m_vertexesPCU.size()));
}

MeshComponent* MeshComponent::AppendVertices(std::vector<Vertex_PCUTBN> vertices, std::vector<unsigned int>& indices)
{
    m_vertexesPCUTBN.insert(m_vertexesPCUTBN.end(), vertices.begin(), vertices.end());
    m_indices.insert(m_indices.end(), indices.begin(), indices.end());
    m_dirty = true;
    return this;
}

MeshComponent* MeshComponent::AppendVertices(std::vector<Vertex_PCU> vertices, std::vector<unsigned>& indices)
{
    m_vertexesPCU.insert(m_vertexesPCU.end(), vertices.begin(), vertices.end());
    m_indices.insert(m_indices.end(), indices.begin(), indices.end());
    m_dirty = true;
    return this;
}

MeshComponent* MeshComponent::AppendVertices(std::vector<Vertex_PCU> vertices)
{
    m_vertexesPCU.insert(m_vertexesPCU.end(), vertices.begin(), vertices.end());
    m_dirty = true;
    return this;
}

MeshComponent* MeshComponent::AppendVertices(std::vector<Vertex_PCUTBN> vertices)
{
    m_vertexesPCUTBN.insert(m_vertexesPCUTBN.end(), vertices.begin(), vertices.end());
    m_dirty = true;
    return this;
}


MeshComponent* MeshComponent::AppendIndices(std::vector<unsigned int>& indices)
{
    m_indices.insert(m_indices.end(), indices.begin(), indices.end());
    m_dirty = true;
    return this;
}

MeshComponent* MeshComponent::Build(BakedModel* model)
{
    std::vector<Vertex_PCUTBN> ver = model->GetVerticesByID();
    std::vector<unsigned int>  ind = model->GetIndicesByID();
    AppendVertices(ver, ind);
    return this;
}

MeshComponent* MeshComponent::SetModel(BakedModel* model)
{
    m_model = model;
    m_dirty = true;
    m_vertexesPCUTBN.clear();
    m_indices.clear();
    Build(model);
    return this;
}

MeshComponent* MeshComponent::SetMesh(std::shared_ptr<FMesh> mesh)
{
    m_mesh = mesh;
    m_vertexesPCUTBN.clear();
    m_indices.clear();
    AppendVertices(mesh.get()->m_vertices);
    AppendIndices(mesh.get()->m_indices);
    return this;
}


void MeshComponent::UploadIfDirty()
{
    if (m_dirty)
    {
        EnsureCapacity();
        CopyBufferData();
        m_dirty = false;
    }
}

MeshComponent* MeshComponent::ApplyMaterial(const FMaterial& material)
{
    if (material.HasTexture(EMaterialChannel::Albedo)) m_diffuseTexture = material.GetTexture(EMaterialChannel::Albedo);

    if (material.HasTexture(EMaterialChannel::Normal)) m_normalTexture = material.GetTexture(EMaterialChannel::Normal);

    if (material.HasTexture(EMaterialChannel::MetallicRoughness)) m_specGlossEmitTexture = material.GetTexture(EMaterialChannel::MetallicRoughness);

    if (material.HasTexture(EMaterialChannel::Occlusion))
    {
    }

    if (material.HasTexture(EMaterialChannel::Emission))
    {
    }

    // 应用材质参数
    m_color = Rgba8(
        static_cast<unsigned char>(RangeMapClamped(material.baseColorFactor.x, 0.0f, 1.0f, 0.f, 255.f)),
        static_cast<unsigned char>(RangeMapClamped(material.baseColorFactor.y, 0.0f, 1.0f, 0.f, 255.f)),
        static_cast<unsigned char>(RangeMapClamped(material.baseColorFactor.z, 0.0f, 1.0f, 0.f, 255.f)),
        static_cast<unsigned char>(RangeMapClamped(material.baseColorFactor.w, 0.0f, 1.0f, 0.f, 255.f))
    );
    return this;
}

MeshComponent* MeshComponent::ApplyMaterialByIndex(size_t materialIndex)
{
    if (!m_mesh || materialIndex >= m_mesh->GetMaterialCount())
    {
        LOG(LogResource, Warning, Stringf("Invalid material index: %d (available: %d)",
                materialIndex, m_mesh ? m_mesh->GetMaterialCount() : 0).c_str());
        return this;
    }

    const FMaterial* material = m_mesh->GetMaterial(materialIndex);
    if (material)
    {
        m_currentMaterialIndex = materialIndex;
        ApplyMaterial(*material);

        LOG(LogResource, Info, Stringf("Switched to material %d: %s",
                materialIndex, material->name.c_str()).c_str());
    }

    return this;
}

const FMaterial* MeshComponent::GetCurrentMaterial() const
{
    if (!m_mesh || m_currentMaterialIndex >= m_mesh->GetMaterialCount())
    {
        return nullptr;
    }
    return m_mesh->GetMaterial(m_currentMaterialIndex);
}

size_t MeshComponent::GetMaterialCount() const
{
    return m_mesh ? m_mesh->GetMaterialCount() : 0;
}

MeshComponent* MeshComponent::SetDiffuseTexture(std::unique_ptr<Texture> texture)
{
    m_ownedDiffuseTexture = std::move(texture);
    m_diffuseTexture      = m_ownedDiffuseTexture.get();
    return this;
}

MeshComponent* MeshComponent::SetNormalTexture(std::unique_ptr<Texture> texture)
{
    m_ownedNormalTexture = std::move(texture);
    m_normalTexture      = m_ownedNormalTexture.get();
    return this;
}

MeshComponent* MeshComponent::SetSpecularTexture(std::unique_ptr<Texture> texture)
{
    m_ownedSpecularTexture = std::move(texture);
    m_specGlossEmitTexture = m_ownedSpecularTexture.get();
    return this;
}

bool MeshComponent::IsMaterialValid(size_t materialIndex) const
{
    if (!m_mesh || materialIndex >= m_mesh->GetMaterialCount())
    {
        return false;
    }

    const FMaterial* material = m_mesh->GetMaterial(materialIndex);
    if (!material)
    {
        return false;
    }

    if (!material->HasTexture(EMaterialChannel::Albedo))
    {
        //LOG(LogResource, Warning, Stringf("Material '%s' has no albedo texture", material->name.c_str()).c_str());
    }

    return true;
}

void MeshComponent::CreateBuffers()
{
    CreateVertexBuffer();
    CreateIndexBuffer();
}

void MeshComponent::CreateVertexBuffer()
{
    m_vertexBufferPCUTBN = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
    m_vertexBufferPCU    = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), sizeof(Vertex_PCU));
}

void MeshComponent::CreateIndexBuffer()
{
    if (m_indices.empty()) return;
    m_indexBuffer = g_theRenderer->CreateIndexBuffer(sizeof(unsigned int));
    m_indexBuffer->Resize(static_cast<int>(m_indices.size()) * sizeof(unsigned int));
}

void MeshComponent::CopyBufferData()
{
    if (m_vertexBufferPCU)
        g_theRenderer->CopyCPUToGPU(m_vertexesPCU.data(), static_cast<int>(m_vertexesPCU.size()) * sizeof(Vertex_PCU), m_vertexBufferPCU);
    if (m_vertexBufferPCUTBN)
        g_theRenderer->CopyCPUToGPU(m_vertexesPCUTBN.data(), static_cast<int>(m_vertexesPCUTBN.size()) * sizeof(Vertex_PCUTBN), m_vertexBufferPCUTBN);
    if (m_indexBuffer)
        g_theRenderer->CopyCPUToGPU(m_indices.data(), static_cast<int>(m_indices.size()) * sizeof(unsigned int), m_indexBuffer);
}

void MeshComponent::EnsureCapacity()
{
    const unsigned int newVBSize    = static_cast<unsigned int>(m_vertexesPCUTBN.size() * sizeof(Vertex_PCUTBN));
    const unsigned int newVBTBNSize = static_cast<unsigned int>(m_vertexesPCU.size() * sizeof(Vertex_PCU));
    const unsigned int newIBSize    = static_cast<unsigned int>(m_indices.size() * sizeof(unsigned int));

    if (!m_vertexBufferPCUTBN || !m_vertexBufferPCU)
    {
        CreateBuffers();
        return;
    }

    if (newVBSize > m_vertexBufferPCUTBN->GetSize() || newVBTBNSize > m_vertexBufferPCU->GetSize() || newIBSize > m_indexBuffer->GetSize())
    {
        if (newVBSize != 0)
            m_vertexBufferPCUTBN->Resize(newVBSize);
        if (newVBTBNSize != 0)
            m_vertexBufferPCU->Resize(newVBTBNSize);
        if (newIBSize != 0)
            m_indexBuffer->Resize(newIBSize);
    }
}
