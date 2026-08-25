#include <G4SystemOfUnits.hh>
#include <G4String.hh>

#include <string>
#include <iomanip>
#include <sstream>

#include "RunAction.hh"
#include "Analysis.hh"

using namespace std;

// RunAction, class created at run start, with RunAction::EndOfRunAction executed at the end of the run

RunAction::RunAction() :  G4UserRunAction()
{
}

void RunAction::BeginOfRunAction(const G4Run*)
{
    G4AnalysisManager* analysis = G4AnalysisManager::Instance();
    analysis->SetNtupleMerging(true);
    analysis->SetVerboseLevel(0);  // set analysis manager verbosity here

    if (!b_ntuple_created) {
        analysis->SetFirstNtupleId(0);
        analysis->CreateNtuple(
            CustomMessenger::Instance()->OutTreeName(),
            CustomMessenger::Instance()->OutTreeTitle()
        );

        // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
        // create the ntuple columns (remember the order, it is needed to fill them) here, or...
        // e.g. analysis->CreateNtupleDColumn("NEvent");

        analysis->CreateNtupleDColumn("NEvent");

        ///////////////////////////
        //// true primary info ////
        analysis->CreateNtupleDColumn("true_PDG");
        analysis->CreateNtupleDColumn("true_KE");
        analysis->CreateNtupleDColumn("true_X");
        analysis->CreateNtupleDColumn("true_Y");
        analysis->CreateNtupleDColumn("true_thetaX");
        analysis->CreateNtupleDColumn("true_thetaY");
        //// true primary info ////
        ///////////////////////////

        const G4int n_periods = CustomMessenger::Instance()->NPeriods();
        const G4int n_layers = CustomMessenger::Instance()->NLayers();
        const G4int n_stacked_mods = CustomMessenger::Instance()->NStackedMods();
        const G4int coarse_ro = CustomMessenger::Instance()->CoarseRO();
        auto Id = [](const G4int id) {
            std::ostringstream stream;
            stream << std::setw(3) << std::setfill('0') << id;
            return G4String(stream.str());
        };

        for (G4int imod = 0; imod < n_stacked_mods; imod++) {
            G4String mod_prefix = "M" + Id(imod);

            analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_Total");

            if (coarse_ro == 2) {
                continue;
            }

            analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_Front");
            analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_Back");
            analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_Side" + Id(0));
            analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_Side" + Id(1));

            if (coarse_ro == 1) {
                for (G4int j = 0; j < n_layers; j++) {
                    for (G4int iperiod = 0; iperiod < n_periods; iperiod++) {
                        analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_L" + Id(j) + "_P" + Id(iperiod) + "_Cell");
                    }
                }
                continue;
            }

            for (G4int j = 0; j < n_layers; j++) {
                for (G4int i = 0; i < n_periods * 2 - 1; i++) {
                    G4int iperiod = floor(i/2);

                    analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_L" + Id(j) + "_P" + Id(iperiod) + "_Master" + Id(i%2));
                }
            }

            for (G4int j = 0; j < n_layers; j++) {
                for (G4int i = 0; i < n_periods * 2; i++) {
                    G4int iperiod = floor(i/2);

                    G4int b_spc = ((i%2) + (j%2)) % 2;
                    if (b_spc) {
                        analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_L" + Id(j) + "_P" + Id(iperiod) + "_Spacer");
                    } else {
                        for (G4int k = 0; k < 2; k++) {
                            analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_L" + Id(j) + "_P" + Id(iperiod) + "_Scintillator" + Id(k));
                            analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_L" + Id(j) + "_P" + Id(iperiod) + "_Fibre" + Id(k));
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
		
        analysis->FinishNtuple(0);
        b_ntuple_created = true;
    }

    // open output file - output file will have extension .root and custom path and name
    G4String outFileName = CustomMessenger::Instance()->OutFileName();
    G4String outFilePath = CustomMessenger::Instance()->OutFilePath();
    analysis->OpenFile(outFilePath+"/"+outFileName+".root");
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
