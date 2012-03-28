#Makefile at top of application tree
TOP = .
RULES=$(TOP)
RSYNC_DIST_VERSION_FILE=doc/version
DOC_INDEX_TITLE=BESSY extensions of EPICS build rules
include $(TOP)/configure/CONFIG
DIRS := $(DIRS) $(filter-out $(DIRS), configure)
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *App))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *app))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard iocBoot))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard iocboot))
include $(TOP)/configure/RULES_TOP

# list distclean and all target
all:
distclean:
# list all rsync-dist targets for zsh completion ;)
rsync-dist:
rsync-dist.DOC:
add-names:
add-names.DOC:
activate-version:
activate-version.DOC:
deprecate-version:
deprecate-version.DOC:
list-all-versions:
list-all-versions.DOC:
list-active-versions:
list-active-versions.DOC:
unlock:
unlock.DOC:
force-unlock:
force-unlock.DOC:
