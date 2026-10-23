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
        "output ROOT file basename - str (<OutFilePath>/<OutFileName>.root)"
    );

    fMessenger->DeclareProperty(
        "OutFilePath", out_file_path,
        "output ROOT file path - str (<OutFilePath>/<OutFileName>.root)"
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

    fMessenger->DeclarePropertyWithUnit(
        "StackRotationY", "deg", stack_rotation_y,
        "whole-stack rotation about world y through the envelope centre; positive angles turn +z towards +x - deg"
    ).SetStates(G4State_PreInit);

    fMessenger->DeclarePropertyWithUnit(
        "StackTranslationX", "mm", stack_translation_x,
        "whole-stack translation along world x, applied after StackRotationY - mm"
    ).SetStates(G4State_PreInit);

    fMessenger->DeclarePropertyWithUnit(
        "StackTranslationY", "mm", stack_translation_y,
        "whole-stack translation along world y, applied after StackTranslationX - mm"
    ).SetStates(G4State_PreInit);

    fMessenger->DeclarePropertyWithUnit(
        "StackTranslationZ", "m", stack_translation_z,
        "additional whole-stack shift along world z, applied after StackTranslationY; positive values move downstream - m"
    ).SetStates(G4State_PreInit);

    // readout granularity of each module:
    // - 0: single-element readout, support read out separately
    // - 1: scintillator-only single-element readout, support read out separately
    // - 2: one readout channel per cell for the inner structure, support read out separately
    // - 3: just the total energy in the inner structure, support read out separately
    fMessenger->DeclareProperty(
        "CoarseRO", coarse_ro,
        "readout granularity: 0 single-element, 1 scintillator-only, 2 cell, 3 full-module total - int"
    );

    fMessenger->DeclareProperty(
        "BPlaceSupport", b_place_support,
        "if true, place support volumes - bool"
    );

    fMessenger->DeclareProperty(
        "BShowInner", b_show_inner,
        "if true, show inner volumes in graphical mode - bool"
    );

    fMessenger->DeclareProperty(
        "BShowSupport", b_show_support,
        "if true, show support volumes in graphical mode - bool"
    );

    fMessenger->DeclareProperty(
        "BShowCatcherFrontBack", b_show_catcher_front_back,
        "if true, show front/back catcher volumes in graphical mode - bool"
    );

    fMessenger->DeclareProperty(
        "BShowCatcherSide", b_show_catcher_side,
        "if true, show side catcher volumes in graphical mode - bool"
    );

    fMessenger->DeclareProperty(
        "BShowCatcherPhi", b_show_catcher_phi,
        "if true, show azimuthal catcher volumes in graphical mode - bool"
    );
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

}

CustomMessenger::~CustomMessenger(){delete fMessenger;}
