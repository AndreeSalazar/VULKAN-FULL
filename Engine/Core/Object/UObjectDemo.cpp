#include "UObjectDemo.h"

UObjectDemo::UObjectDemo() 
    : counter(0)
    , tickAccumulator(0.0f)
{
    SetName("UObjectDemo");
    SetFlags(EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
    
    UE_LOG_INFO(LogCategories::Core, "╔══════════════════════════════════════╗");
    UE_LOG_INFO(LogCategories::Core, "║  UObjectDemo creado                 ║");
    UE_LOG_INFO(LogCategories::Core, "║  ID: %-30u║", GetUniqueID());
    UE_LOG_INFO(LogCategories::Core, "║  Nombre: %-26s║", GetName().c_str());
    UE_LOG_INFO(LogCategories::Core, "╚══════════════════════════════════════╝");
}

UObjectDemo::~UObjectDemo() {
    UE_LOG_INFO(LogCategories::Core, "UObjectDemo '%s' (ID: %u) destruido", 
                GetName().c_str(), GetUniqueID());
}

const UClass* UObjectDemo::GetClass() const {
    return StaticClass();
}

const UClass* UObjectDemo::StaticClass() {
    static const UClass* s_Class = nullptr;
    if (!s_Class) {
        s_Class = new UClass("UObjectDemo");
    }
    return s_Class;
}

const char* UObjectDemo::GetClassTypeName() const {
    return "UObjectDemo";
}

void UObjectDemo::BeginPlay() {
    UE_LOG_INFO(LogCategories::Core, "▶ BeginPlay() llamado para '%s'", GetName().c_str());
    counter = 0;
}

void UObjectDemo::Tick(float deltaTime) {
    tickAccumulator += deltaTime;
    
    // Tick cada 1 segundo
    if (tickAccumulator >= 1.0f) {
        counter++;
        tickAccumulator = 0.0f;
        UE_LOG_VERBOSE(LogCategories::Core, "  ↻ Tick() - Counter: %d", counter);
    }
}

void UObjectDemo::EndPlay() {
    UE_LOG_INFO(LogCategories::Core, "■ EndPlay() llamado para '%s'", GetName().c_str());
}

void UObjectDemo::DemonstrateFlags() {
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    UE_LOG_INFO(LogCategories::Core, "📌 DEMOSTRACIÓN: Object Flags");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    // Mostrar flags actuales
    EObjectFlags currentFlags = GetFlags();
    UE_LOG_INFO(LogCategories::Core, "Flags actuales: 0x%08X", static_cast<uint32_t>(currentFlags));
    
    // Verificar flags específicos
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "Verificando flags:");
    UE_LOG_INFO(LogCategories::Core, "  ✓ RF_Public:      %s", 
                HasAnyFlags(EObjectFlags::RF_Public) ? "SÍ" : "NO");
    UE_LOG_INFO(LogCategories::Core, "  ✓ RF_Standalone:  %s", 
                HasAnyFlags(EObjectFlags::RF_Standalone) ? "SÍ" : "NO");
    UE_LOG_INFO(LogCategories::Core, "  ✓ RF_Transient:   %s", 
                HasAnyFlags(EObjectFlags::RF_Transient) ? "SÍ" : "NO");
    UE_LOG_INFO(LogCategories::Core, "  ✓ RF_MarkAsRootSet: %s", 
                HasAnyFlags(EObjectFlags::RF_MarkAsRootSet) ? "SÍ" : "NO");
    
    // Agregar flag
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "Agregando RF_Transactional...");
    SetFlags(EObjectFlags::RF_Transactional);
    UE_LOG_INFO(LogCategories::Core, "  RF_Transactional ahora: %s", 
                HasAnyFlags(EObjectFlags::RF_Transactional) ? "SÍ" : "NO");
    
    // Remover flag
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "Removiendo RF_Transactional...");
    ClearFlags(EObjectFlags::RF_Transactional);
    UE_LOG_INFO(LogCategories::Core, "  RF_Transactional ahora: %s", 
                HasAnyFlags(EObjectFlags::RF_Transactional) ? "SÍ" : "NO");
}

void UObjectDemo::DemonstrateGarbageCollection() {
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    UE_LOG_INFO(LogCategories::Core, "🗑️  DEMOSTRACIÓN: Garbage Collection");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "Estado inicial:");
    UE_LOG_INFO(LogCategories::Core, "  IsRootSet(): %s", IsRootSet() ? "SÍ" : "NO");
    
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "Llamando AddToRoot()...");
    AddToRoot();
    UE_LOG_INFO(LogCategories::Core, "  IsRootSet(): %s", IsRootSet() ? "SÍ" : "NO");
    UE_LOG_INFO(LogCategories::Core, "  → Objeto protegido de garbage collection");
    
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "Llamando RemoveFromRoot()...");
    RemoveFromRoot();
    UE_LOG_INFO(LogCategories::Core, "  IsRootSet(): %s", IsRootSet() ? "SÍ" : "NO");
    UE_LOG_INFO(LogCategories::Core, "  → Objeto puede ser eliminado por GC");
}

void UObjectDemo::DemonstrateNaming() {
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    UE_LOG_INFO(LogCategories::Core, "🏷️  DEMOSTRACIÓN: Naming System");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "Información del objeto:");
    UE_LOG_INFO(LogCategories::Core, "  Unique ID: %u", GetUniqueID());
    UE_LOG_INFO(LogCategories::Core, "  Nombre actual: '%s'", GetName().c_str());
    UE_LOG_INFO(LogCategories::Core, "  Tipo de clase: '%s'", GetClassTypeName());
    
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "Cambiando nombre a 'MiObjetoPersonalizado'...");
    SetName("MiObjetoPersonalizado");
    UE_LOG_INFO(LogCategories::Core, "  Nuevo nombre: '%s'", GetName().c_str());
    
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "Cambiando nombre a 'TestObject_123'...");
    SetName("TestObject_123");
    UE_LOG_INFO(LogCategories::Core, "  Nuevo nombre: '%s'", GetName().c_str());
}

void UObjectDemo::DemonstrateLifecycle() {
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    UE_LOG_INFO(LogCategories::Core, "🔄 DEMOSTRACIÓN: Lifecycle Hooks");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "Simulando ciclo de vida del objeto:");
    
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "1. BeginPlay()");
    BeginPlay();
    
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "2. Tick() varias veces (simulando 3 frames)...");
    Tick(0.016f); // Frame 1
    Tick(0.016f); // Frame 2
    Tick(0.016f); // Frame 3
    Tick(1.0f);   // Acumula 1 segundo (trigger counter)
    UE_LOG_INFO(LogCategories::Core, "   Counter después de ticks: %d", GetCounter());
    
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "3. EndPlay()");
    EndPlay();
}

void UObjectDemo::DemonstrateComparison() {
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    UE_LOG_INFO(LogCategories::Core, "🔍 DEMOSTRACIÓN: Object Comparison");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "Creando objeto de comparación...");
    UObjectDemo* obj2 = new UObjectDemo();
    
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "Comparación:");
    UE_LOG_INFO(LogCategories::Core, "  this->ID:  %u", GetUniqueID());
    UE_LOG_INFO(LogCategories::Core, "  obj2->ID:  %u", obj2->GetUniqueID());
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "  this == this:   %s", (*this == *this) ? "SÍ" : "NO");
    UE_LOG_INFO(LogCategories::Core, "  this == obj2:   %s", (*this == *obj2) ? "SÍ" : "NO");
    UE_LOG_INFO(LogCategories::Core, "  this != obj2:   %s", (*this != *obj2) ? "SÍ" : "NO");
    
    delete obj2;
}

