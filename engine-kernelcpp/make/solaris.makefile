# $Id: solaris.makefile 13795 2009-06-23 11:13:12Z sos $
# Einstellungen nur f\374r Linux


include $(PROD_DIR)/make/gnu.makefile

#CFLAGS += -O2
#CFLAGS += -O0
CFLAGS += -fPIC

ifeq ($(cpuArchitecture),x86)
CFLAGS += -m32
CFLAGS += -D_FILE_OFFSET_BITS=64
LINK_FLAGS += -m32
LIBS += -liconv
else
CFLAGS += -m64
LINK_FLAGS += -m64
endif	

LIBS += -lsocket
LIBS += -lthread

# Damit offene Referenzen nicht bem�ngelt werden:
#LINK_DYNAMIC = -Xlinker -b


#CCPP_LIB_DIR = $(dir $(shell $(CCPP) -print-file-name=libgcc.a))
#LINKER = ld -L$(CCPP_LIB_DIR) -L/usr/local/lib -lstdc++ -lgcc_s -ldl -lnsl -lrt $(CCPP_LIB_DIR)/crt1.o $(CCPP_LIB_DIR)/crti.o $(CCPP_LIB_DIR)/crtbegin.o
#LINKER = ld -L$(dir $(shell $(CCPP) -print-file-name=libgcc.a)) -L/usr/local/lib -lstdc++ -lgcc_s -ldl -lnsl -lrt
INCLUDES += -I$(PROD_DIR)/LINKS/java/include/solaris
