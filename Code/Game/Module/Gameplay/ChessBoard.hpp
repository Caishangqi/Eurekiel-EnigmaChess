#pragma once
#include "ChessObject.hpp"
#include "Engine/Core/EventSystem.hpp"

class CollisionComponent;

class ChessBoard final : public ChessObject
{
public:
    ChessBoard();
    ~ChessBoard() override;
    void OnTick(float deltaTime) override;

    Actor* FromXML(const XmlElement& element) override;
    
    static bool Event_Highlight_Enable(EventArgs& args);
    static bool Event_Highlight_Disable(EventArgs& args);

private:
    MeshComponent* m_squareHighlight = nullptr;
};
