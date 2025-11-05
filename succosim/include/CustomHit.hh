#ifndef CUSTOMHIT_HH
#define CUSTOMHIT_HH

#include <G4VHit.hh>
#include <G4THitsMap.hh>
#include <G4ThreeVector.hh>

// VolumeEDepHit/SD, i.e. to detect the energy deposited inside a certain volume

class VolumeEDepHit : public G4VHit
{
public:
    inline void* operator new(size_t);
    inline void  operator delete(void*);
    void SetEDep(G4double eDep) { fEDep = eDep; }  // deposited energy setter
    G4double GetEDep() const { return fEDep; }  // deposited energy getter

private:
    G4double fEDep;
};

using VolumeEDepHitsCollection = G4THitsCollection<VolumeEDepHit>;
extern G4ThreadLocal G4Allocator<VolumeEDepHit> *allocatorVolumeEDepHit;

inline void* VolumeEDepHit::operator new(size_t)
{
    if (!allocatorVolumeEDepHit)
    {allocatorVolumeEDepHit = new G4Allocator<VolumeEDepHit>;}
    return allocatorVolumeEDepHit->MallocSingle();
}

inline void VolumeEDepHit::operator delete(void *aHit)
{
    if (!allocatorVolumeEDepHit)
    {allocatorVolumeEDepHit = new G4Allocator<VolumeEDepHit>;}
    allocatorVolumeEDepHit->FreeSingle((VolumeEDepHit*) aHit);
}

// VolumeTrackingHit/SD, i.e. to detect hit position and energy deposited for each track inside a certain volume

class VolumeTrackingHit : public G4VHit
{
public:
    inline void* operator new(size_t);
    inline void  operator delete(void*);
    void SetTrackId(G4int id) { fId = id; }  // track ID setter
    void SetX(G4ThreeVector x) { fX = x;}  // position setter
    void SetEDep(G4double eDep) { fEDep = eDep; }  // deposited energy setter
    G4int GetTrackId() const { return fId; }  // track ID getter
    G4ThreeVector GetX() const { return fX; }  // position getter
    G4double GetEDep() const { return fEDep; }  // deposited energy getter

private:
    G4int fId;
    G4double fEDep;
    G4ThreeVector fX;
};

using VolumeTrackingHitsCollection = G4THitsCollection<VolumeTrackingHit>;
extern G4ThreadLocal G4Allocator<VolumeTrackingHit> *allocatorVolumeTrackingHit;

inline void* VolumeTrackingHit::operator new(size_t)
{
    if (!allocatorVolumeTrackingHit)
    {allocatorVolumeTrackingHit = new G4Allocator<VolumeTrackingHit>;}
    return allocatorVolumeTrackingHit->MallocSingle();
}

inline void VolumeTrackingHit::operator delete(void *aHit)
{
    if (!allocatorVolumeTrackingHit)
    {allocatorVolumeTrackingHit = new G4Allocator<VolumeTrackingHit>;}
    allocatorVolumeTrackingHit->FreeSingle((VolumeTrackingHit*) aHit);
}

// ============================================================
// ============================================================
// define custom hit classes here



// ============================================================
#endif
