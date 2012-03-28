;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This file contains graphic rule sets used in MEDM.
; You can edit it to add new sets or replace existents.
;
; Format of the file:
;  - any information started by ``;'' and up to the end of a line is considered
;    as a commentary;
;
;  - each graphic rule set begins with keyword ``graphicRule'' followed by
;    the name of a graphic rule set, it MUST be one word without spaces 
;    (set_Cryogenic, my-graphic-rule, 13891235%^&*=\);
;
;  - a graphic rule set finished by either a starting new rule set or by 
;    the end of file;
;
; Syntax of a rule:
;  - each line of a rule contains at least 4 elements (5th is optional)
;	<lowerBoundary> <upperBoundary> <adl file name> <element name> <macro>
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
graphicRule birke#test
	0 0 ComplexDynSymbolLib.adl elem0
	1 1 ComplexDynSymbolLib.adl elem1
	2 2 ComplexDynSymbolLib.adl elem2
	3 3 ComplexDynSymbolLib.adl elem3
	4 4 ComplexDynSymbolLib.adl elem4
	5 5 ComplexDynSymbolLib.adl elem5

graphicRule ParamPage 
	0 0 /opt/OPI/MedmConfig/dl/ComplexDynSymbolLib.adl ParamPageEntry DEVN=$(DEVN),DOMAIN=$(DOMAIN)

graphicRule PSI-BoosterStatus
	0 0 PSI-Blocks.adl PSI-BoosterStandbyStatus
	1 1 PSI-Blocks.adl PSI-BoosterAreaStatus
	2 2 PSI-Blocks.adl PSI-BoosterSecureStatus

graphicRule PSI-SRingStatus
        0 0 PSI-SR-Blocks.adl PSI-SRingStandbyStatus
        1 1 PSI-SR-Blocks.adl PSI-SRingAreaStatus
        2 2 PSI-SR-Blocks.adl PSI-SRingSecureStatus

graphicRule TriggerP
        8 15 TriggerPLib.adl TriggerMuxRemote
      	0 7 TriggerPLib.adl TriggerMuxLocal

graphicRule TriggerMod
        0 0 TriggerPLib.adl OldMod
      	1 1 TriggerPLib.adl NewMod

graphicRule DynconControl
	0 65536 trigger_synoptic.adl DynconMain

graphicRule TriggerControl
	0 65536 trigger_synoptic.adl TriggerMain

graphicRule ring-straight
	  0.0   0.1 straightSections.adl D1
	  0.9   1.1 straightSections.adl T1
	  1.9   2.1 straightSections.adl D2
	  2.9   3.1 straightSections.adl T2
	  3.9   4.1 straightSections.adl D3
	  4.9   5.1 straightSections.adl T3
	  5.9   6.1 straightSections.adl D4
	  6.9   7.1 straightSections.adl T4
	  7.9   8.1 straightSections.adl D5
	  8.9   9.1 straightSections.adl T5
	  9.9  10.1 straightSections.adl D6
	 10.9  11.1 straightSections.adl T6
	 11.9  12.1 straightSections.adl D7
	 12.9  13.1 straightSections.adl T7
	 13.9  14.1 straightSections.adl D8
	 14.9  15.1 straightSections.adl T8

