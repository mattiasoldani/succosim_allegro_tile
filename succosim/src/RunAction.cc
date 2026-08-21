#include <G4SystemOfUnits.hh>
#include <G4String.hh>

#include "RunAction.hh"
#include "Analysis.hh"
#include "DetectorConstruction.hh"

using namespace std;

// RunAction, class created at run start, with RunAction::EndOfRunAction executed at the end of the run

RunAction::RunAction() :  G4UserRunAction()
{
    // load the analysis manager for data output
    G4AnalysisManager* analysis = G4AnalysisManager::Instance();
    analysis->SetNtupleMerging(true);
    analysis->SetVerboseLevel(1);  // set analysis manager verbosity here
  
    // create output ntuple
    analysis->SetFirstNtupleId(0);
    analysis->CreateNtuple("outData", "output data");
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // create the ntuple columns (remember the order, it is needed to fill them) here, or...
    // e.g. analysis->CreateNtupleDColumn("NEvent");

    //// preliminary tests 01 ////

    analysis->CreateNtupleDColumn("NEvent");

    for (G4int imod = 0; imod < NSTACKEDMODS; imod++) {
        G4String mod_prefix = "M" + std::to_string(imod);

        if (COARSERO == 2) {
            analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_Total");
            continue;
        }

        analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_Front");
        analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_Back");
        analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_Side0");
        analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_Side1");

        if (COARSERO == 1) {
            for (G4int j = 0; j < NLAYERS; j++) {
                for (G4int iperiod = 0; iperiod < NPERIODS; iperiod++) {
                    analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Cell");
                }
            }
            continue;
        }

        for (G4int j = 0; j < NLAYERS; j++) {
            for (G4int i = 0; i < NPERIODS * 2 - 1; i++) {
                G4int iperiod = floor(i/2);

                analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Master" + std::to_string(i%2));
            }
        }

        for (G4int j = 0; j < NLAYERS; j++) {
            for (G4int i = 0; i < NPERIODS * 2; i++) {
                G4int iperiod = floor(i/2);

                G4int b_spc = ((i%2) + (j%2)) % 2;
                if (b_spc) {
                    analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Spacer");
                } else {
                    for (G4int k = 0; k < 2; k++) {
                        analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Scintillator" + std::to_string(k));
                        analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Fibre" + std::to_string(k));
                    }
                }
            }
        }
    }

    // --------------------------------------------------
    // ...uncomment this line for the test ntuple columns (implemented in src/TestMode.cc)
    //OutputNtupleTest(analysis);
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    analysis->FinishNtuple(0);
	
    // open output file
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // choose output file name here --> file will have extension .root and and will be in ./out_data/
    G4String outFileName = "OutData";
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    analysis->OpenFile("./out_data/"+outFileName+".root");
}

void RunAction::EndOfRunAction(const G4Run* run)
{
    // retrieve the number of events produced in the run
    G4int nofEvents = run->GetNumberOfEvent();
    if (nofEvents == 0) return;

    // write output file & close it
    G4AnalysisManager* analysis = G4AnalysisManager::Instance();
    analysis->Write();
    analysis->CloseFile();

    if (IsMaster())
    {
        G4cout << "-----" << G4endl;
        G4cout << "| RunAction.cc: end of run --> generated events: " << nofEvents << G4endl;
        G4cout << "-----" << G4endl;
    }
}
