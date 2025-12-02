#pragma once

#include "UObject.h"
#include "UClass.h"
#include "../Log.h"

// ============================================================================
// UObjectDemo - Clase de demostración para mostrar funcionalidades UObject
// ============================================================================

class UObjectDemo : public UObject {
public:
    UObjectDemo();
    virtual ~UObjectDemo();
    
    // UObject interface
    virtual const UClass* GetClass() const override;
    virtual const char* GetClassTypeName() const override;
    
    // Lifecycle
    virtual void BeginPlay() override;
    virtual void Tick(float deltaTime) override;
    virtual void EndPlay() override;
    
    // Demo methods
    void DemonstrateFlags();
    void DemonstrateGarbageCollection();
    void DemonstrateNaming();
    void DemonstrateLifecycle();
    void DemonstrateComparison();
    
    // Test properties
    int GetCounter() const { return counter; }
    void SetCounter(int value) { counter = value; }

private:
    static const UClass* StaticClass();
    
    int counter;
    float tickAccumulator;
};

