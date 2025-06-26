#pragma once
#include <string>
#include <vector>

#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/Texture.hpp"

class BakedModel;
class Shader;

class ChessPieceDefinition
{
public:
    static std::vector<ChessPieceDefinition> s_definitions;
    static void                              LoadDefinitions(const char* path);
    static void                              ClearDefinitions();
    static ChessPieceDefinition*             GetByName(const std::string& name);
    static void                              ReleaseResources();

    ChessPieceDefinition(const XmlElement& element);
    std::string m_name  = "Unknown";
    std::string m_glyph = "?";
    /// Mesh Component
    BakedModel* m_model                = nullptr;
    bool        m_slide                = false;
    bool        m_renderLit            = true;
    Shader*     m_shader               = nullptr;
    Texture*    m_normalTexture        = nullptr;
    Texture*    m_specGlossEmitTexture = nullptr;
    Texture*    m_texture              = nullptr;
};