graphicRule ring-section
	  0.0	1.1 SEC0D1R.adl   sector
	  1.9	2.1 SEC0T1R.adl   sector 
	  2.9	3.1 SEC0D2R.adl   sector
	  3.9	4.1 SEC0T2R.adl   sector 
	  4.9	5.1 SEC0D3R.adl   sector 
	  5.9	6.1 SEC0T3R.adl   sector 
	  6.9	7.1 SEC0D4R.adl   sector 
	  7.9	8.1 SEC0T4R.adl   sector 
	  8.9	9.1 SEC0D5R.adl   sector 
	  9.9  10.1 SEC0T5R.adl   sector 
	 10.9  11.1 SEC0D6R.adl   sector 
	 11.9  12.1 SEC0T6R.adl   sector 
	 12.9  13.1 SEC0D7R.adl   sector 
	 13.9  14.1 SEC0T7R.adl   sector
	 14.9  15.1 SEC0D8R.adl   sector 
	 15.9  16.1 SEC0T8R.adl   sector 
	 16.9  17.1 SEC1D1R.adl   sector
	 17.9  18.1 SEC1T1R.adl   sector
	 18.9  19.1 SEC1D2R.adl   sector
	 19.9  20.1 SEC1T2R.adl   sector
	 20.9  21.1 SEC1D3R.adl   sector
	 21.9  22.1 SEC1T3R.adl   sector
	 22.9  23.1 SEC1D4R.adl   sector
	 23.9  24.1 SEC1T4R.adl   sector
	 24.9  25.1 SEC1D5R.adl   sector
	 25.9  26.1 SEC1T5R.adl   sector
	 26.9  27.1 SEC1D6R.adl   sector
	 27.9  28.1 SEC1T6R.adl   sector
	 28.9  29.1 SEC1D7R.adl   sector
	 29.9  30.1 SEC1T7R.adl   sector
	 30.9  31.1 SEC1D8R.adl   sector
	 31.9  32.1 SEC1T8R.adl   sector
	 32.9  33.1 SEC2D1R.adl   sector
	 33.9  34.1 SEC2T1R.adl   sector
	 34.9  35.1 SEC2D2R.adl   sector
	 35.9  36.1 SEC2T2R.adl   sector
	 36.9  37.1 SEC2D3R.adl   sector
	 37.9  38.1 SEC2T3R.adl   sector
	 38.9  39.1 SEC2D4R.adl   sector
	 39.9  40.1 SEC2T4R.adl   sector
	 40.9  41.1 SEC2D5R.adl   sector
	 41.9  42.1 SEC2T5R.adl   sector
	 42.9  43.1 SEC2D6R.adl   sector
	 43.9  44.1 SEC2T6R.adl   sector
	 44.9  45.1 SEC2D7R.adl   sector
	 45.9  46.1 SEC2T7R.adl   sector
	 46.9  47.1 SEC2D8R.adl   sector
	 47.9  48.1 SEC2T8R.adl   sector

graphicRule ring-select
	  0.0  16.1 ring-SELECT.adl M0
	 16.9  32.1 ring-SELECT.adl M1
	 32.9  48.1 ring-SELECT.adl M2

graphicRule Ampel
	0.0	0.1   Ampelmann.adl Steher
	0.9	48.1  Ampelmann.adl Geher

graphicRule AmpelSEVR
	0.0	0.1   Ampelmann.adl Geher
	0.9	48.1  Ampelmann.adl Steher

graphicRule miniAmpelSEVR
	-1.0	0.9   Ampelmann.adl miniGeher
	0.9	1.9   Ampelmann.adl miniSteherGelb
	1.9	9.9   Ampelmann.adl miniSteher

graphicRule miniAmpel
	0.0	0.1   Ampelmann.adl miniSteher
	0.9	48.1  Ampelmann.adl miniGeher

graphicRule miniAmpel3
	0.0	0.1   Ampelmann.adl miniSteher
	1.0	1.1   Ampelmann.adl miniSteherGelb
	1.9	48.1  Ampelmann.adl miniGeher

graphicRule miniAmpelGelb
	0.0	0.1   Ampelmann.adl miniSteherGelb
	0.9	48.1  Ampelmann.adl miniGeher

graphicRule sAmpel
	0.0	0.1   Ampelmann.s.adl sSteher
	0.9	48.1  Ampelmann.s.adl sGeher

