#include <G4SDManager.hh>

#include "CustomSD.hh"

// VolumeEDepHit/SD, i.e. to detect the energy deposited inside a certain volume

VolumeEDepSD::VolumeEDepSD(G4String name) :  G4VSensitiveDetector(name)
{collectionName.insert("VolumeEDep");}

G4bool VolumeEDepSD::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
    VolumeEDepHit* hit = new VolumeEDepHit();
    hit->SetEDep(aStep->GetTotalEnergyDeposit());  // getting total energy deposited in the volume in the step
    fVolumeEDepHitsCollection->insert(hit);
    return true;
}

void VolumeEDepSD::Initialize(G4HCofThisEvent* hcof)
{
    fVolumeEDepHitsCollection = new VolumeEDepHitsCollection(SensitiveDetectorName, collectionName[0]);
    if (fVolumeEDepHitsCollectionId < 0)
    {fVolumeEDepHitsCollectionId = G4SDManager::GetSDMpointer()->GetCollectionID(GetName() + "/" + collectionName[0]);}
    hcof->AddHitsCollection(fVolumeEDepHitsCollectionId, fVolumeEDepHitsCollection);
}

// VolumeTrackingHit/SD, i.e. to detect hit position and energy deposited for each track inside a certain volume

VolumeTrackingSD::VolumeTrackingSD(G4String name) :  G4VSensitiveDetector(name)
{collectionName.insert("VolumeTracking");}

G4bool VolumeTrackingSD::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
    VolumeTrackingHit* hit = new VolumeTrackingHit();
    hit->SetTrackId(aStep->GetTrack()->GetTrackID());  // getting current track ID
    hit->SetX(aStep->GetPreStepPoint()->GetPosition());  // getting position at the beginning of the step
    hit->SetEDep(aStep->GetTotalEnergyDeposit());  // getting total energy deposited in the volume in the step 	
    fVolumeTrackingHitsCollection->insert(hit);
    return true;
}

void VolumeTrackingSD::Initialize(G4HCofThisEvent* hcof)
{
    fVolumeTrackingHitsCollection = new VolumeTrackingHitsCollection(SensitiveDetectorName, collectionName[0]);
    if (fVolumeTrackingHitsCollectionId < 0)
    {fVolumeTrackingHitsCollectionId = G4SDManager::GetSDMpointer()->GetCollectionID(GetName() + "/" + collectionName[0]);}
    hcof->AddHitsCollection(fVolumeTrackingHitsCollectionId, fVolumeTrackingHitsCollection);
}

// ============================================================
// ============================================================
// implement custom sensitive detector classes here
