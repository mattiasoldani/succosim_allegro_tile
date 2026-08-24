#include <G4SystemOfUnits.hh>
#include <G4SDManager.hh>
#include <G4THitsMap.hh>
#include <G4Event.hh>
#include <G4PrimaryParticle.hh>
#include <G4PrimaryVertex.hh>
#include <G4ThreeVector.hh>

#include <cmath>
#include <string>

#include "EventAction.hh"
#include "Analysis.hh"
#include "CustomHit.hh"
#include <G4RunManager.hh>
#include "DetectorConstruction.hh"

using namespace std;

// EventAction::EndOfEventAction, executed at the end of each event
void EventAction::EndOfEventAction(const G4Event* event)
{
    // load the sensitive detector manager (set verbosity in DetectorConstruction.cc)
    G4SDManager* sdm = G4SDManager::GetSDMpointer();
	
    // load the analysis manager for data output (set verbosity in RunAction.cc)
    G4AnalysisManager* analysis = G4AnalysisManager::Instance();

    // get the set of all the data collections for the current event
    G4HCofThisEvent* hcofEvent = event->GetHCofThisEvent();
    if(!hcofEvent) return;
	
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // implement cast of the data collections, operations on them and ntuple filling here, or... 

    int itemp = 0;

    analysis->FillNtupleDColumn(0, itemp, event->GetEventID());
    itemp++;

    auto det = static_cast<const DetectorConstruction*>(
        G4RunManager::GetRunManager()->GetUserDetectorConstruction());
    if(!det) return;

    ///////////////////////////
    //// true primary info ////
    const G4PrimaryVertex* primaryVertex = event->GetPrimaryVertex(0);
    const G4PrimaryParticle* primaryParticle = primaryVertex ? primaryVertex->GetPrimary(0) : nullptr;
    if (primaryVertex && primaryParticle) {
        const G4ThreeVector momentumDirection = primaryParticle->GetMomentumDirection();
        analysis->FillNtupleDColumn(0, itemp, primaryParticle->GetKineticEnergy() / MeV); itemp++;
        analysis->FillNtupleDColumn(0, itemp, primaryVertex->GetX0() / mm); itemp++;
        analysis->FillNtupleDColumn(0, itemp, primaryVertex->GetY0() / mm); itemp++;
        analysis->FillNtupleDColumn(0, itemp, std::atan2(momentumDirection.x(), momentumDirection.z())); itemp++;
        analysis->FillNtupleDColumn(0, itemp, std::atan2(momentumDirection.y(), momentumDirection.z())); itemp++;
    } else {
        analysis->FillNtupleDColumn(0, itemp, -9999.0); itemp++;
        analysis->FillNtupleDColumn(0, itemp, -9999.0); itemp++;
        analysis->FillNtupleDColumn(0, itemp, -9999.0); itemp++;
        analysis->FillNtupleDColumn(0, itemp, -9999.0); itemp++;
        analysis->FillNtupleDColumn(0, itemp, -9999.0); itemp++;
    }
    //// true primary info ////
    ///////////////////////////

    ////////////////////////////
    //// beamline hodoscope ////
    if (det->IsHodo() && B_HODO_DET) {
        G4double thresholdHodoEDep = 50 * keV;

        G4int fHodo = sdm->GetCollectionID("SD_hodo/VolumeTracking");
        VolumeTrackingHitsCollection* hitCollectionHodo = fHodo >= 0 ? dynamic_cast<VolumeTrackingHitsCollection*>(hcofEvent->GetHC(fHodo)) : nullptr;

        if (hitCollectionHodo){
            G4int lastTrackId = -1;
            G4int NStep = 1;
            G4int NHits = 0;
            G4double horsa = -9999.0*mm;
            G4double versa = -9999.0*mm;
            for (auto hit: *hitCollectionHodo->GetVector()){
                if (hit->GetEDep()>thresholdHodoEDep){
                    if(hit->GetTrackId() != lastTrackId){
                        NHits+=1;
                        NStep=1;
                        horsa=hit->GetX()[0];
                        versa=hit->GetX()[1];
                    }else{
                        NStep+=1;
                        horsa+=hit->GetX()[0];
                        versa+=hit->GetX()[1];
                    }
                    lastTrackId = hit->GetTrackId();
                }
            }
            if (NHits > 0) {
                horsa = horsa / NStep;
                versa = versa / NStep;
            }
            analysis->FillNtupleDColumn(0, itemp, NHits); itemp++;
            analysis->FillNtupleDColumn(0, itemp, horsa / mm); itemp++;
            analysis->FillNtupleDColumn(0, itemp, versa / mm); itemp++;
        }else{
            analysis->FillNtupleDColumn(0, itemp, 0); itemp++;
            analysis->FillNtupleDColumn(0, itemp, -9999.0 / mm); itemp++;
            analysis->FillNtupleDColumn(0, itemp, -9999.0 / mm); itemp++;
        }
    }else{
        analysis->FillNtupleDColumn(0, itemp, 0); itemp++;
        analysis->FillNtupleDColumn(0, itemp, -9999.0 / mm); itemp++;
        analysis->FillNtupleDColumn(0, itemp, -9999.0 / mm); itemp++;
    }
    //// beamline hodoscope ////
    ////////////////////////////

    ///////////////////////
    //// Pb glass calo ////
    if (det->IsPbGl()) {
        G4int fIdEPbGl = sdm->GetCollectionID("SD_PbGl/VolumeEDep");
        VolumeEDepHitsCollection* hitCollectionPbGl = fIdEPbGl >= 0 ? dynamic_cast<VolumeEDepHitsCollection*>(hcofEvent->GetHC(fIdEPbGl)) : nullptr;

        G4double ePbGl = 0.0;
        if (hitCollectionPbGl)
        {
            for (auto hit: *hitCollectionPbGl->GetVector())
            {ePbGl += hit->GetEDep();}
            analysis->FillNtupleDColumn(0, itemp, ePbGl / MeV);
        }else{analysis->FillNtupleDColumn(0, itemp, -1.0);}
        itemp++;
    } else {
        analysis->FillNtupleDColumn(0, itemp, -1.0);
        itemp++;
    }
    //// Pb glass calo ////
    ///////////////////////

    ////////////////////////////
    //// scintillating pads ////
    for (G4int i = 0; i < 4; i++) {
        const G4bool isScinti = (i < 2) ? (det->IsScintiSmall() && B_SCINTISMALL_DET) : det->IsScintiBig();
        if (isScinti) {
            G4int fIdEScinti = sdm->GetCollectionID("SD_scinti" + std::to_string(i) + "/VolumeEDep");
            VolumeEDepHitsCollection* hitCollectionScinti = fIdEScinti >= 0 ? dynamic_cast<VolumeEDepHitsCollection*>(hcofEvent->GetHC(fIdEScinti)) : nullptr;

            G4double eScinti = 0.0;
            if (hitCollectionScinti)
            {
                for (auto hit: *hitCollectionScinti->GetVector())
                {eScinti += hit->GetEDep();}
                analysis->FillNtupleDColumn(0, itemp, eScinti / MeV);
            }else{analysis->FillNtupleDColumn(0, itemp, -1.0);}
        } else {
            analysis->FillNtupleDColumn(0, itemp, -1.0);
        }
        itemp++;
    }
    //// scintillating pads ////
    ////////////////////////////

    /////////////////////////
    //// Cherenkov pipes ////
    if (det->IsCher() && B_CHER_DET) {
        for (G4int i = 0; i < 2; i++) {
            G4int fIdECher = sdm->GetCollectionID("SD_cher" + std::to_string(i) + "/VolumeEDep");
            VolumeEDepHitsCollection* hitCollectionCher = fIdECher >= 0 ? dynamic_cast<VolumeEDepHitsCollection*>(hcofEvent->GetHC(fIdECher)) : nullptr;

            G4double eCher = 0.0;
            if (hitCollectionCher)
            {
                for (auto hit: *hitCollectionCher->GetVector())
                {eCher += hit->GetEDep();}
                analysis->FillNtupleDColumn(0, itemp, eCher / MeV);
            }else{analysis->FillNtupleDColumn(0, itemp, -1.0);}
            itemp++;
        }
    } else {
        for (G4int i = 0; i < 2; i++) {
            analysis->FillNtupleDColumn(0, itemp, -1.0);
            itemp++;
        }
    }
    //// Cherenkov pipes ////
    /////////////////////////

    ///////////////////
    //// FZU stack ////
    if (det->IsFZU()) {
        for (G4int i = 0; i < TILEFZU_N; i++) {
            G4int fIdEFZU = sdm->GetCollectionID("SD_FZU" + std::to_string(i) + "/VolumeEDep");
            VolumeEDepHitsCollection* hitCollectionFZU = fIdEFZU >= 0 ? dynamic_cast<VolumeEDepHitsCollection*>(hcofEvent->GetHC(fIdEFZU)) : nullptr;

            G4double eFZU = 0.0;
            if (hitCollectionFZU)
            {
                for (auto hit: *hitCollectionFZU->GetVector())
                {eFZU += hit->GetEDep();}
                analysis->FillNtupleDColumn(0, itemp, eFZU / MeV);
            }else{analysis->FillNtupleDColumn(0, itemp, -1.0);}
            itemp++;
        }
    } else {
        for (G4int i = 0; i < TILEFZU_N; i++) {
            analysis->FillNtupleDColumn(0, itemp, -1.0);
            itemp++;
        }
    }
    //// FZU stack ////
    ///////////////////

    ////////////////////////////////////
    //// CERN stack - trigger tiles ////
    if (det->IsCERNTrig()) {
        for (G4int i = 0; i < 2; i++) {
            G4int fIdETileCernTrigger = sdm->GetCollectionID("SD_tileCernTrigger" + std::to_string(i) + "/VolumeEDep");
            VolumeEDepHitsCollection* hitCollectionTileCernTrigger = fIdETileCernTrigger >= 0 ? dynamic_cast<VolumeEDepHitsCollection*>(hcofEvent->GetHC(fIdETileCernTrigger)) : nullptr;

            G4double eTileCernTrigger = 0.0;
            if (hitCollectionTileCernTrigger)
            {
                for (auto hit: *hitCollectionTileCernTrigger->GetVector())
                {eTileCernTrigger += hit->GetEDep();}
                analysis->FillNtupleDColumn(0, itemp, eTileCernTrigger / MeV);
            }else{analysis->FillNtupleDColumn(0, itemp, -1.0);}
            itemp++;
        }
    } else {
        for (G4int i = 0; i < 2; i++) {
            analysis->FillNtupleDColumn(0, itemp, -1.0);
            itemp++;
        }
    }
    //// CERN stack - trigger tiles ////
    ////////////////////////////////////

    ////////////////////
    //// CERN stack ////
    if (det->IsCERNAny()) {
        for (G4int i = 0; i < TILECERN_N; i++) {
            G4int fIdETileCern = sdm->GetCollectionID("SD_tileCern" + std::to_string(i) + "/VolumeEDep");
            VolumeEDepHitsCollection* hitCollectionTileCern = fIdETileCern >= 0 ? dynamic_cast<VolumeEDepHitsCollection*>(hcofEvent->GetHC(fIdETileCern)) : nullptr;

            G4double eTileCern = 0.0;
            if (hitCollectionTileCern)
            {
                for (auto hit: *hitCollectionTileCern->GetVector())
                {eTileCern += hit->GetEDep();}
                analysis->FillNtupleDColumn(0, itemp, eTileCern / MeV);
            }else{analysis->FillNtupleDColumn(0, itemp, -1.0);}
            itemp++;
        }
    } else {
        for (G4int i = 0; i < TILECERN_N; i++) {
            analysis->FillNtupleDColumn(0, itemp, -1.0);
            itemp++;
        }
    }
    //// CERN stack ////
    ////////////////////

    analysis->AddNtupleRow(0);

    // --------------------------------------------------
    // ...uncomment this line for the test event action (implemented in src/TestMode.cc)
    //EndOfEventScoringTest(event, sdm, hcofEvent, analysis);
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}
