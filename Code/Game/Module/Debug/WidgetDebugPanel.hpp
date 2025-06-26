#pragma once
#include <vector>

#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Core/Widget.hpp"

class WidgetDebugPanel : public Widget
{
public:
    WidgetDebugPanel();

public:
    void OnInit() override;
    void Draw() const override;
    void Update() override;

private:
    std::vector<Vertex_PCU> m_cursorVertex;
};
