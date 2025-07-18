#include "TestModelActor.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Resource/Resource.hpp"
#include "Engine/Resource/Loader/ModelLoader/GlbModelLoader.hpp"
#include "Engine/Resource/Loader/ModelLoader/ObjModelLoader.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Core/Component/MeshComponent.hpp"
#include "Game/Core/Component/CollisionComponent.hpp"

TestModelActor::~TestModelActor()
{
}

void TestModelActor::Initialize()
{
    Actor::Initialize();
    // Add the test Mesh Component for model testing.
    m_meshComponent = AddComponent<MeshComponent>();
    // Add the debug Collision component
    m_collisionComponent = AddComponent<CollisionComponent>();
    AABB3 box;
    box.SetDimensions(Vec3(0.5f, 0.5f, 1.0f));
    m_collisionComponent->SetCollisionBox(box);
    m_collisionComponent->SetPosition(Vec3(0.f, 0.f, 0.5f));
    m_collisionComponent->SetEnableDebugDraw(true);
    m_collisionComponent->SetDebugColor(Rgba8::DEBUG_BLUE);
    // Set the FMesh for the MeshComponent
#define  OBJ_LOADER_TEST
    // ObjModelLoader
#ifdef OBJ_LOADER_TEST
    ObjModelLoader         objLoader(g_theRenderer);
    std::shared_ptr<FMesh> mesh = objLoader.Load(ResourceLocation("enigma", "cube"), "Data\\Models\\Woman\\Woman.obj");
    m_meshComponent->SetMesh(mesh);
    m_meshComponent->m_shader         = g_theRenderer->CreateOrGetShader("Data/Shaders/Diffuse", VertexType::Vertex_PCUTBN);
    m_meshComponent->m_diffuseTexture = g_theRenderer->CreateTextureFromFile(mesh->m_MetaData["textures"]["diffuse"].get<std::string>().c_str());
    m_meshComponent->m_normalTexture  = g_theRenderer->CreateTextureFromFile(mesh->m_MetaData["textures"]["normal"].get<std::string>().c_str());
#endif

#ifdef GLB_LOADER_TEST
    GlbModelLoader glbLoader(g_theRenderer);
    //auto           mesh       = glbLoader.Load(ResourceLocation("enigma", "rook"), "Data\\Models\\Chess.glb");
    auto           mesh       = glbLoader.Load(ResourceLocation("enigma", "rook"), "Data\\Models\\Chess1.gltf");
    m_meshComponent->SetMesh(std::move(mesh));
    m_meshComponent->m_shader = g_theRenderer->CreateOrGetShader("Data/Shaders/Diffuse", VertexType::Vertex_PCUTBN);
    //m_meshComponent->m_diffuseTexture = g_theRenderer->CreateTextureFromFile("Data\\Models\\Woman\\Chess_Default.png");
    m_meshComponent->m_normalTexture  = g_theRenderer->CreateTextureFromFile("Data\\Models\\Woman\\default_normal.png");

#endif
}

void TestModelActor::OnTick(float deltaTime)
{
    Actor::OnTick(deltaTime);
}

XmlElement* TestModelActor::ToXML() const
{
    return Actor::ToXML();
}

Actor* TestModelActor::FromXML(const XmlElement& element)
{
    return Actor::FromXML(element);
}
