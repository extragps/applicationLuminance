
PROJET=Pmv

UCLINUX=../../sys/uClinux

include ../generation/defines.mk

ifeq ($(CIBLE),cpu432)
CPPFLAGS 	 = -DLINUX -DSANS_FLASH -DSANS_QUART $(INCLUDE_FLAGS) -DPMV -Iinclude \
	-Iinclude/pmv \
	-Iinclude/pmv/station \
	-Iinclude/worldFip \
	-Iinclude/worldFip/usr_fip \
	-Iinclude/worldFip/mcs 
#DEPS=  cpuRdtWebServer cpu432Tools cpu432Board  cpuRdtCommon cpuRdtComm klog kcommon
DEPS= c4sWebServer AfficheurC Ecouteur cpu432Tools cpu432Board cpuRdtComm kxml klog kcommon
VARIANTE_UCLINUX=rdt

else
# Pour l'instant rien n'est prevu pour faire une compilation appli... 
export CPPFLAGS 	 = -DLINUX -DSANS_FLASH -DSANS_QUART -DSANS_WORLDFIP $(INCLUDE_FLAGS)  -DPMV -Iinclude \
	-Wno-deprecated \
	-Iinclude/pmv \
	-Iinclude/pmv/station 
#DEPS=  cpuRdtWebServer cpu432Tools cpu432Board  cpuRdtCommon cpuRdtComm klog kcommon kxml
DEPS= c4sWebServer AfficheurC Ecouteur cpu432Tools cpu432Board cpuRdtComm klog kxml kcommon 
endif

	

TESTS = test

# Les sources de l'application sont dans un repertoire qui n'est pas le repertoire classique.
# On ajoute donc une definition.

SOURCES=sources
CPPFLAGS := $(CPPFLAGS) $(DEPS:%=-I../%/$(SOURCES))

