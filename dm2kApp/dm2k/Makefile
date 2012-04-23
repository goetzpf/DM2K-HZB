#	Makefile.Host  for  base/src/sample
#
#
#	Sample Makefile.Host showing all possible entries
#	that are allowed using RULES.Host.
#

TOP = ../..
include $(TOP)/configure/CONFIG

##### Variables to control build of dm2k
### 
### XRTHOME		holds the directory of the XRT/graph installation.
###			If this variable is set, dm2k is built with
###			XRT/graph support instead of JPT.
###
### STATIC_BUILD	controls whether to build dm2k statically or with
### 			shared libraries. The default setting is somewhere
### 			in your local/global config-directory.
###
### PURIFY		To debug dm2k with purify, set this variable to the
###			command-name to prepend to the linker-call
###			(e.g. "purify"). Setting PURIFY automatically sets
###			STATIC_BUILD to YES!
###
### The name of the produced binary is constructed the following way.
###
### 	+ The stem is always "dm2k"
###	+ if dm2k is built with cdev-support, "_cdev" is appended
###	+ if dm2k is built statically, ".static" is appended
###	+ if dm2k is to be debugged with purify ".pure" is appended


CMPLR = STRICT
# ANSI = GCC

#	Add-on CFLAGS that are needed by this Makefile.
#	(If possible, all system specific flags should be
#	 defined in CONFIG.Host.$(ARCH_CLASS))
#
#       These CFLAGS rules also apply to these Makefile-variables:
#		CXXFLAGS    C++ flags
#		LDFLAGS     link flags
#
#	This is used on all systems:
#USR_CFLAGS	  += -G
USR_CFLAGS        += -I../../graphX -I../../xc -I.
USR_CFLAGS	  += -DCONFIG_NO_TEAR_OFF
# If you want to be asked `Do you really want to Exit?' you have to define this
# USR_CFLAGS	  += -DPROMPT_TO_EXIT=1
# instead of
USR_CFLAGS	  += -DPROMPT_TO_EXIT=0

USR_CFLAGS += -DBUILT_AT="\"$(shell date)\"" -DBUILT_BY="\"$(USER)\""

USR_CXXFLAGS := $(USR_CFLAGS)

#	This is added to the above, but only for ARCH_CLASS=BSD:
# USR_CFLAGS_BSD     = -DVERSION='Berkeley enhanced'
#	..only for WIN32:
# USR_CFLAGS_WIN32   = -DVERSION='WIN32 port'
#
#	-nil- is special:
#	if USR_CFLAGS_SYSV was undefined or empty, .._DEFAULT would have
#	been used.
#	To indicate
#		"yes, there is a special USR_CFLAGS for SYSV, but it's empty"
#	you have to set it to -nil-:
# USR_CFLAGS_SYSV    = -nil-
#	.. for all other arch classes:
# USR_CFLAGS_DEFAULT = -DVERSION='generic Unix'

#	CFLAGS that are only used to compile a_file.c or a_file.cc:
#
# a_file_CFLAGS      = -DIN_A_FILE
# a_file_CFLAGS_WIN32   = -DVERSION='WIN32 port'

#	---------------------------------------------------------
#	general rule for all .c .cc .h .hh files and scripts:
#
#	In here you supply just the filename without '../' etc.
#	While building in an O.xxx subdir, the
#	sources are extracted from either the
#	             '..'
#	dir or - if it exists - the dir
#	             '../$(ARCH_CLASS)'
#	is preferred.
#	---------------------------------------------------------


#	includes to install from this Makefile
#
#	again: if INC_$(ARCH_CLASS) is defined, it is added to INC,
#	otherwise INC_DEFAULT (if defined) is added:
#
# INC_DEFAULT = for_all_but_WIN32_or_hp700.h
# INC_WIN32   = only_for_WIN32.h
# INC_hp700   = -nil-		# hp700 uses no special include
# INC         = file.h

