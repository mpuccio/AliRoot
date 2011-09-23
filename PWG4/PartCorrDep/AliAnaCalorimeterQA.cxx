/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/
/* $Id: $ */

//_________________________________________________________________________
// Class to check results from simulations or reconstructed real data. 
// Fill few histograms and do some checking plots
//
//-- Author: Gustavo Conesa (INFN-LNF)
//_________________________________________________________________________


// --- ROOT system ---
//#include "Riostream.h"
#include "TObjArray.h"
#include "TParticle.h"
#include "TDatabasePDG.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TROOT.h"
#include "TH3F.h"
#include "TH2F.h"
#include "TLegend.h"
#include <TObjString.h>

//---- AliRoot system ----
#include "AliAnaCalorimeterQA.h"
#include "AliCaloTrackReader.h"
#include "AliStack.h"
#include "AliVCaloCells.h"
#include "AliFiducialCut.h"
#include "AliAODTrack.h"
#include "AliVCluster.h"
#include "AliVEvent.h"
#include "AliVEventHandler.h"
#include "AliAnalysisManager.h"
#include "AliAODMCParticle.h"
#include "AliMCAnalysisUtils.h"
#include "AliAODPid.h"
#include "AliExternalTrackParam.h"

ClassImp(AliAnaCalorimeterQA)

//____________________________________________________________________________
AliAnaCalorimeterQA::AliAnaCalorimeterQA() : 
AliAnaPartCorrBaseClass(),             
fCalorimeter(""),         
fFillAllPosHisto(kFALSE),              fFillAllPosHisto2(kTRUE), 
fFillAllTH12(kFALSE),                  fFillAllTH3(kTRUE), 
fFillAllTMHisto(kTRUE),                fFillAllPi0Histo(kTRUE),                 
fCorrelate(kTRUE),                     
fNModules(12),                         fNRCU(2),
fNMaxCols(48),                         fNMaxRows(24),  
fTimeCutMin(-1),                       fTimeCutMax(9999999),
fEMCALCellAmpMin(0),                   fPHOSCellAmpMin(0), 
fhE(0),                                fhPt(0),                                
fhPhi(0),                              fhEta(0),                               fhEtaPhiE(0),
fhECharged(0),                         fhPtCharged(0),             
fhPhiCharged(0),                       fhEtaCharged(0),                        fhEtaPhiECharged(0), 

//Invariant mass
fhIM(0 ),                              fhAsym(0), 
fhNCellsPerCluster(0),                 fhNCellsPerClusterNoCut(0), 
fhNCellsPerClusterMIP(0),              fhNCellsPerClusterMIPCharged(0), 
fhNCellsvsClusterMaxCellDiffE0(0),     fhNCellsvsClusterMaxCellDiffE2(0),      
fhNCellsvsClusterMaxCellDiffE6(0),     fhNClusters(0),    

//Timing
fhClusterTimeEnergy(0),                fhCellTimeSpreadRespectToCellMax(0),  
fhClusterMaxCellDiffAverageTime(0),    fhClusterMaxCellDiffWeightTime(0),
fhClusterDiffWeightAverTime(0),
fhClusterMaxCellDiffAverageNoMaxTime(0), fhClusterMaxCellDiffWeightNoMaxTime(0),
fhClusterNoMaxCellWeight(0),
fhCellIdCellLargeTimeSpread(0),        fhClusterPairDiffTimeE(0),

fhClusterMaxCellCloseCellRatio(0),     fhClusterMaxCellCloseCellDiff(0), 
fhClusterMaxCellDiff(0),               fhClusterMaxCellDiffNoCut(0), 
fhLambda0vsClusterMaxCellDiffE0(0),    fhLambda0vsClusterMaxCellDiffE2(0),     fhLambda0vsClusterMaxCellDiffE6(0),

//bad cells
fhBadClusterEnergy(0),                 fhBadClusterTimeEnergy(0),              fhBadClusterPairDiffTimeE(0),
fhBadClusterMaxCellCloseCellRatio(0),  fhBadClusterMaxCellCloseCellDiff(0),    fhBadClusterMaxCellDiff(0),
fhBadClusterMaxCellDiffAverageTime(0), fhBadClusterMaxCellDiffWeightTime(0),    
fhBadClusterDiffWeightAverTime(0),     
fhBadClusterMaxCellDiffAverageNoMaxTime(0), fhBadClusterMaxCellDiffWeightNoMaxTime(0),
fhBadClusterNoMaxCellWeight(0),        fhBadCellTimeSpreadRespectToCellMax(0), 
fhBadClusterL0(0),                     fhBadClusterL1(0),                      fhBadClusterD(0),

//Cluster size
fhDeltaIEtaDeltaIPhiE0(),              fhDeltaIEtaDeltaIPhiE2(),               fhDeltaIEtaDeltaIPhiE6(), 
fhDeltaIA(),                           fhDeltaIAL0(),                         
fhDeltaIAL1(),                         fhDeltaIANCells(),

//Position
fhRNCells(0),                          fhXNCells(0),               
fhYNCells(0),                          fhZNCells(0),
fhRE(0),                               fhXE(0),                    
fhYE(0),                               fhZE(0),    
fhXYZ(0),
fhRCellE(0),                           fhXCellE(0),                
fhYCellE(0),                           fhZCellE(0),
fhXYZCell(0),
fhDeltaCellClusterRNCells(0),          fhDeltaCellClusterXNCells(0),
fhDeltaCellClusterYNCells(0),          fhDeltaCellClusterZNCells(0),
fhDeltaCellClusterRE(0),               fhDeltaCellClusterXE(0),     
fhDeltaCellClusterYE(0),               fhDeltaCellClusterZE(0),
// Cells
fhNCells(0),                           fhAmplitude(0),             
fhAmpId(0),                            fhEtaPhiAmp(0), 
fhTime(0),                             fhTimeId(0),                            fhTimeAmp(0), 
fhCaloCorrNClusters(0),                fhCaloCorrEClusters(0),     
fhCaloCorrNCells(0),                   fhCaloCorrECells(0),
fhCaloV0SCorrNClusters(0),             fhCaloV0SCorrEClusters(0),              
fhCaloV0SCorrNCells(0),                fhCaloV0SCorrECells(0),
fhCaloV0MCorrNClusters(0),             fhCaloV0MCorrEClusters(0),  
fhCaloV0MCorrNCells(0),                fhCaloV0MCorrECells(0),
fhCaloTrackMCorrNClusters(0),          fhCaloTrackMCorrEClusters(0), 
fhCaloTrackMCorrNCells(0),             fhCaloTrackMCorrECells(0),
//Super-Module dependent histgrams
fhEMod(0),                 fhNClustersMod(0),          
fhNCellsPerClusterMod(0),  fhNCellsPerClusterModNoCut(0), 
fhNCellsMod(0),  
fhGridCellsMod(0),         fhGridCellsEMod(0),         fhGridCellsTimeMod(0), 
fhTimeAmpPerRCU(0),        fhIMMod(0),              

// MC and reco
fhRecoMCE(),               fhRecoMCPhi(),              fhRecoMCEta(), 
fhRecoMCDeltaE(),          fhRecoMCDeltaPhi(),         fhRecoMCDeltaEta(),               

// MC only
fhGenMCE(),                fhGenMCEtaPhi(),   
fhGenMCAccE(),             fhGenMCAccEtaPhi(),   

//matched MC
fhEMVxyz(0),               fhEMR(0),                   fhHaVxyz(0),             fhHaR(0)
//fh1pOverE(0),              fh1dR(0),                   fh2EledEdx(0),           fh2MatchdEdx(0),
//fhMCEle1pOverE(0),         fhMCEle1dR(0),              fhMCEle2MatchdEdx(0),
//fhMCChHad1pOverE(0),       fhMCChHad1dR(0),            fhMCChHad2MatchdEdx(0),
//fhMCNeutral1pOverE(0),     fhMCNeutral1dR(0),          fhMCNeutral2MatchdEdx(0),fh1pOverER02(0),           
//fhMCEle1pOverER02(0),      fhMCChHad1pOverER02(0),     fhMCNeutral1pOverER02(0)
{
  //Default Ctor
  
  //Initialize parameters
  InitParameters();
}

//________________________________________________________________________
TObjString *  AliAnaCalorimeterQA::GetAnalysisCuts()
{  	
  //Save parameters used for analysis
  TString parList ; //this will be list of parameters used for this analysis.
  const Int_t buffersize = 255;
  char onePar[buffersize] ;
  
  snprintf(onePar,buffersize,"--- AliAnaCalorimeterQA ---\n") ;
  parList+=onePar ;	
  snprintf(onePar,buffersize,"Calorimeter: %s\n",fCalorimeter.Data()) ;
  parList+=onePar ;
  snprintf(onePar,buffersize,"Time Cut : %2.2f < T < %2.2f ns  \n",fTimeCutMin, fTimeCutMax) ;
  parList+=onePar ;
  snprintf(onePar,buffersize,"PHOS Cell Amplitude > %2.2f GeV, EMCAL Cell Amplitude > %2.2f GeV  \n",fPHOSCellAmpMin, fEMCALCellAmpMin) ;
  parList+=onePar ;
  //Get parameters set in base class.
  //parList += GetBaseParametersList() ;
  
  //Get parameters set in FiducialCut class (not available yet)
  //parlist += GetFidCut()->GetFidCutParametersList() 
	
  return new TObjString(parList) ;
}


