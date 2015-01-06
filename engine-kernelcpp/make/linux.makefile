# linux.mak $Id: linux.makefile 14247 2011-05-05 04:42:13Z jz $
# Einstellungen nur f�r Linux

include $(PROD_DIR)/make/gnu.makefile

ifeq ($(cpuArchitecture),x86)
CFLAGS += -m32
CFLAGS += -D_FILE_OFFSET_BITS=64
LINK_FLAGS += -m32
else
CFLAGS += -m64
LINK_FLAGS += -m64
endif	

COPTIMIZE += -O3


# MAKE_UBUNTU setzen
CFLAGS += $(if $(shell test -e /usr/include/ansidecl.h && echo 1),,-DMAKE_UBUNTU)

INCLUDES += -I$(JAVA_HOME)/include/linux

# LINKS/include kann auf Suse 8.0 /usr/include verweisen, damit beim Ablauf nicht die neueste glibc verlangt wird.
INCLUDES += $(if $(shell test -e $(PROD_DIR)/LINKS/include && echo 1),-I$(PROD_DIR)/LINKS/include -I$(PROD_DIR)/LINKS/include/linux,)


# libstdc++ statisch einbinden (geht leider nicht auf Solaris)
#LIBS += -nodefaultlibs `g++ -print-file-name=libstdc++.a` `g++ -print-file-name=libgcc_eh.a` -lm -lgcc -lc -ldl -lcrypt

LIBS += -lpthread
LIBS += -ldl
