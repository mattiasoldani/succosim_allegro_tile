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

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// prototype ///////////////////////////////////////////////////////////////////////////////////////////////////////

    // context-specific colors and material
    G4VisAttributes* col_passive = BSHOWINNER ? grey : invisible;
    G4VisAttributes* col_support = BSHOWSUPPORT ? magenta : invisible;
    G4VisAttributes* col_scintillator = BSHOWINNER ? cyan : invisible;
    G4VisAttributes* col_fibre = BSHOWINNER ? green : invisible;
    G4VisAttributes* col_envelope = invisible;

    G4Material* mat_passive = SS;
    G4Material* mat_support = SS;
    G4Material* mat_scintillator = plastic;
    G4Material* mat_fibre = plastic;
    G4Material* mat_envelope = vacuum;

    G4LogicalVolume* frontLogs[NSTACKEDMODS] = {};
    G4LogicalVolume* backLogs[NSTACKEDMODS] = {};
    G4LogicalVolume* sideLogs[NSTACKEDMODS] = {};
    G4LogicalVolume* mstLogs[NSTACKEDMODS] = {};
    G4LogicalVolume* spcLogs[NSTACKEDMODS] = {};
    G4LogicalVolume* modEnvLogs[NSTACKEDMODS] = {};
    G4LogicalVolume* sciLogs[NSTACKEDMODS][NPERIODS*NLAYERS*2] = {};
    G4LogicalVolume* fibreLogs[NSTACKEDMODS][NPERIODS*NLAYERS*2] = {};

    for (G4int i = 0; i < NSTACKEDMODS; i++) {
        G4double ang_x_temp = ((2*i - NSTACKEDMODS + 1) / 2.) * mod_dphi;
        pos_temp = G4ThreeVector(0, -sin(ang_x_temp) * mod_radial_env/2, cos(ang_x_temp) * mod_radial_env/2);
        pos_rot_temp = 
            G4Translate3D(pos_temp)* 
            G4Translate3D(G4ThreeVector(0, 0, -mod_rmin))*
            G4Rotate3D(ang_x_temp, G4Vector3D(1,0,0))* 
            G4Rotate3D(0, G4Vector3D(0,1,0))* 
            G4Rotate3D(90*deg, G4Vector3D(0,0,1))*
            G4Translate3D(G4ThreeVector(0, 0, mod_rmin)) ;
        modEnvLogs[i] = CreateModule(
            "M" + std::to_string(i),
            frontLogs[i],
            backLogs[i],
            sideLogs[i],
            mstLogs[i],
            spcLogs[i],
            sciLogs[i],
            fibreLogs[i],
            mat_envelope, col_envelope,
            mat_passive, col_passive,
            mat_support, col_support,
            mat_scintillator, col_scintillator,
            mat_fibre, col_fibre
        );
        new G4PVPlacement(pos_rot_temp, modEnvLogs[i], modEnvLogs[i]->GetName(), worldLog, false, i);
    }

    //// prototype ///////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

    // VolumeEDepSD* tileTest000SD = new VolumeEDepSD("tile000_SD");
    // SetSensitiveDetector("tile000_Log", tileTest000SD);
    // sdm->AddNewDetector(tileTest000SD);
	
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

// module radial extension - net (sensitive volume only) - function initialised in DetectorConstruction.hh
G4double DetectorConstruction::mod_radial() {
    G4double mod_radial_temp = 0;
    for (G4int j=0; j<NLAYERS; j++) {
        mod_radial_temp += (j%2) ? spc_hgt(j) : (sci_hgt(j) + 2*sci_r_gap);
        if (NLAYERS%2) {mod_radial_temp += spc_r_overlap;}
    }
    return mod_radial_temp;
}

// spacer cross-section shapes (vs layer ID) - functions initialised in DetectorConstruction.hh
G4double DetectorConstruction::spc_hgt(G4int j){
    if (j<4) {return 55*mm;}
    else if (j<10) {return 105*mm;}
    else {return 205*mm;}
}
G4double DetectorConstruction::spc_sidegap(G4int j){
    G4int nstep = static_cast<G4int>(floor(j/8));
    return 2.6*mm*(nstep+1);
}
G4double DetectorConstruction::spc_r(G4int j){
    G4double spc_rmin_temp = mod_rmin;
    if (j>0) {
        for (G4int k=0; k<j; k++) {spc_rmin_temp += spc_hgt(k);}
    }
    return spc_rmin_temp + spc_hgt(j)/2;
}
G4double DetectorConstruction::spc_r_rel(G4int j){
    return spc_r(j) - mod_rmin - mod_radial()/2 + mod_centre_rel;
}

