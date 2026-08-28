#include <G4SystemOfUnits.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4NistManager.hh>
#include <G4SystemOfUnits.hh>
#include <G4VisAttributes.hh>
#include <G4SDManager.hh>
#include <G4LogicalVolumeStore.hh>
#include <G4HCofThisEvent.hh>
#include <G4UniformMagField.hh>
#include <G4FieldManager.hh>
#include <G4TransportationManager.hh>
#include <G4ChordFinder.hh>
#include <G4MultiFunctionalDetector.hh>
#include <G4Box.hh>
#include <G4Trd.hh>

#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <sstream>

#include "DetectorConstruction.hh"
#include "Analysis.hh"
#include "CustomSD.hh"
#include "CustomHit.hh"

// DetectorConstruction general methods ///////////////////////

// create the full tile solid
G4VSolid* DetectorConstruction::fShapeTileFull(
    G4String name, 
    geomTrapezoid* pGeom, 
    G4double dThk
){
    G4double dHor_b = pGeom->GetDHor_b();
    G4double dHor_t = pGeom->GetDHor_t();
    G4double dVer = pGeom->GetDVer();

    G4String name_solid = name + "_Shape";
    G4VSolid* shape = new G4Trd(name_solid, dHor_b/2, dHor_t/2, dThk/2, dThk/2, dVer/2);

    return shape;
}

// create the tile (customised) logical volume
G4LogicalVolume* DetectorConstruction::fLogTile(
    G4String name, 
    G4Material* pMaterial, 
    G4VisAttributes* pColour,
    geomTrapezoid* pGeom, 
    G4double dThk,

    G4int signHalf = 0, // if 0, full tile is kept; if > (<) 0 , half tile is selected from positive (negative) half

    G4double holeR = 0., // radius of the service holes - if 0, no holes are modelled
    G4double holeX = 0., // hole centre, horizontal distance from tile centre
    G4double holeY = 0. // hole centre, vertical distance from tile centre
    // note: two holes are always added to the full tile: one in (holeX, holey), one in (-holeX, -holey)
){
    G4VSolid* shape_full = fShapeTileFull(name, pGeom, dThk);
    G4VSolid* shape_fin;

    G4String name_solid = name + "_Shape";

    // model service holes
    G4VSolid* shape_holes;
    if (holeR > 0) {
        G4RotationMatrix* holeRotation = new G4RotationMatrix();
        holeRotation->rotateX(90 * deg);

        G4VSolid* shape_hole = new G4Tubs("hole_Shape", 0., holeR, 1.002*dThk / 2, 0., 2*pi);

        G4VSolid* shape_holes_temp = new G4SubtractionSolid(
            "temp", shape_full, shape_hole, holeRotation, G4ThreeVector(holeX, 0., holeY)
        );
        shape_holes = new G4SubtractionSolid(
            name_solid, shape_holes_temp, shape_hole, holeRotation, G4ThreeVector(-holeX, 0., -holeY)
        );
    }
    shape_fin = holeR > 0 ? shape_holes : shape_full;
    
    // turn full tile into half tile
    G4VSolid* shape_halved;
    if (signHalf) {
        G4int sign = (signHalf > 0) ? 1 : -1 ;

        G4double dHor_b = pGeom->GetDHor_b();
        G4double dHor_t = pGeom->GetDHor_t();
        G4double dHor_max = dHor_t > dHor_b ? dHor_t : dHor_b ;
        G4double dVer = pGeom->GetDVer();
        G4VSolid* shape_rm = new G4Box("rm_Shape", dHor_max / 4, dThk / 2, dVer / 2);

        G4VSolid* shape_intersect = new G4IntersectionSolid(
            name_solid, shape_fin, shape_rm, nullptr, G4ThreeVector(sign*dHor_max/4, 0., 0.)
        );

        shape_halved = shape_intersect;
    }
    shape_fin = signHalf ? shape_halved : shape_fin ;

    G4String name_log = name + "_Log";
    G4LogicalVolume* logvol = new G4LogicalVolume(shape_fin, pMaterial, name_log);
    logvol->SetVisAttributes(pColour);

    return logvol;
}

// create and place the optical fibre next to a tile
// overloaded version using G4ThreeVector+G4RotationMatrix* for envelope roto-translation instead of G4Transform3D, for back-compatibility
G4LogicalVolume* DetectorConstruction::fLogPlaceFibreCirc(
    G4String name, 
    G4Material* pMaterial, 
    G4VisAttributes* pColour,
    geomTrapezoid* pTileGeom, 
    G4LogicalVolume* pEnvelope, // logical volume in which to place the fibre

    G4double sectionR, // fibre section radius
    G4double extraRIn, // fibre extension (along the wedge side) towards inner radii (i.e. towards collision point)
    G4double extraROut, // fibre extension (along the wedge side) towards outer radii

    G4ThreeVector tilePos, // central coordinates used for tile placement
    G4RotationMatrix* pTileRot = nullptr, // rotation matrix used for tile placement
    G4int signHalf = 0 // see signHalf in tile logical volume creation
){
	G4Transform3D tilePosRot = pTileRot
        ? G4Translate3D(tilePos) * G4Rotate3D(*pTileRot)
        : G4Translate3D(tilePos);
		
	return fLogPlaceFibreCirc(name, pMaterial, pColour, pTileGeom, pEnvelope, sectionR, extraRIn, extraROut, tilePosRot, signHalf);
}

