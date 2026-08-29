#include <G4SystemOfUnits.hh>
#include <G4SDManager.hh>
#include <G4THitsMap.hh>
#include <G4Event.hh>
#include <G4PrimaryParticle.hh>
#include <G4PrimaryVertex.hh>
#include <G4ThreeVector.hh>
#include <G4RunManager.hh>

#include <cmath>
#include <string>

#include "EventAction.hh"
#include "Analysis.hh"
#include "CustomHit.hh"
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

    G4int col = 0;

    analysis->FillNtupleDColumn(0, col++, event->GetEventID());

    ///////////////////////////
    //// true primary info ////
    const G4PrimaryVertex* primaryVertex = event->GetPrimaryVertex(0);
    const G4PrimaryParticle* primaryParticle = primaryVertex ? primaryVertex->GetPrimary(0) : nullptr;
    if (primaryVertex && primaryParticle) {
        const G4ThreeVector momentumDirection = primaryParticle->GetMomentumDirection();
        analysis->FillNtupleDColumn(0, col, primaryParticle->GetPDGcode()); col++;
        analysis->FillNtupleDColumn(0, col, primaryParticle->GetKineticEnergy() / GeV); col++;
        analysis->FillNtupleDColumn(0, col, primaryVertex->GetX0() / mm); col++;
        analysis->FillNtupleDColumn(0, col, primaryVertex->GetY0() / mm); col++;
        analysis->FillNtupleDColumn(0, col, std::atan2(momentumDirection.x(), momentumDirection.z())); col++;
        analysis->FillNtupleDColumn(0, col, std::atan2(momentumDirection.y(), momentumDirection.z())); col++;
    } else {
        analysis->FillNtupleDColumn(0, col, -9999.0); col++;
        analysis->FillNtupleDColumn(0, col, -9999.0); col++;
        analysis->FillNtupleDColumn(0, col, -9999.0); col++;
        analysis->FillNtupleDColumn(0, col, -9999.0); col++;
        analysis->FillNtupleDColumn(0, col, -9999.0); col++;
        analysis->FillNtupleDColumn(0, col, -9999.0); col++;
    }
    //// true primary info ////
    ///////////////////////////

    // module-related output data
    G4int n_stacked_mods = CustomMessenger::Instance()->NStackedMods();
    for (G4int i = 0; i < n_stacked_mods; i++) {
        DetectorConstruction::fullTileCalModule* module = DetectorConstruction::fullTileCalModule::GetModule(i);
        if (!module) {continue;}
        col = module->FillNtupleColumns(analysis, sdm, hcofEvent, col);
    }

    analysis->AddNtupleRow(0);

    // --------------------------------------------------
    // ...uncomment this line for the test event action (implemented in src/TestMode.cc)
    //EndOfEventScoringTest(event, sdm, hcofEvent, analysis);
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}
