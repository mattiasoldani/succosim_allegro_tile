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
    G4bool BPlaceOnlyInner() const { return b_place_only_inner; }
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
    G4int coarse_ro = 2;
    G4bool b_place_only_inner = false;
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    G4GenericMessenger* fMessenger;
};

#endif