// scintillating tile cross-section shapes (vs layer ID) - functions initialised in DetectorConstruction.hh
G4double DetectorConstruction::sci_hgt(G4int j){
    return spc_hgt(j) - 2*(spc_r_overlap + sci_r_gap);
}
G4double DetectorConstruction::sci_sidegap(G4int j){
    return spc_sidegap(j);
}
G4double DetectorConstruction::sci_r(G4int j){
    return spc_r(j);
}
G4double DetectorConstruction::sci_r_rel(G4int j){
    return spc_r_rel(j);
}

// transverse positions (vs period ID) - functions initialised in DetectorConstruction.hh
G4double DetectorConstruction::mst_transv(G4int i){
	return -mod_thk / 2 + spc_thk * (1 + i) + mst_thk * (0.5 + i);
} 
G4double DetectorConstruction::til_transv(G4int i){
	return mst_transv(i) - (spc_thk + mst_thk) / 2;
} 

// function to create and place a whole module, initialised in DetectorConstruction.hh
G4LogicalVolume* DetectorConstruction::CreateModule(
    G4String mod_suffix,
    G4LogicalVolume*& frontLog, 
    G4LogicalVolume*& backLog, 
    G4LogicalVolume*& sideLog, 
    G4LogicalVolume*& mstLog, 
    G4LogicalVolume*& spcLog,
    G4LogicalVolume** sciLogs,
    G4LogicalVolume** fibreLogs, 
    G4Material* mat_envelope, G4VisAttributes* col_envelope,
    G4Material* mat_passive, G4VisAttributes* col_passive,
    G4Material* mat_support, G4VisAttributes* col_support,
    G4Material* mat_scintillator, G4VisAttributes* col_scintillator,
    G4Material* mat_fibre, G4VisAttributes* col_fibre
)
{

	// generic translation and roto-translation, to be applied element-by-element
	G4ThreeVector pos_temp;
	G4Transform3D pos_rot_temp;

    // create module envelope
    geomTrapezoid* modEnvGeom = new geomTrapezoid(mod_rmin + mod_radial_env / 2 - front_thk, mod_radial_env, mod_dphi);
    G4LogicalVolume* modEnvLog = fLogTile("module_envelope_"+mod_suffix, mat_envelope, col_envelope, modEnvGeom, mod_thk_env, 0, 0, 0, 0);

    // place elements in the envelope...

    if (BPLACEONLYTILES) {goto label_tiles;}

    // --> front plate
    if (front_thk > 0) {
        geomTrapezoid* frontGeom = new geomTrapezoid(mod_rmin + front_thk / 2, front_thk, mod_dphi);
        frontLog = fLogTile("front_"+mod_suffix, mat_support , col_support, frontGeom, mod_thk, 0, 0, 0, 0);
        pos_temp = G4ThreeVector(0, 0, -mod_radial_env / 2 + front_thk / 2);
        new G4PVPlacement(nullptr, pos_temp, frontLog, "front_"+mod_suffix, modEnvLog, false, 0);
    }

    // --> back plate
    if (back_thk > 0) {
        geomTrapezoid* backGeom = new geomTrapezoid(mod_rmin + mod_radial_env - back_thk / 2, back_thk, mod_dphi);
        backGeom->AddHorGaps(spc_sidegap(NLAYERS-1)>sci_sidegap(NLAYERS-1) ? spc_sidegap(NLAYERS-1) : sci_sidegap(NLAYERS-1));
        backLog = fLogTile("back_"+mod_suffix, mat_support , col_support, backGeom, mod_thk, 0, 0, 0, 0);
        pos_temp = G4ThreeVector(0, 0, mod_radial_env / 2 - back_thk / 2);
        new G4PVPlacement(nullptr, pos_temp, backLog, "back_"+mod_suffix, modEnvLog, false, 0);
        backGeom->RmHorGaps();
    }

    // --> side plates
    if (side_thk > 0) {
        geomTrapezoid* sideGeom = modEnvGeom; // side plates have the same transverse cross section as envelope
        sideLog = fLogTile("side_"+mod_suffix, mat_support , col_support, sideGeom, side_thk, 0, 0, 0, 0);
        pos_temp = G4ThreeVector(0, -mod_thk_env / 2 + side_thk / 2, 0);
        new G4PVPlacement(nullptr, pos_temp, sideLog, "side_"+mod_suffix, modEnvLog, false, 0);
        pos_temp = G4ThreeVector(0, mod_thk_env / 2 - side_thk / 2, 0);
        new G4PVPlacement(nullptr, pos_temp, sideLog, "side_"+mod_suffix, modEnvLog, false, 1);
    }
    
    // --> master plates
    {
        geomTrapezoid* mstGeom = new geomTrapezoid(mod_rmin + mod_radial() / 2, mod_radial(), mod_dphi);
       mstLog = fLogTile("masters_"+mod_suffix, mat_passive , col_passive, mstGeom, mst_thk, 0, 0, 0, 0);
        for (G4int i = 0; i < NPERIODS * 2 - 1; i++) {
            pos_temp = G4ThreeVector(0, mst_transv(i), mod_centre_rel);
            new G4PVPlacement(nullptr, pos_temp, mstLog, "masters_"+mod_suffix, modEnvLog, false, i);
        }
    }

    label_tiles: {;}

    // --> scintillating tiles and spacer plates
    G4int b_spc; // if true (false) place spacer (tile)
    G4double til_hgt, til_r, til_r_rel, til_sidegap, til_thk;
    G4VisAttributes* til_col;
    G4Material* til_mat;
    G4LogicalVolume* tilLogTemp;
    G4int q = 0;
    for (G4int i = 0; i < NPERIODS * 2; i++) {
        for (G4int j = 0; j < NLAYERS; j++) {
            b_spc = ((i%2) + (j%2)) % 2;
            til_hgt = b_spc ? spc_hgt(j) : sci_hgt(j);
            til_r = b_spc ? spc_r(j) : sci_r(j); 
            til_r_rel = b_spc ? spc_r_rel(j) : sci_r_rel(j);
            til_sidegap = b_spc ? spc_sidegap(j) : sci_sidegap(j);
            til_thk = b_spc ? spc_thk : sci_thk;
            til_col = b_spc ? col_passive : col_scintillator;
            til_mat = b_spc ? mat_passive : mat_scintillator;

            geomTrapezoid* tilGeom = new geomTrapezoid(til_r, til_hgt, mod_dphi);
            G4String til_name = "tile_" + mod_suffix + "_P" + std::to_string(i) + "_L" + std::to_string(j);
            G4String fibre_name = "fibre" + mod_suffix + "_P" + std::to_string(i) + "_L" + std::to_string(j);

            G4double til_sidegap_extra = b_spc ? 0 : inner_gap/2; // extra side gap for scintillating tiles to account for central gap
            tilGeom->AddHorGaps(til_sidegap + til_sidegap_extra*cos(tilGeom->GetTheta()));
            if (b_spc) {
                spcLog = fLogTile(til_name, til_mat , til_col, tilGeom, til_thk, 0, hole_r, hole_x, hole_y);
                pos_temp = G4ThreeVector(0, til_transv(i), til_r_rel);
                new G4PVPlacement(nullptr, pos_temp, spcLog, til_name, modEnvLog, false, 2*(i*NLAYERS+j));
            }else{
                for (G4int k = 0; k<2; k++) {
                    G4int til_sign = k ? -1 : 1;
                    tilLogTemp = fLogTile(til_name, til_mat , til_col, tilGeom, til_thk, til_sign, hole_r, hole_x, hole_y);
                    pos_temp = G4ThreeVector(til_sign*inner_gap/2, til_transv(i), til_r_rel);
                    new G4PVPlacement(nullptr, pos_temp, tilLogTemp, til_name, modEnvLog, false, 2*(i*NLAYERS+j)+k);
                    sciLogs[q] = tilLogTemp;
                    fibreLogs[q] = fLogPlaceFibreCirc(fibre_name, mat_fibre, col_fibre, tilGeom, modEnvLog, fibre_r, 0, 3*cm, pos_temp, nullptr, til_sign);
                    q++;
                }
            }
            tilGeom->RmHorGaps();
        }
    }

    // return envelope for placing...
    return modEnvLog;
}

// --> tile-specific stuff in DetectorConstruction_tile.cc
