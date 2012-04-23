/*
*****************************************************************
                          COPYRIGHT NOTIFICATION
*****************************************************************

THE FOLLOWING IS A NOTICE OF COPYRIGHT, AVAILABILITY OF THE CODE,
AND DISCLAIMER WHICH MUST BE INCLUDED IN THE PROLOGUE OF THE CODE
AND IN ALL SOURCE LISTINGS OF THE CODE.

(C)  COPYRIGHT 1993 UNIVERSITY OF CHICAGO

Argonne National Laboratory (ANL), with facilities in the States of
Illinois and Idaho, is owned by the United States Government, and
operated by the University of Chicago under provision of a contract
with the Department of Energy.

Portions of this material resulted from work developed under a U.S.
Government contract and are subject to the following license:  For
a period of five years from March 30, 1993, the Government is
granted for itself and others acting on its behalf a paid-up,
nonexclusive, irrevocable worldwide license in this computer
software to reproduce, prepare derivative works, and perform
publicly and display publicly.  With the approval of DOE, this
period may be renewed for two additional five year periods.
Following the expiration of this period or periods, the Government
is granted for itself and others acting on its behalf, a paid-up,
nonexclusive, irrevocable worldwide license in this computer
software to reproduce, prepare derivative works, distribute copies
to the public, perform publicly and display publicly, and to permit
others to do so.

*****************************************************************
                                DISCLAIMER
*****************************************************************

NEITHER THE UNITED STATES GOVERNMENT NOR ANY AGENCY THEREOF, NOR
THE UNIVERSITY OF CHICAGO, NOR ANY OF THEIR EMPLOYEES OR OFFICERS,
MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL
LIABILITY OR RESPONSIBILITY FOR THE ACCURACY, COMPLETENESS, OR
USEFULNESS OF ANY INFORMATION, APPARATUS, PRODUCT, OR PROCESS
DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT INFRINGE PRIVATELY
OWNED RIGHTS.

*****************************************************************
LICENSING INQUIRIES MAY BE DIRECTED TO THE INDUSTRIAL TECHNOLOGY
DEVELOPMENT CENTER AT ARGONNE NATIONAL LABORATORY (708-252-2000).
*/
/*****************************************************************************
 *
 *     Original Author : Mark Andersion
 *     Current Author  : Frederick Vong
 *
 * Modification Log:
 * -----------------
 * .01  03-01-95        vong    2.0.0 release
 * .02  09-05-95        vong    2.1.0 release
 *
 *****************************************************************************
 *
 *      03-18-97        Fabien  Addition of various kind of message buttons
 *                              Capability to run dm2k in READ ONLY mode
 *                              defined by -readonly option
 *                              or by environment variable defined
 *
 *****************************************************************************
*/

#ifndef __DM2K_H__
#define __DM2K_H__

#undef __MONITOR_CA_PEND_EVENT__
#define SUPPORT_0201XX_FILE_FORMAT


/* STANDARDS CONFORMANCE: AES, XPG2, XPG3, XPG4, POSIX.1, POSIX.2 */
#include <unistd.h>
#include <limits.h>
/*#include <float.h>	XPG4 limits.h doesn't include float.h */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* I hope this catches all cases... T.B. */

/* for dumb SUNOS and GNU... */
#ifndef FLT_MAX		/* FLT_MAX is supposed to be in limits.h/float.h */
#  ifdef SOLARIS
#    include <float.h>
#  else
#    define FLT_MAX ((float)1.e+30)
#  endif
#endif
#ifndef M_PI		/* similarly M_PI should be in math.h */
#  define M_PI    3.14159265358979323846
#endif

#ifndef MAXFLOAT
# ifdef FLT_MAX
#  define MAXFLOAT FLT_MAX
# else
#  ifdef HUGE
#   define MAXFLOAT HUGE
#  else

#  endif
# endif
#endif


#if defined(SYSV) || defined(SVR4) || defined(SOLARIS)
#include <string.h>
#else
#include <strings.h>
#endif

#define DM2K_DIALOG_OK_BUTTON     1
#define DM2K_DIALOG_CANCEL_BUTTON 2
#define DM2K_DIALOG_HELP_BUTTON   3
#define DM2K_DIALOG_MESSAGE_LABEL 4
#define DM2K_DIALOG_CUSTEM_BUTTON 5
#define DM2K_DIALOG_TITLE         6

#define DM2K_ADL_FILE_JUMP_VERSION  20200

#define FILE_NEW_BTN           0
#define FILE_OPEN_BTN          1
#define FILE_SAVE_BTN          2
#define FILE_SAVE_AS_BTN       3
#define FILE_CLOSE_BTN         4
#define FILE_SEPARATOR1        5
#define FILE_FACEPLATELOAD_BTN 6
#define FILE_SEPARATOR2        7
#define FILE_PRINT_SETUP_BTN   8
#define FILE_PRINT_BTN         9
#define FILE_SEPARATOR3       10
#define FILE_EXIT_BTN         11
#define FILE_DISPLAY_LIST_ENV 12
/* DON'T EVER APPEND ENTRIES TO THIS LIST, BECAUSE ENTRIES >
   FILE_DISPLAY_LIST_ENV ARE RESERVED !!! */

