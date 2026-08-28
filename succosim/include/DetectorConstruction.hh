#ifndef DETECTOR_CONSTRUCTION_HH
#define DETECTOR_CONSTRUCTION_HH

#include <G4SystemOfUnits.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4NistManager.hh>
#include <G4VisAttributes.hh>
#include <G4SDManager.hh>
#include <G4Trd.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4IntersectionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4Vector3D.hh>
#include <G4Transform3D.hh>
#include <G4HCofThisEvent.hh>
#include <G4LogicalVolume.hh>
#include <CLHEP/Units/PhysicalConstants.h>

#include "Analysis.hh"
#include "CustomMessenger.hh"
#include "CustomSD.hh"

// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// define hard-coded parameters
// e.g. #define NLAYERS 10

// number of tiles in stacks
#define TILECERN_N 10
#define TILEFZU_N 32

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

using namespace std;

// ubiquitous definition of pi
inline const G4double pi = CLHEP::pi;

// DetectorConstruction, i.e. the class with all the setup info (physical objects, detectors, magnetic fields)
class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // define custom public methods/members here
    // e.g. getters for private stuff

    //// tile-specific framework ////
    #include "DetectorConstruction_tile.hh"

    // getters related to private booleans defined below
    G4bool IsConfigCalib() const { return b_config_calib; }
    G4bool IsConfigBB() const { return b_config_BB; }
    G4bool IsConfigCERNS2() const { return b_config_CERN_S2; }
    G4bool IsConfigCERNS6() const { return b_config_CERN_S6; }
    G4bool IsConfigCERNS6Upstr1() const { return b_config_CERN_S6_upstr1; }
    G4bool IsConfigCERNS6Upstr2() const { return b_config_CERN_S6_upstr2; }
    G4bool IsConfigCERNS6Upstr4() const { return b_config_CERN_S6_upstr4; }
    G4bool IsCERNAny() const { return b_CERN_any; }
    G4bool IsFZU() const { return b_FZU; }
    G4bool IsCERNTrig() const { return b_CERN_trig; }
    G4bool IsCERNS2() const { return b_CERN_S2; }
    G4bool IsCERNS6() const { return b_CERN_S6; }
    G4bool IsPbGl() const { return b_PbGl; }
    G4bool IsScintiSmall() const { return b_scinti_small; }
    G4bool IsScintiBig() const { return b_scinti_big; }
    G4bool IsScinti() const { return IsScintiSmall() || IsScintiBig(); }
    G4bool IsCher() const { return b_cher; }
    G4bool IsHodo() const { return b_hodo; }
    
    G4bool IsScintiSmallDet() const { return b_scinti_small && b_scinti_small_det; }
    G4bool IsCherDet() const { return b_cher && b_cher_det; }
    G4bool IsHodoDet() const { return b_hodo && b_hodo_det; }

    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

