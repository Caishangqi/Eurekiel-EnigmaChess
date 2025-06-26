#include "Widget.hpp"

#include "WidgetSubsystem.hpp"
#include "Actor/Actor.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"

Widget::Widget()
{
}

Widget::~Widget()
{
}

void Widget::BeginFrame()
{
}

void Widget::Render()
{
    if (m_owner == nullptr) // means viewport
    {
        Camera* viewportCam = g_theWidgetSubsystem->m_config.m_viewportCamera;
        if (viewportCam == nullptr)
            ERROR_AND_DIE("Widget::Render       App viewportCam is null")
        g_theRenderer->BeginCamera(*viewportCam);
        Draw();
        g_theRenderer->EndCamera(*viewportCam);
    }
    /*else
    {
        Camera* playerViewportCam = m_owner->m_viewCamera;
        if (playerViewportCam == nullptr)
            ERROR_AND_DIE("Widget::Render       Player viewportCam is null")
        g_theRenderer->BeginCamera(*playerViewportCam);
        Draw();
        g_theRenderer->EndCamera(*playerViewportCam);
    }*/
}

void Widget::OnInit()
{
}

void Widget::Draw() const
{
}

void Widget::Update()
{
}

void Widget::EndFrame()
{
}

Actor* Widget::GetOwner()
{
    return m_owner;
}

int Widget::GetZOrder() const
{
    return m_zOrder;
}

std::string Widget::GetName() const
{
    return m_name;
}


void Widget::SetUserFocus()
{
    m_bIsFocused = true;
}

void Widget::AddToViewport(int zOrder)
{
    g_theWidgetSubsystem->AddToViewport(this, zOrder);
}

void Widget::AddToPlayerViewport(Actor* player, int zOrder)
{
    g_theWidgetSubsystem->AddToPlayerViewport(this, player, zOrder);
}

void Widget::RemoveFromViewport()
{
    m_bIsGarbage = true;
}
