import os
import FWCore.ParameterSet.Config as cms

process = cms.Process("L1TRegionDumperTest")

process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.RawToDigi_Data_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '130X_dataRun3_Prompt_v4', '')

process.load('L1Trigger.Configuration.SimL1Emulator_cff')
process.load('L1Trigger.Configuration.CaloTriggerPrimitives_cff')
process.load('EventFilter.L1TXRawToDigi.caloLayer1Stage2Digis_cfi')
process.load('SimCalorimetry.HcalTrigPrimProducers.hcaltpdigi_cff')
process.load('CalibCalorimetry.CaloTPG.CaloTPGTranscoder_cfi')

process.raw2digi_step = cms.Path(process.RawToDigi)
process.endjob_step = cms.EndPath(process.endOfProcess)

process.schedule = cms.Schedule(process.raw2digi_step, process.endjob_step)
from PhysicsTools.PatAlgos.tools.helpers import associatePatAlgosToolsTask
associatePatAlgosToolsTask(process)

from L1Trigger.Configuration.customiseReEmul import L1TReEmulFromRAW
process = L1TReEmulFromRAW(process)

process.l1tRegionProd = cms.EDProducer(
    "L1TRegionProd",
    UCTRegion = cms.untracked.InputTag("simCaloStage2Layer1Digis"),
)
process.l1tRegionProdTask = cms.Task(
    process.l1tRegionProd,
)
#process.l1tRegionProdPath = cms.Path(process.l1tRegionProdTask)
#process.schedule.append(process.l1tRegionProdPath)

process.load('L1Trigger.L1TCaloLayer1.L1TCaloSummaryCICADAv2p1p1')
process.productionTask = cms.Task(
    process.L1TCaloSummaryCICADAv2p1p1,
)
process.productionPath = cms.Path(process.productionTask)
process.schedule.append(process.productionPath)

process.caloDigis = cms.EDAnalyzer(
    "Layer1Emulator",
    hcalDigis = cms.InputTag("hcalDigis"),
    ecalDigis = cms.InputTag("ecalDigis:EcalTriggerPrimitives"),
    hcalValue = cms.untracked.int32(40),
    ecalValue = cms.untracked.int32(40),
    debug = cms.bool(False)
)

#process.uctDigiStep = cms.Path(gctDigis*gtDigis*ecalDigis*hcalDigis)
#process.caloDigis = cms.Path(process.Layer1Digis)

process.l1tRegionDumper = cms.EDAnalyzer(
    "L1TRegionDumper",
    UCTRegion = cms.untracked.InputTag("simCaloStage2Layer1Digis"),
    #UCTRegion = cms.untracked.InputTag("l1tRegionProd", "TestRegion"),
    scoreSource = cms.InputTag("L1TCaloSummaryCICADAv2p1p1", "CICADAScore"),
    boostedJetCollection = cms.InputTag("L1TCaloSummaryCICADAv2p1p1", "Boosted"),
)
process.ntuple = cms.Path(process.caloDigis*process.l1tCaloLayer1Digis*process.simCaloStage2Layer1Digis*process.l1tRegionDumper)

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(256) )

process.source = cms.Source(
    "PoolSource",
    fileNames = cms.untracked.vstring(
        'root://cms-xrd-global.cern.ch://store/data/Run2023D/ZeroBias/RAW/v1/000/369/869/00000/ebb4bfa3-c235-4534-95f5-5a83f52de1d2.root'
        #'root://cms-xrd-global.cern.ch://store/data/Run2018A/ZeroBias/RAW/v1/000/315/267/00000/FEF4A8AF-E449-E811-BF43-02163E017F01.root'
    )
)

#process.source.eventsToProcess = cms.untracked.VEventRange("369869:56300216","369869:56194154","369869:58331384","369869:58280392","369869:58340410")

process.options = cms.untracked.PSet(
    
)

# Production Info
process.configurationMetadata = cms.untracked.PSet(
    annotation = cms.untracked.string('step2 nevts:1'),
    name = cms.untracked.string('Applications'),
    version = cms.untracked.string('$Revision: 1.19 $')
)

process.out = cms.OutputModule(
    "PoolOutputModule",
    fileName = cms.untracked.string("l1TFullEvent.root"),
    outputCommands = cms.untracked.vstring('drop *')
)

process.end = cms.EndPath(process.out)
#process.schedule.append(process.end)

#Output
process.TFileService = cms.Service(
    "TFileService",
    fileName = cms.string("output.root")
)

process.schedule.append(process.ntuple)

from PhysicsTools.PatAlgos.tools.helpers import associatePatAlgosToolsTask
associatePatAlgosToolsTask(process)

# Add early deletion of temporary data products to reduce peak memory need
from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete
process = customiseEarlyDelete(process)
# End adding early deletion
