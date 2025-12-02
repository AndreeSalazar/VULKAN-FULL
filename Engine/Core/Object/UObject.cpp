#include "UObject.h"
#include "../Log.h"

// Static member initialization
uint32_t UObject::nextUniqueId = 1;

UObject::UObject()
    : uniqueId(nextUniqueId++)
    , name("UObject")
    , objectFlags(EObjectFlags::RF_NoFlags)
    , outer(nullptr)
    , bEnabled(true)
    , bPendingKill(false)
{
    // Default name with ID
    name += "_" + std::to_string(uniqueId);
}

UObject::~UObject() {
    // Cleanup is handled by derived classes
}

void UObject::AddToRoot() {
    SetFlags(EObjectFlags::RF_MarkAsRootSet);
    UE_LOG_VERBOSE(LogCategories::Core, "UObject '%s' (ID: %u) added to root set", 
                   name.c_str(), uniqueId);
}

void UObject::RemoveFromRoot() {
    ClearFlags(EObjectFlags::RF_MarkAsRootSet);
    UE_LOG_VERBOSE(LogCategories::Core, "UObject '%s' (ID: %u) removed from root set", 
                   name.c_str(), uniqueId);
}