//________________________________________________________________________
TList *  AliAnaCalorimeterQA::GetCreateOutputObjects()
{  
  // Create histograms to be saved in output file and 
  // store them in outputContainer
  
  TList * outputContainer = new TList() ; 
  outputContainer->SetName("QAHistos") ; 
  
  //Histograms
  Int_t nptbins     = GetHistoPtBins(); 	        Float_t ptmax     = GetHistoPtMax();           Float_t ptmin     = GetHistoPtMin();
  Int_t nfineptbins = GetHistoFinePtBins(); 	    Float_t ptfinemax = GetHistoFinePtMax();       Float_t ptfinemin = GetHistoFinePtMin();
  Int_t nphibins    = GetHistoPhiBins();     	    Float_t phimax    = GetHistoPhiMax();          Float_t phimin    = GetHistoPhiMin();
  Int_t netabins    = GetHistoEtaBins();          Float_t etamax    = GetHistoEtaMax();          Float_t etamin    = GetHistoEtaMin();	
  Int_t nmassbins   = GetHistoMassBins();         Float_t massmax   = GetHistoMassMax(); 	       Float_t massmin   = GetHistoMassMin();
  Int_t nasymbins   = GetHistoAsymmetryBins();    Float_t asymmax   = GetHistoAsymmetryMax();    Float_t asymmin   = GetHistoAsymmetryMin();
  //Int_t nPoverEbins = GetHistoPOverEBins();       Float_t pOverEmax = GetHistoPOverEMax();       Float_t pOverEmin = GetHistoPOverEMin();
  //Int_t ndedxbins   = GetHistodEdxBins();         Float_t dedxmax   = GetHistodEdxMax();         Float_t dedxmin   = GetHistodEdxMin();
  //Int_t ndRbins     = GetHistodRBins();           Float_t dRmax     = GetHistodRMax();           Float_t dRmin     = GetHistodRMin();
  Int_t ntimebins   = GetHistoTimeBins();         Float_t timemax   = GetHistoTimeMax();         Float_t timemin   = GetHistoTimeMin();       
  Int_t nclbins     = GetHistoNClustersBins();    Int_t   nclmax    = GetHistoNClustersMax();    Int_t   nclmin    = GetHistoNClustersMin(); 
  Int_t ncebins     = GetHistoNCellsBins();       Int_t   ncemax    = GetHistoNCellsMax();       Int_t   ncemin    = GetHistoNCellsMin(); 
  Int_t nceclbins   = GetHistoNClusterCellBins(); Int_t   nceclmax  = GetHistoNClusterCellMax(); Int_t   nceclmin  = GetHistoNClusterCellMin(); 
  Int_t nvdistbins  = GetHistoVertexDistBins();   Float_t vdistmax  = GetHistoVertexDistMax();   Float_t vdistmin  = GetHistoVertexDistMin();
  Int_t rbins       = GetHistoRBins();            Float_t rmax      = GetHistoRMax();            Float_t rmin      = GetHistoRMin(); 
  Int_t xbins       = GetHistoXBins();            Float_t xmax      = GetHistoXMax();            Float_t xmin      = GetHistoXMin(); 
  Int_t ybins       = GetHistoYBins();            Float_t ymax      = GetHistoYMax();            Float_t ymin      = GetHistoYMin(); 
  Int_t zbins       = GetHistoZBins();            Float_t zmax      = GetHistoZMax();            Float_t zmin      = GetHistoZMin(); 
  Int_t ssbins      = GetHistoShowerShapeBins();  Float_t ssmax     = GetHistoShowerShapeMax();  Float_t ssmin     = GetHistoShowerShapeMin();
  Int_t tdbins      = GetHistoDiffTimeBins() ;    Float_t tdmax     = GetHistoDiffTimeMax();     Float_t tdmin     = GetHistoDiffTimeMin();

  Int_t nv0sbins    = GetHistoV0SignalBins();          Int_t nv0smax = GetHistoV0SignalMax();          Int_t nv0smin = GetHistoV0SignalMin(); 
  Int_t nv0mbins    = GetHistoV0MultiplicityBins();    Int_t nv0mmax = GetHistoV0MultiplicityMax();    Int_t nv0mmin = GetHistoV0MultiplicityMin(); 
  Int_t ntrmbins    = GetHistoTrackMultiplicityBins(); Int_t ntrmmax = GetHistoTrackMultiplicityMax(); Int_t ntrmmin = GetHistoTrackMultiplicityMin(); 
  
  //EMCAL
  fNMaxCols = 48;
  fNMaxRows = 24;
  fNRCU     = 2 ;
  //PHOS
  if(fCalorimeter=="PHOS"){
    fNMaxCols = 56;
    fNMaxRows = 64;
    fNRCU     = 4 ;
  }
  
  fhE  = new TH1F ("hE","E reconstructed clusters ", nptbins*5,ptmin,ptmax*5);  
  fhE->SetXTitle("E (GeV)");
  outputContainer->Add(fhE);
  
  if(fFillAllTH12){
    fhPt  = new TH1F ("hPt","p_{T} reconstructed clusters", nptbins,ptmin,ptmax); 
    fhPt->SetXTitle("p_{T} (GeV/c)");
    outputContainer->Add(fhPt);
    
    fhPhi  = new TH1F ("hPhi","#phi reconstructed clusters ",nphibins,phimin,phimax); 
    fhPhi->SetXTitle("#phi (rad)");
    outputContainer->Add(fhPhi);
    
    fhEta  = new TH1F ("hEta","#eta reconstructed clusters ",netabins,etamin,etamax); 
    fhEta->SetXTitle("#eta ");
    outputContainer->Add(fhEta);
  }
  
  fhEtaPhiE  = new TH3F ("hEtaPhiE","#eta vs #phi vs energy, reconstructed clusters",
                         netabins,etamin,etamax,nphibins,phimin,phimax,nptbins,ptmin,ptmax); 
  fhEtaPhiE->SetXTitle("#eta ");
  fhEtaPhiE->SetYTitle("#phi (rad)");
  fhEtaPhiE->SetZTitle("E (GeV) ");
  outputContainer->Add(fhEtaPhiE);
  
  fhClusterTimeEnergy  = new TH2F ("hClusterTimeEnergy","energy vs TOF, reconstructed clusters",
                                   nptbins,ptmin,ptmax, ntimebins,timemin,timemax); 
  fhClusterTimeEnergy->SetXTitle("E (GeV) ");
  fhClusterTimeEnergy->SetYTitle("TOF (ns)");
  outputContainer->Add(fhClusterTimeEnergy);
    
  fhClusterPairDiffTimeE = new TH2F("hClusterPairDiffTimeE","cluster pair time difference vs E, only good clusters",
                                    nptbins,ptmin,ptmax, tdbins,tdmin,tdmax);
  fhClusterPairDiffTimeE->SetXTitle("E_{cluster} (GeV)");
  fhClusterPairDiffTimeE->SetYTitle("#Delta t (ns)");
  outputContainer->Add(fhClusterPairDiffTimeE);  
  
  
  fhClusterMaxCellCloseCellRatio  = new TH2F ("hClusterMaxCellCloseCellRatio","energy vs ratio of max cell / neighbour cell, reconstructed clusters",
                                              nptbins,ptmin,ptmax, 100,0,1.); 
  fhClusterMaxCellCloseCellRatio->SetXTitle("E_{cluster} (GeV) ");
  fhClusterMaxCellCloseCellRatio->SetYTitle("E_{cell i}/E_{cell max}");
  outputContainer->Add(fhClusterMaxCellCloseCellRatio);
  
  fhClusterMaxCellCloseCellDiff  = new TH2F ("hClusterMaxCellCloseCellDiff","energy vs ratio of max cell / neighbour cell, reconstructed clusters",
                                              nptbins,ptmin,ptmax, 500,0,100.); 
  fhClusterMaxCellCloseCellDiff->SetXTitle("E_{cluster} (GeV) ");
  fhClusterMaxCellCloseCellDiff->SetYTitle("E_{cell max}-E_{cell i} (GeV)");
  outputContainer->Add(fhClusterMaxCellCloseCellDiff);
  
  fhClusterMaxCellDiff  = new TH2F ("hClusterMaxCellDiff","energy vs difference of cluster energy - max cell energy / cluster energy, good clusters",
                                       nptbins,ptmin,ptmax, 500,0,1.); 
  fhClusterMaxCellDiff->SetXTitle("E_{cluster} (GeV) ");
  fhClusterMaxCellDiff->SetYTitle("(E_{cluster} - E_{cell max})/ E_{cluster}");
  outputContainer->Add(fhClusterMaxCellDiff);  

  fhClusterMaxCellDiffNoCut  = new TH2F ("hClusterMaxCellDiffNoCut","energy vs difference of cluster energy - max cell energy / cluster energy",
                                    nptbins,ptmin,ptmax, 500,0,1.); 
  fhClusterMaxCellDiffNoCut->SetXTitle("E_{cluster} (GeV) ");
  fhClusterMaxCellDiffNoCut->SetYTitle("(E_{cluster} - E_{cell max})/ E_{cluster}");
  outputContainer->Add(fhClusterMaxCellDiffNoCut);  
    
  fhLambda0vsClusterMaxCellDiffE0  = new TH2F ("hLambda0vsClusterMaxCellDiffE0","shower shape, #lambda^{2}_{0} vs fraction of energy carried by max cell, E < 2 GeV ",
                                               ssbins,ssmin,ssmax,500,0,1.); 
  fhLambda0vsClusterMaxCellDiffE0->SetYTitle("(E_{cluster} - E_{cell max})/ E_{cluster}");
  fhLambda0vsClusterMaxCellDiffE0->SetXTitle("#lambda^{2}_{0}");
  outputContainer->Add(fhLambda0vsClusterMaxCellDiffE0); 
  
  fhLambda0vsClusterMaxCellDiffE2  = new TH2F ("hLambda0vsClusterMaxCellDiffE2","shower shape, #lambda^{2}_{0} vs fraction of energy carried by max cell, 2 < E < 6 GeV ",
                                               ssbins,ssmin,ssmax,500,0,1.); 
  fhLambda0vsClusterMaxCellDiffE2->SetYTitle("(E_{cluster} - E_{cell max})/ E_{cluster}");
  fhLambda0vsClusterMaxCellDiffE2->SetXTitle("#lambda^{2}_{0}");
  outputContainer->Add(fhLambda0vsClusterMaxCellDiffE2); 
  
  fhLambda0vsClusterMaxCellDiffE6  = new TH2F ("hLambda0vsClusterMaxCellDiffE6","shower shape, #lambda^{2}_{0} vs fraction of energy carried by max cell, E > 6 ",
                                               ssbins,ssmin,ssmax,500,0,1.); 
  fhLambda0vsClusterMaxCellDiffE6->SetYTitle("(E_{cluster} - E_{cell max})/ E_{cluster}");
  fhLambda0vsClusterMaxCellDiffE6->SetXTitle("#lambda^{2}_{0}");
  outputContainer->Add(fhLambda0vsClusterMaxCellDiffE6); 

  fhNCellsvsClusterMaxCellDiffE0  = new TH2F ("hNCellsvsClusterMaxCellDiffE0","N cells per cluster vs fraction of energy carried by max cell, E < 2 GeV ",
                                               nceclbins,nceclmin,nceclmax,500,0,1.); 
  fhNCellsvsClusterMaxCellDiffE0->SetYTitle("(E_{cluster} - E_{cell max})/ E_{cluster}");
  fhNCellsvsClusterMaxCellDiffE0->SetXTitle("N cells per cluster");
  outputContainer->Add(fhNCellsvsClusterMaxCellDiffE0); 
  
  fhNCellsvsClusterMaxCellDiffE2  = new TH2F ("hNCellsvsClusterMaxCellDiffE2","N cells per cluster vs fraction of energy carried by max cell, 2 < E < 6 GeV ",
                                               nceclbins,nceclmin,nceclmax,500,0,1.); 
  fhNCellsvsClusterMaxCellDiffE2->SetYTitle("(E_{cluster} - E_{cell max})/ E_{cluster}");
  fhNCellsvsClusterMaxCellDiffE2->SetXTitle("N cells per cluster");
  outputContainer->Add(fhNCellsvsClusterMaxCellDiffE2); 
  
  fhNCellsvsClusterMaxCellDiffE6  = new TH2F ("hNCellsvsClusterMaxCellDiffE6","N cells per cluster vs fraction of energy carried by max cell, E > 6 ",
                                               nceclbins,nceclmin,nceclmax,500,0,1.); 
  fhNCellsvsClusterMaxCellDiffE6->SetYTitle("(E_{cluster} - E_{cell max})/ E_{cluster}");
  fhNCellsvsClusterMaxCellDiffE6->SetXTitle("N cells per cluster");
  outputContainer->Add(fhNCellsvsClusterMaxCellDiffE6); 
  
  
  if(fCalorimeter=="EMCAL" && !GetCaloUtils()->GetEMCALRecoUtils()->IsRejectExoticCluster()){
    
    fhBadClusterEnergy  = new TH1F ("hBadClusterEnergy","Bad cluster energy", nptbins,ptmin,ptmax); 
    fhBadClusterEnergy->SetXTitle("E_{cluster} (GeV) ");
    outputContainer->Add(fhBadClusterEnergy);
    
    fhBadClusterMaxCellCloseCellRatio  = new TH2F ("hBadClusterMaxCellCloseCellRatio","energy vs ratio of max cell / neighbour cell constributing cell, reconstructed bad clusters",
                                                   nptbins,ptmin,ptmax, 100,0,1.); 
    fhBadClusterMaxCellCloseCellRatio->SetXTitle("E_{cluster} (GeV) ");
    fhBadClusterMaxCellCloseCellRatio->SetYTitle("ratio");
    outputContainer->Add(fhBadClusterMaxCellCloseCellRatio);
        
    fhBadClusterMaxCellCloseCellDiff  = new TH2F ("hBadClusterMaxCellCloseCellDiff","energy vs ratio of max cell - neighbour cell constributing cell, reconstructed bad clusters",
                                                   nptbins,ptmin,ptmax, 500,0,100); 
    fhBadClusterMaxCellCloseCellDiff->SetXTitle("E_{cluster} (GeV) ");
    fhBadClusterMaxCellCloseCellDiff->SetYTitle("E_{cell max} - E_{cell i} (GeV)");
    outputContainer->Add(fhBadClusterMaxCellCloseCellDiff);    
    
    fhBadClusterMaxCellDiff  = new TH2F ("hBadClusterMaxCellDiff","energy vs difference of cluster energy - max cell energy / cluster energy for bad clusters",
                                                   nptbins,ptmin,ptmax, 500,0,1.); 
    fhBadClusterMaxCellDiff->SetXTitle("E_{cluster} (GeV) ");
    fhBadClusterMaxCellDiff->SetYTitle("(E_{cluster} - E_{cell max}) / E_{cluster}");
    outputContainer->Add(fhBadClusterMaxCellDiff);
    
    fhBadClusterTimeEnergy  = new TH2F ("hBadClusterTimeEnergy","energy vs TOF of reconstructed bad clusters",
                                               nptbins,ptmin,ptmax, ntimebins,timemin,timemax); 
    fhBadClusterTimeEnergy->SetXTitle("E_{cluster} (GeV) ");
    fhBadClusterTimeEnergy->SetYTitle("TOF (ns)");
    outputContainer->Add(fhBadClusterTimeEnergy);    

    fhBadClusterPairDiffTimeE = new TH2F("hBadClusterPairDiffTimeE","cluster pair time difference (bad - good) vs E from bad cluster",nptbins,ptmin,ptmax, tdbins,tdmin,tdmax);
    fhBadClusterPairDiffTimeE->SetXTitle("E_{bad cluster} (GeV)");
    fhBadClusterPairDiffTimeE->SetYTitle("#Delta t (ns)");
    outputContainer->Add(fhBadClusterPairDiffTimeE);    
    
    fhBadClusterL0  = new TH2F ("hBadClusterL0","shower shape, #lambda^{2}_{0} vs E for bad cluster ",
                                                 nptbins,ptmin,ptmax,ssbins,ssmin,ssmax); 
    fhBadClusterL0->SetXTitle("E_{cluster}");
    fhBadClusterL0->SetYTitle("#lambda^{2}_{0}");
    outputContainer->Add(fhBadClusterL0); 
    
    fhBadClusterL1  = new TH2F ("hBadClusterL1","shower shape, #lambda^{2}_{1} vs E for bad cluster ",
                                     nptbins,ptmin,ptmax,ssbins,ssmin,ssmax); 
    fhBadClusterL1->SetXTitle("E_{cluster}");
    fhBadClusterL1->SetYTitle("#lambda^{2}_{1}");
    outputContainer->Add(fhBadClusterL1); 
    
    fhBadClusterD  = new TH2F ("hBadClusterD","shower shape, Dispersion^{2} vs E for bad cluster ",
                                     nptbins,ptmin,ptmax,ssbins,ssmin,ssmax); 
    fhBadClusterD->SetXTitle("E_{cluster}");
    fhBadClusterD->SetYTitle("Dispersion");
    outputContainer->Add(fhBadClusterD);     
    
    if(GetReader()->GetDataType()==AliCaloTrackReader::kESD) {
      fhBadCellTimeSpreadRespectToCellMax = new TH2F ("hBadCellTimeSpreadRespectToCellMax","t_{cell max}-t_{cell i} per cluster", nptbins,ptmin,ptmax, 250,-500,500); 
      fhBadCellTimeSpreadRespectToCellMax->SetXTitle("E (GeV)");
      fhBadCellTimeSpreadRespectToCellMax->SetYTitle("#Delta t_{cell max - i} (ns)");
      outputContainer->Add(fhBadCellTimeSpreadRespectToCellMax);
      
      fhBadClusterMaxCellDiffAverageTime = new TH2F ("hBadClusterMaxCellDiffAverageTime","t_{cell max}-t_{average} per cluster", nptbins,ptmin,ptmax, 250,-500,500); 
      fhBadClusterMaxCellDiffAverageTime->SetXTitle("E (GeV)");
      fhBadClusterMaxCellDiffAverageTime->SetYTitle("#Delta t_{cell max - average} (ns)");
      outputContainer->Add(fhBadClusterMaxCellDiffAverageTime);
      
      fhBadClusterMaxCellDiffWeightTime = new TH2F ("hBadClusterMaxCellDiffWeightTime","t_{cell max weighted}-t_{average weighted} per cluster", nptbins,ptmin,ptmax, 250,-500,500); 
      fhBadClusterMaxCellDiffWeightTime->SetXTitle("E (GeV)");
      fhBadClusterMaxCellDiffWeightTime->SetYTitle("#Delta t_{cell max - average weighted} (ns)");
      outputContainer->Add(fhBadClusterMaxCellDiffWeightTime);
      
      fhBadClusterDiffWeightAverTime = new TH2F ("hBadClusterDiffWeightAverTime","Average Time in cluster - Weighted time in cluster", nptbins,ptmin,ptmax, 250,-500,500); 
      fhBadClusterDiffWeightAverTime->SetXTitle("E (GeV)");
      fhBadClusterDiffWeightAverTime->SetYTitle("#bar{t}-wt");
      outputContainer->Add(fhBadClusterDiffWeightAverTime);
      
      fhBadClusterMaxCellDiffAverageNoMaxTime = new TH2F ("hBadClusterMaxCellDiffAverageNoMaxTime","t_{cell max}-t_{average} per cluster", nptbins,ptmin,ptmax, 250,-500,500); 
      fhBadClusterMaxCellDiffAverageNoMaxTime->SetXTitle("E (GeV)");
      fhBadClusterMaxCellDiffAverageNoMaxTime->SetYTitle("#Delta t_{cell max - average} (ns)");
      outputContainer->Add(fhBadClusterMaxCellDiffAverageNoMaxTime);
      
      fhBadClusterMaxCellDiffWeightNoMaxTime = new TH2F ("hBadClusterMaxCellDiffWeightNoMaxTime","t_{cell max weighted}-t_{average weighted} per cluster", nptbins,ptmin,ptmax, 250,-500,500); 
      fhBadClusterMaxCellDiffWeightNoMaxTime->SetXTitle("E (GeV)");
      fhBadClusterMaxCellDiffWeightNoMaxTime->SetYTitle("#Delta t_{cell max - average weighted} (ns)");
      outputContainer->Add(fhBadClusterMaxCellDiffWeightNoMaxTime);      
      
    }    
    
    fhBadClusterNoMaxCellWeight = new TH2F ("hBadClusterNoMaxCellWeight"," #S weight_{no max} / #S weight_{tot} per cluster", nptbins,ptmin,ptmax, 100,0,1); 
    fhBadClusterNoMaxCellWeight->SetXTitle("E (GeV)");
    fhBadClusterNoMaxCellWeight->SetYTitle("#S weight_{no max} / #S weight_{tot}");
    outputContainer->Add(fhBadClusterNoMaxCellWeight);     
    
  }
  
  // Cluster size in terms of cells
  
  fhDeltaIEtaDeltaIPhiE0[0]  = new TH2F ("hDeltaIEtaDeltaIPhiE0"," Cluster size in columns vs rows for E < 2 GeV, n cells > 3",
                         50,0,50,50,0,50); 
  fhDeltaIEtaDeltaIPhiE0[0]->SetXTitle("#Delta Column");
  fhDeltaIEtaDeltaIPhiE0[0]->SetYTitle("#Delta Row");
  outputContainer->Add(fhDeltaIEtaDeltaIPhiE0[0]); 
  
  fhDeltaIEtaDeltaIPhiE2[0]  = new TH2F ("hDeltaIEtaDeltaIPhiE2"," Cluster size in columns vs rows for 2 <E < 6 GeV, n cells > 3",
                                      50,0,50,50,0,50); 
  fhDeltaIEtaDeltaIPhiE2[0]->SetXTitle("#Delta Column");
  fhDeltaIEtaDeltaIPhiE2[0]->SetYTitle("#Delta Row");
  outputContainer->Add(fhDeltaIEtaDeltaIPhiE2[0]); 
  
  fhDeltaIEtaDeltaIPhiE6[0]  = new TH2F ("hDeltaIEtaDeltaIPhiE6"," Cluster size in columns vs rows for E > 6 GeV, n cells > 3",
                                      50,0,50,50,0,50); 
  fhDeltaIEtaDeltaIPhiE6[0]->SetXTitle("#Delta Column");
  fhDeltaIEtaDeltaIPhiE6[0]->SetYTitle("#Delta Row");
  outputContainer->Add(fhDeltaIEtaDeltaIPhiE6[0]); 
  
  fhDeltaIA[0]  = new TH2F ("hDeltaIA"," Cluster *asymmetry* in cell units vs E",
                         nptbins,ptmin,ptmax,21,-1.05,1.05); 
  fhDeltaIA[0]->SetXTitle("E_{cluster}");
  fhDeltaIA[0]->SetYTitle("A_{cell in cluster}");
  outputContainer->Add(fhDeltaIA[0]); 
    
  fhDeltaIAL0[0]  = new TH2F ("hDeltaIAL0"," Cluster *asymmetry* in cell units vs #lambda^{2}_{0}",
                         ssbins,ssmin,ssmax,21,-1.05,1.05); 
  fhDeltaIAL0[0]->SetXTitle("#lambda^{2}_{0}");
  fhDeltaIAL0[0]->SetYTitle("A_{cell in cluster}");
  outputContainer->Add(fhDeltaIAL0[0]); 
    
  fhDeltaIAL1[0]  = new TH2F ("hDeltaIAL1"," Cluster *asymmetry* in cell units vs #lambda^{2}_{1}",
                           ssbins,ssmin,ssmax,21,-1.05,1.05); 
  fhDeltaIAL1[0]->SetXTitle("#lambda^{2}_{1}");
  fhDeltaIAL1[0]->SetYTitle("A_{cell in cluster}");
  outputContainer->Add(fhDeltaIAL1[0]); 
  
  fhDeltaIANCells[0]  = new TH2F ("hDeltaIANCells"," Cluster *asymmetry* in cell units vs N cells in cluster",
                               nceclbins,nceclmin,nceclmax,21,-1.05,1.05); 
  fhDeltaIANCells[0]->SetXTitle("N_{cell in cluster}");
  fhDeltaIANCells[0]->SetYTitle("A_{cell in cluster}");
  outputContainer->Add(fhDeltaIANCells[0]); 
  
  
  fhDeltaIEtaDeltaIPhiE0[1]  = new TH2F ("hDeltaIEtaDeltaIPhiE0Charged"," Cluster size in columns vs rows for E < 2 GeV, n cells > 3, matched with track",
                                         50,0,50,50,0,50); 
  fhDeltaIEtaDeltaIPhiE0[1]->SetXTitle("#Delta Column");
  fhDeltaIEtaDeltaIPhiE0[1]->SetYTitle("#Delta Row");
  outputContainer->Add(fhDeltaIEtaDeltaIPhiE0[1]); 
  
  fhDeltaIEtaDeltaIPhiE2[1]  = new TH2F ("hDeltaIEtaDeltaIPhiE2Charged"," Cluster size in columns vs rows for 2 <E < 6 GeV, n cells > 3, matched with track",
                                         50,0,50,50,0,50); 
  fhDeltaIEtaDeltaIPhiE2[1]->SetXTitle("#Delta Column");
  fhDeltaIEtaDeltaIPhiE2[1]->SetYTitle("#Delta Row");
  outputContainer->Add(fhDeltaIEtaDeltaIPhiE2[1]); 
  
  fhDeltaIEtaDeltaIPhiE6[1]  = new TH2F ("hDeltaIEtaDeltaIPhiE6Charged"," Cluster size in columns vs rows for E > 6 GeV, n cells > 3, matched with track",
                                         50,0,50,50,0,50); 
  fhDeltaIEtaDeltaIPhiE6[1]->SetXTitle("#Delta Column");
  fhDeltaIEtaDeltaIPhiE6[1]->SetYTitle("#Delta Row");
  outputContainer->Add(fhDeltaIEtaDeltaIPhiE6[1]); 
  
  fhDeltaIA[1]  = new TH2F ("hDeltaIACharged"," Cluster *asymmetry* in cell units vs E, matched with track",
                            nptbins,ptmin,ptmax,21,-1.05,1.05); 
  fhDeltaIA[1]->SetXTitle("E_{cluster}");
  fhDeltaIA[1]->SetYTitle("A_{cell in cluster}");
  outputContainer->Add(fhDeltaIA[1]); 
  
  fhDeltaIAL0[1]  = new TH2F ("hDeltaIAL0Charged"," Cluster *asymmetry* in cell units vs #lambda^{2}_{0}, matched with track",
                              ssbins,ssmin,ssmax,21,-1.05,1.05); 
  fhDeltaIAL0[1]->SetXTitle("#lambda^{2}_{0}");
  fhDeltaIAL0[1]->SetYTitle("A_{cell in cluster}");
  outputContainer->Add(fhDeltaIAL0[1]); 
  
  fhDeltaIAL1[1]  = new TH2F ("hDeltaIAL1Charged"," Cluster *asymmetry* in cell units vs #lambda^{2}_{1}, matched with track",
                              ssbins,ssmin,ssmax,21,-1.05,1.05); 
  fhDeltaIAL1[1]->SetXTitle("#lambda^{2}_{1}");
  fhDeltaIAL1[1]->SetYTitle("A_{cell in cluster}");
  outputContainer->Add(fhDeltaIAL1[1]); 
  
  fhDeltaIANCells[1]  = new TH2F ("hDeltaIANCellsCharged"," Cluster *asymmetry* in cell units vs N cells in cluster, matched with track",
                                  nceclbins,nceclmin,nceclmax,21,-1.05,1.05); 
  fhDeltaIANCells[1]->SetXTitle("N_{cell in cluster}");
  fhDeltaIANCells[1]->SetYTitle("A_{cell in cluster}");
  outputContainer->Add(fhDeltaIANCells[1]); 
  
  if(IsDataMC()){
    TString particle[]={"Photon","Electron","Conversion","Hadron"};
    for (Int_t iPart = 0; iPart < 4; iPart++) {
      
      fhDeltaIAMC[iPart]  = new TH2F (Form("hDeltaIA_MC%s",particle[iPart].Data()),Form(" Cluster *asymmetry* in cell units vs E, from %s",particle[iPart].Data()),
                                    nptbins,ptmin,ptmax,21,-1.05,1.05); 
      fhDeltaIAMC[iPart]->SetXTitle("E_{cluster}");
      fhDeltaIAMC[iPart]->SetYTitle("A_{cell in cluster}");
      outputContainer->Add(fhDeltaIAMC[iPart]);     
    }
  }
  
  //Track Matching
  if(fFillAllTMHisto){
    if(fFillAllTH12){
      fhECharged  = new TH1F ("hECharged","E reconstructed clusters, matched with track", nptbins,ptmin,ptmax); 
      fhECharged->SetXTitle("E (GeV)");
      outputContainer->Add(fhECharged);
      
      fhPtCharged  = new TH1F ("hPtCharged","p_{T} reconstructed clusters, matched with track", nptbins,ptmin,ptmax); 
      fhPtCharged->SetXTitle("p_{T} (GeV/c)");
      outputContainer->Add(fhPtCharged);
      
      fhPhiCharged  = new TH1F ("hPhiCharged","#phi reconstructed clusters, matched with track",nphibins,phimin,phimax); 
      fhPhiCharged->SetXTitle("#phi (rad)");
      outputContainer->Add(fhPhiCharged);
      
      fhEtaCharged  = new TH1F ("hEtaCharged","#eta reconstructed clusters, matched with track",netabins,etamin,etamax); 
      fhEtaCharged->SetXTitle("#eta ");
      outputContainer->Add(fhEtaCharged);
    }
    if(fFillAllTH3){
      fhEtaPhiECharged  = new TH3F ("hEtaPhiECharged","#eta vs #phi, reconstructed clusters, matched with track",
                                    netabins,etamin,etamax,nphibins,phimin,phimax,nptbins,ptmin,ptmax); 
      fhEtaPhiECharged->SetXTitle("#eta ");
      fhEtaPhiECharged->SetYTitle("#phi ");
      fhEtaPhiECharged->SetZTitle("E (GeV) ");
      outputContainer->Add(fhEtaPhiECharged);	
    }
    
//    fh1pOverE = new TH2F("h1pOverE","TRACK matches p/E",nptbins,ptmin,ptmax, nPoverEbins,pOverEmin,pOverEmax);
//    fh1pOverE->SetYTitle("p/E");
//    fh1pOverE->SetXTitle("p_{T} (GeV/c)");
//    outputContainer->Add(fh1pOverE);
//    
//    fh1dR = new TH1F("h1dR","TRACK matches dR",ndRbins,dRmin,dRmax);
//    fh1dR->SetXTitle("#Delta R (rad)");
//    outputContainer->Add(fh1dR) ;
//    
//    fh2MatchdEdx = new TH2F("h2MatchdEdx","dE/dx vs. p for all matches",nptbins,ptmin,ptmax,ndedxbins,dedxmin,dedxmax);
//    fh2MatchdEdx->SetXTitle("p (GeV/c)");
//    fh2MatchdEdx->SetYTitle("<dE/dx>");
//    outputContainer->Add(fh2MatchdEdx);
//    
//    fh2EledEdx = new TH2F("h2EledEdx","dE/dx vs. p for electrons",nptbins,ptmin,ptmax,ndedxbins,dedxmin,dedxmax);
//    fh2EledEdx->SetXTitle("p (GeV/c)");
//    fh2EledEdx->SetYTitle("<dE/dx>");
//    outputContainer->Add(fh2EledEdx) ;
//    
//    fh1pOverER02 = new TH2F("h1pOverER02","TRACK matches p/E, all",nptbins,ptmin,ptmax, nPoverEbins,pOverEmin,pOverEmax);
//    fh1pOverER02->SetYTitle("p/E");
//    fh1pOverER02->SetXTitle("p_{T} (GeV/c)");
//    outputContainer->Add(fh1pOverER02);	
  }
  
  if(fFillAllPi0Histo){
    fhIM  = new TH2F ("hIM","Cluster pairs Invariant mass vs reconstructed pair energy, ncell > 1",nptbins,ptmin,ptmax,nmassbins,massmin,massmax); 
    fhIM->SetXTitle("p_{T, cluster pairs} (GeV) ");
    fhIM->SetYTitle("M_{cluster pairs} (GeV/c^{2})");
    outputContainer->Add(fhIM);
    
    fhAsym  = new TH2F ("hAssym","Cluster pairs Asymmetry vs reconstructed pair energy",nptbins,ptmin,ptmax,nasymbins,asymmin,asymmax); 
    fhAsym->SetXTitle("p_{T, cluster pairs} (GeV) ");
    fhAsym->SetYTitle("Asymmetry");
    outputContainer->Add(fhAsym);	
    
  }

  fhNCellsPerClusterNoCut  = new TH2F ("hNCellsPerClusterNoCut","# cells per cluster vs energy vs #eta, no bad clusters cut",
                                       nptbins,ptmin,ptmax, nceclbins,nceclmin,nceclmax); 
  fhNCellsPerClusterNoCut->SetXTitle("E (GeV)");
  fhNCellsPerClusterNoCut->SetYTitle("n cells");
  outputContainer->Add(fhNCellsPerClusterNoCut);
    
  fhNCellsPerCluster  = new TH2F ("hNCellsPerCluster","# cells per cluster vs energy vs #eta",nptbins,ptmin,ptmax, nceclbins,nceclmin,nceclmax); 
  fhNCellsPerCluster->SetXTitle("E (GeV)");
  fhNCellsPerCluster->SetYTitle("n cells");
  outputContainer->Add(fhNCellsPerCluster);
    
  if((fCalorimeter=="EMCAL" && GetReader()->GetEMCALPtMin() < 0.3) ||
     (fCalorimeter=="PHOS"  && GetReader()->GetPHOSPtMin()  < 0.3)) {
    fhNCellsPerClusterMIP  = new TH2F ("hNCellsPerClusterMIP","# cells per cluster vs energy vs #eta, smaller bin for MIP search", 
                                       40,0.,2., 11,0,10); 
    fhNCellsPerClusterMIP->SetXTitle("E (GeV)");
    fhNCellsPerClusterMIP->SetYTitle("n cells");
    outputContainer->Add(fhNCellsPerClusterMIP);
    
    
    if(fFillAllTMHisto){
      fhNCellsPerClusterMIPCharged  = new TH2F ("hNCellsPerClusterMIPCharged","# cells per track-matched cluster vs energy vs #eta, smaller bin for MIP search", 
                                                40,0.,2., 11,0,10); 
      fhNCellsPerClusterMIPCharged->SetXTitle("E (GeV)");
      fhNCellsPerClusterMIPCharged->SetYTitle("n cells");
      outputContainer->Add(fhNCellsPerClusterMIPCharged);
    }
	}
  
  fhNClusters  = new TH1F ("hNClusters","# clusters", nclbins,nclmin,nclmax); 
  fhNClusters->SetXTitle("number of clusters");
  outputContainer->Add(fhNClusters);
  
  if(fFillAllPosHisto2){
    
    if(fFillAllTH3){
      fhXYZ  = new TH3F ("hXYZ","Cluster: x vs y vs z",xbins,xmin,xmax,ybins,ymin,ymax,zbins,zmin,zmax); 
      fhXYZ->SetXTitle("x (cm)");
      fhXYZ->SetYTitle("y (cm)");
      fhXYZ->SetZTitle("z (cm) ");
      outputContainer->Add(fhXYZ);  
    }
    
    fhXNCells  = new TH2F ("hXNCells","Cluster X position vs N Cells per Cluster",xbins,xmin,xmax,nceclbins,nceclmin,nceclmax); 
    fhXNCells->SetXTitle("x (cm)");
    fhXNCells->SetYTitle("N cells per cluster");
    outputContainer->Add(fhXNCells);
    
    fhZNCells  = new TH2F ("hZNCells","Cluster Z position vs N Cells per Cluster",zbins,zmin,zmax,nceclbins,nceclmin,nceclmax); 
    fhZNCells->SetXTitle("z (cm)");
    fhZNCells->SetYTitle("N cells per cluster");
    outputContainer->Add(fhZNCells);
    
    fhXE  = new TH2F ("hXE","Cluster X position vs cluster energy",xbins,xmin,xmax,nptbins,ptmin,ptmax); 
    fhXE->SetXTitle("x (cm)");
    fhXE->SetYTitle("E (GeV)");
    outputContainer->Add(fhXE);
    
    fhZE  = new TH2F ("hZE","Cluster Z position vs cluster energy",zbins,zmin,zmax,nptbins,ptmin,ptmax); 
    fhZE->SetXTitle("z (cm)");
    fhZE->SetYTitle("E (GeV)");
    outputContainer->Add(fhZE);    
    
    fhRNCells  = new TH2F ("hRNCells","Cluster R position vs N Cells per Cluster",rbins,rmin,rmax,nceclbins,nceclmin,nceclmax); 
    fhRNCells->SetXTitle("r = #sqrt{x^{2}+y^{2}} (cm)");
    fhRNCells->SetYTitle("N cells per cluster");
    outputContainer->Add(fhRNCells);
    
    
    fhYNCells  = new TH2F ("hYNCells","Cluster Y position vs N Cells per Cluster",ybins,ymin,ymax,nceclbins,nceclmin,nceclmax); 
    fhYNCells->SetXTitle("y (cm)");
    fhYNCells->SetYTitle("N cells per cluster");
    outputContainer->Add(fhYNCells);
    
    fhRE  = new TH2F ("hRE","Cluster R position vs cluster energy",rbins,rmin,rmax,nptbins,ptmin,ptmax); 
    fhRE->SetXTitle("r = #sqrt{x^{2}+y^{2}} (cm)");
    fhRE->SetYTitle("E (GeV)");
    outputContainer->Add(fhRE);
    
    fhYE  = new TH2F ("hYE","Cluster Y position vs cluster energy",ybins,ymin,ymax,nptbins,ptmin,ptmax); 
    fhYE->SetXTitle("y (cm)");
    fhYE->SetYTitle("E (GeV)");
    outputContainer->Add(fhYE);
  }
  if(fFillAllPosHisto){
    
    fhRCellE  = new TH2F ("hRCellE","Cell R position vs cell energy",rbins,rmin,rmax,nptbins,ptmin,ptmax); 
    fhRCellE->SetXTitle("r = #sqrt{x^{2}+y^{2}} (cm)");
    fhRCellE->SetYTitle("E (GeV)");
    outputContainer->Add(fhRCellE);
    
    fhXCellE  = new TH2F ("hXCellE","Cell X position vs cell energy",xbins,xmin,xmax,nptbins,ptmin,ptmax); 
    fhXCellE->SetXTitle("x (cm)");
    fhXCellE->SetYTitle("E (GeV)");
    outputContainer->Add(fhXCellE);
    
    fhYCellE  = new TH2F ("hYCellE","Cell Y position vs cell energy",ybins,ymin,ymax,nptbins,ptmin,ptmax); 
    fhYCellE->SetXTitle("y (cm)");
    fhYCellE->SetYTitle("E (GeV)");
    outputContainer->Add(fhYCellE);
    
    fhZCellE  = new TH2F ("hZCellE","Cell Z position vs cell energy",zbins,zmin,zmax,nptbins,ptmin,ptmax); 
    fhZCellE->SetXTitle("z (cm)");
    fhZCellE->SetYTitle("E (GeV)");
    outputContainer->Add(fhZCellE);
    
    fhXYZCell  = new TH3F ("hXYZCell","Cell : x vs y vs z",xbins,xmin,xmax,ybins,ymin,ymax,zbins,zmin,zmax); 
    fhXYZCell->SetXTitle("x (cm)");
    fhXYZCell->SetYTitle("y (cm)");
    fhXYZCell->SetZTitle("z (cm)");
    outputContainer->Add(fhXYZCell);
    
    
    Float_t dx = TMath::Abs(xmin)+TMath::Abs(xmax);
    Float_t dy = TMath::Abs(ymin)+TMath::Abs(ymax);
    Float_t dz = TMath::Abs(zmin)+TMath::Abs(zmax);
    Float_t dr = TMath::Abs(rmin)+TMath::Abs(rmax);
    
    fhDeltaCellClusterRNCells  = new TH2F ("hDeltaCellClusterRNCells","Cluster-Cell R position vs N Cells per Cluster",rbins*2,-dr,dr,nceclbins,nceclmin,nceclmax); 
    fhDeltaCellClusterRNCells->SetXTitle("r = #sqrt{x^{2}+y^{2}} (cm)");
    fhDeltaCellClusterRNCells->SetYTitle("N cells per cluster");
    outputContainer->Add(fhDeltaCellClusterRNCells);
    
    fhDeltaCellClusterXNCells  = new TH2F ("hDeltaCellClusterXNCells","Cluster-Cell X position vs N Cells per Cluster",xbins*2,-dx,dx,nceclbins,nceclmin,nceclmax); 
    fhDeltaCellClusterXNCells->SetXTitle("x (cm)");
    fhDeltaCellClusterXNCells->SetYTitle("N cells per cluster");
    outputContainer->Add(fhDeltaCellClusterXNCells);
    
    fhDeltaCellClusterYNCells  = new TH2F ("hDeltaCellClusterYNCells","Cluster-Cell Y position vs N Cells per Cluster",ybins*2,-dy,dy,nceclbins,nceclmin,nceclmax); 
    fhDeltaCellClusterYNCells->SetXTitle("y (cm)");
    fhDeltaCellClusterYNCells->SetYTitle("N cells per cluster");
    outputContainer->Add(fhDeltaCellClusterYNCells);
    
    fhDeltaCellClusterZNCells  = new TH2F ("hDeltaCellClusterZNCells","Cluster-Cell Z position vs N Cells per Cluster",zbins*2,-dz,dz,nceclbins,nceclmin,nceclmax); 
    fhDeltaCellClusterZNCells->SetXTitle("z (cm)");
    fhDeltaCellClusterZNCells->SetYTitle("N cells per cluster");
    outputContainer->Add(fhDeltaCellClusterZNCells);
    
    fhDeltaCellClusterRE  = new TH2F ("hDeltaCellClusterRE","Cluster-Cell R position vs cluster energy",rbins*2,-dr,dr,nptbins,ptmin,ptmax); 
    fhDeltaCellClusterRE->SetXTitle("r = #sqrt{x^{2}+y^{2}} (cm)");
    fhDeltaCellClusterRE->SetYTitle("E (GeV)");
    outputContainer->Add(fhDeltaCellClusterRE);		
    
    fhDeltaCellClusterXE  = new TH2F ("hDeltaCellClusterXE","Cluster-Cell X position vs cluster energy",xbins*2,-dx,dx,nptbins,ptmin,ptmax); 
    fhDeltaCellClusterXE->SetXTitle("x (cm)");
    fhDeltaCellClusterXE->SetYTitle("E (GeV)");
    outputContainer->Add(fhDeltaCellClusterXE);
    
    fhDeltaCellClusterYE  = new TH2F ("hDeltaCellClusterYE","Cluster-Cell Y position vs cluster energy",ybins*2,-dy,dy,nptbins,ptmin,ptmax); 
    fhDeltaCellClusterYE->SetXTitle("y (cm)");
    fhDeltaCellClusterYE->SetYTitle("E (GeV)");
    outputContainer->Add(fhDeltaCellClusterYE);
    
    fhDeltaCellClusterZE  = new TH2F ("hDeltaCellClusterZE","Cluster-Cell Z position vs cluster energy",zbins*2,-dz,dz,nptbins,ptmin,ptmax); 
    fhDeltaCellClusterZE->SetXTitle("z (cm)");
    fhDeltaCellClusterZE->SetYTitle("E (GeV)");
    outputContainer->Add(fhDeltaCellClusterZE);
    
    fhEtaPhiAmp  = new TH3F ("hEtaPhiAmp","Cell #eta vs cell #phi vs cell energy",netabins,etamin,etamax,nphibins,phimin,phimax,nptbins,ptmin,ptmax); 
    fhEtaPhiAmp->SetXTitle("#eta ");
    fhEtaPhiAmp->SetYTitle("#phi (rad)");
    fhEtaPhiAmp->SetZTitle("E (GeV) ");
    outputContainer->Add(fhEtaPhiAmp);		
    
  }
  
  //Calo cells
  fhNCells  = new TH1F ("hNCells","# cells", ncebins,ncemin,ncemax); 
  fhNCells->SetXTitle("n cells");
  outputContainer->Add(fhNCells);
  
  fhAmplitude  = new TH1F ("hAmplitude","Cell Energy", nptbins*2,ptmin,ptmax); 
  fhAmplitude->SetXTitle("Cell Energy (GeV)");
  outputContainer->Add(fhAmplitude);
  
  fhAmpId  = new TH2F ("hAmpId","Cell Energy", nfineptbins,ptfinemin,ptfinemax,fNMaxRows*fNMaxCols*fNModules,0,fNMaxRows*fNMaxCols*fNModules); 
  fhAmpId->SetXTitle("Cell Energy (GeV)");
  outputContainer->Add(fhAmpId);
  
  //Cell Time histograms, time only available in ESDs
  if(GetReader()->GetDataType()==AliCaloTrackReader::kESD) {
    
    fhCellTimeSpreadRespectToCellMax = new TH2F ("hCellTimeSpreadRespectToCellMax","t_{cell max}-t_{cell i} per cluster", nptbins,ptmin,ptmax, 250,-500,500); 
    fhCellTimeSpreadRespectToCellMax->SetXTitle("E (GeV)");
    fhCellTimeSpreadRespectToCellMax->SetYTitle("#Delta t_{cell max-i} (ns)");
    outputContainer->Add(fhCellTimeSpreadRespectToCellMax);
    
    fhClusterMaxCellDiffAverageTime = new TH2F ("hClusterMaxCellDiffAverageTime","t_{cell max}-t_{average} per cluster", nptbins,ptmin,ptmax, 250,-500,500); 
    fhClusterMaxCellDiffAverageTime->SetXTitle("E (GeV)");
    fhClusterMaxCellDiffAverageTime->SetYTitle("#Delta t_{cell max - average} (ns)");
    outputContainer->Add(fhClusterMaxCellDiffAverageTime);
    
    fhClusterMaxCellDiffWeightTime = new TH2F ("hClusterMaxCellDiffWeightTime","t_{cell max weighted}-t_{average weighted} per cluster", nptbins,ptmin,ptmax, 250,-500,500); 
    fhClusterMaxCellDiffWeightTime->SetXTitle("E (GeV)");
    fhClusterMaxCellDiffWeightTime->SetYTitle("#Delta t_{cell max - average weighted} (ns)");
    outputContainer->Add(fhClusterMaxCellDiffWeightTime);    
    
    fhClusterMaxCellDiffAverageNoMaxTime = new TH2F ("hClusterMaxCellDiffAverageNoMaxTime","t_{cell max}-t_{average} per cluster", nptbins,ptmin,ptmax, 250,-500,500); 
    fhClusterMaxCellDiffAverageNoMaxTime->SetXTitle("E (GeV)");
    fhClusterMaxCellDiffAverageNoMaxTime->SetYTitle("#Delta t_{cell max - average} (ns)");
    outputContainer->Add(fhClusterMaxCellDiffAverageNoMaxTime);
    
    fhClusterMaxCellDiffWeightNoMaxTime = new TH2F ("hClusterMaxCellDiffWeightNoMaxTime","t_{cell max weighted}-t_{average weighted} per cluster", nptbins,ptmin,ptmax, 250,-500,500); 
    fhClusterMaxCellDiffWeightNoMaxTime->SetXTitle("E (GeV)");
    fhClusterMaxCellDiffWeightNoMaxTime->SetYTitle("#Delta t_{cell max - average weighted} (ns)");
    outputContainer->Add(fhClusterMaxCellDiffWeightNoMaxTime);          
    
    fhClusterDiffWeightAverTime = new TH2F ("hClusterDiffWeightAverTime","Average Time in cluster - Weighted time in cluster", nptbins,ptmin,ptmax, 250,-500,500); 
    fhClusterDiffWeightAverTime->SetXTitle("E (GeV)");
    fhClusterDiffWeightAverTime->SetYTitle("#bar{t} - wt");
    outputContainer->Add(fhClusterDiffWeightAverTime);
    
    fhCellIdCellLargeTimeSpread= new TH1F ("hCellIdCellLargeTimeSpread","Cells with time 100 ns larger than cell max in cluster ", 
                                           fNMaxCols*fNMaxRows*fNModules,0,fNMaxCols*fNMaxRows*fNModules); 
    fhCellIdCellLargeTimeSpread->SetXTitle("Absolute Cell Id");
    outputContainer->Add(fhCellIdCellLargeTimeSpread);
    
    fhTime  = new TH1F ("hTime","Cell Time",ntimebins,timemin,timemax); 
    fhTime->SetXTitle("Cell Time (ns)");
    outputContainer->Add(fhTime);
    
    fhTimeId  = new TH2F ("hTimeId","Cell Time vs Absolute Id",
                          ntimebins,timemin,timemax,fNMaxRows*fNMaxCols*fNModules,0,fNMaxRows*fNMaxCols*fNModules); 
    fhTimeId->SetXTitle("Cell Time (ns)");
    fhTimeId->SetYTitle("Cell Absolute Id");
    outputContainer->Add(fhTimeId);
    
    fhTimeAmp  = new TH2F ("hTimeAmp","Cell Time vs Cell Energy",nptbins*2,ptmin,ptmax,ntimebins,timemin,timemax); 
    fhTimeAmp->SetYTitle("Cell Time (ns)");
    fhTimeAmp->SetXTitle("Cell Energy (GeV)");
    outputContainer->Add(fhTimeAmp);
    
  }
  
  
  fhClusterNoMaxCellWeight = new TH2F ("hClusterNoMaxCellWeight"," #S weight_{no max} / #S weight_{tot} per cluster", nptbins,ptmin,ptmax, 100,0,1); 
  fhClusterNoMaxCellWeight->SetXTitle("E (GeV)");
  fhClusterNoMaxCellWeight->SetYTitle("#S weight_{no max} / #S weight_{tot}");
  outputContainer->Add(fhClusterNoMaxCellWeight);     
	
  if(fCorrelate){
    //PHOS vs EMCAL
    fhCaloCorrNClusters  = new TH2F ("hCaloCorrNClusters","# clusters in EMCAL vs PHOS", nclbins,nclmin,nclmax,nclbins,nclmin,nclmax); 
    fhCaloCorrNClusters->SetXTitle("number of clusters in EMCAL");
    fhCaloCorrNClusters->SetYTitle("number of clusters in PHOS");
    outputContainer->Add(fhCaloCorrNClusters);
    
    fhCaloCorrEClusters  = new TH2F ("hCaloCorrEClusters","summed energy of clusters in EMCAL vs PHOS", nptbins,ptmin,ptmax,nptbins,ptmin,ptmax); 
    fhCaloCorrEClusters->SetXTitle("#Sigma E of clusters in EMCAL (GeV)");
    fhCaloCorrEClusters->SetYTitle("#Sigma E of clusters in PHOS (GeV)");
    outputContainer->Add(fhCaloCorrEClusters);
    
    fhCaloCorrNCells  = new TH2F ("hCaloCorrNCells","# Cells in EMCAL vs PHOS", ncebins,ncemin,ncemax, ncebins,ncemin,ncemax); 
    fhCaloCorrNCells->SetXTitle("number of Cells in EMCAL");
    fhCaloCorrNCells->SetYTitle("number of Cells in PHOS");
    outputContainer->Add(fhCaloCorrNCells);
    
    fhCaloCorrECells  = new TH2F ("hCaloCorrECells","summed energy of Cells in EMCAL vs PHOS", nptbins*2,ptmin,ptmax*2,nptbins*2,ptmin,ptmax*2); 
    fhCaloCorrECells->SetXTitle("#Sigma E of Cells in EMCAL (GeV)");
    fhCaloCorrECells->SetYTitle("#Sigma E of Cells in PHOS (GeV)");
    outputContainer->Add(fhCaloCorrECells);
    
    //Calorimeter VS V0 signal
    fhCaloV0SCorrNClusters  = new TH2F ("hCaloV0SNClusters",Form("# clusters in %s vs V0 signal",fCalorimeter.Data()), nv0sbins,nv0smin,nv0smax,nclbins,nclmin,nclmax); 
    fhCaloV0SCorrNClusters->SetXTitle("V0 signal");
    fhCaloV0SCorrNClusters->SetYTitle(Form("number of clusters in %s",fCalorimeter.Data()));
    outputContainer->Add(fhCaloV0SCorrNClusters);
    
    fhCaloV0SCorrEClusters  = new TH2F ("hCaloV0SEClusters",Form("summed energy of clusters in %s vs V0 signal",fCalorimeter.Data()), nv0sbins,nv0smin,nv0smax,nptbins,ptmin,ptmax); 
    fhCaloV0SCorrEClusters->SetXTitle("V0 signal");
    fhCaloV0SCorrEClusters->SetYTitle(Form("#Sigma E of clusters in %s (GeV)",fCalorimeter.Data()));
    outputContainer->Add(fhCaloV0SCorrEClusters);
    
    fhCaloV0SCorrNCells  = new TH2F ("hCaloV0SNCells",Form("# Cells in %s vs V0 signal",fCalorimeter.Data()), nv0sbins,nv0smin,nv0smax, ncebins,ncemin,ncemax); 
    fhCaloV0SCorrNCells->SetXTitle("V0 signal");
    fhCaloV0SCorrNCells->SetYTitle(Form("number of Cells in %s",fCalorimeter.Data()));
    outputContainer->Add(fhCaloV0SCorrNCells);
    
    fhCaloV0SCorrECells  = new TH2F ("hCaloV0SECells",Form("summed energy of Cells in %s vs V0 signal",fCalorimeter.Data()), nv0sbins,nv0smin,nv0smax,nptbins,ptmin,ptmax); 
    fhCaloV0SCorrECells->SetXTitle("V0 signal");
    fhCaloV0SCorrECells->SetYTitle(Form("#Sigma E of Cells in %s (GeV)",fCalorimeter.Data()));
    outputContainer->Add(fhCaloV0SCorrECells);    
    
    //Calorimeter VS V0 multiplicity
    fhCaloV0MCorrNClusters  = new TH2F ("hCaloV0MNClusters",Form("# clusters in %s vs V0 signal",fCalorimeter.Data()), nv0mbins,nv0mmin,nv0mmax,nclbins,nclmin,nclmax); 
    fhCaloV0MCorrNClusters->SetXTitle("V0 signal");
    fhCaloV0MCorrNClusters->SetYTitle(Form("number of clusters in %s",fCalorimeter.Data()));
    outputContainer->Add(fhCaloV0MCorrNClusters);
    
    fhCaloV0MCorrEClusters  = new TH2F ("hCaloV0MEClusters",Form("summed energy of clusters in %s vs V0 signal",fCalorimeter.Data()), nv0mbins,nv0mmin,nv0mmax,nptbins,ptmin,ptmax); 
    fhCaloV0MCorrEClusters->SetXTitle("V0 signal");
    fhCaloV0MCorrEClusters->SetYTitle(Form("#Sigma E of clusters in %s (GeV)",fCalorimeter.Data()));
    outputContainer->Add(fhCaloV0MCorrEClusters);
    
    fhCaloV0MCorrNCells  = new TH2F ("hCaloV0MNCells",Form("# Cells in %s vs V0 signal",fCalorimeter.Data()), nv0mbins,nv0mmin,nv0mmax, ncebins,ncemin,ncemax); 
    fhCaloV0MCorrNCells->SetXTitle("V0 signal");
    fhCaloV0MCorrNCells->SetYTitle(Form("number of Cells in %s",fCalorimeter.Data()));
    outputContainer->Add(fhCaloV0MCorrNCells);
    
    fhCaloV0MCorrECells  = new TH2F ("hCaloV0MECells",Form("summed energy of Cells in %s vs V0 signal",fCalorimeter.Data()), nv0mbins,nv0mmin,nv0mmax,nptbins,ptmin,ptmax); 
    fhCaloV0MCorrECells->SetXTitle("V0 signal");
    fhCaloV0MCorrECells->SetYTitle(Form("#Sigma E of Cells in %s (GeV)",fCalorimeter.Data()));
    outputContainer->Add(fhCaloV0MCorrECells);    
    
    //Calorimeter VS Track multiplicity
    fhCaloTrackMCorrNClusters  = new TH2F ("hCaloTrackMNClusters",Form("# clusters in %s vs # tracks",fCalorimeter.Data()), ntrmbins,ntrmmin,ntrmmax,nclbins,nclmin,nclmax); 
    fhCaloTrackMCorrNClusters->SetXTitle("# tracks");
    fhCaloTrackMCorrNClusters->SetYTitle(Form("number of clusters in %s",fCalorimeter.Data()));
    outputContainer->Add(fhCaloTrackMCorrNClusters);
    
    fhCaloTrackMCorrEClusters  = new TH2F ("hCaloTrackMEClusters",Form("summed energy of clusters in %s vs # tracks",fCalorimeter.Data()), ntrmbins,ntrmmin,ntrmmax,nptbins,ptmin,ptmax); 
    fhCaloTrackMCorrEClusters->SetXTitle("# tracks");
    fhCaloTrackMCorrEClusters->SetYTitle(Form("#Sigma E of clusters in %s (GeV)",fCalorimeter.Data()));
    outputContainer->Add(fhCaloTrackMCorrEClusters);
    
    fhCaloTrackMCorrNCells  = new TH2F ("hCaloTrackMNCells",Form("# Cells in %s vs # tracks",fCalorimeter.Data()), ntrmbins,ntrmmin,ntrmmax, ncebins,ncemin,ncemax); 
    fhCaloTrackMCorrNCells->SetXTitle("# tracks");
    fhCaloTrackMCorrNCells->SetYTitle(Form("number of Cells in %s",fCalorimeter.Data()));
    outputContainer->Add(fhCaloTrackMCorrNCells);
    
    fhCaloTrackMCorrECells  = new TH2F ("hCaloTrackMECells",Form("summed energy of Cells in %s vs # tracks",fCalorimeter.Data()), ntrmbins,ntrmmin,ntrmmax,nptbins,ptmin,ptmax); 
    fhCaloTrackMCorrECells->SetXTitle("# tracks");
    fhCaloTrackMCorrECells->SetYTitle(Form("#Sigma E of Cells in %s (GeV)",fCalorimeter.Data()));
    outputContainer->Add(fhCaloTrackMCorrECells);    
    
    
  }//correlate calorimeters
  
  //Module histograms
  
  fhEMod  = new TH2F ("hE_Mod","Cluster reconstructed Energy in each present Module",nptbins,ptmin,ptmax,fNModules,0,fNModules); 
  fhEMod->SetXTitle("E (GeV)");
  fhEMod->SetYTitle("Module");
  outputContainer->Add(fhEMod);
  
  fhNClustersMod  = new TH2F ("hNClusters_Mod","# clusters vs Module", nclbins,nclmin,nclmax,fNModules,0,fNModules); 
  fhNClustersMod->SetXTitle("number of clusters");
  fhNClustersMod->SetYTitle("Module");
  outputContainer->Add(fhNClustersMod);
  
  fhNCellsMod  = new TH2F ("hNCells_Mod","# cells vs Module", fNMaxCols*fNMaxRows,0,fNMaxCols*fNMaxRows,fNModules,0,fNModules); 
  fhNCellsMod->SetXTitle("n cells");
  fhNCellsMod->SetYTitle("Module");
  outputContainer->Add(fhNCellsMod);
    
  Int_t colmaxs = fNMaxCols;
  Int_t rowmaxs = fNMaxRows;
  if(fCalorimeter=="EMCAL"){
    colmaxs=2*fNMaxCols;
    rowmaxs=Int_t(fNModules/2)*fNMaxRows;
  }
  else{
    rowmaxs=fNModules*fNMaxRows;
  }
  
  fhGridCellsMod  = new TH2F ("hGridCells",Form("Entries in grid of cells"), 
                                    colmaxs+2,-1.5,colmaxs+0.5, rowmaxs+2,-1.5,rowmaxs+0.5); 
  fhGridCellsMod->SetYTitle("row (phi direction)");
  fhGridCellsMod->SetXTitle("column (eta direction)");
  outputContainer->Add(fhGridCellsMod);
  
  fhGridCellsEMod  = new TH2F ("hGridCellsE","Accumulated energy in grid of cells", 
                                     colmaxs+2,-1.5,colmaxs+0.5, rowmaxs+2,-1.5,rowmaxs+0.5); 
  fhGridCellsEMod->SetYTitle("row (phi direction)");
  fhGridCellsEMod->SetXTitle("column (eta direction)");
  outputContainer->Add(fhGridCellsEMod);
  
  fhGridCellsTimeMod  = new TH2F ("hGridCellsTime","Accumulated time in grid of cells", 
                                        colmaxs+2,-1.5,colmaxs+0.5, rowmaxs+2,-1.5,rowmaxs+0.5); 
  fhGridCellsTimeMod->SetYTitle("row (phi direction)");
  fhGridCellsTimeMod->SetXTitle("column (eta direction)");
  outputContainer->Add(fhGridCellsTimeMod);  
  
  fhNCellsPerClusterMod  = new TH2F*[fNModules];
  fhNCellsPerClusterModNoCut = new TH2F*[fNModules];
  fhTimeAmpPerRCU        = new TH2F*[fNModules*fNRCU];
  fhIMMod                = new TH2F*[fNModules];
  
  for(Int_t imod = 0; imod < fNModules; imod++){
    
    fhNCellsPerClusterMod[imod]  = new TH2F (Form("hNCellsPerCluster_Mod%d",imod),
                                             Form("# cells per cluster vs cluster energy in Module %d",imod), 
                                             nptbins,ptmin,ptmax, nceclbins,nceclmin,nceclmax); 
    fhNCellsPerClusterMod[imod]->SetXTitle("E (GeV)");
    fhNCellsPerClusterMod[imod]->SetYTitle("n cells");
    outputContainer->Add(fhNCellsPerClusterMod[imod]);

    fhNCellsPerClusterModNoCut[imod]  = new TH2F (Form("hNCellsPerClusterNoCut_Mod%d",imod),
                                             Form("# cells per cluster vs cluster energy in Module %d, no cut",imod), 
                                             nptbins,ptmin,ptmax, nceclbins,nceclmin,nceclmax); 
    fhNCellsPerClusterModNoCut[imod]->SetXTitle("E (GeV)");
    fhNCellsPerClusterModNoCut[imod]->SetYTitle("n cells");
    outputContainer->Add(fhNCellsPerClusterModNoCut[imod]);
    
    if(GetReader()->GetDataType()==AliCaloTrackReader::kESD) {
      
      for(Int_t ircu = 0; ircu < fNRCU; ircu++){
        fhTimeAmpPerRCU[imod*fNRCU+ircu]  = new TH2F (Form("hTimeAmp_Mod%d_RCU%d",imod,ircu),
                                                      Form("Cell Energy vs Cell Time in Module %d, RCU %d ",imod,ircu), 
                                                      nptbins,ptmin,ptmax,ntimebins,timemin,timemax); 
        fhTimeAmpPerRCU[imod*fNRCU+ircu]->SetXTitle("E (GeV)");
        fhTimeAmpPerRCU[imod*fNRCU+ircu]->SetYTitle("time (ns)");
        outputContainer->Add(fhTimeAmpPerRCU[imod*fNRCU+ircu]);
        
      }
    }
    
    if(fFillAllPi0Histo){
      fhIMMod[imod]  = new TH2F (Form("hIM_Mod%d",imod),
                                 Form("Cluster pairs Invariant mass vs reconstructed pair energy in Module %d, n cell > 1",imod),
                                 nptbins,ptmin,ptmax,nmassbins,massmin,massmax); 
      fhIMMod[imod]->SetXTitle("p_{T, cluster pairs} (GeV) ");
      fhIMMod[imod]->SetYTitle("M_{cluster pairs} (GeV/c^{2})");
      outputContainer->Add(fhIMMod[imod]);
      
    }
  }
  
  //Monte Carlo Histograms
  
  TString particleName[] = { "Photon", "Pi0", "Eta", "Electron", "NeutralHadron", "ChargedHadron" };

  if(IsDataMC()){
    for(Int_t iPart = 0; iPart < 6; iPart++){
      for(Int_t iCh = 0; iCh < 2; iCh++){
        
        fhRecoMCDeltaE[iPart][iCh]  = new TH2F (Form("hRecoMCDeltaE_%s_Match%d",particleName[iPart].Data(),iCh),
                                                Form("MC - Reco E, %s, Matched %d",particleName[iPart].Data(),iCh), 
                                                nptbins, ptmin, ptmax, nptbins*2,-ptmax,ptmax); 
        fhRecoMCDeltaE[iPart][iCh]->SetXTitle("#Delta E (GeV)");
        outputContainer->Add(fhRecoMCDeltaE[iPart][iCh]);
        
        fhRecoMCDeltaPhi[iPart][iCh]  = new TH2F (Form("hRecoMCDeltaPhi_%s_Match%d",particleName[iPart].Data(),iCh),
                                                  Form("MC - Reco #phi, %s, Matched %d",particleName[iPart].Data(),iCh),
                                                  nptbins, ptmin, ptmax, nphibins*2,-phimax,phimax); 
        fhRecoMCDeltaPhi[iPart][iCh]->SetXTitle("#Delta #phi (rad)");
        outputContainer->Add(fhRecoMCDeltaPhi[iPart][iCh]);
        
        fhRecoMCDeltaEta[iPart][iCh]  = new TH2F (Form("hRecoMCDeltaEta_%s_Match%d",particleName[iPart].Data(),iCh),
                                                  Form("MC- Reco #eta, %s, Matched %d",particleName[iPart].Data(),iCh),
                                                  nptbins, ptmin, ptmax,netabins*2,-etamax,etamax); 
        fhRecoMCDeltaEta[iPart][iCh]->SetXTitle("#Delta #eta ");
        outputContainer->Add(fhRecoMCDeltaEta[iPart][iCh]);
        
        fhRecoMCE[iPart][iCh]  = new TH2F (Form("hRecoMCE_%s_Match%d",particleName[iPart].Data(),iCh),
                                           Form("E distribution, reconstructed vs generated, %s, Matched %d",particleName[iPart].Data(),iCh),
                                           nptbins,ptmin,ptmax,nptbins,ptmin,ptmax); 
        fhRecoMCE[iPart][iCh]->SetXTitle("E_{rec} (GeV)");
        fhRecoMCE[iPart][iCh]->SetYTitle("E_{gen} (GeV)");
        outputContainer->Add(fhRecoMCE[iPart][iCh]);	  
        
        fhRecoMCPhi[iPart][iCh]  = new TH2F (Form("hRecoMCPhi_%s_Match%d",particleName[iPart].Data(),iCh),
                                             Form("#phi distribution, reconstructed vs generated, %s, Matched %d",particleName[iPart].Data(),iCh),
                                             nphibins,phimin,phimax, nphibins,phimin,phimax); 
        fhRecoMCPhi[iPart][iCh]->SetXTitle("#phi_{rec} (rad)");
        fhRecoMCPhi[iPart][iCh]->SetYTitle("#phi_{gen} (rad)");
        outputContainer->Add(fhRecoMCPhi[iPart][iCh]);
        
        fhRecoMCEta[iPart][iCh]  = new TH2F (Form("hRecoMCEta_%s_Match%d",particleName[iPart].Data(),iCh),
                                             Form("#eta distribution, reconstructed vs generated, %s, Matched %d",particleName[iPart].Data(),iCh), 
                                             netabins,etamin,etamax,netabins,etamin,etamax); 
        fhRecoMCEta[iPart][iCh]->SetXTitle("#eta_{rec} ");
        fhRecoMCEta[iPart][iCh]->SetYTitle("#eta_{gen} ");
        outputContainer->Add(fhRecoMCEta[iPart][iCh]);
      }
    }  
    
    //Pure MC
    for(Int_t iPart = 0; iPart < 4; iPart++){
      fhGenMCE[iPart]     = new TH1F(Form("hGenMCE_%s",particleName[iPart].Data()) ,
                                     Form("p_{T} of generated %s",particleName[iPart].Data()),
                                     nptbins,ptmin,ptmax);
      fhGenMCEtaPhi[iPart] = new TH2F(Form("hGenMCEtaPhi_%s",particleName[iPart].Data()),
                                      Form("Y vs #phi of generated %s",particleName[iPart].Data()),
                                      netabins,etamin,etamax,nphibins,phimin,phimax);
    	
      fhGenMCE[iPart]    ->SetXTitle("p_{T} (GeV/c)");
      fhGenMCEtaPhi[iPart]->SetXTitle("#eta");
      fhGenMCEtaPhi[iPart]->SetYTitle("#phi (rad)");
      
      outputContainer->Add(fhGenMCE[iPart]);
      outputContainer->Add(fhGenMCEtaPhi[iPart]);
      
      
      fhGenMCAccE[iPart]     = new TH1F(Form("hGenMCAccE_%s",particleName[iPart].Data()) ,
                                        Form("p_{T} of generated %s",particleName[iPart].Data()),
                                        nptbins,ptmin,ptmax);
      fhGenMCAccEtaPhi[iPart] = new TH2F(Form("hGenMCAccEtaPhi_%s",particleName[iPart].Data()),
                                         Form("Y vs #phi of generated %s",particleName[iPart].Data()),
                                         netabins,etamin,etamax,nphibins,phimin,phimax);
    	
      fhGenMCAccE[iPart]    ->SetXTitle("p_{T} (GeV/c)");
      fhGenMCAccEtaPhi[iPart]->SetXTitle("#eta");
      fhGenMCAccEtaPhi[iPart]->SetYTitle("#phi (rad)");
      
      outputContainer->Add(fhGenMCAccE[iPart]);
      outputContainer->Add(fhGenMCAccEtaPhi[iPart]);
      
    }    
    
    //Vertex of generated particles 
    
    fhEMVxyz  = new TH2F ("hEMVxyz","Production vertex of reconstructed ElectroMagnetic particles",nvdistbins,vdistmin,vdistmax,nvdistbins,vdistmin,vdistmax);//,100,0,500); 
    fhEMVxyz->SetXTitle("v_{x}");
    fhEMVxyz->SetYTitle("v_{y}");
    //fhEMVxyz->SetZTitle("v_{z}");
    outputContainer->Add(fhEMVxyz);
    
    fhHaVxyz  = new TH2F ("hHaVxyz","Production vertex of reconstructed hadrons",nvdistbins,vdistmin,vdistmax,nvdistbins,vdistmin,vdistmax);//,100,0,500); 
    fhHaVxyz->SetXTitle("v_{x}");
    fhHaVxyz->SetYTitle("v_{y}");
    //fhHaVxyz->SetZTitle("v_{z}");
    outputContainer->Add(fhHaVxyz);
    
    fhEMR  = new TH2F ("hEMR","Distance to production vertex of reconstructed ElectroMagnetic particles vs E rec",nptbins,ptmin,ptmax,nvdistbins,vdistmin,vdistmax); 
    fhEMR->SetXTitle("E (GeV)");
    fhEMR->SetYTitle("TMath::Sqrt(v_{x}^{2}+v_{y}^{2})");
    outputContainer->Add(fhEMR);
    
    fhHaR  = new TH2F ("hHaR","Distance to production vertex of reconstructed Hadrons vs E rec",nptbins,ptmin,ptmax,nvdistbins,vdistmin,vdistmax); 
    fhHaR->SetXTitle("E (GeV)");
    fhHaR->SetYTitle("TMath::Sqrt(v_{x}^{2}+v_{y}^{2})");
    outputContainer->Add(fhHaR);
    
    
    //Track Matching 
    
//    fhMCEle1pOverE = new TH2F("hMCEle1pOverE","TRACK matches p/E, MC electrons",nptbins,ptmin,ptmax, nPoverEbins,pOverEmin,pOverEmax);
//    fhMCEle1pOverE->SetYTitle("p/E");
//    fhMCEle1pOverE->SetXTitle("p_{T} (GeV/c)");
//    outputContainer->Add(fhMCEle1pOverE);
//    
//    fhMCEle1dR = new TH1F("hMCEle1dR","TRACK matches dR, MC electrons",ndRbins,dRmin,dRmax);
//    fhMCEle1dR->SetXTitle("#Delta R (rad)");
//    outputContainer->Add(fhMCEle1dR) ;
//    
//    fhMCEle2MatchdEdx = new TH2F("hMCEle2MatchdEdx","dE/dx vs. p for all matches, MC electrons",nptbins,ptmin,ptmax,ndedxbins,dedxmin,dedxmax);
//    fhMCEle2MatchdEdx->SetXTitle("p (GeV/c)");
//    fhMCEle2MatchdEdx->SetYTitle("<dE/dx>");
//    outputContainer->Add(fhMCEle2MatchdEdx);
//    
//    fhMCChHad1pOverE = new TH2F("hMCChHad1pOverE","TRACK matches p/E, MC charged hadrons",nptbins,ptmin,ptmax, nPoverEbins,pOverEmin,pOverEmax);
//    fhMCChHad1pOverE->SetYTitle("p/E");
//    fhMCChHad1pOverE->SetXTitle("p_{T} (GeV/c)");
//    outputContainer->Add(fhMCChHad1pOverE);
//    
//    fhMCChHad1dR = new TH1F("hMCChHad1dR","TRACK matches dR, MC charged hadrons",ndRbins,dRmin,dRmax);
//    fhMCChHad1dR->SetXTitle("#Delta R (rad)");
//    outputContainer->Add(fhMCChHad1dR) ;
//    
//    fhMCChHad2MatchdEdx = new TH2F("hMCChHad2MatchdEdx","dE/dx vs. p for all matches, MC charged hadrons",nptbins,ptmin,ptmax,ndedxbins,dedxmin,dedxmax);
//    fhMCChHad2MatchdEdx->SetXTitle("p (GeV/c)");
//    fhMCChHad2MatchdEdx->SetYTitle("<dE/dx>");
//    outputContainer->Add(fhMCChHad2MatchdEdx);
//    
//    fhMCNeutral1pOverE = new TH2F("hMCNeutral1pOverE","TRACK matches p/E, MC neutrals",nptbins,ptmin,ptmax, nPoverEbins,pOverEmin,pOverEmax);
//    fhMCNeutral1pOverE->SetYTitle("p/E");
//    fhMCNeutral1pOverE->SetXTitle("p_{T} (GeV/c)");
//    outputContainer->Add(fhMCNeutral1pOverE);
//    
//    fhMCNeutral1dR = new TH1F("hMCNeutral1dR","TRACK matches dR, MC neutrals",ndRbins,dRmin,dRmax);
//    fhMCNeutral1dR->SetXTitle("#Delta R (rad)");
//    outputContainer->Add(fhMCNeutral1dR) ;
//    
//    fhMCNeutral2MatchdEdx = new TH2F("hMCNeutral2MatchdEdx","dE/dx vs. p for all matches, MC neutrals",nptbins,ptmin,ptmax,ndedxbins,dedxmin,dedxmax);
//    fhMCNeutral2MatchdEdx->SetXTitle("p (GeV/c)");
//    fhMCNeutral2MatchdEdx->SetYTitle("<dE/dx>");
//    outputContainer->Add(fhMCNeutral2MatchdEdx);
//    
//    fhMCEle1pOverER02 = new TH2F("hMCEle1pOverER02","TRACK matches p/E, MC electrons",nptbins,ptmin,ptmax, nPoverEbins,pOverEmin,pOverEmax);
//    fhMCEle1pOverER02->SetYTitle("p/E");
//    fhMCEle1pOverER02->SetXTitle("p_{T} (GeV/c)");
//    outputContainer->Add(fhMCEle1pOverER02);
//    
//    fhMCChHad1pOverER02 = new TH2F("hMCChHad1pOverER02","TRACK matches p/E, MC charged hadrons",nptbins,ptmin,ptmax, nPoverEbins,pOverEmin,pOverEmax);
//    fhMCChHad1pOverER02->SetYTitle("p/E");
//    fhMCChHad1pOverER02->SetXTitle("p_{T} (GeV/c)");
//    outputContainer->Add(fhMCChHad1pOverER02);
//    
//    fhMCNeutral1pOverER02 = new TH2F("hMCNeutral1pOverER02","TRACK matches p/E, MC neutrals",nptbins,ptmin,ptmax, nPoverEbins,pOverEmin,pOverEmax);
//    fhMCNeutral1pOverER02->SetYTitle("p/E");
//    fhMCNeutral1pOverER02->SetXTitle("p_{T} (GeV/c)");
//    outputContainer->Add(fhMCNeutral1pOverER02);
  }
  
  //  for(Int_t i = 0; i < outputContainer->GetEntries() ; i++)
  //    printf("i=%d, name= %s\n",i,outputContainer->At(i)->GetName());
  
  return outputContainer;
}

