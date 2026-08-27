#ifndef DETECTOR_CONSTRUCTION_HH
#define DETECTOR_CONSTRUCTION_HH

#include <G4SystemOfUnits.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4NistManager.hh>
#include <G4SDManager.hh>
#include <CLHEP/Units/PhysicalConstants.h>

#include <G4Trd.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4IntersectionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4Vector3D.hh>
#include <G4Transform3D.hh>

#include "CustomMessenger.hh"

// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// define hard-coded parameters
// e.g. #define NLAYERS 10

// maximum numbers used for fixed-size array allocation
#define NPERIODSMAX 50
#define NLAYERSMAX 50
#define NSTACKEDMODSMAX 32

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

using namespace std;

inline const G4double pi = CLHEP::pi;

// class for logical volumes
class G4LogicalVolume;

// DetectorConstruction, i.e. the class with all the setup info (physical objects, detectors, magnetic fields)
class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // define custom public methods/members here
    // e.g. getters for private stuff

    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

private:
    // custom messenger
    CustomMessenger* custom = CustomMessenger::Instance();

    // world size to be set here - full sides
    G4double worldSizeX = 10 * m;
    G4double worldSizeY = 10 * m;
    G4double worldSizeZ = 10 * m;

    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // define custom methods here
    // e.g. void ConstructCalo(G4LogicalVolume* worldLog);

    //// specific for this application ////

    // single-element thicknesses which define the period thickness
    G4double mst_thk = 5*mm; // master thickness
    G4double spc_thk = 4*mm; // spacer thickness
    G4double sci_thk = 3*mm; // tile thickness
    G4double period_thk = 2*(mst_thk + spc_thk); // thickness of the full period

    // support/catcher specific features
    G4double front_thk = 20*mm; // thickness (along radial direction) of the front plate
    G4double back_thk = 41*mm; // thickness (along radial direction) of the back structure
    G4double side_thk = 20*mm; // thickness (along longitudinal direction) of the side plates
    G4double catcher_thk = 1*mm; // thickness of all catcher plates around the envelope

    // global module features
    G4double mod_rmin = 2.8*m; // module minimum radius - net (sensitive volume only)
    G4double mod_rmin_env = mod_rmin - front_thk; // module minimum radius - gross (envelope volume)
    G4double mod_dphi = 2*pi/128; // module full azimuthal opening - readout segmentation will be halved
    G4double mod_radial() { // module radial extension - net (sensitive volume only)
        G4double mod_radial_temp = 0;
        const G4int n_layers = custom->NLayers();
        for (G4int j=0; j<n_layers; j++) {
            mod_radial_temp += (j%2) ? spc_hgt(j) : (sci_hgt(j) + 2*sci_r_gap);
            if (n_layers%2) {mod_radial_temp += spc_r_overlap;}
        }
        return mod_radial_temp;
    }
    G4double mod_radial_env() { return mod_radial() + front_thk + back_thk; } // module radial extension - gross (envelope volume)
    G4double mod_thk() { return custom->NPeriods() * period_thk - mst_thk; } // module thickness along longitudinal direction (orthogonal to tiles) - net (sensitive volume only)
    G4double mod_thk_env() { return mod_thk() + 2*side_thk; } // module thickness along longitudinal direction (orthogonal to tiles) - gross
    G4double mod_centre_rel() { return front_thk + mod_radial()/2 - mod_radial_env()/2; } // radial centre of the module net part relative to the gross size

    // master (partial) cross-section shapes etcetera
    G4double mst_r_rel(G4int j){ return mst_r(j) - mod_rmin - mod_radial()/2 + mod_centre_rel(); } // per-layer radius (relative to the gross radial position)

    G4double mst_hgt(G4int j){ // per-layer heights
        if (j<4) {return 55*mm;}
        else if (j<10) {return 105*mm;}
        else {return 205*mm;}
    }

    G4double mst_r(G4int j){ // per-layer radius (absolute)
        G4double mst_rmin_temp = mod_rmin;
        if (j>0) {
            for (G4int k=0; k<j; k++) {mst_rmin_temp += mst_hgt(k);}
        }
        return mst_rmin_temp + mst_hgt(j)/2;
    }

    G4double mst_transv(G4int i){ return -mod_thk() / 2 + spc_thk * (1 + i) + mst_thk * (0.5 + i); } // functions for iterative element placing, master

    // spacer/scintillating tile cross-section shapes
    G4double spc_r_overlap = 2*mm; // overlapping portion between spacers in two successive columns along radius
    G4double sci_r_gap = 1*mm; // 1-side air gap between spacer and scintillator tile (removed from scintillator) along radius
    G4double spc_hgt(G4int j){ return mst_hgt(j); } // per-layer heights
    G4double spc_sidegap(G4int j){ return 2.6*mm*(static_cast<G4int>(floor(j/8))+1); } // per-layer width of the side gap
    G4double spc_r(G4int j){ return mst_r(j); } // per-layer radius (absolute)
    G4double spc_r_rel(G4int j){ return mst_r_rel(j); } // per-layer radius (relative to the gross radial position)
    G4double sci_hgt(G4int j){ return spc_hgt(j) - 2*(spc_r_overlap + sci_r_gap); } // per-layer heights
    G4double sci_sidegap(G4int j){ return spc_sidegap(j); } // per-layer width of the side gap
    G4double sci_r(G4int j){ return spc_r(j); } // per-layer radius (absolute)
    G4double sci_r_rel(G4int j){ return spc_r_rel(j); } // per-layer radius (relative to the gross radial position)

    G4double til_transv(G4int i){ return mst_transv(i) - (spc_thk + mst_thk) / 2; } // functions for iterative element placing, tile

    // other miscellaneous geometric parameters
    G4double inner_gap = 0.4*mm; // gap between adjacent scintillating tiles in the same module slot (full) - side gap will be increased accordingly, to keep sides aligned with spacers
    G4double fibre_r = 0.65*mm; // WLS fibre radius 
    G4double hole_r = 0; // radius of the pipe/rod holes in the tiles
    G4double hole_x = 0; // x position of the pipe/rod holes in the tiles
    G4double hole_y = 0; // y position of the pipe/rod holes in the tiles
    G4double l_fibre_extra = 0.5*cm; // out-of-tile extra length of the fibre towards the drawer
    G4double zshift = catcher_thk; // longitudinal displacement of the module wrt the beam source

    class geomTrapezoid;

    // function to create and place a whole module, defined in DetectorConstruction.cc
    void CreateModule(
        G4String mod_id,
        G4LogicalVolume*& frontLog, 
        G4LogicalVolume*& backLog, 
        G4LogicalVolume** sideLogs, 
        G4LogicalVolume** mstLogs, 
        G4LogicalVolume** spcLogs,
        G4LogicalVolume** sciLogs,
        G4LogicalVolume** fibreLogs, 

        geomTrapezoid*& modEnvGeom,

        G4LogicalVolume*& modEnvLog, 
        G4LogicalVolume*& catcherFrontLog, 
        G4LogicalVolume*& catcherBackLog, 
        G4LogicalVolume** catcherSideLogs,
        G4LogicalVolume** catcherPhiLogs,

        G4Material* mat_passive, G4VisAttributes* col_passive,
        G4Material* mat_support, G4VisAttributes* col_support,
        G4Material* mat_scintillator, G4VisAttributes* col_scintillator,
        G4Material* mat_fibre, G4VisAttributes* col_fibre,
        G4Material* mat_envelope, G4VisAttributes* col_envelope,
        G4Material* mat_catcher,
        G4VisAttributes* col_catcher_front_back,
        G4VisAttributes* col_catcher_side,
        G4VisAttributes* col_catcher_phi
    );

    // tile-specific framework
    #include "DetectorConstruction_tile.hh"
		
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
    // test setup (implemented in src/TestMode.cc)
    G4VPhysicalVolume* SetupTest(G4NistManager* nist);
    void SDTest(G4SDManager* sdm);
};

#endif
