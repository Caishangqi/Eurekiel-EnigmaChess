#include "WidgetSubsystem.hpp"

#include "LoggerSubsystem.hpp"
#include "Widget.hpp"
#include "Game/GameCommon.hpp"

bool DescendingZOrderPtr::operator()(const Widget* lhs, const Widget* rhs) const
{
    // If two pointers point to the same object, they are considered equivalent
    if (lhs == rhs)
    {
        return false;
    }
    // If the zorder is the same, you need to define whether they are considered equal in the set.
    // Here, in order to meet the strict weak ordering (Strict Weak Ordering), when the zorder is the same,
    // We compare the pointers themselves to ensure that equal elements are not considered unequal in the set.
    if (lhs->GetZOrder() == rhs->GetZOrder())
    {
        return lhs < rhs;
    }
    // Larger zorders are placed on the "left" (i.e. keys that are considered smaller in the collection)
    return lhs->GetZOrder() > rhs->GetZOrder();
}

WidgetSubsystem::WidgetSubsystem(WidgetSystemConfig config) : m_config(config)
{
}

WidgetSubsystem::~WidgetSubsystem()
{
    for (Widget* widget : m_widgets)
    {
        delete widget;
        widget = nullptr;
    }
}

void WidgetSubsystem::BeginFrame()
{
    for (int i = 0; i < static_cast<int>(m_widgets.size()); ++i)
    {
        if (m_widgets[i] && m_widgets[i]->m_bIsGarbage)
        {
            delete m_widgets[i];
            m_widgets[i] = nullptr;
        }
    }

    for (Widget* widget : m_widgets)
    {
        if (widget)
            widget->BeginFrame();
    }
}

void WidgetSubsystem::Startup()
{
    g_theLoggerSubsystem->EnableCategory(ELogCategory::LogWidget);
    LOG(LogWidget, Info, "Start up Widget subsystem...");
}

void WidgetSubsystem::Shutdown()
{
    LOG(LogWidget, Info, "Shutdown Widget subsystem...");
    for (Widget* widget : m_widgets)
    {
        if (widget)
        {
            delete widget;
            widget = nullptr;
        }
        else
        {
            widget = nullptr;
        }
    }
    m_widgets.clear();
}

void WidgetSubsystem::Update()
{
    for (int i = 0; i < static_cast<int>(m_widgets.size()); ++i)
    {
        if (m_widgets[i] && !m_widgets[i]->m_bIsGarbage)
        {
            m_widgets[i]->Update();
        }
    }
}

void WidgetSubsystem::Render()
{
    for (Widget* widget : m_widgets)
    {
        if (widget && !widget->m_bIsGarbage && widget->GetIsVisible())
            widget->Render();
    }
}

void WidgetSubsystem::HotReload()
{
    for (Widget* widget : m_widgets)
    {
        delete widget;
        widget = nullptr;
    }
    m_widgets.clear();
}

void WidgetSubsystem::EndFrame()
{
    for (Widget* widget : m_widgets)
    {
        if (widget)
            widget->EndFrame();
    }
}

std::vector<Widget*> WidgetSubsystem::GetViewportWidgets(std::string widgetName) const
{
    std::vector<Widget*> widgets;
    widgets.reserve(static_cast<int>(m_widgets.size()));
    for (Widget* widget : m_widgets)
    {
        if (widget && !widget->m_bIsGarbage)
        {
            if (widget->GetName() == widgetName)
                widgets.emplace_back(widget);
        }
    }
    return widgets;
}

std::vector<Widget*> WidgetSubsystem::GetViewportWidgets() const
{
    return m_widgets;
}

void WidgetSubsystem::AddToViewport(Widget* widget, int zOrder)
{
    widget->m_zOrder = zOrder;
    m_widgets.push_back(widget);
    widget->OnInit();
    LOG(LogWidget, Info, "Add widget [ %s ] to Viewport", widget->GetName().c_str());
}

void WidgetSubsystem::AddToPlayerViewport(Widget* widget, Actor* player, int zOrder)
{
    widget->m_zOrder = zOrder;
    widget->m_owner  = player;
    m_widgets.push_back(widget);
}

void WidgetSubsystem::RemoveFromViewport(Widget* widget)
{
    for (Widget* m_widget : m_widgets)
    {
        if (m_widget == widget)
        {
            m_widget->RemoveFromViewport();
        }
    }
}

void WidgetSubsystem::RemoveFromViewport(std::string widgetName)
{
    for (Widget* widget : m_widgets)
    {
        if (widget && widget->m_name == widgetName)
        {
            widget->RemoveFromViewport();
        }
    }
}

void WidgetSubsystem::RemoveFromPlayerViewport(Actor* player, std::string widgetName)
{
    for (Widget* widget : m_widgets)
    {
        if (widget && widget->m_name == widgetName && widget->m_owner == player)
        {
            widget->RemoveFromViewport();
        }
    }
}
