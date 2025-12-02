#pragma once

#include <string>
#include <memory>
#include <functional>

// Forward declaration
struct lua_State;

// ============================================================================
// LuaUI - Sistema de scripting Lua para UI
// ============================================================================

namespace UI {

class LuaUI {
public:
    static LuaUI& Get();
    
    // Inicialización y shutdown
    bool Initialize();
    void Shutdown();
    
    // Cargar scripts
    bool LoadScript(const std::string& scriptPath);
    bool ExecuteString(const std::string& code);
    
    // Llamar funciones desde C++
    bool CallFunction(const std::string& functionName);
    bool CallFunction(const std::string& functionName, float arg1);
    bool CallFunction(const std::string& functionName, const std::string& arg1);
    
    // Obtener valores desde Lua
    float GetFloat(const std::string& variableName);
    std::string GetString(const std::string& variableName);
    bool GetBool(const std::string& variableName);
    
    // Setear valores desde C++
    void SetFloat(const std::string& variableName, float value);
    void SetString(const std::string& variableName, const std::string& value);
    void SetBool(const std::string& variableName, bool value);
    
    // Registrar funciones C++ para Lua
    void RegisterFunction(const std::string& name, std::function<void()> func);
    void RegisterFunction(const std::string& name, std::function<void(float)> func);
    
    // Estados
    bool IsInitialized() const { return bInitialized; }
    
    // Obtener lua state (para bindings avanzados)
    lua_State* GetLuaState() { return luaState; }

private:
    LuaUI() = default;
    ~LuaUI();
    LuaUI(const LuaUI&) = delete;
    LuaUI& operator=(const LuaUI&) = delete;
    
    bool bInitialized = false;
    lua_State* luaState = nullptr;
    
    void RegisterEngineBindings();
    void RegisterUIBindings();
};

} // namespace UI

