#**********************************************************************************************************************
# SIAT - Copyright SIAT
#----------------------------------------------------------------------------------------------------------------------
# fichier     : 
# description : 
# prefixe     : 
#----------------------------------------------------------------------------------------------------------------------
# Auteur      : 
# Date        : 
# OS          : Linux/ucLinux
#----------------------------------------------------------------------------------------------------------------------
# $Log: uClinux.mk,v $
# Revision 1.2  2018/06/04 08:38:42  xg
# Passage en version V122
#
# Revision 1.8  2009/06/30 11:04:42  xgaillard
# Ajout du forcage pour la creation du lien uClinux
#
# Revision 1.7  2009/06/04 16:21:44  xag
# Deplacement de copieSysteme dans /usr/bin
#
# Revision 1.6  2009/01/09 16:28:50  xag
# Copie du fichier pour le flashage.
#
# Revision 1.5  2009/01/09 16:27:15  xag
# Traitement du fichier password
#
# Revision 1.4  2008/10/13 08:19:04  xag
# *** empty log message ***
#
# Revision 1.3  2008/09/29 08:01:18  xag
# Ajout de la recompilation systematique du fichier de version.
#
# Revision 1.2  2008/09/22 09:37:15  xag
# *** empty log message ***
#
# Revision 1.1  2008/09/12 15:02:58  xag
# Archivage de printemps
#
# Revision 1.1  2008/03/07 17:28:37  xag
# *** empty log message ***
#
# Revision 1.1  2008/01/02 17:40:27  sgronchi
# regles specifiques a la construction du fichier image.bin
#
# Revision 1.1  2008/01/02 13:05:17  sgronchi
# Definitions et regles generales
#
#**********************************************************************************************************************
ifndef $(UCLINUX)
UCLINUX = ../../sys/uClinux
endif

cleanVersion : 
	rm -rf cpu432/obj/pmv/LCR_TST.o
	
allExecUClinux: cleanVersion allExec
	@if [ ! -d $(UCLINUX)/romfs/home ] ; then \
		mkdir -p $(UCLINUX)/romfs/home ; \
	fi;
	@if [ ! -d $(UCLINUX)/romfs/etc ] ; then \
		mkdir -p $(UCLINUX)/romfs/etc ; \
	fi;
	@if [ ! -d $(UCLINUX)/romfs/usr ] ; then \
		mkdir -p $(UCLINUX)/romfs/usr ; \
	fi;
	@if [ ! -d $(UCLINUX)/romfs/usr/bin ] ; then \
		mkdir -p $(UCLINUX)/romfs/usr/bin ; \
	fi;
	@if [ ! -L $(UCLINUX)/config.arch ] ; then \
		(cd $(UCLINUX) ; ln -s vendors/SIAT/CPU432/config.arch . )  \
	fi;
	@if [ ! -L $(UCLINUX)/lib/uClibc ] ; then \
		(cd $(UCLINUX)/lib ; ln -s ../uClibc . )  \
	fi;
	cp $(BIN_DIR)/pmvWatchdog $(UCLINUX)/romfs/home/
	cp $(BIN_DIR)/pmvMain $(UCLINUX)/romfs/home/
	cp $(BIN_DIR)/pmvLanceur $(UCLINUX)/romfs/home/
	cp $(BIN_DIR)/copieSysteme $(UCLINUX)/romfs/usr/bin/
	ln -fs ./copieSysteme $(UCLINUX)/romfs/usr/bin/utilSysteme
	#cp init.cfg $(UCLINUX)/romfs/etc
	cp romfs/etc/eth.cfg $(UCLINUX)/romfs/etc/eth.cfg
	cp romfs/etc/ftpd.conf $(UCLINUX)/romfs/etc/ftpd.conf
	cp romfs/etc/passwd $(UCLINUX)/vendors/Generic/big/passwd
	cp romfs/etc/rc 	$(UCLINUX)/vendors/Generic/big
	(cd $(UCLINUX) ; make -f setconfig.mk loadcfg CFG_NAME=$(VARIANTE_UCLINUX))
	(cd $(UCLINUX) ; make)
	
uClinux:
	(make allExecUClinux CIBLE=cpu432)

uForce:
	(make CIBLE=cpu432 cleanall allExecUClinux)

uClean:
	(make CIBLE=cpu432 cleanall)
