#!/bin/bash

# Script para compilar los shaders GLSL a SPIR-V

COMPILER=""
if command -v glslc &> /dev/null
then
    COMPILER="glslc"
    echo "Usando glslc..."
elif command -v glslangValidator &> /dev/null
then
    COMPILER="glslangValidator"
    echo "Usando glslangValidator (glslc no encontrado)..."
else
    echo "ERROR: No se encontró ningún compilador de shaders."
    echo "Por favor instala uno de los siguientes:"
    echo "  - glslc (viene con Vulkan SDK completo)"
    echo "  - glslang-tools (incluye glslangValidator)"
    echo ""
    echo "Para instalar glslang-tools:"
    echo "  sudo apt-get install glslang-tools"
    echo ""
    echo "Para instalar Vulkan SDK completo:"
    echo "  Visita: https://vulkan.lunarg.com/sdk/home#linux"
    exit 1
fi

echo "Compilando shaders..."

if [ "$COMPILER" = "glslc" ]; then
    glslc shaders/vert.vert -o shaders/vert.spv
    glslc shaders/frag.frag -o shaders/frag.spv
elif [ "$COMPILER" = "glslangValidator" ]; then
    glslangValidator -V shaders/vert.vert -o shaders/vert.spv
    glslangValidator -V shaders/frag.frag -o shaders/frag.spv
fi

if [ $? -eq 0 ]; then
    echo "Shaders compilados exitosamente!"
else
    echo "Error al compilar shaders"
    exit 1
fi

