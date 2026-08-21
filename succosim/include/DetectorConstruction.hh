#ifndef DETECTOR_CONSTRUCTION_HH
#define DETECTOR_CONSTRUCTION_HH

#include <G4SystemOfUnits.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4NistManager.hh>
#include <G4SDManager.hh>

#include <G4Trd.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4IntersectionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4Vector3D.hh>
#include <G4Transform3D.hh>

// numbers of
// - periods (along barrel z)
// - layers (along barrel radius)
// - modules (stacked one on top of the other à la beamtest area)
#define NPERIODS 39
#define NLAYERS 13
#define NSTACKEDMODS 3

// readout granularity of each module:
// - 0: single-element readout
// - 1: one readout channel per cell for the inner structure, support read out separately
// - 2: just the total energy in the module
#define COARSERO 1

// if 1, only inner part (scintillating tiles, spacers, masters, fibres) is placed (no support)
#define BPLACEONLYINNER 0

// if 1 (0), inner structure (masters, spacers, scintillators, fibres) is (not) shown in graphical mode - note that volumes are placed anyway
#define BSHOWINNER 1

// if 1 (0), support is (not) shown in graphical mode - note that volumes are placed anyway
#define BSHOWSUPPORT 1

using namespace std;

// class for logical volumes
class G4LogicalVolume;

// DetectorConstruction, i.e. the class with all the setup info (physical objects, detectors, magnetic fields)
class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

