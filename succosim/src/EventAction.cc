#include <G4SystemOfUnits.hh>
#include <G4SDManager.hh>
#include <G4THitsMap.hh>
#include <G4Event.hh>

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

    // G4int fIdETileTest000 = sdm->GetCollectionID("tile000_SD/VolumeEDep");

    // VolumeEDepHitsCollection* hitCollectionTileTest000 = dynamic_cast<VolumeEDepHitsCollection*>(hcofEvent->GetHC(fIdETileTest000));

    int colNEv = 0;
    int colETileTest000 = 1;

    G4double E000 = 0.0;

    analysis->FillNtupleDColumn(0, colNEv, event->GetEventID());
    // if (hitCollectionTileTest000)
    // {
    //     for (auto hit: *hitCollectionTileTest000->GetVector())
    //     {E000 += hit->GetEDep();}
    //     analysis->FillNtupleDColumn(0, colETileTest000, E000 / MeV);
    // }else{analysis->FillNtupleDColumn(0, colETileTest000, -1.0);}

    analysis->AddNtupleRow(0);

    // --------------------------------------------------
    // ...uncomment this line for the test event action (implemented in src/TestMode.cc)
    //EndOfEventScoringTest(event, sdm, hcofEvent, analysis);
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}
