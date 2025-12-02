-- Example UI Panel Script in Lua
-- This demonstrates how to create UI panels using Lua

local ExamplePanel = {}

-- Panel state
local fps = 0.0
local frameCount = 0
local showWindow = true

-- Initialize panel
function ExamplePanel.OnInit()
    print("ExamplePanel initialized from Lua!")
    showWindow = true
end

-- Update panel (called every frame)
function ExamplePanel.OnUpdate(deltaTime)
    -- Update data from engine
    fps = Engine:GetFPS()
    frameCount = Engine:GetFrameCount()
end

-- Render panel (called every frame)
function ExamplePanel.OnRender()
    if not showWindow then
        return
    end
    
    -- Begin window
    if UI:Begin("Example Panel (Lua)", showWindow) then
        -- Display FPS
        UI:Text("FPS: %.1f", fps)
        UI:Text("Frame Count: %d", frameCount)
        
        UI:Separator()
        
        -- Button example
        if UI:Button("Click Me!") then
            print("Button clicked from Lua!")
            Engine:ShowMessage("Hello from Lua!")
        end
        
        UI:Separator()
        
        -- Input example
        local text = "Type here..."
        if UI:InputText("Input", text) then
            print("Text changed: " .. text)
        end
        
        -- Checkbox example
        local enabled = true
        if UI:Checkbox("Enable Feature", enabled) then
            print("Feature enabled: " .. tostring(enabled))
        end
    end
    UI:End()
end

-- Cleanup
function ExamplePanel.OnShutdown()
    print("ExamplePanel shutting down from Lua!")
end

return ExamplePanel

