// -*- C++ -*-
//
// Package:    anomalyDetection/L1TRegionProd
// Class:      L1TRegionProd
//
/**\class L1TRegionProd L1TRegionProd.cc anomalyDetection/L1TRegionProd/plugins/L1TRegionProd.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Pallabi Das
//         Created:  Thu, 22 Feb 2024 17:27:51 GMT
//
//

// system include files
#include <memory>
#include <vector>
#include <iostream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"
#include "DataFormats/L1CaloTrigger/interface/L1CaloCollections.h"
#include "DataFormats/L1CaloTrigger/interface/L1CaloRegion.h"

//
// class declaration
//

class L1TRegionProd : public edm::stream::EDProducer<> {
public:
  explicit L1TRegionProd(const edm::ParameterSet&);
  ~L1TRegionProd() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginStream(edm::StreamID) override;
  void produce(edm::Event&, const edm::EventSetup&) override;
  void endStream() override;

  //void beginRun(edm::Run const&, edm::EventSetup const&) override;
  //void endRun(edm::Run const&, edm::EventSetup const&) override;
  //void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
  //void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

  // ----------member data ---------------------------
  edm::EDGetTokenT<std::vector <L1CaloRegion> > regionsToken_;
  int readCount_;
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
L1TRegionProd::L1TRegionProd(const edm::ParameterSet& iConfig) 
  : regionsToken_(consumes<std::vector <L1CaloRegion> >(iConfig.getUntrackedParameter<edm::InputTag>("UCTRegion"))),
  readCount_(0){
  produces<L1CaloRegionCollection>("TestRegion");
  //register your products
  /* Examples
  produces<ExampleData2>();

  //if do put with a label
  produces<ExampleData2>("label");
 
  //if you want to put into the Run
  produces<ExampleData2,InRun>();
  */
  //now do what ever other initialization is needed
}

L1TRegionProd::~L1TRegionProd() {
  // do anything here that needs to be done at destruction time
  // (e.g. close files, deallocate resources etc.)
  //
  // please remove this method altogether if it would be left empty
}

//
// member functions
//

// ------------ method called to produce the data  ------------
void L1TRegionProd::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;
  std::unique_ptr<L1CaloRegionCollection> rgnCollection(std::make_unique<L1CaloRegionCollection>());
  int event = iEvent.id().event();

  for (const auto& region : iEvent.get(regionsToken_)) {
    uint32_t ieta = region.id().ieta();
    uint32_t iphi = region.id().iphi();
    //uint16_t regionSummary = region.raw();
    uint16_t regionSummary = 0;
    if (event == 27080210 && iphi == 3) {
      regionSummary = region.raw();
      regionSummary |= 0x03FF;
      std::cout<<std::hex<<regionSummary<<std::endl;
    }
    //if (iphi==9 && (ieta-4==7 || ieta-4==13)) regionSummary = region.raw();
    //if (iphi==0 && ieta-4==7) regionSummary = region.raw();
    //if (readCount_==10 && iphi==0 && ieta-4==10) regionSummary = region.raw();
    //if (readCount_==99 && iphi==7 && ieta-4==1) regionSummary = region.raw();
    //std::cout<<ieta<<"\t"<<iphi<<"\t"<<regionSummary<<std::endl;
    rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)iphi, (int16_t)0));
    // copy the content of middle crate for the two other crates
    //if (iphi==15) {
    //  rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)iphi, (int16_t)0));
    //  rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)3, (int16_t)0));
    //  rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)9, (int16_t)0));
    //}
    //if (iphi==16) {
    //  rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)iphi, (int16_t)0));
    //  rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)4, (int16_t)0));
    //  rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)10, (int16_t)0));
    //}
    //if (iphi==17) {
    //  rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)iphi, (int16_t)0));
    //  rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)5, (int16_t)0));
    //  rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)11, (int16_t)0));
    //}
    //if (iphi==0) {
    //  rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)iphi, (int16_t)0));
    //  rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)6, (int16_t)0));
    //  rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)12, (int16_t)0));
    //}
    //if (iphi==1) {
    //  rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)iphi, (int16_t)0));
    //  rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)7, (int16_t)0));
    //  rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)13, (int16_t)0));
    //}
    //if (iphi==2) {
    //  rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)iphi, (int16_t)0));
    //  rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)8, (int16_t)0));
    //  rgnCollection->push_back(L1CaloRegion((uint16_t)regionSummary, (unsigned)ieta, (unsigned)14, (int16_t)0));
    //}
  }
  iEvent.put(std::move(rgnCollection), "TestRegion");
  ++readCount_;
  /* This is an event example
  //Read 'ExampleData' from the Event
  ExampleData const& in = iEvent.get(inToken_);

  //Use the ExampleData to create an ExampleData2 which 
  // is put into the Event
  iEvent.put(std::make_unique<ExampleData2>(in));
  */

  /* this is an EventSetup example
  //Read SetupData from the SetupRecord in the EventSetup
  SetupData& setup = iSetup.getData(setupToken_);
  */
}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void L1TRegionProd::beginStream(edm::StreamID) {
  // please remove this method if not needed
}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void L1TRegionProd::endStream() {
  // please remove this method if not needed
}

// ------------ method called when starting to processes a run  ------------
/*
void
L1TRegionProd::beginRun(edm::Run const&, edm::EventSetup const&)
{
}
*/

// ------------ method called when ending the processing of a run  ------------
/*
void
L1TRegionProd::endRun(edm::Run const&, edm::EventSetup const&)
{
}
*/

// ------------ method called when starting to processes a luminosity block  ------------
/*
void
L1TRegionProd::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/

// ------------ method called when ending the processing of a luminosity block  ------------
/*
void
L1TRegionProd::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void L1TRegionProd::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(L1TRegionProd);