//__________________________________________________
void AliAnaCalorimeterQA::Init()
{ 
  //Check if the data or settings are ok
  
  if(fCalorimeter != "PHOS" && fCalorimeter !="EMCAL")
    AliFatal(Form("Wrong calorimeter name <%s>", fCalorimeter.Data()));
  
  if(GetReader()->GetDataType()== AliCaloTrackReader::kMC)
    AliFatal("Analysis of reconstructed data, MC reader not aplicable");
  
}


//__________________________________________________
void AliAnaCalorimeterQA::InitParameters()
{ 
  //Initialize the parameters of the analysis.
  AddToHistogramsName("AnaCaloQA_");
  
  fCalorimeter     = "EMCAL"; //or PHOS
  fNModules        = 12; // set maximum to maximum number of EMCAL modules
  fNRCU            = 2;  // set maximum number of RCU in EMCAL per SM
  fTimeCutMin      = -1;
  fTimeCutMax      = 9999999;
  fEMCALCellAmpMin = 0.2;
  fPHOSCellAmpMin  = 0.2;
  	
}

//__________________________________________________________________
void AliAnaCalorimeterQA::Print(const Option_t * opt) const
{
  //Print some relevant parameters set for the analysis
  if(! opt)
    return;
  
  printf("**** Print %s %s ****\n", GetName(), GetTitle() ) ;
  AliAnaPartCorrBaseClass::Print(" ");
  
  printf("Select Calorimeter %s \n",fCalorimeter.Data());
  printf("Time Cut: %3.1f < TOF  < %3.1f\n", fTimeCutMin, fTimeCutMax);
  printf("EMCAL Min Amplitude   : %2.1f GeV/c\n", fEMCALCellAmpMin) ;
  printf("PHOS Min Amplitude    : %2.1f GeV/c\n", fPHOSCellAmpMin) ;

} 

