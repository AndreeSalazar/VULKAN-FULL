# Solución Definitiva para Instalar glslc

## Problema Identificado

1. ❌ La descarga directa del SDK da 404 (URL cambió)
2. ❌ El SDK completo tiene conflictos de dependencias con Pop!_OS
3. ✅ Pero `glslangValidator` ya funciona perfectamente

## Opción Recomendada: Usar glslangValidator

**Ya funciona con tu script actual.** No necesitas instalar nada más.

```bash
# Tu script ya está configurado para usar glslangValidator
./compile_shaders.sh
```

## Si Realmente Necesitas glslc

### Método 1: Descargar Manualmente desde el Sitio Web

1. **Visita**: https://vulkan.lunarg.com/sdk/home#linux
2. **Descarga** el SDK completo (tar.gz)
3. **Extrae** y copia solo `glslc`:

```bash
# Después de descargar y extraer el SDK
cd ~/vulkan-sdk
tar -xzf vulkansdk-linux-x86_64-*.tar.gz

# Buscar glslc
find . -name glslc -type f

# Copiar (ajusta la ruta según tu estructura)
sudo cp ./1.*/x86_64/bin/glslc /usr/local/bin/
sudo chmod +x /usr/local/bin/glslc

# Verificar
glslc --version
```

### Método 2: Intentar Instalar shaderc

```bash
# Instalar shaderc (puede incluir glslc)
sudo apt install shaderc

# Verificar
which glslc || echo "glslc no incluido en shaderc"
```

### Método 3: Usar APT Pinning (Avanzado)

```bash
# Crear preferencias para priorizar LunarG solo para glslc
sudo tee /etc/apt/preferences.d/lunarg-glslc > /dev/null <<'EOF'
Package: glslc shaderc* libshaderc*
Pin: origin packages.lunarg.com
Pin-Priority: 1001
EOF

sudo apt update
sudo apt install glslc
```

## Recomendación Final

**Usa `glslangValidator`** - Ya tienes todo lo necesario.

El comando `glslangValidator -V shader.vert -o shader.spv` hace exactamente lo mismo que `glslc shader.vert -o shader.spv`.

Tu script `compile_shaders.sh` ya detecta automáticamente qué compilador usar.

