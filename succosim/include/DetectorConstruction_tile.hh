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

// all about rectangles - derived from geomTrapezoid
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

static G4VSolid* fShapeTileFull( // defined in DetectorConstruction_tile.cc
    G4String name,
    geomTrapezoid* pGeom,
    G4double dThk
);

static G4LogicalVolume* fLogTile( // defined in DetectorConstruction_tile.cc
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

static G4LogicalVolume* fLogPlaceFibreCirc( // defined in DetectorConstruction_tile.cc
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

static G4LogicalVolume* fLogPlaceFibreCirc( // defined in DetectorConstruction_tile.cc
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

// all about full TileCal modules
class fullTileCalModule {
    public:
        // structure to pass settings to the module constructor
        struct ModConfig {
            // global construction settings - to be set, mandatory
            G4int n_periods; // number of longitudinal periods
            G4int n_layers; // number of radial layers
            G4bool b_place_support; // if true (false), place (hide) external support

            // single-element thicknesses which define the period thickness - to be set, optional
            G4double mst_thk = 5*mm; // master thickness
            G4double spc_thk = 4*mm; // spacer thickness
            G4double sci_thk = 3*mm; // tile thickness

            // support/catcher specific features - to be set, optional
            G4double front_thk = 20*mm; // thickness (along radial direction) of the front plate
            G4double back_thk = 41*mm; // thickness (along radial direction) of the back structure
            G4double side_thk = 20*mm; // thickness (along longitudinal direction) of the side plates
            G4double catcher_thk = 1*mm; // thickness of all catcher plates around the envelope

            // global module features - to be set, optional
            G4double mod_rmin = 2.8*m; // module minimum radius - net (sensitive volume only)
            G4double mod_dphi = 2*pi/128; // module full azimuthal opening - readout segmentation will be halved

            // layer/period settings - to be set, optional
            G4int n_layers_A = 4; // number of tier-A layers
            G4int n_layers_B = 6; // number of tier-B layers
            G4int n_layer_per_profile = 8;
            G4double til_hgt_A = 55*mm; // height of tier-A layers
            G4double til_hgt_B = 105*mm; // height of tier-B layers
            G4double til_hgt_C = 205*mm; // height of tier-C layers

            // spacer/scintillating tile cross-section shapes - to be set, optional
            G4double spc_r_overlap = 2*mm; // overlapping portion between spacers in two successive columns along radius
            G4double sci_r_gap = 1*mm; // 1-side air gap between spacer and scintillator tile (removed from scintillator) along radius

            // other miscellaneous geometric parameters - to be set, optional
            G4double fibre_profile_thk = 2.6*mm; // cross-section of the single fibre profile
            G4double inner_gap = 0.4*mm; // gap between adjacent scintillating tiles in the same module slot (full) - side gap will be increased accordingly, to keep sides aligned with spacers
            G4double fibre_r = 0.65*mm; // WLS fibre radius
            G4double hole_r = 0; // radius of the pipe/rod holes in the tiles
            G4double hole_x = 0; // x position of the pipe/rod holes in the tiles
            G4double hole_y = 0; // y position of the pipe/rod holes in the tiles
            G4double l_fibre_extra = 0.5*cm; // out-of-tile extra length of the fibre towards the drawer

            // material settings - to be set, optional
            G4Material* mat_passive = G4NistManager::Instance()->FindOrBuildMaterial("G4_STAINLESS-STEEL");
            G4Material* mat_support = G4NistManager::Instance()->FindOrBuildMaterial("G4_STAINLESS-STEEL");
            G4Material* mat_scintillator = G4NistManager::Instance()->FindOrBuildMaterial("G4_POLYSTYRENE");
            G4Material* mat_fibre = G4NistManager::Instance()->FindOrBuildMaterial("G4_POLYSTYRENE");
            G4Material* mat_envelope = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");
            G4Material* mat_catcher = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");

            // visual settings - to be set, optional
            G4VisAttributes* col_passive = new G4VisAttributes(G4Colour::Grey());
            G4VisAttributes* col_support = new G4VisAttributes(G4Colour::Grey());
            G4VisAttributes* col_scintillator = new G4VisAttributes(G4Colour::Cyan());
            G4VisAttributes* col_fibre = new G4VisAttributes(G4Colour::Green());
            G4VisAttributes* col_envelope = new G4VisAttributes(false);
            G4VisAttributes* col_catcher_front_back = new G4VisAttributes(false);
            G4VisAttributes* col_catcher_side = new G4VisAttributes(false);
            G4VisAttributes* col_catcher_phi = new G4VisAttributes(false);
        };

        fullTileCalModule(
            G4String id,
            ModConfig& config
        );

        // defined in DetectorConstruction_tile.cc
        void PlaceLog(
            G4LogicalVolume* worldLog,
            G4Transform3D& stack_pos_rot,
            G4bool b_place_catcher_front,
            G4bool b_place_catcher_back,
            G4bool b_place_catcher_side0,
            G4bool b_place_catcher_side1,
            G4bool b_place_catcher_phi0,
            G4bool b_place_catcher_phi1
        );

        // defined in DetectorConstruction_tile.cc
        void CreateAllSDs(
            G4SDManager* sdm,
            G4int coarse_ro
        );

        // defined in DetectorConstruction_tile.cc
        static void CreateNtupleColumns(
            G4AnalysisManager* analysis,
            G4int coarse_ro,
            G4String mod_id,
            G4int n_periods,
            G4int n_layers
        );

        // defined in DetectorConstruction_tile.cc
        static G4int FillNtupleColumns(
            G4AnalysisManager* analysis,
            G4SDManager* sdm,
            G4int coarse_ro,
            G4HCofThisEvent* hcofEvent,
            G4int col,
            G4String mod_id,
            G4int n_periods,
            G4int n_layers
        );

        // global construction settings - to be set, mandatory
        G4int GetNPeriods() { return n_periods; } // number of longitudinal periods
        G4int GetNLayers() { return n_layers; } // number of radial layers
        G4bool GetBPlaceSupport() { return b_place_support; } // if true (false), place (hide) external support

        // single-element thicknesses which define the period thickness - to be set, optional
        G4double GetMstThk() { return mst_thk; } // master thickness
        G4double GetSpcThk() { return spc_thk; } // spacer thickness
        G4double GetSciThk() { return sci_thk; } // tile thickness

        // single-element thicknesses which define the period thickness - derived
        G4double GetPeriodThk() { return period_thk(); } // thickness of the full period

        // support/catcher specific features - to be set, optional
        G4double GetFrontThk() { return front_thk; } // thickness (along radial direction) of the front plate
        G4double GetBackThk() { return back_thk; } // thickness (along radial direction) of the back structure
        G4double GetSideThk() { return side_thk; } // thickness (along longitudinal direction) of the side plates
        G4double GetCatcherThk() { return catcher_thk; } // thickness of all catcher plates around the envelope

        // global module features - to be set, optional
        G4double GetModRMin() { return mod_rmin; } // module minimum radius - net (sensitive volume only)
        G4double GetModDPhi() { return mod_dphi; } // module full azimuthal opening - readout segmentation will be halved

        // global module features - derived
        G4double GetModRMinEnv() { return mod_rmin_env(); } // module minimum radius - gross (envelope volume)
        G4double GetModRadial() { return mod_radial(); } // module radial extension - net (sensitive volume only)
        G4double GetModRadialEnv() { return mod_radial_env(); } // module radial extension - gross (envelope volume)
        G4double GetModThk() { return mod_thk(); } // module thickness along longitudinal direction (orthogonal to tiles) - net (sensitive volume only)
        G4double GetModThkEnv() { return mod_thk_env(); } // module thickness along longitudinal direction (orthogonal to tiles) - gross
        G4double GetModCentreRel() { return mod_centre_rel(); } // radial centre of the module net part relative to the gross size

        // layer/period settings - to be set, optional
        G4int GetNLayersA() { return n_layers_A; } // number of tier-A layers
        G4int GetNLayersB() { return n_layers_B; } // number of tier-B layers
        G4int GetNLayerPerProfile() { return n_layer_per_profile; }
        G4double GetTilHgtA() { return til_hgt_A; } // height of tier-A layers
        G4double GetTilHgtB() { return til_hgt_B; } // height of tier-B layers
        G4double GetTilHgtC() { return til_hgt_C; } // height of tier-C layers

        // master (partial) cross-section shapes etcetera - derived
        G4double GetMstRRel(G4int j) { return mst_r_rel(j); } // per-layer radius (relative to the gross radial position)
        G4double GetMstTransv(G4int i) { return mst_transv(i); } // functions for iterative element placing, master
        G4double GetMstHgt(G4int j) { return mst_hgt(j); } // per-layer heights
        G4double GetMstR(G4int j) { return mst_r(j); } // per-layer radius (absolute)

        // spacer/scintillating tile cross-section shapes - to be set, optional
        G4double GetSpcROverlap() { return spc_r_overlap; } // overlapping portion between spacers in two successive columns along radius
        G4double GetSciRGap() { return sci_r_gap; } // 1-side air gap between spacer and scintillator tile (removed from scintillator) along radius

        // spacer/scintillating tile cross-section shapes - derived
        G4double GetSpcHgt(G4int j) { return spc_hgt(j); } // per-layer heights
        G4double GetSpcSidegap(G4int j) { return spc_sidegap(j); } // per-layer width of the side gap
        G4double GetSpcR(G4int j) { return spc_r(j); } // per-layer radius (absolute)
        G4double GetSpcRRel(G4int j) { return spc_r_rel(j); } // per-layer radius (relative to the gross radial position)
        G4double GetSciHgt(G4int j) { return sci_hgt(j); } // per-layer heights
        G4double GetSciSidegap(G4int j) { return sci_sidegap(j); } // per-layer width of the side gap
        G4double GetSciR(G4int j) { return sci_r(j); } // per-layer radius (absolute)
        G4double GetSciRRel(G4int j) { return sci_r_rel(j); } // per-layer radius (relative to the gross radial position)
        G4double GetTilTransv(G4int i) { return til_transv(i); } // functions for iterative element placing, tile

        // other miscellaneous geometric parameters - to be set, optional
        G4double GetFibreProfileThk() { return fibre_profile_thk; } // cross-section of the single fibre profile
        G4double GetInnerGap() { return inner_gap; } // gap between adjacent scintillating tiles in the same module slot (full) - side gap will be increased accordingly, to keep sides aligned with spacers
        G4double GetFibreR() { return fibre_r; } // WLS fibre radius
        G4double GetHoleR() { return hole_r; } // radius of the pipe/rod holes in the tiles
        G4double GetHoleX() { return hole_x; } // x position of the pipe/rod holes in the tiles
        G4double GetHoleY() { return hole_y; } // y position of the pipe/rod holes in the tiles
        G4double GetLFibreExtra() { return l_fibre_extra; } // out-of-tile extra length of the fibre towards the drawer

    private:
        // defined in DetectorConstruction_tile.cc
        void CreateLog();

        // global construction settings - to be set, mandatory
        G4String mod_id; // module name (will be used in all volume names)
        G4int n_periods; // number of longitudinal periods
        G4int n_layers; // number of radial layers
        G4bool b_place_support; // if true (false), place (hide) external support

        // module creation logical volumes, internal (i.e. placed in the envelope) - output
        G4LogicalVolume* frontLog = nullptr;
        G4LogicalVolume* backLog = nullptr;
        G4LogicalVolume* sideLogs[2] = {};
        G4LogicalVolume* mstLogs[NPERIODSMAX*NLAYERSMAX*2] = {};
        G4LogicalVolume* spcLogs[NPERIODSMAX*NLAYERSMAX] = {};
        G4LogicalVolume* sciLogs[NPERIODSMAX*NLAYERSMAX*2] = {};
        G4LogicalVolume* fibreLogs[NPERIODSMAX*NLAYERSMAX*2] = {};

        // module creation logical volumes, envelope and external catchers - output
        geomTrapezoid* modEnvGeom = nullptr;
        G4LogicalVolume* modEnvLog = nullptr;
        G4LogicalVolume* catcherFrontLog = nullptr;
        G4LogicalVolume* catcherBackLog = nullptr;
        G4LogicalVolume* catcherSideLogs[2] = {};
        G4LogicalVolume* catcherPhiLogs[2] = {};

        // single-element thicknesses which define the period thickness - to be set, optional
        G4double mst_thk; // master thickness
        G4double spc_thk; // spacer thickness
        G4double sci_thk; // tile thickness

        // single-element thicknesses which define the period thickness - derived
        G4double period_thk() { return 2*(mst_thk + spc_thk); } // thickness of the full period

        // support/catcher specific features - to be set, optional
        G4double front_thk; // thickness (along radial direction) of the front plate
        G4double back_thk; // thickness (along radial direction) of the back structure
        G4double side_thk; // thickness (along longitudinal direction) of the side plates
        G4double catcher_thk; // thickness of all catcher plates around the envelope

        // global module features - to be set, optional
        G4double mod_rmin; // module minimum radius - net (sensitive volume only)
        G4double mod_dphi; // module full azimuthal opening - readout segmentation will be halved

        // global module features - derived
        G4double mod_rmin_env() { return mod_rmin - front_thk; } // module minimum radius - gross (envelope volume)
        G4double mod_radial_env() { return mod_radial() + front_thk + back_thk; } // module radial extension - gross (envelope volume)
        G4double mod_thk() { return n_periods * period_thk() - mst_thk; } // module thickness along longitudinal direction (orthogonal to tiles) - net (sensitive volume only)
        G4double mod_thk_env() { return mod_thk() + 2*side_thk; } // module thickness along longitudinal direction (orthogonal to tiles) - gross
        G4double mod_centre_rel() { return front_thk + mod_radial()/2 - mod_radial_env()/2; } // radial centre of the module net part relative to the gross size

        G4double mod_radial() { // module radial extension - net (sensitive volume only)
            G4double mod_radial_temp = 0;
            for (G4int j=0; j<n_layers; j++) {
                mod_radial_temp += (j%2) ? spc_hgt(j) : (sci_hgt(j) + 2*sci_r_gap);
                if (n_layers%2) {mod_radial_temp += spc_r_overlap;}
            }
            return mod_radial_temp;
        }

        // layer/period settings - to be set, optional
        G4int n_layers_A; // number of tier-A layers
        G4int n_layers_B; // number of tier-B layers
        G4int n_layer_per_profile;
        G4double til_hgt_A; // height of tier-A layers
        G4double til_hgt_B; // height of tier-B layers
        G4double til_hgt_C; // height of tier-C layers

        // master (partial) cross-section shapes etcetera - derived
        G4double mst_r_rel(G4int j) { return mst_r(j) - mod_rmin - mod_radial()/2 + mod_centre_rel(); } // per-layer radius (relative to the gross radial position)
        G4double mst_transv(G4int i) { return -mod_thk() / 2 + spc_thk * (1 + i) + mst_thk * (0.5 + i); } // functions for iterative element placing, master

        G4double mst_hgt(G4int j) { // per-layer heights
            if (j<n_layers_A) {return til_hgt_A;}
            else if (j<n_layers_A+n_layers_B) {return til_hgt_B;}
            else {return til_hgt_C;}
        }

        G4double mst_r(G4int j) { // per-layer radius (absolute)
            G4double mst_rmin_temp = mod_rmin;
            if (j>0) {
                for (G4int k=0; k<j; k++) {mst_rmin_temp += mst_hgt(k);}
            }
            return mst_rmin_temp + mst_hgt(j)/2;
        }

        // spacer/scintillating tile cross-section shapes - to be set, optional
        G4double spc_r_overlap; // overlapping portion between spacers in two successive columns along radius
        G4double sci_r_gap; // 1-side air gap between spacer and scintillator tile (removed from scintillator) along radius

        // spacer/scintillating tile cross-section shapes - derived
        G4double spc_hgt(G4int j) { return mst_hgt(j); } // per-layer heights
        G4double spc_sidegap(G4int j) { return fibre_profile_thk*(static_cast<G4int>(floor(j/n_layer_per_profile))+1); } // per-layer width of the side gap
        G4double spc_r(G4int j) { return mst_r(j); } // per-layer radius (absolute)
        G4double spc_r_rel(G4int j) { return mst_r_rel(j); } // per-layer radius (relative to the gross radial position)
        G4double sci_hgt(G4int j) { return spc_hgt(j) - 2*(spc_r_overlap + sci_r_gap); } // per-layer heights
        G4double sci_sidegap(G4int j) { return spc_sidegap(j); } // per-layer width of the side gap
        G4double sci_r(G4int j) { return spc_r(j); } // per-layer radius (absolute)
        G4double sci_r_rel(G4int j) { return spc_r_rel(j); } // per-layer radius (relative to the gross radial position)
        G4double til_transv(G4int i) { return mst_transv(i) - (spc_thk + mst_thk) / 2; } // functions for iterative element placing, tile

        // other miscellaneous geometric parameters - to be set, optional
        G4double fibre_profile_thk; // cross-section of the single fibre profile
        G4double inner_gap; // gap between adjacent scintillating tiles in the same module slot (full) - side gap will be increased accordingly, to keep sides aligned with spacers
        G4double fibre_r; // WLS fibre radius
        G4double hole_r; // radius of the pipe/rod holes in the tiles
        G4double hole_x; // x position of the pipe/rod holes in the tiles
        G4double hole_y; // y position of the pipe/rod holes in the tiles
        G4double l_fibre_extra; // out-of-tile extra length of the fibre towards the drawer

        // material settings - to be set, optional
        G4Material* mat_passive;
        G4Material* mat_support;
        G4Material* mat_scintillator;
        G4Material* mat_fibre;
        G4Material* mat_envelope;
        G4Material* mat_catcher;

        // visual settings - to be set, optional
        G4VisAttributes* col_passive;
        G4VisAttributes* col_support;
        G4VisAttributes* col_scintillator;
        G4VisAttributes* col_fibre;
        G4VisAttributes* col_envelope;
        G4VisAttributes* col_catcher_front_back;
        G4VisAttributes* col_catcher_side;
        G4VisAttributes* col_catcher_phi;
};
