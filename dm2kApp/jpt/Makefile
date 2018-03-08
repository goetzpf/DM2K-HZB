# Makefile.Host
TOP = ../..
include $(TOP)/configure/CONFIG

CMPLR = STRICT
#ANSI = GCC

USR_CFLAGS         = -I$(X11_INC) -I$(MOTIF_INC)
ifeq ($(T_A),hp700)
USR_CFLAGS += +DAportable
endif

INC += \
	At.h\
	Plot.h\
	Plotter.h\
	Scale.h\
	XrtGraph.h

LIBSRCS = AtConverters.c \
	AxisCore.c \
	DataHandle.c \
	Plot.c \
	Plotter1.c \
	PlotterZoom.c \
	Scale.c \
	Text.c \
	Text2.c \
	XYAxis.c \
	XYLinePlot.c \
	XYPlot.c \
	strcasecmp.c

LIBRARY=Jpt

SHARED_LIBRARIES = NO

LIB_SYS_LIBS += Xm Xt X11

#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

APPNAME = jptTest
RULES_HOST_REQ_REV = 20000

include $(TOP)/configure/RULES

#----------------------------------------
#  ADD RULES AFTER THIS LINE
