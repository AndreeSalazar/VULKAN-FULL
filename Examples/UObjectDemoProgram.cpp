#include "Core/Log.h"
#include "Core/Object/UObjectDemo.h"
#include <iostream>

// Programa de demostración del sistema UObject
int main() {
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "╔══════════════════════════════════════════════════════════╗");
    UE_LOG_INFO(LogCategories::Core, "║                                                          ║");
    UE_LOG_INFO(LogCategories::Core, "║        UObject System - Programa de Demostración        ║");
    UE_LOG_INFO(LogCategories::Core, "║                                                          ║");
    UE_LOG_INFO(LogCategories::Core, "╚══════════════════════════════════════════════════════════╝");
    UE_LOG_INFO(LogCategories::Core, "");
    
    // Crear objeto de demostración
    UObjectDemo* demo = new UObjectDemo();
    
    // Demostrar todas las funcionalidades
    demo->DemonstrateFlags();
    demo->DemonstrateGarbageCollection();
    demo->DemonstrateNaming();
    demo->DemonstrateLifecycle();
    demo->DemonstrateComparison();
    
    // Mostrar información final
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    UE_LOG_INFO(LogCategories::Core, "📊 RESUMEN DEL OBJETO");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "  ID:          %u", demo->GetUniqueID());
    UE_LOG_INFO(LogCategories::Core, "  Nombre:      %s", demo->GetName().c_str());
    UE_LOG_INFO(LogCategories::Core, "  Clase:       %s", demo->GetClassTypeName());
    UE_LOG_INFO(LogCategories::Core, "  Enabled:     %s", demo->IsEnabled() ? "SÍ" : "NO");
    UE_LOG_INFO(LogCategories::Core, "  Valid:       %s", demo->IsValid() ? "SÍ" : "NO");
    UE_LOG_INFO(LogCategories::Core, "  RootSet:     %s", demo->IsRootSet() ? "SÍ" : "NO");
    UE_LOG_INFO(LogCategories::Core, "  Counter:     %d", demo->GetCounter());
    
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "╔══════════════════════════════════════════════════════════╗");
    UE_LOG_INFO(LogCategories::Core, "║  ✅ Demostración completada exitosamente               ║");
    UE_LOG_INFO(LogCategories::Core, "╚══════════════════════════════════════════════════════════╝");
    UE_LOG_INFO(LogCategories::Core, "");
    
    // Limpiar
    demo->EndPlay();
    delete demo;
    
    return 0;
}

