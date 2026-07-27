#include <G4SystemOfUnits.hh>
#include <G4SDManager.hh>
#include <G4THitsMap.hh>
#include <G4Event.hh>

#include <string>

#include "EventAction.hh"
#include "Analysis.hh"
#include "CustomHit.hh"

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

    ///////////////////
    //// FZU stack ////

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

    //// FZU stack ////
    ///////////////////

    ////////////////////////////////////
    //// CERN stack - trigger tiles ////

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

    //// CERN stack - trigger tiles ////
    ////////////////////////////////////

    ////////////////////
    //// CERN stack ////

    if (TILECERN_B_ANY) {
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
