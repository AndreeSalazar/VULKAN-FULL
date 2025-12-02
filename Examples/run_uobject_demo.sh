#!/bin/bash

# Script para compilar y ejecutar la demostración de UObject

echo "🔧 Compilando demostración de UObject..."
cd "$(dirname "$0")/../build"

# Compilar con BUILD_EXAMPLES=ON
cmake .. -DBUILD_EXAMPLES=ON
make UObjectDemo

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Compilación exitosa!"
    echo ""
    echo "🚀 Ejecutando demostración..."
    echo ""
    ./UObjectDemo
else
    echo "❌ Error en la compilación"
    exit 1
fi

