#include <G4SystemOfUnits.hh>
#include <G4String.hh>
#include <G4RunManager.hh>

#include <string>
#include <iomanip>
#include <sstream>

#include "RunAction.hh"
#include "Analysis.hh"
#include "DetectorConstruction.hh"

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

        //// preliminary tests 01 ////

        analysis->CreateNtupleDColumn("NEvent");
        analysis->CreateNtupleDColumn("E_tileTest000");
        analysis->CreateNtupleDColumn("E_tileTest001");
        
        // --------------------------------------------------
        // ...uncomment this line for the test ntuple columns (implemented in src/TestMode.cc)
        //OutputNtupleTest(analysis);
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
