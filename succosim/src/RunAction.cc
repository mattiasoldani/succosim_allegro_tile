#include <G4SystemOfUnits.hh>
#include <G4String.hh>

#include <string>

#include "RunAction.hh"
#include "Analysis.hh"

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


    analysis->CreateNtupleDColumn("NEvent");

    ///////////////////
    //// FZU stack ////

    for (G4int i = 0; i < TILEFZU_N; i++) {
        analysis->CreateNtupleDColumn("E_FZU" + std::to_string(i));
    }

    //// FZU stack ////
    ///////////////////

    ////////////////////////////////////
    //// CERN stack - trigger tiles ////

    for (G4int i = 0; i < 2; i++) {
        analysis->CreateNtupleDColumn("E_tileCernTrigger" + std::to_string(i));
    }

    //// CERN stack - trigger tiles ////
    ////////////////////////////////////

    ////////////////////
    //// CERN stack ////

    for (G4int i = 0; i < TILECERN_N; i++) {
        analysis->CreateNtupleDColumn("E_tileCern" + std::to_string(i));
    }

    //// CERN stack ////
    ////////////////////

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
