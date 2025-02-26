#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "CalibFormats/CaloTPG/interface/CaloTPGTranscoder.h"
#include "CalibFormats/CaloTPG/interface/CaloTPGRecord.h"
#include "FWCore/Utilities/interface/ESGetToken.h"
#include <fstream>
#include <iomanip>
#include <iostream>

using namespace std;


class Layer1Emulator : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  Layer1Emulator(const edm::ParameterSet& pset);
  virtual ~Layer1Emulator(){}
  bool findHcal(int ieta, int iphi, 
		edm::Handle<edm::SortedCollection<HcalTriggerPrimitiveDigi> > &hcalTpgs, 
		int &hcalEt, int &hcalFG);

  bool writeLink(std::string CTP7Name, int zside, int ietaIn, int iphiIn, unsigned int gctphiIn,
		 edm::Handle<edm::SortedCollection<HcalTriggerPrimitiveDigi> > &hcalTpgs,
		 edm::Handle<EcalTrigPrimDigiCollection> &ecalTpgs, int count);

  bool findEcal(int ieta, int iphi, 
		edm::Handle<EcalTrigPrimDigiCollection> &ecalTpgs, 
		int &ecalEt, int &ecalFG);

  void makeword(unsigned int &outputWord, 
		unsigned int word0, 
		unsigned int word1, 
		unsigned int word2, 
		unsigned int word3);

  void getGCTphi(const int iPhi, unsigned int &gctPhi);
  
private:
  void analyze(const edm::Event&, const edm::EventSetup&) override;

  edm::EDGetTokenT<HcalTrigPrimDigiCollection> hcalDigisToken_;
  edm::EDGetTokenT<EcalTrigPrimDigiCollection> ecalDigisToken_;  

  int hcalValue_;
  int ecalValue_;
  bool debug_;
  edm::ESGetToken<CaloTPGTranscoder, CaloTPGRecord> decoderToken_;
  int readCount_;
  std::string folder_;
};

Layer1Emulator::Layer1Emulator(const edm::ParameterSet& pset) {
  hcalDigisToken_ = consumes<HcalTrigPrimDigiCollection>(pset.getParameter<edm::InputTag>("hcalDigis"));
  ecalDigisToken_ = consumes<EcalTrigPrimDigiCollection>(pset.getParameter<edm::InputTag>("ecalDigis"));
  hcalValue_ = pset.getUntrackedParameter<int>("hcalValue",0);
  ecalValue_ = pset.getUntrackedParameter<int>("ecalValue",0);
  debug_ = pset.exists("debug") ? pset.getParameter<bool>("debug") : false;
  decoderToken_ = esConsumes<CaloTPGTranscoder, CaloTPGRecord>();
  readCount_ = 0;
  folder_ = pset.getUntrackedParameter<std::string>("foldername","");
}