graphicRule ring-current
    -1000.0000000     0.1   ring-current.adl lvl.a
          0.10001     5.0   ring-current.adl lvl.b
          5.00001    30.0   ring-current.adl lvl.c
         30.00001    45.0   ring-current.adl lvl.d
         45.00001    60.0   ring-current.adl lvl.e
         60.00001   200.0   ring-current.adl lvl.f
        200.00001   250.0   ring-current.adl lvl.g
        250.00001   350.0   ring-current.adl lvl.h
        350.00001   399.0   ring-current.adl lvl.i
        399.00001  1000.0   ring-current.adl lvl.warn

graphicRule ring-lifetime
    -0.10   0.9   ring-current.adl storage
     0.91   1.1   ring-current.adl inject

graphicRule injection-status
    -0.10     0.1   inj-status.adl  inactive
     0.10001  1.1   inj-status.adl  autoInject
     1.10001  2.1   inj-status.adl  autoTop
     2.10001  3.1   inj-status.adl  passive
     3.10001  4.1   inj-status.adl  inject
     4.10001  5.1   inj-status.adl  hv_gun  
     5.10001  6.1   inj-status.adl  trg_gun 
     6.10001  7.1   inj-status.adl  kickers 
     7.10001  8.1   inj-status.adl  septa   
     8.10001  9.1   inj-status.adl  triggers
     9.10001 10.1   inj-status.adl  wait    
    10.10001 99.9   inj-status.adl  dump    

graphicRule gKickerR
	0.0     0.1   ring-kicker.adl rk-general
	0.11 9999.9   ring-kicker.adl rk-sync

graphicRule ScraperX1
	0 1 Scraper.Axis.adl elemAxis AXIS=X1,PREFIX=SCRP

graphicRule ScraperY1
	0 1 Scraper.Axis.adl elemAxis AXIS=Y1,PREFIX=SCRP

graphicRule ScraperX2
	0 1 Scraper.Axis.adl elemAxis AXIS=X2,PREFIX=SCRP

graphicRule ScraperY2
	0 1 Scraper.Axis.adl elemAxis AXIS=Y2,PREFIX=SCRP

; Rules for Power Mux

graphicRule PMux-Sector1
	0 1 PMux.adl Sector SEC=1

graphicRule PMux-Sector2
	0 1 PMux.adl Sector SEC=2

graphicRule PMux-Sector3
	0 1 PMux.adl Sector SEC=3

graphicRule PMux-Sector4
	0 1 PMux.adl Sector SEC=4

graphicRule PMux-Sector5
	0 1 PMux.adl Sector SEC=5

graphicRule PMux-Sector6
	0 1 PMux.adl Sector SEC=6

graphicRule PMux-Sector7
	0 1 PMux.adl Sector SEC=7

graphicRule PMux-Sector8
	0 1 PMux.adl Sector SEC=8

graphicRule PMux-PS
	0 1 PMux.adl PS-Control PS=QSPAZR,DOMAIN=24

graphicRule W7-W7IT1R-Press
        0 1 WLS7Blocks.adl W7IT1R

graphicRule W7-W7IT2R-Press
        0 1 WLS7Blocks.adl W7IT2R

graphicRule W7-W7IT7R-Press
        0 1 WLS7Blocks.adl W7IT7R

graphicRule HF-PAHB-Temp
	0 1 HFBlocks.adl NoTemp DEVN=PAHB

graphicRule HF-PAH1R-Temp
	0 1 HFBlocks.adl WithTemp DEVN=PAH1R,IND=1

graphicRule HF-PAH2R-Temp
	0 1 HFBlocks.adl WithTemp DEVN=PAH2R,IND=2

graphicRule HF-PAH3R-Temp
	0 1 HFBlocks.adl WithTemp DEVN=PAH3R,IND=3

graphicRule HF-PAH4R-Temp
	0 1 HFBlocks.adl WithTemp DEVN=PAH4R,IND=4

graphicRule HF-PAHB-Volt
	0 1 HFBlocks.adl cavvoltfix DEVN=PAHB

graphicRule HF-PAH1R-Volt
	0 1 HFBlocks.adl cavvoltedit DEVN=PAH1R

