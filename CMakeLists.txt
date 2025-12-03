cmake_minimum_required(VERSION 3.10)
project(VulkanCube)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Engine root directory
set(ENGINE_ROOT ${CMAKE_SOURCE_DIR}/Engine)

# Find packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

# Third-party directories (definir PRIMERO antes de usarlos)
set(THIRDPARTY_ROOT ${CMAKE_SOURCE_DIR}/Engine/ThirdParty)
set(SOL2_ROOT ${THIRDPARTY_ROOT}/sol2)

# Rust UI (eGUI) - compilar librería Rust
# Ubicación: Engine/UI/Rust (integrad dentro de UI)
set(RUST_UI_ROOT ${CMAKE_SOURCE_DIR}/Engine/UI/Rust)
find_program(CARGO cargo)
if(CARGO)
    message(STATUS "✓ Cargo encontrado: ${CARGO}")
    
    # Compilar librería Rust
    # La librería se genera en target/release/libengine_ui.a
    set(RUST_LIB_PATH "${RUST_UI_ROOT}/target/release/libengine_ui.a")
    
    add_custom_command(
        OUTPUT ${RUST_LIB_PATH}
        COMMAND ${CARGO} build --release --manifest-path ${RUST_UI_ROOT}/Cargo.toml
        WORKING_DIRECTORY ${RUST_UI_ROOT}
        COMMENT "Building Rust UI library (eGUI)"
    )
    
    add_custom_target(rust_ui_lib DEPENDS ${RUST_LIB_PATH})
    message(STATUS "✓ Rust UI library target created")
else()
    message(WARNING "Cargo no encontrado - eGUI features will be disabled")
    message(WARNING "Instalar Rust: curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh")
    set(RUST_UI_LIB "")
endif()

# Lua: Usar headers del sistema (ya instalado)
# Verificar si Lua está instalado en el sistema
find_path(LUA_INCLUDE_DIR
    NAMES lua.h
    PATHS
        /usr/include/lua5.4
        /usr/include/lua
        /usr/local/include/lua5.4
        /usr/local/include/lua
    NO_DEFAULT_PATH
)

if(LUA_INCLUDE_DIR)
    set(LUA_ROOT ${LUA_INCLUDE_DIR}/..)
    message(STATUS "✓ Lua encontrado en: ${LUA_INCLUDE_DIR}")
else()
    # Fallback: intentar usar ThirdParty
    set(LUA_ROOT ${THIRDPARTY_ROOT}/lua)
    message(WARNING "Lua no encontrado en sistema, usando ThirdParty (si existe)")
endif()

# Lua: Usar biblioteca del sistema (preferido) o compilar desde fuente
if(LUA_INCLUDE_DIR)
    # Usar biblioteca del sistema (más fácil)
    find_library(LUA_LIBRARY
        NAMES lua5.4 lua54 lua
        PATHS
            /usr/lib
            /usr/local/lib
            /lib/x86_64-linux-gnu
    )
    if(LUA_LIBRARY)
        message(STATUS "✓ Biblioteca Lua encontrada: ${LUA_LIBRARY}")
        set(LUA_SOURCES "")  # No necesitamos compilar desde fuente
    else()
        message(WARNING "Biblioteca Lua no encontrada, intentando compilar desde fuente")
        set(LUA_SOURCES "")
    endif()
elseif(EXISTS "${LUA_ROOT}/src")
    # Fallback: compilar desde fuente en ThirdParty
    file(GLOB LUA_SOURCES
        "${LUA_ROOT}/src/*.c"
    )
    list(REMOVE_ITEM LUA_SOURCES "${LUA_ROOT}/src/lua.c" "${LUA_ROOT}/src/luac.c")
    message(STATUS "✓ Compilando Lua desde ThirdParty")
endif()


# Include directories (before add_executable)
set(INCLUDE_DIRS
    ${VULKAN_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}
    ${ENGINE_ROOT}
    ${ENGINE_ROOT}/Core
    ${ENGINE_ROOT}/Core/Math
    ${ENGINE_ROOT}/Core/Object
    ${ENGINE_ROOT}/Core/Threading
    ${ENGINE_ROOT}/UI
    ${ENGINE_ROOT}/UI/Scripting
    ${ENGINE_ROOT}/RHI
    ${ENGINE_ROOT}/Rendering
    ${ENGINE_ROOT}/Input
    ${ENGINE_ROOT}/Platform
    ${LUA_INCLUDE_DIR}  # Headers del sistema (ej: /usr/include/lua5.4)
    ${LUA_ROOT}/src     # Fallback a ThirdParty si existe
    ${SOL2_ROOT}/include
)

