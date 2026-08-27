#ifndef DETECTOR_CONSTRUCTION_HH
#define DETECTOR_CONSTRUCTION_HH

#include <G4SystemOfUnits.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4NistManager.hh>
#include <G4SDManager.hh>
#include <CLHEP/Units/PhysicalConstants.h>

#include <G4Trd.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4IntersectionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4Vector3D.hh>
#include <G4Transform3D.hh>

#include "CustomMessenger.hh"

// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// define hard-coded parameters
// e.g. #define NLAYERS 10

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

using namespace std;

inline const G4double pi = CLHEP::pi;

// class for logical volumes
class G4LogicalVolume;

// DetectorConstruction, i.e. the class with all the setup info (physical objects, detectors, magnetic fields)
class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // define custom public methods/members here
    // e.g. getters for private stuff

    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

private:
    // custom messenger
    CustomMessenger* custom = CustomMessenger::Instance();

    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // define custom methods here
    // e.g. void ConstructCalo(G4LogicalVolume* worldLog);

    //// tile-specific framework ////
    #include "DetectorConstruction_tile.hh"

    //// specific for this application ////

	// tile shape and orientation
    G4double height = 50*mm;
    G4double radius = 2.8*m + 10*mm + height/2;
    G4double angle = 2*pi/128;
    G4double thickness = 3*mm;
    G4double sidegap = 2*mm;
    G4double holeradius = 3*mm;
    G4double holey = height/2 - 6*mm;
    G4double holex = 6*mm;
    G4double fibreradius = 0.5*mm;
    G4double rot_x_ang = -90*deg;
    G4double rot_y_ang = 0*deg;
    G4double rot_z_ang = 0*deg;
	
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
    // test setup (implemented in src/TestMode.cc)
    G4VPhysicalVolume* SetupTest(G4NistManager* nist);
    void SDTest(G4SDManager* sdm);
};

#endif
