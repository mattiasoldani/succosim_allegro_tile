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

using namespace std;

// PrimaryGeneratorAction::PrimaryGeneratorAction, i.e. where the particle gun is defined and the beam features that are constant throughout the run are set
PrimaryGeneratorAction::PrimaryGeneratorAction()
{
    // define the particle gun
    //fGun = new G4ParticleGun();
    fGPS = new G4GeneralParticleSource();
	
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // set the beam features that are constant throughout the run here, or...

    // particle type
    //fGun->SetParticleDefinition(G4ParticleTable::GetParticleTable()->FindParticle("e-"));
    fGPS->SetParticleDefinition(G4ParticleTable::GetParticleTable()->FindParticle("geantino"));

    // energy
    //fGun->SetParticleEnergy(1 * GeV);
    G4SPSEneDistribution* fGPS_E = fGPS->GetCurrentSource()->GetEneDist();
    fGPS_E->SetEnergyDisType("Mono");
    fGPS_E->SetMonoEnergy(1 * GeV);

    // --------------------------------------------------
    // ...uncomment this line for the test beam (implemented in src/TestMode.cc)
    //BeamFeaturesFxdTest(fGun);
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    //delete fGun;
    delete fGPS;
}

// PrimaryGeneratorAction::GeneratePrimaries, i.e. where the beam features that change at each event are set and the particles are shot
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // set the beam features that change at each event here, or...

    // beam source position - uniform square
    G4double zFixed = 0 * cm;
    G4double xRnd = (5*mm) * (G4UniformRand()-0.5);
    G4double yRnd = (5*mm) * (G4UniformRand()-0.5);
    //fGun->SetParticlePosition(G4ThreeVector(xRnd, yRnd, zFixed));
    fGPS->SetParticlePosition(G4ThreeVector(xRnd, yRnd, zFixed));

    // angle wrt the longitudinal axis - gaussian
    G4double thRnd = G4RandGauss::shoot(0, 0.001);
    G4double phiRnd = 2 * 3.1415926535 * G4UniformRand();
    //fGun->SetParticleMomentumDirection(G4ThreeVector(sin(thRnd)*cos(phiRnd), sin(thRnd)*sin(phiRnd), cos(thRnd)));
    fGPS->GetCurrentSource()->GetAngDist()->SetParticleMomentumDirection(G4ThreeVector(sin(thRnd)*cos(phiRnd), sin(thRnd)*sin(phiRnd), cos(thRnd)));

    // --------------------------------------------------
    // ...uncomment this line for the test beam (implemented in src/TestMode.cc)
    //BeamFeaturesRndTest(fGun);
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
    // shot the event primary particle
    //fGun->GeneratePrimaryVertex(anEvent);
    fGPS->GeneratePrimaryVertex(anEvent);
}