# --------------------------------------------------------------------
#	defining a library
# --------------------------------------------------------------------
#
#	Contents of a library are specified via LIBSRCS.
#	From this the platform specific object names (.o, .obj, ...)
#	are derived automatically.
#
#	Platform specific objects:
#	use LIBOBJS_$(ARCH_CLASS)  or  .._DEFAULT
#
#	Platform specific files can also be put in
#	separate os/ARCH_CLASS directories!
#
#	For almost every file the seach order is:
#	./os/ARCH_CLASS
#	./os/generic
#	.
#	So usually only LIBSRCS should be sufficient!
#
# LIBSRCS         = placeholder.c productDescriptionShell.c
# LIBSRCS_DEFAULT = posix.c
# LIBSRCS_WIN32   = win32_special.c
# LIBSRCS_BSD     = -nil-

#	Library to build:
#	lib$(LIBRARY).a  or   ..dll/..exp/..lib
#
#	Currently you can build only one (1) lib per Makefile.Host!
#	
# LIBRARY=Unix

# if SHARED_LIBRARIES is YES then shared and archive libraries will
#	both be built 
# SHARED_LIBRARIES = YES
SHARED_LIBRARIES = NO
#
#	Library version
# SHRLIB_VERSION = 
#	On WIN32 results in /version:$(SHRLIB_VERSION) link option
#	On Unix type hosts .$(SHRLIB_VERSION) is appended to library name  

# --------------------------------------------------------------------
#	defining products (executable programs)
# --------------------------------------------------------------------
#
#	if SRCS is undefined, it defaults to $(PROD).c 
SRCS=	actions.c \
	amDialog.c \
	callbacks.c \
	channelPalette.c \
	colorPalette.c \
	display.c \
	dmInit.c \
	eventHandlers.c \
	faceplateBase.c \
	faceplateGUI.c \
	faceplateGUICallback.c \
	faceplateDm2k.c \
	getArchiveData.c \
	help.c \
	dm2k.c \
	dm2kArc.c \
	dm2kBar.c \
	dm2kByte.c \
	dm2kCartesianPlot.c \
	dm2kChoiceButtons.c \
	dm2kCommon.c \
	dm2kComposite.c \
	dm2kControl.c \
	dm2kDelay.c \
	dm2kDynSymbol.c \
	dm2kImage.c \
	dm2kIndicator.c \
	dm2kMenu.c \
	dm2kMessageButton.c \
	dm2kMeter.c \
	dm2kMonitor.c \
	dm2kOval.c \
	dm2kPixmap.c \
	dm2kPolygon.c \
	dm2kPolyline.c \
	dm2kRectangle.c \
	dm2kRelatedDisplay.c \
	dm2kShellCommand.c \
	dm2kStripChart.c \
	dm2kText.c \
	dm2kTextEntry.c \
	dm2kTextUpdate.c \
	dm2kValuator.c \
	dm2kWidget.c \
	objectPalette.c \
	productDescription.c \
	resourcePalette.c \
	shared.c \
	updateMonitors.c \
	utils.c \
	xgif.c \
	clipbd.c

#	SRCS that are only used for PROD a_file
#
# a_file_SRCS = aa.c bb.c

#	EPICS libs needed to link PROD, TESTPROD and sharable library
#
#	note that DLL_LIBS (the libraries needed to link a shareable
#	library) is created by default from the PROD/SYS libraries specified 
#	below minus the name of the sharable library (LIBRARY)
#	
#
# for all systems:
# PROD_LIBS       = ca Com
# for most systems:
# PROD_LIBS_DEFAULT = mathlib
# PROD_LIBS_WIN32   = -nil-

PROD = dm2k

ifeq ($(STATIC_BUILD),YES)
 PROD := $(PROD).static
 ifneq ($(T_A),solaris)
  LDFLAGS += -Wl,-a,archive
 endif
endif
#USR_LDFLAGS += /usr/lib/libdld.1

ifdef PURIFY
  PROD := $(PROD).pure
endif

ifdef XRTHOME 
  USR_CFLAGS += -DXRT_DOCSTR="\"XRT/graph version 4.2.0\""
  USR_CFLAGS += -DUSE_XRT -I$(XRTHOME)/include/Xm
  USR_CXXFLAGS += -DUSE_XRT -I$(XRTHOME)/include/Xm
  xrtm_DIR = $(XRTHOME)/lib
  pdsutil_DIR = $(XRTHOME)/lib
  PROD_LIBS += xrtm pdsutil