# Engine Core sources
set(ENGINE_CORE_SOURCES
    ${ENGINE_ROOT}/Core/Log.cpp
    ${ENGINE_ROOT}/Core/Timer.cpp
    ${ENGINE_ROOT}/Core/Math/Vector.cpp
    ${ENGINE_ROOT}/Core/Math/Matrix.cpp
    ${ENGINE_ROOT}/Core/Math/Quaternion.cpp
    ${ENGINE_ROOT}/Core/Math/Transform.cpp
    ${ENGINE_ROOT}/Core/Object/UObject.cpp
    ${ENGINE_ROOT}/Core/Object/UClass.cpp
    ${ENGINE_ROOT}/Core/Object/UObjectDemo.cpp
    ${ENGINE_ROOT}/Core/Threading/RenderCommandQueue.cpp
    ${ENGINE_ROOT}/Core/Threading/ThreadManager.cpp
    ${ENGINE_ROOT}/UI/UIBase.cpp
    ${ENGINE_ROOT}/UI/UIManager.cpp
    ${ENGINE_ROOT}/UI/EGUIWrapper.cpp
    ${ENGINE_ROOT}/UI/VulkanRenderer.cpp
    ${ENGINE_ROOT}/UI/Scripting/LuaUI.cpp
        ${ENGINE_ROOT}/UI/Panels/DebugOverlay.cpp
        ${ENGINE_ROOT}/UI/Panels/StatsPanel.cpp
        ${ENGINE_ROOT}/UI/Panels/ObjectHierarchyPanel.cpp
        ${ENGINE_ROOT}/UI/Panels/ViewportPanel.cpp
        ${ENGINE_ROOT}/UI/Panels/DetailsPanel.cpp
        ${ENGINE_ROOT}/UI/Panels/ContentBrowserPanel.cpp
        ${ENGINE_ROOT}/UI/Panels/ConsolePanel.cpp
        ${ENGINE_ROOT}/UI/Panels/MenuBar.cpp
        ${ENGINE_ROOT}/UI/Panels/StatusBar.cpp
    ${ENGINE_ROOT}/Rendering/Camera.cpp
)

# RHI sources
set(RHI_SOURCES
    ${ENGINE_ROOT}/RHI/vulkan_cube.cpp
)

# Input sources
set(INPUT_SOURCES
    ${ENGINE_ROOT}/Input/InputManager.cpp
)

# Application sources
set(APP_SOURCES
    ${CMAKE_SOURCE_DIR}/Source/main.cpp
)

# Example programs (optional)
option(BUILD_EXAMPLES "Build example programs" OFF)

if(BUILD_EXAMPLES)
    # UObject Demo
    add_executable(UObjectDemo ${CMAKE_SOURCE_DIR}/Examples/UObjectDemoProgram.cpp ${ENGINE_CORE_SOURCES})
    target_include_directories(UObjectDemo PRIVATE ${INCLUDE_DIRS})
    target_link_libraries(UObjectDemo 
        PRIVATE
        ${VULKAN_LIBRARIES}
        vulkan
        glfw
        pthread
        dl
        X11
    )
    
    # Threading Demo
    add_executable(ThreadingDemo ${CMAKE_SOURCE_DIR}/Examples/ThreadingDemo.cpp ${ENGINE_CORE_SOURCES})
    target_include_directories(ThreadingDemo PRIVATE ${INCLUDE_DIRS})
    target_link_libraries(ThreadingDemo 
        PRIVATE
        pthread
    )
endif()

# All sources
set(ALL_SOURCES
    ${ENGINE_CORE_SOURCES}
    ${RHI_SOURCES}
    ${INPUT_SOURCES}
    ${APP_SOURCES}
    ${LUA_SOURCES}
)

# Create executable
add_executable(${PROJECT_NAME} ${ALL_SOURCES})

# Dependencia en librería Rust (OPCIONAL - permite compilar sin Rust)
if(CARGO)
    add_dependencies(${PROJECT_NAME} rust_ui_lib)
    message(STATUS "✓ Rust dependency configured")
else()
    message(STATUS "⚠ Rust/Cargo not found - UI will be minimal (panels are stubs)")
endif()

# Set include directories for main executable
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})

# Link libraries - Vulkan se carga dinámicamente, pero necesitamos el loader
find_library(VULKAN_LIB vulkan HINTS ${VULKAN_LIBRARIES})

target_link_libraries(${PROJECT_NAME} 
        PRIVATE
        ${VULKAN_LIBRARIES}
        vulkan
        glfw
        pthread
        dl
        X11
        stdc++fs  # Para std::experimental::filesystem
    )

# Linkear librería Rust si existe
if(CARGO)
    set(RUST_LIB_PATH "${RUST_UI_ROOT}/target/release/libengine_ui.a")
    # Si la librería ya existe o se va a construir, linkearla
    if(EXISTS ${RUST_LIB_PATH} OR TARGET rust_ui_lib)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${RUST_LIB_PATH})
        message(STATUS "✓ Rust UI library will be linked: ${RUST_LIB_PATH}")
    else()
        message(WARNING "Rust library not found, will be built during compilation")
        # Linkear de todos modos (se construirá durante el build)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${RUST_LIB_PATH})
    endif()
endif()

# Linkear Lua si se encontró la biblioteca del sistema
if(LUA_LIBRARY)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${LUA_LIBRARY})
    message(STATUS "✓ Lua linkeado: ${LUA_LIBRARY}")
endif()

# Copy shaders to build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
