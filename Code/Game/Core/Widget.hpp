#pragma once
#include <string>

class Actor;
class ChessPlayer;

class Widget
{
    friend class WidgetSubsystem;

public:
    Widget();
    virtual ~Widget();

    virtual void BeginFrame();
    virtual void Render();
    virtual void OnInit();
    virtual void Draw() const;
    virtual void Update();
    virtual void EndFrame();

    /// Getter
    virtual Actor*      GetOwner();
    virtual int         GetZOrder() const;
    virtual std::string GetName() const;
    virtual bool        GetIsVisible() const { return m_bIsVisible; }
    /// Setter
    virtual void SetUserFocus();

    virtual bool SetVisibility(bool bIsVisible)
    {
        m_bIsVisible = bIsVisible;
        return m_bIsVisible;
    }

    virtual void AddToViewport(int zOrder = 0);
    virtual void AddToPlayerViewport(Actor* player, int zOrder = 0);
    virtual void RemoveFromViewport();

protected:
    Actor*      m_owner      = nullptr; // If player controller is null it basic means that it is the viewport widget.
    int         m_zOrder     = 0;
    bool        m_bIsTick    = true;
    std::string m_name       = "Untitled";
    bool        m_bIsVisible = true;
    bool        m_bIsGarbage = false;
    bool        m_bIsFocused = false;
};
