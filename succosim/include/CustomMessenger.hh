#ifndef CUSTOMMESSENGER_HH
#define CUSTOMMESSENGER_HH

#include <G4String.hh>
#include <globals.hh>

#include <memory>

class G4GenericMessenger;

class CustomMessenger
{
public:
    static CustomMessenger* Instance();

    CustomMessenger();
    ~CustomMessenger();

    G4String OutFileName() const { return out_file_name; }
    G4String OutFilePath() const { return out_file_path; }
    G4String OutTreeName() const { return out_tree_name; }
    G4String OutTreeTitle() const { return out_tree_title; }

    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // implement custom macro parameter value getters here
    G4int NPeriods() const { return n_periods; }
    G4int NLayers() const { return n_layers; }
    G4int NStackedMods() const { return n_stacked_mods; }
    G4int CoarseRO() const { return coarse_ro; }
    G4bool BPlaceSupport() const { return b_place_support; }
    G4bool BShowInner() const { return b_show_inner; }
    G4bool BShowSupport() const { return b_show_support; }
    G4bool BShowCatcherFrontBack() const { return b_show_catcher_front_back; }
    G4bool BShowCatcherSide() const { return b_show_catcher_side; }
    G4bool BShowCatcherPhi() const { return b_show_catcher_phi; }
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

private:

    G4String out_file_name = "temp";
    G4String out_file_path = "./out_data";
    G4String out_tree_name = "t";
    G4String out_tree_title = "output data";

    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // implement custom macro parameter internal variables here
    G4int n_periods = 13;
    G4int n_layers = 13;
    G4int n_stacked_mods = 1;
    G4int coarse_ro = 3;
    G4bool b_place_support = true;
    G4bool b_show_inner = true;
    G4bool b_show_support = true;
    G4bool b_show_catcher_front_back = false;
    G4bool b_show_catcher_side = false;
    G4bool b_show_catcher_phi = false;
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    G4GenericMessenger* fMessenger;
};

#endif
