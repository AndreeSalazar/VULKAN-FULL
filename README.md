# Proyecto Vulkan - Cubo 3D

Este proyecto implementa un cubo 3D rotatorio usando Vulkan. El cubo tiene diferentes colores en cada cara y rota automáticamente.

## Requisitos

- Vulkan SDK instalado
- CMake (versión 3.10 o superior)
- GLFW3
- Compilador C++17 compatible (GCC, Clang, etc.)

### Instalación de dependencias en Linux

#### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install libvulkan-dev vulkan-tools vulkan-validationlayers
sudo apt-get install libglfw3-dev
sudo apt-get install cmake build-essential
```

#### Arch Linux:
```bash
sudo pacman -S vulkan-devel glfw cmake
```

#### Fedora:
```bash
sudo dnf install vulkan-devel glfw-devel cmake
```

## Compilación

1. **Compilar los shaders:**
   ```bash
   ./compile_shaders.sh
   ```
   
   Si no tienes `glslc` instalado, puedes instalarlo como parte del Vulkan SDK o usar:
   ```bash
   sudo apt-get install glslang-tools  # Ubuntu/Debian
   ```

2. **Crear directorio de build:**
   ```bash
   mkdir build
   cd build
   ```

3. **Configurar CMake:**
   ```bash
   cmake ..
   ```

4. **Compilar:**
   ```bash
   make
   ```

## Ejecución

Desde el directorio `build`:
```bash
./VulkanCube
```

O desde el directorio raíz:
```bash
./build/VulkanCube
```

## Estructura del Proyecto

- `main.cpp`: Punto de entrada principal, maneja la ventana GLFW y el loop principal
- `vulkan_cube.h` / `vulkan_cube.cpp`: Implementación completa de Vulkan (instancia, dispositivo, swap chain, pipeline, buffers, etc.)
- `shaders/vert.vert`: Shader de vértices (transformaciones 3D)
- `shaders/frag.frag`: Shader de fragmentos (colores)
- `CMakeLists.txt`: Configuración de CMake
- `compile_shaders.sh`: Script para compilar shaders GLSL a SPIR-V

## Características

- ✅ Cubo 3D con colores diferentes en cada cara
- ✅ Rotación automática en múltiples ejes
- ✅ Transformaciones 3D (modelo, vista, proyección)
- ✅ Manejo de redimensionamiento de ventana
- ✅ Sincronización adecuada de frames
- ✅ Validación layers de Vulkan (en modo debug)

## Notas

- Los shaders deben compilarse antes de ejecutar el programa
- Asegúrate de tener drivers de Vulkan instalados y actualizados
- En modo debug, se mostrarán mensajes de validación en la consola

## Solución de Problemas

### Error: "failed to find GPUs with Vulkan support"
- Verifica que tengas drivers de Vulkan instalados: `vulkaninfo`
- Si estás en una máquina virtual, puede que necesites habilitar soporte 3D

### Error: "failed to create instance"
- Verifica que Vulkan SDK esté instalado correctamente
- Asegúrate de tener las variables de entorno configuradas (si es necesario)

### Error: "failed to open file: shaders/vert.spv"
- Ejecuta `./compile_shaders.sh` para compilar los shaders
- Asegúrate de que los archivos `.spv` estén en el directorio `shaders/`

