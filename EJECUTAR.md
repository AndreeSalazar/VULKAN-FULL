# 🚀 Guía de Compilación y Ejecución

## 📋 Compilar el Proyecto

```bash
# 1. Ir al directorio del proyecto
cd "/home/andre/Documentos/VULKAN FULL"

# 2. Crear directorio de build (si no existe)
mkdir -p build
cd build

# 3. Configurar con CMake
cmake ..

# 4. Compilar
make VulkanCube

# O compilar todo en un paso:
cd build && cmake .. && make VulkanCube
```

## ▶️ Ejecutar el Motor

```bash
# Desde el directorio build/
cd build
./VulkanCube

# O desde el directorio raíz:
cd "/home/andre/Documentos/VULKAN FULL"
./build/VulkanCube
```

## 🎮 Controles

- **WASD**: Mover cámara
- **Mouse**: Rotar vista (cuando el mouse está bloqueado)
- **ESC**: Bloquear/Desbloquear mouse
- **F11**: Maximizar/Restaurar ventana
- **TAB**: Mostrar/Ocultar UI (ImGui)

## ✅ Verificación Pre-Ejecución

Antes de ejecutar, verifica:

1. **Compilación exitosa:**
   ```bash
   cd build
   make VulkanCube
   # Debe mostrar: [100%] Built target VulkanCube
   ```

2. **Ejecutable existe:**
   ```bash
   ls -lh build/VulkanCube
   # Debe mostrar el archivo ejecutable
   ```

3. **Dependencias instaladas:**
   - Vulkan: `vulkaninfo` (opcional, para verificar)
   - GLFW: Ya incluido
   - Lua: `liblua5.4-dev` instalado
   - ImGui: Incluido como ThirdParty

## 🐛 Solución de Problemas

### Error: "failed to find validation layers"
- **Solución**: Es una advertencia, no un error. El motor funciona sin validation layers.

### Error: "failed to create GLFW window"
- **Verificar**: Drivers de gráficos instalados
- **Verificar**: Vulkan funciona: `vulkaninfo` (opcional)

### Error: "ImGui initialization failed"
- **Solución**: El motor funciona sin ImGui, pero la UI no estará disponible.

### Ventana no se abre
- **Verificar**: Logs en `Engine.log`
- **Verificar**: Variables de entorno de Vulkan si es necesario

## 📊 Salida Esperada

Al ejecutar, deberías ver:

```
[INFO] === Vulkan Engine Starting ===
[INFO] Initializing GLFW...
[INFO] Creating window: 1920x1080...
[INFO] Initializing Vulkan...
[INFO] Vulkan initialized successfully
[INFO] Camera initialized - Position: (0, 0, -3)
[INFO] Initializing ImGui...
[INFO] ImGui initialized successfully
[INFO] Entering main loop...
[INFO] Controls: WASD - Move | Mouse - Look | ESC - Lock/Unlock Mouse | F11 - Maximize/Restore | TAB - Toggle UI
```

## 🔧 Comandos Rápidos

```bash
# Compilar y ejecutar en un paso:
cd "/home/andre/Documentos/VULKAN FULL/build" && make VulkanCube && ./VulkanCube

# Solo compilar:
cd build && make

# Limpiar y recompilar:
cd build && rm -rf * && cmake .. && make VulkanCube

# Ver logs mientras ejecutas:
cd build && ./VulkanCube 2>&1 | tee ../Engine.log
```

## 📝 Notas

- El log se guarda en `Engine.log` en el directorio raíz
- Los shaders se copian automáticamente al directorio `build/`
- La UI de ImGui aparece cuando presionas TAB (si está habilitada)

---

**Última actualización**: Diciembre 2024