//__________________________________________________________________
void  AliAnaCalorimeterQA::MakeAnalysisFillHistograms() 
{
  //Fill Calorimeter QA histograms
  TLorentzVector mom  ;
  TLorentzVector mom2 ;
  TObjArray * caloClusters = NULL;
  Int_t nLabel = 0;
  Int_t *labels=0x0;
  Int_t nCaloClusters = 0;
  Int_t nCaloClustersAccepted = 0;
  Int_t nCaloCellsPerCluster = 0;
  Int_t nTracksMatched = 0;
  Int_t trackIndex = 0;
  Int_t nModule = -1;
  
  //Get vertex for photon momentum calculation and event selection
  Double_t v[3] = {0,0,0}; //vertex ;
  GetReader()->GetVertex(v);
  if (TMath::Abs(v[2]) > GetZvertexCut()) return ;  
  
  //Play with the MC stack if available	
  //Get the MC arrays and do some checks
  if(IsDataMC()){
    if(GetReader()->ReadStack()){
      
      if(!GetMCStack()) 
        AliFatal("Stack not available, is the MC handler called?\n");
      
      //Fill some pure MC histograms, only primaries.
      for(Int_t i=0 ; i<GetMCStack()->GetNprimary(); i++){//Only primary particles, for all MC transport put GetNtrack()
        TParticle *primary = GetMCStack()->Particle(i) ;
        //printf("i %d, %s: status = %d, primary? %d\n",i, primary->GetName(), primary->GetStatusCode(), primary->IsPrimary());
        if (primary->GetStatusCode() > 11) continue; //Working for PYTHIA and simple generators, check for HERWIG 
        primary->Momentum(mom);
        MCHistograms(mom,TMath::Abs(primary->GetPdgCode()));
      } //primary loop
    }
    else if(GetReader()->ReadAODMCParticles()){
      
      if(!GetReader()->GetAODMCParticles(0)) 	
        AliFatal("AODMCParticles not available!");
      
      //Fill some pure MC histograms, only primaries.
      for(Int_t i=0 ; i < (GetReader()->GetAODMCParticles(0))->GetEntriesFast(); i++){
        AliAODMCParticle *aodprimary = (AliAODMCParticle*) (GetReader()->GetAODMCParticles(0))->At(i) ;
        //printf("i %d, %s: primary? %d physical primary? %d, flag %d\n",
        //	   i,(TDatabasePDG::Instance()->GetParticle(aodprimary->GetPdgCode()))->GetName(), 
        //	   aodprimary->IsPrimary(), aodprimary->IsPhysicalPrimary(), aodprimary->GetFlag());
        if (!aodprimary->IsPrimary()) continue; //accept all which is not MC transport generated. Don't know how to avoid partons
        //aodprimary->Momentum(mom);
        mom.SetPxPyPzE(aodprimary->Px(),aodprimary->Py(),aodprimary->Pz(),aodprimary->E());
        MCHistograms(mom,TMath::Abs(aodprimary->GetPdgCode()));
      } //primary loop
      
    }
  }// is data and MC	
  
  
  //Get List with CaloClusters  
  if      (fCalorimeter == "PHOS")  caloClusters = GetPHOSClusters();
  else if (fCalorimeter == "EMCAL") caloClusters = GetEMCALClusters();
  else 
    AliFatal(Form("AliAnaCalorimeterQA::MakeAnalysisFillHistograms() - Wrong calorimeter name <%s>, END\n", fCalorimeter.Data()));
  
  
  if(!caloClusters) {
    AliFatal(Form("AliAnaCalorimeterQA::MakeAnalysisFillHistograms() - No CaloClusters available\n"));
  }
  else{
    
    //----------------------------------------------------------
    //Correlate Calorimeters and V0 and track Multiplicity
    //----------------------------------------------------------

    if(fCorrelate)	Correlate();
    
    //----------------------------------------------------------
    // CALOCLUSTERS
    //----------------------------------------------------------
    
    nCaloClusters = caloClusters->GetEntriesFast() ; 
    Int_t *nClustersInModule = new Int_t[fNModules];
    for(Int_t imod = 0; imod < fNModules; imod++ ) nClustersInModule[imod] = 0;
    
    if(GetDebug() > 0)
      printf("AliAnaCalorimeterQA::MakeAnalysisFillHistograms() - In %s there are %d clusters \n", fCalorimeter.Data(), nCaloClusters);
    
    AliVTrack * track = 0x0;
    Float_t pos[3] ;
    Double_t tof = 0;
    //Loop over CaloClusters
    //if(nCaloClusters > 0)printf("QA  : Vertex Cut passed %f, cut %f, entries %d, %s\n",v[2], 40., nCaloClusters, fCalorimeter.Data());
    for(Int_t iclus = 0; iclus < nCaloClusters; iclus++){
      
      if(GetDebug() > 0) printf("AliAnaCalorimeterQA::MakeAnalysisFillHistograms() - cluster: %d/%d, data %d \n",
                                iclus+1,nCaloClusters,GetReader()->GetDataType());
      
      AliVCluster* clus =  (AliVCluster*)caloClusters->At(iclus);
      AliVCaloCells * cell = 0x0; 
      if(fCalorimeter == "PHOS") cell =  GetPHOSCells();
      else			                 cell =  GetEMCALCells();
      
      //Get cluster kinematics
      clus->GetPosition(pos);
      clus->GetMomentum(mom,v);
      tof = clus->GetTOF()*1e9;
      if(tof < fTimeCutMin || tof > fTimeCutMax) continue;
      
      //Check only certain regions
      Bool_t in = kTRUE;
      if(IsFiducialCutOn()) in =  GetFiducialCut()->IsInFiducialCut(mom,fCalorimeter) ;
      if(!in) continue;
      
      //MC labels
      nLabel = clus->GetNLabels();
      labels = clus->GetLabels();
      
      //Cells per cluster
      nCaloCellsPerCluster = clus->GetNCells();
      //if(mom.E() > 10 && nCaloCellsPerCluster == 1 ) printf("%s:************** E = %f ********** ncells = %d\n",fCalorimeter.Data(), mom.E(),nCaloCellsPerCluster);
      
      //matched cluster with tracks
      nTracksMatched = clus->GetNTracksMatched();
      if(GetReader()->GetDataType() == AliCaloTrackReader::kESD){
        trackIndex     = clus->GetTrackMatchedIndex();
        if(trackIndex >= 0){
          track = (AliVTrack*)GetReader()->GetInputEvent()->GetTrack(trackIndex);
        }
        else{
          if(nTracksMatched == 1) nTracksMatched = 0;
          track = 0;
        }
      }//kESD
      else{//AODs
        if(nTracksMatched > 0) track = (AliVTrack*)clus->GetTrackMatched(0);
      }
      
      //======================
      //Cells in cluster
      //======================
      
      //Get list of contributors
      UShort_t * indexList = clus->GetCellsAbsId() ;
      
      if(fFillAllPosHisto) FillCellPositionHistograms(nCaloCellsPerCluster,indexList,pos,mom.E());
      
      // Get the fraction of the cluster energy that carries the cell with highest energy
      Float_t maxCellFraction = 0.;
      Int_t absIdMax = GetCaloUtils()->GetMaxEnergyCell(cell, clus,maxCellFraction);
      Int_t smMax =0; Int_t ietaaMax=-1; Int_t iphiiMax = 0; Int_t rcuMax = 0;
      smMax = GetModuleNumberCellIndexes(absIdMax,fCalorimeter, ietaaMax, iphiiMax, rcuMax);
      Int_t dIeta = 0;
      Int_t dIphi = 0;
      Double_t tmax  = cell->GetCellTime(absIdMax)*1e9;
      //Float_t  emax  = cell->GetCellAmplitude(absIdMax);
      
      if     (clus->E() < 2.){
        fhLambda0vsClusterMaxCellDiffE0->Fill(clus->GetM02(),      maxCellFraction);
        fhNCellsvsClusterMaxCellDiffE0 ->Fill(nCaloCellsPerCluster,maxCellFraction);
      }
      else if(clus->E() < 6.){
        fhLambda0vsClusterMaxCellDiffE2->Fill(clus->GetM02(),      maxCellFraction);
        fhNCellsvsClusterMaxCellDiffE2 ->Fill(nCaloCellsPerCluster,maxCellFraction);
      }
      else{
        fhLambda0vsClusterMaxCellDiffE6->Fill(clus->GetM02(),      maxCellFraction);  
        fhNCellsvsClusterMaxCellDiffE6 ->Fill(nCaloCellsPerCluster,maxCellFraction);
      }
      
      fhNCellsPerClusterNoCut  ->Fill(clus->E(), nCaloCellsPerCluster);
      nModule = GetModuleNumber(clus);
      if(nModule >=0 && nModule < fNModules) fhNCellsPerClusterModNoCut[nModule]->Fill(clus->E(), nCaloCellsPerCluster);
      
      fhClusterMaxCellDiffNoCut->Fill(clus->E(),maxCellFraction);
      
      // Calculate average time of cells in cluster and weighted average
      Double_t averTime     = 0;
      Double_t weightedTime = 0;
      Double_t weight       = 0;
      Double_t averTimeNoMax     = 0;
      Double_t weightedTimeNoMax = 0;
      Double_t weightNoMax       = 0;
      
      //if(GetReader()->GetDataType()==AliCaloTrackReader::kESD) {
        Float_t rawEnergy = 0;
        for (Int_t ipos = 0; ipos < nCaloCellsPerCluster; ipos++) 
          rawEnergy += cell->GetCellAmplitude(indexList[ipos]);
        
        for (Int_t ipos = 0; ipos < nCaloCellsPerCluster; ipos++) {
          Double_t w    = TMath::Max( 0., 4.5 + TMath::Log(cell->GetCellAmplitude(indexList[ipos]) / rawEnergy ));
          averTime     += cell->GetCellTime(indexList[ipos])*1e9;
          weightedTime += cell->GetCellTime(indexList[ipos])*1e9 * w;
          weight       += w;
          if(indexList[ipos]!=absIdMax){
            averTimeNoMax     += cell->GetCellTime(indexList[ipos])*1e9;
            weightedTimeNoMax += cell->GetCellTime(indexList[ipos])*1e9 * w;
            weightNoMax       += w;
          }
        }        
        
        averTime      /= nCaloCellsPerCluster;
        if(nCaloCellsPerCluster > 1 ) averTimeNoMax /= (nCaloCellsPerCluster-1);

       if(weight > 0 )      weightedTime /= weight;        
       else {         
         printf("AliAnaCalorimeterQA:: Null weight! Investigate: E %f GeV, ncells %d, time max cell %f ns, average time %f ns, absIdMax %d, SM %d\n",
                   rawEnergy,nCaloCellsPerCluster, tmax, averTime,absIdMax,GetModuleNumber(clus));
       }
        
       if(weightNoMax > 0 ) weightedTimeNoMax /= weightNoMax;

      //} // only possible in ESDs
      
      //======================
      //Bad clusters selection
      //======================
      //Check bad clusters if rejection was not on
      
      Bool_t badCluster = kFALSE;
      if(fCalorimeter=="EMCAL" && !GetCaloUtils()->GetEMCALRecoUtils()->IsRejectExoticCluster()){
        //Bad clusters histograms
        Float_t minNCells = 1;
        if(clus->E() > 7) minNCells = TMath::Max(1,TMath::FloorNint(1 + TMath::Log(clus->E() - 7 )*1.7 ));
        if(nCaloCellsPerCluster <= minNCells) {
          //if(clus->GetM02() < 0.05) {
          
          badCluster = kTRUE;
          
          fhBadClusterEnergy     ->Fill(clus->E());
          fhBadClusterTimeEnergy ->Fill(clus->E(),tof);
          fhBadClusterMaxCellDiff->Fill(clus->E(),maxCellFraction);

          if(clus->GetM02() > 0 || TMath::Abs(clus->GetM20()) > 0 || clus->GetDispersion() > 0){
            
            fhBadClusterL0         ->Fill(clus->E(),clus->GetM02());
            fhBadClusterL1         ->Fill(clus->E(),clus->GetM20());
            fhBadClusterD          ->Fill(clus->E(),clus->GetDispersion());
            
          }
          
          //Clusters in event time difference
          
          for(Int_t iclus2 = 0; iclus2 < nCaloClusters; iclus2++ ){
            
            AliVCluster* clus2 =  (AliVCluster*)caloClusters->At(iclus2);
            
            if(clus->GetID()==clus2->GetID()) continue;
            
            if(clus->GetM02() > 0.01) 
              fhBadClusterPairDiffTimeE  ->Fill(clus->E(), tof-clus2->GetTOF()*1.e9);
            
          } // loop
          
          // Max cell compared to other cells in cluster
          if(GetReader()->GetDataType()==AliCaloTrackReader::kESD) {
            fhBadClusterMaxCellDiffAverageTime->Fill(clus->E(),tmax-averTime);
            fhBadClusterMaxCellDiffWeightTime ->Fill(clus->E(),tmax-weightedTime);
            fhBadClusterDiffWeightAverTime    ->Fill(clus->E(),weightedTime-averTime);
            fhBadClusterMaxCellDiffAverageNoMaxTime->Fill(clus->E(),tmax-averTimeNoMax);
            fhBadClusterMaxCellDiffWeightNoMaxTime ->Fill(clus->E(),tmax-weightedTimeNoMax);
            //printf("E %f, tmax %f, aver %f, weigh %f, averNoMax %f, weightNoMax %f\n",
            //       clus->E(),tmax,averTime,weightedTime,averTimeNoMax,weightedTimeNoMax);
          }           

          if( weight > 0 ) fhBadClusterNoMaxCellWeight ->Fill(clus->E(),weightNoMax/weight);
          //if( weight > 0 && weightNoMax > 0.0)printf("weight %f, weightNoMax %f\n",weight,weightNoMax);

          for (Int_t ipos = 0; ipos < nCaloCellsPerCluster; ipos++) {
            Int_t absId  = indexList[ipos]; 
            if(absId!=absIdMax){
              Float_t frac = cell->GetCellAmplitude(absId)/cell->GetCellAmplitude(absIdMax);
              fhBadClusterMaxCellCloseCellRatio->Fill(mom.E(),frac);
              fhBadClusterMaxCellCloseCellDiff ->Fill(mom.E(),cell->GetCellAmplitude(absIdMax)-cell->GetCellAmplitude(absId));
              if(GetReader()->GetDataType()==AliCaloTrackReader::kESD) {
                Float_t diff = (tmax-cell->GetCellTime(absId)*1e9);
                fhBadCellTimeSpreadRespectToCellMax->Fill(mom.E(), diff);
              }
            }// Not max
          }//loop
        }//Bad cluster
      }
      
      if(!badCluster){
                
        fhClusterMaxCellDiff->Fill(clus->E(),maxCellFraction);        
        fhClusterTimeEnergy ->Fill(mom.E(),tof);
        
        //Clusters in event time difference
        for(Int_t iclus2 = 0; iclus2 < nCaloClusters; iclus2++ ){
          
          AliVCluster* clus2 =  (AliVCluster*) caloClusters->At(iclus2);
          
          if(clus->GetID()==clus2->GetID()) continue;
          
          if(clus->GetM02() > 0.01) {
            fhClusterPairDiffTimeE  ->Fill(clus->E(), tof-clus2->GetTOF()*1.e9);
          }
        }        
        
        if(nCaloCellsPerCluster > 1){
          
          // check time of cells respect to max energy cell
          
          if(GetReader()->GetDataType()==AliCaloTrackReader::kESD) {
            fhClusterMaxCellDiffAverageTime->Fill(clus->E(),tmax-averTime);
            fhClusterMaxCellDiffWeightTime ->Fill(clus->E(),tmax-weightedTime);
            fhClusterDiffWeightAverTime ->Fill(clus->E(), weightedTime-averTime);
            fhClusterMaxCellDiffAverageNoMaxTime->Fill(clus->E(),tmax-averTimeNoMax);
            fhClusterMaxCellDiffWeightNoMaxTime ->Fill(clus->E(),tmax-weightedTimeNoMax);
 
          }
          
          if( weight > 0 )fhClusterNoMaxCellWeight ->Fill(clus->E(),weightNoMax/weight);

          for (Int_t ipos = 0; ipos < nCaloCellsPerCluster; ipos++) {
            
            Int_t absId  = indexList[ipos];             
            if(absId == absIdMax) continue;
            
            Float_t frac = cell->GetCellAmplitude(absId)/cell->GetCellAmplitude(absIdMax);            
            fhClusterMaxCellCloseCellRatio->Fill(mom.E(),frac);
            fhClusterMaxCellCloseCellDiff ->Fill(mom.E(),cell->GetCellAmplitude(absIdMax)-cell->GetCellAmplitude(absId));

            if(GetReader()->GetDataType()==AliCaloTrackReader::kESD) {
 
              Float_t diff = (tmax-cell->GetCellTime(absId)*1e9);
              fhCellTimeSpreadRespectToCellMax->Fill(mom.E(), diff);
              if(TMath::Abs(TMath::Abs(diff) > 100) && mom.E() > 1 ) fhCellIdCellLargeTimeSpread->Fill(absId);
            }
            
            Int_t sm =0; Int_t ietaa=-1; Int_t iphii = 0; Int_t rcu = 0;
            sm = GetModuleNumberCellIndexes(absId,fCalorimeter, ietaa, iphii, rcu);
            if(dIphi < TMath::Abs(iphii-iphiiMax)) dIphi = TMath::Abs(iphii-iphiiMax);
            if(smMax==sm){
              if(dIeta < TMath::Abs(ietaa-ietaaMax)) dIeta = TMath::Abs(ietaa-ietaaMax);
            }
            else {
              Int_t ietaaShift    = ietaa;
              Int_t ietaaMaxShift = ietaaMax;
              if (ietaa > ietaaMax)  ietaaMaxShift+=48;
              else                   ietaaShift   +=48;
              if(dIeta < TMath::Abs(ietaaShift-ietaaMaxShift)) dIeta = TMath::Abs(ietaaShift-ietaaMaxShift);
            }
            
            //if(TMath::Abs(clus->GetM20()) < 0.0001 && clus->GetNCells() > 3){
            //  printf("Good : E %f, mcells %d, l0 %f, l1 %f, d %f, cell max t %f, cluster TOF %f, sm %d, icol %d, irow %d; Max icol %d, irow %d \n", 
            //            clus->E(), clus->GetNCells(),clus->GetM02(), clus->GetM20(), clus->GetDispersion(),tmax, tof,sm,ietaa,iphii, ietaaMax, iphiiMax);
            //}
            
            
          }// fill cell-cluster histogram loop

          if(nCaloCellsPerCluster > 3){
            
            Int_t matched = 0;
            if( nTracksMatched > 0 ) matched = 1;
            Float_t dIA    = 1.*(dIphi-dIeta)/(dIeta+dIphi);
            
            if     (mom.E() < 2 ) fhDeltaIEtaDeltaIPhiE0[matched]->Fill(dIeta,dIphi);
            else if(mom.E() < 6 ) fhDeltaIEtaDeltaIPhiE2[matched]->Fill(dIeta,dIphi);
            else                  fhDeltaIEtaDeltaIPhiE6[matched]->Fill(dIeta,dIphi);
            
            fhDeltaIA[matched]->Fill(mom.E(),dIA);
            
            if(mom.E() > 0.5){
                     
              fhDeltaIAL0[matched]->Fill(clus->GetM02(),dIA);
              fhDeltaIAL1[matched]->Fill(clus->GetM20(),dIA);
              fhDeltaIANCells[matched]->Fill(nCaloCellsPerCluster,dIA);
              
            }
            
            if(IsDataMC()){
              Int_t tag = GetMCAnalysisUtils()->CheckOrigin(labels,nLabel, GetReader(),0);
              if( (GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCPhoton) || 
                   GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCPi0)    || 
                   GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCEta)         ) &&
                 !GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCConversion)        ){
                fhDeltaIAMC[0]->Fill(mom.E(),dIA);//Pure Photon
              }
              else if ( GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCElectron) &&
                       !GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCConversion)  ){
                fhDeltaIAMC[1]->Fill(mom.E(),dIA);//Pure electron
              }
              else if ( GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCConversion)  ){
                fhDeltaIAMC[2]->Fill(mom.E(),dIA);//Converted cluster
              }
              else{ 
                fhDeltaIAMC[3]->Fill(mom.E(),dIA);//Hadrons
              }
              
            }
          }// 4 cells at least in cluster for size study
          
        }//check time and energy of cells respect to max energy cell if cluster of more than 1 cell
        
        
        //Get module of cluster

        nCaloClustersAccepted++;
        if(nModule >=0 && nModule < fNModules) {
          if     (fCalorimeter=="EMCAL" && mom.E() > 2*fEMCALCellAmpMin)  nClustersInModule[nModule]++;
          else if(fCalorimeter=="PHOS"  && mom.E() > 2*fPHOSCellAmpMin )  nClustersInModule[nModule]++;
        }
        
        //-----------------------------------------------------------
        //Fill histograms related to single cluster or track matching
        //-----------------------------------------------------------
        ClusterHistograms(mom, pos, nCaloCellsPerCluster, nModule, nTracksMatched, track, labels, nLabel);	
        
        
        //-----------------------------------------------------------
        //Invariant mass
        //-----------------------------------------------------------
        if(fFillAllPi0Histo){
          if(GetDebug()>1) printf("Invariant mass \n");
          
          //do not do for bad vertex
          // Float_t fZvtxCut = 40. ;	
          if(v[2]<-GetZvertexCut() || v[2]> GetZvertexCut()) continue ; //Event can not be used (vertex, centrality,... cuts not fulfilled)
          
          Int_t nModule2 = -1;
          if (nCaloClusters > 1 && nCaloCellsPerCluster > 1) {
            for(Int_t jclus = iclus + 1 ; jclus < nCaloClusters ; jclus++) {
              AliVCluster* clus2 =  (AliVCluster*)caloClusters->At(jclus);
              if( clus2->GetNCells() > 1 ){
                
                //Get cluster kinematics
                clus2->GetMomentum(mom2,v);
                
                //Check only certain regions
                Bool_t in2 = kTRUE;
                if(IsFiducialCutOn()) in2 =  GetFiducialCut()->IsInFiducialCut(mom2,fCalorimeter) ;
                if(!in2) continue;	
                
                //Get module of cluster
                nModule2 = GetModuleNumber(clus2);
                
                //Fill invariant mass histograms
                
                //All modules
                fhIM  ->Fill((mom+mom2).Pt(),(mom+mom2).M());
                
                //Single module
                if(nModule == nModule2 && nModule >=0 && nModule < fNModules)
                  fhIMMod[nModule]->Fill((mom+mom2).Pt(),(mom+mom2).M());
                
                //Asymetry histograms
                fhAsym->Fill((mom+mom2).Pt(),TMath::Abs((mom.E()-mom2.E())/(mom.E()+mom2.E())));
              } 
            }// 2nd cluster loop
          } // At least one cell in cluster and one cluster in the array
        }//Fill Pi0
        
      }//good cluster
      
    }//cluster loop
    
    //Number of clusters histograms
    if(nCaloClustersAccepted > 0) fhNClusters->Fill(nCaloClustersAccepted);
    //  Number of clusters per module
    for(Int_t imod = 0; imod < fNModules; imod++ ){ 
      if(GetDebug() > 1) 
        printf("AliAnaCalorimeterQA::MakeAnalysisFillHistograms() - module %d calo %s clusters %d\n", imod, fCalorimeter.Data(), nClustersInModule[imod]); 
      fhNClustersMod->Fill(nClustersInModule[imod],imod);
    }
    delete [] nClustersInModule;
    //delete caloClusters;
  }// calo clusters array exists
  
  //----------------------------------------------------------
  // CALOCELLS
  //----------------------------------------------------------
  
  AliVCaloCells * cell = 0x0; 
  Int_t ncells = 0;
  if(fCalorimeter == "PHOS") 
    cell = GetPHOSCells();
  else		              
    cell = GetEMCALCells();
  
  if(!cell){ 
    AliFatal(Form("No %s CELLS available for analysis",fCalorimeter.Data()));
    return; // just to trick coverity
  }
  
  if(GetDebug() > 0) 
    printf("AliAnaCalorimeterQA::MakeAnalysisFillHistograms() - %s cell entries %d\n", fCalorimeter.Data(), cell->GetNumberOfCells());    
  
  //Init arrays and used variables
  Int_t *nCellsInModule = new Int_t[fNModules];
  for(Int_t imod = 0; imod < fNModules; imod++ ) nCellsInModule[imod] = 0;
  Int_t icol     = -1;
  Int_t irow     = -1;
  Int_t iRCU     = -1;
  Float_t amp    = 0.;
  Float_t time   = 0.;
  Int_t id       = -1;
  Float_t recalF = 1.;  
  
  for (Int_t iCell = 0; iCell < cell->GetNumberOfCells(); iCell++) {      
    if(GetDebug() > 2)  
      printf("AliAnaCalorimeterQA::MakeAnalysisFillHistograms() - Cell : amp %f, absId %d \n", cell->GetAmplitude(iCell), cell->GetCellNumber(iCell));
    nModule = GetModuleNumberCellIndexes(cell->GetCellNumber(iCell),fCalorimeter, icol, irow, iRCU);
    if(GetDebug() > 2) 
      printf("\t module %d, column %d, row %d \n", nModule,icol,irow);
    
    if(nModule < fNModules) {	
      
      //Check if the cell is a bad channel
      if(GetCaloUtils()->IsBadChannelsRemovalSwitchedOn()){
        if(fCalorimeter=="EMCAL"){
          if(GetCaloUtils()->GetEMCALChannelStatus(nModule,icol,irow)) continue;
        }
        else {
          if(GetCaloUtils()->GetPHOSChannelStatus(nModule,icol,irow)) {
            printf("PHOS bad channel\n");
            continue;
          }
        }
      } // use bad channel map
      
      //Get Recalibration factor if set
      if (GetCaloUtils()->IsRecalibrationOn()) {
        if(fCalorimeter == "PHOS") recalF = GetCaloUtils()->GetPHOSChannelRecalibrationFactor(nModule,icol,irow);
        else		                   recalF = GetCaloUtils()->GetEMCALChannelRecalibrationFactor(nModule,icol,irow);
        //if(fCalorimeter == "PHOS")printf("Recalibration factor (sm,row,col)=(%d,%d,%d) -  %f\n",nModule,icol,irow,recalF);
      }
      
      amp     = cell->GetAmplitude(iCell)*recalF;
      time    = cell->GetTime(iCell)*1e9;//transform time to ns
      
      //Remove noisy channels, only possible in ESDs
      if(GetReader()->GetDataType() == AliCaloTrackReader::kESD){
        if(time < fTimeCutMin || time > fTimeCutMax) continue;
      }
      //if(amp > 3 && fCalorimeter=="EMCAL") printf("Amp = %f, time = %f, (mod, col, row)= (%d,%d,%d)\n",
      //										   amp,time,nModule,icol,irow);
      
      id      = cell->GetCellNumber(iCell);
      fhAmplitude->Fill(amp);
      fhAmpId    ->Fill(amp,id);
            
      if ((fCalorimeter=="EMCAL" && amp > fEMCALCellAmpMin) ||
          (fCalorimeter=="PHOS"  && amp > fPHOSCellAmpMin))   {
        
        nCellsInModule[nModule]++ ;
        
        Int_t icols = icol;
        Int_t irows = irow;
        if(fCalorimeter=="EMCAL"){
          icols = (nModule % 2) ? icol + fNMaxCols : icol;				
          irows = irow + fNMaxRows * Int_t(nModule / 2);
        }
        else {
          irows = irow + fNMaxRows * fNModules;
        }

        fhGridCellsMod ->Fill(icols,irows);
        fhGridCellsEMod->Fill(icols,irows,amp);
        
        if(GetReader()->GetDataType() == AliCaloTrackReader::kESD){
          //printf("%s: time %g\n",fCalorimeter.Data(), time);
          fhTime     ->Fill(time);
          fhTimeId   ->Fill(time,id);
          fhTimeAmp  ->Fill(amp,time);
          fhGridCellsTimeMod->Fill(icols,irows,time);

          fhTimeAmpPerRCU  [nModule*fNRCU+iRCU]->Fill(amp, time);
    
        }
      }
      
      //Get Eta-Phi position of Cell
      if(fFillAllPosHisto)
      {
        if(fCalorimeter=="EMCAL" && GetCaloUtils()->IsEMCALGeoMatrixSet()){
          Float_t celleta = 0.;
          Float_t cellphi = 0.;
          GetEMCALGeometry()->EtaPhiFromIndex(id, celleta, cellphi); 
          
          fhEtaPhiAmp->Fill(celleta,cellphi,amp);
          Double_t cellpos[] = {0, 0, 0};
          GetEMCALGeometry()->GetGlobal(id, cellpos);
          fhXCellE->Fill(cellpos[0],amp)  ; 
          fhYCellE->Fill(cellpos[1],amp)  ; 
          fhZCellE->Fill(cellpos[2],amp)  ;
          Float_t rcell = TMath::Sqrt(cellpos[0]*cellpos[0]+cellpos[1]*cellpos[1]);//+cellpos[2]*cellpos[2]);
          fhRCellE->Fill(rcell,amp)  ;
          fhXYZCell->Fill(cellpos[0],cellpos[1],cellpos[2])  ;
        }//EMCAL Cells
        else if(fCalorimeter=="PHOS" && GetCaloUtils()->IsPHOSGeoMatrixSet()){
          TVector3 xyz;
          Int_t relId[4], module;
          Float_t xCell, zCell;
          
          GetPHOSGeometry()->AbsToRelNumbering(id,relId);
          module = relId[0];
          GetPHOSGeometry()->RelPosInModule(relId,xCell,zCell);
          GetPHOSGeometry()->Local2Global(module,xCell,zCell,xyz);
          Float_t rcell = TMath::Sqrt(xyz.X()*xyz.X()+xyz.Y()*xyz.Y());
          fhXCellE ->Fill(xyz.X(),amp)  ; 
          fhYCellE ->Fill(xyz.Y(),amp)  ; 
          fhZCellE ->Fill(xyz.Z(),amp)  ;
          fhRCellE ->Fill(rcell  ,amp)  ;
          fhXYZCell->Fill(xyz.X(),xyz.Y(),xyz.Z())  ;
        }//PHOS cells
      }//fill cell position histograms
      
      if     (fCalorimeter=="EMCAL" && amp > fEMCALCellAmpMin) ncells ++ ;
      else if(fCalorimeter=="PHOS"  && amp > fPHOSCellAmpMin)  ncells ++ ;
      //else  
      //  printf("AliAnaCalorimeterQA::MakeAnalysisFillHistograms() - no %s CELLS passed the analysis cut\n",fCalorimeter.Data());    
    }//nmodules
  }//cell loop
  
  if(ncells > 0 )fhNCells->Fill(ncells) ; //fill the cells after the cut 
  
  //Number of cells per module
  for(Int_t imod = 0; imod < fNModules; imod++ ) {
    if(GetDebug() > 1) 
      printf("AliAnaCalorimeterQA::MakeAnalysisFillHistograms() - module %d calo %s cells %d\n", imod, fCalorimeter.Data(), nCellsInModule[imod]); 
    fhNCellsMod->Fill(nCellsInModule[imod],imod) ;
  }
  delete [] nCellsInModule;
  
  if(GetDebug() > 0)
    printf("AliAnaCalorimeterQA::MakeAnalysisFillHistograms() - End \n");
}


