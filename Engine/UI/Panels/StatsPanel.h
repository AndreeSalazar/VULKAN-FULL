#pragma once

#include "../UIBase.h"

// ============================================================================
// StatsPanel - Panel de estadísticas del motor
// ============================================================================

namespace UI {

class StatsPanel : public IWindow {
public:
    StatsPanel();
    virtual ~StatsPanel() = default;
    
    virtual void Render() override;
    virtual void Update(float deltaTime) override;
    
    void UpdateStats(float fps, float deltaTime, uint64_t frameCount, float totalTime);

private:
    float currentFPS = 0.0f;
    float deltaTime = 0.0f;
    uint64_t frameCount = 0;
    float totalTime = 0.0f;
};

} // namespace UI