FICHIERS =  \
$(SOURCES)/md5.c \
$(SOURCES)/DialogueLCR.c \
$(SOURCES)/LCR_CF6.c \
$(SOURCES)/LCR_IDF.c \
$(SOURCES)/LCR_SET.c \
$(SOURCES)/LCR_ST1.c \
$(SOURCES)/LCR_TRC.c \
$(SOURCES)/LCR_TST0.c \
$(SOURCES)/LCR_TST1.c \
$(SOURCES)/LCR_VT.c \
$(SOURCES)/MON_C32.c \
$(SOURCES)/MON_VAR.c \
$(SOURCES)/mon_time.c \
$(SOURCES)/MQ.c \
$(SOURCES)/Mon_s32.c \
$(SOURCES)/PipeLinux.c \
$(SOURCES)/SIR_DV1.c \
$(SOURCES)/Semaphore.c \
$(SOURCES)/SupInit.c \
$(SOURCES)/SupInitLib.c \
$(SOURCES)/Superviseur.c \
$(SOURCES)/TED_PROT.c \
$(SOURCES)/TRT_ALT.c \
$(SOURCES)/Tache.c \
$(SOURCES)/X01_TRC.c \
$(SOURCES)/X01_VAR.c \
$(SOURCES)/X01_VCT.c \
$(SOURCES)/X02_VCT.c \
$(SOURCES)/XDG_VAR.c \
$(SOURCES)/YPC_LAS.c \
$(SOURCES)/anaSys.c \
$(SOURCES)/bpTestLib.c \
$(SOURCES)/eriLib.c \
$(SOURCES)/etaSyst.c \
$(SOURCES)/extLib.c \
$(SOURCES)/ficTrace.c \
$(SOURCES)/identLib.c \
$(SOURCES)/ioPortLib2.c \
$(SOURCES)/lcr_cf0.c \
$(SOURCES)/lcr_cf2.c \
$(SOURCES)/lcr_cf3.c \
$(SOURCES)/lcr_cf5.c \
$(SOURCES)/lcr_cfid.c \
$(SOURCES)/lcr_ident.c \
$(SOURCES)/lcr_pcp.c \
$(SOURCES)/lcr_st_eri.c \
$(SOURCES)/lcr_st_dbg.c \
$(SOURCES)/lcr_st_lcom.c \
$(SOURCES)/lcr_st_ovf.c \
$(SOURCES)/lcr_st_ver.c \
$(SOURCES)/lcr_tc_e.c \
$(SOURCES)/lcr_temp.c \
$(SOURCES)/lcr_trace.c \
$(SOURCES)/lcr_tst_cnf.c \
$(SOURCES)/lcr_tst_debug.c \
$(SOURCES)/lcr_tst_sc.c \
$(SOURCES)/lcr_tst_cfsp.c \
$(SOURCES)/lcr_util.c \
$(SOURCES)/mgpLib.c \
$(SOURCES)/mon_debug.c \
$(SOURCES)/perfLib.c \
$(SOURCES)/pmv/lcr_cftp_util.c \
$(SOURCES)/pmv/LCR_CFES.c \
$(SOURCES)/pmv/LCR_CFPL.c \
$(SOURCES)/pmv/LCR_CFPP.c \
$(SOURCES)/pmv/LCR_CFTP.c \
$(SOURCES)/pmv/LCR_INIT.c \
$(SOURCES)/pmv/LCR_P.c \
$(SOURCES)/pmv/LCR_SC.c \
$(SOURCES)/pmv/LCR_TST.c \
$(SOURCES)/pmv/PIP_VAR.c \
$(SOURCES)/pmv/TAC_CTRL.c \
$(SOURCES)/pmv/X01_LMP.c \
$(SOURCES)/pmv/lcr_bk.c \
$(SOURCES)/pmv/lcr_cfet.c \
$(SOURCES)/pmv/lcr_cfsl.c \
$(SOURCES)/pmv/lcr_pa.c \
$(SOURCES)/pmv/lcr_pe.c \
$(SOURCES)/pmv/lcr_st_lcpi.c \
$(SOURCES)/pmv/lcr_tst_def.c \
$(SOURCES)/pmv/lcr_tst_lum.c \
$(SOURCES)/pmv/pipTest.c \
$(SOURCES)/pmv/pip_util.c \
$(SOURCES)/pmv/x03_pol.c \
$(SOURCES)/pmv/DialogueLuminance.c \
$(SOURCES)/pmv/station/event/event.c \
$(SOURCES)/pmv/station/cartes/station.c \
$(SOURCES)/pmv/station/cartes/stationAfficheurs.c \
$(SOURCES)/pmv/station/cartes/stationEs.c \
$(SOURCES)/pmv/station/cartes/stationThl.c \
$(SOURCES)/pmv/station/traiteurLum/lumTraiter.c \
$(SOURCES)/pmv/station/sequenceur/sequenceur.c \
$(SOURCES)/pmv/station/sequenceur/sequenceurTraiter.c \
$(SOURCES)/pmv/station/util/tokenizer.c \
$(SOURCES)/pmv/station/configuration/configuration.c \
$(SOURCES)/pmv/station/configuration/configurationEtat.c \
$(SOURCES)/pmv/station/configuration/configurationParam.c \
$(SOURCES)/pmv/station/configuration/configurationSram.c \
$(SOURCES)/pmv/station/configuration/sramMessage.c \
$(SOURCES)/pmv/station/configuration/sramBdd.c \
$(SOURCES)/portMsgQueue.c \
$(SOURCES)/portSemLib.c\
$(SOURCES)/rec_util.c \
$(SOURCES)/spcLib.c \
$(SOURCES)/stAlLib.c \
$(SOURCES)/supRun.c \
$(SOURCES)/tac_conf.c \
$(SOURCES)/tac_conf_fichier.c \
$(SOURCES)/teleLib.c \
$(SOURCES)/tempLib.c \
$(SOURCES)/versionApplication.c \
$(SOURCES)/passerelle/dialoguePass.c \

# $(SOURCES)/pmv/lon/TAC_PSL.c 
# $(SOURCES)/pmv/station/watchdog/watchdog.c \
# $(SOURCES)/pmv/lon/TAC_TRI.c 
# $(SOURCES)/pmv/lon/TAC_LMP.c 
# $(SOURCES)/pmv/lon/L_LT501.c \
# $(SOURCES)/pmv/lon/L_CD501.c \
# $(SOURCES)/pmv/lon/L_CD503.c \
# $(SOURCES)/pmv/lon/L_CL501.c \
# $(SOURCES)/pmv/lon/L_CO501.c \
# $(SOURCES)/pmv/lon/L_ES502.c \
# $(SOURCES)/pmv/lon/LON_MAI1.c \
# $(SOURCES)/pmv/lon/LON_PC.c 
# $(SOURCES)/pmv/lon/LON_TRC.c \
# $(SOURCES)/pmv/lon/PIP_MAIN.c \
# $(SOURCES)/pmv/lon/PIP_MAI1.c \
# $(SOURCES)/pmv/lon/LON_MAI0.c \
# $(SOURCES)/pmv/LCR_SCCC.c \
# $(SOURCES)/pmv/LCR_SCFR.c \
# $(SOURCES)/pmv/lcr_ver.c \
# $(SOURCES)/simuLib.c \
# $(SOURCES)/MAS_PROT.c \
# $(SOURCES)/pmv/lon/LON_VAR.c \
# $(SOURCES)/pmv/lon/PIP_LON.c \
# $(SOURCES)/mgpLib.c \
# $(SOURCES)/HDL_CRT.c \

