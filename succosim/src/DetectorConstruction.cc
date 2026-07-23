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

#include <string>

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
    G4double worldSizeZ = 15 * m;
    G4RotationMatrix* worldRotation = new G4RotationMatrix();
    worldRotation->rotateX(90 * deg);
    worldRotation->rotateZ(180 * deg);
    G4VSolid* worldBox = new G4Box("world_Shape", worldSizeX / 2, worldSizeZ / 2, worldSizeY / 2);
    G4LogicalVolume* worldLog = new G4LogicalVolume(worldBox, air, "world_Logical");
    G4VisAttributes* visAttrWorld = new G4VisAttributes();
    visAttrWorld->SetVisibility(false);
    worldLog->SetVisAttributes(visAttrWorld);
    G4VPhysicalVolume* worldPhys = new G4PVPlacement(nullptr, {}, worldLog, "world", nullptr, false, 0);

    // 2nd world layer, rotated to have the beam travelling horizontally along z
    G4LogicalVolume* worldLog_rot = new G4LogicalVolume(worldBox, air, "world_Logical_Rot");
    G4VisAttributes* visAttrWorld_rot = new G4VisAttributes();
    visAttrWorld_rot->SetVisibility(true);
    worldLog_rot->SetVisAttributes(visAttrWorld_rot);
    new G4PVPlacement(worldRotation, {}, worldLog_rot, "world_Rot", worldLog, false, 0);

    G4ThreeVector pos_temp;
	
	// tile shapes - CERN S2
    G4int S2_sign = -1; // half-tile chirality; if 0, then the full tile is created
    G4double S2_w = 70.5*mm; // short-side width (half-module)
    G4double S2_W = 75*mm; // long-side width (half-module)
    G4double S2_h = 97*mm; // height
    G4double S2_thk = 3*mm; // thickness
    G4double S2_fibreradius = 0.5*mm; // radius of the WLS fibre
    G4double S2_sidegap = 0*mm; // side reduction to account for the fibres
    G4double S2_holeradius = 5*mm; // radius of the pipe/rod hole
    G4double S2_holey = S2_h/2 - 6*mm; // pipe/rod hole centre y (relative to full-module tile centre)
    G4double S2_holex = 6*mm; // pipe/rod hole centre x (relative to full-module tile centre)

	// tile shapes - CERN S6
    G4int S6_sign = -1; // half-tile chirality; if 0, then the full tile is created
    G4double S6_w = 91*mm; // short-side width (half-module)
    G4double S6_W = 100*mm; // long-side width (half-module)
    G4double S6_h = 187*mm; // height
    G4double S6_thk = 3*mm; // thickness
    G4double S6_fibreradius = 0.5*mm; // radius of the WLS fibre
    G4double S6_sidegap = 0*mm; // side reduction to account for the fibres
    G4double S6_holeradius = 5*mm; // radius of the pipe/rod hole
    G4double S6_holey = S6_h/2 - 6*mm; // pipe/rod hole centre y (relative to full-module tile centre)
    G4double S6_holex = 6*mm; // pipe/rod hole centre x (relative to full-module tile centre)

    // steel tile shapes
    G4double passive_thk = 3*cm;

    // general positioning
    G4double z_tileCern_front = 3*m;

    //// CERN stack ////

    G4RotationMatrix* tileCern_rot = new G4RotationMatrix();

    G4int tileCern_n = 10;

    tileCern_rot->rotateY(-90 * deg);

    G4int tileCern_sign = S6_sign;
    G4double tileCern_w = S6_w;
    G4double tileCern_W = S6_W;
    G4double tileCern_h = S6_h;
    G4double tileCern_thk = S6_thk;
    G4double tileCern_fibreradius = S6_fibreradius;
    G4double tileCern_sidegap = S6_sidegap;
    G4double tileCern_holeradius = S6_holeradius;
    G4double tileCern_holey = S6_holey;
    G4double tileCern_holex = S6_holex;

    G4double tileCern_theta = 2 * atan((tileCern_W - tileCern_w) / tileCern_h); // angle (full-module)
    G4double tileCern_r = tileCern_W / tan(tileCern_theta / 2); // radial distance from cylinder centre (full-module)

	geomTrapezoid* tileCern_geom = new geomTrapezoid(tileCern_r, tileCern_h, tileCern_theta);

    G4LogicalVolume* tileCern_lvols[tileCern_n];
    G4LogicalVolume* tileCern_lvols_fibres[tileCern_n];
    G4LogicalVolume* tileCern_lvols_passive[tileCern_n];

    tileCern_geom->AddHorGaps(tileCern_sidegap);

    for (G4int i = 0; i < tileCern_n; i++) {
        G4String tileName = "log_tileCern" + std::to_string(i);
        G4String fibreName = "log_fibreCern" + std::to_string(i);

        pos_temp = G4ThreeVector(0, z_tileCern_front + i*(passive_thk + tileCern_thk/2), 0);

        tileCern_lvols[i] = fLogTile(tileName, bc400, cyan, tileCern_geom, tileCern_thk, tileCern_sign, tileCern_holeradius, tileCern_holex, tileCern_holey);
        new G4PVPlacement(tileCern_rot, pos_temp, tileCern_lvols[i], tileName, worldLog_rot, false, i);

        tileCern_lvols_fibres[i] = fLogPlaceFibreCirc(
            fibreName, bc400, green, tileCern_geom, worldLog_rot, tileCern_fibreradius, 5., 50., pos_temp, tileCern_rot, tileCern_sign
        );
    }

    tileCern_geom->RmHorGaps();

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

    //// S6 stack ////

    const G4int tileCern_n = 10;
    for (G4int i = 0; i < tileCern_n; i++) {
        G4String tileName = "log_tileCern" + std::to_string(i);
        VolumeEDepSD* tileSD = new VolumeEDepSD("SD_tileCern" + std::to_string(i));
        SetSensitiveDetector(tileName + "_Log", tileSD);
        sdm->AddNewDetector(tileSD);
    }
	
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