void Layer1Emulator::analyze(const edm::Event& evt, const edm::EventSetup& es) {
  using namespace edm;
  std::string foldername_ = folder_;

  int nCTP7s = 36;
  //int nCTP7s = 4;

  struct ctp7{
    std::string name;
    int zside;
    int iphi;
    unsigned int gctphi;
  }CTP7[nCTP7s];

  //CTP7[0].zside =  1; CTP7[0].iphi = 71; sprintf(CTP7[0].name,"CTP7_0"); 
  //CTP7[1].zside = -1; CTP7[1].iphi = 71; sprintf(CTP7[1].name,"CTP7_1"); 
  //CTP7[2].zside =  1; CTP7[2].iphi = 3; sprintf(CTP7[2].name,"CTP7_2"); 
  //CTP7[3].zside = -1; CTP7[3].iphi = 3; sprintf(CTP7[3].name,"CTP7_3"); 
  
  for(int i = 0 ; i < 18; i++){
    int iP = i*2;
    int iM = i*2 + 1;
    int temp_phi = i*4 - 1;
    if(temp_phi < 1) temp_phi += 72;
    CTP7[iP].zside =  1; CTP7[iP].iphi = temp_phi; CTP7[iP].name = "P";
    CTP7[iM].zside = -1; CTP7[iM].iphi = temp_phi; CTP7[iM].name = "M";
    }

  //calculate gct phi for all
  for(int iCTP7 = 0; iCTP7 < nCTP7s; iCTP7++){
    getGCTphi(CTP7[iCTP7].iphi,CTP7[iCTP7].gctphi);
    //std::cout<<"CTP7 iphi "<< CTP7[iCTP7].iphi<<"CTP7 gctiphi "<<CTP7[iCTP7].gctphi<<std::endl;
  }

  edm::ESHandle<CaloTPGTranscoder> decoder;
  decoder = es.getHandle(decoderToken_);
  edm::Handle<HcalTrigPrimDigiCollection> hcalTpgs;
  edm::Handle<EcalTrigPrimDigiCollection> ecalTpgs;

  std::ofstream file;

  std::string fileName;
  fileName = foldername_ + "Events.txt";
  file.open(fileName,std::fstream::in | std::fstream::out | std::fstream::app);


  //get ecal and hcal digis
  if(evt.getByToken(ecalDigisToken_, ecalTpgs))
    if(evt.getByToken(hcalDigisToken_, hcalTpgs)){

      //int tpgEt=0,tpgiEta=0,tpgiPhi=0;
      //Write the event!!
      file<<evt.id().run()<<":"<<evt.luminosityBlock()<<":"<<evt.id().event()<<std::endl;
      //std::cout<< evt.id().run() << ":" <<evt.luminosityBlock()<<":"<<evt.id().event()<<std::endl;
      //file<<" tpgEt "<<tpgEt<<" tpgiEta "<<tpgiEta<<" tpgiPhi "<<tpgiPhi<<std::endl;
      
      for(int iCTP7 = 0; iCTP7 < nCTP7s; iCTP7++){
	//each bram is 4 in iphi and 2 in ieta
	int zside = CTP7[iCTP7].zside;
	int iphi  = CTP7[iCTP7].iphi;
	unsigned int gctphi  = CTP7[iCTP7].gctphi;
	
	for(int ieta = 1 ; ieta < 28; ieta=ieta+2){
	  if(!writeLink(CTP7[iCTP7].name,zside,ieta,iphi,gctphi,hcalTpgs,ecalTpgs,readCount_))
	    std::cout<<"Error Writing Link"<<std::endl;
	}
      }
    }
  file.close();
  ++readCount_;
}


