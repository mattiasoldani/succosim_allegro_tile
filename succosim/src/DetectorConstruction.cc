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
    G4VisAttributes* white = new G4VisAttributes(G4Colour::White())
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
    G4Material* plastic = nist->FindOrBuildMaterial("G4_POLYSTYRENE"); // plastic
    G4Material* pbGl = nist->FindOrBuildMaterial("G4_GLASS_LEAD"); // Pb glass
    G4Material* vacuum = nist->FindOrBuildMaterial("G4_Galactic"); // vacuum
	
    // world
    G4RotationMatrix* worldRotation = new G4RotationMatrix();
    G4VSolid* worldBox = new G4Box("world_Shape", worldSizeX / 2, worldSizeY / 2, worldSizeZ / 2);
    G4LogicalVolume* worldLog = new G4LogicalVolume(worldBox, air, "world_Log");
    worldLog->SetVisAttributes(invisible);
    G4VPhysicalVolume* worldPhys = new G4PVPlacement(nullptr, {}, worldLog, "world", nullptr, false, 0);
	
	// generic translation and roto-translation, to be applied element-by-element
	G4ThreeVector pos_temp;
	G4Transform3D pos_rot_temp;

    ////////////////////////////
    //////// test tiles ////////

    G4double offset;
    G4int sign;

    G4Material* plastic_fibre = plastic;
    
    geomTrapezoid* tileTestGeom = new geomTrapezoid(radius, height, angle);

    // vvvvvvvvvv trapezoidal tiles, w/ fibre gap correction

    tileTestGeom->AddHorGaps(sidegap);

    sign = 1;
    pos_temp = G4ThreeVector(-sign*0.5*cm, 0, 0*thickness);
    pos_rot_temp = G4Translate3D(pos_temp) * G4Rotate3D(rot_x_ang, G4Vector3D(1,0,0)) * G4Rotate3D(rot_y_ang, G4Vector3D(0,1,0)) * G4Rotate3D(rot_z_ang, G4Vector3D(0,0,1));
    G4LogicalVolume* tileTest000Log = fLogTile("tile000", plastic, cyan, tileTestGeom, thickness, sign, holeradius, holex, holey);
    new G4PVPlacement(pos_rot_temp, tileTest000Log, "tile000", worldLog, false, 0);

    G4LogicalVolume* tileTest000FibreLog = fLogPlaceFibreCirc("fibre000", plastic_fibre, green, tileTestGeom, worldLog, fibreradius, 5., 50., pos_rot_temp, sign);

    sign = -1;
    pos_temp = G4ThreeVector(-sign*0.5*cm, 0, 10*thickness);
    pos_rot_temp = G4Translate3D(pos_temp) * G4Rotate3D(rot_x_ang, G4Vector3D(1,0,0)) * G4Rotate3D(rot_y_ang, G4Vector3D(0,1,0)) * G4Rotate3D(rot_z_ang, G4Vector3D(0,0,1));
    G4LogicalVolume* tileTest001Log = fLogTile("tile001", plastic, cyan, tileTestGeom, thickness, sign, holeradius, holex, holey);
    new G4PVPlacement(pos_rot_temp, tileTest001Log, "tile001", worldLog, false, 0);

    G4LogicalVolume* tileTest001FibreLog = fLogPlaceFibreCirc("fibre001", plastic_fibre, green, tileTestGeom, worldLog, fibreradius, 5., 50., pos_rot_temp, sign);

    sign = 0;
    pos_temp = G4ThreeVector(-3*cm, 0, -10*thickness);
    pos_rot_temp = G4Translate3D(pos_temp) * G4Rotate3D(rot_x_ang, G4Vector3D(1,0,0)) * G4Rotate3D(rot_y_ang, G4Vector3D(0,1,0)) * G4Rotate3D(rot_z_ang, G4Vector3D(0,0,1));
    G4LogicalVolume* tileTest002Log = fLogTile("tile002", plastic, cyan, tileTestGeom, thickness, sign, holeradius, holex, holey);
    new G4PVPlacement(pos_rot_temp, tileTest002Log, "tile002", worldLog, false, 0);

    G4LogicalVolume* tileTest002FibreLog = fLogPlaceFibreCirc("fibre002", plastic_fibre, green, tileTestGeom, worldLog, fibreradius, 5., 50., pos_rot_temp, sign);

    tileTestGeom->RmHorGaps();

    // vvvvvvvvvv rectangular tiles, w/o fibre gap correction

	geomTrapezoid* tileTestGeomRect = new geomRectangle(height, height);

    sign = 1;
    pos_temp = G4ThreeVector(-sign*0.5*cm, 0, 0*thickness + 200*mm);
    pos_rot_temp = G4Translate3D(pos_temp) * G4Rotate3D(rot_x_ang, G4Vector3D(1,0,0)) * G4Rotate3D(rot_y_ang, G4Vector3D(0,1,0)) * G4Rotate3D(rot_z_ang, G4Vector3D(0,0,1));
    G4LogicalVolume* tileTest003Log = fLogTile("tile003", plastic, cyan, tileTestGeomRect, thickness, sign, holeradius, holex, holey);
    new G4PVPlacement(pos_rot_temp, tileTest003Log, "tile003", worldLog, false, 0);

    G4LogicalVolume* tileTest003FibreLog = fLogPlaceFibreCirc("fibre003", plastic_fibre, green, tileTestGeomRect, worldLog, fibreradius, 5., 50., pos_rot_temp, sign);

    sign = -1;
    pos_temp = G4ThreeVector(-sign*0.5*cm, 0, 10*thickness + 200*mm);
    pos_rot_temp = G4Translate3D(pos_temp) * G4Rotate3D(rot_x_ang, G4Vector3D(1,0,0)) * G4Rotate3D(rot_y_ang, G4Vector3D(0,1,0)) * G4Rotate3D(rot_z_ang, G4Vector3D(0,0,1));
    G4LogicalVolume* tileTest004Log = fLogTile("tile004", plastic, cyan, tileTestGeomRect, thickness, sign, holeradius, holex, holey);
    new G4PVPlacement(pos_rot_temp, tileTest004Log, "tile004", worldLog, false, 0);

    G4LogicalVolume* tileTest004FibreLog = fLogPlaceFibreCirc("fibre004", plastic_fibre, green, tileTestGeomRect, worldLog, fibreradius, 5., 50., pos_rot_temp, sign);

    sign = 0;
    pos_temp = G4ThreeVector(-3*cm, 0, -10*thickness + 200*mm);
    pos_rot_temp = G4Translate3D(pos_temp) * G4Rotate3D(rot_x_ang, G4Vector3D(1,0,0)) * G4Rotate3D(rot_y_ang, G4Vector3D(0,1,0)) * G4Rotate3D(rot_z_ang, G4Vector3D(0,0,1));
    G4LogicalVolume* tileTest005Log = fLogTile("tile005", plastic, cyan, tileTestGeomRect, thickness, sign, holeradius, holex, holey);
    new G4PVPlacement(pos_rot_temp, tileTest005Log, "tile005", worldLog, false, 0);

    G4LogicalVolume* tileTest005FibreLog = fLogPlaceFibreCirc("fibre005", plastic_fibre, green, tileTestGeomRect, worldLog, fibreradius, 5., 50., pos_rot_temp, sign);

    //////// test tiles ////////
    ////////////////////////////

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

// --> tile-specific stuff in DetectorConstruction_tile.cc