#include <G4SystemOfUnits.hh>
#include <G4ParticleTable.hh>
#include <G4Event.hh>
#include <G4ParticleGun.hh>
#include <Randomize.hh>
#include <math.h>

#include "PrimaryGeneratorAction.hh"

#include <G4GeneralParticleSource.hh>
#include <G4SPSAngDistribution.hh>
#include <G4SPSEneDistribution.hh>
#include <G4SPSPosDistribution.hh>

using namespace std;

// PrimaryGeneratorAction::PrimaryGeneratorAction, i.e. where the particle gun is defined and the beam features that are constant throughout the run are set
PrimaryGeneratorAction::PrimaryGeneratorAction()
{
    // define the particle gun
    fGun = new G4ParticleGun();
    fGPS = new G4GeneralParticleSource();
	
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // set the beam features that are constant throughout the run here, or...

    // default particle: 1-GeV geantino originating from the centre of the world and propagating towards positive z, no randomisation

    // particle type
    fGPS->SetParticleDefinition(G4ParticleTable::GetParticleTable()->FindParticle("geantino"));

    // energy
    G4SPSEneDistribution* fGPS_E = fGPS->GetCurrentSource()->GetEneDist();
    fGPS_E->SetEnergyDisType("Mono");
    fGPS_E->SetMonoEnergy(1 * GeV);

    // source position
    G4SPSPosDistribution* fGPS_Pos = fGPS->GetCurrentSource()->GetPosDist();
    fGPS_Pos->SetPosDisType("Point");
    fGPS_Pos->SetCentreCoords(G4ThreeVector(0., 0., 0.));

    // direction
    G4SPSAngDistribution* fGPS_Ang = fGPS->GetCurrentSource()->GetAngDist();
    fGPS_Ang->SetAngDistType("planar");
    fGPS_Ang->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));

    // --------------------------------------------------
    // ...uncomment this line for the test beam (implemented in src/TestMode.cc)
    //BeamFeaturesFxdTest(fGun);
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fGun;
    delete fGPS;
}

// PrimaryGeneratorAction::GeneratePrimaries, i.e. where the beam features that change at each event are set and the particles are shot
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // set the beam features that change at each event here, or...

    // default particle: 1-GeV geantino originating from the centre of the world and propagating towards positive z, no randomisation

    // --------------------------------------------------
    // ...uncomment this line for the test beam (implemented in src/TestMode.cc)
    //BeamFeaturesRndTest(fGun);
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
    // shot the event primary particle
    fGPS->GeneratePrimaryVertex(anEvent);
}
