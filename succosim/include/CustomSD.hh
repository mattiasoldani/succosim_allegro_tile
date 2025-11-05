#ifndef CUSTOMSD_HH
#define CUSTOMSD_HH

#include <G4VSensitiveDetector.hh>

#include "CustomHit.hh"

// VolumeEDepHit/SD, i.e. to detect the energy deposited inside a certain volume

class VolumeEDepSD : public G4VSensitiveDetector
{
public:
    VolumeEDepSD(G4String name);
    void Initialize(G4HCofThisEvent*) override;

protected:
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override;

private:
    VolumeEDepHitsCollection* fVolumeEDepHitsCollection { nullptr };
    G4int fVolumeEDepHitsCollectionId { -1 };
};

// VolumeTrackingHit/SD, i.e. to detect hit position and energy deposited for each track inside a certain volume

class VolumeTrackingSD : public G4VSensitiveDetector
{
public:
    VolumeTrackingSD(G4String name);
    void Initialize(G4HCofThisEvent*) override;

protected:
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override;

private:
    VolumeTrackingHitsCollection* fVolumeTrackingHitsCollection { nullptr };
    G4int fVolumeTrackingHitsCollectionId { -1 };
};

// ============================================================
// ============================================================
// define custom sensitive detector classes here



// ============================================================
#endif