// create and place the optical fibre next to a tile
G4LogicalVolume* DetectorConstruction::fLogPlaceFibreCirc(
    G4String name, 
    G4Material* pMaterial, 
    G4VisAttributes* pColour,
    geomTrapezoid* pTileGeom, 
    G4LogicalVolume* pEnvelope, // logical volume in which to place the fibre

    G4double sectionR, // fibre section radius
    G4double extraRIn, // fibre extension (along the wedge side) towards inner radii (i.e. towards collision point)
    G4double extraROut, // fibre extension (along the wedge side) towards outer radii

    G4Transform3D tilePosRot, // general 3D transformation for tile placement
    G4int signHalf = 0 // see signHalf in tile logical volume creation
){
    // create shape and logical volume (the latter will be returned)
    G4double length_contact = pTileGeom->GetDSide();
    G4double length_total = length_contact + extraRIn + extraROut;

    G4String name_solid = name + "_Shape";
    G4VSolid* shape = new G4Tubs(name_solid, 0., sectionR, length_total/2, 0., 2*pi);

    G4String name_log = name + "_Log";
    G4LogicalVolume* logvol = new G4LogicalVolume(shape, pMaterial, name_log);
    logvol->SetVisAttributes(pColour);

    // placement of the physical volume
    G4NistManager* nist = G4NistManager::Instance();
	G4Material* air = nist->FindOrBuildMaterial("G4_AIR");

    G4String name_phys = name + "_Phys";

    G4double tilt = pTileGeom->GetTheta()/2;
    G4double fibre_centre_shift = 0.5*( length_total - 2*extraRIn - length_contact );
    G4double pos_tran_ver = fibre_centre_shift*cos(tilt); // add vertical shift to the centre of the fibre part in contact
    G4double pos_tran_hor;
    G4ThreeVector pos_internal = G4ThreeVector(0, 0, pos_tran_ver); // x is set below (once sign is defined)
    G4RotationMatrix* rot_internal;

    G4int sign;
    G4int sign0 = signHalf ? ((signHalf > 0) ? 1 : -1) : 0;
    G4int isignlim = signHalf ? 0 : 1;

    // G4VSolid* envelopeInternalBox = new G4Box(name + "_Shape_EnvIntNoRot", 0.5*(pTileGeom->GetDHor_t() + sectionR), sectionR/2, length_total/2);
    // G4LogicalVolume* pEnvelopeInternal = new G4LogicalVolume(envelopeInternalBox, air, name + "_Log_EnvIntNoRot");
    geomTrapezoid pTileGeomExtended = *pTileGeom;
    pTileGeomExtended.SetDHor_b(true, pTileGeom->GetDHor_b() + sectionR/cos(tilt) * (signHalf ? 2 : 4));
    pTileGeomExtended.SetDHor_t(true, pTileGeom->GetDHor_t() + sectionR/cos(tilt) * (signHalf ? 2 : 4));
    G4LogicalVolume* pEnvelopeInternal = fLogTile(name + "_EnvIntNoRot", pMaterial, pColour, &pTileGeomExtended, sectionR, sign0);
    G4VisAttributes* visAttrEnvelopeInternal = new G4VisAttributes();
    visAttrEnvelopeInternal->SetVisibility(false);
    pEnvelopeInternal->SetVisAttributes(visAttrEnvelopeInternal);

    for(int isign=0; isign<=isignlim; isign++){
    // if half module, fibre only on one side
    // if full module, fibre is in principle on both sides (set signHalf properly to only have it on one side anyway)
        sign = signHalf ? ((signHalf > 0) ? 1 : -1) : (isign ? -1 : 1);
        pos_tran_hor = sign*(pTileGeom->GetDHor_mid()/2 + sectionR/cos(tilt)); // move horizontally centre of the total fibre, then...
        pos_tran_hor += sign*fibre_centre_shift*sin(tilt); // ... add horizontal shift to the centre of the fibre part in contact
        pos_internal.setX(pos_tran_hor);
        rot_internal = new G4RotationMatrix();
        rot_internal->rotateY(-sign*tilt);
        new G4PVPlacement(rot_internal, pos_internal, logvol, name + "_Phys_EnvIntNoRot", pEnvelopeInternal, false, 0);
    }

    //pTileRot->setAxis(tilePos);
    new G4PVPlacement(tilePosRot, pEnvelopeInternal, name_phys, pEnvelope, false, 0);

    return logvol;
}

// fullTileCalModule methods //////////////////////////////////

