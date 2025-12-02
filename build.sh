#!/bin/bash

# Script para compilar todo el proyecto

echo "=== Compilando shaders ==="
./compile_shaders.sh

if [ $? -ne 0 ]; then
    echo "Error al compilar shaders"
    exit 1
fi

echo ""
echo "=== Creando directorio de build ==="
mkdir -p build
cd build

echo ""
echo "=== Configurando CMake ==="
cmake ..

if [ $? -ne 0 ]; then
    echo "Error al configurar CMake"
    exit 1
fi

echo ""
echo "=== Compilando proyecto ==="
make

if [ $? -eq 0 ]; then
    echo ""
    echo "=== Compilación exitosa! ==="
    echo "Ejecuta: ./build/VulkanCube"
else
    echo ""
    echo "=== Error en la compilación ==="
    exit 1
fi

