import os
import FWCore.ParameterSet.Config as cms

process = cms.Process("L1TRegionDumperTest")

#import EventFilter.L1TXRawToDigi.util as util

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.Geometry.GeometryExtended2016Reco_cff')
process.load('Configuration.StandardSequences.MagneticField_AutoFromDBCurrent_cff')
process.load('Configuration.StandardSequences.RawToDigi_Data_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc', '')

process.load('L1Trigger.Configuration.SimL1Emulator_cff')
process.load('L1Trigger.Configuration.CaloTriggerPrimitives_cff')
process.load('EventFilter.L1TXRawToDigi.caloLayer1Stage2Digis_cfi')

process.load('L1Trigger.L1TCaloLayer1.simCaloStage2Layer1Digis_cfi')
process.simCaloStage2Layer1Digis.useECALLUT = cms.bool(True)
process.simCaloStage2Layer1Digis.useHCALLUT = cms.bool(True)
process.simCaloStage2Layer1Digis.useHFLUT = cms.bool(True)
process.simCaloStage2Layer1Digis.useLSB = cms.bool(True)
process.simCaloStage2Layer1Digis.verbose = cms.bool(True)
process.simCaloStage2Layer1Digis.ecalToken = cms.InputTag("l1tCaloLayer1Digis")
process.simCaloStage2Layer1Digis.hcalToken = cms.InputTag("l1tCaloLayer1Digis")

process.l1tRegionDumper = cms.EDAnalyzer(
    "L1TRegionDumper",
    UCTRegion = cms.untracked.InputTag("simCaloStage2Layer1Digis","","L1TRegionDumperTest")
)

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(1000) )

process.source = cms.Source(
    "PoolSource",
    fileNames = cms.untracked.vstring(
        'file:/eos/user/p/pdas/L1Boosted/ggHbb/DR/RunIIAutumn18DRPremix_step1_21Dec_0_5300.root'
        #'root://cms-xrd-global.cern.ch://store/data/Run2018A/ZeroBias/RAW/v1/000/315/267/00000/FEF4A8AF-E449-E811-BF43-02163E017F01.root'
    )
)

#process.source.eventsToProcess = cms.untracked.VEventRange("1:11")

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


#Output
process.TFileService = cms.Service(
    "TFileService",
    fileName = cms.string("l1TNtuple.root")
)

process.p = cms.Path(process.l1tCaloLayer1Digis*process.simCaloStage2Layer1Digis*process.l1tRegionDumper)

process.e = cms.EndPath(process.out)

process.schedule = cms.Schedule(process.p)

from PhysicsTools.PatAlgos.tools.helpers import associatePatAlgosToolsTask
associatePatAlgosToolsTask(process)

# Add early deletion of temporary data products to reduce peak memory need
from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete
process = customiseEarlyDelete(process)
# End adding early deletion

#dump_file = open('dump.py','w')
#dump_file.write(process.dumpPython())
