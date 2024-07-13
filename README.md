#L1TRegionDumper

This repository contains a CMSSW analyzer and a configuration file:

plugins/L1TRegionDumper.cc: produces `.txt` files in the required ASCII format: 
```
Regions.txt : output of calolayer-1 top master configuration
Regions_input.txt : input to HLS
Regions_match.txt : input to HDL
Outputs.txt : cicada output
Events.txt : events processed
```

test/testL1TRegionDumper.py: Configures to read both data and MC, emulates CMS Calorimeter Layer-1 Trigger starting from unpacked Layer-1 input data recorded.

This configuration can also run plugins/Layer1Emulator.cc and plugins/L1TRegionProd.cc.

Layer1Emulator produces the ECAl and HCAL (not HF) TP input information in the required ASCII format for Layer-1 input BRAMS. The file names MUST follow this syntax: `calo_slice_phi_XX_P(M)_ieta_YY_ECAL(HCAL)` where `XX` runs from `01` to `17` and `YY` runs from `01` to `27` (only odd values for eta, `01` contains towers ieta=1 and 2, and so on). In total there should be `18 x 28 x 2 = 5008` of these files.

L1TRegionProd can be used to produce custom region collections for pattern tests, called "TestRegion", which can be input to L1TRegionDumper.

This is to be used together with https://github.com/aloeliger/anomalyDetection to be run on Run-3 data/MC using different CICADA versions.
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
git clone git@github.com:pallabidas/L1TRegionDumper.git -b folder_option

cd ../

scram b -j 12

cd anomalyDetection
cmsRun L1TRegionDumper/test/testL1TRegionDumper.py inputFiles="{input file name}" foldename="{full path to storage directory}"
```

The analyzer produces input to the algoblock taking the top crate as master, the output follows the interface document discussed with uGT.
