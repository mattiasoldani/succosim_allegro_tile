#ifndef PRIMARY_GENERATOR_ACTION_HH
#define PRIMARY_GENERATOR_ACTION_HH

#include <G4SystemOfUnits.hh>
#include <G4VUserPrimaryGeneratorAction.hh>

// class for particle gun, i.e. the beam source
class G4ParticleGun;

// PrimaryGeneratorAction, i.e. the class with all the beam features
class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction();
    void GeneratePrimaries(G4Event* anEvent) override;

private:
    G4ParticleGun* fGun;
	
    // test beam (implemented in src/TestMode.cc)
    void BeamFeaturesFxdTest(G4ParticleGun* fGunTest);
    void BeamFeaturesRndTest(G4ParticleGun* fGunTest);
};

#endif