// module constructor
DetectorConstruction::fullTileCalModule::fullTileCalModule(
    G4String id,
    DetectorConstruction::fullTileCalModule::ModConfig& config
)
    : 
      // global construction settings - to be set, mandatory
      mod_id(id), // module name (will be used in all volume names)
      n_periods(config.n_periods), // number of longitudinal periods
      n_layers(config.n_layers), // number of radial layers
      b_place_support(config.b_place_support), // if true (false), place (hide) external support

      // single-element thicknesses which define the period thickness - to be set, optional
      mst_thk(config.mst_thk), // master thickness
      spc_thk(config.spc_thk), // spacer thickness
      sci_thk(config.sci_thk), // tile thickness

      // support/catcher specific features - to be set, optional
      front_thk(config.front_thk), // thickness (along radial direction) of the front plate
      back_thk(config.back_thk), // thickness (along radial direction) of the back structure
      side_thk(config.side_thk), // thickness (along longitudinal direction) of the side plates
      catcher_thk(config.catcher_thk), // thickness of all catcher plates around the envelope

      // global module features - to be set, optional
      mod_rmin(config.mod_rmin), // module minimum radius - net (sensitive volume only)
      mod_dphi(config.mod_dphi), // module full azimuthal opening - readout segmentation will be halved

      // layer/period settings - to be set, optional
      n_layers_A(config.n_layers_A), // number of tier-A layers
      n_layers_B(config.n_layers_B), // number of tier-B layers
      n_layer_per_profile(config.n_layer_per_profile),
      til_hgt_A(config.til_hgt_A), // height of tier-A layers
      til_hgt_B(config.til_hgt_B), // height of tier-B layers
      til_hgt_C(config.til_hgt_C), // height of tier-C layers

      // spacer/scintillating tile cross-section shapes - to be set, optional
      spc_r_overlap(config.spc_r_overlap), // overlapping portion between spacers in two successive columns along radius
      sci_r_gap(config.sci_r_gap), // 1-side air gap between spacer and scintillator tile (removed from scintillator) along radius

      // other miscellaneous geometric parameters - to be set, optional
      fibre_profile_thk(config.fibre_profile_thk), // cross-section of the single fibre profile
      inner_gap(config.inner_gap), // gap between adjacent scintillating tiles in the same module slot (full) - side gap will be increased accordingly, to keep sides aligned with spacers
      fibre_r(config.fibre_r), // WLS fibre radius
      hole_r(config.hole_r), // radius of the pipe/rod holes in the tiles
      hole_x(config.hole_x), // x position of the pipe/rod holes in the tiles
      hole_y(config.hole_y), // y position of the pipe/rod holes in the tiles
      l_fibre_extra(config.l_fibre_extra), // out-of-tile extra length of the fibre towards the drawer

      // material settings - to be set, optional
      mat_passive(config.mat_passive),
      mat_support(config.mat_support),
      mat_scintillator(config.mat_scintillator),
      mat_fibre(config.mat_fibre),
      mat_envelope(config.mat_envelope),
      mat_catcher(config.mat_catcher),

      // visual settings - to be set, optional
      col_passive(config.col_passive),
      col_support(config.col_support),
      col_scintillator(config.col_scintillator),
      col_fibre(config.col_fibre),
      col_envelope(config.col_envelope),
      col_catcher_front_back(config.col_catcher_front_back),
      col_catcher_side(config.col_catcher_side),
      col_catcher_phi(config.col_catcher_phi)
{CreateLog();}

// function to place the logical volume(s) resulting from the constructor inside a higher-level module, initialised in DetectorConstruction.hh
void DetectorConstruction::fullTileCalModule::PlaceLog(
    G4LogicalVolume* pEnvelope,
    G4Transform3D& stack_pos_rot,
    G4bool b_place_catcher_front,
    G4bool b_place_catcher_back,
    G4bool b_place_catcher_side0,
    G4bool b_place_catcher_side1,
    G4bool b_place_catcher_phi0,
    G4bool b_place_catcher_phi1
){
    G4int copy_no = (mod_id.size() > 1) ? std::atoi(mod_id.substr(1).c_str()) : 0;
    G4ThreeVector pos_temp;

    new G4PVPlacement(stack_pos_rot, modEnvLog, modEnvLog->GetName(), pEnvelope, false, copy_no);

    if (b_place_catcher_front) {
        pos_temp = G4ThreeVector(0, 0, -(mod_radial_env()/2 + catcher_thk/2));
        new G4PVPlacement(stack_pos_rot*G4Translate3D(pos_temp), catcherFrontLog, catcherFrontLog->GetName(), pEnvelope, false, copy_no);
    }

    if (b_place_catcher_back) {
        pos_temp = G4ThreeVector(0, 0, mod_radial_env()/2 + catcher_thk/2);
        new G4PVPlacement(stack_pos_rot*G4Translate3D(pos_temp), catcherBackLog, catcherBackLog->GetName(), pEnvelope, false, copy_no);
    }

    if (b_place_catcher_side0) {
        pos_temp = G4ThreeVector(0, -(mod_thk_env()/2 + catcher_thk/2), 0);
        new G4PVPlacement(stack_pos_rot*G4Translate3D(pos_temp), catcherSideLogs[0], catcherSideLogs[0]->GetName(), pEnvelope, false, copy_no);
    }

    if (b_place_catcher_side1) {
        pos_temp = G4ThreeVector(0, mod_thk_env()/2 + catcher_thk/2, 0);
        new G4PVPlacement(stack_pos_rot*G4Translate3D(pos_temp), catcherSideLogs[1], catcherSideLogs[1]->GetName(), pEnvelope, false, copy_no);
    }

    for (G4int j=0; j<2; j++) {
        if ((j == 0 && !b_place_catcher_phi0) || (j == 1 && !b_place_catcher_phi1)) {continue;}

        G4double sign = 1 - 2*j;
        pos_temp = G4ThreeVector(
            sign*(modEnvGeom->GetDHor_mid()/2 + catcher_thk*cos(mod_dphi/2)/2),
            0,
            -catcher_thk*sin(mod_dphi/2)/2
        );

        G4Transform3D catcher_pos_rot = stack_pos_rot*
            G4Translate3D(pos_temp)*
            G4Rotate3D(sign*mod_dphi/2, G4Vector3D(0,1,0))*
            G4Rotate3D(90*deg, G4Vector3D(0,0,1));

        new G4PVPlacement(
            catcher_pos_rot,
            catcherPhiLogs[j],
            catcherPhiLogs[j]->GetName(),
            pEnvelope,
            false,
            copy_no
        );
    }
}

