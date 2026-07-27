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
	G4VisAttributes* grey = new G4VisAttributes(G4Colour::Grey());
	
    // off-the-shelf materials (from NIST)
    G4Material* air = nist->FindOrBuildMaterial("G4_AIR"); // air
	G4Material* SS = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL"); // steel
	G4Material* plastic = nist->FindOrBuildMaterial("G4_POLYSTYRENE"); // plastic scintillator
	
    // world
    G4RotationMatrix* worldRotation = new G4RotationMatrix();
    G4VSolid* worldBox = new G4Box("world_Shape", worldSizeX / 2, worldSizeY / 2, worldSizeZ / 2);
    G4LogicalVolume* worldLog = new G4LogicalVolume(worldBox, air, "world_Logical");
    G4VisAttributes* visAttrWorld = new G4VisAttributes();
    visAttrWorld->SetVisibility(false);
    worldLog->SetVisAttributes(visAttrWorld);
    G4VPhysicalVolume* worldPhys = new G4PVPlacement(nullptr, {}, worldLog, "world", nullptr, false, 0);
	
	// generic translation and roto-translation, to be applied element-by-element
	G4ThreeVector pos_temp;
	G4Transform3D pos_rot_temp;

    ///////////////////
    //// FZU stack ////

    G4double FZU_theta = 2 * atan((FZU_W - FZU_w) / FZU_h);
    G4double FZU_r = FZU_W / tan(FZU_theta / 2);

    geomTrapezoid* FZU_geom = new geomTrapezoid(FZU_r, FZU_h, FZU_theta);

    G4LogicalVolume* FZU_lvols[TILEFZU_N];
    G4LogicalVolume* FZU_lvols_fibres[2*TILEFZU_N];

    FZU_geom->AddHorGaps(FZU_sidegap);

    for (G4int i = 0; i < TILEFZU_N; i++) {

        G4String FZUName = "FZU" + std::to_string(i);
        G4String FZUfibreName = "FZU_fibre" + std::to_string(i);

        // tiles...

        pos_temp = G4ThreeVector(0, 0, z_FZU_front + i*(FZU_thk + FZU_zgap) + FZU_thk/2);
        pos_rot_temp = G4Translate3D(pos_temp) * G4Rotate3D(FZU_ang_x, G4Vector3D(1,0,0)) * G4Rotate3D(FZU_ang_y, G4Vector3D(0,1,0)) * G4Rotate3D(FZU_ang_z, G4Vector3D(0,0,1));
        FZU_lvols[i] = fLogTile(FZUName, plastic, cyan, FZU_geom, FZU_thk, 0, FZU_holeradius, FZU_holex, FZU_holey);
        new G4PVPlacement(pos_rot_temp, FZU_lvols[i], FZUName + "_Phys", worldLog, false, i);

        // ... + fibres - keeping left and right separate
        FZU_lvols_fibres[2*i] = fLogPlaceFibreCirc(FZUfibreName, plastic, green, FZU_geom, worldLog, FZU_fibreradius, 2., 200., pos_rot_temp, -1);
        FZU_lvols_fibres[2*i+1] = fLogPlaceFibreCirc(FZUfibreName, plastic, green, FZU_geom, worldLog, FZU_fibreradius, 2., 200., pos_rot_temp, 1);
    }

    FZU_geom->RmHorGaps();

    //// FZU stack ////
    ///////////////////

    ////////////////////////////////////
    //// CERN stack - trigger tiles ////

    G4double tileCernTrigger_sign = -1;

    G4double tileCernTrigger_theta = 2 * atan((S2_W - S2_w) / S2_h);
    G4double tileCernTrigger_r = S2_W / tan(tileCernTrigger_theta / 2);

    geomTrapezoid* tileCernTrigger_geom = new geomTrapezoid(tileCernTrigger_r, S2_h, tileCernTrigger_theta);

    G4LogicalVolume* tileCernTrigger_lvols[2];
    G4LogicalVolume* tileCernTrigger_lvols_fibres[2];

    tileCernTrigger_geom->AddHorGaps(S2_sidegap);

    for (G4int i = 0; i < 2; i++) {

        G4String tileTriggerName = "tileCernTrigger" + std::to_string(i);
        G4String fibreTriggerName = "tileCernTrigger_fibre" + std::to_string(i);

        G4double z_temp = TILECERN_B_ANY ? (z_tileCern_front - i*(S2_thk + gen_gap) - S2_thk/2) : z_FZU_rear + (i==0 ? 17.5*cm : 51.5*cm);

		// tiles...
        pos_temp = G4ThreeVector(0, - passive_h/2 + passive_trig_shift, z_temp);
		pos_rot_temp = G4Translate3D(pos_temp) * G4Rotate3D(S6_ang_x, G4Vector3D(1,0,0)) * G4Rotate3D(S6_ang_y, G4Vector3D(0,1,0)) * G4Rotate3D(S6_ang_z, G4Vector3D(0,0,1)); // note: trigger tiles are oriented like those in S6 stack
        tileCernTrigger_lvols[i] = fLogTile(tileTriggerName, plastic, cyan, tileCernTrigger_geom, S2_thk, tileCernTrigger_sign, S2_holeradius, S2_holex, S2_holey);
        new G4PVPlacement(pos_rot_temp, tileCernTrigger_lvols[i], tileTriggerName + "_Phys", worldLog, false, i);

		// ... + fibres
        tileCernTrigger_lvols_fibres[i] = fLogPlaceFibreCirc(fibreTriggerName, plastic, green, tileCernTrigger_geom, worldLog, S2_fibreradius, 2., 200., pos_rot_temp, tileCernTrigger_sign);

    }

    tileCernTrigger_geom->RmHorGaps();

    //// CERN stack - trigger tiles ////
    ////////////////////////////////////

    ////////////////////
    //// CERN stack ////

    if (TILECERN_B_ANY) {

        G4int tileCern_sign;

        G4double tileCern_ang_x;
        G4double tileCern_ang_y;
        G4double tileCern_ang_z;
        G4double tileCern_w;
        G4double tileCern_W;
        G4double tileCern_h;
        G4double tileCern_thk;
        G4double tileCern_fibreradius;
        G4double tileCern_sidegap;
        G4double tileCern_holeradius;
        G4double tileCern_holey;
        G4double tileCern_holex;
        auto tileCern_fzrel = [this](G4int i) { return TILECERN_B_S6 ? S6_fzrel(i) : S2_fzrel(i); };
        auto tileCern_fxrel = [this](G4int i) { return TILECERN_B_S6 ? S6_fxrel(i) : S2_fxrel(i); };
        auto tileCern_fyrel = [this](G4int i) { return TILECERN_B_S6 ? S6_fyrel(i) : S2_fyrel(i); };   

        if (TILECERN_B_S6) {
            tileCern_ang_x = S6_ang_x;
            tileCern_ang_y = S6_ang_y;
            tileCern_ang_z = S6_ang_z;
            tileCern_w = S6_w;
            tileCern_W = S6_W;
            tileCern_h = S6_h;
            tileCern_thk = S6_thk;
            tileCern_fibreradius = S6_fibreradius;
            tileCern_sidegap = S6_sidegap;
            tileCern_holeradius = S6_holeradius;
            tileCern_holey = S6_holey;
            tileCern_holex = S6_holex;
        } else {
            tileCern_ang_x = S2_ang_x;
            tileCern_ang_y = S2_ang_y;
            tileCern_ang_z = S2_ang_z;
            tileCern_w = S2_w;
            tileCern_W = S2_W;
            tileCern_h = S2_h;
            tileCern_thk = S2_thk;
            tileCern_fibreradius = S2_fibreradius;
            tileCern_sidegap = S2_sidegap;
            tileCern_holeradius = S2_holeradius;
            tileCern_holey = S2_holey;
            tileCern_holex = S2_holex;
        }

        G4double tileCern_theta = 2 * atan((tileCern_W - tileCern_w) / tileCern_h);
        G4double tileCern_r = tileCern_W / tan(tileCern_theta / 2);

        geomTrapezoid* tileCern_geom = new geomTrapezoid(tileCern_r, tileCern_h, tileCern_theta);
        geomRectangle* passive_geom_u = new geomRectangle(passive_w_u, passive_h);
        geomRectangle* passive_geom_d = new geomRectangle(passive_w_d, passive_h);

        G4LogicalVolume* tileCern_lvols[TILECERN_N];
        G4LogicalVolume* tileCern_lvols_fibres[TILECERN_N];
        G4LogicalVolume* tileCern_lvols_passive[2*TILECERN_N];

        tileCern_geom->AddHorGaps(tileCern_sidegap);

        for (G4int i = 0; i < TILECERN_N; i++) {

            tileCern_sign = TILECERN_B_S6 ? -1 : ((i+1)%2 ? -1 : 1);

            G4String tileName = "tileCern" + std::to_string(i);
            G4String fibreName = "tileCern_fibre" + std::to_string(i);

            // passive layers
            pos_temp = G4ThreeVector(0, 0, z_tileCern_front + tileCern_fzrel(i) - (passive_thk_gross + tileCern_thk)/2 - (passive_thk_gross - 2*passive_thk_gap - passive_thk_u)/2);
            pos_rot_temp = G4Translate3D(pos_temp) * G4Rotate3D(passive_ang_x, G4Vector3D(1,0,0)) * G4Rotate3D(passive_ang_y, G4Vector3D(0,1,0)) * G4Rotate3D(passive_ang_z, G4Vector3D(0,0,1));
            tileCern_lvols_passive[2*i] = fLogTile("tileCern_passive", SS, grey, passive_geom_u, passive_thk_u, 0, 0, 0, 0);
            new G4PVPlacement(pos_rot_temp, tileCern_lvols_passive[2*i], "tileCern_passive_Phys", worldLog, false, i);

            pos_temp = G4ThreeVector(0, 0, z_tileCern_front + tileCern_fzrel(i) - (passive_thk_gross + tileCern_thk)/2 + (passive_thk_gross - 2*passive_thk_gap - passive_thk_d)/2);
            pos_rot_temp = G4Translate3D(pos_temp) * G4Rotate3D(passive_ang_x, G4Vector3D(1,0,0)) * G4Rotate3D(passive_ang_y, G4Vector3D(0,1,0)) * G4Rotate3D(passive_ang_z, G4Vector3D(0,0,1));
            tileCern_lvols_passive[2*i+1] = fLogTile("tileCern_passive", SS, grey, passive_geom_d, passive_thk_d, 0, 0, 0, 0);
            new G4PVPlacement(pos_rot_temp, tileCern_lvols_passive[2*i+1], "tileCern_passive_Phys", worldLog, false, i);

            // tiles...
            pos_temp = G4ThreeVector(tileCern_fxrel(i), tileCern_fyrel(i), z_tileCern_front + tileCern_fzrel(i));
            pos_rot_temp = G4Translate3D(pos_temp) * G4Rotate3D(tileCern_ang_x, G4Vector3D(1,0,0)) * G4Rotate3D(tileCern_ang_y, G4Vector3D(0,1,0)) * G4Rotate3D(tileCern_ang_z, G4Vector3D(0,0,1));
            tileCern_lvols[i] = fLogTile(tileName, plastic, cyan, tileCern_geom, tileCern_thk, tileCern_sign, tileCern_holeradius, tileCern_holex, tileCern_holey);
            new G4PVPlacement(pos_rot_temp, tileCern_lvols[i], tileName + "_Phys", worldLog, false, i);

            // ... + fibres
            tileCern_lvols_fibres[i] = fLogPlaceFibreCirc(fibreName, plastic, green, tileCern_geom, worldLog, tileCern_fibreradius, 2., 200., pos_rot_temp, tileCern_sign);
        }

        tileCern_geom->RmHorGaps();

    }

    //// CERN stack ////
    ////////////////////

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

    ///////////////////
    //// FZU stack ////

    for (G4int i = 0; i < TILEFZU_N; i++) {
        G4String FZUName = "FZU" + std::to_string(i);
        VolumeEDepSD* FZUSD = new VolumeEDepSD("SD_FZU" + std::to_string(i));
        SetSensitiveDetector(FZUName + "_Log", FZUSD);
        sdm->AddNewDetector(FZUSD);
    }

    //// FZU stack ////
    ///////////////////

    ////////////////////////////////////
    //// CERN stack - trigger tiles ////

    for (G4int i = 0; i < 2; i++) {
        G4String tileTriggerName = "tileCernTrigger" + std::to_string(i);
        VolumeEDepSD* tileCernTriggerSD = new VolumeEDepSD("SD_tileCernTrigger" + std::to_string(i));
        SetSensitiveDetector(tileTriggerName + "_Log", tileCernTriggerSD);
        sdm->AddNewDetector(tileCernTriggerSD);
    }

    //// CERN stack - trigger tiles ////
    ////////////////////////////////////

    ////////////////////
    //// CERN stack ////

    if (TILECERN_B_ANY) {
        for (G4int i = 0; i < TILECERN_N; i++) {
            G4String tileName = "tileCern" + std::to_string(i);
            VolumeEDepSD* tileCernSD = new VolumeEDepSD("SD_tileCern" + std::to_string(i));
            SetSensitiveDetector(tileName + "_Log", tileCernSD);
            sdm->AddNewDetector(tileCernSD);
        }
    }

    //// CERN stack ////
    ////////////////////
    
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

G4double DetectorConstruction::S2_fzrel(G4int i){ // longitudinal position of single tiles, relative to front
	return floor(i/2)*(passive_thk_gross + S2_thk) + passive_thk_gross + S2_thk/2;
} 
G4double DetectorConstruction::S2_fxrel(G4int i){ // transverse (x) position of single tiles, relative to centre
	return ((i+1)%2 ? -1 : 1) * S2_xgap/2;
}
G4double DetectorConstruction::S2_fyrel(G4int i){ // transverse (y) position of single tiles, relative to centre
	return 0;
}

G4double DetectorConstruction::S6_fzrel(G4int i) { // longitudinal position of single tiles, relative to front
	return i*(passive_thk_gross + S6_thk) + passive_thk_gross + S6_thk/2;
} 
G4double DetectorConstruction::S6_fxrel(G4int i){ // transverse (x) position of single tiles, relative to centre
	return 0;
}
G4double DetectorConstruction::S6_fyrel(G4int i){ // transverse (y) position of single tiles, relative to centre
	return - passive_h/2 + passive_S6_shift;
}

// --> tile-specific stuff in DetectorConstruction_tile.cc
