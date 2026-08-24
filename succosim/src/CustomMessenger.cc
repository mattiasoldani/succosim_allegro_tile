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

    /* instructions for IdConfig:
    - 0 --> calibration mode
    - 1 --> physics mode for Bethe-Bloch
    - 2 --> physics mode with CERN S2
    - 3 --> physics mode with CERN S6 (no upstream steel)
    - 4 --> physics mode with CERN S6 (x1 upstream steel)
    - 5 --> physics mode with CERN S6 (x2 upstream steel)
    - 6 --> physics mode with CERN S6 (x4 upstream steel)
    */
    fMessenger->DeclareProperty(
        "IdConfig", id_config,
        "setup configuration - int: [0] calib, [1] Bethe-Bloch, [2] CERN S2, [3] CERN S6, [4/5/6] CERN S6 with 1/2/4 upstream steel layers"
    );

    fMessenger->DeclareProperty(
        "BPlaceUpstream", b_place_upstream,
        "place all detectors upstream of the hodoscope - true/false"
    );

    fMessenger->DeclareProperty(
        "BScintiSmallDet", b_scinti_small_det,
        "activate upstream scintillator (small; S0-1) detection - true/false"
    );

    fMessenger->DeclareProperty(
        "BCherDet", b_cher_det,
        "activate Cherenkov detection - true/false"
    );

    fMessenger->DeclareProperty(
        "BHodoDet", b_hodo_det,
        "activate hodoscope detection - true/false"
    );

    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

}

CustomMessenger::~CustomMessenger(){delete fMessenger;}
