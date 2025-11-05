#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "Analysis.hh"
#include "PhysicsList.hh"

// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// add new physics lists here
// e.g. #include <FTFP_BERT.hh>
#include <FTFP_BERT.hh>
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#include "G4RunManagerFactory.hh"
#include <G4VisExecutive.hh>
#include <G4UIExecutive.hh>
#include <G4String.hh>
#include <G4UImanager.hh>
#include <vector>

using namespace std;

int main(int argc, char** argv)
{
    cout << "-----" << endl;
    cout << "| main.cc: let's start!" << endl;
    cout << "-----" << endl;

    // find execution type (interactive or not) and detect all the macros given as executable arguments
    vector<G4String> macros;
    bool interactive = false;

    #ifdef G4MULTITHREADED
    G4int nThreads = 0;
    #endif

    if (argc == 1)
    {
        interactive = true;
    }
    else
    {
        for (int i = 1; i < argc; i++)
        {
            G4String arg = argv[i];
            if (arg == "-i" || arg == "--interactive")
            {
                interactive = true;
                continue;
            }
            else
            {
                macros.push_back(arg);
            }
        }
    }

    // load user interface manager and program executor
    G4UIExecutive* ui = nullptr;
    if (interactive)
    {
        ui = new G4UIExecutive(argc, argv);
    }
    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    // load run manager
    auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
    #ifdef G4MULTITHREADED
    if (nThreads > 0) {
        runManager->SetNumberOfThreads(nThreads);
    }
    #endif
    runManager->SetVerboseLevel(0);  // <<< set run manager verbosity here

    // load physics list, detector construction and action initialisation
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // set physics list name here: (new ...()), set PhysicsList for the custom list
    // e.g. runManager->SetUserInitialization(new FTFP_BERT());
    runManager->SetUserInitialization(new FTFP_BERT());
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    runManager->SetUserInitialization(new DetectorConstruction());
    runManager->SetUserInitialization(new ActionInitialization());

    // if in graphical mode, start visualisation
    auto visManager = new G4VisExecutive("Quiet");  // <<< set visualisation manager verbosity here
    visManager->SetVerboseLevel(0);  // <<< set visualisation manager verbosity here
    visManager->Initialize();

    // execute all the macros given as executable arguments
    for (auto macro : macros)
    {
        G4String command = "/control/execute ";
        UImanager->ApplyCommand(command + macro);
    }

    // if in interactive mode, open it; moreover, if in graphical mode, execute the default macro(s) for graphical mode startup
    if (interactive)
    {
        if (ui->IsGUI())
        {
            // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
            // select macro to be executed at graphical mode startup
            // e.g. UImanager->ApplyCommand("/control/execute macros/gui.mac");
            UImanager->ApplyCommand("/control/execute macros/gui.mac");
            // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
        }
        else
        {
            UImanager->ApplyCommand("/run/initialize");
        }
        ui->SessionStart();
        delete ui;
    }

    // at the end of the session, delete the graphical and run manager
    delete visManager;
    delete runManager;
	
    cout << "-----" << endl;
    cout << "| main.cc: done, goodbye!" << endl;
    cout << "-----" << endl;

    return 0;
}