//_____________________________________________________________________________________________
void AliAnaCalorimeterQA::ClusterHistograms(const TLorentzVector mom, 
                                            Float_t *pos, const Int_t nCaloCellsPerCluster,const Int_t nModule,
                                            const Int_t nTracksMatched,  const AliVTrack * /*track*/,  
                                            const Int_t * labels, const Int_t nLabels){
  //Fill CaloCluster related histograms
	
  AliAODMCParticle * aodprimary  = 0x0;
  TParticle * primary = 0x0;
  Int_t tag = 0;	
  
  Float_t e   = mom.E();
  Float_t pt  = mom.Pt();
  Float_t eta = mom.Eta();
  Float_t phi = mom.Phi();
  if(phi < 0) phi +=TMath::TwoPi();
  if(GetDebug() > 0) {
    printf("AliAnaCalorimeterQA::ClusterHistograms() - cluster: E %2.3f, pT %2.3f, eta %2.3f, phi %2.3f \n",e,pt,eta,phi*TMath::RadToDeg());
    if(IsDataMC()) {
      //printf("\t Primaries: nlabels %d, labels pointer %p\n",nLabels,labels);
      printf("\t Primaries: nlabels %d\n",nLabels);
      if(!nLabels || !labels) printf("\t Strange, no labels!!!\n");
    }
  }
  
  fhE     ->Fill(e);	
  if(nModule >=0 && nModule < fNModules) fhEMod->Fill(e,nModule);
  if(fFillAllTH12){
    fhPt     ->Fill(pt);
    fhPhi    ->Fill(phi);
    fhEta    ->Fill(eta);
  }
  
  fhEtaPhiE->Fill(eta,phi,e);
  
  //Cells per cluster
  fhNCellsPerCluster   ->Fill(e, nCaloCellsPerCluster);
  if((fCalorimeter=="EMCAL" && GetReader()->GetEMCALPtMin() < 0.3) ||
     (fCalorimeter=="PHOS"  && GetReader()->GetPHOSPtMin()  < 0.3)) fhNCellsPerClusterMIP->Fill(e, nCaloCellsPerCluster);
  
  //Position
  if(fFillAllPosHisto2){
    fhXE     ->Fill(pos[0],e);
    fhYE     ->Fill(pos[1],e);
    fhZE     ->Fill(pos[2],e);
    if(fFillAllTH3)
      fhXYZ    ->Fill(pos[0], pos[1],pos[2]);
    
    fhXNCells->Fill(pos[0],nCaloCellsPerCluster);
    fhYNCells->Fill(pos[1],nCaloCellsPerCluster);
    fhZNCells->Fill(pos[2],nCaloCellsPerCluster);
    Float_t rxyz = TMath::Sqrt(pos[0]*pos[0]+pos[1]*pos[1]);//+pos[2]*pos[2]);
    fhRE     ->Fill(rxyz,e);
    fhRNCells->Fill(rxyz  ,nCaloCellsPerCluster);
  }
  
  if(nModule >=0 && nModule < fNModules) fhNCellsPerClusterMod[nModule]->Fill(e, nCaloCellsPerCluster);
  
  //Fill histograms only possible when simulation
  if(IsDataMC() && nLabels > 0 && labels){
    
    //Play with the MC stack if available
    Int_t label = labels[0];
    
    if(label < 0) {
      if(GetDebug() >= 0) printf("AliAnaCalorimeterQA::ClusterHistograms() *** bad label ***:  label %d \n", label);
      return;
    }
    
    Int_t pdg  =-1; Int_t pdg0  =-1;Int_t status = -1; Int_t iMother = -1; Int_t iParent = -1;
    Float_t vxMC= 0; Float_t vyMC = 0;	
    Float_t eMC = 0; Float_t ptMC= 0; Float_t phiMC =0; Float_t etaMC = 0;
    Int_t charge = 0;	
    
    //Check the origin.
    tag = GetMCAnalysisUtils()->CheckOrigin(labels,nLabels, GetReader(),0);
    
    if(GetReader()->ReadStack() && !GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCUnknown)){ //it MC stack and known tag
      
      if( label >= GetMCStack()->GetNtrack()) {
        if(GetDebug() >= 0) printf("AliAnaCalorimeterQA::ClusterHistograms() *** large label ***:  label %d, n tracks %d \n", label, GetMCStack()->GetNtrack());
        return ;
      }
      
      primary = GetMCStack()->Particle(label);
      iMother = label;
      pdg0    = TMath::Abs(primary->GetPdgCode());
      pdg     = pdg0;
      status  = primary->GetStatusCode();
      vxMC    = primary->Vx();
      vyMC    = primary->Vy();
      iParent = primary->GetFirstMother();
      
      if(GetDebug() > 1 ) {
        printf("AliAnaCalorimeterQA::ClusterHistograms() - Cluster most contributing mother: \n");
        printf("\t Mother label %d, pdg %d, %s, status %d, parent %d \n",iMother, pdg0, primary->GetName(),status, iParent);
      }
      
      //Get final particle, no conversion products
      if(GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCConversion)){
        //Get the parent
        primary = GetMCStack()->Particle(iParent);
        pdg = TMath::Abs(primary->GetPdgCode());
        if(GetDebug() > 1 ) printf("AliAnaCalorimeterQA::ClusterHistograms() - Converted cluster!. Find before conversion: \n");
        while((pdg == 22 || pdg == 11) && status != 1){
          iMother = iParent;
          primary = GetMCStack()->Particle(iMother);
          status  = primary->GetStatusCode();
          iParent = primary->GetFirstMother();
          pdg     = TMath::Abs(primary->GetPdgCode());
          if(GetDebug() > 1 )printf("\t pdg %d, index %d, %s, status %d \n",pdg, iMother,  primary->GetName(),status);	
        }	
        
        if(GetDebug() > 1 ) {
          printf("AliAnaCalorimeterQA::ClusterHistograms() - Converted Cluster mother before conversion: \n");
          printf("\t Mother label %d, pdg %d, %s, status %d, parent %d \n",iMother, pdg, primary->GetName(), status, iParent);
        }
        
      }
      
      //Overlapped pi0 (or eta, there will be very few), get the meson
      if(GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCPi0) || 
         GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCEta)){
        if(GetDebug() > 1 ) printf("AliAnaCalorimeterQA::ClusterHistograms() - Overlapped Meson decay!, Find it: \n");
        while(pdg != 111 && pdg != 221){
          iMother = iParent;
          primary = GetMCStack()->Particle(iMother);
          status  = primary->GetStatusCode();
          iParent = primary->GetFirstMother();
          pdg     = TMath::Abs(primary->GetPdgCode());
          if(GetDebug() > 1 ) printf("\t pdg %d, %s, index %d\n",pdg,  primary->GetName(),iMother);
          if(iMother==-1) {
            printf("AliAnaCalorimeterQA::ClusterHistograms() - Tagged as Overlapped photon but meson not found, why?\n");
            //break;
          }
        }
        
        if(GetDebug() > 2 ) printf("AliAnaCalorimeterQA::ClusterHistograms() - Overlapped %s decay, label %d \n", 
                                   primary->GetName(),iMother);
      }
      
      eMC    = primary->Energy();
      ptMC   = primary->Pt();
      phiMC  = primary->Phi();
      etaMC  = primary->Eta();
      pdg    = TMath::Abs(primary->GetPdgCode());
      charge = (Int_t) TDatabasePDG::Instance()->GetParticle(pdg)->Charge();
      
    }
    else if(GetReader()->ReadAODMCParticles() && !GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCUnknown)){//it MC AOD and known tag
      //Get the list of MC particles
      if(!GetReader()->GetAODMCParticles(0)) 
        AliFatal("MCParticles not available!");
      
      aodprimary = (AliAODMCParticle*) (GetReader()->GetAODMCParticles(0))->At(label);
      iMother = label;
      pdg0    = TMath::Abs(aodprimary->GetPdgCode());
      pdg     = pdg0;
      status  = aodprimary->IsPrimary();
      vxMC    = aodprimary->Xv();
      vyMC    = aodprimary->Yv();
      iParent = aodprimary->GetMother();
      
      if(GetDebug() > 1 ) {
        printf("AliAnaCalorimeterQA::ClusterHistograms() - Cluster most contributing mother: \n");
        printf("\t Mother label %d, pdg %d, Primary? %d, Physical Primary? %d, parent %d \n",
               iMother, pdg0, aodprimary->IsPrimary(), aodprimary->IsPhysicalPrimary(), iParent);
      }
      
      //Get final particle, no conversion products
      if(GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCConversion)){
        if(GetDebug() > 1 ) 
          printf("AliAnaCalorimeterQA::ClusterHistograms() - Converted cluster!. Find before conversion: \n");
        //Get the parent
        aodprimary = (AliAODMCParticle*)(GetReader()->GetAODMCParticles(0))->At(iParent);
        pdg = TMath::Abs(aodprimary->GetPdgCode());
        while ((pdg == 22 || pdg == 11) && !aodprimary->IsPhysicalPrimary()) {
          iMother    = iParent;
          aodprimary = (AliAODMCParticle*)(GetReader()->GetAODMCParticles(0))->At(iMother);
          status     = aodprimary->IsPrimary();
          iParent    = aodprimary->GetMother();
          pdg        = TMath::Abs(aodprimary->GetPdgCode());
          if(GetDebug() > 1 )
            printf("\t pdg %d, index %d, Primary? %d, Physical Primary? %d \n",
                   pdg, iMother, aodprimary->IsPrimary(), aodprimary->IsPhysicalPrimary());	
        }	
        
        if(GetDebug() > 1 ) {
          printf("AliAnaCalorimeterQA::ClusterHistograms() - Converted Cluster mother before conversion: \n");
          printf("\t Mother label %d, pdg %d, parent %d, Primary? %d, Physical Primary? %d \n",
                 iMother, pdg, iParent, aodprimary->IsPrimary(), aodprimary->IsPhysicalPrimary());
        }
        
      }
      
      //Overlapped pi0 (or eta, there will be very few), get the meson
      if(GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCPi0) || 
         GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCEta)){
        if(GetDebug() > 1 ) printf("AliAnaCalorimeterQA::ClusterHistograms() - Overlapped Meson decay!, Find it: PDG %d, mom %d \n",pdg, iMother);
        while(pdg != 111 && pdg != 221){
          
          iMother    = iParent;
          aodprimary = (AliAODMCParticle*)(GetReader()->GetAODMCParticles(0))->At(iMother);
          status     = aodprimary->IsPrimary();
          iParent    = aodprimary->GetMother();
          pdg        = TMath::Abs(aodprimary->GetPdgCode());
          
          if(GetDebug() > 1 ) printf("\t pdg %d, index %d\n",pdg, iMother);
          
          if(iMother==-1) {
            printf("AliAnaCalorimeterQA::ClusterHistograms() - Tagged as Overlapped photon but meson not found, why?\n");
            //break;
          }
        }	
        
        if(GetDebug() > 2 ) printf("AliAnaCalorimeterQA::ClusterHistograms() - Overlapped %s decay, label %d \n", 
                                   aodprimary->GetName(),iMother);
      }	
      
      status = aodprimary->IsPrimary();
      eMC    = aodprimary->E();
      ptMC   = aodprimary->Pt();
      phiMC  = aodprimary->Phi();
      etaMC  = aodprimary->Eta();
      pdg    = TMath::Abs(aodprimary->GetPdgCode());
      charge = aodprimary->Charge();
      
    }
    
    //Float_t vz = primary->Vz();
    Float_t rVMC = TMath::Sqrt(vxMC*vxMC + vyMC*vyMC);
    if((pdg == 22 || TMath::Abs(pdg)==11) && status!=1) {
      fhEMVxyz   ->Fill(vxMC,vyMC);//,vz);
      fhEMR      ->Fill(e,rVMC);
    }
    
    //printf("reco e %f, pt %f, phi %f, eta %f \n", e, pt, phi, eta);
    //printf("prim e %f, pt %f, phi %f, eta %f \n", eMC,ptMC,phiMC ,etaMC );
    //printf("vertex: vx %f, vy %f, vz %f, r %f \n", vxMC, vyMC, vz, r);
    
    
    //Overlapped pi0 (or eta, there will be very few)
    if(GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCPi0)){
      fhRecoMCE  [mcPi0][(nTracksMatched>0)]     ->Fill(e,eMC);	
      if(e > 0.5 && eMC > 0.5) fhRecoMCEta[mcPi0][(nTracksMatched>0)]->Fill(eta,etaMC);	
      if(e > 0.5 && eMC > 0.5) fhRecoMCPhi[mcPi0][(nTracksMatched>0)]->Fill(phi,phiMC);
      fhRecoMCDeltaE  [mcPi0][(nTracksMatched>0)]->Fill(e,eMC-e);
      fhRecoMCDeltaPhi[mcPi0][(nTracksMatched>0)]->Fill(e,phiMC-phi);
      fhRecoMCDeltaEta[mcPi0][(nTracksMatched>0)]->Fill(e,etaMC-eta);
    }//Overlapped pizero decay
    else     if(GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCEta)){
      fhRecoMCE  [mcEta][(nTracksMatched>0)]     ->Fill(e,eMC);	
      if(e > 0.5 && eMC > 0.5) fhRecoMCEta[mcEta][(nTracksMatched>0)]->Fill(eta,etaMC);	
      if(e > 0.5 && eMC > 0.5) fhRecoMCPhi[mcEta][(nTracksMatched>0)]->Fill(phi,phiMC);
      fhRecoMCDeltaE  [mcEta][(nTracksMatched>0)]->Fill(e,eMC-e);
      fhRecoMCDeltaPhi[mcEta][(nTracksMatched>0)]->Fill(e,phiMC-phi);
      fhRecoMCDeltaEta[mcEta][(nTracksMatched>0)]->Fill(e,etaMC-eta);
    }//Overlapped eta decay
    else if(GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCPhoton)){
      fhRecoMCE  [mcPhoton][(nTracksMatched>0)]     ->Fill(e,eMC);	
      if(e > 0.5 && eMC > 0.5) fhRecoMCEta[mcPhoton][(nTracksMatched>0)]->Fill(eta,etaMC);	
      if(e > 0.5 && eMC > 0.5) fhRecoMCPhi[mcPhoton][(nTracksMatched>0)]->Fill(phi,phiMC);
      fhRecoMCDeltaE  [mcPhoton][(nTracksMatched>0)]->Fill(e,eMC-e);
      fhRecoMCDeltaPhi[mcPhoton][(nTracksMatched>0)]->Fill(e,phiMC-phi);
      fhRecoMCDeltaEta[mcPhoton][(nTracksMatched>0)]->Fill(e,etaMC-eta);      
     }//photon
    else if(GetMCAnalysisUtils()->CheckTagBit(tag, AliMCAnalysisUtils::kMCElectron)){
      fhRecoMCE  [mcElectron][(nTracksMatched>0)]     ->Fill(e,eMC);	
      if(e > 0.5 && eMC > 0.5) fhRecoMCEta[mcElectron][(nTracksMatched>0)]->Fill(eta,etaMC);	
      if(e > 0.5 && eMC > 0.5) fhRecoMCPhi[mcElectron][(nTracksMatched>0)]->Fill(phi,phiMC);
      fhRecoMCDeltaE  [mcElectron][(nTracksMatched>0)]->Fill(e,eMC-e);
      fhRecoMCDeltaPhi[mcElectron][(nTracksMatched>0)]->Fill(e,phiMC-phi);
      fhRecoMCDeltaEta[mcElectron][(nTracksMatched>0)]->Fill(e,etaMC-eta);
      fhEMVxyz   ->Fill(vxMC,vyMC);//,vz);
      fhEMR      ->Fill(e,rVMC);
    }
    else if(charge == 0){
      fhRecoMCE  [mcNeHadron][(nTracksMatched>0)]     ->Fill(e,eMC);	
      if(e > 0.5 && eMC > 0.5) fhRecoMCEta[mcNeHadron][(nTracksMatched>0)]->Fill(eta,etaMC);	
      if(e > 0.5 && eMC > 0.5) fhRecoMCPhi[mcNeHadron][(nTracksMatched>0)]->Fill(phi,phiMC);
      fhRecoMCDeltaE  [mcNeHadron][(nTracksMatched>0)]->Fill(e,eMC-e);
      fhRecoMCDeltaPhi[mcNeHadron][(nTracksMatched>0)]->Fill(e,phiMC-phi);
      fhRecoMCDeltaEta[mcNeHadron][(nTracksMatched>0)]->Fill(e,etaMC-eta);      
      fhHaVxyz     ->Fill(vxMC,vyMC);//,vz);
      fhHaR        ->Fill(e,rVMC);
    }
    else if(charge!=0){
      fhRecoMCE  [mcChHadron][(nTracksMatched>0)]     ->Fill(e,eMC);	
      if(e > 0.5 && eMC > 0.5) fhRecoMCEta[mcChHadron][(nTracksMatched>0)]->Fill(eta,etaMC);	
      if(e > 0.5 && eMC > 0.5) fhRecoMCPhi[mcChHadron][(nTracksMatched>0)]->Fill(phi,phiMC);
      fhRecoMCDeltaE  [mcChHadron][(nTracksMatched>0)]->Fill(e,eMC-e);
      fhRecoMCDeltaPhi[mcChHadron][(nTracksMatched>0)]->Fill(e,phiMC-phi);
      fhRecoMCDeltaEta[mcChHadron][(nTracksMatched>0)]->Fill(e,etaMC-eta);     
      fhHaVxyz     ->Fill(vxMC,vyMC);//,vz);
      fhHaR        ->Fill(e,rVMC);
    }
  }//Work with MC
  
  //Match tracks and clusters
  //To be Modified in case of AODs
  
  if( nTracksMatched > 0 &&  fFillAllTMHisto){
    if(fFillAllTH12 && fFillAllTMHisto){
      fhECharged      ->Fill(e);	
      fhPtCharged     ->Fill(pt);
      fhPhiCharged    ->Fill(phi);
      fhEtaCharged    ->Fill(eta);
    }
    
    if(fFillAllTMHisto){
      if(fFillAllTH3)fhEtaPhiECharged->Fill(eta,phi,e);		
      if((fCalorimeter=="EMCAL" && GetReader()->GetEMCALPtMin() < 0.3) ||
         (fCalorimeter=="PHOS"  && GetReader()->GetPHOSPtMin()  < 0.3))   fhNCellsPerClusterMIPCharged->Fill(e, nCaloCellsPerCluster);
    }
    //printf("track index %d ntracks %d\n", esd->GetNumberOfTracks());
    
//    //Study the track and matched cluster if track exists.
//    if(!track) return;
//    Double_t emcpos[3] = {0.,0.,0.};
//    Double_t emcmom[3] = {0.,0.,0.};
//    Double_t radius    = 441.0; //[cm] EMCAL radius +13cm
//    Double_t bfield    = 0.;
//    Double_t tphi      = 0;
//    Double_t teta      = 0;
//    Double_t tmom      = 0;
//    Double_t tpt       = 0;
//    Double_t tmom2     = 0;
//    Double_t tpcSignal = 0;
//    Bool_t okpos = kFALSE;
//    Bool_t okmom = kFALSE;
//    Bool_t okout = kFALSE;
//    Int_t nITS   = 0;
//    Int_t nTPC   = 0;
//    
//    //In case of ESDs get the parameters in this way
//    if(GetReader()->GetDataType()==AliCaloTrackReader::kESD) {
//      if (track->GetOuterParam() ) {
//        okout = kTRUE;
//        
//        bfield = GetReader()->GetInputEvent()->GetMagneticField();
//        okpos = track->GetOuterParam()->GetXYZAt(radius,bfield,emcpos);
//        okmom = track->GetOuterParam()->GetPxPyPzAt(radius,bfield,emcmom);
//        if(!(okpos && okmom)) return;
//        
//        TVector3 position(emcpos[0],emcpos[1],emcpos[2]);
//        TVector3 momentum(emcmom[0],emcmom[1],emcmom[2]);
//        tphi = position.Phi();
//        teta = position.Eta();
//        tmom = momentum.Mag();
//        
//        tpt       = track->Pt();
//        tmom2     = track->P();
//        tpcSignal = track->GetTPCsignal();
//        
//        nITS = track->GetNcls(0);
//        nTPC = track->GetNcls(1);
//      }//Outer param available 
//    }// ESDs
//    else if(GetReader()->GetDataType()==AliCaloTrackReader::kAOD) {
//      AliAODPid* pid = (AliAODPid*) ((AliAODTrack *) track)->GetDetPid();
//      if (pid) {
//        okout = kTRUE;
//        pid->GetEMCALPosition(emcpos);
//        pid->GetEMCALMomentum(emcmom);	
//        
//        TVector3 position(emcpos[0],emcpos[1],emcpos[2]);
//        TVector3 momentum(emcmom[0],emcmom[1],emcmom[2]);
//        tphi = position.Phi();
//        teta = position.Eta();
//        tmom = momentum.Mag();
//        
//        tpt       = track->Pt();
//        tmom2     = track->P();
//        tpcSignal = pid->GetTPCsignal();
//        
//       }//pid 
//    }//AODs
//		
//    if(okout){
//      printf("okout\n");
//      Double_t deta = teta - eta;
//      Double_t dphi = tphi - phi;
//      if(dphi > TMath::Pi()) dphi -= 2*TMath::Pi();
//      if(dphi < -TMath::Pi()) dphi += 2*TMath::Pi();
//      Double_t dR = sqrt(dphi*dphi + deta*deta);
//			
//      Double_t pOverE = tmom/e;
//			
//      fh1pOverE->Fill(tpt, pOverE);
//      if(dR < 0.02) fh1pOverER02->Fill(tpt,pOverE);
//			
//      fh1dR->Fill(dR);
//      fh2MatchdEdx->Fill(tmom2,tpcSignal);
//			
//      if(IsDataMC() && primary){ 
//        Int_t pdg = primary->GetPdgCode();
//        Double_t  charge = TDatabasePDG::Instance()->GetParticle(pdg)->Charge();
//				
//        if(TMath::Abs(pdg) == 11){
//          fhMCEle1pOverE->Fill(tpt,pOverE);
//          fhMCEle1dR->Fill(dR);
//          fhMCEle2MatchdEdx->Fill(tmom2,tpcSignal);		
//          if(dR < 0.02) fhMCEle1pOverER02->Fill(tpt,pOverE);
//        }
//        else if(charge!=0){
//          fhMCChHad1pOverE->Fill(tpt,pOverE);
//          fhMCChHad1dR->Fill(dR);
//          fhMCChHad2MatchdEdx->Fill(tmom2,tpcSignal);	
//          if(dR < 0.02) fhMCChHad1pOverER02->Fill(tpt,pOverE);
//        }
//        else if(charge == 0){
//          fhMCNeutral1pOverE->Fill(tpt,pOverE);
//          fhMCNeutral1dR->Fill(dR);
//          fhMCNeutral2MatchdEdx->Fill(tmom2,tpcSignal);	
//          if(dR < 0.02) fhMCNeutral1pOverER02->Fill(tpt,pOverE);
//        }
//      }//DataMC
//      
//      if(dR < 0.02 && pOverE > 0.5 && pOverE < 1.5
//         && nCaloCellsPerCluster > 1 && nITS > 3 && nTPC > 20) {
//        fh2EledEdx->Fill(tmom2,tpcSignal);
//      }
//    }
//    else{//no ESD external param or AODPid
//
//      if(GetDebug() >= 0) printf("No ESD external param or AliAODPid \n");
//      
//    }//No out params
  }//matched clusters with tracks
  
}// Clusters