bool Layer1Emulator::writeLink(std::string CTP7Name, int zside, int ietaIn, int iphiIn, unsigned int gctphiIn,
			       edm::Handle<edm::SortedCollection<HcalTriggerPrimitiveDigi> > &hcalTpgs,
			       edm::Handle<EcalTrigPrimDigiCollection> &ecalTpgs, int count)
{
  std::ofstream fileEcal;
  std::ofstream fileHcal;
  std::string string_ieta;
  std::string string_iphi;
  if(ietaIn < 10) string_ieta = "0" + std::to_string(ietaIn);
  else string_ieta = std::to_string(ietaIn);
  if(gctphiIn < 10) string_iphi = "0" + std::to_string(gctphiIn);
  else string_iphi = std::to_string(gctphiIn);

  std::string foldername_ = folder_;
  std::string fileNameHcal = foldername_ + "calo_slice_phi_" + string_iphi + "_" + CTP7Name + "_ieta_" + string_ieta + "_HCAL";
  std::string fileNameEcal = foldername_ + "calo_slice_phi_" + string_iphi + "_" + CTP7Name + "_ieta_" + string_ieta + "_ECAL";

  fileEcal.open(fileNameEcal,std::fstream::in | std::fstream::out | std::fstream::app);
  fileHcal.open(fileNameHcal,std::fstream::in | std::fstream::out | std::fstream::app);
  
  int ecalEt[8] = {0}; //, ecalEt2 = 0, ecalEt3 = 0, ecalEt4 = 0, ecalEt5 = 0, ecalEt6 = 0, ecalEt7 = 0, ecalEt8 = 0;
  int hcalEt[8] = {0}; //, hcalEt2 = 0, hcalEt3 = 0, hcalEt4 = 0, hcalEt5 = 0, hcalEt6 = 0, hcalEt7 = 0, hcalEt8 = 0;
  int ecalFG[8] = {0};
  int hcalFG[8] = {0};
  
  unsigned int eWord[4] = {0};

  unsigned int hWord[4] = {0};

  int ieta = ietaIn;
  //unsigned int iphi = iphiIn;

  /* Current Implementation
   * TODO:Check filling
   * 
   *            ieta      ieta+1
   * iphi     hcalET[0]  hcalET[1]
   * iphi+1   hcalET[2]  hcalET[3]
   * iphi+2   hcalET[4]  hcalET[5]
   * iphi+3   hcalET[6]  hcalET[7]
   */

  for(int iphi = iphiIn, index = 0; iphi < 4 + iphiIn; iphi++, index+=2 ){
    int iphiFind = iphi;
    if(iphiFind == 73) iphiFind = 1;
    if(iphiFind == 74) iphiFind = 2;

    if(!findHcal(  zside*ieta   , iphiFind, hcalTpgs,  hcalEt[index],    hcalFG[index]))
      std::cout<<"Error!" <<std::endl;
  
    if(!findEcal(  zside*ieta   , iphiFind, ecalTpgs,  ecalEt[index],    ecalFG[index]))
      std::cout<<"Error!"<<std::endl;

    if(!findHcal( zside*(ieta+1), iphiFind, hcalTpgs, hcalEt[index+1], hcalFG[index+1]))
      std::cout<<"Error!" <<std::endl;
    
    if(!findEcal( zside*(ieta+1), iphiFind, ecalTpgs, ecalEt[index+1], ecalFG[index+1]))
      std::cout<<"Error!"<<std::endl;

  }
  
  /* Now to make the words per the protocol
   * 
   * Current implementation is:
   *       word0   hcalET[6]    hcalET[4]    hcalET[2]    hcalET[0] 
   *       word1   hcalET[7]    hcalET[5]    hcalET[3]    hcalET[1]
   *       word2   other
   *       word3   other
   */
  
  makeword( eWord[0], ecalEt[0], ecalEt[2], ecalEt[4], ecalEt[6]);
  makeword( eWord[1], ecalEt[1], ecalEt[3], ecalEt[5], ecalEt[7]);

  makeword( hWord[0], hcalEt[0], hcalEt[2], hcalEt[4], hcalEt[6]);
  makeword( hWord[1], hcalEt[1], hcalEt[3], hcalEt[5], hcalEt[7]);

  //if(ecalEt[0]>0||ecalEt[1]>0||ecalEt[2]>0||ecalEt[0]>3)
  //std::cout << "Non Zero eWord[0] " << eWord[0] << std::endl;
  
  fileEcal <<"0x" << std::hex << std::setfill('0') << std::setw(8) << eWord[0] << " ";
  fileEcal <<"0x" << std::hex << std::setfill('0') << std::setw(8) << eWord[1] << " ";
  fileEcal <<"0x" << std::hex << std::setfill('0') << std::setw(8) << eWord[2] << " ";
  fileEcal <<"0x" << std::hex << std::setfill('0') << std::setw(8) << eWord[3] << " ";
  fileEcal <<std::endl;
  
  fileHcal <<"0x" << std::hex << std::setfill('0') << std::setw(8) << hWord[0] << " ";
  fileHcal <<"0x" << std::hex << std::setfill('0') << std::setw(8) << hWord[1] << " ";
  fileHcal <<"0x" << std::hex << std::setfill('0') << std::setw(8) << hWord[2] << " ";
  fileHcal <<"0x" << std::hex << std::setfill('0') << std::setw(8) << hWord[3] << " ";
  fileHcal <<std::endl;


  fileHcal.close();
  fileEcal.close();
  
  return true;

}