else
  JPT_CFLAGS  = -DXRT_VERSION=2 -DXRT_TYPE_PLOT=1 -DXRT_TYPE_BAR=2 -DUSE_XRT -DXRT_DOCSTR="\"Jpt Jefferson Lab Plotting Toolkit (by Ge Lei)\""
  USR_CFLAGS += $(JPT_CFLAGS)
  USR_CCFLAGS += $(JPT_CFLAGS)
  PROD_LIBS += Jpt
  Jpt_DIR = $(INSTALL_LIB)
endif

ifdef Xpm_INC
  USR_CFLAGS += -DUSE_XPM -I$(Xpm_INC) -I/opt/xpm/include
  USR_CXXFLAGS += -DUSE_XPM
  SYS_PROD_LIBS += Xpm
endif

PROD_LIBS += ca Com
ca_DIR = $(EPICS_BASE_LIB)
Com_DIR = $(EPICS_BASE_LIB)
# USR_LDLIBS += -L$(EPICS_BASE_LIB) -lca -lCom
SRCS += dm2kCA.c

#	system libs needed to link PROD, TESTPROD and sharable library
#
# for all systems:
#USR_CFLAGS += -I$(MOTIF_INC) -I$(X11_INC)
#USR_CXXFLAGS += -I$(MOTIF_INC) -I$(X11_INC) -I/usr/contrib/X11R6/include
SYS_PROD_LIBS += m
# for most systems:
# SYS_PROD_LIBS_DEFAULT = foolib
# SYS_PROD_LIBS_WIN32   = -nil-
SYS_PROD_LIBS_solaris += gen

#	other libs needed to link PROD, TESTPROD and sharable library
#
# for all systems:
PROD_LIBS += Unix ANLwidgets PrintUtils GraphX 

Xmu_INC = /usr/contrib/X11R6/include

SYS_PROD_LIBS += Xm Xt X11 Xext Xmu Xp SM ICE
USR_CFLAGS += -I$(X11_INC) -I$(Xmu_INC) # -I$(Xm_INC)
USR_LDFLAGS += -L$(MOTIF_LIB) -L/usr/lib -L/usr/contrib/X11R6/lib
USR_LDFLAGS_hpux = -L/usr/lib/X11R6
#USR_LIBS += dl

Unix_DIR = $(INSTALL_LIB)

# for most systems:
# USR_LIBS_DEFAULT = foolib
# USR_LIBS_WIN32   = -nil-
# foolib_DIR = $(FOO_LIB)

#	Product,
#	may be   caRepeater.o -> caRepeater
#	or       caRepeater.obj -> caRepeater.exe




# PROD_DEFAULT = product_for_rest
# PROD_WIN32   = product_only_for_WIN32
# PROD_BSD     = product_only_for_BSD
# PROD_SYSV    = product_only_for_SYSV

#	Product version
# PROD_VERSION = 
#	On WIN32 results in /version:$(SHRLIB_VERSION) link option
#	On Unix type hosts PROD_VERSION) is ignored

#	Scripts to install
#
#	If there is  both  ../$(SCRIPT) and  ../$(ARCH_CLASS)/$(SCRIPT),
#	the latter, system specific version will be installed!
#
# SCRIPTS_DEFAULT = script_for_rest
# SCRIPTS_WIN32   = script_only_for_WIN32
# SCRIPTS_BSD     = script_only_for_BSD
# SCRIPTS         = script

#	if you want to build products locally without installing:
# TESTPROD = test

# put all definitions before the following include line
# put all rules after the following include line

HOST_OPT=NO

build: enums.h

include $(TOP)/configure/RULES

enums.h: ../enums
	perl ../makeEnums.pl <../enums >enums.h

$(PRODNAME): $(PRODNAME_OBJS) $(PRODNAME_RESS) $(PROD_DEPLIBS)
	@$(RM) $@
	$(LINK.cpp)
ifdef XRTHOME 
	@echo Patching for XRT-License...
	@scp -q $(PRODNAME) $(XRT_HOST):/tmp/dm2k
	@ssh $(XRT_HOST) $(XRTHOME)/bin/xrt_auth /tmp/dm2k
	@scp -q $(XRT_HOST):/tmp/dm2k $(PRODNAME)
	@echo done.
endif

#	EOF Makefile.Host
