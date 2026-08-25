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
    G4VisAttributes* invisible = new G4VisAttributes(false);
    G4VisAttributes* white = new G4VisAttributes(G4Colour::White());
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
    G4Material* aluminium = nist->FindOrBuildMaterial("G4_Al"); // aluminium
    G4Material* mylar = nist->FindOrBuildMaterial("G4_MYLAR"); // Mylar

    G4Material* mat_world = air; // material for world
    G4Material* plastic_ancillary = plastic; // plastic for ancillary detectors
    G4Material* plastic_fibre = plastic; // plastic for WLS fibres

    // manual material: CO2 for Cherenkov pipes
    G4Element* elC = new G4Element("Carbon", "C", 6., 12.01*g/mole);
    G4Element* elO  = new G4Element("Oxygen","O" , 8., 16.00*g/mole);
    G4Material* cherGas = new G4Material("CO2", 1.977*273.*mg/cm3/293., 2);
    cherGas->AddElement(elC, 1);
    cherGas->AddElement(elO, 2);

    // custom parameters
    id_config = CustomMessenger::Instance()->IdConfig();
    b_config_calib = id_config == 0;
    b_config_BB = id_config == 1;
    b_config_CERN_S2 = id_config == 2;
    b_config_CERN_S6 = id_config == 3;
    b_config_CERN_S6_upstr1 = id_config == 4;
    b_config_CERN_S6_upstr2 = id_config == 5;
    b_config_CERN_S6_upstr4 = id_config == 6;
    b_CERN_any = b_config_CERN_S2 || b_config_CERN_S6 || b_config_CERN_S6_upstr1 || b_config_CERN_S6_upstr2 || b_config_CERN_S6_upstr4;
    b_FZU = b_config_BB || b_CERN_any;
    b_CERN_trig = b_config_BB || b_CERN_any;
    b_CERN_S2 = b_config_CERN_S2;
    b_CERN_S6 = b_config_CERN_S6 || b_config_CERN_S6_upstr1 || b_config_CERN_S6_upstr2 || b_config_CERN_S6_upstr4;
    b_PbGl = !b_config_BB;
    b_scinti_small = CustomMessenger::Instance()->BPlaceUpstream();
    b_cher = CustomMessenger::Instance()->BPlaceUpstream();
    b_scinti_small_det = CustomMessenger::Instance()->BScintiSmallDet();
    b_cher_det = CustomMessenger::Instance()->BCherDet();
    b_hodo_det = CustomMessenger::Instance()->BHodoDet();
		
    // world
    G4RotationMatrix* worldRotation = new G4RotationMatrix();
    G4VSolid* worldBox = new G4Box("world_Shape", worldSizeX / 2, worldSizeY / 2, worldSizeZ / 2);
    G4LogicalVolume* worldLog = new G4LogicalVolume(worldBox, mat_world, "world_Log");
    worldLog->SetVisAttributes(invisible);
    G4VPhysicalVolume* worldPhys = new G4PVPlacement(nullptr, {}, worldLog, "world", nullptr, false, 0);
	
    // generic translation and roto-translation, to be applied element-by-element
    G4ThreeVector pos_temp;
    G4Transform3D pos_rot_temp;

    ///////////////////
    //// FZU stack ////
    if (IsFZU()) {

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

            pos_temp = G4ThreeVector(0, FZU_beamref_shift, z_FZU_front + i*(FZU_thk + FZU_zgap) + FZU_thk/2);
            pos_rot_temp = G4Translate3D(pos_temp) * G4Rotate3D(FZU_ang_x, G4Vector3D(1,0,0)) * G4Rotate3D(FZU_ang_y, G4Vector3D(0,1,0)) * G4Rotate3D(FZU_ang_z, G4Vector3D(0,0,1));
            FZU_lvols[i] = fLogTile(FZUName, plastic, cyan, FZU_geom, FZU_thk, 0, FZU_holeradius, FZU_holex, FZU_holey);
            new G4PVPlacement(pos_rot_temp, FZU_lvols[i], FZUName + "_Phys", worldLog, false, i);

            // ... + fibres - keeping left and right separate
            FZU_lvols_fibres[2*i] = fLogPlaceFibreCirc(FZUfibreName, plastic_fibre, green, FZU_geom, worldLog, FZU_fibreradius, 2., 200., pos_rot_temp, -1);
            FZU_lvols_fibres[2*i+1] = fLogPlaceFibreCirc(FZUfibreName, plastic_fibre, green, FZU_geom, worldLog, FZU_fibreradius, 2., 200., pos_rot_temp, 1);
        }

        FZU_geom->RmHorGaps();

    }
    //// FZU stack ////
    ///////////////////

    ////////////////////////////////////
    //// CERN stack - trigger tiles ////
    if (IsCERNTrig()) {

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

            G4double z_temp = (IsConfigBB()) ? z_FZU_rear + (i==0 ? 17.5*cm : 51.5*cm) : (z_tileCern_front - i*(S2_thk + gen_gap) - S2_thk/2);

            // tiles...
            pos_temp = G4ThreeVector(0, tileCern_beamref_shift - passive_h/2 + passive_trig_shift, z_temp);
            pos_rot_temp = G4Translate3D(pos_temp) * G4Rotate3D(S6_ang_x, G4Vector3D(1,0,0)) * G4Rotate3D(S6_ang_y, G4Vector3D(0,1,0)) * G4Rotate3D(S6_ang_z, G4Vector3D(0,0,1)); // note: trigger tiles are oriented like those in S6 stack
            tileCernTrigger_lvols[i] = fLogTile(tileTriggerName, plastic, cyan, tileCernTrigger_geom, S2_thk, tileCernTrigger_sign, S2_holeradius, S2_holex, S2_holey);
            new G4PVPlacement(pos_rot_temp, tileCernTrigger_lvols[i], tileTriggerName + "_Phys", worldLog, false, i);

            // ... + fibres
            tileCernTrigger_lvols_fibres[i] = fLogPlaceFibreCirc(fibreTriggerName, plastic_fibre, green, tileCernTrigger_geom, worldLog, S2_fibreradius, 2., 200., pos_rot_temp, tileCernTrigger_sign);

        }

        tileCernTrigger_geom->RmHorGaps();

    }
    //// CERN stack - trigger tiles ////
    ////////////////////////////////////

    ////////////////////
    //// CERN stack ////
    if (IsCERNAny()) {

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
        auto tileCern_fzrel = [this](G4int i) { return IsCERNS6() ? S6_fzrel(i) : S2_fzrel(i); };
        auto tileCern_fxrel = [this](G4int i) { return IsCERNS6() ? S6_fxrel(i) : S2_fxrel(i); };
        auto tileCern_fyrel = [this](G4int i) { return IsCERNS6() ? S6_fyrel(i) : S2_fyrel(i); };   

        if (IsCERNS6()) {
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

            tileCern_sign = IsCERNS6() ? -1 : ((i+1)%2 ? -1 : 1);

            G4String passiveName = "tileCern_passive" + std::to_string(i);
            G4String tileName = "tileCern" + std::to_string(i);
            G4String fibreName = "tileCern_fibre" + std::to_string(i);

            // passive layers
            pos_temp = G4ThreeVector(0, tileCern_beamref_shift, z_tileCern_front + tileCern_fzrel(i) - (passive_thk_gross + tileCern_thk)/2 - (passive_thk_gross - 2*passive_thk_gap - passive_thk_u)/2);
            pos_rot_temp = G4Translate3D(pos_temp) * G4Rotate3D(passive_ang_x, G4Vector3D(1,0,0)) * G4Rotate3D(passive_ang_y, G4Vector3D(0,1,0)) * G4Rotate3D(passive_ang_z, G4Vector3D(0,0,1));
            tileCern_lvols_passive[2*i] = fLogTile(passiveName, SS, grey, passive_geom_u, passive_thk_u, 0, 0, 0, 0);
            new G4PVPlacement(pos_rot_temp, tileCern_lvols_passive[2*i], passiveName + "_Phys", worldLog, false, i);

            pos_temp = G4ThreeVector(0, tileCern_beamref_shift, z_tileCern_front + tileCern_fzrel(i) - (passive_thk_gross + tileCern_thk)/2 + (passive_thk_gross - 2*passive_thk_gap - passive_thk_d)/2);
            pos_rot_temp = G4Translate3D(pos_temp) * G4Rotate3D(passive_ang_x, G4Vector3D(1,0,0)) * G4Rotate3D(passive_ang_y, G4Vector3D(0,1,0)) * G4Rotate3D(passive_ang_z, G4Vector3D(0,0,1));
            tileCern_lvols_passive[2*i+1] = fLogTile(passiveName, SS, grey, passive_geom_d, passive_thk_d, 0, 0, 0, 0);
            new G4PVPlacement(pos_rot_temp, tileCern_lvols_passive[2*i+1], passiveName + "_Phys", worldLog, false, i);

            // tiles...
            pos_temp = G4ThreeVector(tileCern_fxrel(i), tileCern_beamref_shift + tileCern_fyrel(i), z_tileCern_front + tileCern_fzrel(i));
            pos_rot_temp = G4Translate3D(pos_temp) * G4Rotate3D(tileCern_ang_x, G4Vector3D(1,0,0)) * G4Rotate3D(tileCern_ang_y, G4Vector3D(0,1,0)) * G4Rotate3D(tileCern_ang_z, G4Vector3D(0,0,1));
            tileCern_lvols[i] = fLogTile(tileName, plastic, cyan, tileCern_geom, tileCern_thk, tileCern_sign, tileCern_holeradius, tileCern_holex, tileCern_holey);
            new G4PVPlacement(pos_rot_temp, tileCern_lvols[i], tileName + "_Phys", worldLog, false, i);

            // ... + fibres
            tileCern_lvols_fibres[i] = fLogPlaceFibreCirc(fibreName, plastic_fibre, green, tileCern_geom, worldLog, tileCern_fibreradius, 2., 200., pos_rot_temp, tileCern_sign);
        }

        tileCern_geom->RmHorGaps();

    }
    //// CERN stack ////
    ////////////////////

    ///////////////////////
    //// Pb glass calo ////
    if (IsPbGl()) {

        G4double pbGlThickness = 40 * cm;
        G4double pbGlWidth = 10 * cm;
        G4double pbGlHeight = 10 * cm;

        G4double pbGlOffsetHorizontal = 0*cm;
        G4double pbGlOffsetVertical = -0.5*cm;
        G4VSolid* pbGlBox = new G4Box("pbGl", pbGlWidth / 2, pbGlHeight / 2, pbGlThickness / 2);
        G4LogicalVolume* pbGlLog = new G4LogicalVolume(pbGlBox, pbGl, "pbGl_Log");
        pbGlLog->SetVisAttributes(brown);
        new G4PVPlacement(nullptr, G4ThreeVector(pbGlOffsetHorizontal, pbGlOffsetVertical, z_pbGl_front + pbGlThickness / 2), pbGlLog, "pbGl_Phys", worldLog, false, 0);

    }
    //// Pb glass calo ////
    ///////////////////////

    ////////////////////////////
    //// scintillating pads ////
    if (IsScintiSmall()) {
        G4double scintiSmallThickness = 0.5*cm;
        G4double scintiSmallWidth = 10.5*cm;
        G4double scintiSmallHeight = 11.5*cm;

        G4double scintiSmall0OffsetHorizontal = 0*cm;
        G4double scintiSmall0OffsetVertical = 0*cm;
        G4VSolid* scintiSmall0Box = new G4Box("scintiSmall0", scintiSmallWidth / 2, scintiSmallHeight / 2, scintiSmallThickness / 2);
        G4LogicalVolume* scintiSmall0Log = new G4LogicalVolume(scintiSmall0Box, plastic_ancillary, "scintiSmall0_Log");
        scintiSmall0Log->SetVisAttributes(blue);
        new G4PVPlacement(nullptr, G4ThreeVector(scintiSmall0OffsetHorizontal, scintiSmall0OffsetVertical, z_scintiSmall0_front + scintiSmallThickness / 2), scintiSmall0Log, "scintiSmall0_Phys", worldLog, false, 0);

        G4double scintiSmall1OffsetHorizontal = 0*cm;
        G4double scintiSmall1OffsetVertical = -0.25*cm;
        G4VSolid* scintiSmall1Box = new G4Box("scintiSmall1", scintiSmallWidth / 2, scintiSmallHeight / 2, scintiSmallThickness / 2);
        G4LogicalVolume* scintiSmall1Log = new G4LogicalVolume(scintiSmall1Box, plastic_ancillary, "scintiSmall1_Log");
        scintiSmall1Log->SetVisAttributes(blue);
        new G4PVPlacement(nullptr, G4ThreeVector(scintiSmall1OffsetHorizontal, scintiSmall1OffsetVertical, z_scintiSmall1_front + scintiSmallThickness / 2), scintiSmall1Log, "scintiSmall1_Phys", worldLog, false, 0);
    }

    if (IsScintiBig()) {
        G4double scintiBigThickness = 1*cm;
        G4double scintiBigWidth = 12*cm;
        G4double scintiBigHeight = 11*cm;

        G4double scintiBig0OffsetHorizontal = 0*cm;
        G4double scintiBig0OffsetVertical = -0.4*cm;
        G4VSolid* scintiBig0Box = new G4Box("scintiBig0", scintiBigWidth / 2, scintiBigHeight / 2, scintiBigThickness / 2);
        G4LogicalVolume* scintiBig0Log = new G4LogicalVolume(scintiBig0Box, plastic_ancillary, "scintiBig0_Log");
        scintiBig0Log->SetVisAttributes(blue);
        new G4PVPlacement(nullptr, G4ThreeVector(scintiBig0OffsetHorizontal, scintiBig0OffsetVertical, z_scintiBig0_front + scintiBigThickness / 2), scintiBig0Log, "scintiBig0_Phys", worldLog, false, 0);

        G4double scintiBig1OffsetHorizontal = 0*cm;
        G4double scintiBig1OffsetVertical = 0*cm;
        G4VSolid* scintiBig1Box = new G4Box("scintiBig1", scintiBigWidth / 2, scintiBigHeight / 2, scintiBigThickness / 2);
        G4LogicalVolume* scintiBig1Log = new G4LogicalVolume(scintiBig1Box, plastic_ancillary, "scintiBig1_Log");
        scintiBig1Log->SetVisAttributes(blue);
        new G4PVPlacement(nullptr, G4ThreeVector(scintiBig1OffsetHorizontal, scintiBig1OffsetVertical, z_scintiBig1_front + scintiBigThickness / 2), scintiBig1Log, "scintiBig1_Phys", worldLog, false, 0);

    }
    //// scintillating pads ////
    ////////////////////////////

    /////////////////////////
    //// Cherenkov pipes ////
    if (IsCher()) {

        G4double cherOuterRadius = 9*cm;
        G4double cherInnerRadius = 8*cm;

        G4double cher0Length = 298*cm;
        G4VSolid* cher0Solid = new G4Tubs("cher0", cherInnerRadius, cherOuterRadius, cher0Length/2, 0, 2*pi);
        G4VSolid* cher0VacuumSolid = new G4Tubs("cher0Vacuum", 0, cherInnerRadius, (cher0Length-2*cm)/2, 0, 2*pi);
        G4VSolid* cher0CapSolid = new G4Tubs("cher0", 0, cherOuterRadius, thk_cher0_cap, 0, 2*pi);
        G4LogicalVolume* cher0Log = new G4LogicalVolume(cher0Solid, SS, "cher0_Log");
        G4LogicalVolume* cher0VacuumLog = new G4LogicalVolume(cher0VacuumSolid, cherGas, "cher0Vacuum_Log");
        G4LogicalVolume* cher0CapLog = new G4LogicalVolume(cher0CapSolid, aluminium, "cher0Cap_Log"); // Cherenkov #1 has aluminium windows
        cher0Log->SetVisAttributes(grey);
        cher0VacuumLog->SetVisAttributes(invisible);
        cher0CapLog->SetVisAttributes(grey); // Cherenkov #1 has aluminium windows
	    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, z_cher0_front + cher0Length/2), cher0Log, "cher0_Phys", worldLog, false, 0);
        new G4PVPlacement(nullptr, G4ThreeVector(0, 0, z_cher0_front + cher0Length/2), cher0VacuumLog, "cher0Vacuum_Phys", worldLog, false, 0);
        new G4PVPlacement(nullptr, G4ThreeVector(0, 0, z_cher0_front - thk_cher0_cap/2), cher0CapLog, "cher0Cap_Phys_0", worldLog, false, 0);
        new G4PVPlacement(nullptr, G4ThreeVector(0, 0, z_cher0_front + cher0Length + thk_cher0_cap/2), cher0CapLog, "cher0Cap_Phys_1", worldLog, false, 0);

        G4double cher1Length = 259*cm;
        G4VSolid* cher1Solid = new G4Tubs("cher1", cherInnerRadius, cherOuterRadius, cher1Length/2, 0, 2*pi);
        G4VSolid* cher1VacuumSolid = new G4Tubs("cher1Vacuum", 0, cherInnerRadius, (cher1Length-2*cm)/2, 0, 2*pi);
        G4VSolid* cher1CapSolid = new G4Tubs("cher1", 0, cherOuterRadius, thk_cher1_cap, 0, 2*pi);
        G4LogicalVolume* cher1Log = new G4LogicalVolume(cher1Solid, SS, "cher1_Log");
        G4LogicalVolume* cher1VacuumLog = new G4LogicalVolume(cher1VacuumSolid, cherGas, "cher1Vacuum_Log");
        G4LogicalVolume* cher1CapLog = new G4LogicalVolume(cher1CapSolid, mylar, "cher1Cap_Log"); // Cherenkov #2 has Mylar windows
        cher1Log->SetVisAttributes(grey);
        cher1VacuumLog->SetVisAttributes(invisible);
        cher1CapLog->SetVisAttributes(brown); // Cherenkov #2 has Mylar windows
	    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, z_cher1_front + cher1Length/2), cher1Log, "cher1_Phys", worldLog, false, 0);
        new G4PVPlacement(nullptr, G4ThreeVector(0, 0, z_cher1_front + cher1Length/2), cher1VacuumLog, "cher1Vacuum_Phys", worldLog, false, 0);
        new G4PVPlacement(nullptr, G4ThreeVector(0, 0, z_cher1_front - thk_cher1_cap/2), cher1CapLog, "cher1Cap_Phys_0", worldLog, false, 0);
        new G4PVPlacement(nullptr, G4ThreeVector(0, 0, z_cher1_front + cher1Length + thk_cher1_cap/2), cher1CapLog, "cher1Cap_Phys_1", worldLog, false, 0);

    }
    //// Cherenkov pipes ////
    /////////////////////////

    ////////////////////////////
    //// beamline hodoscope ////
    if (IsHodo()) {

        G4double hodoThickness = 2 * cm;
        G4double hodoWidth = 10 * cm;
        G4double hodoHeight = 10 * cm;

        G4double hodoOffsetHorizontal = 0*cm;
        G4double hodoOffsetVertical = 0*cm;
        G4VSolid* hodoBox = new G4Box("hodo", hodoWidth / 2, hodoHeight / 2, hodoThickness / 2);
        G4LogicalVolume* hodoLog = new G4LogicalVolume(hodoBox, plastic_ancillary, "hodo_Log");
        hodoLog->SetVisAttributes(magenta);
        new G4PVPlacement(nullptr, G4ThreeVector(hodoOffsetHorizontal, hodoOffsetVertical, z_hodo_centre), hodoLog, "hodo_Phys", worldLog, false, 0);

    }
    //// beamline hodoscope ////
    ////////////////////////////

    //////////////////////////////////////////
    //// additional steel layers upstream ////
    if (IsConfigCERNS6Upstr1() || IsConfigCERNS6Upstr2() || IsConfigCERNS6Upstr4()) {

        G4double addPassiveOffsetHorizontal = 0*cm;
        G4double addPassiveOffsetVertical = -1.6*cm + tileCern_beamref_shift;

        G4int nAddPassive;
        if (IsConfigCERNS6Upstr1()) {nAddPassive=1;}
        else if (IsConfigCERNS6Upstr2() ) {nAddPassive=2;}
        else {nAddPassive=4;}

        geomRectangle* passive_geom_u = new geomRectangle(passive_w_u, passive_h);
        geomRectangle* passive_geom_d = new geomRectangle(passive_w_d, passive_h);

        G4LogicalVolume* tileCern_lvols_passive_additional[8];

        for (G4int i = 0; i < nAddPassive; i++) {

            G4String passiveName = "tileCern_additionalPassive" + std::to_string(i);

            pos_temp = G4ThreeVector(0, addPassiveOffsetVertical, z_additionalPassive_rear - (i+0.5)*passive_thk_gross - (passive_thk_gross - 2*passive_thk_gap - passive_thk_u)/2);
            pos_rot_temp = G4Translate3D(pos_temp) * G4Rotate3D(passive_ang_x, G4Vector3D(1,0,0)) * G4Rotate3D(passive_ang_y, G4Vector3D(0,1,0)) * G4Rotate3D(passive_ang_z, G4Vector3D(0,0,1));
            tileCern_lvols_passive_additional[2*i] = fLogTile(passiveName, SS, grey, passive_geom_u, passive_thk_u, 0, 0, 0, 0);
            new G4PVPlacement(pos_rot_temp, tileCern_lvols_passive_additional[2*i], passiveName + "_Phys", worldLog, false, i);

            pos_temp = G4ThreeVector(0, addPassiveOffsetVertical, z_additionalPassive_rear - (i+0.5)*passive_thk_gross + (passive_thk_gross - 2*passive_thk_gap - passive_thk_d)/2);
            pos_rot_temp = G4Translate3D(pos_temp) * G4Rotate3D(passive_ang_x, G4Vector3D(1,0,0)) * G4Rotate3D(passive_ang_y, G4Vector3D(0,1,0)) * G4Rotate3D(passive_ang_z, G4Vector3D(0,0,1));
            tileCern_lvols_passive_additional[2*i+1] = fLogTile(passiveName, SS, grey, passive_geom_d, passive_thk_d, 0, 0, 0, 0);
            new G4PVPlacement(pos_rot_temp, tileCern_lvols_passive_additional[2*i+1], passiveName + "_Phys", worldLog, false, i);

        }

    }
    //// additional steel layers upstream ////
    //////////////////////////////////////////

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
    if (IsFZU()) {
        for (G4int i = 0; i < TILEFZU_N; i++) {
            G4String FZUName = "FZU" + std::to_string(i);
            VolumeEDepSD* FZUSD = new VolumeEDepSD("SD_FZU" + std::to_string(i));
            SetSensitiveDetector(FZUName + "_Log", FZUSD);
            sdm->AddNewDetector(FZUSD);
        }
    }
    //// FZU stack ////
    ///////////////////

    ////////////////////////////////////
    //// CERN stack - trigger tiles ////
    if (IsCERNTrig()) {
        for (G4int i = 0; i < 2; i++) {
            G4String tileTriggerName = "tileCernTrigger" + std::to_string(i);
            VolumeEDepSD* tileCernTriggerSD = new VolumeEDepSD("SD_tileCernTrigger" + std::to_string(i));
            SetSensitiveDetector(tileTriggerName + "_Log", tileCernTriggerSD);
            sdm->AddNewDetector(tileCernTriggerSD);
        }
    }
    //// CERN stack - trigger tiles ////
    ////////////////////////////////////

    ////////////////////
    //// CERN stack ////
    if (IsCERNAny()) {
        for (G4int i = 0; i < TILECERN_N; i++) {
            G4String tileName = "tileCern" + std::to_string(i);
            VolumeEDepSD* tileCernSD = new VolumeEDepSD("SD_tileCern" + std::to_string(i));
            SetSensitiveDetector(tileName + "_Log", tileCernSD);
            sdm->AddNewDetector(tileCernSD);
        }
    }
    //// CERN stack ////
    ////////////////////

    ///////////////////////
    //// Pb glass calo ////
    if (IsPbGl()) {
        VolumeEDepSD* pbGlSD = new VolumeEDepSD("SD_PbGl");
        SetSensitiveDetector("pbGl_Log", pbGlSD);
        sdm->AddNewDetector(pbGlSD);
    }
    //// Pb glass calo ////
    ///////////////////////

    ////////////////////////////
    //// scintillating pads ////
    if (IsScintiSmall() && IsScintiSmallDet()) {
        VolumeEDepSD* scintiSmall0SD = new VolumeEDepSD("SD_scinti0");
        SetSensitiveDetector("scintiSmall0_Log", scintiSmall0SD);
        sdm->AddNewDetector(scintiSmall0SD);

        VolumeEDepSD* scintiSmall1SD = new VolumeEDepSD("SD_scinti1");
        SetSensitiveDetector("scintiSmall1_Log", scintiSmall1SD);
        sdm->AddNewDetector(scintiSmall1SD);
    }

    if (IsScintiBig()) {
        VolumeEDepSD* scintiBig0SD = new VolumeEDepSD("SD_scinti2");
        SetSensitiveDetector("scintiBig0_Log", scintiBig0SD);
        sdm->AddNewDetector(scintiBig0SD);

        VolumeEDepSD* scintiBig1SD = new VolumeEDepSD("SD_scinti3");
        SetSensitiveDetector("scintiBig1_Log", scintiBig1SD);
        sdm->AddNewDetector(scintiBig1SD);
    }
    //// scintillating pads ////
    ////////////////////////////

    /////////////////////////
    //// Cherenkov pipes ////
    if (IsCher() && IsCherDet()) {
        VolumeEDepSD* cher0SD = new VolumeEDepSD("SD_cher0");
        SetSensitiveDetector("cher0Vacuum_Log", cher0SD);
        sdm->AddNewDetector(cher0SD);

        VolumeEDepSD* cher1SD = new VolumeEDepSD("SD_cher1");
        SetSensitiveDetector("cher1Vacuum_Log", cher1SD);
        sdm->AddNewDetector(cher1SD);
    }
    //// Cherenkov pipes ////
    /////////////////////////

    ////////////////////////////
    //// beamline hodoscope ////
    if (IsHodo() && IsHodoDet()) {
        VolumeTrackingSD* hodoSD = new VolumeTrackingSD("SD_hodo");
        SetSensitiveDetector("hodo_Log", hodoSD);
        sdm->AddNewDetector(hodoSD);
    }
    //// beamline hodoscope ////
    ////////////////////////////
    
    // --------------------------------------------------
    // ...uncomment this line for the test sensitive detectors (implemented in src/TestMode.cc)
    //SDTest(sdm);
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}

// ============================================================
// ============================================================
// implement custom methods here

// DetectorConstruction methods ///////////////////////////////

// tile shapes - CERN S2 - declared in DetectorConstruction.hh
G4double DetectorConstruction::S2_fzrel(G4int i){ // longitudinal position of single tiles, relative to front
	return floor(i/2)*(passive_thk_gross + S2_thk) + passive_thk_gross + S2_thk/2;
} 
G4double DetectorConstruction::S2_fxrel(G4int i){ // transverse (x) position of single tiles, relative to centre
	return ((i+1)%2 ? -1 : 1) * S2_xgap/2;
}
G4double DetectorConstruction::S2_fyrel(G4int i){ // transverse (y) position of single tiles, relative to centre
	return 0;
}

// tile shapes - CERN S6 - declared in DetectorConstruction.hh
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
