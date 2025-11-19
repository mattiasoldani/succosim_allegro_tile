#include <G4ProductionCutsTable.hh>

// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// add new physics list modules here
// e.g. #include <G4EmStandardPhysics.hh>
#include <G4EmStandardPhysics.hh>
#include <G4DecayPhysics.hh>
#include <G4EmExtraPhysics.hh>
#include <G4HadronElasticPhysics.hh>
#include <G4HadronPhysicsFTFP_BERT.hh>
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#include "PhysicsList.hh"

using namespace std;

// PhysicsList, the custom modular physics list
// note: this is completely overridden when using a preset physics list

PhysicsList::PhysicsList() : G4VModularPhysicsList()
{
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// register new physics list modules here: RegisterPhysics(new ...());
// e.g. RegisterPhysics(new G4EmStandardPhysics());

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}

void PhysicsList::SetCuts()
{
    G4VUserPhysicsList::SetCuts();
    DumpCutValuesTable();
}
