#include "UClass.h"
#include "../Log.h"

// Static member initialization
std::unordered_map<std::string, const UClass*> UClass::classRegistry;

UClass::UClass(const char* className, const UClass* parentClass)
    : className(className)
    , superClass(parentClass)
{
    // Auto-register class
    RegisterClass(this);
}

UClass::~UClass() {
    // Remove from registry (if this class is being destroyed)
    auto it = classRegistry.find(className);
    if (it != classRegistry.end() && it->second == this) {
        classRegistry.erase(it);
    }
}

bool UClass::IsChildOf(const UClass* otherClass) const {
    if (!otherClass) return false;
    
    const UClass* current = this;
    while (current) {
        if (current == otherClass) {
            return true;
        }
        current = current->superClass;
    }
    return false;
}

void UClass::RegisterClass(const UClass* classInfo) {
    if (!classInfo) return;
    
    classRegistry[classInfo->className] = classInfo;
    UE_LOG_VERBOSE(LogCategories::Core, "Registered UClass: %s", classInfo->className.c_str());
}

const UClass* UClass::FindClass(const std::string& className) {
    auto it = classRegistry.find(className);
    if (it != classRegistry.end()) {
        return it->second;
    }
    return nullptr;
}

