#ifndef DETECTOR_CONSTRUCTION_HH
#define DETECTOR_CONSTRUCTION_HH

#include <G4SystemOfUnits.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4NistManager.hh>
#include <G4SDManager.hh>

#include <G4Trd.hh>

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

    class geomTrapezoid
    {
        public:
            geomTrapezoid(G4double r, G4double h, G4double theta) {
                H = h;
                R = r;
                Theta = theta;
            }
            void SetH(G4double h) {H = h;}
            void SetR(G4double r) {R = r;}
            void SetTheta(G4double theta) {Theta = theta;}

            G4double GetH() {return H;}
            G4double GetR() {return R;}
            G4double GetTheta() {return Theta;}

            G4double GetDVer() {return H;}
            G4double GetDHor_b() {return L_b(R, H, Theta);}
            G4double GetDHor_t() {return L_t(R, H, Theta);}

            G4double GetDHor_mid() {return L_mid(R, H, Theta);}

            G4double GetFullToHalfCentreOffset() {return L_mid(R, H, Theta)/4;}

        private:
            G4double H;
            G4double R;
            G4double Theta;

            G4double R_b(G4double r, G4double h) {return r - h/2;}
            G4double R_t(G4double r, G4double h) {return r + h/2;}

            G4double L_b(G4double r, G4double h, G4double theta) {
                return R_b(r, h) * sin(theta/2) / cos (theta/2);
            }
            G4double L_t(G4double r, G4double h, G4double theta) {
                return R_t(r, h) * sin(theta/2) / cos (theta/2);
            }

            G4double L_mid(G4double r, G4double h, G4double theta) {
                return 0.5 * (L_t(r, h, theta) - L_b(r, h, theta));
            }

            G4double side(G4double r, G4double h, G4double theta) {
                return (L_t(r, h, theta) - L_b(r, h, theta)) * sin(theta/2) / sin(theta);
            }

    };

    G4LogicalVolume* fLogTileFullLarge(
        geomTrapezoid* pGeom, 
        G4String name, 
        G4double dThk,
        G4Material* pMaterial, 
        G4VisAttributes* pColour
    ){
        G4double dHor_b = pGeom->GetDHor_b();
        G4double dHor_t = pGeom->GetDHor_t();
        G4double dVer = pGeom->GetDVer();

        G4String name_solid = name + "_Shape";
        G4VSolid* shape = new G4Trd(name_solid, dHor_b/2, dHor_t/2, dThk/2, dThk/2, dVer/2);

        G4String name_log = name + "_Log";
        G4LogicalVolume* logvol = new G4LogicalVolume(shape, pMaterial, name_log);
        logvol->SetVisAttributes(pColour);

        return logvol;
    }

    G4LogicalVolume* fLogTileFullWGap(
        geomTrapezoid* pGeom, 
        G4String name, 
        G4double dThk,
        G4double dGap,
        G4Material* pMaterial, 
        G4VisAttributes* pColour
    ){
        G4double dGapHor = dGap / cos(pGeom->GetTheta() / 2);

        G4double dHor_b = pGeom->GetDHor_b() - 2*dGapHor;
        G4double dHor_t = pGeom->GetDHor_t() - 2*dGapHor;
        G4double dVer = pGeom->GetDVer();

        G4String name_solid = name + "_Shape";
        G4VSolid* shape = new G4Trd(name_solid, dHor_b/2, dHor_t/2, dThk/2, dThk/2, dVer/2);

        G4String name_log = name + "_Log";
        G4LogicalVolume* logvol = new G4LogicalVolume(shape, pMaterial, name_log);
        logvol->SetVisAttributes(pColour);

        return logvol;
    }
	
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
    // test setup (implemented in src/TestMode.cc)
    G4VPhysicalVolume* SetupTest(G4NistManager* nist);
    void SDTest(G4SDManager* sdm);
};

#endif
