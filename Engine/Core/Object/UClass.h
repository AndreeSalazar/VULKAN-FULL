#pragma once

#include "UObject.h"
#include <string>
#include <vector>
#include <unordered_map>

// ============================================================================
// UClass - Class reflection information (similar to UE5's UClass)
// ============================================================================

class UClass {
public:
    UClass(const char* className, const UClass* parentClass = nullptr);
    ~UClass();
    
    // Class information
    const std::string& GetName() const { return className; }
    const UClass* GetSuperClass() const { return superClass; }
    
    // Class comparison
    bool operator==(const UClass& other) const {
        return className == other.className;
    }
    
    bool IsChildOf(const UClass* otherClass) const;
    
    // Static registration (for future reflection system)
    static void RegisterClass(const UClass* classInfo);
    static const UClass* FindClass(const std::string& className);

private:
    std::string className;
    const UClass* superClass;
    
    // Static registry
    static std::unordered_map<std::string, const UClass*> classRegistry;
};

