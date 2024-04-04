#L1TRegionDumper

This repository contains a very simple CMSSW analyzer and a configuration file:

plugins/L1TRegionDumper.cc: Dumps CMS Calorimeter Layer-1 Trigger with UCTRegions to the log file in ASCII format

test/testL1TRegionDumper.py: Configures to read RAW data, emulates CMS Calorimeter Layer-1 Trigger starting from unpacked Layer-1 input data recorded in the RAW data to produce new UCTRegions

This is to be used together with https://github.com/aloeliger/anomalyDetection to be run on 2023 RAW data using different CICADA versions.
```
cmsrel CMSSW_14_0_0_pre2
cd CMSSW_14_0_0_pre2/src/
cmsenv && git cms-init

git cms-addpkg L1Trigger/L1TCaloLayer1

git cms-rebase-topic pallabidas:CICADA_Paper_Mods_14_0_0_pre2_testvectors

git clone --recursive https://github.com/aloeliger/anomalyDetection.git

cd anomalyDetection/

git clone https://github.com/Xilinx/HLS_arbitrary_Precision_Types hls

cd hls4mlEmulatorExtras/
make install
cd ../CICADA/
make

cd ../
git clone git@github.com:pallabidas/L1TRegionDumper.git -b top_master

cd ../

scram b -j 12

cd anomalyDetection
cmsRun L1TRegionDumper/test/testL1TRegionDumper.py 
```

The analyzer produces input to the algoblock taking the top crate as master, the output follows the interface document discussed with uGT.