#define FULLPATHNAME_SIZE 1024

#ifdef ALLOCATE_STORAGE
#define EXTERN
#else
#define EXTERN extern
#endif

/* environment variable name for a READ ONLY execution */
#define DISPLAY_LIST_ENV	"EPICS_DISPLAY_PATH"
#define USER_ENV		"USER"
#define PIXMAP_PATH_ENV		"PIXMAP_PATH"

EXTERN char *MAIN_name;
EXTERN char *MAIN_Name;
EXTERN char *MAIN_NAME;
EXTERN char READ_ONLY_ENV[32];
EXTERN char DM2K_WM_POSITION_POLICY_ENV[32];
EXTERN char DM2K_COLOR_RULE_ENV[32];
EXTERN char DM2K_GRAPHIC_RULE_ENV[32];
EXTERN char EPICS_DM2K_DEFAULT_MB3_DISPLAY_ENV[32];
EXTERN char DM2K_MAIL_CMD_ENV[32];
EXTERN char DM2K_HELP_ENV[32];
EXTERN char DM2K_HELP_PATH_ENV[32];


#ifdef __cplusplus
extern "C" {
#endif

/*
 * X/Xt/Xm includes, globals
 */
#include "xtParams.h"
#include <Xm/VirtKeys.h>

/*
 * DM2K includes
 */
#include "dm2kCA.h"
#include "epicsVersion.h"

#include "dm2kWidget.h"
#include "parse.h"
#include "xgif.h"


#include "dm2kVersion.h"

/***
 *** and on with the rest of Dm2k
 ***/

/*
 * define the help layers
 */
#define HELP_MAIN 0



EXTERN char *programName;
/*
 * global widgets (all permanent shells, most MWs, etc )
 */
EXTERN Widget mainShell, mainMW;
EXTERN Widget objectS, objectMW;
EXTERN Widget resourceS, resourceMW;
EXTERN Widget colorS, colorMW;
EXTERN Widget channelS, channelMW;
EXTERN Widget variableS, variableMW;
/* shells for related display, shell command,
	cartesian plot and strip chart data vectors */
EXTERN Widget relatedDisplayS, shellCommandS, cartesianPlotS,
	cartesianPlotAxisS, stripChartS;
EXTERN Widget cpAxisForm, executeTimeCartesianPlotWidget;

EXTERN Widget exitQD, saveAsPD;

/* the global Help Information Dialog */
EXTERN Widget helpS, helpMessageBox;


/* in main shell: labels on bulletin board for current display information */
EXTERN Widget statusBB, displayL, nElementsL, nColorsL;

/* currently specified image type (from ObjectPalette's OpenFSD) */
EXTERN ImageType imageType;

/* resource bundle stuff */
#define SELECTION_BUNDLE 0
EXTERN int resourceBundleCounter;
extern void utilPrint(Display *, Window, char *);

EXTERN XtWorkProcId dm2kWorkProcId;
EXTERN long dm2kUpdateRequestCount;
EXTERN long dm2kCAEventCount, dm2kScreenUpdateCount, dm2kUpdateMissedCount;
EXTERN Widget caStudyLabel;
EXTERN XtIntervalId dm2kStatusIntervalId;
EXTERN Boolean Dm2kUseNewFileFormat;

EXTERN Boolean globalDm2kReadOnly;

typedef struct menuEntry{
  char*           label;
  WidgetClass*    widgetClass;
  char            mnemonic;
  char*           accelerator;
  char*           accText;
  Widget          widget;
  XtCallbackProc  callback;
  XtPointer       callbackData;
  struct menuEntry *subItems;
} menuEntry_t;

typedef void(*dm2kExecProc)(DisplayInfo *,DlElement *);
typedef void(*dm2kWriteProc)(FILE *,DlElement *,int);
typedef void(*dm2kSetGetProc)(ResourceBundle *, DlElement *);

#define SET_IF_NOT_MAXFLOAT(l,r) if ((r) != MAXFLOAT) l = (r)
#define SET_IF_NOT_MINUSONE(l,r) if ((r) != -1) l = (r)

#include "proto.h"
#include "dm2kInitTask.h"
#include "faceplateBase.h"

/* pixmap names : must be accessible by program according to Motif rules:

 rectangle25
 oval25
 arc25
 text25
 line25
 polyline25
 polygon25
 bezierCurve25

 meter25
 bar25
 indicator25
 textUpdate25
 stripChart25
 cartesianPlot25
 surfacePlot25

 choiceButton25
 messageButton25
 menu25
 textEntry25
 valuator25

 relatedDisplay25
 shellCommand25
 */

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif  /* __DM2K_H__ */
