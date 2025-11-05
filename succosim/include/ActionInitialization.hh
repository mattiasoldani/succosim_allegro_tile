#ifndef ACTION_INITIALIZATION_HH
#define ACTION_INITIALIZATION_HH

#include <G4SystemOfUnits.hh>
#include <G4VUserActionInitialization.hh>

// ActionInitialization, i.e. where the action classes are conjured

class ActionInitialization : public G4VUserActionInitialization
{
public:
    void Build() const override;
    void BuildForMaster() const override;   
};

#endif
