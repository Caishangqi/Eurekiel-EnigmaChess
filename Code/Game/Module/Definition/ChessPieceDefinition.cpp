#include "ChessPieceDefinition.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Core/LoggerSubsystem.hpp"
#include "Game/Core/Render/BakedModel.hpp"
std::vector<ChessPieceDefinition> ChessPieceDefinition::s_definitions = {};

void ChessPieceDefinition::LoadDefinitions(const char* path)
{
    LOG(LogResource, Info, "%s", "Loading ChessPieceDefinition");
    XmlDocument chessDefinition;
    XmlResult   result = chessDefinition.LoadFile(path);
    if (result == XmlResult::XML_SUCCESS)
    {
        XmlElement* rootElement = chessDefinition.RootElement();
        if (rootElement)
        {
            LOG(LogResource, Info, "ChessPieceDefinition XML file from \"%s\" was loaded", path);
            const XmlElement* element = rootElement->FirstChildElement();
            while (element != nullptr)
            {
                auto chessPieceDef = ChessPieceDefinition(*element);
                s_definitions.push_back(chessPieceDef);
                LOG(LogResource, Info, "ChessPieceDefinition \"%s\" was loaded", chessPieceDef.m_name.c_str());
                element = element->NextSiblingElement();
            }
        }
        else
        {
            LOG(LogResource, Info, Stringf("ChessPieceDefinition from \"%s\"was invalid (missing root element)\n", path).c_str());
        }
    }
    else
    {
        printf("Failed to load ChessPieceDefinition from \"%s\"\n", path);
    }
}

void ChessPieceDefinition::ClearDefinitions()
{
}

ChessPieceDefinition* ChessPieceDefinition::GetByName(const std::string& name)
{
    for (auto& definition : s_definitions)
    {
        if (Common::ToLower(definition.m_name) == Common::ToLower(name))
            return &definition;
    }
    return nullptr;
}

void ChessPieceDefinition::ReleaseResources()
{
    for (auto& definition : s_definitions)
    {
        POINTER_SAFE_DELETE(definition.m_shader)
    }
}

ChessPieceDefinition::ChessPieceDefinition(const XmlElement& element)
{
    m_name                             = ParseXmlAttribute(element, "name", m_name);
    m_slide                            = ParseXmlAttribute(element, "slide", m_slide);
    m_glyph                            = ParseXmlAttribute(element, "glyph", m_glyph);
    const XmlElement* componentElement = FindChildElementByName(element, "Components");
    if (componentElement)
    {
        if (componentElement->ChildElementCount() > 0)
        {
            /// Handle Animation
            const XmlElement* comp = componentElement->FirstChildElement();
            while (comp != nullptr)
            {
                // TODO: Consider To build a Component factory create component from XML.
                // In that cases, we only store components XML element in a vector, let actor to read.
                std::string compName = ParseXmlAttribute(*comp, "type", compName);
                if (compName == "MeshComponent")
                {
                    m_model                = BakedModel::GetModelByName(ParseXmlAttribute(*comp, "bakeModel", std::string()));
                    m_renderLit            = ParseXmlAttribute(*comp, "renderLit", m_renderLit);
                    m_shader               = g_theRenderer->CreateShaderFromFile(ParseXmlAttribute(*comp, "shader", std::string()).c_str(), VertexType::Vertex_PCUTBN);
                    m_texture              = g_theRenderer->CreateOrGetTextureFromFile(ParseXmlAttribute(*comp, "texture", std::string()).c_str());
                    m_normalTexture        = g_theRenderer->CreateOrGetTextureFromFile(ParseXmlAttribute(*comp, "normal", std::string()).c_str());
                    m_specGlossEmitTexture = g_theRenderer->CreateOrGetTextureFromFile(ParseXmlAttribute(*comp, "specGlossEmit", std::string()).c_str());
                }
                comp = componentElement->NextSiblingElement();
            }
        }
    }
}
