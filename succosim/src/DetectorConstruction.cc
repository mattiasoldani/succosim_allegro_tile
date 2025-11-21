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
    G4double angle = 2*pi/128;
    G4double thickness = 3*mm;
    G4double sidegap = 2*mm;
    G4double holeradius = 3*mm;
    G4double holey = height/2 - 6*mm;
    G4double holex = 6*mm;
    G4double fibreradius = 0.5*mm;

    G4RotationMatrix* tileRotation = new G4RotationMatrix();
    tileRotation->rotateY(20 * deg);

	geomTrapezoid* tileTestGeom = new geomTrapezoid(radius, height, angle);
    G4double offset;
    G4ThreeVector pos;
    G4int sign;

    //// preliminary tests 00 ////

    // // full tile
    // G4LogicalVolume* tileTest0Log = fLogTile(tileTestGeom, "tile0", thickness, bc400, cyan, 0, holeradius, 0, holey);
    // new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), tileTest0Log, "tile0", worldLog_rot, false, 0);

    // tileTestGeom->AddHorGaps(sidegap);
    // G4LogicalVolume* tileTest1Log = fLogTile(tileTestGeom, "tile1", thickness, bc400, cyan, 0, holeradius, holex, holey);
    // new G4PVPlacement(nullptr, G4ThreeVector(0, -2*thickness, 0), tileTest1Log, "tile1", worldLog_rot, false, 0);
    // tileTestGeom->RmHorGaps();

    // // half tile, positive horizontal side
    // G4LogicalVolume* tileTest2Log = fLogTile(tileTestGeom, "tile2", thickness, bc400, blue, 1, holeradius, 0, holey);
    // new G4PVPlacement(nullptr, G4ThreeVector(0, -4*thickness, 0), tileTest2Log, "tile2", worldLog_rot, false, 0);

    // tileTestGeom->AddHorGaps(sidegap);
    // G4LogicalVolume* tileTest3Log = fLogTile(tileTestGeom, "tile3", thickness, bc400, blue, 1, holeradius, holex, holey);
    // new G4PVPlacement(nullptr, G4ThreeVector(0, -6*thickness, 0), tileTest3Log, "tile3", worldLog_rot, false, 0);
    // tileTestGeom->RmHorGaps();

    // // half tile, negative horizontal side
    // G4LogicalVolume* tileTest4Log = fLogTile(tileTestGeom, "tile4", thickness, bc400, red, -1, holeradius, 0, holey);
    // new G4PVPlacement(nullptr, G4ThreeVector(0, -8*thickness, 0), tileTest4Log, "tile4", worldLog_rot, false, 0);

    // tileTestGeom->AddHorGaps(sidegap);
    // G4LogicalVolume* tileTest5Log = fLogTile(tileTestGeom, "tile5", thickness, bc400, red, -1, holeradius, holex, holey);
    // new G4PVPlacement(nullptr, G4ThreeVector(0, -10*thickness, 0), tileTest5Log, "tile5", worldLog_rot, false, 0);
    // tileTestGeom->RmHorGaps();

    // // half tile, positive horizontal side, centred (notice the -offset)
    // offset = tileTestGeom->GetFullToHalfCentreOffset();
    // G4LogicalVolume* tileTest6Log = fLogTile(tileTestGeom, "tile6", thickness, bc400, green, 1, holeradius, 0, holey);
    // new G4PVPlacement(nullptr, G4ThreeVector(-offset, -12*thickness, 0), tileTest6Log, "tile6", worldLog_rot, false, 0);

    // tileTestGeom->AddHorGaps(sidegap);
    // offset = tileTestGeom->GetFullToHalfCentreOffset();
    // G4LogicalVolume* tileTest7Log = fLogTile(tileTestGeom, "tile7", thickness, bc400, green, 1, holeradius, holex, holey);
    // new G4PVPlacement(nullptr, G4ThreeVector(-offset, -14*thickness, 0), tileTest7Log, "tile7", worldLog_rot, false, 0);
    // tileTestGeom->RmHorGaps();

    // // half tile, negative horizontal side, centred (notice the +offset)
    // offset = tileTestGeom->GetFullToHalfCentreOffset();
    // G4LogicalVolume* tileTest8Log = fLogTile(tileTestGeom, "tile8", thickness, bc400, magenta, -1, holeradius, 0, holey);
    // new G4PVPlacement(nullptr, G4ThreeVector(offset, -16*thickness, 0), tileTest8Log, "tile8", worldLog_rot, false, 0);

    // tileTestGeom->AddHorGaps(sidegap);
    // offset = tileTestGeom->GetFullToHalfCentreOffset();
    // G4LogicalVolume* tileTest9Log = fLogTile(tileTestGeom, "tile9", thickness, bc400, magenta, -1, holeradius, holex, holey);
    // new G4PVPlacement(nullptr, G4ThreeVector(offset, -18*thickness, 0), tileTest9Log, "tile9", worldLog_rot, false, 0);
    // tileTestGeom->RmHorGaps();

    //// preliminary tests 01 ////

    tileTestGeom->AddHorGaps(sidegap);

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

        G4VSolid* shape_hole = new G4Tubs("hole_Shape", 0., holeR, dThk / 2, 0., 2*pi);

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
    G4VSolid* envelopeInternalBox = new G4Box("temp_Shape", length_total/2, sectionR/2, length_total/2);
    G4LogicalVolume* pEnvelopeInternal = new G4LogicalVolume(envelopeInternalBox, air, "temp_Log");
    G4VisAttributes* visAttrEnvelopeInternal = new G4VisAttributes();
    visAttrEnvelopeInternal->SetVisibility(false);
    pEnvelopeInternal->SetVisAttributes(visAttrEnvelopeInternal);

    G4String name_phys = name + "_Fibre";

    G4double tilt = pTileGeom->GetTheta()/2;
    G4double fibre_centre_shift = 0.5*( length_total - 2*extraRIn - length_contact );
    G4double pos_long = tilePos.getY();
    G4double pos_tran_ver = tilePos.getZ() + fibre_centre_shift*cos(tilt);
    G4double pos_tran_hor;
    G4ThreeVector pos_final = G4ThreeVector(0, pos_long, pos_tran_ver); // x is set below (once sign is defined)
    G4RotationMatrix* rot_internal = new G4RotationMatrix();

    G4int sign, isignlim = signHalf ? 0 : 1;

    for(int isign=0; isign<=isignlim; isign++){
    // if half module, fibre only on one side
    // if full module, fibre is in principle on both sides (set signHalf properly to only have it on one side anyway)
        sign = signHalf ? ((signHalf > 0) ? 1 : -1) : (isign ? -1 : 1);
        pos_tran_hor = tilePos.getX() + sign*(pTileGeom->GetDHor_mid()/2 + sectionR/cos(tilt)); // move horizontally centre of the total fibre, then...
        pos_tran_hor += sign*fibre_centre_shift*sin(tilt); // ... add shift to the centre of the fibre part in contact
        pos_final.setX(pos_tran_hor);
        rot_internal->rotateY(-sign*tilt);
        new G4PVPlacement(rot_internal, pos_final, logvol, "temp_Phys", pEnvelopeInternal, false, 0);
    }

    pTileRot->setAxis(tilePos);
    new G4PVPlacement(pTileRot, {}, pEnvelopeInternal, name_phys, pEnvelope, false, 0);

    return logvol;
}

// geomTrapezoid methods //////////////////////////////////////

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

