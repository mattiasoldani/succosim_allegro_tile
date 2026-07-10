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
    G4VSolid* worldBox = new G4Box("world_Shape", worldSizeX / 2, worldSizeY / 2, worldSizeZ / 2);
    G4LogicalVolume* worldLog = new G4LogicalVolume(worldBox, air, "world_Logical");
    G4VisAttributes* visAttrWorld = new G4VisAttributes();
    visAttrWorld->SetVisibility(false);
    worldLog->SetVisAttributes(visAttrWorld);
    G4VPhysicalVolume* worldPhys = new G4PVPlacement(nullptr, {}, worldLog, "world", nullptr, false, 0);

    // 2nd world layer, rotated to have the beam travelling horizontally along z
    G4LogicalVolume* worldLog_rot = new G4LogicalVolume(worldBox, air, "world_Logical_Rot");
    G4VisAttributes* visAttrWorld_rot = new G4VisAttributes();
    visAttrWorld_rot->SetVisibility(false);
    worldLog_rot->SetVisAttributes(visAttrWorld_rot);
    new G4PVPlacement(worldRotation, {}, worldLog_rot, "world_Rot", worldLog, false, 0);

    G4RotationMatrix* tileRotation = new G4RotationMatrix();
    tileRotation->rotateY(20 * deg);
	
	// tile shapes - S2
    G4double S2_w = 70.5*mm; // short-side width (half-module)
    G4double S2_W = 75*mm; // long-side width (half-module)
    G4double S2_h = 97*mm; // height
    G4double S2_thk = 3*mm; // thickness
    G4double S2_fibreradius = 0.5*mm; // radius of the WLS fibre
    G4double S2_sidegap = 0*mm; // side reduction to account for the fibres
    G4double S2_holeradius = 0*mm; // radius of the pipe/rod hole
    G4double S2_holey = S2_h/2 - 6*mm; // pipe/rod hole centre y (relative to full-module tile centre)
    G4double S2_holex = 6*mm; // pipe/rod hole centre x (relative to full-module tile centre)

    G4double S2_theta = 2 * atan((S2_W - S2_w) / S2_h); // angle (full-module)
    G4double S2_r = S2_W / tan(S2_theta / 2); // radial distance from cylinder centre (full-module)


	geomTrapezoid* S2_geom = new geomTrapezoid(S2_r, S2_h, S2_theta);

	// tile shapes - S6
    G4double S6_w = 91*mm; // short-side width (half-module)
    G4double S6_W = 100*mm; // long-side width (half-module)
    G4double S6_h = 187*mm; // height
    G4double S6_thk = 3*mm; // thickness
    G4double S6_fibreradius = 0.5*mm; // radius of the WLS fibre
    G4double S6_sidegap = 0*mm; // side reduction to account for the fibres
    G4double S6_holeradius = 0*mm; // radius of the pipe/rod hole
    G4double S6_holey = S6_h/2 - 6*mm; // pipe/rod hole centre y (relative to full-module tile centre)
    G4double S6_holex = 6*mm; // pipe/rod hole centre x (relative to full-module tile centre)

    G4double S6_theta = 2 * atan((S6_W - S6_w) / S6_h); // angle (full-module)
    G4double S6_r = S6_W / tan(S6_theta / 2); // radial distance from cylinder centre (full-module)

	geomTrapezoid* S6_geom = new geomTrapezoid(S6_r, S6_h, S6_theta);

    //// preliminary tests 00 ////

    // S2_geom->AddHorGaps(sidegap);

    // sign = 1;
    // pos = G4ThreeVector(-sign*0.5*cm, 0*thickness, 0);
    // G4LogicalVolume* tileTest000Log = fLogTile("tile000", bc400, cyan, tileTestGeom, thickness, sign, holeradius, holex, holey);
    // new G4PVPlacement(tileRotation, pos, tileTest000Log, "tile000", worldLog_rot, false, 0);

    // G4LogicalVolume* tileTest000FibreLog = fLogPlaceFibreCirc("fibre000", bc400, green, tileTestGeom, worldLog_rot, fibreradius, 5., 50., pos, tileRotation, sign);

    // S2_geom->RmHorGaps();

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

    //// preliminary tests 01 ////

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



