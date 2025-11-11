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

    // all about isosceles trapezoids
    class geomTrapezoid
    {
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

            void SetR_b() {R_b = fR_b(GetR(), GetH());}
            void SetR_t() {R_t = fR_t(GetR(), GetH());}
            G4double GetR_b() {return R_b;}
            G4double GetR_t() {return R_t;}

            void SetDHor_b() {L_b = fL_b(GetR(), GetH(), GetTheta());}
            void SetDHor_t() {L_t = fL_t(GetR(), GetH(), GetTheta());}
            G4double GetDHor_b() {return L_b;}
            G4double GetDHor_t() {return L_t;}
            G4double GetDVer() {return GetH();} // just an alias for GetH

            void SetDHor_mid() {L_mid = fL_mid(GetDHor_b(), GetDHor_t());}
            void SetDSide() {side = fside(GetDVer(), GetTheta());}
            G4double GetDHor_mid() {return L_mid;}
            G4double GetDSide() {return side;}

            // update all derived variables
            void SetAllDerived() {
                SetR_b();
                SetR_t();
                SetDHor_b();
                SetDHor_t();
                SetDHor_mid();
                SetDSide();
            }

            // calculate and get the (horizontal) distance between the full tile and the half tile
            G4double GetFullToHalfCentreOffset() {return L_mid/4;}

            // reduce the horizontal dimensions to account for the fibre gaps
            // only (some) horizontal quantities are affected
            void AddHorGaps(G4double gapsize) {
                GapHor = gapsize / cos(GetTheta() / 2);
                isGaps = true;

                L_b = GetDHor_b() - 2*GapHor;
                L_t = GetDHor_t() - 2*GapHor;
                SetDHor_mid(); // recalculate L_mid
            }

            //
            void RmHorGaps() {
                if (isGaps) {
                    L_b = GetDHor_b() + 2*GapHor;
                    L_t = GetDHor_t() + 2*GapHor;
                    SetDHor_mid(); // recalculate L_mid

                    GapHor = 0;
                    isGaps = false;
                } else {return;}
            }

        private:
            G4double pi = acos(-1);

            G4double H;
            G4double R;
            G4double Theta;

            G4double R_b, R_t;
            G4double L_b, L_t;
            G4double L_mid;
            G4double side;

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

    // create the full tile solid
    G4VSolid* fShapeTileFull(
        geomTrapezoid* pGeom, 
        G4String name, 
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
    G4LogicalVolume* fLogTile(
        geomTrapezoid* pGeom, 
        G4String name, 
        G4double dThk,
        G4Material* pMaterial, 
        G4VisAttributes* pColour,

        G4int signHalf = 0, // if 0, full tile is kept; if > (<) 0 , half tile is selected from positive (negative) half

        G4double holeR = 0., // radius of the service holes - if 0, no holes are modelled
        G4double holeX = 0., // hole centre, horizontal distance from tile centre
        G4double holeY = 0. // hole centre, vertical distance from tile centre
        // note: two holes are always added to the full tile: one in (holeX, holey), one in (-holeX, -holey)
    ){
        G4VSolid* shape_full = fShapeTileFull(pGeom, name, dThk);
        G4VSolid* shape_fin;

        G4String name_solid = name + "_Shape";

        // model service holes
        G4VSolid* shape_holes;
        if (holeR > 0) {
            G4RotationMatrix* holeRotation = new G4RotationMatrix();
            holeRotation->rotateX(90 * deg);

            G4VSolid* shape_hole = new G4Tubs("hole_Shape", 0., holeR, dThk / 2, 0., 2*acos(-1));

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
	
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
    // test setup (implemented in src/TestMode.cc)
    G4VPhysicalVolume* SetupTest(G4NistManager* nist);
    void SDTest(G4SDManager* sdm);
};

#endif