// function to create a whole module, used by the constructor, initialised in DetectorConstruction.hh
void DetectorConstruction::fullTileCalModule::CreateLog(){
	// generic translation and roto-translation, to be applied element-by-element
	G4ThreeVector pos_temp;
	G4Transform3D pos_rot_temp;

    // create module envelope
    modEnvGeom = new geomTrapezoid(mod_rmin_env() + mod_radial_env() / 2, mod_radial_env(), mod_dphi);
    modEnvLog = DetectorConstruction::fLogTile(mod_id+"_Envelope", mat_envelope, col_envelope, modEnvGeom, mod_thk_env(), 0, 0, 0, 0);

    // place elements in the envelope...

    G4LogicalVolume* tilLogTemp;
    G4int iperiod;

    // --> front plate
    if (b_place_support) {
        if (front_thk > 0) {
            geomTrapezoid* frontGeom = new geomTrapezoid(mod_rmin_env() + front_thk / 2, front_thk, mod_dphi);
            frontLog = DetectorConstruction::fLogTile(mod_id+"_Front", mat_support , col_support, frontGeom, mod_thk(), 0, 0, 0, 0);
            pos_temp = G4ThreeVector(0, 0, -mod_radial_env() / 2 + front_thk / 2);
            new G4PVPlacement(nullptr, pos_temp, frontLog, mod_id+"_Front", modEnvLog, false, 0);
        }
    }
    geomTrapezoid* catcherFrontGeom = new geomTrapezoid(mod_rmin_env() - catcher_thk / 2, catcher_thk, mod_dphi);
    catcherFrontLog = DetectorConstruction::fLogTile(mod_id+"_CatcherFront", mat_catcher , col_catcher_front_back, catcherFrontGeom, mod_thk_env(), 0, 0, 0, 0);

    // --> back plate
    if (b_place_support) {
        if (back_thk > 0) {
            geomTrapezoid* backGeom = new geomTrapezoid(mod_rmin_env() + mod_radial_env() - back_thk / 2, back_thk, mod_dphi);
            G4int last_layer = n_layers - 1;
            backGeom->AddHorGaps(spc_sidegap(last_layer)>sci_sidegap(last_layer) ? spc_sidegap(last_layer) : sci_sidegap(last_layer));
            backLog = DetectorConstruction::fLogTile(mod_id+"_Back", mat_support , col_support, backGeom, mod_thk(), 0, 0, 0, 0);
            pos_temp = G4ThreeVector(0, 0, mod_radial_env() / 2 - back_thk / 2);
            new G4PVPlacement(nullptr, pos_temp, backLog, backLog->GetName(), modEnvLog, false, 0);
            backGeom->RmHorGaps();
        }
    }
    geomTrapezoid* catcherBackGeom = new geomTrapezoid(mod_rmin_env() + mod_radial_env() + catcher_thk / 2, catcher_thk, mod_dphi);
    catcherBackLog = DetectorConstruction::fLogTile(mod_id+"_CatcherBack", mat_catcher , col_catcher_front_back, catcherBackGeom, mod_thk_env(), 0, 0, 0, 0);

    // --> side plates
    if (b_place_support) {
        if (side_thk > 0) {
            geomTrapezoid* sideGeom = modEnvGeom; // side plates have the same transverse cross section as envelope
            tilLogTemp = DetectorConstruction::fLogTile(mod_id+"_Side0", mat_support , col_support, sideGeom, side_thk, 0, 0, 0, 0);
            pos_temp = G4ThreeVector(0, -mod_thk_env() / 2 + side_thk / 2, 0);
            new G4PVPlacement(nullptr, pos_temp, tilLogTemp, tilLogTemp->GetName(), modEnvLog, false, 0);
            sideLogs[0] = tilLogTemp;
            tilLogTemp = DetectorConstruction::fLogTile(mod_id+"_Side1", mat_support , col_support, sideGeom, side_thk, 0, 0, 0, 0);
            pos_temp = G4ThreeVector(0, mod_thk_env() / 2 - side_thk / 2, 0);
            new G4PVPlacement(nullptr, pos_temp, tilLogTemp, tilLogTemp->GetName(), modEnvLog, false, 1);
            sideLogs[1] = tilLogTemp;
        }
    }
    geomTrapezoid* catcherSideGeom = modEnvGeom; // side catcher plates have the same transverse cross section as envelope
    catcherSideLogs[0] = DetectorConstruction::fLogTile(mod_id+"_CatcherSide0", mat_catcher , col_catcher_side, catcherSideGeom, catcher_thk, 0, 0, 0, 0);
    catcherSideLogs[1] = DetectorConstruction::fLogTile(mod_id+"_CatcherSide1", mat_catcher , col_catcher_side, catcherSideGeom, catcher_thk, 0, 0, 0, 0);

    // --> phi plates (only catchers)
    geomRectangle* catcherPhiGeom = new geomRectangle(mod_thk_env(), mod_radial_env()/cos(mod_dphi/2));
    catcherPhiLogs[0] = DetectorConstruction::fLogTile(mod_id+"_CatcherPhi0", mat_catcher , col_catcher_phi, catcherPhiGeom, catcher_thk, 0, 0, 0, 0);
    catcherPhiLogs[1] = DetectorConstruction::fLogTile(mod_id+"_CatcherPhi1", mat_catcher , col_catcher_phi, catcherPhiGeom, catcher_thk, 0, 0, 0, 0);
    
    // --> master plates
    // (splitted in radial segments separate for each layer, in order to measure the energy deposit in each period)
    G4int q_mst = 0;
    iperiod = 0;

    for (G4int j = 0; j < n_layers; j++) {
        for (G4int i = 0; i < n_periods * 2 - 1; i++) {
            iperiod = floor(i/2);

            geomTrapezoid* mstGeom = new geomTrapezoid(mst_r(j), mst_hgt(j), mod_dphi);

            G4String mst_name = mod_id + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Master" + std::to_string(i%2);

            tilLogTemp = DetectorConstruction::fLogTile(mst_name, mat_passive , col_passive, mstGeom, mst_thk, 0, 0, 0, 0);
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
                tilLogTemp = DetectorConstruction::fLogTile(til_name, til_mat , til_col, tilGeom, til_thk, 0, hole_r, hole_x, hole_y);
                pos_temp = G4ThreeVector(0, til_transv(i), til_r_rel);
                new G4PVPlacement(nullptr, pos_temp, tilLogTemp, til_name, modEnvLog, false, 2*(i*n_layers+j));
                spcLogs[q_spc++] = tilLogTemp;
            }else{
                for (G4int k = 0; k<2; k++) {
                    G4int til_sign = k ? -1 : 1;
                    til_name = mod_id + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Scintillator" + std::to_string(k);
                    fibre_name = mod_id + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Fibre" + std::to_string(k);
                    tilLogTemp = DetectorConstruction::fLogTile(til_name, til_mat , til_col, tilGeom, til_thk, til_sign, hole_r, hole_x, hole_y);
                    pos_temp = G4ThreeVector(til_sign*inner_gap/2, til_transv(i), til_r_rel);
                    new G4PVPlacement(nullptr, pos_temp, tilLogTemp, til_name, modEnvLog, false, 2*(i*n_layers+j)+k);
                    sciLogs[q_sci] = tilLogTemp;
                    fibreLogs[q_sci++] = DetectorConstruction::fLogPlaceFibreCirc(fibre_name, mat_fibre, col_fibre, tilGeom, modEnvLog, fibre_r, 0, l_fibre_extra, pos_temp, nullptr, til_sign);
                }
            }
            tilGeom->RmHorGaps();
        }
    }

    // return full envelope and catcher modules for placing...
    return;
}