//__________________________________
void AliAnaCalorimeterQA::Correlate(){
  // Correlate information from PHOS and EMCAL and with V0 and track multiplicity
  
  //Clusters 
  TObjArray * caloClustersEMCAL = GetEMCALClusters();
  TObjArray * caloClustersPHOS  = GetPHOSClusters();
  
  Int_t nclEMCAL = caloClustersEMCAL->GetEntriesFast();
  Int_t nclPHOS  = caloClustersPHOS ->GetEntriesFast();
  
  Float_t sumClusterEnergyEMCAL = 0;
  Float_t sumClusterEnergyPHOS  = 0;
  Int_t iclus = 0;
  for(iclus = 0 ; iclus <  caloClustersEMCAL->GetEntriesFast() ; iclus++) 
    sumClusterEnergyEMCAL += ((AliVCluster*)caloClustersEMCAL->At(iclus))->E();
  for(iclus = 0 ; iclus <  caloClustersPHOS->GetEntriesFast(); iclus++) 
    sumClusterEnergyPHOS += ((AliVCluster*)caloClustersPHOS->At(iclus))->E();
  
  
  //Cells
  
  AliVCaloCells * cellsEMCAL = GetEMCALCells();
  AliVCaloCells * cellsPHOS  = GetPHOSCells();
  
  Int_t ncellsEMCAL = cellsEMCAL->GetNumberOfCells();
  Int_t ncellsPHOS  = cellsPHOS ->GetNumberOfCells();
  
  Float_t sumCellEnergyEMCAL = 0;
  Float_t sumCellEnergyPHOS  = 0;
  Int_t icell = 0;
  for(icell = 0 ; icell < cellsEMCAL->GetNumberOfCells()  ; icell++) 
    sumCellEnergyEMCAL += cellsEMCAL->GetAmplitude(icell);
  for(icell = 0 ; icell <  cellsPHOS->GetNumberOfCells(); icell++) 
    sumCellEnergyPHOS += cellsPHOS->GetAmplitude(icell);
  
  
  //Fill Histograms
  fhCaloCorrNClusters->Fill(nclEMCAL,nclPHOS);
  fhCaloCorrEClusters->Fill(sumClusterEnergyEMCAL,sumClusterEnergyPHOS);
  fhCaloCorrNCells   ->Fill(ncellsEMCAL,ncellsPHOS);
  fhCaloCorrECells   ->Fill(sumCellEnergyEMCAL,sumCellEnergyPHOS);
  
  Int_t v0S = GetV0Signal(0)+GetV0Signal(1);
  Int_t v0M = GetV0Multiplicity(0)+GetV0Multiplicity(1);
  Int_t trM = GetTrackMultiplicity();
  if(fCalorimeter=="PHOS"){
    fhCaloV0MCorrNClusters   ->Fill(v0M,nclPHOS);
    fhCaloV0MCorrEClusters   ->Fill(v0M,sumClusterEnergyPHOS);
    fhCaloV0MCorrNCells      ->Fill(v0M,ncellsPHOS);
    fhCaloV0MCorrECells      ->Fill(v0M,sumCellEnergyPHOS);
    
    fhCaloV0SCorrNClusters   ->Fill(v0S,nclPHOS);
    fhCaloV0SCorrEClusters   ->Fill(v0S,sumClusterEnergyPHOS);
    fhCaloV0SCorrNCells      ->Fill(v0S,ncellsPHOS);
    fhCaloV0SCorrECells      ->Fill(v0S,sumCellEnergyPHOS);
    
    fhCaloTrackMCorrNClusters->Fill(trM,nclPHOS);
    fhCaloTrackMCorrEClusters->Fill(trM,sumClusterEnergyPHOS);    
    fhCaloTrackMCorrNCells   ->Fill(trM,ncellsPHOS);
    fhCaloTrackMCorrECells   ->Fill(trM,sumCellEnergyPHOS);
  }
  else{
    fhCaloV0MCorrNClusters   ->Fill(v0M,nclEMCAL);
    fhCaloV0MCorrEClusters   ->Fill(v0M,sumClusterEnergyEMCAL);
    fhCaloV0MCorrNCells      ->Fill(v0M,ncellsEMCAL);
    fhCaloV0MCorrECells      ->Fill(v0M,sumCellEnergyEMCAL);
    
    fhCaloV0SCorrNClusters   ->Fill(v0S,nclEMCAL);
    fhCaloV0SCorrEClusters   ->Fill(v0S,sumClusterEnergyEMCAL);
    fhCaloV0SCorrNCells      ->Fill(v0S,ncellsEMCAL);
    fhCaloV0SCorrECells      ->Fill(v0S,sumCellEnergyEMCAL);
    
    fhCaloTrackMCorrNClusters->Fill(trM,nclEMCAL);
    fhCaloTrackMCorrEClusters->Fill(trM,sumClusterEnergyEMCAL);    
    fhCaloTrackMCorrNCells   ->Fill(trM,ncellsEMCAL);
    fhCaloTrackMCorrECells   ->Fill(trM,sumCellEnergyEMCAL);
  }
  
  if(GetDebug() > 0 )
  {
    printf("AliAnaCalorimeterQA::Correlate(): \n");
    printf("\t EMCAL: N cells %d, N clusters  %d, summed E cells %f, summed E clusters %f \n",
           ncellsEMCAL,nclEMCAL, sumCellEnergyEMCAL,sumClusterEnergyEMCAL);
    printf("\t PHOS : N cells %d, N clusters  %d, summed E cells %f, summed E clusters %f \n",
           ncellsPHOS,nclPHOS,sumCellEnergyPHOS,sumClusterEnergyPHOS);
    printf("\t V0 : Signal %d, Multiplicity  %d, Track Multiplicity %d \n", v0S,v0M,trM);
  }
}

