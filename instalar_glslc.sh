#!/bin/bash

# Script para instalar glslc usando APT pinning
# Esto prioriza el repositorio de LunarG solo para glslc y sus dependencias

echo "=== Instalación de glslc usando APT Pinning ==="
echo ""

# Verificar si ya está instalado
if command -v glslc &> /dev/null; then
    echo "glslc ya está instalado:"
    glslc --version
    exit 0
fi

# Crear archivo de preferencias para priorizar LunarG solo para paquetes específicos
echo "Configurando APT pinning para glslc..."

sudo tee /etc/apt/preferences.d/lunarg-glslc > /dev/null <<'EOF'
Package: glslc shaderc* libshaderc*
Pin: origin packages.lunarg.com
Pin-Priority: 1001
EOF

echo "Archivo de preferencias creado."
echo ""

# Intentar instalar solo glslc (sin todo el SDK)
echo "Intentando instalar glslc..."
sudo apt update

# Buscar el paquete exacto
if apt-cache search glslc | grep -q glslc; then
    echo "Instalando glslc..."
    sudo apt install -y glslc
else
    echo "glslc no está disponible como paquete separado."
    echo "Intentando instalar desde shaderc..."
    
    # Intentar instalar shaderc que incluye glslc
    sudo apt install -y shaderc || {
        echo ""
        echo "❌ No se pudo instalar glslc desde los repositorios."
        echo ""
        echo "✅ SOLUCIÓN: Usa glslangValidator que ya funciona."
        echo "   Tu script compile_shaders.sh ya lo usa automáticamente."
        echo ""
        echo "Si realmente necesitas glslc, descárgalo manualmente:"
        echo "   1. Visita: https://vulkan.lunarg.com/sdk/home#linux"
        echo "   2. Descarga el SDK completo"
        echo "   3. Extrae solo glslc del directorio bin/"
        exit 1
    }
fi

# Verificar instalación
if command -v glslc &> /dev/null; then
    echo ""
    echo "✅ glslc instalado correctamente!"
    glslc --version
else
    echo ""
    echo "⚠️  glslc no se instaló, pero puedes usar glslangValidator."
    echo "   Ya está instalado y funciona perfectamente."
fi