// function to create all the module SDs
void DetectorConstruction::fullTileCalModule::CreateAllSDs(
    G4SDManager* sdm,
    G4int coarse_ro
){
    G4bool b_scinti_ro_only = coarse_ro == 1;
    G4bool b_cell_ro = coarse_ro == 2;
    G4bool b_total_ro = coarse_ro == 3;

    G4String mod_prefix = mod_id;

    auto SetVolumeEDepSD = [sdm](const G4String& volumeName) {
        VolumeEDepSD* volumeSD = new VolumeEDepSD(volumeName + "_SD");
        G4LogicalVolume* logVolume = G4LogicalVolumeStore::GetInstance()->GetVolume(volumeName + "_Log");
        if (logVolume) {logVolume->SetSensitiveDetector(volumeSD);}
        sdm->AddNewDetector(volumeSD);
    };

    auto AttachVolumeEDepSD = [](const G4String& volumeName, VolumeEDepSD* volumeSD) {
        G4LogicalVolume* logVolume = G4LogicalVolumeStore::GetInstance()->GetVolume(volumeName + "_Log");
        if (logVolume) {logVolume->SetSensitiveDetector(volumeSD);}
    };

    auto SetEntryKineticEnergySD = [sdm](const G4String& volumeName) {
        EntryKineticEnergySD* volumeSD = new EntryKineticEnergySD(volumeName + "_SD");
        G4LogicalVolume* logVolume = G4LogicalVolumeStore::GetInstance()->GetVolume(volumeName + "_Log");
        if (logVolume) {logVolume->SetSensitiveDetector(volumeSD);}
        sdm->AddNewDetector(volumeSD);
    };

    VolumeEDepSD* coarseSD = nullptr;
    if (b_total_ro) {
        coarseSD = new VolumeEDepSD(mod_prefix + "_Total_SD");
        sdm->AddNewDetector(coarseSD);
    }

    VolumeEDepSD* cellSDs[NLAYERSMAX][NPERIODSMAX] = {};
    if (b_cell_ro) {
        for (G4int j = 0; j < n_layers; j++) {
            for (G4int iperiod = 0; iperiod < n_periods; iperiod++) {
                G4String cell_name = mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Cell";
                cellSDs[j][iperiod] = new VolumeEDepSD(cell_name + "_SD");
                sdm->AddNewDetector(cellSDs[j][iperiod]);
            }
        }
    }

    if (b_place_support) {
        SetVolumeEDepSD(mod_prefix + "_Front");
        SetVolumeEDepSD(mod_prefix + "_Back");
        SetVolumeEDepSD(mod_prefix + "_Side0");
        SetVolumeEDepSD(mod_prefix + "_Side1");
    }

    SetEntryKineticEnergySD(mod_prefix + "_CatcherFront");
    SetEntryKineticEnergySD(mod_prefix + "_CatcherBack");
    SetEntryKineticEnergySD(mod_prefix + "_CatcherSide0");
    SetEntryKineticEnergySD(mod_prefix + "_CatcherSide1");
    SetEntryKineticEnergySD(mod_prefix + "_CatcherPhi0");
    SetEntryKineticEnergySD(mod_prefix + "_CatcherPhi1");

    for (G4int j = 0; j < n_layers; j++) {
        for (G4int i = 0; i < n_periods * 2 - 1; i++) {
            G4int iperiod = floor(i/2);

            G4String mst_name = mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Master" + std::to_string(i%2);
            if (b_total_ro) {AttachVolumeEDepSD(mst_name, coarseSD);}
            else if (b_cell_ro) {AttachVolumeEDepSD(mst_name, cellSDs[j][iperiod]);}
            else if (!b_scinti_ro_only) {SetVolumeEDepSD(mst_name);}
        }
    }

    for (G4int j = 0; j < n_layers; j++) {
        for (G4int i = 0; i < n_periods * 2; i++) {
            G4int iperiod = floor(i/2);
        
            G4int b_spc = ((i%2) + (j%2)) % 2;
            if (b_spc) {
                G4String spc_name = mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Spacer";
                if (b_total_ro) {AttachVolumeEDepSD(spc_name, coarseSD);}
                else if (b_cell_ro) {AttachVolumeEDepSD(spc_name, cellSDs[j][iperiod]);}
                else if (!b_scinti_ro_only) {SetVolumeEDepSD(spc_name);}
            } else {
                for (G4int k = 0; k < 2; k++) {
                    G4String sci_name = mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Scintillator" + std::to_string(k);
                    G4String fibre_name = mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Fibre" + std::to_string(k);
                    if (b_total_ro) {
                        AttachVolumeEDepSD(sci_name, coarseSD);
                        AttachVolumeEDepSD(fibre_name, coarseSD);
                    } else if (b_cell_ro) {
                        AttachVolumeEDepSD(sci_name, cellSDs[j][iperiod]);
                        AttachVolumeEDepSD(fibre_name, cellSDs[j][iperiod]);
                    } else {
                        SetVolumeEDepSD(sci_name);
                        if (!b_scinti_ro_only) {SetVolumeEDepSD(fibre_name);}
                    }
                }
            }
        }
    }
}

