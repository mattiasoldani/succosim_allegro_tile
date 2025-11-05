#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"

// ActionInitialization, i.e. where the action classes are conjured

void ActionInitialization::Build() const
{
    SetUserAction(new PrimaryGeneratorAction());
    RunAction* theRunAction = new RunAction();
    SetUserAction(theRunAction);
    SetUserAction(new EventAction());
}

// just for multithreading
void ActionInitialization::BuildForMaster() const
{SetUserAction(new RunAction());}
