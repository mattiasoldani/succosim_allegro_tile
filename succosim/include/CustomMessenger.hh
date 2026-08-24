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
    G4String OutTreeName() const { return out_tree_name; }
    G4String OutTreeTitle() const { return out_tree_title; }

    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // implement custom macro parameter value getters here
    G4int nPeriods() const { return n_periods; }
    G4int nLayers() const { return n_layers; }
    G4int nStackedMods() const { return n_stacked_mods; }
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

private:

    G4String out_file_name = "OutData";
    G4String out_tree_name = "outData";
    G4String out_tree_title = "output data";

    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // implement custom macro parameter internal variables here
    G4int n_periods = 13;
    G4int n_layers = 13;
    G4int n_stacked_mods = 1;
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    G4GenericMessenger* fMessenger;
};

#endif
