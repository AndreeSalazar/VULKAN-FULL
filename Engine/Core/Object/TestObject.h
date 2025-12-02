#pragma once

#include "UObject.h"
#include "UClass.h"

// ============================================================================
// TestObject - Example derived class from UObject
// ============================================================================

class TestObject : public UObject {
public:
    TestObject();
    virtual ~TestObject();
    
    // UObject interface
    virtual const UClass* GetClass() const override;
    virtual const char* GetClassTypeName() const override;
    
    // Lifecycle
    virtual void BeginPlay() override;
    virtual void Tick(float deltaTime) override;
    virtual void EndPlay() override;
    
    // Test properties
    int GetValue() const { return testValue; }
    void SetValue(int value) { testValue = value; }

private:
    static const UClass* StaticClass();
    
    int testValue;
};

