cmake_minimum_required(VERSION 3.10)
project(VulkanCube)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Engine root directory
set(ENGINE_ROOT ${CMAKE_SOURCE_DIR}/Engine)

# Find packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

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
    ${ENGINE_ROOT}/RHI
    ${ENGINE_ROOT}/Rendering
    ${ENGINE_ROOT}/Input
    ${ENGINE_ROOT}/Platform
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
    ${ENGINE_ROOT}/UI/Panels/DebugOverlay.cpp
    ${ENGINE_ROOT}/UI/Panels/StatsPanel.cpp
    ${ENGINE_ROOT}/UI/Panels/ObjectHierarchyPanel.cpp
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
)

# Create executable
add_executable(${PROJECT_NAME} ${ALL_SOURCES})

# Set include directories for main executable
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})

# Link libraries - Vulkan se carga dinámicamente, pero necesitamos el loader
find_library(VULKAN_LIB vulkan HINTS ${VULKAN_LIBRARIES})

target_link_libraries(${PROJECT_NAME} 
    ${VULKAN_LIBRARIES}
    vulkan
    glfw
    pthread
    dl
    X11
)

# Copy shaders to build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
