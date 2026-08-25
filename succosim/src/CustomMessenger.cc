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
        "output ROOT file basename - str (./<OutFilePath>/<OutFileName>.root)"
    );

    fMessenger->DeclareProperty(
        "OutFilePath", out_file_path,
        "output ROOT file path - str (./<OutFilePath>/<OutFileName>.root)"
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
    fMessenger->DeclareProperty(
        "NPeriods", n_periods,
        "nr. of periods - int"
    );

    fMessenger->DeclareProperty(
        "NLayers", n_layers,
        "nr. of layers - int"
    );

    fMessenger->DeclareProperty(
        "NStackedMods", n_stacked_mods,
        "nr. of modules stacked on top of each other beamtest style - int"
    );

    // readout granularity of each module:
    // - 0: single-element readout
    // - 1: one readout channel per cell for the inner structure, support read out separately
    // - 2: just the total energy in the module (always available anyway)
    fMessenger->DeclareProperty(
        "CoarseRO", coarse_ro,
        "readout granularity: 0 single-element, 1 cell, 2 full-module total - int"
    );

    fMessenger->DeclareProperty(
        "BPlaceOnlyInner", b_place_only_inner,
        "if true, place only inner volumes and skip support - bool"
    );
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

}

CustomMessenger::~CustomMessenger(){delete fMessenger;}
