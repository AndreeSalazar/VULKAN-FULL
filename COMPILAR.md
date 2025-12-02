# Guía de Compilación

## ✅ Estado Actual

- ✅ Shaders compilados correctamente con `glslangValidator`
- ✅ CMake instalado (v3.22.1)
- ⚠️  Falta instalar librerías de desarrollo

## Pasos para Compilar

### 1. Instalar Dependencias

```bash
sudo apt-get install -y libvulkan-dev libglfw3-dev
```

### 2. Compilar Shaders (ya hecho)

```bash
./compile_shaders.sh
```

### 3. Compilar el Proyecto

```bash
# Opción A: Usar el script automatizado
./build.sh

# Opción B: Manualmente
mkdir -p build
cd build
cmake ..
make
```

### 4. Ejecutar

```bash
./build/VulkanCube
```

## Resumen

**`glslangValidator` es la mejor opción** porque:
- ✅ Ya está instalado y funciona
- ✅ No requiere dependencias adicionales
- ✅ Compila shaders igual que glslc
- ✅ El script lo detecta automáticamente

Solo necesitas instalar `libvulkan-dev` y `libglfw3-dev` para compilar el proyecto.

