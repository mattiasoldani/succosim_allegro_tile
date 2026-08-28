#include "CustomHit.hh"

// VolumeEDepHit/SD, i.e. to detect the energy deposited inside a certain volume
G4ThreadLocal G4Allocator<VolumeEDepHit>* allocatorVolumeEDepHit = nullptr;

// VolumeTrackingHit/SD, i.e. to detect hit position and energy deposited for each track inside a certain volume
G4ThreadLocal G4Allocator<VolumeTrackingHit>* allocatorVolumeTrackingHit = nullptr;

// ============================================================
// ============================================================
// allocate a new custom hit class here, G4ThreadLocal G4Allocator<...>* allocator... = nullptr;
