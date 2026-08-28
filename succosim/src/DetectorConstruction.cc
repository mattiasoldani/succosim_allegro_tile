#include <G4SystemOfUnits.hh>
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
#include <G4Trd.hh>

#include <string>

#include "DetectorConstruction.hh"
#include "CustomSD.hh"

// DetectorConstruction::Construct, i.e. where the setup geometry is implemented
G4VPhysicalVolume* DetectorConstruction::Construct()
{
    // adding elements to default manual material CO2 - created in DetectorConstruction.hh
    co2->AddElement(elC, 1);
    co2->AddElement(elO, 2);

    // adding elements to default manual material: BC400 (plastic scintillator) - created in DetectorConstruction.hh
    bc400->AddElement(elH, 0.085);
    bc400->AddElement(elC, 0.915);

    // world
    G4RotationMatrix* worldRotation = new G4RotationMatrix();
    G4VSolid* worldBox = new G4Box("world_Shape", worldSizeX / 2, worldSizeY / 2, worldSizeZ / 2);
    G4LogicalVolume* worldLog = new G4LogicalVolume(worldBox, mat_world, "world_Log");
    worldLog->SetVisAttributes(invisible);
    G4VPhysicalVolume* worldPhys = new G4PVPlacement(nullptr, {}, worldLog, "world", nullptr, false, 0);

    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // define all the setup stuff (materials, volumes) here, or...

    // source the custom messenger
    CustomMessenger* custom = CustomMessenger::Instance();
    G4int n_periods = custom->NPeriods();
    G4int n_layers = custom->NLayers();
    G4int n_stacked_mods = custom->NStackedMods();
    G4bool b_place_support = custom->BPlaceSupport();

    // context-specific colors and material
    G4VisAttributes* col_passive = custom->BShowInner() ? grey : invisible;
    G4VisAttributes* col_support = custom->BShowSupport() ? magenta : invisible;
    G4VisAttributes* col_scintillator = custom->BShowInner() ? cyan : invisible;
    G4VisAttributes* col_fibre = custom->BShowInner() ? green : invisible;
    G4VisAttributes* col_envelope = invisible;
    G4VisAttributes* col_catcher_front_back = custom->BShowCatcherFrontBack() ? green : invisible;
    G4VisAttributes* col_catcher_side = custom->BShowCatcherSide() ? green : invisible;
    G4VisAttributes* col_catcher_phi = custom->BShowCatcherPhi() ? green : invisible;
    G4Material* mat_passive = SS;
    G4Material* mat_support = SS;
    G4Material* mat_scintillator = plastic;
    G4Material* mat_fibre = plastic;
    G4Material* mat_envelope = vacuum;
    G4Material* mat_catcher = vacuum;

    // actual creation and placement of prototype stack
    fullTileCalModule::ModConfig module_config = {n_periods, n_layers, b_place_support};
    module_config.col_passive = col_passive;
    module_config.col_support = col_support;
    module_config.col_scintillator = col_scintillator;
    module_config.col_fibre = col_fibre;
    module_config.col_catcher_front_back = col_catcher_front_back;
    module_config.col_catcher_side = col_catcher_side;
    module_config.col_catcher_phi = col_catcher_phi;

    for (G4int i = 0; i < n_stacked_mods; i++) {
        modules[i] = new fullTileCalModule("M" + std::to_string(i), module_config);

        G4double ang_x_temp = ((2*i - n_stacked_mods + 1) / 2.) * modules[i]->GetModDPhi();
        pos_rot_temp =     
            G4Translate3D(G4ThreeVector(0, 0, zshift))*
            G4Translate3D(G4ThreeVector(0, -sin(ang_x_temp) * modules[i]->GetModRadialEnv()/2, cos(ang_x_temp) * modules[i]->GetModRadialEnv()/2))*   
            G4Translate3D(G4ThreeVector(0, 0, -modules[i]->GetModRMinEnv()))*
            G4Rotate3D(ang_x_temp, G4Vector3D(1,0,0))* 
            G4Rotate3D(0, G4Vector3D(0,1,0))* 
            G4Rotate3D(90*deg, G4Vector3D(0,0,1))*
            G4Translate3D(G4ThreeVector(0, 0, modules[i]->GetModRMinEnv()));

        modules[i]->PlaceLog(
            worldLog,
            pos_rot_temp,
            true,
            true,
            true,
            true,
            i == 0,
            i == n_stacked_mods-1
        );
    }

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

    // module stack readout
    G4int coarse_ro = CustomMessenger::Instance()->CoarseRO();
    G4int n_stacked_mods = CustomMessenger::Instance()->NStackedMods();
    for (G4int i = 0; i < n_stacked_mods; i++) {
        modules[i]->CreateAllSDs(sdm, coarse_ro);
    }
	
    // --------------------------------------------------
    // ...uncomment this line for the test sensitive detectors (implemented in src/TestMode.cc)
    //SDTest(sdm);
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}
