# Makefile.Host
TOP = ..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

APPNAME = RulesFiles

#USR_CFLAGS += 

ETC += ColorRules.CTL
ETC += GraphicRules.CTL

include $(TOP)/configure/RULES

#----------------------------------------
#  ADD RULES AFTER THIS LINE

$(eval $(call GENERIC_INSTALL_RULE,CTL,request,$(INSTALL_ETC)))
