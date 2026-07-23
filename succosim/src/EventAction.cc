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

    int colNEv = 0;
    analysis->FillNtupleDColumn(0, colNEv, event->GetEventID());

    const G4int tileCern_n = 10;
    for (G4int i = 0; i < tileCern_n; i++) {
        G4int fIdETile = sdm->GetCollectionID("SD_tileCern" + std::to_string(i) + "/VolumeEDep");
        VolumeEDepHitsCollection* hitCollectionTile = dynamic_cast<VolumeEDepHitsCollection*>(hcofEvent->GetHC(fIdETile));

        G4double eTile = 0.0;
        if (hitCollectionTile)
        {
            for (auto hit: *hitCollectionTile->GetVector())
            {eTile += hit->GetEDep();}
            analysis->FillNtupleDColumn(0, i + 1, eTile / MeV);
        }else{analysis->FillNtupleDColumn(0, i + 1, -1.0);}
    }

    analysis->AddNtupleRow(0);

    // --------------------------------------------------
    // ...uncomment this line for the test event action (implemented in src/TestMode.cc)
    //EndOfEventScoringTest(event, sdm, hcofEvent, analysis);
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}
