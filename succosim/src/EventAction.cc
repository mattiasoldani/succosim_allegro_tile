#include <G4SystemOfUnits.hh>
#include <G4SDManager.hh>
#include <G4THitsMap.hh>
#include <G4Event.hh>

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

    // function to extract hits and fill the corresponding ntuple column
    auto FillVolumeEDep = [sdm, hcofEvent, analysis](G4int& col, const G4String& volumeName) {
        G4double eDep = 0.;
        G4int collectionId = sdm->GetCollectionID(volumeName + "_SD/VolumeEDep");
        VolumeEDepHitsCollection* hitCollection = nullptr;
        if (collectionId >= 0) {
            hitCollection = dynamic_cast<VolumeEDepHitsCollection*>(hcofEvent->GetHC(collectionId));
        }

        if (hitCollection) {
            for (auto hit: *hitCollection->GetVector()) {
                eDep += hit->GetEDep();
            }
            analysis->FillNtupleDColumn(0, col, eDep / MeV);
        } else {
            analysis->FillNtupleDColumn(0, col, -1.);
        }
        col++;
    };

    G4int col = 0;

    analysis->FillNtupleDColumn(0, col++, event->GetEventID());

    const G4int n_periods = CustomMessenger::Instance()->nPeriods();
    const G4int n_layers = CustomMessenger::Instance()->nLayers();
    const G4int n_stacked_mods = CustomMessenger::Instance()->nStackedMods();

    for (G4int imod = 0; imod < n_stacked_mods; imod++) {
        G4String mod_prefix = "M" + std::to_string(imod);

        if (COARSERO == 2) {
            FillVolumeEDep(col, mod_prefix + "_Total");
            continue;
        }

        FillVolumeEDep(col, mod_prefix + "_Front");
        FillVolumeEDep(col, mod_prefix + "_Back");
        FillVolumeEDep(col, mod_prefix + "_Side0");
        FillVolumeEDep(col, mod_prefix + "_Side1");

        if (COARSERO == 1) {
            for (G4int j = 0; j < n_layers; j++) {
                for (G4int iperiod = 0; iperiod < n_periods; iperiod++) {
                    FillVolumeEDep(col, mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Cell");
                }
            }
            continue;
        }

        for (G4int j = 0; j < n_layers; j++) {
            for (G4int i = 0; i < n_periods * 2 - 1; i++) {
                G4int iperiod = floor(i/2);

                FillVolumeEDep(col, mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Master" + std::to_string(i%2));
            }
        }

        for (G4int j = 0; j < n_layers; j++) {
            for (G4int i = 0; i < n_periods * 2; i++) {
                G4int iperiod = floor(i/2);

                G4int b_spc = ((i%2) + (j%2)) % 2;
                if (b_spc) {
                    FillVolumeEDep(col, mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Spacer");
                } else {
                    for (G4int k = 0; k < 2; k++) {
                        FillVolumeEDep(col, mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Scintillator" + std::to_string(k));
                        FillVolumeEDep(col, mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Fibre" + std::to_string(k));
                    }
                }
            }
        }
    }

    analysis->AddNtupleRow(0);

    // --------------------------------------------------
    // ...uncomment this line for the test event action (implemented in src/TestMode.cc)
    //EndOfEventScoringTest(event, sdm, hcofEvent, analysis);
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}
