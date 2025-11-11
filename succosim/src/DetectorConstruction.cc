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

#include "DetectorConstruction.hh"
#include "CustomSD.hh"

#include <G4Trd.hh>

// DetectorConstruction::Construct, i.e. where the setup geometry is implemented
G4VPhysicalVolume* DetectorConstruction::Construct()
{
    // NIST database
    G4NistManager* nist = G4NistManager::Instance();
	
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // define the world and all the setup stuff (materials, volumes) here, or...
	
    // colors
    G4VisAttributes* cyan = new G4VisAttributes(G4Colour::Cyan());
    G4VisAttributes* blue = new G4VisAttributes(G4Colour::Blue());
    G4VisAttributes* red = new G4VisAttributes(G4Colour::Red());
    G4VisAttributes* green = new G4VisAttributes(G4Colour::Green());
    G4VisAttributes* magenta = new G4VisAttributes(G4Colour::Magenta());
	
    // off-the-shelf materials (from NIST)
    G4Material* air = nist->FindOrBuildMaterial("G4_AIR"); // air
	
    // manual material: BC400 scintillator
    G4Element* elH = new G4Element("Hydrogen", "H", 1., 1.0079 * g/mole);
    G4Element* elC = new G4Element("Carbon", "C", 6., 12.01 * g/mole);
    G4Material* bc400 = new G4Material("BC400", 1.032*g/cm3, 2);
    bc400->AddElement(elH, 0.085);
    bc400->AddElement(elC, 0.915);
	
    // world
    G4double worldSizeX = 2 * m;
    G4double worldSizeY = 2 * m;
    G4double worldSizeZ = 2 * m;
    G4RotationMatrix* worldRotation = new G4RotationMatrix();
    worldRotation->rotateX(90 * deg);
    G4VSolid* worldBox = new G4Box("World_Shape", worldSizeX / 2, worldSizeY / 2, worldSizeZ / 2);
    G4LogicalVolume* worldLog = new G4LogicalVolume(worldBox, air, "World_Logical");
    G4VisAttributes* visAttrWorld = new G4VisAttributes();
    visAttrWorld->SetVisibility(false);
    worldLog->SetVisAttributes(visAttrWorld);
    G4VPhysicalVolume* worldPhys = new G4PVPlacement(nullptr, {}, worldLog, "World", nullptr, false, 0);

    // 2nd world layer, rotated to have the beam travelling horizontally along z
    G4LogicalVolume* worldLog_rot = new G4LogicalVolume(worldBox, air, "World_Logical_Rot");
    G4VisAttributes* visAttrWorld_rot = new G4VisAttributes();
    visAttrWorld_rot->SetVisibility(false);
    worldLog_rot->SetVisAttributes(visAttrWorld_rot);
    new G4PVPlacement(worldRotation, {}, worldLog_rot, "World_Rot", worldLog, false, 0);
	
	// tile shapes
    G4double height = 50*mm;
    G4double radius = 2.8*m + 10*mm + height/2;
    G4double angle = 2*acos(-1)/128;
    G4double thickness = 3*mm;
    G4double sidegap = 2*mm;
    G4double holeradius = 3*mm;
    G4double holey = height/2 - 6*mm;
    G4double holex = 6*mm;

	geomTrapezoid* tileTestGeom = new geomTrapezoid(radius, height, angle);
    G4double offset;

    // full tile
    G4LogicalVolume* tileTest0Log = fLogTile(tileTestGeom, "tile0", thickness, bc400, cyan, 0, holeradius, 0, holey);
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), tileTest0Log, "tile0", worldLog_rot, false, 0);

    tileTestGeom->AddHorGaps(sidegap);
    G4LogicalVolume* tileTest1Log = fLogTile(tileTestGeom, "tile1", thickness, bc400, cyan, 0, holeradius, holex, holey);
    new G4PVPlacement(nullptr, G4ThreeVector(0, -2*thickness, 0), tileTest1Log, "tile1", worldLog_rot, false, 0);
    tileTestGeom->RmHorGaps();

    // half tile, positive horizontal side
    G4LogicalVolume* tileTest2Log = fLogTile(tileTestGeom, "tile2", thickness, bc400, blue, 1, holeradius, 0, holey);
    new G4PVPlacement(nullptr, G4ThreeVector(0, -4*thickness, 0), tileTest2Log, "tile2", worldLog_rot, false, 0);

    tileTestGeom->AddHorGaps(sidegap);
    G4LogicalVolume* tileTest3Log = fLogTile(tileTestGeom, "tile3", thickness, bc400, blue, 1, holeradius, holex, holey);
    new G4PVPlacement(nullptr, G4ThreeVector(0, -6*thickness, 0), tileTest3Log, "tile3", worldLog_rot, false, 0);
    tileTestGeom->RmHorGaps();

    // half tile, negative horizontal side
    G4LogicalVolume* tileTest4Log = fLogTile(tileTestGeom, "tile4", thickness, bc400, red, -1, holeradius, 0, holey);
    new G4PVPlacement(nullptr, G4ThreeVector(0, -8*thickness, 0), tileTest4Log, "tile4", worldLog_rot, false, 0);

    tileTestGeom->AddHorGaps(sidegap);
    G4LogicalVolume* tileTest5Log = fLogTile(tileTestGeom, "tile5", thickness, bc400, red, -1, holeradius, holex, holey);
    new G4PVPlacement(nullptr, G4ThreeVector(0, -10*thickness, 0), tileTest5Log, "tile5", worldLog_rot, false, 0);
    tileTestGeom->RmHorGaps();

    // half tile, positive horizontal side, centred (notice the -offset)
    offset = tileTestGeom->GetFullToHalfCentreOffset();
    G4LogicalVolume* tileTest6Log = fLogTile(tileTestGeom, "tile6", thickness, bc400, green, 1, holeradius, 0, holey);
    new G4PVPlacement(nullptr, G4ThreeVector(-offset, -12*thickness, 0), tileTest6Log, "tile6", worldLog_rot, false, 0);

    tileTestGeom->AddHorGaps(sidegap);
    offset = tileTestGeom->GetFullToHalfCentreOffset();
    G4LogicalVolume* tileTest7Log = fLogTile(tileTestGeom, "tile7", thickness, bc400, green, 1, holeradius, holex, holey);
    new G4PVPlacement(nullptr, G4ThreeVector(-offset, -14*thickness, 0), tileTest7Log, "tile7", worldLog_rot, false, 0);
    tileTestGeom->RmHorGaps();

    // half tile, negative horizontal side, centred (notice the +offset)
    offset = tileTestGeom->GetFullToHalfCentreOffset();
    G4LogicalVolume* tileTest8Log = fLogTile(tileTestGeom, "tile8", thickness, bc400, magenta, -1, holeradius, 0, holey);
    new G4PVPlacement(nullptr, G4ThreeVector(offset, -16*thickness, 0), tileTest8Log, "tile8", worldLog_rot, false, 0);

    tileTestGeom->AddHorGaps(sidegap);
    offset = tileTestGeom->GetFullToHalfCentreOffset();
    G4LogicalVolume* tileTest9Log = fLogTile(tileTestGeom, "tile9", thickness, bc400, magenta, -1, holeradius, holex, holey);
    new G4PVPlacement(nullptr, G4ThreeVector(offset, -18*thickness, 0), tileTest9Log, "tile9", worldLog_rot, false, 0);
    tileTestGeom->RmHorGaps();

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
	
    // --------------------------------------------------
    // ...uncomment this line for the test sensitive detectors (implemented in src/TestMode.cc)
    //SDTest(sdm);
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}

// ============================================================
// ============================================================
// implement custom methods here

