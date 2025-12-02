cmake_minimum_required(VERSION 3.10)
project(VulkanCube)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

# Include directories
include_directories(${VULKAN_INCLUDE_DIRS})

# Source files
set(SOURCES
    main.cpp
    vulkan_cube.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

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
