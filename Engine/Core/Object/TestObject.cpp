#include "TestObject.h"
#include "../Log.h"

// Static class instance
static const UClass* s_TestObjectClass = nullptr;

TestObject::TestObject() 
    : testValue(0)
{
    SetName("TestObject");
    SetFlags(EObjectFlags::RF_Public);
    
    UE_LOG_INFO(LogCategories::Core, "TestObject '%s' (ID: %u) created", 
                GetName().c_str(), GetUniqueID());
}

TestObject::~TestObject() {
    UE_LOG_INFO(LogCategories::Core, "TestObject '%s' (ID: %u) destroyed", 
                GetName().c_str(), GetUniqueID());
}

const UClass* TestObject::GetClass() const {
    return StaticClass();
}

const char* TestObject::GetClassTypeName() const {
    return "TestObject";
}

void TestObject::BeginPlay() {
    UE_LOG_INFO(LogCategories::Core, "TestObject '%s' BeginPlay called", GetName().c_str());
}

void TestObject::Tick(float deltaTime) {
    // Example: Could update object state here
}

void TestObject::EndPlay() {
    UE_LOG_INFO(LogCategories::Core, "TestObject '%s' EndPlay called", GetName().c_str());
}

const UClass* TestObject::StaticClass() {
    if (!s_TestObjectClass) {
        s_TestObjectClass = new UClass("TestObject");
    }
    return s_TestObjectClass;
}