/*
 * Make a 32bit word
 *            word  3  2  1  0
 * outputWord = 0x FF FF FF FF
 */
void Layer1Emulator::makeword(unsigned int &outputWord, unsigned int word0, unsigned int word1, unsigned int word2, unsigned int word3){
  
  outputWord  = (0xFF & word0)    ;
  outputWord |= (0xFF & word1)<<8 ;
  outputWord |= (0xFF & word2)<<16;
  outputWord |= (0xFF & word3)<<24;

}

bool Layer1Emulator::findHcal(int ieta, int iphi, 
			      edm::Handle<edm::SortedCollection<HcalTriggerPrimitiveDigi> > &hcalTpgs, 
			      int &hcalEt, int &hcalFG)
{
  bool foundDigi = false;  

  for (size_t i = 0; i < hcalTpgs->size(); ++i) {
    HcalTriggerPrimitiveDigi tpg = (*hcalTpgs)[i];
    short ietaRef = tpg.id().ieta();
    short iphiRef = tpg.id().iphi();
    short zside = tpg.id().zside();
    //std::cout<<"HCAL ietaRef "<<ietaRef<<" zside "<<zside<<std::endl;
    if(ieta == ietaRef && iphi == iphiRef && zside == ieta/(abs(ieta))){
      hcalEt = tpg.SOI_compressedEt();
      //if((iphiRef > 2 && iphiRef < 7) && (ietaRef > 4 && ietaRef < 9)) hcalEt = tpg.SOI_compressedEt(); // example to select Region 22
      //hcalEt = 255; // saturated Et for the TPG
      hcalFG = tpg.SOI_fineGrain();
      foundDigi = true;
      break;
    }
  }

  if(!foundDigi){
    hcalEt = 0;
    hcalFG = 0;
    if(debug_)std::cout<<"No HCAL Digi Found"<<std::endl;
  }

  return true;
}

bool Layer1Emulator::findEcal(int ieta, int iphi, 
			      edm::Handle<EcalTrigPrimDigiCollection> &ecalTpgs, 
			      int &ecalEt, int &ecalFG)
{
  bool foundDigi = false;  

  for (size_t i = 0; i < ecalTpgs->size(); ++i) {
    EcalTriggerPrimitiveDigi tpg = (*ecalTpgs)[i];
    short ietaRef = tpg.id().ieta();
    short iphiRef = tpg.id().iphi();
    short zside = tpg.id().zside();

    if(ieta == ietaRef && iphi == iphiRef && zside == ieta/(abs(ieta))){
      //if(tpg.compressedEt()>0)
	//std::cout<<"ECAL ietaRef "<<ietaRef<<" iphiRef "<< iphiRef<<" zside "<<zside<< " tpgET "<< tpg.compressedEt() <<std::endl;
      ecalEt = tpg.compressedEt();
      //if((iphiRef > 2 && iphiRef < 7) && (ietaRef > 4 && ietaRef < 9)) ecalEt = tpg.compressedEt(); // example to select Region 22
      //ecalEt = 255; // saturated Et for the TPG
      ecalFG = tpg.fineGrain();
      foundDigi = true;
      break;
    }
  }

  if(!foundDigi){
    ecalEt = 0;
    ecalFG = 0;
    if(debug_)std::cout<<"No ECAL Digi Found"<<std::endl;
  }

  return true;
}



void Layer1Emulator::getGCTphi(const int iPhi, unsigned int &gctPhi){
  if(iPhi>=1&&iPhi<=2)
    gctPhi = 0;
  else if(iPhi>=71&&iPhi<=72)
    gctPhi = 0;
  else
    gctPhi = (((unsigned int) iPhi)+1)/4;

  //std::cout<<"iPhi "<<iPhi<<" gctPhi "<< gctPhi<<std::endl;
}


#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(Layer1Emulator);