//______________________________________________________________________________
void AliAnaCalorimeterQA::FillCellPositionHistograms(const Int_t nCaloCellsPerCluster,const UShort_t * indexList,
                                                     const Float_t pos[3], const Float_t clEnergy){
  // Fill histograms releated to cell position
  
  //Loop on cluster cells
  for (Int_t ipos = 0; ipos < nCaloCellsPerCluster; ipos++) {
    
    //	printf("Index %d\n",ipos);
    Int_t absId  = indexList[ipos]; 
    
    //Get position of cell compare to cluster
    
    if(fCalorimeter=="EMCAL" && GetCaloUtils()->IsEMCALGeoMatrixSet()){
      
      Double_t cellpos[] = {0, 0, 0};
      GetEMCALGeometry()->GetGlobal(absId, cellpos);
      
      fhDeltaCellClusterXNCells->Fill(pos[0]-cellpos[0],nCaloCellsPerCluster) ; 
      fhDeltaCellClusterYNCells->Fill(pos[1]-cellpos[1],nCaloCellsPerCluster) ; 
      fhDeltaCellClusterZNCells->Fill(pos[2]-cellpos[2],nCaloCellsPerCluster) ;
      
      fhDeltaCellClusterXE->Fill(pos[0]-cellpos[0],clEnergy)  ; 
      fhDeltaCellClusterYE->Fill(pos[1]-cellpos[1],clEnergy)  ; 
      fhDeltaCellClusterZE->Fill(pos[2]-cellpos[2],clEnergy)  ; 
      
      Float_t r     = TMath::Sqrt(pos[0]    *pos[0]     + pos[1]    * pos[1]    );
      Float_t rcell = TMath::Sqrt(cellpos[0]*cellpos[0] + cellpos[1]* cellpos[1]);
      
      fhDeltaCellClusterRNCells->Fill(r-rcell, nCaloCellsPerCluster) ; 
      fhDeltaCellClusterRE     ->Fill(r-rcell, clEnergy)  ; 			
      
    }//EMCAL and its matrices are available
    else if(fCalorimeter=="PHOS" && GetCaloUtils()->IsPHOSGeoMatrixSet()){
      TVector3 xyz;
      Int_t relId[4], module;
      Float_t xCell, zCell;
      
      GetPHOSGeometry()->AbsToRelNumbering(absId,relId);
      module = relId[0];
      GetPHOSGeometry()->RelPosInModule(relId,xCell,zCell);
      GetPHOSGeometry()->Local2Global(module,xCell,zCell,xyz);
      
      fhDeltaCellClusterXNCells->Fill(pos[0]-xyz.X(),nCaloCellsPerCluster) ; 
      fhDeltaCellClusterYNCells->Fill(pos[1]-xyz.Y(),nCaloCellsPerCluster) ; 
      fhDeltaCellClusterZNCells->Fill(pos[2]-xyz.Z(),nCaloCellsPerCluster) ;
      
      fhDeltaCellClusterXE->Fill(pos[0]-xyz.X(),clEnergy)  ; 
      fhDeltaCellClusterYE->Fill(pos[1]-xyz.Y(),clEnergy)  ; 
      fhDeltaCellClusterZE->Fill(pos[2]-xyz.Z(),clEnergy)  ; 
      
      Float_t r     = TMath::Sqrt(pos[0]  * pos[0]  + pos[1]  * pos[1] );
      Float_t rcell = TMath::Sqrt(xyz.X() * xyz.X() + xyz.Y() * xyz.Y());
      
      fhDeltaCellClusterRNCells->Fill(r-rcell, nCaloCellsPerCluster) ; 
      fhDeltaCellClusterRE     ->Fill(r-rcell, clEnergy)  ; 
      
    }//PHOS and its matrices are available
  }// cluster cell loop
}//Fill all position histograms


