#include "ChessBoard.hpp"

#include "ChessMatch.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Core/Component/CollisionComponent.hpp"
#include "Game/Core/Component/MeshComponent.hpp"
#include "Game/Core/Render/BakedModel.hpp"

ChessBoard::ChessBoard()
{
    AABB3 box;
    box.SetDimensions(Vec3(8.0f, 8.0f, 0.5f));
    m_collisionComponent->SetCollisionBox(box);
    m_collisionComponent->SetPosition(Vec3(4.f, 4.f, -0.25f));
    m_collisionComponent->SetEnableDebugDraw(true);
    m_meshComponent->Build(BakedModel::GetModelByName("ChessBoard"));

    AABB3 highLightBox;
    highLightBox.SetDimensions(Vec3(1.f, 1.f, 0.5f));
    std::vector<Vertex_PCU> vertices = {};
    vertices.reserve(128);
    AddVertsForCube3DWireFrame(vertices, highLightBox, Rgba8::ORANGE);
    m_squareHighlight = AddComponent<MeshComponent>();
    m_squareHighlight->AppendVertices(vertices);
    g_theEventSystem->SubscribeEventCallbackFunction("event.highlight.enable", Event_Highlight_Enable);
    g_theEventSystem->SubscribeEventCallbackFunction("event.highlight.disable", Event_Highlight_Disable);
}

ChessBoard::~ChessBoard()
{
    // Like CreateOrShader()
    // POINTER_SAFE_DELETE(m_meshComponent->m_shader)
}

void ChessBoard::OnTick(float deltaTime)
{
    UNUSED(deltaTime)
}

Actor* ChessBoard::FromXML(const XmlElement& element)
{
    const XmlElement* chessBoardElement = FindChildElementByName(element, "ChessBoard");
    if (!chessBoardElement) return this;
    Texture* texture                        = g_theRenderer->CreateOrGetTexture(ParseXmlAttribute(*chessBoardElement, "texture", std::string()).c_str());
    Texture* textureNormal                  = g_theRenderer->CreateOrGetTexture(ParseXmlAttribute(*chessBoardElement, "normal", std::string()).c_str());
    Texture* specGlossEmit                  = g_theRenderer->CreateOrGetTexture(ParseXmlAttribute(*chessBoardElement, "specGlossEmit", std::string()).c_str());
    m_meshComponent->m_diffuseTexture       = texture;
    m_meshComponent->m_normalTexture        = textureNormal;
    m_meshComponent->m_specGlossEmitTexture = specGlossEmit;
    m_meshComponent->m_shader               = g_theRenderer->CreateOrGetShader(ParseXmlAttribute(*chessBoardElement, "shader", std::string()).c_str(), VertexType::Vertex_PCUTBN);
    return this;
}

bool ChessBoard::Event_Highlight_Enable(EventArgs& args)
{
    IntVec2     gridPos    = args.GetValue("position", IntVec2::ZERO);
    ChessBoard* chessBoard = g_theGame->match->m_chessBoard;
    if (!chessBoard)
        return false;
    chessBoard->m_squareHighlight->SetEnable(true);
    chessBoard->m_squareHighlight->SetPosition(Vec3((float)gridPos.x + 0.5f, (float)gridPos.y + 0.5f, -0.25f));
    return true;
}

bool ChessBoard::Event_Highlight_Disable(EventArgs& args)
{
    UNUSED(args)
    ChessBoard* chessBoard = g_theGame->match->m_chessBoard;
    if (!chessBoard)
        return false;
    chessBoard->m_squareHighlight->SetEnable(false);
    return true;
}
