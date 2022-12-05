//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file SteppingAction.cc
/// \brief Implementation of the B1::SteppingAction class

#include "SteppingAction.hh"
#include "EventAction.hh"
#include "DetectorConstruction.hh"

#include "G4Step.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4LogicalVolume.hh"
#include"G4SystemOfUnits.hh"

#include<cmath>

namespace B1
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::SteppingAction(EventAction* eventAction, RunAction* runAction)
: fEventAction(eventAction), fRunAction(runAction)
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::~SteppingAction()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::UserSteppingAction( const G4Step *step )
{
  G4Track* track = step->GetTrack();
  G4StepPoint* stepPoint_pre  = step->GetPreStepPoint();
  G4StepPoint* stepPoint_post = step->GetPostStepPoint();

  if( track->GetParentID() == 0 ) {
    prev_energy = track->GetKineticEnergy()/MeV;
    prev_len    = track->GetTrackLength()  /cm;
  }

  if( track->GetParentID() == 0 && stepPoint_post->GetStepStatus() == 1 ) { // Save primary info if crossing border
    // G4cout << "Boundary!" << G4endl;
    if( first_step ) 
      first_step = false;
    else {
      auto analysisManager = G4AnalysisManager::Instance();
      analysisManager->FillNtupleDColumn( 0, 0, prev_energy );                // current energy
      analysisManager->FillNtupleDColumn( 0, 1, prev_energy - prev_boundary_energy );  // delta energy
      analysisManager->FillNtupleDColumn( 0, 2, prev_len );                   // current track length
      analysisManager->FillNtupleDColumn( 0, 3, prev_len - prev_boundary_len );        // delta track length
      analysisManager->FillNtupleDColumn( 0, 4, ( prev_energy - prev_boundary_energy ) // dEdX
                                                / ( prev_len - prev_boundary_len ) );    
      analysisManager->AddNtupleRow(0);
    }
    prev_boundary_energy = prev_energy;
    prev_boundary_len    = prev_len;
    return;
  }
  else if( track->GetParticleDefinition()->GetParticleSubType() == "photon" ){ // Save photon info then kill
    // G4cout << "Photon!" << G4endl;
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->FillNtupleDColumn( 1, 0, prev_energy );
    analysisManager->FillNtupleDColumn( 1, 1, prev_len );
    analysisManager->FillNtupleDColumn( 1, 2, acos( track->GetMomentumDirection().x()/cm / ( track->GetMomentumDirection().mag()/cm ) ) );
    analysisManager->FillNtupleDColumn( 1, 3, track->GetTotalEnergy()/MeV );
    analysisManager->FillNtupleIColumn( 1, 4, std::hash<std::string>()(track->GetCreatorProcess()->GetProcessName()) );
    analysisManager->FillNtupleSColumn( 1, 5, track->GetCreatorProcess()->GetProcessName() );
    analysisManager->AddNtupleRow(1);
    track->SetTrackStatus( fKillTrackAndSecondaries );
  }
  else if( track->GetParticleDefinition()->GetPDGEncoding() ==  12 || // Kill neutrinos
           track->GetParticleDefinition()->GetPDGEncoding() == -12 ||
           track->GetParticleDefinition()->GetPDGEncoding() ==  14 ||
           track->GetParticleDefinition()->GetPDGEncoding() == -14 ||
           track->GetParticleDefinition()->GetPDGEncoding() ==  16 ||
           track->GetParticleDefinition()->GetPDGEncoding() == -16 ||
           track->GetParticleDefinition()->GetPDGEncoding() ==  18 ||
           track->GetParticleDefinition()->GetPDGEncoding() == -18 ){
           track->SetTrackStatus( fKillTrackAndSecondaries );
  }
  else if( track->GetParentID() != 0 ){
    // G4cout << "Particle: " << track->GetParticleDefinition()->GetParticleName() 
    //        << " || PDG: " << track->GetParticleDefinition()->GetPDGEncoding()
    //        << " || Step Status: " << stepPoint_post->GetStepStatus()
    //        << " || ParentID: " << track->GetParentID()
    //        << " || KE: " << track->GetKineticEnergy()
    //        << " || Volume: " << track->GetVolume()->GetName() << G4endl;
  }
}

};
