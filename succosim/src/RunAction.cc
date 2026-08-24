#include <G4SystemOfUnits.hh>
#include <G4String.hh>

#include <string>

#include "RunAction.hh"
#include "Analysis.hh"
#include "CustomMessenger.hh"

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
        analysis->CreateNtupleDColumn("true_KE");
        analysis->CreateNtupleDColumn("true_X");
        analysis->CreateNtupleDColumn("true_Y");
        analysis->CreateNtupleDColumn("true_thetaX");
        analysis->CreateNtupleDColumn("true_thetaY");
        //// true primary info ////
        ///////////////////////////

        ////////////////////////////
        //// beamline hodoscope ////
        analysis->CreateNtupleDColumn("hodo_N");
        analysis->CreateNtupleDColumn("hodo_X");
        analysis->CreateNtupleDColumn("hodo_Y");
        ////////////////////////////

        ///////////////////////
        //// Pb glass calo ////
        analysis->CreateNtupleDColumn("Edep_PbGl");
        //// Pb glass calo ////
        ///////////////////////

        ////////////////////////////
        //// scintillating pads ////
        for (G4int i = 0; i < 4; i++) {
            analysis->CreateNtupleDColumn("Edep_S" + std::to_string(i));
        }
        //// scintillating pads ////
        ////////////////////////////

        /////////////////////////
        //// Cherenkov pipes ////
        for (G4int i = 0; i < 2; i++) {
            analysis->CreateNtupleDColumn("Edep_cher_" + std::to_string(i));
        }
        //// Cherenkov pipes ////
        /////////////////////////

        ///////////////////
        //// FZU stack ////
        for (G4int i = 0; i < TILEFZU_N; i++) {
            analysis->CreateNtupleDColumn("Edep_FZU_" + std::to_string(i));
        }
        //// FZU stack ////
        ///////////////////

        ////////////////////////////////////
        //// CERN stack - trigger tiles ////
        for (G4int i = 0; i < 2; i++) {
            analysis->CreateNtupleDColumn("Edep_CERNTrig_" + std::to_string(i));
        }
        //// CERN stack - trigger tiles ////
        ////////////////////////////////////

        ////////////////////
        //// CERN stack ////
        for (G4int i = 0; i < TILECERN_N; i++) {
            analysis->CreateNtupleDColumn("Edep_CERN_" + std::to_string(i));
        }
        //// CERN stack ////
        ////////////////////

        // --------------------------------------------------
        // ...uncomment this line for the test ntuple columns (implemented in src/TestMode.cc)
        //OutputNtupleTest(analysis);
        // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
        
        analysis->FinishNtuple(0);
        b_ntuple_created = true;
    }

    // open output file - output file will have extension .root and and will be in ./out_data/
    G4String outFileName = CustomMessenger::Instance()->OutFileName();
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
