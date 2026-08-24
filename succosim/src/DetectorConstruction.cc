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

    G4LogicalVolume* frontLog[NSTACKEDMODSMAX] = {};
    G4LogicalVolume* backLog[NSTACKEDMODSMAX] = {};
    G4LogicalVolume* sideLogs[NSTACKEDMODSMAX][2] = {};
    G4LogicalVolume* mstLogs[NSTACKEDMODSMAX][NPERIODSMAX*NLAYERSMAX*2] = {};
    G4LogicalVolume* spcLogs[NSTACKEDMODSMAX][NPERIODSMAX*NLAYERSMAX] = {};
    G4LogicalVolume* sciLogs[NSTACKEDMODSMAX][NPERIODSMAX*NLAYERSMAX*2] = {};
    G4LogicalVolume* fibreLogs[NSTACKEDMODSMAX][NPERIODSMAX*NLAYERSMAX*2] = {};
    G4LogicalVolume* modEnvLogs[NSTACKEDMODSMAX] = {};

    const G4int n_stacked_mods = nStackedMods();

    for (G4int i = 0; i < n_stacked_mods; i++) {
        G4double ang_x_temp = ((2*i - n_stacked_mods + 1) / 2.) * mod_dphi;
        pos_temp = G4ThreeVector(0, -sin(ang_x_temp) * mod_radial_env()/2, cos(ang_x_temp) * mod_radial_env()/2);
        pos_rot_temp = 
            G4Translate3D(pos_temp)* 
            G4Translate3D(G4ThreeVector(0, 0, -mod_rmin))*
            G4Rotate3D(ang_x_temp, G4Vector3D(1,0,0))* 
            G4Rotate3D(0, G4Vector3D(0,1,0))* 
            G4Rotate3D(90*deg, G4Vector3D(0,0,1))*
            G4Translate3D(G4ThreeVector(0, 0, mod_rmin)) ;
        modEnvLogs[i] = CreateModule(
            "M" + std::to_string(i),
            frontLog[i],
            backLog[i],
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

    // function to create SD
    auto SetVolumeEDepSD = [this, sdm](const G4String& volumeName) {
        VolumeEDepSD* volumeSD = new VolumeEDepSD(volumeName + "_SD");
        SetSensitiveDetector(volumeName + "_Log", volumeSD);
        sdm->AddNewDetector(volumeSD);
    };

    // function to attach a new logical volume to existing SD
    auto AttachVolumeEDepSD = [this](const G4String& volumeName, VolumeEDepSD* volumeSD) {
        SetSensitiveDetector(volumeName + "_Log", volumeSD);
    };

    const G4int n_periods = nPeriods();
    const G4int n_layers = nLayers();
    const G4int n_stacked_mods = nStackedMods();

    for (G4int imod = 0; imod < n_stacked_mods; imod++) {
        G4String mod_prefix = "M" + std::to_string(imod);
        VolumeEDepSD* coarseSD = nullptr;
        if (COARSERO == 2) {
            coarseSD = new VolumeEDepSD(mod_prefix + "_Total_SD");
            sdm->AddNewDetector(coarseSD);
            AttachVolumeEDepSD(mod_prefix + "_Envelope", coarseSD);
        }
        VolumeEDepSD* cellSDs[NLAYERSMAX][NPERIODSMAX] = {};
        if (COARSERO == 1) {
            for (G4int j = 0; j < n_layers; j++) {
                for (G4int iperiod = 0; iperiod < n_periods; iperiod++) {
                    G4String cell_name = mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Cell";
                    cellSDs[j][iperiod] = new VolumeEDepSD(cell_name + "_SD");
                    sdm->AddNewDetector(cellSDs[j][iperiod]);
                }
            }
        }

        if (BPLACEONLYINNER) {goto label_inner_sd;}

        if (front_thk > 0) {
            if (COARSERO == 2) {AttachVolumeEDepSD(mod_prefix + "_Front", coarseSD);}
            else {SetVolumeEDepSD(mod_prefix + "_Front");}
        }
        if (back_thk > 0) {
            if (COARSERO == 2) {AttachVolumeEDepSD(mod_prefix + "_Back", coarseSD);}
            else {SetVolumeEDepSD(mod_prefix + "_Back");}
        }
        if (side_thk > 0) {
            if (COARSERO == 2) {
                AttachVolumeEDepSD(mod_prefix + "_Side0", coarseSD);
                AttachVolumeEDepSD(mod_prefix + "_Side1", coarseSD);
            } else {
                SetVolumeEDepSD(mod_prefix + "_Side0");
                SetVolumeEDepSD(mod_prefix + "_Side1");
            }
        }
        
        label_inner_sd: {;}

        for (G4int j = 0; j < n_layers; j++) {
            for (G4int i = 0; i < n_periods * 2 - 1; i++) {
                G4int iperiod = floor(i/2);

                G4String mst_name = mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Master" + std::to_string(i%2);
                if (COARSERO == 2) {AttachVolumeEDepSD(mst_name, coarseSD);}
                else if (COARSERO == 1) {AttachVolumeEDepSD(mst_name, cellSDs[j][iperiod]);}
                else {SetVolumeEDepSD(mst_name);}
            }
        }

        for (G4int j = 0; j < n_layers; j++) {
            for (G4int i = 0; i < n_periods * 2; i++) {
                G4int iperiod = floor(i/2);
            
                G4int b_spc = ((i%2) + (j%2)) % 2;
                if (b_spc) {
                    G4String spc_name = mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Spacer";
                    if (COARSERO == 2) {AttachVolumeEDepSD(spc_name, coarseSD);}
                    else if (COARSERO == 1) {AttachVolumeEDepSD(spc_name, cellSDs[j][iperiod]);}
                    else {SetVolumeEDepSD(spc_name);}
                } else {
                    for (G4int k = 0; k < 2; k++) {
                        G4String sci_name = mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Scintillator" + std::to_string(k);
                        G4String fibre_name = mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Fibre" + std::to_string(k);
                        if (COARSERO == 2) {
                            AttachVolumeEDepSD(sci_name, coarseSD);
                            AttachVolumeEDepSD(fibre_name, coarseSD);
                        } else if (COARSERO == 1) {
                            AttachVolumeEDepSD(sci_name, cellSDs[j][iperiod]);
                            AttachVolumeEDepSD(fibre_name, cellSDs[j][iperiod]);
                        } else {
                            SetVolumeEDepSD(sci_name);
                            SetVolumeEDepSD(fibre_name);
                        }
                    }
                }
            }
        }
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

// module radial extension - net (sensitive volume only) - function initialised in DetectorConstruction.hh
G4double DetectorConstruction::mod_radial() {
    G4double mod_radial_temp = 0;
    const G4int n_layers = nLayers();
    for (G4int j=0; j<n_layers; j++) {
        mod_radial_temp += (j%2) ? spc_hgt(j) : (sci_hgt(j) + 2*sci_r_gap);
        if (n_layers%2) {mod_radial_temp += spc_r_overlap;}
    }
    return mod_radial_temp;
}

// master (partial) cross-section shapes (vs layer ID) - functions initialised in DetectorConstruction.hh
G4double DetectorConstruction::mst_hgt(G4int j){
    if (j<4) {return 55*mm;}
    else if (j<10) {return 105*mm;}
    else {return 205*mm;}
}
G4double DetectorConstruction::mst_r(G4int j){
    G4double mst_rmin_temp = mod_rmin;
    if (j>0) {
        for (G4int k=0; k<j; k++) {mst_rmin_temp += mst_hgt(k);}
    }
    return mst_rmin_temp + mst_hgt(j)/2;
}

// function to create and place a whole module, initialised in DetectorConstruction.hh
G4LogicalVolume* DetectorConstruction::CreateModule(
    G4String mod_id,
    G4LogicalVolume*& frontLog, 
    G4LogicalVolume*& backLog, 
    G4LogicalVolume** sideLogs, 
    G4LogicalVolume** mstLogs, 
    G4LogicalVolume** spcLogs,
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
    geomTrapezoid* modEnvGeom = new geomTrapezoid(mod_rmin + mod_radial_env() / 2 - front_thk, mod_radial_env(), mod_dphi);
    G4LogicalVolume* modEnvLog = fLogTile(mod_id+"_Envelope", mat_envelope, col_envelope, modEnvGeom, mod_thk_env(), 0, 0, 0, 0);

    // place elements in the envelope...

    G4LogicalVolume* tilLogTemp;
    G4int iperiod;

    if (BPLACEONLYINNER) {goto label_inner_geom;}

    // --> front plate
    if (front_thk > 0) {
        geomTrapezoid* frontGeom = new geomTrapezoid(mod_rmin + front_thk / 2, front_thk, mod_dphi);
        frontLog = fLogTile(mod_id+"_Front", mat_support , col_support, frontGeom, mod_thk(), 0, 0, 0, 0);
        pos_temp = G4ThreeVector(0, 0, -mod_radial_env() / 2 + front_thk / 2);
        new G4PVPlacement(nullptr, pos_temp, frontLog, mod_id+"_Front", modEnvLog, false, 0);
    }

    // --> back plate
    if (back_thk > 0) {
        geomTrapezoid* backGeom = new geomTrapezoid(mod_rmin + mod_radial_env() - back_thk / 2, back_thk, mod_dphi);
        const G4int last_layer = nLayers() - 1;
        backGeom->AddHorGaps(spc_sidegap(last_layer)>sci_sidegap(last_layer) ? spc_sidegap(last_layer) : sci_sidegap(last_layer));
        backLog = fLogTile(mod_id+"_Back", mat_support , col_support, backGeom, mod_thk(), 0, 0, 0, 0);
        pos_temp = G4ThreeVector(0, 0, mod_radial_env() / 2 - back_thk / 2);
        new G4PVPlacement(nullptr, pos_temp, backLog, backLog->GetName(), modEnvLog, false, 0);
        backGeom->RmHorGaps();
    }

    // --> side plates
    if (side_thk > 0) {
        geomTrapezoid* sideGeom = modEnvGeom; // side plates have the same transverse cross section as envelope
        tilLogTemp = fLogTile(mod_id+"_Side0", mat_support , col_support, sideGeom, side_thk, 0, 0, 0, 0);
        pos_temp = G4ThreeVector(0, -mod_thk_env() / 2 + side_thk / 2, 0);
        new G4PVPlacement(nullptr, pos_temp, tilLogTemp, tilLogTemp->GetName(), modEnvLog, false, 0);
        sideLogs[0] = tilLogTemp;
        tilLogTemp = fLogTile(mod_id+"_Side1", mat_support , col_support, sideGeom, side_thk, 0, 0, 0, 0);
        pos_temp = G4ThreeVector(0, mod_thk_env() / 2 - side_thk / 2, 0);
        new G4PVPlacement(nullptr, pos_temp, tilLogTemp, tilLogTemp->GetName(), modEnvLog, false, 1);
        sideLogs[1] = tilLogTemp;
    }

    label_inner_geom: {;}
    
    // --> master plates
    // (splitted in radial segments separate for each layer, in order to measure the energy deposit in each period)
    G4int q_mst = 0;
    iperiod = 0;

    const G4int n_periods = nPeriods();
    const G4int n_layers = nLayers();

    for (G4int j = 0; j < n_layers; j++) {
        for (G4int i = 0; i < n_periods * 2 - 1; i++) {
            iperiod = floor(i/2);

            geomTrapezoid* mstGeom = new geomTrapezoid(mst_r(j), mst_hgt(j), mod_dphi);

            G4String mst_name = mod_id + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Master" + std::to_string(i%2);

            tilLogTemp = fLogTile(mst_name, mat_passive , col_passive, mstGeom, mst_thk, 0, 0, 0, 0);
            pos_temp = G4ThreeVector(0, mst_transv(i), mst_r_rel(j));
            new G4PVPlacement(nullptr, pos_temp, tilLogTemp, mst_name, modEnvLog, false, i);
            mstLogs[q_mst++] = tilLogTemp;
        }
    }

    // --> scintillating tiles and spacer plates
    G4int b_spc; // if true (false) place spacer (tile)
    G4double til_hgt, til_r, til_r_rel, til_sidegap, til_thk;
    G4VisAttributes* til_col;
    G4Material* til_mat;
    G4int q_sci = 0, q_spc = 0;
    iperiod = 0;

    for (G4int j = 0; j < n_layers; j++) {
        for (G4int i = 0; i < n_periods * 2; i++) {
            iperiod = floor(i/2);

            b_spc = ((i%2) + (j%2)) % 2;
            til_hgt = b_spc ? spc_hgt(j) : sci_hgt(j);
            til_r = b_spc ? spc_r(j) : sci_r(j); 
            til_r_rel = b_spc ? spc_r_rel(j) : sci_r_rel(j);
            til_sidegap = b_spc ? spc_sidegap(j) : sci_sidegap(j);
            til_thk = b_spc ? spc_thk : sci_thk;
            til_col = b_spc ? col_passive : col_scintillator;
            til_mat = b_spc ? mat_passive : mat_scintillator;

            geomTrapezoid* tilGeom = new geomTrapezoid(til_r, til_hgt, mod_dphi);
            G4String til_name;
            G4String fibre_name;

            G4double til_sidegap_extra = b_spc ? 0 : inner_gap/2; // extra side gap for scintillating tiles to account for central gap
            tilGeom->AddHorGaps(til_sidegap + til_sidegap_extra*cos(tilGeom->GetTheta()));
            if (b_spc) {
                til_name = mod_id + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Spacer";
                tilLogTemp = fLogTile(til_name, til_mat , til_col, tilGeom, til_thk, 0, hole_r, hole_x, hole_y);
                pos_temp = G4ThreeVector(0, til_transv(i), til_r_rel);
                new G4PVPlacement(nullptr, pos_temp, tilLogTemp, til_name, modEnvLog, false, 2*(i*n_layers+j));
                spcLogs[q_spc++] = tilLogTemp;
            }else{
                for (G4int k = 0; k<2; k++) {
                    G4int til_sign = k ? -1 : 1;
                    til_name = mod_id + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Scintillator" + std::to_string(k);
                    fibre_name = mod_id + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Fibre" + std::to_string(k);
                    tilLogTemp = fLogTile(til_name, til_mat , til_col, tilGeom, til_thk, til_sign, hole_r, hole_x, hole_y);
                    pos_temp = G4ThreeVector(til_sign*inner_gap/2, til_transv(i), til_r_rel);
                    new G4PVPlacement(nullptr, pos_temp, tilLogTemp, til_name, modEnvLog, false, 2*(i*n_layers+j)+k);
                    sciLogs[q_sci] = tilLogTemp;
                    fibreLogs[q_sci++] = fLogPlaceFibreCirc(fibre_name, mat_fibre, col_fibre, tilGeom, modEnvLog, fibre_r, 0, 3*cm, pos_temp, nullptr, til_sign);
                }
            }
            tilGeom->RmHorGaps();
        }
    }

    // return envelope for placing...
    return modEnvLog;
}

// --> tile-specific stuff in DetectorConstruction_tile.cc
