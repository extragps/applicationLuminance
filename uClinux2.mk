#**********************************************************************************************************************
# SIAT - Copyright SIAT
#----------------------------------------------------------------------------------------------------------------------
# fichier     : 
# description : 
# prefixe     : 
#----------------------------------------------------------------------------------------------------------------------
# Auteur      : 
# Date        : 
#**********************************************************************************************************************
# definition emplacement noyau
ifndef $(UCLINUX)
UCLINUX := ../../sys/uClinux
endif
UCLINUX_CFG := config.pip

HOST_CIBLE := lx86_64

# chemin vers les utilitaires
CPU432TOOLS_DIR := ../cpu432Tools

WWW_RSC := ../c4sWebServer/www/build
ROMFS := $(UCLINUX)/romfs

TARGET_DIR := ../../binaries
ifndef $(TARGET_NAME)
TARGET_NAME := c4s
endif

#----------------------------------------------------------------------------------------------------------------------
# Nettoyage systeme Linux
cleansys:
	(cd $(UCLINUX) ; ./ucclean)

#----------------------------------------------------------------------------------------------------------------------
# Construction globale de la cible
buildUClinux: sysCreateLinks build buildsys

#----------------------------------------------------------------------------------------------------------------------
# Chargement configuration uClinux
#
sysCreateLinks:
	(cd $(UCLINUX) ; ./ucload $(UCLINUX_CFG))

#----------------------------------------------------------------------------------------------------------------------
# construction des outils de generation hote
buildhost:
	@echo
	@echo "================================================================================"
	@echo "==== Generation des outils de generation ..."
	@echo "================================================================================"
	(cd $(CPU432TOOLS_DIR) ; make build CIBLE=$(HOST_CIBLE) -s)
	
#----------------------------------------------------------------------------------------------------------------------
# construction utilitaires
buildtools:
	@echo
	@echo "================================================================================"
	@echo "==== Generation des utilitaires"
	@echo "================================================================================"
	(cd $(CPU432TOOLS_DIR) ; make build CIBLE=$(CIBLE) -s)

#----------------------------------------------------------------------------------------------------------------------
# Mise a jour du systeme de fichiers
#
preparesys:
	@echo
	@echo "================================================================================"
	@echo "==== Construction du systeme de fichiers"
	@echo "================================================================================"
	# preparation systeme
	mkdir -p $(UCLINUX)/romfs/home ;
	@if [ ! -d $(UCLINUX)/romfs/usr/bin ] ; then \
		mkdir -p $(UCLINUX)/romfs/usr/bin ; \
	fi;
	@if [ ! -d $(UCLINUX)/romfs/etc ] ; then \
		mkdir -p $(UCLINUX)/romfs/etc ; \
	fi;
	@if [ ! -L $(UCLINUX)/config.arch ] ; then \
		(cd $(UCLINUX) ; ln -s vendors/SIAT/CPU432/config.arch . )  \
	fi;
	@if [ ! -L $(UCLINUX)/lib/uClibc ] ; then \
		(cd $(UCLINUX)/lib ; ln -s ../uClibc . )  \
	fi;
	
	# configuration systeme	
	cp romfs/etc/eth.cfg $(UCLINUX)/romfs/etc/eth.cfg
	cp romfs/etc/ftpd.conf $(UCLINUX)/romfs/etc/ftpd.conf
	cp romfs/etc/group $(UCLINUX)/romfs/etc/group
	cp romfs/etc/passwd $(UCLINUX)/vendors/Generic/big/
	cp romfs/etc/rc 	$(UCLINUX)/vendors/Generic/big

	# copie application et utilitaires
	mkdir -p $(ROMFS)/usr/bin
	rm -f $(ROMFS)/usr/bin/*
	cp $(BIN_DIR)/c4s $(ROMFS)/usr/bin
	cp $(BIN_DIR)/c4busybox $(ROMFS)/usr/bin
	
	mkdir -p $(ROMFS)/usr/http
	rm -rf $(ROMFS)/usr/http/*
	cp $(WWW_RSC)/httpd.conf $(ROMFS)/usr/http
	mkdir -p $(ROMFS)/usr/http/rsc/font
	cp -R $(WWW_RSC)/rsc/font/*.* $(ROMFS)/usr/http/rsc/font/
	cp $(WWW_RSC)/rsc/*.* $(ROMFS)/usr/http/rsc/
	cp $(WWW_RSC)/index.html $(ROMFS)/usr/http	

	ln -s c4busybox $(ROMFS)/usr/bin/cpu432Watchdog
	ln -s c4busybox $(ROMFS)/usr/bin/cpu432Launcher
	ln -s c4busybox $(ROMFS)/usr/bin/copieSysteme
	ln -s c4busybox $(ROMFS)/usr/bin/c4cmd
	ln -s c4busybox $(ROMFS)/usr/bin/c4syscopy
	ln -s c4busybox $(ROMFS)/usr/bin/c4su3
	ln -s c4busybox $(ROMFS)/usr/bin/c4watchdog
	ln -s c4busybox $(ROMFS)/usr/bin/c4shttpd
	
#----------------------------------------------------------------------------------------------------------------------
# Creation de l'image
#
buildsys: buildhost sysCreateLinks buildtools preparesys
	@echo
	@echo "================================================================================"
	@echo "==== Generation de l'image systeme"
	@echo "================================================================================"	
	(cd $(UCLINUX) ; ./ucload $(UCLINUX_CFG) ; ./ucbuild)

#----------------------------------------------------------------------------------------------------------------------
# Generation du fichier de mise a jour
#
copyimage:
	@echo
	@echo "================================================================================"
	@echo "==== Generation du fichier de mise a jour ($(TARGET_DIR)/$(TARGET_NAME).c4f)"
	@echo "================================================================================"
	mkdir -p $(TARGET_DIR)
	cp $(UCLINUX)/images/image.bin $(TARGET_DIR)/$(TARGET_NAME).bin
	cp $(CPU432TOOLS_DIR)/build/$(HOST_CIBLE)/bin/c4fmaker $(TARGET_DIR)
	(cd $(TARGET_DIR) ; ./c4fmaker -i $(TARGET_NAME).bin -s 0x700000 -t UCLINUX -o $(TARGET_NAME).c4f)
	rm $(TARGET_DIR)/c4fmaker