// function to create all ntuple columns associated with the module
// static method, intended for use without object instantiation in RunAction.cc
void DetectorConstruction::fullTileCalModule::CreateNtupleColumns(
    G4AnalysisManager* analysis,
    G4int coarse_ro,
    G4String mod_id,
    G4int n_periods,
    G4int n_layers
){
    G4bool b_scinti_ro_only = coarse_ro == 1;
    G4bool b_cell_ro = coarse_ro == 2;
    G4bool b_total_ro = coarse_ro == 3;

    auto Id = [](G4int id) {
        std::ostringstream stream;
        stream << std::setw(3) << std::setfill('0') << id;
        return G4String(stream.str());
    };

    G4String mod_prefix = mod_id;
    if (mod_id.size() > 1 && mod_id[0] == 'M') {
        mod_prefix = "M" + Id(std::atoi(mod_id.substr(1).c_str()));
    }

    analysis->CreateNtupleDColumn("true_EkinOut_" + mod_prefix + "_Front");
    analysis->CreateNtupleDColumn("true_EkinOut_" + mod_prefix + "_Back");
    analysis->CreateNtupleDColumn("true_EkinOut_" + mod_prefix + "_Side0");
    analysis->CreateNtupleDColumn("true_EkinOut_" + mod_prefix + "_Side1");
    analysis->CreateNtupleDColumn("true_EkinOut_" + mod_prefix + "_Phi0");
    analysis->CreateNtupleDColumn("true_EkinOut_" + mod_prefix + "_Phi1");

    analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_Total");

    analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_Front");
    analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_Back");
    analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_Side0");
    analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_Side1");

    if (b_cell_ro) {
        for (G4int j = 0; j < n_layers; j++) {
            for (G4int iperiod = 0; iperiod < n_periods; iperiod++) {
                analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_L" + Id(j) + "_P" + Id(iperiod) + "_Cell");
            }
        }
    } else if (!b_total_ro) {
        if (!b_scinti_ro_only) {
            for (G4int j = 0; j < n_layers; j++) {
                for (G4int i = 0; i < n_periods * 2 - 1; i++) {
                    G4int iperiod = floor(i/2);

                    analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_L" + Id(j) + "_P" + Id(iperiod) + "_Master" + Id(i%2));
                }
            }
        }

        for (G4int j = 0; j < n_layers; j++) {
            for (G4int i = 0; i < n_periods * 2; i++) {
                G4int iperiod = floor(i/2);

                G4int b_spc = ((i%2) + (j%2)) % 2;
                if (b_spc) {
                    if (!b_scinti_ro_only) {
                        analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_L" + Id(j) + "_P" + Id(iperiod) + "_Spacer");
                    }
                } else {
                    for (G4int k = 0; k < 2; k++) {
                        analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_L" + Id(j) + "_P" + Id(iperiod) + "_Scintillator" + std::to_string(k));
                        if (!b_scinti_ro_only) {
                            analysis->CreateNtupleDColumn("Edep_" + mod_prefix + "_L" + Id(j) + "_P" + Id(iperiod) + "_Fibre" + std::to_string(k));
                        }
                    }
                }
            }
        }
    }
}

