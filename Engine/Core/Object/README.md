# UObject System - Sistema Base de Objetos

## 📋 Descripción

Sistema de objetos base similar a Unreal Engine 5's UObject, proporcionando:
- Identificación única de objetos
- Sistema de flags (RF_Public, RF_Standalone, etc.)
- Garbage collection básico (AddToRoot/RemoveFromRoot)
- Reflection básico (UClass)
- Lifecycle hooks (BeginPlay, Tick, EndPlay)

## 🔧 Uso Básico

### Crear una clase derivada de UObject

```cpp
#include "Core/Object/UObject.h"
#include "Core/Object/UClass.h"

class MyObject : public UObject {
public:
    MyObject() {
        SetName("MyObject");
        SetFlags(EObjectFlags::RF_Public);
    }
    
    virtual ~MyObject() = default;
    
    // Requerido: Implementar GetClass
    virtual const UClass* GetClass() const override {
        static const UClass* s_Class = new UClass("MyObject");
        return s_Class;
    }
    
    virtual const char* GetClassTypeName() const override {
        return "MyObject";
    }
    
    // Lifecycle hooks
    virtual void BeginPlay() override {
        UE_LOG_INFO(LogCategories::Core, "MyObject BeginPlay");
    }
    
    virtual void Tick(float deltaTime) override {
        // Update logic
    }
    
    virtual void EndPlay() override {
        UE_LOG_INFO(LogCategories::Core, "MyObject EndPlay");
    }
};

// Uso
MyObject* obj = new MyObject();
obj->SetName("MyCustomObject");
obj->AddToRoot();  // Prevenir garbage collection
obj->BeginPlay();
```

## 🎯 Características

### Object Flags

```cpp
// Verificar flags
if (obj->HasAnyFlags(EObjectFlags::RF_Public)) {
    // Object is public
}

// Agregar flags
obj->SetFlags(EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);

// Remover flags
obj->ClearFlags(EObjectFlags::RF_Transient);
```

### Garbage Collection

```cpp
// Agregar a root (prevenir GC)
obj->AddToRoot();

// Remover de root (permitir GC)
obj->RemoveFromRoot();

// Verificar si está en root
if (obj->IsRootSet()) {
    // Object won't be garbage collected
}
```

### Object Information

```cpp
// ID único
uint32_t id = obj->GetUniqueID();

// Nombre
const std::string& name = obj->GetName();
obj->SetName("NewName");

// Enabled/Disabled
obj->SetEnabled(false);
bool isEnabled = obj->IsEnabled();
```

### Lifecycle

```cpp
// Llamados automáticamente en el orden correcto:
obj->BeginPlay();   // Inicialización
obj->Tick(deltaTime); // Actualización (si está habilitado)
obj->EndPlay();     // Limpieza
```

## 📚 Flags Disponibles

- `RF_Public` - Objeto es público
- `RF_Standalone` - Mantener en memoria incluso si no está referenciado
- `RF_MarkAsRootSet` - Marcar como root (prevenir GC)
- `RF_Transactional` - Objeto es transaccional (para undo/redo)
- `RF_ClassDefaultObject` - Es un objeto default de clase
- `RF_ArchetypeObject` - Es un archetype
- `RF_Transient` - Objeto es transitorio (no guardar)
- `RF_MarkAsNative` - Marcado como nativo (clase C++)
- `RF_LoadCompleted` - Carga de objeto completada
- `RF_HasLoaded` - Objeto ha sido cargado
- `RF_WasLoaded` - Objeto fue cargado desde disco

## 🔮 Próximas Mejoras

- [ ] Garbage Collector completo
- [ ] Serialización (Archive system)
- [ ] Property reflection avanzado
- [ ] Function reflection
- [ ] Object pooling
- [ ] Tags system

