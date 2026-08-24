#include <G4GenericMessenger.hh>
#include <G4StateManager.hh>
#include <G4ApplicationState.hh>

#include "CustomMessenger.hh"

CustomMessenger* CustomMessenger::Instance()
{
    static CustomMessenger instance;
    return &instance;
}

CustomMessenger::CustomMessenger()
{
    fMessenger = new G4GenericMessenger(this, "/custom/", "custom macro parameters");
    // usage: /custom/[PROPERTY] [VALUE] (before /run/initialize)

    fMessenger->DeclareProperty(
        "OutFileName", out_file_name,
        "output ROOT file basename (./out_data/<OutFileName>.root) - str"
    );

    fMessenger->DeclareProperty(
        "OutTreeName", out_tree_name,
        "output ROOT tree name - str"
    );

    fMessenger->DeclareProperty(
        "OutTreeTitle", out_tree_title,
        "output ROOT tree title - str"
    );

    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // implement custom macro parameter parser here

    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

}

CustomMessenger::~CustomMessenger(){delete fMessenger;}
