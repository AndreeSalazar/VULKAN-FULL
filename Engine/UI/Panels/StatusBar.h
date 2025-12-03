#pragma once

#include "UIBase.h"

namespace UI {

class StatusBar : public IPanel {
public:
    StatusBar();
    virtual ~StatusBar() = default;
    
    virtual void Render() override;
    virtual void Update(float deltaTime) override;
    
    void SetStatus(const char* status);
    void SetFPS(float fps);
    void SetFrameTime(float frameTime);
    
private:
    std::string currentStatus;
    float currentFPS;
    float currentFrameTime;
};

} // namespace UI

