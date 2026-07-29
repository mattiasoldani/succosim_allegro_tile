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


/* instructions:
- TARGET_B_ANY=0, TILECERN_B_ANY=*, TILECERN_B_S6=* --> no stacks, only ancillary detectors
- TARGET_B_ANY=1, TILECERN_B_ANY=0, TILECERN_B_S6=* --> configuration for Bethe-Bloch measurement
- TARGET_B_ANY=1, TILECERN_B_ANY=1, TILECERN_B_S6=0 --> configuration for CERN calorimetric stack, CERN S2
- TARGET_B_ANY=1, TILECERN_B_ANY=1, TILECERN_B_S6=1 --> configuration for CERN calorimetric stack, CERN S6
*/ 
#define TARGET_B_ANY 1
#define TILECERN_B_ANY 1
#define TILECERN_B_S6 1

#define TILECERN_N 10
#define TILEFZU_N 32

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
	
	//// specific for this application ////

    // misc general stuff
    G4double gen_gap = 1*mm;
    G4double gen_fibreradius = 0.5*mm; // radius of the WLS fibres
    G4double gen_thk = 3*mm; // tile thickness
    G4double gen_holeradius = 0*mm; // radius of the pipe/rod hole
    G4double gen_sidegap = 0*mm; // side reduction to account for the fibres

    // world
    G4double worldSizeX = 2 * m;
    G4double worldSizeY = 2 * m;
    G4double worldSizeZ = 30 * m;

    // general positioning (source will be in world centre)
    G4double z_scintiSmall0_front = (0)*cm;
    G4double z_scintiSmall1_front = (817)*cm;
    G4double z_scintiBig0_front = (817+43.5+9+51.5+29.5-46-8.3)*cm;
    G4double z_scintiBig1_front = (817+43.5+9+51.5+29.5+91.5+70.1+49.5+57.4)*cm;
    G4double z_pipe0_front = (30)*cm; // not measured
    G4double z_pipe1_front = (817-34.5-259)*cm;
    G4double z_pbGl_front = (817+43.5+9+51.5+29.5+91.5+70.1)*cm;
    G4double z_tileCern_front = (817+43.5+9+51.5+29.5+36)*cm;
    G4double z_FZU_front = (817+43.5+9+51.5+29.5+7)*cm;
    G4double z_FZU_rear = z_FZU_front + TILEFZU_N*(gen_thk+gen_gap); 
    G4double passive_S6_shift = 47.17*mm;
    G4double passive_trig_shift = 59*mm;

	// tile shapes - CERN S2
	G4double FZU_ang_x = -90*deg;
	G4double FZU_ang_y = 0*deg;
	G4double FZU_ang_z = 0*deg;
    G4double FZU_w = 70.5*mm; // short-side width (half-module)
    G4double FZU_W = 75*mm; // long-side width (half-module)
    G4double FZU_h = 97*mm; // height
    G4double FZU_thk = gen_thk;
    G4double FZU_fibreradius = gen_fibreradius;
    G4double FZU_sidegap = gen_sidegap;
    G4double FZU_holeradius = gen_holeradius;
    G4double FZU_holey = 0; // pipe/rod hole centre y (relative to full-module tile centre)
    G4double FZU_holex = 0; // pipe/rod hole centre x (relative to full-module tile centre)

    G4double FZU_zgap = gen_gap; // longitudinal gap between successive tiles, FZU only

	// tile shapes - CERN S2
	G4double S2_ang_x = 90*deg;
	G4double S2_ang_y = 180*deg;
	G4double S2_ang_z = 0*deg;
    G4double S2_w = 70.5*mm; // short-side width (half-module)
    G4double S2_W = 75*mm; // long-side width (half-module)
    G4double S2_h = 97*mm; // height
    G4double S2_thk = gen_thk;
    G4double S2_fibreradius = gen_fibreradius;
    G4double S2_sidegap = gen_sidegap;
    G4double S2_holeradius = gen_holeradius;
    G4double S2_holey = S2_h/2 - 6*mm; // pipe/rod hole centre y (relative to full-module tile centre)
    G4double S2_holex = 6*mm; // pipe/rod hole centre x (relative to full-module tile centre)

    G4double S2_xgap = gen_gap; // transverse (horizontal) gap between adjacent tiles, S2 only 

	G4double S2_fzrel(G4int i), S2_fxrel(G4int i), S2_fyrel(G4int i); // functions for iterative tile placing, defined in DetectorConstruction.cc

	// tile shapes - CERN S6
	G4double S6_ang_x = 90*deg;
	G4double S6_ang_y = -90*deg;
	G4double S6_ang_z = 0*deg;
    G4double S6_w = 91*mm; // short-side width (half-module)
    G4double S6_W = 100*mm; // long-side width (half-module)
    G4double S6_h = 187*mm; // height
    G4double S6_thk = gen_thk;
    G4double S6_fibreradius = gen_fibreradius;
    G4double S6_sidegap = gen_sidegap;
    G4double S6_holeradius = gen_holeradius;
    G4double S6_holey = S6_h/2 - 6*mm; // pipe/rod hole centre y (relative to full-module tile centre)
    G4double S6_holex = 6*mm; // pipe/rod hole centre x (relative to full-module tile centre)

	G4double S6_fzrel(G4int i), S6_fxrel(G4int i), S6_fyrel(G4int i); // functions for iterative tile placing, defined in DetectorConstruction.cc

    // steel tile shapes
	G4double passive_ang_x = 90*deg;
	G4double passive_ang_y = 0*deg;
	G4double passive_ang_z = 0*deg;
    G4double passive_thk_u = 9.9*mm;
    G4double passive_w_u = 17.38*cm;
    G4double passive_thk_d = 14.5*mm;
    G4double passive_w_d = 18.08*cm;
    G4double passive_h = 19.2*cm;
    G4double passive_thk_gap = gen_gap;

    G4double passive_thk_gross = passive_thk_u + passive_thk_d + 2*passive_thk_gap;
		
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
