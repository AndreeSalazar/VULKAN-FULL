#pragma once

#include <string>
#include <cstdint>

// ============================================================================
// UObject - Base class for all engine objects (similar to UE5's UObject)
// ============================================================================

// Object flags (similar to RF_ flags in Unreal)
enum class EObjectFlags : uint32_t {
    RF_NoFlags = 0,
    RF_Public = 1 << 0,           // Object is public and can be accessed
    RF_Standalone = 1 << 1,       // Keep object in memory even if unreferenced
    RF_MarkAsRootSet = 1 << 2,    // Mark as root (prevent GC)
    RF_Transactional = 1 << 3,    // Object is transactional (for undo/redo)
    RF_ClassDefaultObject = 1 << 4, // Is a class default object
    RF_ArchetypeObject = 1 << 5,  // Is an archetype
    RF_Transient = 1 << 6,        // Object is transient (don't save)
    RF_MarkAsNative = 1 << 7,     // Marked as native (C++ class)
    RF_LoadCompleted = 1 << 8,    // Object load is complete
    RF_HasLoaded = 1 << 9,        // Object has been loaded
    RF_WasLoaded = 1 << 10,       // Object was loaded from disk
};

// Bitwise operators for flags
inline EObjectFlags operator|(EObjectFlags a, EObjectFlags b) {
    return static_cast<EObjectFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline EObjectFlags operator&(EObjectFlags a, EObjectFlags b) {
    return static_cast<EObjectFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline EObjectFlags& operator|=(EObjectFlags& a, EObjectFlags b) {
    a = a | b;
    return a;
}

inline EObjectFlags& operator&=(EObjectFlags& a, EObjectFlags b) {
    a = a & b;
    return a;
}

inline EObjectFlags operator~(EObjectFlags a) {
    return static_cast<EObjectFlags>(~static_cast<uint32_t>(a));
}

// Forward declaration
class UClass;

class UObject {
public:
    UObject();
    virtual ~UObject();
    
    // Object identification
    uint32_t GetUniqueID() const { return uniqueId; }
    const std::string& GetName() const { return name; }
    void SetName(const std::string& newName) { name = newName; }
    
    // Object flags
    bool HasAnyFlags(EObjectFlags flags) const {
        return (objectFlags & flags) != EObjectFlags::RF_NoFlags;
    }
    
    bool HasAllFlags(EObjectFlags flags) const {
        return (objectFlags & flags) == flags;
    }
    
    void SetFlags(EObjectFlags flags) {
        objectFlags = objectFlags | flags;
    }
    
    void ClearFlags(EObjectFlags flags) {
        objectFlags = objectFlags & (~flags);
    }
    
    EObjectFlags GetFlags() const { return objectFlags; }
    
    // Garbage collection
    void AddToRoot();      // Prevent garbage collection
    void RemoveFromRoot(); // Allow garbage collection
    
    bool IsRootSet() const {
        return HasAnyFlags(EObjectFlags::RF_MarkAsRootSet);
    }
    
    // Reflection (basic)
    virtual const UClass* GetClass() const = 0;
    virtual const char* GetClassTypeName() const = 0;
    
    // Object lifecycle (similar to UE5)
    virtual void BeginPlay() {}      // Called when object is created/loaded
    virtual void Tick(float deltaTime) {} // Called every frame (if enabled)
    virtual void EndPlay() {}        // Called when object is destroyed
    
    // Object state
    bool IsValid() const { return !bPendingKill; }
    void MarkPendingKill() { bPendingKill = true; }
    bool IsPendingKill() const { return bPendingKill; }
    
    // Outer object (parent object in hierarchy)
    UObject* GetOuter() const { return outer; }
    void SetOuter(UObject* newOuter) { outer = newOuter; }
    
    // Enabled/Disabled
    bool IsEnabled() const { return bEnabled; }
    void SetEnabled(bool enabled) { bEnabled = enabled; }
    
    // Object comparison
    bool operator==(const UObject& other) const {
        return uniqueId == other.uniqueId;
    }
    
    bool operator!=(const UObject& other) const {
        return !(*this == other);
    }

protected:
    uint32_t uniqueId;               // Unique identifier
    std::string name;                // Object name
    EObjectFlags objectFlags;        // Object flags
    UObject* outer;                  // Outer (parent) object
    bool bEnabled;                   // Is object enabled?
    bool bPendingKill;               // Marked for deletion
    
    // Static counter for unique IDs
    static uint32_t nextUniqueId;

private:
    // Disable copy (objects should be managed by GC)
    UObject(const UObject&) = delete;
    UObject& operator=(const UObject&) = delete;
};