graphicRule HF-PAH2R-Volt
	0 1 HFBlocks.adl cavvoltedit DEVN=PAH2R

graphicRule HF-PAH3R-Volt
	0 1 HFBlocks.adl cavvoltedit DEVN=PAH3R

graphicRule HF-PAH4R-Volt
	0 1 HFBlocks.adl cavvoltedit DEVN=PAH4R

graphicRule HF-PAHB-Phas
	0 1 HFBlocks.adl cavphasedit DEVN=PAHB

graphicRule HF-PAH1R-Phas
	0 1 HFBlocks.adl cavphasfix DEVN=PAH1R

graphicRule HF-PAH2R-Phas
	0 1 HFBlocks.adl cavphasfix DEVN=PAH2R

graphicRule HF-PAH3R-Phas
	0 1 HFBlocks.adl cavphasfix DEVN=PAH3R

graphicRule HF-PAH4R-Phas
	0 1 HFBlocks.adl cavphasfix DEVN=PAH4R

graphicRule HF-PAHB-Circ
	0 1 HFBlocks.adl NoCircPar DEVN=PAHB

graphicRule HF-PAH1R-Circ
	0 1 HFBlocks.adl CircPar DEVN=PAH1R

graphicRule HF-PAH2R-Circ
	0 1 HFBlocks.adl CircPar DEVN=PAH2R

graphicRule HF-PAH3R-Circ
	0 1 HFBlocks.adl CircPar DEVN=PAH3R

graphicRule HF-PAH4R-Circ
	0 1 HFBlocks.adl CircPar DEVN=PAH4R

graphicRule HF-PAHB-CalcXtra
        0 1 HFBlocks.adl NoCalcXtra DEVN=PAHB

graphicRule HF-PAH1R-CalcXtra
        0 1 HFBlocks.adl WithCalcXtra DEVN=PAH1R

graphicRule HF-PAH2R-CalcXtra
        0 1 HFBlocks.adl WithCalcXtra DEVN=PAH2R

graphicRule HF-PAH3R-CalcXtra
        0 1 HFBlocks.adl WithCalcXtra DEVN=PAH3R

graphicRule HF-PAH4R-CalcXtra
        0 1 HFBlocks.adl WithCalcXtra DEVN=PAH4R

graphicRule CavityTemp1PreCavity
	0 1 CavityTempLib.adl PreCavity

graphicRule CavityTemp1PostValve
	0 1 CavityTempLib.adl PostValve

graphicRule CavityTemp2PreCavity
	0 1 CavityTempLib.adl NoTemp

graphicRule CavityTemp2PostValve
	0 1 CavityTempLib.adl NoTemp

graphicRule CavityTemp3PreCavity
	0 1 CavityTempLib.adl NoTemp

graphicRule CavityTemp3PostValve
	0 1 CavityTempLib.adl NoTemp

graphicRule CavityTemp4PreCavity
	0 1 CavityTempLib.adl NoTemp

graphicRule CavityTemp4PostValve
	0 1 CavityTempLib.adl NoTemp

graphicRule GunPulseForm
        0 0 gun-control-include.adl MBPFN
        1 1 gun-control-include.adl SBPFN

graphicRule GFOCPM
	0 0 gun-control-include.adl GFOCPM_local
	1 1 gun-control-include.adl GFOCPM_remote

graphicRule GMC-MOD-CTL
	0 0 gun-control-include.adl gmc-old-ctl
	1 1 gun-control-include.adl gmc-ppt-ctl

graphicRule GMC-MOD-PFN
	0 0 gun-control-include.adl gmc-old-pfn
	1 1 gun-control-include.adl gmc-ppt-pfn

graphicRule MO-PFN
	0 0 gun-control-include.adl mo-old
	1 1 gun-control-include.adl mo-ppt

graphicRule MO-LINAC
	0 0 gun-control-include.adl mo-old-linac
	1 1 gun-control-include.adl mo-ppt-linac