private:
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // define custom methods here
    // e.g. void ConstructCalo(G4LogicalVolume* worldLog);

    // simply pi
    static G4double pi;

    // world size - full sides
    G4double worldSizeX = 10 * m;
    G4double worldSizeY = 10 * m;
    G4double worldSizeZ = 10 * m;

    //// specific for this application ////

    // module general parameters
    G4double mst_thk = 5*mm; // master thickness
    G4double spc_thk = 4*mm; // spacer thickness
    G4double sci_thk = 3*mm; // tile thickness
    G4double period_thk = 2*(mst_thk + spc_thk); // thickness of the full period

    G4double front_thk = 20*mm; // thickness (along radial direction) of the front plate
    G4double back_thk = 191*mm; // thickness (along radial direction) of the back structure
    G4double side_thk = 20*mm; // thickness (along longitudinal direction) of the side plates

    G4double mod_rmin = 2.8*m; // module minimum radius - net (sensitive volume only)
    G4double mod_radial_env = mod_radial() + front_thk + back_thk; // module radial extension - gross (envelope volume)
    G4double mod_thk = NPERIODS * period_thk - mst_thk; // module thickness along longitudinal direction (orthogonal to tiles) - net (sensitive volume only)
    G4double mod_thk_env = mod_thk + 2*side_thk; // module thickness along longitudinal direction (orthogonal to tiles) - gross (envelope volume)
    G4double mod_centre_rel = front_thk + mod_radial()/2 - mod_radial_env/2; // radial centre of the module net part relative to the gross size
    G4double mod_dphi = 2*pi/128; // module full azimuthal opening - readout segmentation will be halved
    G4double mod_radial(); // module radial extension - net (sensitive volume only), defined in DetectorConstruction.cc

    G4double mst_transv(G4int i), til_transv(G4int i); // functions for iterative element placing, defined in DetectorConstruction.cc

    // master (partial) cross-section shapes
    G4double mst_hgt(G4int j); // per-layer heights, defined in DetectorConstruction.cc
    G4double mst_r(G4int j), mst_r_rel(G4int j); // per-layer radius (absolute and relative to the gross radial position), defined in DetectorConstruction.cc

    // spacer cross-section shapes
    G4double spc_r_overlap = 2*mm; // overlapping portion between spacers in two successive columns along radius
    G4double spc_hgt(G4int j); // per-layer heights, defined in DetectorConstruction.cc
    G4double spc_sidegap(G4int j); // per-layer width of the side gap, defined in DetectorConstruction.cc
    G4double spc_r(G4int j), spc_r_rel(G4int j); // per-layer radius (absolute and relative to the gross radial position), defined in DetectorConstruction.cc

	// scintillating tile cross-section shapes
    G4double sci_r_gap = 1*mm; // 1-side air gap between spacer and scintillator tile (removed from scintillator) along radius
    G4double sci_hgt(G4int j); // per-layer heights, defined in DetectorConstruction.cc
    G4double sci_sidegap(G4int j); // per-layer width of the side gap, defined in DetectorConstruction.cc
    G4double sci_r(G4int j), sci_r_rel(G4int j); // per-layer radius (absolute and relative to the gross radial position), defined in DetectorConstruction.cc

    // other miscellaneous geometric parameters
    G4double inner_gap = 0.4*mm; // gap between adjacent scintillating tiles in the same module slot (full) - side gap will be increased accordingly, to keep sides aligned with spacers
    G4double fibre_r = 0.65*mm; // WLS fibre radius 
    G4double hole_r = 0; // radius of the pipe/rod holes in the tiles
    G4double hole_x = 0; // x position of the pipe/rod holes in the tiles
    G4double hole_y = 0; // y position of the pipe/rod holes in the tiles

    // function to create and place a whole module, defined in DetectorConstruction.cc
    G4LogicalVolume* CreateModule(
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
    );
		
	//// specific for DetectorConstruction_tile ////

    // all about isosceles trapezoids (full or half)
    class geomTrapezoid{
        public:
            geomTrapezoid(G4double r, G4double h, G4double theta) {
                R = r; // R: central radial position, i.e. distance between the height mid-point and the point in which the prolongations of the two non-parallel sides meet
                Theta = theta; // Theta: full angular aperture between the two non-parallel sides 
                H = h; // H: full height

                SetAllDerived();

                // derived quantities:
                // - R_b: radial position of the lower base
                // - R_t: radial position of the upper base
                // - L_b: full length of the lower base
                // - L_t: full length of the upper base
                // - L_mid: full width (i.e. horizontal length) at mid height
                // - side: full length of one of the non-parallel sides
            }
            
            void SetR(G4double r) {R = r; SetAllDerived();}
            void SetTheta(G4double theta) {Theta = theta; SetAllDerived();}
            void SetH(G4double h) {H = h; SetAllDerived();}
            G4double GetR() {return R;}
            G4double GetTheta() {return Theta;}
            G4double GetH() {return H;}

            void SetR_b(G4bool override=false, G4double newval=0); // defined in DetectorConstruction_tile.cc
            void SetR_t(G4bool override=false, G4double newval=0); // defined in DetectorConstruction_tile.cc
            G4double GetR_b() {return R_b;}
            G4double GetR_t() {return R_t;}

            void SetDHor_b(G4bool override=false, G4double newval=0); // defined in DetectorConstruction_tile.cc
            void SetDHor_t(G4bool override=false, G4double newval=0); // defined in DetectorConstruction_tile.cc
            G4double GetDHor_b() {return L_b;}
            G4double GetDHor_t() {return L_t;}
            G4double GetDVer() {return GetH();} // just an alias for GetH

            void SetDHor_mid() {L_mid = fL_mid(GetDHor_b(), GetDHor_t());}
            void SetDSide() {side = fside(GetDVer(), GetTheta());}
            G4double GetDHor_mid() {return L_mid;}
            G4double GetDSide() {return side;}

            G4bool GetIsConsistent() {return isConsistent;}
            G4bool GetIsGaps() {return isGaps;}

            // calculate and get the (horizontal) distance between the full tile and the half tile
            G4double GetFullToHalfCentreOffset() {return L_mid/4;}

            // update all derived variables
            virtual void SetAllDerived() {
                SetR_b();
                SetR_t();
                SetDHor_b();
                SetDHor_t();
                SetDHor_mid();
                SetDSide();
                
                isConsistent = true;
            }

            void AddHorGaps(G4double gapsize); // defined in DetectorConstruction_tile.cc
            void RmHorGaps(); // defined in DetectorConstruction_tile.cc

        protected:
            G4double pi = DetectorConstruction::pi;

            G4double H;
            G4double R;
            G4double Theta;

            G4double R_b, R_t;
            G4double L_b, L_t;
            G4double L_mid;
            G4double side;

            G4bool isConsistent = false;
            G4bool isConsistentRectangle = false;

            G4bool isGaps = false;
            G4double GapHor = 0;

            G4double fR_b(G4double r, G4double h) {return r - h/2;}
            G4double fR_t(G4double r, G4double h) {return r + h/2;}

            G4double fL_b(G4double r, G4double h, G4double theta) {
                return 2 * fR_b(r, h) * sin(theta/2) / cos (theta/2);
            }
            G4double fL_t(G4double r, G4double h, G4double theta) {
                return 2 * fR_t(r, h) * sin(theta/2) / cos (theta/2);
            }

            G4double fL_mid(G4double l_b, G4double l_t) {
                return 0.5 * abs(l_t + l_b);
            }

            G4double fside(G4double h, G4double theta) { 
                return (h / sin(pi/2 - theta/2));
            }
    };

    // rectangular geometry - derived from geomTrapezoid
    class geomRectangle : public geomTrapezoid
    {
        public:
            geomRectangle(G4double w, G4double h) : geomTrapezoid(0, h, 0) {
                R = 0; // R: meaningless in case of rectangular tiles
                Theta = 0; // Theta: meaningless in case of rectangular tiles
                H = h; // H: full height

                W = w; // W: full width

                SetAllDerived();

                // derived quantities - trivial in case of rectangular tiles:
                // - R_b: radial position of the lower base --> = 0
                // - R_t: radial position of the upper base --> = 0
                // - L_b: full length of the lower base --> = W
                // - L_t: full length of the upper base --> = W
                // - L_mid: full width (i.e. horizontal length) at mid height --> = W
                // - side: full length of one of the non-parallel sides --> = H
            }  

            void SetW(G4double w) {W = w; SetAllDerived();}
            G4double GetW() {return W;}

            // update all derived variables - special version for rectangular tiles
            void SetAllDerived() override {
                SetR_b(true, 0);
                SetR_t(true, 0);
                SetDHor_b(true, W);
                SetDHor_t(true, W);
                SetDHor_mid();
                SetDSide();

                isConsistentRectangle = true;
            }
            
        protected:
            G4double W;
    };

    G4VSolid* fShapeTileFull( // defined in DetectorConstruction_tile.cc
        G4String name, 
        geomTrapezoid* pGeom, 
        G4double dThk
    );

    G4LogicalVolume* fLogTile( // defined in DetectorConstruction_tile.cc
        G4String name, 
        G4Material* pMaterial, 
        G4VisAttributes* pColour,
        geomTrapezoid* pGeom, 
        G4double dThk,
        G4int signHalf,
        G4double holeR,
        G4double holeX,
        G4double holeY
    );

    G4LogicalVolume* fLogPlaceFibreCirc( // defined in DetectorConstruction_tile.cc
        G4String name, 
        G4Material* pMaterial, 
        G4VisAttributes* pColour,
        geomTrapezoid* pTileGeom, 
        G4LogicalVolume* pEnvelope,
        G4double sectionR,
        G4double extraRIn,
        G4double extraRLOut,
        G4ThreeVector tilePos,
        G4RotationMatrix* pTileRot,
        G4int signHalf
    );
	
    G4LogicalVolume* fLogPlaceFibreCirc( // defined in DetectorConstruction_tile.cc
        G4String name, 
        G4Material* pMaterial, 
        G4VisAttributes* pColour,
        geomTrapezoid* pTileGeom, 
        G4LogicalVolume* pEnvelope,
        G4double sectionR,
        G4double extraRIn,
        G4double extraRLOut,
        G4Transform3D tilePosRot,
        G4int signHalf
    );
	
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
    // test setup (implemented in src/TestMode.cc)
    G4VPhysicalVolume* SetupTest(G4NistManager* nist);
    void SDTest(G4SDManager* sdm);
};

#endif