FICHIERS_FIP =  \
$(SOURCES)/worldFip/dialogueFip.c \
$(SOURCES)/worldFip/usr_fip/usr_fip_time.c \
$(SOURCES)/worldFip/usr_fip/usr_fip_init.c \
$(SOURCES)/worldFip/usr_fip/usr_fip_mcs_init.c \
$(SOURCES)/worldFip/usr_fip/usr_fip.c \
$(SOURCES)/worldFip/usr_fip/usr_fip_mess.c \
$(SOURCES)/worldFip/usr_fip/usr_fip_msg.c \
$(SOURCES)/worldFip/usr_fip/usr_fip_var.c \
$(SOURCES)/worldFip/MCS/mcs_user.c \
$(SOURCES)/worldFip/MCS/mcsheart.c \
$(SOURCES)/worldFip/MCS/mcstools.c \
$(SOURCES)/worldFip/MCS/mcs_os.c \
$(SOURCES)/worldFip/MCS/mcsconfi.c \
$(SOURCES)/worldFip/fdm/fdm_os.c \
$(SOURCES)/worldFip/fdm/fdmevt.c \
$(SOURCES)/worldFip/fdm/St3/fipcod6_3.c \
$(SOURCES)/worldFip/fdm/fdmsmmps.c \
$(SOURCES)/worldFip/fdm/fdmhandl.c \
$(SOURCES)/worldFip/fdm/fdmgenbd.c \
$(SOURCES)/worldFip/fdm/fdmmsg.c \
$(SOURCES)/worldFip/fdm/fdm_fifo.c \
$(SOURCES)/worldFip/fdm/fdmbamed.c \
$(SOURCES)/worldFip/fdm/fdm_gdm.c \
$(SOURCES)/worldFip/fdm/fdm_gdt.c \
$(SOURCES)/worldFip/fdm/fdmtime.c \
$(SOURCES)/worldFip/fdm/fdm_lai.c \
$(SOURCES)/worldFip/fdm/fdmdicho.c \
$(SOURCES)/worldFip/fdm/fdmcreba.c \
$(SOURCES)/worldFip/fdm/fdmdiag.c \
$(SOURCES)/worldFip/fdm/fdmaele.c \

UNUSED = \
$(SOURCES)/worldFip/usr_fip/usr_fip_abt.c \
$(SOURCES)/WatchDog.c \
$(SOURCES)/TAC_VIP.c \
$(SOURCES)/worldFip/MCS/versionMcs.c \
$(SOURCES)/pipeLinux.c \
$(SOURCES)/mas_prot.c \
$(SOURCES)/rad/TST_VIP.c \
$(SOURCES)/worldFip/usr_fip/usr_fip_mcs_appli.c \
$(SOURCES)/xversion.c \
$(SOURCES)/rad/VIP_PROT.c 

AUTRES= \

OBJETS_APPLI = $(FICHIERS:$(SOURCES)/%.c=$(OBJ_DIR)/%.o)
OBJETS_FIP = $(FICHIERS_FIP:$(SOURCES)/%.c=$(OBJ_DIR)/%.o)
ifeq ($(CIBLE),cpu432)
SRC_FILES = $(FICHIERS) $(FICHIERS_FIP)
OBJ_FILES = $(OBJETS_APPLI) $(OBJETS_FIP)
else ifeq ($(CIBLE),rad_ip)
SRC_FILES = $(FICHIERS)
OBJ_FILES = $(OBJETS_APPLI)
else
SRC_FILES = $(FICHIERS)
OBJ_FILES = $(OBJETS_APPLI)
endif



# OBJETS = $(FICHIERS:$(SOURCES)/%.c=$(REP_OBJ)/%.o)

specificAllExec:

include ../generation/rules.mk
include ./uClinux.mk

