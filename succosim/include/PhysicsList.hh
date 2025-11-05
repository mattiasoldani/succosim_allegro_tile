#ifndef PHYSICS_LIST_HH
#define PHYSICS_LIST_HH

#include <G4SystemOfUnits.hh>
#include <G4VModularPhysicsList.hh>

// PhysicsList, the custom modular physics list
// note: this is completely overridden when using a preset physics list

class PhysicsList : public G4VModularPhysicsList
{
public:
    PhysicsList();
    ~PhysicsList(){;};
    void SetCuts ();
};

#endif
