#ifndef DETECTOR_CONSTRUCTION_HH
#define DETECTOR_CONSTRUCTION_HH

#include <G4SystemOfUnits.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4NistManager.hh>
#include <G4SDManager.hh>

using namespace std;

// class for logical volumes
class G4LogicalVolume;

// DetectorConstruction, i.e. the class with all the setup info (physical objects, detectors, magnetic fields)
class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

private:
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // define custom methods here
    // e.g. void ConstructCalo(G4LogicalVolume* worldLog);
	
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
    // test setup (implemented in src/TestMode.cc)
    G4VPhysicalVolume* SetupTest(G4NistManager* nist);
    void SDTest(G4SDManager* sdm);
};

#endif