private:
    // custom messenger
    CustomMessenger* custom = CustomMessenger::Instance();

    // NIST database
    G4NistManager* nist = G4NistManager::Instance();

    // default colors
    G4VisAttributes* invisible = new G4VisAttributes(false);
    G4VisAttributes* white = new G4VisAttributes(G4Colour::White());
    G4VisAttributes* cyan = new G4VisAttributes(G4Colour::Cyan());
    G4VisAttributes* blue = new G4VisAttributes(G4Colour::Blue());
    G4VisAttributes* red = new G4VisAttributes(G4Colour::Red());
    G4VisAttributes* green = new G4VisAttributes(G4Colour::Green());
    G4VisAttributes* magenta = new G4VisAttributes(G4Colour::Magenta());
	G4VisAttributes* grey = new G4VisAttributes(G4Colour::Grey());
    G4VisAttributes* brown = new G4VisAttributes(true, G4Colour::Brown());
    G4VisAttributes* yellow = new G4VisAttributes(true, G4Colour::Yellow());
    G4VisAttributes* black = new G4VisAttributes(true, G4Colour::Black());
	
   // default manual material: single elements
    G4Element* elC = new G4Element("Carbon", "C", 6., 12.01*g/mole);
    G4Element* elO  = new G4Element("Oxygen","O" , 8., 16.00*g/mole);
    G4Element* elH = new G4Element("Hydrogen", "H", 1., 1.0079*g/mole);

    // default off-the-shelf materials (from NIST)
    G4Material* air = nist->FindOrBuildMaterial("G4_AIR");
    G4Material* SS = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
    G4Material* plastic = nist->FindOrBuildMaterial("G4_POLYSTYRENE");
    G4Material* pbGl = nist->FindOrBuildMaterial("G4_GLASS_LEAD");
    G4Material* vacuum = nist->FindOrBuildMaterial("G4_Galactic");
    G4Material* aluminium = nist->FindOrBuildMaterial("G4_Al");
    G4Material* mylar = nist->FindOrBuildMaterial("G4_MYLAR");
    G4Material* lead = nist->FindOrBuildMaterial("G4_Pb"); 
    G4Material* silicon = nist->FindOrBuildMaterial("G4_Si"); 
    G4Material* iron = nist->FindOrBuildMaterial("G4_Fe");
    G4Material* tungsten = nist->FindOrBuildMaterial("G4_W");
    G4Material* co2 = new G4Material("CO2", 1.977*273.*mg/cm3/293., 2); // default manual material: CO2 - elements added in DetectorConstruction.cc
    G4Material* bc400 = new G4Material("BC400", 1.032*g/cm3, 2); // default manual material: BC400 (plastic scintillator) - elements added in DetectorConstruction.cc

	// generic translation and roto-translation, for element-by-element use
	G4ThreeVector pos_temp;
	G4Transform3D pos_rot_temp;

    // world size (full sides) and material - to be set here
    G4double worldSizeX = 3 * m;
    G4double worldSizeY = 3 * m;
    G4double worldSizeZ = 30 * m;
    G4Material* mat_world = vacuum;

    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // define custom private methods/members here
    // e.g. void ConstructCalo(G4LogicalVolume* worldLog);

    // configuration boleans and other settings
    G4bool b_CERN_any = true;
    G4bool b_FZU = true;
    G4bool b_CERN_trig = true;
    G4bool b_CERN_S2 = true;
    G4bool b_CERN_S6 = false;
    G4bool b_PbGl = true;
    G4bool b_scinti_big = true;
    G4bool b_hodo = true;

    G4int id_config = 0; // redefined in DetectorConstruction.cc (from custom macro parameter)
    G4bool b_config_calib = true; // redefined in DetectorConstruction.cc (from custom macro parameter)
    G4bool b_config_BB = false; // redefined in DetectorConstruction.cc (from custom macro parameter)
    G4bool b_config_CERN_S2 = false; // redefined in DetectorConstruction.cc (from custom macro parameter)
    G4bool b_config_CERN_S6 = false; // redefined in DetectorConstruction.cc (from custom macro parameter)
    G4bool b_config_CERN_S6_upstr1 = false; // redefined in DetectorConstruction.cc (from custom macro parameter)
    G4bool b_config_CERN_S6_upstr2 = false; // redefined in DetectorConstruction.cc (from custom macro parameter)
    G4bool b_config_CERN_S6_upstr4 = false; // redefined in DetectorConstruction.cc (from custom macro parameter)
    G4bool b_scinti_small = false; // redefined in DetectorConstruction.cc (from custom macro parameter)
    G4bool b_cher = false; // redefined in DetectorConstruction.cc (from custom macro parameter)

    // sensitive-detector booleans (subordinate to the respective detector-placement booleans)
    G4bool b_scinti_small_det = false; // redefined in DetectorConstruction.cc (from custom macro parameter)
    G4bool b_cher_det = false; // redefined in DetectorConstruction.cc (from custom macro parameter)
    G4bool b_hodo_det = true; // redefined in DetectorConstruction.cc (from custom macro parameter)

    // misc general stuff
    G4double gen_gap = 1*mm; // gap between adjacent tiles (longitudinal, transverse)
    G4double gen_fibreradius = 0.5*mm; // radius of the WLS fibres
    G4double gen_thk = 3*mm; // thickness
    G4double gen_holeradius = 0*mm; // radius of the pipe/rod hole
    G4double gen_sidegap = 0*mm; // side reduction to account for the fibres
    G4double thk_cher0_cap = 1.4 * mm; // thickness of front/rear caps of 1st Cherenkov
    G4double thk_cher1_cap = 0.25 * mm; // thickness of front/rear caps of 2nd Cherenkov

    // general positioning (source will be in world centre)
    G4double z_scintiSmall0_front = (0)*cm; // z of S0 front
    G4double z_scintiSmall1_front = (817)*cm; // z of S1 front
    G4double z_scintiBig0_front = (817+43.5+9+51.5+29.5-46-8.3)*cm; // z of S2 front
    G4double z_scintiBig1_front = (817+43.5+9+51.5+29.5+91.5+70.1+49.5+57.4)*cm; // z of S3 front
    G4double z_cher0_front = (30)*cm; // z of front of 1st Cherenkov - not measured
    G4double z_cher1_front = (817-34.5-259)*cm; // z of front of 2nd Cherenkov
    G4double z_pbGl_front = (817+43.5+9+51.5+29.5+91.5+70.1)*cm; // z of Pb glass front
    G4double z_tileCern_front = (817+43.5+9+51.5+29.5+36)*cm; // z of CERN stack front
    G4double z_FZU_front = (817+43.5+9+51.5+29.5+7)*cm; // z of FZU stack front
    G4double z_FZU_rear = z_FZU_front + TILEFZU_N*(gen_thk+gen_gap);  // z of FZU stack rear
    G4double z_hodo_centre = (817+43.5+4.5)*cm; // z of plastic hodoscope centre
    G4double z_additionalPassive_rear = (817+43.5+9+51.5+29.5-1.5)*cm; // z of rear of extra passive layers in specific runs
    G4double tileCern_beamref_shift = -1*cm; // relative vertical offset of the CERN stack wrt the beam reference markings
    G4double FZU_beamref_shift = -0.5*cm; // relative vertical offset of the FZU stack wrt the beam reference markings
    G4double passive_S6_shift = 47.17*mm; // relative vertical shift between passive layers and S6 (half) tiles in CERN stack
    G4double passive_trig_shift = 59*mm; // relative vertical shift between passive layers and S2 trigger (half) tiles in CERN stack

	// tile shapes - CERN S2
	G4double FZU_ang_x = -90*deg; // angle wrt original x axis
	G4double FZU_ang_y = 0*deg; // angle wrt original y axis
	G4double FZU_ang_z = 0*deg; // angle wrt original z axis
    G4double FZU_w = 70.5*mm; // short-side width (half-module)
    G4double FZU_W = 75*mm; // long-side width (half-module)
    G4double FZU_h = 97*mm; // height
    G4double FZU_thk = gen_thk; // thickness
    G4double FZU_fibreradius = gen_fibreradius; // radius of the WLS fibres
    G4double FZU_sidegap = gen_sidegap; // side reduction to account for the fibres
    G4double FZU_holeradius = gen_holeradius; // radius of the pipe/rod hole
    G4double FZU_holey = 0; // pipe/rod hole centre y (relative to full-module tile centre)
    G4double FZU_holex = 0; // pipe/rod hole centre x (relative to full-module tile centre)
    G4double FZU_zgap = gen_gap; // longitudinal gap between successive tiles, FZU only

	// tile shapes - CERN S2
	G4double S2_ang_x = 90*deg; // angle wrt original x axis
	G4double S2_ang_y = 180*deg; // angle wrt original y axis 
	G4double S2_ang_z = 0*deg; // angle wrt original z axis
    G4double S2_w = 70.5*mm; // short-side width (half-module)
    G4double S2_W = 75*mm; // long-side width (half-module)
    G4double S2_h = 97*mm; // height
    G4double S2_thk = gen_thk; // thickness
    G4double S2_fibreradius = gen_fibreradius; // radius of the WLS fibres
    G4double S2_sidegap = gen_sidegap; // side reduction to account for the fibres
    G4double S2_holeradius = gen_holeradius; // radius of the pipe/rod hole
    G4double S2_holey = S2_h/2 - 6*mm; // pipe/rod hole centre y (relative to full-module tile centre)
    G4double S2_holex = 6*mm; // pipe/rod hole centre x (relative to full-module tile centre)
    G4double S2_xgap = gen_gap; // transverse (horizontal) gap between adjacent tiles, S2 only 
	G4double S2_fzrel(G4int i), S2_fxrel(G4int i), S2_fyrel(G4int i); // functions for iterative tile placing, defined in DetectorConstruction.cc

	// tile shapes - CERN S6
	G4double S6_ang_x = 90*deg; // angle wrt original x axis
	G4double S6_ang_y = -90*deg; // angle wrt original y axis
	G4double S6_ang_z = 0*deg; // angle wrt original z axis
    G4double S6_w = 91*mm; // short-side width (half-module)
    G4double S6_W = 100*mm; // long-side width (half-module)
    G4double S6_h = 187*mm; // height
    G4double S6_thk = gen_thk; // thickness
    G4double S6_fibreradius = gen_fibreradius; // radius of the WLS fibres
    G4double S6_sidegap = gen_sidegap; // side reduction to account for the fibres
    G4double S6_holeradius = gen_holeradius; // radius of the pipe/rod hole
    G4double S6_holey = S6_h/2 - 6*mm; // pipe/rod hole centre y (relative to full-module tile centre)
    G4double S6_holex = 6*mm; // pipe/rod hole centre x (relative to full-module tile centre)
	G4double S6_fzrel(G4int i), S6_fxrel(G4int i), S6_fyrel(G4int i); // functions for iterative tile placing, defined in DetectorConstruction.cc

    // steel tile shapes
	G4double passive_ang_x = 90*deg; // angle wrt original x axis
	G4double passive_ang_y = 0*deg; // angle wrt original y axis
	G4double passive_ang_z = 0*deg; // angle wrt original z axis
    G4double passive_thk_u = 9.9*mm; // thickness - upstream half
    G4double passive_w_u = 17.38*cm; // width - upstream half
    G4double passive_thk_d = 14.5*mm; // thickness - downstream half
    G4double passive_w_d = 18.08*cm; // width - downstream half
    G4double passive_h = 19.2*cm; // height
    G4double passive_thk_gap = gen_gap; // longitudinal gap between steel layers and plastic tiles
    G4double passive_thk_gross = passive_thk_u + passive_thk_d + 2*passive_thk_gap; // total thickness of whole passive layer and gaps
    
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
    // test setup (implemented in src/TestMode.cc)
    G4VPhysicalVolume* SetupTest(G4NistManager* nist);
    void SDTest(G4SDManager* sdm);
};

#endif
