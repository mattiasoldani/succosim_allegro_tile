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

    const G4int n_periods = CustomMessenger::Instance()->NPeriods();
    const G4int n_layers = CustomMessenger::Instance()->NLayers();
    const G4int n_stacked_mods = CustomMessenger::Instance()->NStackedMods();
    const G4int coarse_ro = CustomMessenger::Instance()->CoarseRO();
    const G4bool b_scinti_ro_only = coarse_ro == 1;
    const G4bool b_cell_ro = coarse_ro == 2;
    const G4bool b_total_ro = coarse_ro == 3;

    // function to extract hits
    auto GetVolumeEDep = [sdm, hcofEvent](const G4String& volumeName, const G4double missingValue) {
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
            return eDep;
        }

        return missingValue;
    };

    // function to extract hits and fill the corresponding ntuple column
    auto FillVolumeEDep = [analysis, GetVolumeEDep](G4int& col, const G4String& volumeName) {
        G4double eDep = GetVolumeEDep(volumeName, -1. * GeV);
        analysis->FillNtupleDColumn(0, col, eDep / GeV);
        col++;
    };

    auto GetEntryKineticEnergy = [sdm, hcofEvent](const G4String& volumeName, const G4double missingValue) {
        G4double eKin = 0.;
        G4int collectionId = sdm->GetCollectionID(volumeName + "_SD/EntryKineticEnergy");
        EntryKineticEnergyHitsCollection* hitCollection = nullptr;
        if (collectionId >= 0) {
            hitCollection = dynamic_cast<EntryKineticEnergyHitsCollection*>(hcofEvent->GetHC(collectionId));
        }

        if (hitCollection) {
            for (auto hit: *hitCollection->GetVector()) {
                eKin += hit->GetEKin();
            }
            return eKin;
        }

        return missingValue;
    };

    auto FillModuleEkinOut = [analysis, GetEntryKineticEnergy](G4int& col, const G4String& mod_prefix) {
        G4double eKinFront = GetEntryKineticEnergy(mod_prefix + "_CatcherFront", 0.);
        G4double eKinBack = GetEntryKineticEnergy(mod_prefix + "_CatcherBack", 0.);
        G4double eKinSide0 = GetEntryKineticEnergy(mod_prefix + "_CatcherSide0", 0.);
        G4double eKinSide1 = GetEntryKineticEnergy(mod_prefix + "_CatcherSide1", 0.);
        G4double eKinPhi0 = GetEntryKineticEnergy(mod_prefix + "_CatcherPhi0", 0.);
        G4double eKinPhi1 = GetEntryKineticEnergy(mod_prefix + "_CatcherPhi1", 0.);

        analysis->FillNtupleDColumn(0, col++, eKinFront / GeV);
        analysis->FillNtupleDColumn(0, col++, eKinBack / GeV);
        analysis->FillNtupleDColumn(0, col++, eKinSide0 / GeV);
        analysis->FillNtupleDColumn(0, col++, eKinSide1 / GeV);
        analysis->FillNtupleDColumn(0, col++, eKinPhi0 / GeV);
        analysis->FillNtupleDColumn(0, col++, eKinPhi1 / GeV);
    };

    // function to sum fine-readout hits for the inner module
    auto SumModuleEDep = [GetVolumeEDep, n_periods, n_layers, b_cell_ro, b_scinti_ro_only](const G4String& mod_prefix) {
        G4double eDep = 0.;

        if (b_cell_ro) {
            for (G4int j = 0; j < n_layers; j++) {
                for (G4int iperiod = 0; iperiod < n_periods; iperiod++) {
                    eDep += GetVolumeEDep(mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Cell", 0.);
                }
            }
            return eDep;
        }

        if (!b_scinti_ro_only) {
            for (G4int j = 0; j < n_layers; j++) {
                for (G4int i = 0; i < n_periods * 2 - 1; i++) {
                    G4int iperiod = floor(i/2);
                    eDep += GetVolumeEDep(mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Master" + std::to_string(i%2), 0.);
                }
            }
        }

        for (G4int j = 0; j < n_layers; j++) {
            for (G4int i = 0; i < n_periods * 2; i++) {
                G4int iperiod = floor(i/2);

                G4int b_spc = ((i%2) + (j%2)) % 2;
                if (b_spc) {
                    if (!b_scinti_ro_only) {
                        eDep += GetVolumeEDep(mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Spacer", 0.);
                    }
                } else {
                    for (G4int k = 0; k < 2; k++) {
                        eDep += GetVolumeEDep(mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Scintillator" + std::to_string(k), 0.);
                        if (!b_scinti_ro_only) {
                            eDep += GetVolumeEDep(mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Fibre" + std::to_string(k), 0.);
                        }
                    }
                }
            }
        }

        return eDep;
    };

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

    for (G4int imod = 0; imod < n_stacked_mods; imod++) {
        FillModuleEkinOut(col, "M" + std::to_string(imod));
    }

    for (G4int imod = 0; imod < n_stacked_mods; imod++) {
        G4String mod_prefix = "M" + std::to_string(imod);

        if (b_total_ro) {
            FillVolumeEDep(col, mod_prefix + "_Total");
            FillVolumeEDep(col, mod_prefix + "_Front");
            FillVolumeEDep(col, mod_prefix + "_Back");
            FillVolumeEDep(col, mod_prefix + "_Side0");
            FillVolumeEDep(col, mod_prefix + "_Side1");
            continue;
        }

        G4double moduleEDep = SumModuleEDep(mod_prefix);
        analysis->FillNtupleDColumn(0, col++, moduleEDep / GeV);

        FillVolumeEDep(col, mod_prefix + "_Front");
        FillVolumeEDep(col, mod_prefix + "_Back");
        FillVolumeEDep(col, mod_prefix + "_Side0");
        FillVolumeEDep(col, mod_prefix + "_Side1");

        if (b_cell_ro) {
            for (G4int j = 0; j < n_layers; j++) {
                for (G4int iperiod = 0; iperiod < n_periods; iperiod++) {
                    FillVolumeEDep(col, mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Cell");
                }
            }
            continue;
        }

        if (!b_scinti_ro_only) {
            for (G4int j = 0; j < n_layers; j++) {
                for (G4int i = 0; i < n_periods * 2 - 1; i++) {
                    G4int iperiod = floor(i/2);

                    FillVolumeEDep(col, mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Master" + std::to_string(i%2));
                }
            }
        }

        for (G4int j = 0; j < n_layers; j++) {
            for (G4int i = 0; i < n_periods * 2; i++) {
                G4int iperiod = floor(i/2);

                G4int b_spc = ((i%2) + (j%2)) % 2;
                if (b_spc) {
                    if (!b_scinti_ro_only) {
                        FillVolumeEDep(col, mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Spacer");
                    }
                } else {
                    for (G4int k = 0; k < 2; k++) {
                        FillVolumeEDep(col, mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Scintillator" + std::to_string(k));
                        if (!b_scinti_ro_only) {
                            FillVolumeEDep(col, mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Fibre" + std::to_string(k));
                        }
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
