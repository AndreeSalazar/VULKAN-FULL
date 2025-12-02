#!/bin/bash

# Script para ejecutar el programa y capturar errores

echo "Ejecutando VulkanCube..."
echo "Presiona Ctrl+C para salir"
echo ""

cd "$(dirname "$0")"
./build/VulkanCube 2>&1

EXIT_CODE=$?

if [ $EXIT_CODE -ne 0 ]; then
    echo ""
    echo "El programa terminó con código de salida: $EXIT_CODE"
    echo "Revisa los mensajes de error arriba."
fi

exit $EXIT_CODE

