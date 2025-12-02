#include "LuaUI.h"
#include "../../Core/Log.h"

// Lua includes - try different paths
#if __has_include(<lua.hpp>)
    #include <lua.hpp>
#elif __has_include(<lua.h>)
    extern "C" {
        #include <lua.h>
        #include <lauxlib.h>
        #include <lualib.h>
    }
#else
    extern "C" {
        #include "lua.h"
        #include "lauxlib.h"
        #include "lualib.h"
    }
#endif
#include <cstring>

namespace UI {

LuaUI& LuaUI::Get() {
    static LuaUI instance;
    return instance;
}

LuaUI::~LuaUI() {
    Shutdown();
}

bool LuaUI::Initialize() {
    if (bInitialized) {
        UE_LOG_WARNING(LogCategories::Core, "LuaUI already initialized");
        return false;
    }
    
    UE_LOG_INFO(LogCategories::Core, "Initializing Lua UI scripting...");
    
    // Crear lua state
    luaState = luaL_newstate();
    if (!luaState) {
        UE_LOG_ERROR(LogCategories::Core, "Failed to create Lua state");
        return false;
    }
    
    // Cargar librerías estándar
    luaL_openlibs(luaState);
    
    // Registrar bindings del motor y UI
    RegisterEngineBindings();
    RegisterUIBindings();
    
    bInitialized = true;
    UE_LOG_INFO(LogCategories::Core, "Lua UI scripting initialized successfully");
    return true;
}

void LuaUI::Shutdown() {
    if (!bInitialized) {
        return;
    }
    
    UE_LOG_INFO(LogCategories::Core, "Shutting down Lua UI scripting...");
    
    if (luaState) {
        lua_close(luaState);
        luaState = nullptr;
    }
    
    bInitialized = false;
    UE_LOG_INFO(LogCategories::Core, "Lua UI scripting shutdown complete");
}

bool LuaUI::LoadScript(const std::string& scriptPath) {
    if (!bInitialized || !luaState) {
        UE_LOG_ERROR(LogCategories::Core, "LuaUI not initialized");
        return false;
    }
    
    int result = luaL_dofile(luaState, scriptPath.c_str());
    if (result != LUA_OK) {
        const char* error = lua_tostring(luaState, -1);
        UE_LOG_ERROR(LogCategories::Core, "Lua script error (%s): %s", scriptPath.c_str(), error);
        lua_pop(luaState, 1);
        return false;
    }
    
    UE_LOG_INFO(LogCategories::Core, "Loaded Lua script: %s", scriptPath.c_str());
    return true;
}

bool LuaUI::ExecuteString(const std::string& code) {
    if (!bInitialized || !luaState) {
        return false;
    }
    
    int result = luaL_dostring(luaState, code.c_str());
    if (result != LUA_OK) {
        const char* error = lua_tostring(luaState, -1);
        UE_LOG_ERROR(LogCategories::Core, "Lua execution error: %s", error);
        lua_pop(luaState, 1);
        return false;
    }
    
    return true;
}

bool LuaUI::CallFunction(const std::string& functionName) {
    if (!bInitialized || !luaState) {
        return false;
    }
    
    lua_getglobal(luaState, functionName.c_str());
    if (lua_isfunction(luaState, -1)) {
        int result = lua_pcall(luaState, 0, 0, 0);
        if (result != LUA_OK) {
            const char* error = lua_tostring(luaState, -1);
            UE_LOG_ERROR(LogCategories::Core, "Lua function call error (%s): %s", functionName.c_str(), error);
            lua_pop(luaState, 1);
            return false;
        }
        return true;
    } else {
        lua_pop(luaState, 1);
        UE_LOG_WARNING(LogCategories::Core, "Lua function '%s' not found", functionName.c_str());
        return false;
    }
}

bool LuaUI::CallFunction(const std::string& functionName, float arg1) {
    if (!bInitialized || !luaState) {
        return false;
    }
    
    lua_getglobal(luaState, functionName.c_str());
    if (lua_isfunction(luaState, -1)) {
        lua_pushnumber(luaState, arg1);
        int result = lua_pcall(luaState, 1, 0, 0);
        if (result != LUA_OK) {
            const char* error = lua_tostring(luaState, -1);
            UE_LOG_ERROR(LogCategories::Core, "Lua function call error (%s): %s", functionName.c_str(), error);
            lua_pop(luaState, 1);
            return false;
        }
        return true;
    } else {
        lua_pop(luaState, 1);
        return false;
    }
}

bool LuaUI::CallFunction(const std::string& functionName, const std::string& arg1) {
    if (!bInitialized || !luaState) {
        return false;
    }
    
    lua_getglobal(luaState, functionName.c_str());
    if (lua_isfunction(luaState, -1)) {
        lua_pushstring(luaState, arg1.c_str());
        int result = lua_pcall(luaState, 1, 0, 0);
        if (result != LUA_OK) {
            const char* error = lua_tostring(luaState, -1);
            UE_LOG_ERROR(LogCategories::Core, "Lua function call error (%s): %s", functionName.c_str(), error);
            lua_pop(luaState, 1);
            return false;
        }
        return true;
    } else {
        lua_pop(luaState, 1);
        return false;
    }
}

float LuaUI::GetFloat(const std::string& variableName) {
    if (!bInitialized || !luaState) {
        return 0.0f;
    }
    
    lua_getglobal(luaState, variableName.c_str());
    if (lua_isnumber(luaState, -1)) {
        float value = (float)lua_tonumber(luaState, -1);
        lua_pop(luaState, 1);
        return value;
    }
    lua_pop(luaState, 1);
    return 0.0f;
}

std::string LuaUI::GetString(const std::string& variableName) {
    if (!bInitialized || !luaState) {
        return "";
    }
    
    lua_getglobal(luaState, variableName.c_str());
    if (lua_isstring(luaState, -1)) {
        std::string value = lua_tostring(luaState, -1);
        lua_pop(luaState, 1);
        return value;
    }
    lua_pop(luaState, 1);
    return "";
}

bool LuaUI::GetBool(const std::string& variableName) {
    if (!bInitialized || !luaState) {
        return false;
    }
    
    lua_getglobal(luaState, variableName.c_str());
    if (lua_isboolean(luaState, -1)) {
        bool value = lua_toboolean(luaState, -1) != 0;
        lua_pop(luaState, 1);
        return value;
    }
    lua_pop(luaState, 1);
    return false;
}

void LuaUI::SetFloat(const std::string& variableName, float value) {
    if (!bInitialized || !luaState) {
        return;
    }
    
    lua_pushnumber(luaState, value);
    lua_setglobal(luaState, variableName.c_str());
}

void LuaUI::SetString(const std::string& variableName, const std::string& value) {
    if (!bInitialized || !luaState) {
        return;
    }
    
    lua_pushstring(luaState, value.c_str());
    lua_setglobal(luaState, variableName.c_str());
}

void LuaUI::SetBool(const std::string& variableName, bool value) {
    if (!bInitialized || !luaState) {
        return;
    }
    
    lua_pushboolean(luaState, value ? 1 : 0);
    lua_setglobal(luaState, variableName.c_str());
}

void LuaUI::RegisterFunction(const std::string& name, std::function<void()> func) {
    // Basic registration - full implementation would use sol2 or manual C function registration
    UE_LOG_VERBOSE(LogCategories::Core, "Registering Lua function: %s", name.c_str());
    // TODO: Implement full function registration with lua_register
}

void LuaUI::RegisterFunction(const std::string& name, std::function<void(float)> func) {
    UE_LOG_VERBOSE(LogCategories::Core, "Registering Lua function: %s (float)", name.c_str());
    // TODO: Implement full function registration
}

void LuaUI::RegisterEngineBindings() {
    if (!luaState) return;
    
    // Registrar tabla Engine
    lua_newtable(luaState);
    
    // Engine.GetFPS()
    // Engine.GetDeltaTime()
    // Engine.GetFrameCount()
    // etc.
    
    lua_setglobal(luaState, "Engine");
    
    UE_LOG_VERBOSE(LogCategories::Core, "Registered Engine bindings for Lua");
}

void LuaUI::RegisterUIBindings() {
    if (!luaState) return;
    
    // Registrar tabla UI/ImGui
    lua_newtable(luaState);
    
    // UI.Begin()
    // UI.End()
    // UI.Text()
    // UI.Button()
    // etc.
    
    lua_setglobal(luaState, "UI");
    
    UE_LOG_VERBOSE(LogCategories::Core, "Registered UI bindings for Lua");
}

} // namespace UI

