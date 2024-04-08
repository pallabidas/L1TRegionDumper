// -*- C++ -*-
//
// Package:    L1Trigger/L1TRegionDumper
// Class:      L1TRegionDumper
//
/**\class L1TRegionDumper L1TRegionDumper.cc L1Trigger/L1TRegionDumper/plugins/L1TRegionDumper.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Sridhara Dasu
//         Created:  Tue, 11 Jan 2022 22:31:24 GMT
//
//

// system include files
#include <memory>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

// user include files

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/L1CaloTrigger/interface/L1CaloCollections.h"
#include "DataFormats/L1Trigger/interface/L1JetParticle.h"
#include "DataFormats/L1Trigger/interface/L1JetParticleFwd.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TFile.h"
#include "TTree.h"
#include "TLorentzVector.h"
#include <vector>
#include "DataFormats/Math/interface/LorentzVector.h"

#include "ap_fixed.h"
#include "hls4ml/emulator.h"

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.

namespace l1tcalo {
  constexpr uint32_t RegionETMask{0x000003FF};
  constexpr uint32_t RegionEGVeto{0x00000400};
  constexpr uint32_t RegionTauVeto{0x00000800};
  constexpr uint32_t HitTowerBits{0x0000F000};
  constexpr uint32_t RegionNoBits{0x000F0000};
  constexpr uint32_t CardNoBits{0x00700000};
  constexpr uint32_t CrateNoBits{0x01800000};
  constexpr uint32_t NegEtaBit{0x80000000};
  constexpr uint32_t LocationBits{0xFFFFF000};
  constexpr uint32_t LocationShift{12};
  constexpr uint32_t RegionNoShift{16};
  constexpr uint32_t CardNoShift{20};
  constexpr uint32_t CrateNoShift{23};
}  // namespace l1tcalo

bool compareByPt (TLorentzVector i, TLorentzVector j) { return(i.Pt() >= j.Pt()); };

const ap_uint<8> ieta_lut[2][42] = {
    {0x00, 0x01, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0D, 0x0F, 0x11, 0x13, 0x15,
     0x17, 0x19, 0x1B, 0x1D, 0x1F, 0x21, 0x23, 0x25, 0x27, 0x29, 0x2B,
     0x2D, 0x30, 0x33, 0x37, 0x3B, 0x40, 0x00, 0x46, 0x4A, 0x4E, 0x52,
     0x56, 0x5A, 0x5E, 0x62, 0x66, 0x6A, 0x6F, 0x72},
    {0xFF, 0xFE, 0xFC, 0xFA, 0xF8, 0xF6, 0xF4, 0xF2, 0xF0, 0xEE, 0xEC, 0xEA,
     0xE8, 0xE6, 0xE4, 0xE2, 0xE0, 0xDE, 0xDC, 0xDA, 0xD8, 0xD6, 0xD4,
     0xD2, 0xCF, 0xCC, 0xC8, 0xC4, 0xBF, 0x00, 0xB9, 0xB5, 0xB1, 0xAD,
     0xA9, 0xA5, 0xA1, 0x9D, 0x99, 0x95, 0x90, 0x8D}};

const ap_uint<8> iphi_lut[72] = {
    0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16,
    0x18, 0x1A, 0x1C, 0x1E, 0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2E,
    0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3E, 0x40, 0x42, 0x44, 0x46,
    0x48, 0x4A, 0x4C, 0x4E, 0x50, 0x52, 0x54, 0x56, 0x58, 0x5A, 0x5C, 0x5E,
    0x60, 0x62, 0x64, 0x66, 0x68, 0x6A, 0x6C, 0x6E, 0x70, 0x72, 0x74, 0x76,
    0x78, 0x7A, 0x7C, 0x7E, 0x80, 0x82, 0x84, 0x86, 0x88, 0x8A, 0x8C, 0x8E};

int eta_converter(float eta_value) {
  float towerEta = 99;
  std::array<float, 42> twrEtaValues = {{0}};
  twrEtaValues[0] = 0;
  for (unsigned int i = 0; i < 20; i++) {
    twrEtaValues[i + 1] = 0.0436 + i * 0.0872;
  }
  twrEtaValues[21] = 1.785;
  twrEtaValues[22] = 1.880;
  twrEtaValues[23] = 1.9865;
  twrEtaValues[24] = 2.1075;
  twrEtaValues[25] = 2.247;
  twrEtaValues[26] = 2.411;
  twrEtaValues[27] = 2.575;
  twrEtaValues[28] = 2.825;
  twrEtaValues[29] = 999.;
  twrEtaValues[30] = (3.15 + 2.98) / 2.;
  twrEtaValues[31] = (3.33 + 3.15) / 2.;
  twrEtaValues[32] = (3.50 + 3.33) / 2.;
  twrEtaValues[33] = (3.68 + 3.50) / 2.;
  twrEtaValues[34] = (3.68 + 3.85) / 2.;
  twrEtaValues[35] = (3.85 + 4.03) / 2.;
  twrEtaValues[36] = (4.03 + 4.20) / 2.;
  twrEtaValues[37] = (4.20 + 4.38) / 2.;
  twrEtaValues[38] = (4.74 + 4.38 * 3) / 4.;
  twrEtaValues[39] = (4.38 + 4.74 * 3) / 4.;
  twrEtaValues[40] = (5.21 + 4.74 * 3) / 4.;
  twrEtaValues[41] = (4.74 + 5.21 * 3) / 4.;
  for(int i = 0; i < 42; i++){
    if(abs(eta_value) == twrEtaValues[i]) towerEta = i;
  }
  //if(eta_value < 0) return -int(towerEta);
  //else return int(towerEta);
  return int(towerEta);
}

int phi_converter(float phi_value) {
  float towerPhi = 99;
  if(phi_value < 0) towerPhi = 73 + floor(phi_value/0.0872);
  else towerPhi = floor(phi_value/0.0872) + 1;
  return int(towerPhi);
}

int pt_converter(float pt_value) {
  float jetEt = pt_value/(0.5*1.5);
  return int(jetEt);
}

class L1TRegionDumper : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit L1TRegionDumper(const edm::ParameterSet&);
  ~L1TRegionDumper() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  int readCount_;

  // ----------member data ---------------------------
  
  edm::EDGetTokenT<std::vector <L1CaloRegion> > regionsToken_;
  edm::EDGetTokenT< l1extra::L1JetParticleCollection > boostedJetToken_;
  edm::EDGetTokenT< float > anomalyToken_;
#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
  edm::ESGetToken<SetupData, SetupRecord> setupToken_;
#endif

  edm::Service<TFileService> fs;

  TTree* triggerTree;
  std::vector<uint16_t> cregions; 
  float anomalyScore;

};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
L1TRegionDumper::L1TRegionDumper(const edm::ParameterSet& iConfig)
  : readCount_(0),
  regionsToken_(consumes<std::vector <L1CaloRegion> >(iConfig.getUntrackedParameter<edm::InputTag>("UCTRegion"))),
  boostedJetToken_(consumes< l1extra::L1JetParticleCollection >(iConfig.getParameter<edm::InputTag>("boostedJetCollection"))),
  anomalyToken_(consumes< float >(iConfig.getParameter<edm::InputTag>("scoreSource"))) {
  //now do what ever initialization is needed
  triggerTree = fs->make<TTree>("triggerTree", "triggerTree");
  triggerTree->Branch("cregions",     &cregions);
  triggerTree->Branch("anomalyScore", &anomalyScore);
}

L1TRegionDumper::~L1TRegionDumper() {
}

//
// member functions
//

// ------------ method called for each event  ------------
void L1TRegionDumper::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;
  cregions.clear();
  uint16_t regionColl[252];
  uint16_t regionColl_input[14][18];
  int count=0;
  // Add default non-zero region indices to be consistent with firmware: regionIndex = 0 corresponding to calo coordinates {71, 25, 1}
  int default_eta = 25;
  int default_phi = 71;
  //std::cout<<"readCount_: "<<readCount_<<std::endl;

  for (const auto& region : iEvent.get(regionsToken_)) {
    uint32_t ieta = region.id().ieta() - 4; // Subtract off the offset for HF
    uint32_t iphi = region.id().iphi();
    uint16_t regionSummary = region.raw();
    //uint32_t et = region.et();
    //uint32_t location = ((regionSummary & l1tcalo::LocationBits) >> l1tcalo::LocationShift);
    //bool eleBit = !((l1tcalo::RegionEGVeto & regionSummary) == l1tcalo::RegionEGVeto);
    //bool tauBit = !((l1tcalo::RegionTauVeto & regionSummary) == l1tcalo::RegionTauVeto);
    //uint32_t hitTowerLocation = (location & 0xF);
    regionColl_input[ieta][iphi] = regionSummary;
    // if(iphi==0) {
    //   std::cout<<"iPhi 0, iEta "<<ieta<<" Region Hex: "<<std::hex<<std::setfill('0')<<std::setw(4)<<regionSummary<<std::endl;
    // }
    //std::cout<<"count: "<<count<<"\t"<<"et: "<<et<<"\t"<<"ieta : "<<ieta<<"\t"<<"iphi: "<<iphi<<"\t"<<"rloc_eta: "<<((0xFFFF & regionSummary) >> 14)<<"\t"<<"rloc_phi: "<<((0x3FFF & regionSummary) >> 12)<<"\t"<<"location: "<<hitTowerLocation<<"\t"<<"eleBit: "<<eleBit<<"\t"<<"tauBit: "<<tauBit<<std::endl;
    if(ieta==0 && iphi==0) { default_eta += ((0xFFFF & regionSummary) >> 14); default_phi += ((0x3FFF & regionSummary) >> 12); }
    if(default_phi > 71) default_phi -= 72;
    count++;
    //For negative eta: most significant bit is Region 0 (ieta=6) and least significant is Region 6 (ieta=0)
    //For positive eta: most significant bit is Region 6 (ieta=13) and least significant is Region 0 (ieta=7)
    // ieta: 0 1 2 3 4 5 6   7 8 9 10 11 12 13 (this is the way we want to read the regions in firmware)
    //This is why we need to invert the region ordering here, because maketv puts Region 6 at least significant bit position by construction
    //If we make ieta=7 at Region 6, it will be put in the least significant bit position and ieta=13 will be put at most significant bit position

    //BRAM 1
    if(iphi==15 && ieta==13) regionColl[0] = regionSummary;
    if(iphi==15 && ieta==12) regionColl[1] = regionSummary;
    if(iphi==15 && ieta==11) regionColl[2] = regionSummary;
    if(iphi==15 && ieta==10) regionColl[3] = regionSummary;
    if(iphi==15 && ieta==9) regionColl[4] = regionSummary;
    if(iphi==15 && ieta==8) regionColl[5] = regionSummary;
    if(iphi==15 && ieta==7) regionColl[6] = regionSummary;
    //BRAM 2
    if(iphi==15 && ieta==6) regionColl[7] = regionSummary;
    if(iphi==15 && ieta==5) regionColl[8] = regionSummary;
    if(iphi==15 && ieta==4) regionColl[9] = regionSummary;
    if(iphi==15 && ieta==3) regionColl[10] = regionSummary;
    if(iphi==15 && ieta==2) regionColl[11] = regionSummary;
    if(iphi==15 && ieta==1) regionColl[12] = regionSummary;
    if(iphi==15 && ieta==0) regionColl[13] = regionSummary;
    //BRAM 3
    if(iphi==0 && ieta==6) regionColl[14] = regionSummary;
    if(iphi==0 && ieta==5) regionColl[15] = regionSummary;
    if(iphi==0 && ieta==4) regionColl[16] = regionSummary;
    if(iphi==0 && ieta==3) regionColl[17] = regionSummary;
    if(iphi==0 && ieta==2) regionColl[18] = regionSummary;
    if(iphi==0 && ieta==1) regionColl[19] = regionSummary;
    if(iphi==0 && ieta==0) regionColl[20] = regionSummary;
    //BRAM 4
    if(iphi==0 && ieta==13) regionColl[21] = regionSummary;
    if(iphi==0 && ieta==12) regionColl[22] = regionSummary;
    if(iphi==0 && ieta==11) regionColl[23] = regionSummary;
    if(iphi==0 && ieta==10) regionColl[24] = regionSummary;
    if(iphi==0 && ieta==9) regionColl[25] = regionSummary;
    if(iphi==0 && ieta==8) regionColl[26] = regionSummary;
    if(iphi==0 && ieta==7) regionColl[27] = regionSummary;
    //BRAM 5
    if(iphi==1 && ieta==6) regionColl[28] = regionSummary;
    if(iphi==1 && ieta==5) regionColl[29] = regionSummary;
    if(iphi==1 && ieta==4) regionColl[30] = regionSummary;
    if(iphi==1 && ieta==3) regionColl[31] = regionSummary;
    if(iphi==1 && ieta==2) regionColl[32] = regionSummary;
    if(iphi==1 && ieta==1) regionColl[33] = regionSummary;
    if(iphi==1 && ieta==0) regionColl[34] = regionSummary;
    //BRAM 6
    if(iphi==1 && ieta==13) regionColl[35] = regionSummary;
    if(iphi==1 && ieta==12) regionColl[36] = regionSummary;
    if(iphi==1 && ieta==11) regionColl[37] = regionSummary;
    if(iphi==1 && ieta==10) regionColl[38] = regionSummary;
    if(iphi==1 && ieta==9) regionColl[39] = regionSummary;
    if(iphi==1 && ieta==8) regionColl[40] = regionSummary;
    if(iphi==1 && ieta==7) regionColl[41] = regionSummary;
    //BRAM 7
    if(iphi==2 && ieta==6) regionColl[42] = regionSummary;
    if(iphi==2 && ieta==5) regionColl[43] = regionSummary;
    if(iphi==2 && ieta==4) regionColl[44] = regionSummary;
    if(iphi==2 && ieta==3) regionColl[45] = regionSummary;
    if(iphi==2 && ieta==2) regionColl[46] = regionSummary;
    if(iphi==2 && ieta==1) regionColl[47] = regionSummary;
    if(iphi==2 && ieta==0) regionColl[48] = regionSummary;
    //BRAM 8
    if(iphi==2 && ieta==13) regionColl[49] = regionSummary;
    if(iphi==2 && ieta==12) regionColl[50] = regionSummary;
    if(iphi==2 && ieta==11) regionColl[51] = regionSummary;
    if(iphi==2 && ieta==10) regionColl[52] = regionSummary;
    if(iphi==2 && ieta==9) regionColl[53] = regionSummary;
    if(iphi==2 && ieta==8) regionColl[54] = regionSummary;
    if(iphi==2 && ieta==7) regionColl[55] = regionSummary;
    //BRAM 9
    if(iphi==17 && ieta==6) regionColl[56] = regionSummary;
    if(iphi==17 && ieta==5) regionColl[57] = regionSummary;
    if(iphi==17 && ieta==4) regionColl[58] = regionSummary;
    if(iphi==17 && ieta==3) regionColl[59] = regionSummary;
    if(iphi==17 && ieta==2) regionColl[60] = regionSummary;
    if(iphi==17 && ieta==1) regionColl[61] = regionSummary;
    if(iphi==17 && ieta==0) regionColl[62] = regionSummary;
    //BRAM 10
    if(iphi==17 && ieta==13) regionColl[63] = regionSummary;
    if(iphi==17 && ieta==12) regionColl[64] = regionSummary;
    if(iphi==17 && ieta==11) regionColl[65] = regionSummary;
    if(iphi==17 && ieta==10) regionColl[66] = regionSummary;
    if(iphi==17 && ieta==9) regionColl[67] = regionSummary;
    if(iphi==17 && ieta==8) regionColl[68] = regionSummary;
    if(iphi==17 && ieta==7) regionColl[69] = regionSummary;
    //BRAM 11
    if(iphi==16 && ieta==6) regionColl[70] = regionSummary;
    if(iphi==16 && ieta==5) regionColl[71] = regionSummary;
    if(iphi==16 && ieta==4) regionColl[72] = regionSummary;
    if(iphi==16 && ieta==3) regionColl[73] = regionSummary;
    if(iphi==16 && ieta==2) regionColl[74] = regionSummary;
    if(iphi==16 && ieta==1) regionColl[75] = regionSummary;
    if(iphi==16 && ieta==0) regionColl[76] = regionSummary;
    //BRAM 12
    if(iphi==16 && ieta==13) regionColl[77] = regionSummary;
    if(iphi==16 && ieta==12) regionColl[78] = regionSummary;
    if(iphi==16 && ieta==11) regionColl[79] = regionSummary;
    if(iphi==16 && ieta==10) regionColl[80] = regionSummary;
    if(iphi==16 && ieta==9) regionColl[81] = regionSummary;
    if(iphi==16 && ieta==8) regionColl[82] = regionSummary;
    if(iphi==16 && ieta==7) regionColl[83] = regionSummary;
    //BRAM 13
    if(iphi==9 && ieta==13) regionColl[84] = regionSummary;
    if(iphi==9 && ieta==12) regionColl[85] = regionSummary;
    if(iphi==9 && ieta==11) regionColl[86] = regionSummary;
    if(iphi==9 && ieta==10) regionColl[87] = regionSummary;
    if(iphi==9 && ieta==9) regionColl[88] = regionSummary;
    if(iphi==9 && ieta==8) regionColl[89] = regionSummary;
    if(iphi==9 && ieta==7) regionColl[90] = regionSummary;
    //BRAM 14
    if(iphi==9 && ieta==6) regionColl[91] = regionSummary;
    if(iphi==9 && ieta==5) regionColl[92] = regionSummary;
    if(iphi==9 && ieta==4) regionColl[93] = regionSummary;
    if(iphi==9 && ieta==3) regionColl[94] = regionSummary;
    if(iphi==9 && ieta==2) regionColl[95] = regionSummary;
    if(iphi==9 && ieta==1) regionColl[96] = regionSummary;
    if(iphi==9 && ieta==0) regionColl[97] = regionSummary;
    //BRAM 15
    if(iphi==12 && ieta==6) regionColl[98] = regionSummary;
    if(iphi==12 && ieta==5) regionColl[99] = regionSummary;
    if(iphi==12 && ieta==4) regionColl[100] = regionSummary;
    if(iphi==12 && ieta==3) regionColl[101] = regionSummary;
    if(iphi==12 && ieta==2) regionColl[102] = regionSummary;
    if(iphi==12 && ieta==1) regionColl[103] = regionSummary;
    if(iphi==12 && ieta==0) regionColl[104] = regionSummary;
    //BRAM 16
    if(iphi==12 && ieta==13) regionColl[105] = regionSummary;
    if(iphi==12 && ieta==12) regionColl[106] = regionSummary;
    if(iphi==12 && ieta==11) regionColl[107] = regionSummary;
    if(iphi==12 && ieta==10) regionColl[108] = regionSummary;
    if(iphi==12 && ieta==9) regionColl[109] = regionSummary;
    if(iphi==12 && ieta==8) regionColl[110] = regionSummary;
    if(iphi==12 && ieta==7) regionColl[111] = regionSummary;
    //BRAM 17
    if(iphi==13 && ieta==6) regionColl[112] = regionSummary;
    if(iphi==13 && ieta==5) regionColl[113] = regionSummary;
    if(iphi==13 && ieta==4) regionColl[114] = regionSummary;
    if(iphi==13 && ieta==3) regionColl[115] = regionSummary;
    if(iphi==13 && ieta==2) regionColl[116] = regionSummary;
    if(iphi==13 && ieta==1) regionColl[117] = regionSummary;
    if(iphi==13 && ieta==0) regionColl[118] = regionSummary;
    //BRAM 18
    if(iphi==13 && ieta==13) regionColl[119] = regionSummary;
    if(iphi==13 && ieta==12) regionColl[120] = regionSummary;
    if(iphi==13 && ieta==11) regionColl[121] = regionSummary;
    if(iphi==13 && ieta==10) regionColl[122] = regionSummary;
    if(iphi==13 && ieta==9) regionColl[123] = regionSummary;
    if(iphi==13 && ieta==8) regionColl[124] = regionSummary;
    if(iphi==13 && ieta==7) regionColl[125] = regionSummary;
    //BRAM 19
    if(iphi==14 && ieta==6) regionColl[126] = regionSummary;
    if(iphi==14 && ieta==5) regionColl[127] = regionSummary;
    if(iphi==14 && ieta==4) regionColl[128] = regionSummary;
    if(iphi==14 && ieta==3) regionColl[129] = regionSummary;
    if(iphi==14 && ieta==2) regionColl[130] = regionSummary;
    if(iphi==14 && ieta==1) regionColl[131] = regionSummary;
    if(iphi==14 && ieta==0) regionColl[132] = regionSummary;
    //BRAM 20
    if(iphi==14 && ieta==13) regionColl[133] = regionSummary;
    if(iphi==14 && ieta==12) regionColl[134] = regionSummary;
    if(iphi==14 && ieta==11) regionColl[135] = regionSummary;
    if(iphi==14 && ieta==10) regionColl[136] = regionSummary;
    if(iphi==14 && ieta==9) regionColl[137] = regionSummary;
    if(iphi==14 && ieta==8) regionColl[138] = regionSummary;
    if(iphi==14 && ieta==7) regionColl[139] = regionSummary;
    //BRAM 21
    if(iphi==11 && ieta==6) regionColl[140] = regionSummary;
    if(iphi==11 && ieta==5) regionColl[141] = regionSummary;
    if(iphi==11 && ieta==4) regionColl[142] = regionSummary;
    if(iphi==11 && ieta==3) regionColl[143] = regionSummary;
    if(iphi==11 && ieta==2) regionColl[144] = regionSummary;
    if(iphi==11 && ieta==1) regionColl[145] = regionSummary;
    if(iphi==11 && ieta==0) regionColl[146] = regionSummary;
    //BRAM 22
    if(iphi==11 && ieta==13) regionColl[147] = regionSummary;
    if(iphi==11 && ieta==12) regionColl[148] = regionSummary;
    if(iphi==11 && ieta==11) regionColl[149] = regionSummary;
    if(iphi==11 && ieta==10) regionColl[150] = regionSummary;
    if(iphi==11 && ieta==9) regionColl[151] = regionSummary;
    if(iphi==11 && ieta==8) regionColl[152] = regionSummary;
    if(iphi==11 && ieta==7) regionColl[153] = regionSummary;
    //BRAM 23
    if(iphi==10 && ieta==6) regionColl[154] = regionSummary;
    if(iphi==10 && ieta==5) regionColl[155] = regionSummary;
    if(iphi==10 && ieta==4) regionColl[156] = regionSummary;
    if(iphi==10 && ieta==3) regionColl[157] = regionSummary;
    if(iphi==10 && ieta==2) regionColl[158] = regionSummary;
    if(iphi==10 && ieta==1) regionColl[159] = regionSummary;
    if(iphi==10 && ieta==0) regionColl[160] = regionSummary;
    //BRAM 24
    if(iphi==10 && ieta==13) regionColl[161] = regionSummary;
    if(iphi==10 && ieta==12) regionColl[162] = regionSummary;
    if(iphi==10 && ieta==11) regionColl[163] = regionSummary;
    if(iphi==10 && ieta==10) regionColl[164] = regionSummary;
    if(iphi==10 && ieta==9) regionColl[165] = regionSummary;
    if(iphi==10 && ieta==8) regionColl[166] = regionSummary;
    if(iphi==10 && ieta==7) regionColl[167] = regionSummary;
    //BRAM 25
    if(iphi==3 && ieta==13) regionColl[168] = regionSummary;
    if(iphi==3 && ieta==12) regionColl[169] = regionSummary;
    if(iphi==3 && ieta==11) regionColl[170] = regionSummary;
    if(iphi==3 && ieta==10) regionColl[171] = regionSummary;
    if(iphi==3 && ieta==9) regionColl[172] = regionSummary;
    if(iphi==3 && ieta==8) regionColl[173] = regionSummary;
    if(iphi==3 && ieta==7) regionColl[174] = regionSummary;
    //BRAM 26
    if(iphi==3 && ieta==6) regionColl[175] = regionSummary;
    if(iphi==3 && ieta==5) regionColl[176] = regionSummary;
    if(iphi==3 && ieta==4) regionColl[177] = regionSummary;
    if(iphi==3 && ieta==3) regionColl[178] = regionSummary;
    if(iphi==3 && ieta==2) regionColl[179] = regionSummary;
    if(iphi==3 && ieta==1) regionColl[180] = regionSummary;
    if(iphi==3 && ieta==0) regionColl[181] = regionSummary;
    //BRAM 27
    if(iphi==6 && ieta==6) regionColl[182] = regionSummary;
    if(iphi==6 && ieta==5) regionColl[183] = regionSummary;
    if(iphi==6 && ieta==4) regionColl[184] = regionSummary;
    if(iphi==6 && ieta==3) regionColl[185] = regionSummary;
    if(iphi==6 && ieta==2) regionColl[186] = regionSummary;
    if(iphi==6 && ieta==1) regionColl[187] = regionSummary;
    if(iphi==6 && ieta==0) regionColl[188] = regionSummary;
    //BRAM 28
    if(iphi==6 && ieta==13) regionColl[189] = regionSummary;
    if(iphi==6 && ieta==12) regionColl[190] = regionSummary;
    if(iphi==6 && ieta==11) regionColl[191] = regionSummary;
    if(iphi==6 && ieta==10) regionColl[192] = regionSummary;
    if(iphi==6 && ieta==9) regionColl[193] = regionSummary;
    if(iphi==6 && ieta==8) regionColl[194] = regionSummary;
    if(iphi==6 && ieta==7) regionColl[195] = regionSummary;
    //BRAM 29
    if(iphi==7 && ieta==6) regionColl[196] = regionSummary;
    if(iphi==7 && ieta==5) regionColl[197] = regionSummary;
    if(iphi==7 && ieta==4) regionColl[198] = regionSummary;
    if(iphi==7 && ieta==3) regionColl[199] = regionSummary;
    if(iphi==7 && ieta==2) regionColl[200] = regionSummary;
    if(iphi==7 && ieta==1) regionColl[201] = regionSummary;
    if(iphi==7 && ieta==0) regionColl[202] = regionSummary;
    //BRAM 30
    if(iphi==7 && ieta==13) regionColl[203] = regionSummary;
    if(iphi==7 && ieta==12) regionColl[204] = regionSummary;
    if(iphi==7 && ieta==11) regionColl[205] = regionSummary;
    if(iphi==7 && ieta==10) regionColl[206] = regionSummary;
    if(iphi==7 && ieta==9) regionColl[207] = regionSummary;
    if(iphi==7 && ieta==8) regionColl[208] = regionSummary;
    if(iphi==7 && ieta==7) regionColl[209] = regionSummary;
    //BRAM 31
    if(iphi==8 && ieta==6) regionColl[210] = regionSummary;
    if(iphi==8 && ieta==5) regionColl[211] = regionSummary;
    if(iphi==8 && ieta==4) regionColl[212] = regionSummary;
    if(iphi==8 && ieta==3) regionColl[213] = regionSummary;
    if(iphi==8 && ieta==2) regionColl[214] = regionSummary;
    if(iphi==8 && ieta==1) regionColl[215] = regionSummary;
    if(iphi==8 && ieta==0) regionColl[216] = regionSummary;
    //BRAM 32
    if(iphi==8 && ieta==13) regionColl[217] = regionSummary;
    if(iphi==8 && ieta==12) regionColl[218] = regionSummary;
    if(iphi==8 && ieta==11) regionColl[219] = regionSummary;
    if(iphi==8 && ieta==10) regionColl[220] = regionSummary;
    if(iphi==8 && ieta==9) regionColl[221] = regionSummary;
    if(iphi==8 && ieta==8) regionColl[222] = regionSummary;
    if(iphi==8 && ieta==7) regionColl[223] = regionSummary;
    //BRAM 33
    if(iphi==5 && ieta==6) regionColl[224] = regionSummary;
    if(iphi==5 && ieta==5) regionColl[225] = regionSummary;
    if(iphi==5 && ieta==4) regionColl[226] = regionSummary;
    if(iphi==5 && ieta==3) regionColl[227] = regionSummary;
    if(iphi==5 && ieta==2) regionColl[228] = regionSummary;
    if(iphi==5 && ieta==1) regionColl[229] = regionSummary;
    if(iphi==5 && ieta==0) regionColl[230] = regionSummary;
    //BRAM 34
    if(iphi==5 && ieta==13) regionColl[231] = regionSummary;
    if(iphi==5 && ieta==12) regionColl[232] = regionSummary;
    if(iphi==5 && ieta==11) regionColl[233] = regionSummary;
    if(iphi==5 && ieta==10) regionColl[234] = regionSummary;
    if(iphi==5 && ieta==9) regionColl[235] = regionSummary;
    if(iphi==5 && ieta==8) regionColl[236] = regionSummary;
    if(iphi==5 && ieta==7) regionColl[237] = regionSummary;
    //BRAM 35
    if(iphi==4 && ieta==6) regionColl[238] = regionSummary;
    if(iphi==4 && ieta==5) regionColl[239] = regionSummary;
    if(iphi==4 && ieta==4) regionColl[240] = regionSummary;
    if(iphi==4 && ieta==3) regionColl[241] = regionSummary;
    if(iphi==4 && ieta==2) regionColl[242] = regionSummary;
    if(iphi==4 && ieta==1) regionColl[243] = regionSummary;
    if(iphi==4 && ieta==0) regionColl[244] = regionSummary;
    //BRAM 36
    if(iphi==4 && ieta==13) regionColl[245] = regionSummary;
    if(iphi==4 && ieta==12) regionColl[246] = regionSummary;
    if(iphi==4 && ieta==11) regionColl[247] = regionSummary;
    if(iphi==4 && ieta==10) regionColl[248] = regionSummary;
    if(iphi==4 && ieta==9) regionColl[249] = regionSummary;
    if(iphi==4 && ieta==8) regionColl[250] = regionSummary;
    if(iphi==4 && ieta==7) regionColl[251] = regionSummary;

  }

  // re-order to properly align the test vector for firmware such that index = phi * 14 + eta;
  for (unsigned int phi = 0; phi < 18; phi++){
    for (int eta = 0; eta < 14; eta++){
      cregions.push_back(regionColl_input[eta][phi]);
    }
  }

  uint16_t eWord[252] = {0};
  uint16_t eWord_input[252] = {0};
  unsigned short lines = 4*readCount_;
  unsigned short lines_input = 4*readCount_;
    unsigned short lines_match = 4*readCount_;
  unsigned short lines_output = 6*readCount_;

  for (unsigned int ireg = 0; ireg < 252; ireg++){
    eWord[ireg] = regionColl[ireg];
    eWord_input[ireg] = cregions[ireg];
  }

  // Write input test vector to algoblock

  std::fstream file1;

  char fileName1[20];
  sprintf(fileName1,"Regions.txt");
  file1.open(fileName1,std::fstream::in | std::fstream::out | std::fstream::app);

  if(readCount_==0) {
     file1 << "=====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================" <<std::endl;
     file1 << "WordCnt LINK_00         LINK_01         LINK_02         LINK_03         LINK_04         LINK_05         LINK_06         LINK_07         LINK_08         LINK_09         LINK_10         LINK_11         LINK_12         LINK_13         LINK_14         LINK_15         LINK_16         LINK_17         LINK_18         LINK_19         LINK_20         LINK_21         LINK_22         LINK_23         LINK_24         LINK_25         LINK_26         LINK_27         LINK_28         LINK_29         LINK_30         LINK_31         LINK_32         LINK_33         LINK_34         LINK_35" <<std::endl;
     file1 << "#BeginData" <<std::endl;
  }

  for(int cyc=0; cyc<4; cyc++){
    file1 << "0x" << std::hex << std::setfill('0') << std::setw(4) << lines << "\t";
    for(int i=0; i<36; i++){
      if(cyc==0) file1 << "0x" << std::hex << std::setfill('0') << std::setw(2) << (0XFF & eWord[i*7+1]) << std::setw(4) << eWord[i*7] << "00\t";
      if(cyc==1) file1 << "0x" << std::hex << std::setfill('0') << std::setw(2) << (0XFF & eWord[i*7+3]) << std::setw(4) << eWord[i*7+2] << std::setfill('0') << std::setw(2) << ( 0xFF & (eWord[i*7+1] >> 8)) <<"\t";
      if(cyc==2) file1 << "0x" << std::hex << std::setfill('0') << std::setw(2) << (0XFF & eWord[i*7+5]) << std::setw(4) << eWord[i*7+4] << std::setfill('0') << std::setw(2) << ( 0xFF & (eWord[i*7+3] >> 8)) <<"\t";
      if(cyc==3) file1 << "0x00" << std::hex << std::setfill('0') << std::setw(4) << eWord[i*7+6] << std::setfill('0') << std::setw(2) << ( 0xFF & (eWord[i*7+5] >> 8)) <<"\t";
    }
    lines++;
    file1 << "\n";
  }

  file1.close();

  // Write input test vector after link rearrangement as algorithm expects it to be ordered

  std::fstream file3;

  char fileName3[20];
  sprintf(fileName3,"Regions_input.txt");
  file3.open(fileName3,std::fstream::in | std::fstream::out | std::fstream::app);

  if(readCount_==0) {
     file3 << "=====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================" <<std::endl;
     file3 << "WordCnt LINK_00         LINK_01         LINK_02         LINK_03         LINK_04         LINK_05         LINK_06         LINK_07         LINK_08         LINK_09         LINK_10         LINK_11         LINK_12         LINK_13         LINK_14         LINK_15         LINK_16         LINK_17         LINK_18         LINK_19         LINK_20         LINK_21         LINK_22         LINK_23         LINK_24         LINK_25         LINK_26         LINK_27         LINK_28         LINK_29         LINK_30         LINK_31         LINK_32         LINK_33         LINK_34         LINK_35" <<std::endl;
     file3 << "#BeginData" <<std::endl;
  }

  for(int cyc=0; cyc<4; cyc++){
    file3 << "0x" << std::hex << std::setfill('0') << std::setw(4) << lines_input << "\t";
    for(int i=0; i<36; i++){
      if(cyc==0) file3 << "0x" << std::hex << std::setfill('0') << std::setw(2) << (0XFF & eWord_input[i*7+1]) << std::setw(4) << eWord_input[i*7] << "00\t";
      if(cyc==1) file3 << "0x" << std::hex << std::setfill('0') << std::setw(2) << (0XFF & eWord_input[i*7+3]) << std::setw(4) << eWord_input[i*7+2] << std::setfill('0') << std::setw(2) << ( 0xFF & (eWord_input[i*7+1] >> 8)) <<"\t";
      if(cyc==2) file3 << "0x" << std::hex << std::setfill('0') << std::setw(2) << (0XFF & eWord_input[i*7+5]) << std::setw(4) << eWord_input[i*7+4] << std::setfill('0') << std::setw(2) << ( 0xFF & (eWord_input[i*7+3] >> 8)) <<"\t";
      if(cyc==3) file3 << "0x00" << std::hex << std::setfill('0') << std::setw(4) << eWord_input[i*7+6] << std::setfill('0') << std::setw(2) << ( 0xFF & (eWord_input[i*7+5] >> 8)) <<"\t";
    }
    lines_input++;
    file3 << "\n";
  }

  file3.close();

  //Yeesh, what a mess
  std::fstream file4;

  char fileName4[20];
  sprintf(fileName4, "Regions_match.txt");
  file4.open(fileName4,std::fstream::in | std::fstream::out | std::fstream::app);
  if (readCount_==0) {
     file4 << "=====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================" <<std::endl;
     file4 << "WordCnt LINK_00         LINK_01         LINK_02         LINK_03         LINK_04         LINK_05         LINK_06         LINK_07         LINK_08         LINK_09         LINK_10         LINK_11         LINK_12         LINK_13         LINK_14         LINK_15         LINK_16         LINK_17         LINK_18         LINK_19         LINK_20         LINK_21         LINK_22         LINK_23         LINK_24         LINK_25         LINK_26         LINK_27         LINK_28         LINK_29         LINK_30         LINK_31         LINK_32         LINK_33         LINK_34         LINK_35" <<std::endl;
     file4 << "#BeginData" <<std::endl;
  }
  for(int cyc=0; cyc<4; cyc++){
    file4 << "0x" << std::hex << std::setfill('0') << std::setw(4) << lines_match << "\t";
    for(int i=0; i<36; i++){
      if(i%2 == 1){ //odd link, positive eta
	if(cyc==0) file4 << "0x" << std::hex << std::setfill('0') << std::setw(2) << (0XFF & eWord_input[i*7+1]) << std::setw(4) << eWord_input[i*7] << "00\t";
	if(cyc==1) file4 << "0x" << std::hex << std::setfill('0') << std::setw(2) << (0XFF & eWord_input[i*7+3]) << std::setw(4) << eWord_input[i*7+2] << std::setfill('0') << std::setw(2) << ( 0xFF & (eWord_input[i*7+1] >> 8)) <<"\t";
	if(cyc==2) file4 << "0x" << std::hex << std::setfill('0') << std::setw(2) << (0XFF & eWord_input[i*7+5]) << std::setw(4) << eWord_input[i*7+4] << std::setfill('0') << std::setw(2) << ( 0xFF & (eWord_input[i*7+3] >> 8)) <<"\t";
	if(cyc==3) file4 << "0x00" << std::hex << std::setfill('0') << std::setw(4) << eWord_input[i*7+6] << std::setfill('0') << std::setw(2) << ( 0xFF & (eWord_input[i*7+5] >> 8)) <<"\t";
      }
      else { //even link, negative eta
	if(cyc==0) file4 << "0x" << std::hex << std::setfill('0') << std::setw(2) << (0XFF & eWord_input[i*7+5]) << std::setw(4) << eWord_input[i*7+6] << "00\t";
	if(cyc==1) file4 << "0x" << std::hex << std::setfill('0') << std::setw(2) << (0XFF & eWord_input[i*7+3]) << std::setw(4) << eWord_input[i*7+4] << std::setfill('0') << std::setw(2) << ( 0xFF & (eWord_input[i*7+5] >> 8)) <<"\t";
	if(cyc==2) file4 << "0x" << std::hex << std::setfill('0') << std::setw(2) << (0XFF & eWord_input[i*7+1]) << std::setw(4) << eWord_input[i*7+2] << std::setfill('0') << std::setw(2) << ( 0xFF & (eWord_input[i*7+3] >> 8)) <<"\t";
	if(cyc==3) file4 << "0x00" << std::hex << std::setfill('0') << std::setw(4) << eWord_input[i*7+0] << std::setfill('0') << std::setw(2) << ( 0xFF & (eWord_input[i*7+1] >> 8)) <<"\t";
      }
    }
    lines_match++;
    file4 << "\n";
  }

  file4.close();

  // Prepare output test vector info
  edm::Handle< float > anomalyHandle;
  iEvent.getByToken(anomalyToken_, anomalyHandle);
  anomalyScore = *anomalyHandle;
  //std::cout<<"RegionDumper: anomalyScore = "<<std::dec<<anomalyScore<<std::endl;

  unsigned int integerRep = anomalyScore * pow(2.0, 8);
  ap_ufixed<16, 16> modelResult = ap_ufixed<16, 16>(integerRep);

  uint32_t output[6] = {0};
  output[0] |= ((0xF & modelResult.range(15, 12)) << 28);
  output[1] |= ((0xF & modelResult.range(11, 8)) << 28);
  output[2] |= ((0xF & modelResult.range(7, 4)) << 28);
  output[3] |= ((0xF & modelResult.range(3, 0)) << 28);

  edm::Handle<l1extra::L1JetParticleCollection> boostedJetCollectionHandle;
  iEvent.getByToken(boostedJetToken_, boostedJetCollectionHandle);
  std::vector<TLorentzVector> tempJets;
  tempJets.clear();
 
  for(const l1extra::L1JetParticle& theJet: *boostedJetCollectionHandle) {
    if(abs(theJet.eta()) < 2.4) {
      TLorentzVector temp;
      temp.SetPtEtaPhiE(theJet.pt(), theJet.eta(), theJet.phi(), theJet.energy());
      tempJets.push_back(temp);
    }
  }

  if(tempJets.size() > 1){  std::sort(tempJets.begin(), tempJets.end(), compareByPt);}
  //if(tempJets.size() > 0) std::cout<<"EVENT WITH BOOSTED JET"<<std::endl;
  for(size_t i = 0; i < 6; i++){
    if(i < tempJets.size()) {
      int jet_et = pt_converter(tempJets[i].Pt());
      int jet_ieta = eta_converter(tempJets[i].Eta());
      int jet_iphi = phi_converter(tempJets[i].Phi());
      //std::cout<<"RegionDumper: "<<std::dec<<tempJets[i].Pt()<<"\t"<<tempJets[i].Eta()<<"\t"<<tempJets[i].Phi()<<std::endl;
      //std::cout<<"RegionDumper: "<<std::dec<<jet_et<<"\t"<<jet_ieta<<"\t"<<jet_iphi<<std::endl;
      output[i] |= (0x000007FF & jet_et);
      if(tempJets[i].Eta() > 0) output[i] |= ((0xFF & ieta_lut[0][jet_ieta]) << 11);
      else output[i] |= ((0xFF & ieta_lut[1][jet_ieta]) << 11);
      output[i] |= ((0xFF & iphi_lut[jet_iphi]) << 19);
    }
    else {
       output[i] |= (0x000007FF & 0);
       output[i] |= ((0xFF & ieta_lut[1][default_eta]) << 11);
       output[i] |= ((0xFF & iphi_lut[default_phi]) << 19);
    }
  }

  // Write output test vector from algoblock

  std::fstream file2;

  char fileName2[20];
  sprintf(fileName2,"Outputs.txt");
  file2.open(fileName2,std::fstream::in | std::fstream::out | std::fstream::app);

  if(readCount_==0) {
    file2 << "=============================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================" << std::endl;
    file2 << "WordCnt             LINK_00               LINK_01" << std::endl;
    file2 << "#BeginData" << std::endl;
  }


  for(int i = 0; i < 6; i++){
    file2 << "0x" << std::hex << std::setfill('0') << std::setw(4) << lines_output << "\t";
    file2 << "0x" << std::hex << std::setfill('0') << std::setw(8) << (0XFFFFFFFF & output[i]) << std::endl;
    lines_output++;
  }

  file2.close();

  triggerTree->Fill();
  ++readCount_;

}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void L1TRegionDumper::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(L1TRegionDumper);
