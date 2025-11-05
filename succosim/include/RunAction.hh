#ifndef RUNACTION_HH
#define RUNACTION_HH

#include <G4SystemOfUnits.hh>
#include <G4UserRunAction.hh>
#include <G4Run.hh>

#include "Analysis.hh"

// RunAction, actions executed at each run

class RunAction : public G4UserRunAction
{
public:
    RunAction();
    ~RunAction() override = default;
    void EndOfRunAction(const G4Run*) override;
  
private:
    // event-by-event scoring for test simulation (implemented in src/TestMode.cc)
    void OutputNtupleTest(G4AnalysisManager* analysis);
};

#endif
