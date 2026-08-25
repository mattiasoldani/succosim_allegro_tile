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
    G4int IdConfig() const { return id_config; }

    G4bool BPlaceUpstream() const { return b_place_upstream; }
    G4bool BScintiSmallDet() const { return b_scinti_small_det; }
    G4bool BCherDet() const { return b_cher_det; }

    G4bool BHodoDet() const { return b_hodo_det; }
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

private:

    G4String out_file_name = "temp";
    G4String out_file_path = "./out_data";
    G4String out_tree_name = "t";
    G4String out_tree_title = "output data";

    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // implement custom macro parameter internal variables here
    G4int id_config = 2;
    G4bool b_place_upstream = false;
    G4bool b_scinti_small_det = false;
    G4bool b_cher_det = false;
    G4bool b_hodo_det = true;
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    G4GenericMessenger* fMessenger;
};

#endif