// function to fill all ntuple columns associated with the module
// static method, intended for use without object instantiation in EventAction.cc
G4int DetectorConstruction::fullTileCalModule::FillNtupleColumns(
    G4AnalysisManager* analysis,
    G4SDManager* sdm,
    G4int coarse_ro,
    G4HCofThisEvent* hcofEvent,
    G4int col,
    G4String mod_id,
    G4int n_periods,
    G4int n_layers
){
    G4bool b_scinti_ro_only = coarse_ro == 1;
    G4bool b_cell_ro = coarse_ro == 2;
    G4bool b_total_ro = coarse_ro == 3;

    auto GetVolumeEDep = [sdm, hcofEvent](const G4String& volumeName, const G4double missingValue) {
        G4double eDep = 0.;
        G4int collectionId = sdm->GetCollectionID(volumeName + "_SD/VolumeEDep");
        VolumeEDepHitsCollection* hitCollection = nullptr;
        if (collectionId >= 0) {
            hitCollection = dynamic_cast<VolumeEDepHitsCollection*>(hcofEvent->GetHC(collectionId));
        }

        if (hitCollection) {
            for (auto hit: *hitCollection->GetVector()) {
                eDep += hit->GetEDep();
            }
            return eDep;
        }

        return missingValue;
    };

    auto FillVolumeEDep = [analysis, GetVolumeEDep](G4int col, const G4String& volumeName) {
        G4double eDep = GetVolumeEDep(volumeName, -1. * GeV);
        analysis->FillNtupleDColumn(0, col, eDep / GeV);
        return ++col;
    };

    auto GetEntryKineticEnergy = [sdm, hcofEvent](const G4String& volumeName, const G4double missingValue) {
        G4double eKin = 0.;
        G4int collectionId = sdm->GetCollectionID(volumeName + "_SD/EntryKineticEnergy");
        EntryKineticEnergyHitsCollection* hitCollection = nullptr;
        if (collectionId >= 0) {
            hitCollection = dynamic_cast<EntryKineticEnergyHitsCollection*>(hcofEvent->GetHC(collectionId));
        }

        if (hitCollection) {
            for (auto hit: *hitCollection->GetVector()) {
                eKin += hit->GetEKin();
            }
            return eKin;
        }

        return missingValue;
    };

    auto FillModuleEkinOut = [analysis, GetEntryKineticEnergy](G4int col, const G4String& mod_prefix) {
        G4double eKinFront = GetEntryKineticEnergy(mod_prefix + "_CatcherFront", 0.);
        G4double eKinBack = GetEntryKineticEnergy(mod_prefix + "_CatcherBack", 0.);
        G4double eKinSide0 = GetEntryKineticEnergy(mod_prefix + "_CatcherSide0", 0.);
        G4double eKinSide1 = GetEntryKineticEnergy(mod_prefix + "_CatcherSide1", 0.);
        G4double eKinPhi0 = GetEntryKineticEnergy(mod_prefix + "_CatcherPhi0", 0.);
        G4double eKinPhi1 = GetEntryKineticEnergy(mod_prefix + "_CatcherPhi1", 0.);

        analysis->FillNtupleDColumn(0, col++, eKinFront / GeV);
        analysis->FillNtupleDColumn(0, col++, eKinBack / GeV);
        analysis->FillNtupleDColumn(0, col++, eKinSide0 / GeV);
        analysis->FillNtupleDColumn(0, col++, eKinSide1 / GeV);
        analysis->FillNtupleDColumn(0, col++, eKinPhi0 / GeV);
        analysis->FillNtupleDColumn(0, col++, eKinPhi1 / GeV);
        return col;
    };

    auto SumModuleEDep = [GetVolumeEDep, n_periods, n_layers, b_cell_ro, b_scinti_ro_only](const G4String& mod_prefix) {
        G4double eDep = 0.;

        if (b_cell_ro) {
            for (G4int j = 0; j < n_layers; j++) {
                for (G4int iperiod = 0; iperiod < n_periods; iperiod++) {
                    eDep += GetVolumeEDep(mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Cell", 0.);
                }
            }
            return eDep;
        }

        if (!b_scinti_ro_only) {
            for (G4int j = 0; j < n_layers; j++) {
                for (G4int i = 0; i < n_periods * 2 - 1; i++) {
                    G4int iperiod = floor(i/2);
                    eDep += GetVolumeEDep(mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Master" + std::to_string(i%2), 0.);
                }
            }
        }

        for (G4int j = 0; j < n_layers; j++) {
            for (G4int i = 0; i < n_periods * 2; i++) {
                G4int iperiod = floor(i/2);

                G4int b_spc = ((i%2) + (j%2)) % 2;
                if (b_spc) {
                    if (!b_scinti_ro_only) {
                        eDep += GetVolumeEDep(mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Spacer", 0.);
                    }
                } else {
                    for (G4int k = 0; k < 2; k++) {
                        eDep += GetVolumeEDep(mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Scintillator" + std::to_string(k), 0.);
                        if (!b_scinti_ro_only) {
                            eDep += GetVolumeEDep(mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Fibre" + std::to_string(k), 0.);
                        }
                    }
                }
            }
        }

        return eDep;
    };

    G4String mod_prefix = mod_id;

    col = FillModuleEkinOut(col, mod_prefix);

    if (b_total_ro) {
        col = FillVolumeEDep(col, mod_prefix + "_Total");
        col = FillVolumeEDep(col, mod_prefix + "_Front");
        col = FillVolumeEDep(col, mod_prefix + "_Back");
        col = FillVolumeEDep(col, mod_prefix + "_Side0");
        col = FillVolumeEDep(col, mod_prefix + "_Side1");
        return col;
    }

    G4double moduleEDep = SumModuleEDep(mod_prefix);
    analysis->FillNtupleDColumn(0, col++, moduleEDep / GeV);

    col = FillVolumeEDep(col, mod_prefix + "_Front");
    col = FillVolumeEDep(col, mod_prefix + "_Back");
    col = FillVolumeEDep(col, mod_prefix + "_Side0");
    col = FillVolumeEDep(col, mod_prefix + "_Side1");

    if (b_cell_ro) {
        for (G4int j = 0; j < n_layers; j++) {
            for (G4int iperiod = 0; iperiod < n_periods; iperiod++) {
                col = FillVolumeEDep(col, mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Cell");
            }
        }
        return col;
    }

    if (!b_scinti_ro_only) {
        for (G4int j = 0; j < n_layers; j++) {
            for (G4int i = 0; i < n_periods * 2 - 1; i++) {
                G4int iperiod = floor(i/2);

                col = FillVolumeEDep(col, mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Master" + std::to_string(i%2));
            }
        }
    }

    for (G4int j = 0; j < n_layers; j++) {
        for (G4int i = 0; i < n_periods * 2; i++) {
            G4int iperiod = floor(i/2);

            G4int b_spc = ((i%2) + (j%2)) % 2;
            if (b_spc) {
                if (!b_scinti_ro_only) {
                    col = FillVolumeEDep(col, mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Spacer");
                }
            } else {
                for (G4int k = 0; k < 2; k++) {
                    col = FillVolumeEDep(col, mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Scintillator" + std::to_string(k));
                    if (!b_scinti_ro_only) {
                        col = FillVolumeEDep(col, mod_prefix + "_L" + std::to_string(j) + "_P" + std::to_string(iperiod) + "_Fibre" + std::to_string(k));
                    }
                }
            }
        }
    }

    return col;
}

// geomTrapezoid methods //////////////////////////////////////

// set radial position of the lower base
// default arguments set in the header
// if override is false, use the standard function to calculate it from the trapezoid parameters
// if override is true, manually set it through newval (ignored otherwise) - this will negate isConsistent flags
void DetectorConstruction::geomTrapezoid::SetR_b(G4bool override, G4double newval) {
    if(override){
        R_b = newval;
        isConsistent = false;
        isConsistentRectangle = false;
    }else{
        R_b = fR_b(GetR(), GetH());
    }
}

// set radial position of the upper base
// default arguments set in the header
// if override is false, use the standard function to calculate it from the trapezoid parameters
// if override is true, manually set it through newval (ignored otherwise) - this will negate isConsistent flags
void DetectorConstruction::geomTrapezoid::SetR_t(G4bool override, G4double newval) {
    if(override){
        R_t = newval;
        isConsistent = false;
        isConsistentRectangle = false;
    }else{
        R_t = fR_t(GetR(), GetH());
    }
}

// set full length of the lower base
// default arguments set in the header
// if override is false, use the standard function to calculate it from the trapezoid parameters
// if override is true, manually set it through newval (ignored otherwise) - this will negate isConsistent flags
void DetectorConstruction::geomTrapezoid::SetDHor_b(G4bool override, G4double newval) {
    if(override){
        L_b = newval;
        isConsistent = false;
        isConsistentRectangle = false;
    }else{
        L_b = fL_b(GetR(), GetH(), GetTheta());
    }
}

// set full length of the upper base
// default arguments set in the header
// if override is false, use the standard function to calculate it from the trapezoid parameters
// if override is true, manually set it through newval (ignored otherwise) - this will negate isConsistent flags
void DetectorConstruction::geomTrapezoid::SetDHor_t(G4bool override, G4double newval) {
    if(override){
        L_t = newval;
        isConsistent = false;
        isConsistentRectangle = false;
    }else{
        L_t = fL_t(GetR(), GetH(), GetTheta());
    }
}

// reduce the horizontal dimensions to account for the fibre gaps
// only (some) horizontal quantities are affected
void DetectorConstruction::geomTrapezoid::AddHorGaps(G4double gapsize) {
    GapHor = gapsize / cos(GetTheta() / 2);
    isGaps = true;

    L_b = GetDHor_b() - 2*GapHor;
    L_t = GetDHor_t() - 2*GapHor;
    SetDHor_mid(); // recalculate L_mid
}

// remove the gap introduced with AddHorGaps
// only (some) horizontal quantities are affected
void DetectorConstruction::geomTrapezoid::RmHorGaps() {
    if (isGaps) {
        L_b = GetDHor_b() + 2*GapHor;
        L_t = GetDHor_t() + 2*GapHor;
        SetDHor_mid(); // recalculate L_mid

        GapHor = 0;
        isGaps = false;
    } else {return;}
}
