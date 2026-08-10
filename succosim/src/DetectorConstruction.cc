#include <G4SystemOfUnits.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4NistManager.hh>
#include <G4SystemOfUnits.hh>
#include <G4VisAttributes.hh>
#include <G4SDManager.hh>
#include <G4LogicalVolumeStore.hh>
#include <G4UniformMagField.hh>
#include <G4FieldManager.hh>
#include <G4TransportationManager.hh>
#include <G4ChordFinder.hh>
#include <G4MultiFunctionalDetector.hh>
#include <G4Box.hh>
#include <G4Trd.hh>

#include "DetectorConstruction.hh"
#include "CustomSD.hh"

// DetectorConstruction::Construct, i.e. where the setup geometry is implemented
G4VPhysicalVolume* DetectorConstruction::Construct()
{
    // NIST database
    G4NistManager* nist = G4NistManager::Instance();
	
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // define the world and all the setup stuff (materials, volumes) here, or...
	
    // colors
    G4VisAttributes* invisible = new G4VisAttributes(false);
    G4VisAttributes* cyan = new G4VisAttributes(G4Colour::Cyan());
    G4VisAttributes* blue = new G4VisAttributes(G4Colour::Blue());
    G4VisAttributes* red = new G4VisAttributes(G4Colour::Red());
    G4VisAttributes* green = new G4VisAttributes(G4Colour::Green());
    G4VisAttributes* magenta = new G4VisAttributes(G4Colour::Magenta());
	G4VisAttributes* grey = new G4VisAttributes(G4Colour::Grey());
    G4VisAttributes* brown = new G4VisAttributes(true, G4Colour::Brown());
	
    // off-the-shelf materials (from NIST)
    G4Material* air = nist->FindOrBuildMaterial("G4_AIR"); // air
	G4Material* SS = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL"); // steel
	G4Material* plastic = nist->FindOrBuildMaterial("G4_POLYSTYRENE"); // plastic for scintillating tiles
	G4Material* plastic_fibre = nist->FindOrBuildMaterial("G4_POLYSTYRENE"); // plastic for WLS fibres
	G4Material* pbGl = nist->FindOrBuildMaterial("G4_GLASS_LEAD"); // Pb glass
    G4Material* vacuum = nist->FindOrBuildMaterial("G4_Galactic"); // vacuum
	
    // world
    G4RotationMatrix* worldRotation = new G4RotationMatrix();
    G4VSolid* worldBox = new G4Box("world_Shape", worldSizeX / 2, worldSizeY / 2, worldSizeZ / 2);
    G4LogicalVolume* worldLog = new G4LogicalVolume(worldBox, air, "world_Log");
    G4VisAttributes* visAttrWorld = new G4VisAttributes();
    worldLog->SetVisAttributes(invisible);
    G4VPhysicalVolume* worldPhys = new G4PVPlacement(nullptr, {}, worldLog, "world", nullptr, false, 0);
	
	// generic translation and roto-translation, to be applied element-by-element
	G4ThreeVector pos_temp;
	G4Transform3D pos_rot_temp;

    ////////////////////////////////////////////////////////////////////////////////////////
    //// prototype /////////////////////////////////////////////////////////////////////////

    

    //// prototype /////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    // --------------------------------------------------
    // ...uncomment this line for the test setup (implemented in src/TestMode.cc) 
    //G4VPhysicalVolume* worldPhys = SetupTest(nist);
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
    // print list of defined material
    G4cout << "-----" << G4endl;
    G4cout << "| DetectorConstruction.cc: material list" << G4endl;
    G4cout << *(G4Material::GetMaterialTable()) << G4endl;
    G4cout << "-----" << G4endl;
	
    return worldPhys;
}

// DetectorConstruction::ConstructSDandField, i.e. where the sensitive detectors and magnetic fields are implemented
void DetectorConstruction::ConstructSDandField()
{
    // load the sensitive detector manager
    G4SDManager* sdm = G4SDManager::GetSDMpointer();
    sdm->SetVerboseLevel(1);  // set sensitive detector manager verbosity here

    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // create the sensitive detectors and bin them to the logical volumes here, or...

    VolumeEDepSD* tileTest000SD = new VolumeEDepSD("tile000_SD");
    SetSensitiveDetector("tile000_Log", tileTest000SD);
    sdm->AddNewDetector(tileTest000SD);

    VolumeEDepSD* tileTest001SD = new VolumeEDepSD("tile001_SD");
    SetSensitiveDetector("tile001_Log", tileTest001SD);
    sdm->AddNewDetector(tileTest001SD);
	
    // --------------------------------------------------
    // ...uncomment this line for the test sensitive detectors (implemented in src/TestMode.cc)
    //SDTest(sdm);
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}

// ============================================================
// ============================================================
// implement custom methods here

// DetectorConstruction methods ///////////////////////////////

G4double DetectorConstruction::pi = acos(-1);

// --> tile-specific stuff in DetectorConstruction_tile.cc



