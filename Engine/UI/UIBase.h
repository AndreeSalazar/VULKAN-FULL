#pragma once

#include <string>
#include <functional>
#include <vector>
#include <memory>

// ============================================================================
// UIBase - Sistema de interfaz gráfica base
// Base para paneles, ventanas y widgets
// ============================================================================

namespace UI {
    
// Estados de widgets
enum class EWidgetState {
    Normal,
    Hovered,
    Pressed,
    Disabled
};

// Rectángulo para layout
struct FRect {
    float x, y, width, height;
    
    FRect() : x(0), y(0), width(0), height(0) {}
    FRect(float _x, float _y, float _w, float _h) : x(_x), y(_y), width(_w), height(_h) {}
    
    bool Contains(float px, float py) const {
        return px >= x && px <= x + width && py >= y && py <= y + height;
    }
};

// Color (RGBA)
struct FColor {
    float r, g, b, a;
    
    FColor() : r(0), g(0), b(0), a(1.0f) {}
    FColor(float _r, float _g, float _b, float _a = 1.0f) : r(_r), g(_g), b(_b), a(_a) {}
    
    static FColor White() { return FColor(1.0f, 1.0f, 1.0f, 1.0f); }
    static FColor Black() { return FColor(0.0f, 0.0f, 0.0f, 1.0f); }
    static FColor Red() { return FColor(1.0f, 0.0f, 0.0f, 1.0f); }
    static FColor Green() { return FColor(0.0f, 1.0f, 0.0f, 1.0f); }
    static FColor Blue() { return FColor(0.0f, 0.0f, 1.0f, 1.0f); }
};

// Clase base para widgets
class IWidget {
public:
    virtual ~IWidget() = default;
    
    virtual void Render() = 0;
    virtual void Update(float deltaTime) {}
    
    // Layout
    virtual FRect GetBounds() const { return bounds; }
    virtual void SetBounds(const FRect& newBounds) { bounds = newBounds; }
    
    // Visibility
    virtual bool IsVisible() const { return bVisible; }
    virtual void SetVisible(bool visible) { bVisible = visible; }
    
    // Enabled/Disabled
    virtual bool IsEnabled() const { return bEnabled; }
    virtual void SetEnabled(bool enabled) { bEnabled = enabled; }

protected:
    FRect bounds;
    bool bVisible = true;
    bool bEnabled = true;
    EWidgetState state = EWidgetState::Normal;
};

// Panel base (contiene widgets)
class IPanel : public IWidget {
public:
    virtual ~IPanel() = default;
    
    virtual void AddWidget(std::shared_ptr<IWidget> widget);
    virtual void RemoveWidget(std::shared_ptr<IWidget> widget);
    virtual void ClearWidgets();
    
    virtual void Render() override;
    virtual void Update(float deltaTime) override;

protected:
    std::vector<std::shared_ptr<IWidget>> widgets;
    std::string panelName;
};

// Window base (ventana con título y contenido)
class IWindow : public IPanel {
public:
    virtual ~IWindow() = default;
    
    virtual void SetTitle(const std::string& title) { windowTitle = title; }
    virtual const std::string& GetTitle() const { return windowTitle; }
    
    virtual void SetResizable(bool resizable) { bResizable = resizable; }
    virtual bool IsResizable() const { return bResizable; }
    
    virtual void SetMovable(bool movable) { bMovable = movable; }
    virtual bool IsMovable() const { return bMovable; }

protected:
    std::string windowTitle;
    bool bResizable = true;
    bool bMovable = true;
    bool bMinimized = false;
};

} // namespace UI

