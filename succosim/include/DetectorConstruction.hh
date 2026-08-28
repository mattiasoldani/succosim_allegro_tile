#ifndef DETECTOR_CONSTRUCTION_HH
#define DETECTOR_CONSTRUCTION_HH

#include <G4SystemOfUnits.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4NistManager.hh>
#include <G4VisAttributes.hh>
#include <G4SDManager.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <G4Trd.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4IntersectionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4Vector3D.hh>
#include <G4Transform3D.hh>
#include <G4HCofThisEvent.hh>
#include <G4LogicalVolume.hh>

#include "Analysis.hh"
#include "CustomMessenger.hh"
#include "CustomSD.hh"

// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// define hard-coded parameters
// e.g. #define NLAYERS 10

// maximum numbers used for fixed-size array allocation
#define NPERIODSMAX 50
#define NLAYERSMAX 50
#define NSTACKEDMODSMAX 32

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

using namespace std;

// ubiquitous definition of pi
inline const G4double pi = CLHEP::pi;

// DetectorConstruction, i.e. the class with all the setup info (physical objects, detectors, magnetic fields)
class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // define custom public methods/members here
    // e.g. getters for private stuff

    //// tile-specific framework ////
    #include "DetectorConstruction_tile.hh"
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

private:
    // custom messenger
    CustomMessenger* custom = CustomMessenger::Instance();

    // NIST database
    G4NistManager* nist = G4NistManager::Instance();

    // default colors
    G4VisAttributes* invisible = new G4VisAttributes(false);
    G4VisAttributes* white = new G4VisAttributes(G4Colour::White());
    G4VisAttributes* cyan = new G4VisAttributes(G4Colour::Cyan());
    G4VisAttributes* blue = new G4VisAttributes(G4Colour::Blue());
    G4VisAttributes* red = new G4VisAttributes(G4Colour::Red());
    G4VisAttributes* green = new G4VisAttributes(G4Colour::Green());
    G4VisAttributes* magenta = new G4VisAttributes(G4Colour::Magenta());
	G4VisAttributes* grey = new G4VisAttributes(G4Colour::Grey());
    G4VisAttributes* brown = new G4VisAttributes(true, G4Colour::Brown());
    G4VisAttributes* yellow = new G4VisAttributes(true, G4Colour::Yellow());
    G4VisAttributes* black = new G4VisAttributes(true, G4Colour::Black());
	
   // default manual material: single elements
    G4Element* elC = new G4Element("Carbon", "C", 6., 12.01*g/mole);
    G4Element* elO  = new G4Element("Oxygen","O" , 8., 16.00*g/mole);
    G4Element* elH = new G4Element("Hydrogen", "H", 1., 1.0079*g/mole);

    // default off-the-shelf materials (from NIST)
    G4Material* air = nist->FindOrBuildMaterial("G4_AIR");
    G4Material* SS = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
    G4Material* plastic = nist->FindOrBuildMaterial("G4_POLYSTYRENE");
    G4Material* pbGl = nist->FindOrBuildMaterial("G4_GLASS_LEAD");
    G4Material* vacuum = nist->FindOrBuildMaterial("G4_Galactic");
    G4Material* aluminium = nist->FindOrBuildMaterial("G4_Al");
    G4Material* mylar = nist->FindOrBuildMaterial("G4_MYLAR");
    G4Material* lead = nist->FindOrBuildMaterial("G4_Pb"); 
    G4Material* silicon = nist->FindOrBuildMaterial("G4_Si"); 
    G4Material* iron = nist->FindOrBuildMaterial("G4_Fe");
    G4Material* tungsten = nist->FindOrBuildMaterial("G4_W");
    G4Material* co2 = new G4Material("CO2", 1.977*273.*mg/cm3/293., 2); // default manual material: CO2 - elements added in DetectorConstruction.cc
    G4Material* bc400 = new G4Material("BC400", 1.032*g/cm3, 2); // default manual material: BC400 (plastic scintillator) - elements added in DetectorConstruction.cc

	// generic translation and roto-translation, for element-by-element use
	G4ThreeVector pos_temp;
	G4Transform3D pos_rot_temp;

    // world size (full sides) and material - to be set here
    G4double worldSizeX = 10 * m;
    G4double worldSizeY = 10 * m;
    G4double worldSizeZ = 10 * m;
    G4Material* mat_world = vacuum;

    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // define custom private methods here
    // e.g. void ConstructCalo(G4LogicalVolume* worldLog);

    G4double zshift = 1*mm; // longitudinal displacement of the module wrt the beam source

    fullTileCalModule* modules[NSTACKEDMODSMAX] = {}; // array of full-module objects

    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
    // test setup (implemented in src/TestMode.cc)
    G4VPhysicalVolume* SetupTest(G4NistManager* nist);
    void SDTest(G4SDManager* sdm);
};

#endif
