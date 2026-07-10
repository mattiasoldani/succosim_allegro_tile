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

>>>>>>> e497abd4908c40a501a5339ac6196963b095c447

    //// preliminary tests 00 ////

    S2_geom->AddHorGaps(sidegap);

    sign = 1;
    pos = G4ThreeVector(-sign*0.5*cm, 0*thickness, 0);
    G4LogicalVolume* tileTest000Log = fLogTile("tile000", bc400, cyan, tileTestGeom, thickness, sign, holeradius, holex, holey);
    new G4PVPlacement(tileRotation, pos, tileTest000Log, "tile000", worldLog_rot, false, 0);

    G4LogicalVolume* tileTest000FibreLog = fLogPlaceFibreCirc("fibre000", bc400, green, tileTestGeom, worldLog_rot, fibreradius, 5., 50., pos, tileRotation, sign);

    sign = -1;
    pos = G4ThreeVector(-sign*0.5*cm, 10*thickness, 0);
    G4LogicalVolume* tileTest001Log = fLogTile("tile001", bc400, cyan, tileTestGeom, thickness, sign, holeradius, holex, holey);
    new G4PVPlacement(tileRotation, pos, tileTest001Log, "tile001", worldLog_rot, false, 0);

    G4LogicalVolume* tileTest001FibreLog = fLogPlaceFibreCirc("fibre001", bc400, green, tileTestGeom, worldLog_rot, fibreradius, 5., 50., pos, tileRotation, sign);

    sign = 0;
    pos = G4ThreeVector(-3*cm, -10*thickness, 0);
    G4LogicalVolume* tileTest002Log = fLogTile("tile002", bc400, cyan, tileTestGeom, thickness, sign, holeradius, holex, holey);
    new G4PVPlacement(tileRotation, pos, tileTest002Log, "tile002", worldLog_rot, false, 0);

    G4LogicalVolume* tileTest002FibreLog = fLogPlaceFibreCirc("fibre002", bc400, green, tileTestGeom, worldLog_rot, fibreradius, 5., 50., pos, tileRotation, sign);

    sign = 1;
    pos = G4ThreeVector(-sign*0.5*cm, 0*thickness - 200*mm, 0);
    G4LogicalVolume* tileTest003Log = fLogTile("tile003", bc400, cyan, tileTestGeomRect, thickness, sign, holeradius, holex, holey);
    new G4PVPlacement(tileRotation, pos, tileTest003Log, "tile003", worldLog_rot, false, 0);

    G4LogicalVolume* tileTest003FibreLog = fLogPlaceFibreCirc("fibre003", bc400, green, tileTestGeomRect, worldLog_rot, fibreradius, 5., 50., pos, tileRotation, sign);

    sign = -1;
    pos = G4ThreeVector(-sign*0.5*cm, 10*thickness - 200*mm, 0);
    G4LogicalVolume* tileTest004Log = fLogTile("tile004", bc400, cyan, tileTestGeomRect, thickness, sign, holeradius, holex, holey);
    new G4PVPlacement(tileRotation, pos, tileTest004Log, "tile004", worldLog_rot, false, 0);

    G4LogicalVolume* tileTest004FibreLog = fLogPlaceFibreCirc("fibre004", bc400, green, tileTestGeomRect, worldLog_rot, fibreradius, 5., 50., pos, tileRotation, sign);

    sign = 0;
    pos = G4ThreeVector(-3*cm, -10*thickness - 200*mm, 0);
    G4LogicalVolume* tileTest005Log = fLogTile("tile005", bc400, cyan, tileTestGeomRect, thickness, sign, holeradius, holex, holey);
    new G4PVPlacement(tileRotation, pos, tileTest005Log, "tile005", worldLog_rot, false, 0);

    G4LogicalVolume* tileTest005FibreLog = fLogPlaceFibreCirc("fibre005", bc400, green, tileTestGeomRect, worldLog_rot, fibreradius, 5., 50., pos, tileRotation, sign);

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

    pTileRot->setAxis(tilePos);
    new G4PVPlacement(pTileRot, tilePos, pEnvelopeInternal, name_phys, pEnvelope, false, 0);

    return logvol;
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