//______________________________________________________________________________
void AliAnaCalorimeterQA::MCHistograms(const TLorentzVector mom, const Int_t pdg){
  //Fill pure monte carlo related histograms
	
  Float_t eMC    = mom.E();
  Float_t phiMC  = mom.Phi();
  if(phiMC < 0) 
    phiMC  += TMath::TwoPi();
  Float_t etaMC  = mom.Eta();
  
  if (TMath::Abs(etaMC) > 1) return;
  
  Bool_t in = kFALSE;
  
  //Rough stimate of acceptance for pi0, Eta and electrons
  if(fCalorimeter == "PHOS"){
      
    if(GetFiducialCut()->IsInFiducialCut(mom,fCalorimeter)) 
      in = kTRUE ;
    if(GetDebug() > 2) printf("AliAnaCalorimeterQA::MCHistograms() - In %s fiducial cut acceptance? %d\n",fCalorimeter.Data(),in);
    
  }	   
  else if(fCalorimeter == "EMCAL" && GetCaloUtils()->IsEMCALGeoMatrixSet()){
    if(GetEMCALGeometry()){
      
      Int_t absID=0;
      GetEMCALGeometry()->GetAbsCellIdFromEtaPhi(mom.Eta(),mom.Phi(),absID);
      
      if( absID >= 0) 
        in = kTRUE;
      
      if(GetDebug() > 2) printf("AliAnaCalorimeterQA::MCHistograms() - In %s Real acceptance? %d\n",fCalorimeter.Data(),in);
    }
    else{
      if(GetFiducialCut()->IsInFiducialCut(mom,fCalorimeter)) 
        in = kTRUE ;
      if(GetDebug() > 2) printf("AliAnaCalorimeterQA::MCHistograms() - In %s fiducial cut acceptance? %d\n",fCalorimeter.Data(),in);
    }
  }	  
  
  if (pdg==22) {
    fhGenMCE[mcPhoton]    ->Fill(eMC);
    if(eMC > 0.5) fhGenMCEtaPhi[mcPhoton]->Fill(etaMC,phiMC);
    if(in){
      fhGenMCAccE[mcPhoton]    ->Fill(eMC);
      if(eMC > 0.5) fhGenMCAccEtaPhi[mcPhoton]->Fill(etaMC,phiMC);	
    }
  }
  else if (pdg==111) {
    fhGenMCE[mcPi0]    ->Fill(eMC);
    if(eMC > 0.5) fhGenMCEtaPhi[mcPi0]->Fill(etaMC,phiMC);
    if(in){
      fhGenMCAccE[mcPi0]    ->Fill(eMC);
      if(eMC > 0.5) fhGenMCAccEtaPhi[mcPi0]->Fill(etaMC,phiMC);	
    }
  }
  else if (pdg==221) {
    fhGenMCE[mcEta]    ->Fill(eMC);
    if(eMC > 0.5) fhGenMCEtaPhi[mcEta]->Fill(etaMC,phiMC);
    if(in){
      fhGenMCAccE[mcEta]    ->Fill(eMC);
      if(eMC > 0.5) fhGenMCAccEtaPhi[mcEta]->Fill(etaMC,phiMC);	
    }    
  }
  else if (TMath::Abs(pdg)==11) {
    fhGenMCE[mcElectron]    ->Fill(eMC);
    if(eMC > 0.5) fhGenMCEtaPhi[mcElectron]->Fill(etaMC,phiMC);
    if(in){
      fhGenMCAccE[mcElectron]    ->Fill(eMC);
      if(eMC > 0.5) fhGenMCAccEtaPhi[mcElectron]->Fill(etaMC,phiMC);	
    }
  }	
}

