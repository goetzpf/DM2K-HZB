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
 *                              - fix the file closing problem
 *                                when using the 'file->close' memu button.
 * .03  09-08-95        vong    conform to c++ syntax
 * .04  09-28-95        vong    add back Ctrl-X to the 'file' menu for 'Exit'
 *                              entry.
 * .05  10-02-95        vong    handle the special case .template suffix.
 * .06  10-25-95        vong    add the primitive color rule set
 *
 *****************************************************************************
 *
 *      24-02-97    Fabien  call createPopupDisplayMenu from dm2k.c
 *                          DM2K crash on Linux X server when popup menu
 *                          are direct child of shell !
 *
 *****************************************************************************
*/

#define ALLOCATE_STORAGE
#include "dm2k.h"
#include <Xm/RepType.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#include <errno.h>

#ifdef USE_XPM
# include <X11/xpm.h>
# include "dm2k_icon.xpm"
# include "dm2k_logo.xpm"
# include "dm2k_logo_bw.xpm"
#endif
#include "icon25"

/* for X property cleanup */
#include <signal.h>
#include <Xm/MwmUtil.h>
#include <X11/IntrinsicP.h>

#include "faceplateGUI.h"

#include "clipbd.h"

#define DM2K_DIALOG_MAX_CUSTEM_BUTTON 40

#define LOCAL_PATH_MAX  1023
#define HOT_SPOT_WIDTH 24

#define N_MAX_MENU_ELES 20
#define N_MAIN_MENU_ELES 5

#define N_FILE_MENU_ELES 8
#define FILE_BTN_POSN 0

#define PRINTER_SETUP_OK     0
#define PRINTER_SETUP_CANCEL 1
#define PRINTER_SETUP_MAP    2

#define EDIT_BTN_POSN 1
#define EDIT_OBJECT_BTN           0
#define EDIT_CUT_BTN              1
#define EDIT_COPY_BTN             2
#define EDIT_PASTE_BTN            3
#define EDIT_RAISE_BTN            4
#define EDIT_LOWER_BTN            5

#define EDIT_GROUP_BTN            6
#define EDIT_UNGROUP_BTN          7

#define EDIT_ALIGN_BTN            8
#define EDIT_UNSELECT_BTN         9
#define EDIT_SELECT_ALL_BTN      10

#define N_VIEW_MENU_ELES     3
#define VIEW_BTN_POSN        2
#define VIEW_MESSAGE_WINDOW_BTN  1
#define VIEW_STATUS_WINDOW_BTN   2

#define N_ALIGN_MENU_ELES 2
#define ALIGN_BTN_POSN 13

#define N_HORIZ_ALIGN_MENU_ELES 3
#define HORIZ_ALIGN_BTN_POSN 0

#define N_VERT_ALIGN_MENU_ELES 3
#define VERT_ALIGN_BTN_POSN 1

#define N_PALETTES_MENU_ELES 3
#define PALETTES_BTN_POSN 3

#define PALETTES_OBJECT_BTN 0
#define PALETTES_RESOURCE_BTN 1
#define PALETTES_COLOR_BTN 2

#define TOOLS_FACEPLATE_BTN 0
#define TOOLS_DUMPDISPLAYINFO_BTN 1

typedef enum
{
  HELP_ON_VERSION_BTN
} HelpButton_t;

const char DEVELOPED_BY[] = 
"developed as edd/dm                              \n"
"    at LANL by J. O. Hill                        \n"
"upgraded to medm                                 \n"
"    at APS/ANL by M. Anderson, F. Vong, K. Evans \n"
"upgraded to medm/DESY (medm 2.4)                 \n"
"    at DESY by M. Clausen, V. Romanovski, X. Geng\n"
"    at CERN by F. Perriollat                     \n"
"    at BESSY by T. Birke, T. Straumann (PTB)     \n"
"relaunched as dm2k.                              ";

ColorRule * colorRuleHead = NULL;
int         colorRuleCounts = 0;

static void fileMenuSimpleCallback(Widget,XtPointer,XtPointer);
static void editMenuSimpleCallback(Widget,XtPointer,XtPointer);
static void palettesMenuSimpleCallback(Widget,XtPointer,XtPointer);
static void toolsMenuSimpleCallback(Widget,XtPointer,XtPointer);
static void helpMenuSimpleCallback(Widget,XtPointer,XtPointer);
static void alignMenuSimpleCallback(Widget,XtPointer,XtPointer);
static void viewMenuSimpleCallback(Widget,XtPointer,XtPointer);

Boolean motifWMRunning;
static Boolean synchroFlag;
Boolean verboseFlag, silentFlag;
Boolean debugFlag;

Boolean positionPolicyFlag;     /* True if the policy is defined by */ 
                                /* option or environment variable */

Boolean positionPolicyFrame;    /* policy is FRAME/USER if positionPolicyFlag*/
                                /* is True */

Widget mainFilePDM, mainHelpPDM;
static Widget printerSetupDlg = 0;
static int dm2kUseBigCursor = 0;
static int dm2kPromptToExit = PROMPT_TO_EXIT;

void dm2kExit();
Boolean dm2kInitWorkProc(XtPointer cd);

#ifdef __TED__
void GetWorkSpaceList(Widget w);
#endif


/* Translations tables for object information */

static char objectPaletteTranslation[] =
    "<Enter>:objectPaletteEnterWindow()\n\
     <Leave>:objectPaletteLeaveWindow()";

XtTranslations objectPaletteTranstable;

static  XtActionsRec objectPaletteActions[] = {
     { "objectPaletteEnterWindow", (XtActionProc) objectPaletteEnterWindow },
     { "objectPaletteLeaveWindow", (XtActionProc) objectPaletteLeaveWindow }
};

#define CASCADE(t,k,m) { t, &xmCascadeButtonGadgetClass, k, NULL, NULL, NULL, NULL, NULL, m }
#define PUSHB(l,k,ot) { l, &xmPushButtonGadgetClass, k, NULL, NULL, NULL, CALLBACK, (XtPointer) ot, NULL }
#define PUSHB2(l,k,k1,k2,ot) { l, &xmPushButtonGadgetClass, k, k1, k2, NULL, CALLBACK, (XtPointer) ot, NULL }
#define SEPARATOR { "Separator", &xmSeparatorGadgetClass,  '\0', NULL, NULL, NULL, NULL, NULL, NULL }
#define END_MENU { NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL }

#define CALLBACK objectMenuCallback
static menuEntry_t graphicsObjectMenu[] = {
   PUSHB("Text",        'T', DL_Text),
   PUSHB("Rectangle",   'G', DL_Rectangle),
   PUSHB("Line",        'i', DL_Line),
   PUSHB("Polygon",     'P', DL_Polygon),
   PUSHB("Polyline",    'L', DL_Polyline),
   PUSHB("Oval",        'O', DL_Oval),
   PUSHB("Arc",         'A', DL_Arc),
   PUSHB("Image",       'I', DL_Image),
   END_MENU
};

static menuEntry_t monitorsObjectMenu[] = {
   PUSHB("Text Update",    'T', DL_TextUpdate),
   PUSHB("Meter",          'M', DL_Meter),
   PUSHB("Bar",            'B', DL_Bar),
   PUSHB("Byte",           'y', DL_Byte),
   PUSHB("Indicator",      'I', DL_Indicator),
   PUSHB("Strip Chart",    'S', DL_StripChart),
   PUSHB("Cartesian Plot", 'C', DL_CartesianPlot),
   PUSHB("Dynamic Symbol", 'D', DL_DynSymbol),
   END_MENU
};

static menuEntry_t controllersObjectMenu[] = {
   PUSHB("Text Entry",      'T', DL_TextEntry),
   PUSHB("Choice Button",   'C', DL_ChoiceButton),
   PUSHB("Menu",            'M', DL_Menu),
   PUSHB("Valuator",        'V', DL_Valuator),
   PUSHB("Message Button",  'B', DL_MessageButton),
   PUSHB("Related Display", 'R', DL_RelatedDisplay),
   PUSHB("Shell Command",   'S', DL_ShellCommand),
   END_MENU
};

#undef CALLBACK
#define CALLBACK alignMenuSimpleCallback
static menuEntry_t editAlignHorzMenu[] = {
   PUSHB("Left",   'L', HORIZ_LEFT),
   PUSHB("Center", 'C', HORIZ_CENTER),
   PUSHB("Right",  'R', HORIZ_RIGHT),
   END_MENU
};
static menuEntry_t editAlignVertMenu[] = {
   PUSHB("Top",    'T', VERT_TOP),
   PUSHB("Center", 'C', VERT_CENTER),
   PUSHB("Bottom", 'B', VERT_BOTTOM),
   END_MENU
};

static menuEntry_t editAlignEntry[] = {
   CASCADE("Horizontal", 'H', editAlignHorzMenu),
   CASCADE("Vertical", 'V', editAlignVertMenu),
   END_MENU
};
  
static menuEntry_t editObjectMenu[] = {
   CASCADE("Graphics", 'G', graphicsObjectMenu),
   CASCADE("Monitors", 'M', monitorsObjectMenu),
   CASCADE("Controls", 'C', controllersObjectMenu),
   END_MENU
};

#undef CALLBACK
#define CALLBACK editMenuSimpleCallback
static menuEntry_t editMenu[] = {
   PUSHB2("Cut",       't', "Shift<Key>DeleteChar", "Shift+Del",  EDIT_CUT_BTN),
   PUSHB2("Copy",      'C', "Ctrl<Key>InsertChar",  "Ctrl+Ins",   EDIT_COPY_BTN),
   PUSHB2("Paste" ,    'P', "Shift<Key>InsertChar", "Shift+Ins",  EDIT_PASTE_BTN),
   SEPARATOR,		   
   PUSHB("Raise",      'R', EDIT_RAISE_BTN),
   PUSHB("Lower",      'L', EDIT_LOWER_BTN),
   SEPARATOR,		   
   PUSHB("Group",      'G', EDIT_GROUP_BTN),
   PUSHB("Ungroup",    'n', EDIT_UNGROUP_BTN),
   SEPARATOR,		   
   CASCADE("Align",    'A', editAlignEntry),
   SEPARATOR,		   
   PUSHB("Unselect",   'U', EDIT_UNSELECT_BTN),
   PUSHB("Select All", 'S', EDIT_SELECT_ALL_BTN),
  END_MENU
};

/* below constant are defined in dm2kWidget.h file
 */
static menuEntry_t executeMenu[] = {
  {
    "Close display",     &xmPushButtonGadgetClass, 'C', NULL, NULL, NULL,
    executePopupMenuCallback, (XtPointer) EXECUTE_POPUP_MENU_CLOSE_ID, NULL
  },
  {
    "Separator", &xmSeparatorGadgetClass,  ' ', NULL, NULL, NULL,
    NULL,        NULL,                     NULL
  },
  {
    "Print",     &xmPushButtonGadgetClass, 'P', NULL, NULL, NULL,
    executePopupMenuCallback, (XtPointer) EXECUTE_POPUP_MENU_PRINT_ID, NULL
  },
  { 
    "Printer Setup",  &xmPushButtonGadgetClass, 'S', NULL, NULL, NULL,
    executePopupMenuCallback, (XtPointer) EXECUTE_POPUP_MENU_P_SETUP_ID, NULL
  },
  { 
    "Property",  &xmPushButtonGadgetClass, 'P', NULL, NULL, NULL,
    executePopupMenuCallback, (XtPointer) EXECUTE_POPUP_MENU_PROPERTY_ID, NULL
  },
  {
    "Help",  &xmPushButtonGadgetClass, 'H', NULL, NULL, NULL,
    executePopupMenuCallback, (XtPointer) EXECUTE_POPUP_MENU_HELP_ID, NULL
  },
  {
    "Separator", &xmSeparatorGadgetClass,  ' ', NULL, NULL, NULL,
    NULL,        NULL,                     NULL
  },
  {
    "Quit DM2K",     &xmPushButtonGadgetClass, 'Q', NULL, NULL, NULL,
    executePopupMenuCallback, (XtPointer) EXECUTE_POPUP_MENU_QUIT_ID, NULL
  },
  { NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL }
};
#define PROPERTY_IDX 4 /* the number of Property entry in menu */


static menuEntry_t displayMenu[] = {
  { 
    "Object",    &xmCascadeButtonGadgetClass, 'O', NULL, NULL, NULL,
    NULL,        NULL,                     editObjectMenu
  },
  { 
    "Cut",       &xmPushButtonGadgetClass, 't', "Shift<Key>DeleteChar",
    "Shift+Del", NULL,
    editMenuSimpleCallback, (XtPointer) EDIT_CUT_BTN,  NULL
  },
  { 
    "Copy",      &xmPushButtonGadgetClass, 'C', "Ctrl<Key>InsertChar", 
    "Ctrl+Ins",   NULL,
    editMenuSimpleCallback, (XtPointer) EDIT_COPY_BTN,  NULL
  },
  {
    "Paste" ,    &xmPushButtonGadgetClass, 'P', "Shift<Key>InsertChar",
    "Shift+Ins",  NULL,
    editMenuSimpleCallback, (XtPointer) EDIT_PASTE_BTN,  NULL
  },
  { 
    "Separator", &xmSeparatorGadgetClass,  '\0', NULL, NULL,         NULL,
    NULL,        NULL,                     NULL
  },
  {
    "Raise",     &xmPushButtonGadgetClass, 'R', NULL, NULL,         NULL,
    editMenuSimpleCallback, (XtPointer) EDIT_RAISE_BTN,  NULL
  },
  {
    "Lower",     &xmPushButtonGadgetClass, 'L', NULL, NULL,         NULL,
    editMenuSimpleCallback, (XtPointer) EDIT_LOWER_BTN,  NULL
  },
  {
    "Separator", &xmSeparatorGadgetClass,  '\0', NULL, NULL,         NULL,
    NULL,        NULL,                     NULL
  },
  {
    "Group",     &xmPushButtonGadgetClass, 'G', NULL, NULL,         NULL,
    editMenuSimpleCallback, (XtPointer) EDIT_GROUP_BTN,  NULL
  },
  {
    "Ungroup",   &xmPushButtonGadgetClass, 'n', NULL, NULL,         NULL,
    editMenuSimpleCallback, (XtPointer) EDIT_UNGROUP_BTN,  NULL
  },
  {
    "Separator", &xmSeparatorGadgetClass,  '\0',NULL, NULL,         NULL,
    NULL,        NULL,                     NULL
  },
  { 
    "Align",     &xmCascadeButtonGadgetClass, 'A', NULL, NULL,         NULL,
    NULL,        NULL,                     editAlignEntry
  },
  { 
    "Separator", &xmSeparatorGadgetClass,  '\0', NULL, NULL,         NULL,
    NULL,        NULL,                     NULL
  },
  {
    "Unselect",  &xmPushButtonGadgetClass, 'U', NULL, NULL,         NULL,
    editMenuSimpleCallback, (XtPointer) EDIT_UNSELECT_BTN,  NULL
  },
  {
    "Select All",&xmPushButtonGadgetClass, 'S', NULL, NULL,         NULL,
    editMenuSimpleCallback, (XtPointer) EDIT_SELECT_ALL_BTN,  NULL
  },
  { NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL }
};

static menuEntry_t fileMenu[] = {
  { "New",       &xmPushButtonGadgetClass, 'N', NULL,         NULL, NULL,
    fileMenuSimpleCallback, (XtPointer) FILE_NEW_BTN, NULL},
  { "Open...",   &xmPushButtonGadgetClass, 'O', "Ctrl<Key>O", "Ctrl+O", NULL,
    fileMenuSimpleCallback, (XtPointer) FILE_OPEN_BTN, NULL},
  { "Save",      &xmPushButtonGadgetClass, 'S', "Ctrl<Key>S", "Ctrl+S", NULL,
    fileMenuSimpleCallback, (XtPointer) FILE_SAVE_BTN, NULL},
  { "Save As...",&xmPushButtonGadgetClass, 'A', NULL,         NULL, NULL,
    fileMenuSimpleCallback, (XtPointer) FILE_SAVE_AS_BTN, NULL},
  { "Close",     &xmPushButtonGadgetClass, 'C', NULL,         NULL, NULL,
    fileMenuSimpleCallback, (XtPointer) FILE_CLOSE_BTN, NULL},
  { "Separator", &xmSeparatorGadgetClass,  '\0', NULL,         NULL, NULL,
    NULL,        NULL,                     NULL},
  { "Load faceplate...", &xmPushButtonGadgetClass, 'L', "Ctrl<Key>L", "Ctrl+L", NULL,
    fileMenuSimpleCallback, (XtPointer) FILE_FACEPLATELOAD_BTN, NULL},
  { "Separator", &xmSeparatorGadgetClass,  '\0', NULL,         NULL, NULL,
    NULL,        NULL,                     NULL},
  { "Printer Setup",  &xmPushButtonGadgetClass, 'u', NULL,      NULL, NULL,
    fileMenuSimpleCallback, (XtPointer) FILE_PRINT_SETUP_BTN, NULL},
  { "Print...",  &xmPushButtonGadgetClass, 'P', NULL,         NULL, NULL,
    fileMenuSimpleCallback, (XtPointer) FILE_PRINT_BTN, NULL},
  { "Separator", &xmSeparatorGadgetClass,  '\0', NULL,         NULL, NULL,
    NULL,        NULL,                     NULL},
  { "Exit",      &xmPushButtonGadgetClass, 'x', "Ctrl<Key>x", "Ctrl+x", NULL,
    fileMenuSimpleCallback, (XtPointer) FILE_EXIT_BTN, NULL},
  { NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL }
};

static menuEntry_t viewMenu[] = {
  { "Message Window", &xmPushButtonGadgetClass, 'M', NULL, NULL, NULL,
    viewMenuSimpleCallback, (XtPointer) VIEW_MESSAGE_WINDOW_BTN, NULL},
  { "Status Window", &xmPushButtonGadgetClass, 'S', NULL, NULL, NULL,
    viewMenuSimpleCallback, (XtPointer) VIEW_STATUS_WINDOW_BTN, NULL},
  { NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL }
};
/*
  { "Grid",           &xmPushButtonGadgetClass, 'G', NULL, NULL, NULL, NULL, NULL, NULL},
  { "Separator",      &xmSeparatorGadgetClass,  NULL, NULL, NULL, NULL, NULL, NULL, NULL},
  { "Refresh Screen", &xmPushButtonGadgetClass, 'R', NULL, NULL, NULL, NULL, NULL, NULL},
*/

static menuEntry_t palettesMenu[] = {
  { "Object",   &xmPushButtonGadgetClass, 'O', NULL, NULL, NULL,
    palettesMenuSimpleCallback, (XtPointer) PALETTES_OBJECT_BTN, NULL},
  { "Resource", &xmPushButtonGadgetClass, 'R', NULL, NULL, NULL,
    palettesMenuSimpleCallback, (XtPointer) PALETTES_RESOURCE_BTN, NULL},
  { "Color",    &xmPushButtonGadgetClass, 'C', NULL, NULL, NULL,
    palettesMenuSimpleCallback, (XtPointer) PALETTES_COLOR_BTN, NULL},
  { NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL }
};

static menuEntry_t toolsMenu[] = {
  { "Faceplate...", &xmPushButtonGadgetClass, 'F', "Ctrl<Key>F","Ctrl+F" , NULL,
    toolsMenuSimpleCallback, (XtPointer) TOOLS_FACEPLATE_BTN, NULL},
  { "Dump DisplayInfo...", &xmPushButtonGadgetClass, 'D', "Ctrl<Key>D","Ctrl+D" , NULL,
    toolsMenuSimpleCallback, (XtPointer) TOOLS_DUMPDISPLAYINFO_BTN, NULL},
  { NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL }
};

static menuEntry_t helpMenu[] =
{
  {"On Version", &xmPushButtonGadgetClass, 'V', NULL, NULL, NULL,
   helpMenuSimpleCallback, (XtPointer) HELP_ON_VERSION_BTN, NULL},
  { NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL }
};

/* last mouse position of the display before popup the menu */
XButtonPressedEvent lastEvent;

static void createMain(void);
static void RegisterCvtStringToPixmap(void);

/*
 * globals for use by all routines in this file
 */
static Widget openFSD = NULL;
static Widget faceplateSelDlg = NULL;
static Widget mainEditPDM = NULL;
static Widget mainViewPDM = NULL;
static Widget mainPalettesPDM = NULL;
static Widget mainToolsPDM = NULL;
static Widget productDescriptionShell = NULL;

static String fallbackResources[] = {
"%s.version: %s",
"",
"! ======================================================================== ",
"!  General resources ",
"",
"!Dm2k.enableEventHandlerAutoRaise: False",
"",
"*initialResourcesPersistent: 			False",
"*foreground: 					black",
"*background: 					#b0c3ca",
"*highlightThickness: 				1",
"*shadowThickness: 					2",
"*XmForm.shadowThickness:				0",
"*XmBulletinBoard.marginWidth: 			2",
"*XmBulletinBoard.marginHeight: 			2",
"*XmMainWindow.showSeparator: 			False",
"*XmTextField.verifyBell: 				False",
"*XmLabel.fontList:					\
				-*-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*",
"!*XmTextField.translations: 				#override \
!		None<Key>osfDelete:     delete-previous-character() \
!                None<Key>osfBackSpace:  delete-next-character()",
"",
"*XmCascadeButton.fontList:				\
				-*-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*",
"*XmPushButton.fontList:					\
				-*-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*",
"*XmRowColumn.XmPushButton*fontList:			\
				-*-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*",
"",
"! ======================================================================== ",
"!  Main window  ",
"",
"*.mainMW*modeRB*XmToggleButton.fontList:		\
				-*-clean-bold-r-*-*-14-*-*-*-*-*-*-*",
"*.mainMW*modeRB*XmToggleButton.indicatorOn: 		False",
"*.mainMW*modeRB*XmToggleButton.shadowThickness: 	2",
"*.mainMW*modeRB*XmToggleButton.highlightThickness: 	1",
"*.mainMW*openFSD.dialogTitle: 			Open",
"*.mainMW*helpMessageBox.dialogTitle: 		Help",
"*.mainMW*saveAsPD.dialogTitle: 			Save As...",
"*.mainMW*saveAsPD.selectionLabelString: Name of file to save display in:",
"*.mainMW*saveAsPD.okLabelString: 			Save",
"*.mainMW*saveAsPD.cancelLabelString: 		Cancel",
"*.mainMW*exitQD*XmPushButton.fontList:		\
				-*-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*",
"*.mainMW*exitQD*XmPushButtonGadget.fontList:		\
				-*-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*",
"*.mainMW*exitQD*XmLabel.fontList:		\
				-*-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*",
"*.mainMW*exitQD*XmLabelGadget.fontList:		\
				-*-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*",
"*.mainMW*exitQD.dialogTitle: 			Exit",
"*.mainMW*exitQD.messageString: 		Do you really want to Exit?",
"*.mainMW*exitQD.okLabelString: 			Yes",
"*.mainMW*exitQD.cancelLabelString: 			No",
"*.mainMW*XmRowColumn.tearOffModel: 			XmTEAR_OFF_ENABLED",
"*.mainMW*mainMB*XmPushButton.fontList:		\
				-*-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*",
"*.mainMW*mainMB*XmPushButtonGadget.fontList:		\
				-*-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*",
"*.mainMW*mainMB*XmCascadeButtonGadget.fontList:		\
				-*-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*",
"",
"! ======================================================================== ",
"!  Object palette  ",
"",
"*objectMW*XmLabel.marginWidth: 			0",
"*objectMW.objectMB*fontList: 			\
				-*-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*",
"",
"! ======================================================================== ",
"!  Display drawing area  ",
"",
"*displayDA*XmRowColumn.tearOffModel: 		XmTEAR_OFF_ENABLED",
"*displayDA*amDialog*XmRowColumn*fontList: 		\
				-*-menu-*-*-*-*-10-*-*-*-*-*-*-*",
"",
"! ---disable default translations of child widgets  ",
"",
"*displayDA*XmPushButton.translations: 		#override <Key>space: ",
"*displayDA*XmPushButtonGadget.translations: 	#override <Key>space: ",
"*displayDA*XmToggleButton.translations: 		#override <Key>space: ",
"*displayDA*XmToggleButtonGadget.translations: 	#override <Key>space: ",
"*displayDA*radioBox*translations: 			#override <Key>space: ",
"",
"! T. Straumann: work around a bug in MetroLink Motif 2.1 (for Linux)",
"!				 setting \"indicatorOn\" after creation crashes program",
"",
"*displayDA*radioBox.XmToggleButtonGadget.indicatorOn:False",
"",
"! ======================================================================== ",
"!  Color palette  ",
"",
"*colorMW*XmLabel.marginWidth: 			0",
"*colorMW*colorPB.width: 				20",
"*colorMW*colorPB.height: 				20",
"*colorMW.colorMB*fontList: 				\
				-*-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*",
"",
"! ======================================================================== ",
"!  Resource palette  ",
"",
"*resourceMW*elementListTreeSW.width:		140",
"*resourceMW*localLabel.marginRight: 		8",
"*resourceMW*bundlesSW.visualPolicy: 		XmCONSTANT",
"*resourceMW*bundlesSW.scrollBarDisplayPolicy: 	XmSTATIC",
"*resourceMW*bundlesSW.scrollingPolicy: 		XmAUTOMATIC",
"*resourceMW*bundlesRB.marginWidth: 			10",
"*resourceMW*bundlesRB.marginHeight: 		10",
"*resourceMW*bundlesRB.spacing: 			10",
"*resourceMW*bundlesRB.packing: 			XmPACK_COLUMN",
"*resourceMW*bundlesRB.orientation: 			XmVERTICAL",
"*resourceMW*bundlesRB.numColumns: 			3",
"*resourceMW*bundlesTB.indicatorOn: 			False",
"*resourceMW*bundlesTB.visibleWhenOff: 		False",
"*resourceMW*bundlesTB.borderWidth: 			0",
"*resourceMW*bundlesTB.shadowThickness: 		2",
"*resourceMW*messageF.rowColumn.spacing: 		10",
"*resourceMW*messageF.resourceElementTypeLabel.fontList: 8x13",
"*resourceMW*messageF.verticalSpacing:		6",
"*resourceMW*messageF.horizontalSpacing:		3",
"*resourceMW*messageF.shadowType:			XmSHADOW_IN",
"*resourceMW*entriesSW.width: 			330",
"*resourceMW*entriesSW.height: 			430",
"*resourceMW*importFSD.dialogTitle:			Import...",
"*resourceMW*importFSD.form.shadowThickness:		0",
"*resourceMW*importFSD.form.typeLabel.labelString:	Image Type:",
"*resourceMW*importFSD.form.typeLabel.marginTop:	4",
"*resourceMW*importFSD.form.frame.radioBox.orientation:XmHORIZONTAL",
"*resourceMW*importFSD.form.frame.radioBox.numColumns:1",
"*resourceMW*importFSD.form.frame.radioBox*shadowThickness: 0",
"*resourceMW*importFSD*XmToggleButton.indicatorOn: 	True",
"*resourceMW*importFSD*XmToggleButton.labelType:	XmSTRING",
"*resourceMW*resourceFrame.label.labelString: 	Element's Resources",
"*resourceMW*listTreeFrame.label.labelString: 	Element Tree",
"*resourceMW*fontList:				\
				-*-times-medium-r-normal-*-14-*-*-*-*-*-*-*",
"*resourceMW.resourceMB*fontList:			\
				-*-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*",
"*resourceMW*entriesSW*XmLabel.fontList:			\
				-*-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*",
"*resourceMW*relatedDisplayDataF*XmTextField.fontList:\
				-*-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*",
"*resourceMW*localElement*fontList:			\
				-*-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*",
"*resourceMW*localLabel*fontList:			\
				-*-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*",
"*resourceMW*resourceElementTypeLabel.fontList:	\
				-*-charter-bold-r-normal-*-14-*-*-*-*-*-*-*",
"*resourceMW*resourceFrame.resourceFrameLabel.fontList:\
				-*-courier-medium-r-normal-*-12-*-*-*-*-*-*-*",
"",
"! ======================================================================== ",
"!  Warning dialog ",
"",
"*warningDialog.background: 				salmon",
"*warningDialog.dialogTitle: 			Warning",
"*warningDialog*fontList:				\
			-*-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*=MESSAGE_TAG,\
			-*-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*=BUTTON_TAG",
"",
"! ======================================================================== ",
"!  Choice dialog ",
"",
"*questionDialog*fontList:				\
			-*-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*=MESSAGE_TAG,\
			-*-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*=BUTTON_TAG",
"",
"! ======================================================================== ",
"!  Dm2k widgets used to present some display elements ",
"",
"*Indicator.AxisWidth:				3",
"*Bar.AxisWidth:					3",
"*Indicator.ShadowThickness:				2",
"*Bar.ShadowThickness:				2",
"*Meter.ShadowThickness:				2",
"",
"! ======================================================================== ",
"!  Associated menu constraction dialog ",
"",
"*resourceMW*localElement*amItemForm*XmTextField.fontList:\
				-*-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*",
"*resourceMW*localElement*amItemForm*XmLabel.fontList:\
				-*-helvetica-medium-r-*-*-14-*-*-*-*-*-*-*",
"*resourceMW*localElement*amScrList*fontList:	\
				-*-courier-*-r-*-*-14-*-*-*-*-*-*-*=TAG1,\
				-*-courier-*-o-*-*-14-*-*-*-*-*-*-*=TAG2",
"",
"! ======================================================================== ",
"!  RelatedDisplay dialog ",
"",
"*relatedDisplayDataF.XmLabel*fontList:		\
				-*-helvetica-bold-r-*-*-10-*-*-*-*-*-*-*",
"*relatedDisplayDataF*rdMatrix*fontList:			\
				-*-helvetica-medium-r-*-*-10-*-*-*-*-*-*-*",
"",
"! ======================================================================== ",
"!  Cartesian Plot Axis Data dialog ",
"",
"*cartesianPlotAxisS*entryRC*localElement*fontList:	\
				-*-courier-bold-r-*-*-12-*-*-*-*-*-*-*",
"*cartesianPlotAxisS*entryRC.localLabel.fontList:	\
				-*-charter-medium-r-*-*-12-*-*-*-*-*-*-*",
"*cartesianPlotAxisS*frame.label.fontList:		\
				-*-times-medium-r-*-*-14-*-*-*-*-*-*-*",
"",
"! ======================================================================== ",
"!  Version notify dialog ",
"",
"*versionNoticeDialog.labelFontList:			\
				-*-times-bold-r-normal-*-14-*-*-*-*-*-*-*",
"*versionNoticeDialog.textFontList:			\
				-*-times-bold-r-normal-*-14-*-*-*-*-*-*-*",
"*versionNoticeDialog.buttonFontList:		\
				-*-helvetica-bold-r-normal-*-12-*-*-*-*-*-*-*",
"*versionNoticeDialog.XmPushButton.fontList:		\
				-*-helvetica-bold-r-normal-*-12-*-*-*-*-*-*-*",
"",
"! ======================================================================== ",
"!  Product decsription shell pops up in the very begining of Dm2k's session ",
"",
"*productDescriptionShell*XmLabel.fontList:		\
		-*-helvetica-bold-r-*-*-34-*-*-*-*-*-*-*=PRODUCT_NAME_TAG,\
		-*-courier-bold-o-*-*-14-*-*-*-*-*-*-*=DESCRIPTION_TAG,\
		-*-times-bold-r-*-*-14-*-*-*-*-*-*-*=VERSION_INFO_TAG,\
	        -*-courier-medium-r-*-*-12-*-*-*-*-*-*-*=DEVELOPED_AT_TAG",
"",
"! ======================================================================== ",
"!  Dm2k's general Help dialog  ",
"",
"*helpMBDialog*XmLabel.fontList:			\
		-*-helvetica-medium-r-normal-*-12-*-*-*-*-*-*-*",
"*helpMBDialog*XmLabelGadget.fontList:			\
		-*-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*",
"*helpMBDialog*XmPushButton.fontList:			\
		-*-helvetica-bold-r-normal-*-12-*-*-*-*-*-*-*",
"",
"! ======================================================================== ",
"!  Dm2k's Faceplate dialog  ",
"",
"*wFaceplateDlg*XmToggleButton.shadowThickness:	0",
"*wFaceplateDlg.fontList:				\
		-*-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*",
"*wFaceplateDlg*XmLabel.fontList:			\
		-*-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*",
  NULL,
};

typedef enum {EDIT,EXECUTE,ERROR,HELP,VERSION} opMode_t;
typedef enum {NORMAL,CLEANUP,LOCAL} dm2kMode_t;
typedef enum {FIXED,SCALABLE} fontStyle_t;

typedef struct {
  opMode_t opMode;
  dm2kMode_t dm2kMode;
  fontStyle_t fontStyle;
  Boolean privateDataCmap;
  char *macroString;
  char displayFont[256];          /* !!!! warning : fix array size */
  char *displayName;
  char *displayGeometry;
  int  fileCnt;
  char **fileList;
  int    productPresentationRun;
} request_t;

void requestDestroy(request_t *request) {
  if (request) {
    if (request->macroString) free(request->macroString);
/*    if (request->displayFont) free(request->displayFont);     */
    if (request->displayName) free(request->displayName);
    if (request->displayGeometry) free(request->displayGeometry);
    if (request->fileList) {
      int i;
      for (i=0; i < request->fileCnt; i++) {
        if (request->fileList[i]) free(request->fileList[i]);
      }
      free((char *)request->fileList);
      request->fileList = NULL;
    }
    free((char *)request);
    request = NULL;
  }
}

request_t * requestCreate(int argc, char *argv[]) {
  int i;
  int argsUsed = 1;                /* because argv[0] = "dm2k" */
  int fileEntryTableSize = 0;
  request_t *request = NULL;
  char currentDirectoryName[FULLPATHNAME_SIZE+1];
  char fullPathName[FULLPATHNAME_SIZE+1];
  char *envstrg;

  request = (request_t *) malloc(sizeof(request_t));
  if (request == NULL) return request;
  request->opMode = EDIT;
  request->dm2kMode = NORMAL;
  request->fontStyle = FIXED;
  request->privateDataCmap = False;
  request->macroString = NULL;
  strcpy(request->displayFont,FONT_ALIASES_STRING);
  request->displayName = NULL;
  request->displayGeometry = NULL;
  request->fileCnt = 0;
  request->fileList = NULL;

  if ( getenv(READ_ONLY_ENV) ) {
    globalDm2kReadOnly = True;
    request->opMode = EXECUTE;
  }

  if (( envstrg = getenv (DM2K_WM_POSITION_POLICY_ENV) )) {
    if ( !strcasecmp (envstrg, "FRAME") ) {
      positionPolicyFlag = True;     /* The policy is defined by environment variable */
      positionPolicyFrame = True;    /* WM position policy is FRAME */
    }
    else if ( !strcasecmp (envstrg, "USER") ) {
      positionPolicyFlag = True;     /* The policy is defined by environment variable */
      positionPolicyFrame = False;   /* WM position policy is USER */
    }
  }

  /* parse the switches */
  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i],"-readonly")) {
      globalDm2kReadOnly = True;
      request->opMode = EXECUTE;
      argsUsed = i;
    } else
    if (!strcmp(argv[i],"-synchro")) {
      synchroFlag = True;
      argsUsed = i;
    } else
    if (!strcmp(argv[i],"-silent")) {
      silentFlag = True;
      argsUsed = i;
    } else
    if (!strcmp(argv[i],"-verbose")) {
      verboseFlag = True;
      argsUsed = i;
    } else
    if (!strcmp(argv[i],"-debug")) {
      debugFlag = True;
      argsUsed = i;
    } else
    if (!strcmp(argv[i],"-x")) {
      request->opMode = EXECUTE;
      argsUsed = i;
    } else
    if (!strcmp(argv[i],"-help") || !strcmp(argv[i],"-?") || !strcmp(argv[i],"-h")) {
      request->opMode = HELP;
      argsUsed = i;
    } else
    if (!strcmp(argv[i],"-version") || !strcmp(argv[i],"-v")) {
      request->opMode = VERSION;
      argsUsed = i;
    } else
    if (!strcmp(argv[i],"-local")) {
      request->dm2kMode = LOCAL;
      argsUsed = i;
    } else
    if (!strcmp(argv[i],"-cleanup")) {
      request->dm2kMode = CLEANUP;
      argsUsed = i;
    } else
    if (!strcmp(argv[i],"-cmap")) {
      request->privateDataCmap = True;
      argsUsed = i;
    } else
    if ( !strcmp(argv[i],"-wmPositionPolicy") ) {
      char *tmp;
      argsUsed = i;
      tmp = (((i+1) < argc) ? argv[i+1] : NULL);
      if (tmp) {
        argsUsed = i + 1;
	if ( !strcasecmp (tmp, "FRAME") ) {
	  positionPolicyFlag = True;     /* The policy is defined by option */
	  positionPolicyFrame = True;    /* WM position policy is FRAME */
	}
	else if ( !strcasecmp (tmp, "USER") ) {
	  positionPolicyFlag = True;     /* The policy is defined by option */
	  positionPolicyFrame = False;   /* WM position policy is USER */
	}
      }
    } else
    if (!strcmp(argv[i],"-macro")) {
      char *tmp;
      argsUsed = i;
      tmp = (((i+1) < argc) ? argv[i+1] : NULL);
      if (tmp) {
        argsUsed = i + 1;
        request->macroString = STRDUP(tmp);
        /* since parameter of form   -macro "a=b,c=d,..."  replace '"' with ' ' */
        if (request->macroString != NULL) {
          int len;
          if (request->macroString[0] == '"') request->macroString[0] = ' ';
          len = STRLEN(request->macroString) - 1;
          if (request->macroString[len] == '"') request->macroString[len] = ' ';
        }
      }
    } else
    if (!strcmp(argv[i],"-displayFont")) {
      char *tmp;
      argsUsed = i;
      tmp = (((i+1) < argc) ? argv[i+1] : NULL);
      if (tmp) {
        argsUsed = i + 1;
        strcpy(request->displayFont,tmp);
        if (request->displayFont[0] == '\0') {
          if (!strcmp(request->displayFont,FONT_ALIASES_STRING))
            request->fontStyle = FIXED;
          else if (!strcmp(request->displayFont,DEFAULT_SCALABLE_STRING))
            request->fontStyle = SCALABLE;
        }
      }
    } else
    if (!strcmp(argv[i],"-display")) {
      char *tmp;
      argsUsed = i;
      tmp = (((i+1) < argc) ? argv[i+1] : NULL);
      if (tmp) {
        argsUsed = i + 1;
        request->displayName = STRDUP(tmp);
      }
    } else 
    if ((!strcmp(argv[i],"-displayGeometry")) || (!strcmp(argv[i],"-dg"))) {
      char *tmp;
      argsUsed = i;
      tmp = (((i+1) < argc) ? argv[i+1] : NULL);
      if (tmp) {
        argsUsed = i + 1;
        request->displayGeometry = STRDUP(tmp);
      }
    } else
    if (!strcmp(argv[i],"-bigMousePointer")) {
      dm2kUseBigCursor = 1;
      argsUsed = i;
    }
    if (!strcmp(argv[i],"-dontPromptExit")) {
      dm2kPromptToExit = 0;
      argsUsed = i;
    }
    if (!strcmp(argv[i],"-promptExit")) {
      dm2kPromptToExit = 1;
      argsUsed = i;
    }
  }

  if ( verboseFlag ) silentFlag = False;

  /* get the current directory */
  currentDirectoryName[0] = '\0';
  getcwd(currentDirectoryName,FULLPATHNAME_SIZE);
  /* make fullPathName is a terminated with '\0' string */
  fullPathName[FULLPATHNAME_SIZE] = '\0';

  /* parse the display name */
  for (i = argsUsed; i < argc; i++) {
    Boolean canAccess;
    char    *fileStr;

    canAccess = False;

    /* check the next argument, if doesn't match the suffix, continue */
    fileStr = argv[i];

/*
    if ( (strstr(fileStr,DISPLAY_FILE_ASCII_SUFFIX) == NULL ) &&
         (strstr(fileStr,DISPLAY_FILE_FACEPLATE_SUFFIX) == NULL) )
	 continue;
*/
    if (STRLEN(fileStr) > (size_t) FULLPATHNAME_SIZE) continue;

    /* mark the fullPathName as an empty string */
    fullPathName[0] = '\0';

    strncpy(fullPathName,fileStr,FULLPATHNAME_SIZE);
    /* found string with right suffix - presume it's a valid display name */
    if ((canAccess = !access(fullPathName,R_OK|F_OK))) { /* found the file */
       if (fileStr[0] == '/') {
	  strcpy(fullPathName,fileStr);
       } else {
        /* insert the path before the file name */
        if (STRLEN(currentDirectoryName)+STRLEN(fullPathName)+1 < (size_t) FULLPATHNAME_SIZE) {
	   
          strcpy(fullPathName,currentDirectoryName);
          strcat(fullPathName,"/");
          strcat(fullPathName,fileStr);
        } else {
          canAccess = False;
        }
      }
    } else { /* try with directory specified in the environment */
      char *dir = NULL;
      char name[FULLPATHNAME_SIZE];
      int startPos;
      dir = getenv(DISPLAY_LIST_ENV);
      if (dir != NULL) {
        startPos = 0;
        while (extractStringBetweenColons(dir,name,startPos,&startPos)) {
          if (STRLEN(name)+STRLEN(fileStr)+1 < (size_t) FULLPATHNAME_SIZE) {
            strcpy(fullPathName,name);
            strcat(fullPathName,"/");
            strcat(fullPathName,fileStr);
            if ((canAccess = !access(fullPathName,R_OK|F_OK))) break;
            strcat(fullPathName,DISPLAY_FILE_ASCII_SUFFIX);
            if ((canAccess = !access(fullPathName,R_OK|F_OK))) break;
          }
        }
      }
    }

    if (canAccess) {
      /* build the request */
      if (fileEntryTableSize == 0) {
        fileEntryTableSize =  10;
        request->fileList = (char **) calloc(fileEntryTableSize,sizeof(char *));
      }
      if (fileEntryTableSize > request->fileCnt) {
        fileEntryTableSize *= 2;
#if defined(__cplusplus) && defined(SUNOS4) && !defined(__GNUG__)
        request->fileList = (char **) realloc((malloc_t)request->fileList,fileEntryTableSize);
#else
        request->fileList = (char **) realloc(request->fileList,fileEntryTableSize);
#endif
      }
      if (request->fileList) {
        request->fileList[request->fileCnt] = STRDUP(fullPathName);
        request->fileCnt++;
      }
    }
  }
  return request;
}

/********************************************
 **************** Callbacks *****************
 ********************************************/
#ifdef __cplusplus
static void printerSetupDlgCb(Widget w, XtPointer cd, XtPointer)
#else
static void printerSetupDlgCb(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  Position X, Y;
  XmString xmString;
  char *printerName;
  char *variable;
  char *prefix;

  if (getenv("EPICS_PS_PRINT_CMD"))
     prefix = "EPICS_PS_PRINT_CMD=";
  else
     prefix = "PSPRINTER=";

  switch ((int)cd) {
    case PRINTER_SETUP_OK :
      XtVaGetValues(w,XmNtextString,&xmString,NULL);
      XmStringGetLtoR(xmString,XmFONTLIST_DEFAULT_TAG,&printerName);
      variable = (char*) malloc(
                 sizeof(char)*(STRLEN(printerName) + STRLEN(prefix) + 1));
      if (variable) {
        strcpy(variable,prefix);
        strcat(variable,printerName);
        putenv(variable);
        /* Warning!!!! : Do not free the variable */
      }
      free(printerName);
      XmStringFree(xmString);
      XtUnmanageChild(w);
      break;
    case PRINTER_SETUP_CANCEL :
      XtUnmanageChild(w);
      break;
    case PRINTER_SETUP_MAP :
      if (getenv("EPICS_PS_PRINT_CMD")) {
        xmString = XmStringCreateLocalized(getenv("EPICS_PS_PRINT_CMD"));
      } else if (getenv("PSPRINTER")) {
	 xmString = XmStringCreateLocalized(getenv("PSPRINTER"));
      } else {
	 xmString = XmStringCreateLocalized("");
      }
      XtVaSetValues(w,XmNtextString,xmString,NULL);
      XmStringFree(xmString);
      XtTranslateCoords(mainShell,0,0,&X,&Y);
      /* try to force correct popup the first time */
      XtMoveWidget(XtParent(w),X,Y);

      break;
  }
}

#ifdef __cplusplus
static void viewMenuSimpleCallback(Widget, XtPointer cd, XtPointer)
#else
static void viewMenuSimpleCallback(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  int buttonNumber = (int) cd;

  switch(buttonNumber) {
    case VIEW_MESSAGE_WINDOW_BTN:
      errMsgDlgCreateDlg(1);
      break;
    case VIEW_STATUS_WINDOW_BTN:
      dm2kCreateCAStudyDlg();
      break;
    default :
      break;
  }
}

#ifdef __cplusplus
static void editMenuSimpleCallback(Widget, XtPointer cd, XtPointer)
#else
static void editMenuSimpleCallback(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
 int buttonNumber = (int) cd;

/* simply return if no current display */
  if (currentDisplayInfo == NULL) return;


/* (MDA) could be smarter about this too, and not do whole traversals...*/

    switch(buttonNumber) {
        case EDIT_OBJECT_BTN:
	   break;

	case EDIT_CUT_BTN:
	   copySelectedElementsIntoClipboard();
	   deleteElementsInDisplay();
	   if (currentDisplayInfo->hasBeenEditedButNotSaved == False)
	     dm2kMarkDisplayBeingEdited(currentDisplayInfo);
	   break;

	case EDIT_COPY_BTN:
	   copySelectedElementsIntoClipboard();
	   break;

  case EDIT_PASTE_BTN:
    copyElementsIntoDisplay();
    if (currentDisplayInfo->hasBeenEditedButNotSaved == False) 
      dm2kMarkDisplayBeingEdited(currentDisplayInfo);
    break;

	case EDIT_RAISE_BTN:
	   raiseSelectedElements(currentDisplayInfo);
           if (currentDisplayInfo->hasBeenEditedButNotSaved == False) 
             dm2kMarkDisplayBeingEdited(currentDisplayInfo);
	   break;

	case EDIT_LOWER_BTN:
	   lowerSelectedElements(currentDisplayInfo);
           if (currentDisplayInfo->hasBeenEditedButNotSaved == False) 
             dm2kMarkDisplayBeingEdited(currentDisplayInfo);
	   break;

	case EDIT_GROUP_BTN:
	   groupObjects(currentDisplayInfo);
           if (currentDisplayInfo->hasBeenEditedButNotSaved == False) 
             dm2kMarkDisplayBeingEdited(currentDisplayInfo);
	   break;

	case EDIT_UNGROUP_BTN:
	   ungroupSelectedElements();
           if (currentDisplayInfo->hasBeenEditedButNotSaved == False) 
             dm2kMarkDisplayBeingEdited(currentDisplayInfo);
	   break;

	case EDIT_UNSELECT_BTN:
	   unselectElementsInDisplay();
	   break;

	case EDIT_SELECT_ALL_BTN:
	   selectAllElementsInDisplay();
	   break;
    }
}


#ifdef __cplusplus
static void alignMenuSimpleCallback(Widget, XtPointer cd, XtPointer)
#else
static void alignMenuSimpleCallback(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
   alignSelectedElements((int)cd);
   if (currentDisplayInfo->hasBeenEditedButNotSaved == False) 
      dm2kMarkDisplayBeingEdited(currentDisplayInfo);
}


#ifdef __cplusplus
static void mapCallback(Widget w, XtPointer , XtPointer)
#else
static void mapCallback(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  Position X, Y;
  XmString xmString;

  XtTranslateCoords(currentDisplayInfo->shell,0,0,&X,&Y);
  /* try to force correct popup the first time */
  XtMoveWidget(XtParent(w),X,Y);

  /* be nice to the users - supply default text field as display name */
  xmString = XmStringCreateSimple(currentDisplayInfo->dlFile->name);
  XtVaSetValues(w,XmNtextString,xmString,NULL);
  XmStringFree(xmString);
}


static void fileTypeCallback(
  Widget w,
  int buttonNumber,
  XmToggleButtonCallbackStruct *call_data)
{
  if (call_data->set == False) return;
  switch(buttonNumber) {
  case 0:
    Dm2kUseNewFileFormat = True;
    break;
  case 1:
    Dm2kUseNewFileFormat = False;
    break;
  }
}

void printerSetup(Widget parent)
{
  int n = 0;
  Arg args[4];
  XmString xmString;
  if (getenv("EPICS_PS_PRINT_CMD"))
     xmString = XmStringCreateLocalized("Enter new PS print command:");
  else
     xmString = XmStringCreateLocalized("Enter new printer name:");

  if (printerSetupDlg)
    XtDestroyWidget(printerSetupDlg);

  XtSetArg(args[n],XmNdefaultPosition,     False); n++;
  XtSetArg(args[n],XmNselectionLabelString, xmString); n++;

  printerSetupDlg = XmCreatePromptDialog(parent,
					 "printerSetupPD",args,n);
  XmStringFree(xmString);

  XtUnmanageChild
    ( XmSelectionBoxGetChild(printerSetupDlg,XmDIALOG_HELP_BUTTON));
  
  XtAddCallback(printerSetupDlg,XmNokCallback,printerSetupDlgCb,
		PRINTER_SETUP_OK);
  XtAddCallback(printerSetupDlg,XmNcancelCallback,
		printerSetupDlgCb,(XtPointer)PRINTER_SETUP_CANCEL);
  XtAddCallback(printerSetupDlg,XmNmapCallback,
		printerSetupDlgCb,(XtPointer)PRINTER_SETUP_MAP);
  XtManageChild(printerSetupDlg);
}  


#ifdef __cplusplus
static void fileMenuSimpleCallback(Widget, XtPointer cd, XtPointer)
#else
static void fileMenuSimpleCallback(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  int      buttonNumber = (int) cd;
  Widget   widget;
  XmString dirMask;
  XEvent   event;

  switch(buttonNumber) {
    case FILE_NEW_BTN:
      currentDisplayInfo = createDisplay ();
      enableEditFunctions();
      enableControllerRC (currentDisplayInfo);
      break;

    case FILE_OPEN_BTN:
      /*
       * note - all FILE pdm entry dialogs are sharing a single callback
       *	  fileMenuDialogCallback, with client data
       *	  of the BTN id in the simple menu...
       */


      /*
       * create the Open... file selection dialog
       */
      XDefineCursor(display,XtWindow(mainShell),watchCursor);
      if (openFSD == NULL) 
      {
	Arg args[4];
        int n = 0;
        XmString label = XmStringCreateSimple(DISPLAY_DIALOG_MASK);

        /* for some odd reason can't get PATH_MAX reliably 
	 * defined between systems 
	 */
        char *cwd = getcwd(NULL,LOCAL_PATH_MAX+1);
        XmString cwdXmString = XmStringCreateSimple(cwd);

        XtSetArg(args[n],XmNpattern,label); n++;
        XtSetArg(args[n],XmNdirectory,cwdXmString); n++;
        openFSD = XmCreateFileSelectionDialog(XtParent(mainFilePDM),
					      "openFSD",args,n);
	XtVaSetValues( openFSD, 
		       XmNuserData, (XtPointer) DISPLAY_DIALOG_MASK, 
		       NULL );

        XtDestroyWidget(XmFileSelectionBoxGetChild(openFSD,XmDIALOG_HELP_BUTTON));

/*...*/ fillFileSelBox( openFSD );
	
        XtAddCallback(openFSD,XmNokCallback,
		      fileMenuDialogCallback,(XtPointer)FILE_OPEN_BTN);
        XtAddCallback(openFSD,XmNcancelCallback,
		      fileMenuDialogCallback,(XtPointer)FILE_OPEN_BTN);
        XmStringFree(label);
        XmStringFree(cwdXmString);
        free(cwd);
      }

      XtVaGetValues(openFSD,XmNdirMask,&dirMask,NULL);
      XmListDeselectAllItems(XmFileSelectionBoxGetChild(openFSD,XmDIALOG_LIST));
      XmFileSelectionDoSearch(openFSD,dirMask);
      XtManageChild(openFSD);
      XUndefineCursor(display,XtWindow(mainShell));
      break;

    case FILE_SAVE_BTN:
      /* no display, do nothing 
       */
      currentDisplayInfo = choiseDisplayInfo (saveCursor);
      
      if (currentDisplayInfo == NULL) {
	XtWarning("No display to be save!");
	break;
      }

      /* check versions
       */
      if (DM2K_ADL_FILE_JUMP_VERSION > currentDisplayInfo->dlFile->versionNumber) 
	{
	  int button;

	  button = getUserChoiseViaPopupQuestionDialog 
	    (currentDisplayInfo->shell,
	     DM2K_DIALOG_MESSAGE_LABEL, 
	     "Format of original adl file differs from current one.",
	     DM2K_DIALOG_OK_BUTTON,     "Save in current format",
	     DM2K_DIALOG_CUSTEM_BUTTON, "Save in original format",
	     DM2K_DIALOG_CANCEL_BUTTON, "Cancel",
	     DM2K_DIALOG_TITLE,         "Format file notice",
	     NULL);

	  if (button == 0) 
	    {
	      /* Save in current format 
	       */
	      Dm2kUseNewFileFormat = True; 
	      dm2kSaveDisplay(currentDisplayInfo, 
			      currentDisplayInfo->dlFile->name,True);
	    }
	  else if (button == 1)
	    {
	      /* Save in original format 
	       */
	      Dm2kUseNewFileFormat = False;
	      dm2kSaveDisplay(currentDisplayInfo, 
			      currentDisplayInfo->dlFile->name,True);
	    }	      
	}
      else
	{
	  /* Save in current format 
	   */
	  Dm2kUseNewFileFormat = True; 
	  dm2kSaveDisplay(currentDisplayInfo, 
			  currentDisplayInfo->dlFile->name,True);
	}

      break;

    case FILE_SAVE_AS_BTN:

      /* create the Open... file selection dialog
       */
      if (!saveAsPD) 
      {
        XmString      buttons[NUM_IMAGE_TYPES-1];
        Widget        radioBox, rowColumn, typeLabel;
        Arg           args[10];
        int           i, n;
        XmString      label = XmStringCreateSimple("*.adl");

        /* for some odd reason can't get PATH_MAX 
	 * reliably defined between systems 
	 */

        char         * cwd = getcwd(NULL, LOCAL_PATH_MAX+1);
        XmString       cwdXmString = XmStringCreateSimple(cwd);

        n = 0;
        XtSetArg(args[n],XmNdefaultPosition, False); n++;
        XtSetArg(args[n],XmNpattern,         label); n++;
        XtSetArg(args[n],XmNdirectory,       cwdXmString); n++;
        saveAsPD = XmCreateFileSelectionDialog(XtParent(mainFilePDM),
					       "saveAsFSD",args,n);

        XtDestroyWidget(XmFileSelectionBoxGetChild(saveAsPD,XmDIALOG_HELP_BUTTON));
	XtVaSetValues( saveAsPD, 
		       XmNuserData, (XtPointer) DISPLAY_DIALOG_MASK, 
		       NULL );
/*	fillFileSelBox( saveAsPD );*/

        XtAddCallback(saveAsPD,XmNokCallback,
          fileMenuDialogCallback,(XtPointer)FILE_SAVE_AS_BTN);

        XtAddCallback(saveAsPD,XmNcancelCallback,
          fileMenuDialogCallback,(XtPointer)FILE_SAVE_AS_BTN);

        XtAddCallback(saveAsPD,XmNmapCallback,mapCallback,(XtPointer)NULL);

        XmStringFree(label);
        XmStringFree(cwdXmString);
        free(cwd);

        n = 0;
        XtSetArg(args[n],XmNorientation,XmHORIZONTAL); n++;
        rowColumn = XmCreateRowColumn(saveAsPD,"rowColumn",args,n);

        n = 0;
        typeLabel = XmCreateLabel(rowColumn,"File Format",args,n);
 
        buttons[0] = XmStringCreateSimple("2.2.x");
        buttons[1] = XmStringCreateSimple("2.1.x");

        n = 0;
        XtSetArg(args[n],XmNbuttonCount,    2); n++;
        XtSetArg(args[n],XmNbuttons,        buttons); n++;
        XtSetArg(args[n],XmNbuttonSet,      0); n++;
        XtSetArg(args[n],XmNorientation,    XmHORIZONTAL); n++;
        XtSetArg(args[n],XmNsimpleCallback, fileTypeCallback); n++;
        radioBox = XmCreateSimpleRadioBox(rowColumn,"radioBox",args,n);

        XtManageChild(typeLabel);
        XtManageChild(radioBox);
        XtManageChild(rowColumn);

        for (i = 0; i < 2; i++) 
	  XmStringFree(buttons[i]);
      }

      /* no display, do nothing 
       */
      currentDisplayInfo = choiseDisplayInfo (saveCursor);
      
      if (currentDisplayInfo == NULL) {
	XtWarning("No display to be save!");
	break;
      }

      /* new display or user want to save as a different name 
       */
      
#ifdef SUPPORT_0201XX_FILE_FORMAT
      Dm2kUseNewFileFormat = True;
#endif
      XtManageChild(saveAsPD);

      break;

    case FILE_CLOSE_BTN:
      if (displayInfoListHead == displayInfoListTail) {
        /* only one display; no need to query user 
	 */
	if (displayInfoListTail == NULL)
	  return;

        widget = displayInfoListTail->drawingArea;
      } 
      else if (displayInfoListHead) {
        /* more than one display; query user 
	 */
        widget = XmTrackingEvent(mainShell,closeCursor,False, &event);
        if (widget == (Widget) NULL) 
	  return;
      } 
      else {
        /* no display */
        return;
      }

      closeDisplay(widget);
      break;

    case FILE_PRINT_SETUP_BTN:
      printerSetup(XtParent(mainFilePDM));
      break;

    case FILE_PRINT_BTN:
      if (displayInfoListHead == displayInfoListTail) {
	/* only one display; no need to query user */
	currentDisplayInfo = displayInfoListHead;
	enableControllerRC (currentDisplayInfo);

	if (currentDisplayInfo != NULL) {
	  /* This make take a second... give user some indication 	 */
	  XDefineCursor(display,
			XtWindow(currentDisplayInfo->drawingArea), 
			watchCursor);
    
	  utilPrint(XtDisplay(currentDisplayInfo->drawingArea),
		    XtWindow(currentDisplayInfo->drawingArea),
		    DISPLAY_XWD_FILE);

	  /* change drawingArea's cursor back to the appropriate cursor */
	  drawingAreaDefineCursor (currentDisplayInfo);
	}

      } 
      else if (displayInfoListHead) {
	/* more than one display; query user */
	widget = XmTrackingEvent(mainShell,printCursor,False,&event);
	if (widget != (Widget)NULL) {
     	  currentDisplayInfo = dmGetDisplayInfoFromWidget(widget);
	  enableControllerRC (currentDisplayInfo);

	  if (currentDisplayInfo != NULL) {
	    /* This make take a second... give user some indication 	 */
	    XDefineCursor(display,
			  XtWindow(currentDisplayInfo->drawingArea), 
			  watchCursor);
	    
	    utilPrint(XtDisplay(currentDisplayInfo->drawingArea),
		      XtWindow(currentDisplayInfo->drawingArea),
		      DISPLAY_XWD_FILE);
	    
	    /* change drawingArea's cursor back to the appropriate cursor */
	    drawingAreaDefineCursor (currentDisplayInfo);
	  }

	}
      }
      break;

    case FILE_FACEPLATELOAD_BTN: 

      XDefineCursor(display,XtWindow(mainShell),watchCursor);

      if (faceplateSelDlg == NULL) {
	Arg    args[4];
	int    n ;
	
	XmString label = XmStringCreateSimple("*.mfp");
	/* for some odd reason can't get PATH_MAX reliably 
	 * defined between systems 
	 */
	char *cwd = getcwd(NULL,LOCAL_PATH_MAX+1);
	XmString cwdXmString = XmStringCreateSimple(cwd);
	
	n = 0;
	XtSetArg(args[n],XmNpattern,label); n++;
	XtSetArg(args[n],XmNdirectory,cwdXmString); n++;
	faceplateSelDlg = XmCreateFileSelectionDialog(XtParent(mainFilePDM),
						      "faceplateSelDlg",args,n);
	XtVaSetValues( faceplateSelDlg, XmNuserData, (XtPointer) "*.mfp", NULL );

        XtDestroyWidget(XmFileSelectionBoxGetChild(faceplateSelDlg,XmDIALOG_HELP_BUTTON));
	fillFileSelBox( faceplateSelDlg );

	XtAddCallback(faceplateSelDlg, XmNokCallback,
		      fileMenuDialogCallback, (XtPointer)FILE_FACEPLATELOAD_BTN);
	XtAddCallback(faceplateSelDlg,XmNcancelCallback,
		      fileMenuDialogCallback, (XtPointer)FILE_FACEPLATELOAD_BTN);
	XmStringFree(label);
	XmStringFree(cwdXmString);
	free(cwd);
      }
	
      XtVaGetValues(faceplateSelDlg,XmNdirMask,&dirMask,NULL);
      XmListDeselectAllItems(XmFileSelectionBoxGetChild(faceplateSelDlg,XmDIALOG_LIST));
      XmFileSelectionDoSearch(faceplateSelDlg,dirMask);
      XtManageChild(faceplateSelDlg);
      XUndefineCursor(display,XtWindow(mainShell));

    break;

    case FILE_EXIT_BTN:
      dm2kExit();
      break;
  }
}

#if 0
static void dm2kExitMapCallback(
  Widget w,
  DisplayInfo *displayInfo,
  XmAnyCallbackStruct *call_data)
{
  Position X, Y;

  XtTranslateCoords(displayInfo->shell,0,0,&X,&Y);
  /* try to force correct popup the first time */
  XtMoveWidget(XtParent(w),X,Y);

  /*
  xmString = XmStringCreateSimple(displayInfo->dlFile->name);
  XtVaSetValues(w,XmNtextString,xmString,NULL);
  XmStringFree(xmString);
  */
}
#endif

/* 
 * dm2k allowed a .template used as a suffix for compatibility. 
 * This exception is caused by a bug in the save routine 
 * at checking the ".adl" suffix.
 */

static const char *templateSuffix = ".template";

Boolean dm2kSaveDisplay(DisplayInfo * displayInfo, 
			char        * filename, 
			Boolean       overwrite) 
{
  char        * suffix;
  char          f1[MAX_FILE_CHARS], f2[MAX_FILE_CHARS+4];
  char          warningString[2*MAX_FILE_CHARS];
  int           strLen1, strLen2, strLen3, strLen4;
  int           status;
  FILE        * stream;
  Boolean       brandNewFile = False;
  Boolean       templateException = False;
  struct stat   statBuf;

  if (displayInfo == NULL || filename == NULL) 
    return False;

  strLen1 = STRLEN(filename);
  strLen2 = STRLEN(DISPLAY_FILE_BACKUP_SUFFIX);
  strLen3 = STRLEN(DISPLAY_FILE_ASCII_SUFFIX);
  strLen4 = STRLEN(templateSuffix);

  if (strLen1 >= MAX_FILE_CHARS) {
    dm2kPrintf("Path too Long %s\n:",filename);
    return False;
  }

  /* search for the position of the .adl suffix 
   */
  strcpy(f1,filename);
  suffix = strstr(f1,DISPLAY_FILE_ASCII_SUFFIX);

  if ((suffix) && (suffix == f1 + strLen1 - strLen3)) {
    /* chop off the .adl suffix */
    *suffix = '\0';
    strLen1 = strLen1 - strLen3;
  } else {
    /* search for the position of the .template suffix */
    suffix = strstr(f1,templateSuffix);
    if ((suffix) && (suffix == f1 + strLen1 - strLen4)) { 
      /* this is a .template special case */
      templateException = True;
    }
  }

  /* create the backup file name with suffux _BAK.adl
   */
  strcpy(f2,f1);
  strcat(f2,DISPLAY_FILE_BACKUP_SUFFIX);
  strcat(f2,DISPLAY_FILE_ASCII_SUFFIX);

  /* append the .adl suffix ;
   * check for the special case .template ;
   */

  if (!templateException) 
    strcat(f1,DISPLAY_FILE_ASCII_SUFFIX);

  /* See whether the file already exists. 
   */
  if (access(f1,W_OK) == -1) {
    if (errno == ENOENT) {
      brandNewFile = True;
    } else {
      sprintf(warningString,"Fail to create/write file :\n%s",filename);
      dmSetAndPopupWarningDialog(displayInfo,warningString,"Ok",NULL,NULL);
      return False;
    }
  } 
  else {
    /* file exists, see whether the user want to overwrite the file. 
     */
    if (!overwrite) {
      sprintf(warningString,"Do you want to overwrite file :\n%s",f1);
      dmSetAndPopupQuestionDialog(displayInfo,warningString,"Yes","No",NULL);
      switch (displayInfo->questionDialogAnswer) {
        case 1 :
	  /* Yes, Save the file */
	  break;
        default :
	  /* No, return */
          return False;
      }
    }
    
    /* see whether the backup file can be overwritten 
     */
    if (access(f2,W_OK) == -1) {
      if (errno != ENOENT) {
        sprintf(warningString,"Cannot write backup file :\n%s",filename);
        dmSetAndPopupWarningDialog(displayInfo,warningString,"Ok",NULL,NULL);
        return False;
      }
    }

    status = stat(f1,&statBuf);
    if (status) {
      dm2kPrintf("Failed to read status of file %s\n",filename);
      return False;
    }

    status = rename(f1,f2);
    if (status) {
      dm2kPrintf("Cannot rename file %s\n",filename);
      return False;
    }
  }

  stream = fopen(f1,"w");
  if (stream == NULL) {
    sprintf(warningString,"Fail to create/write file :\n%s",filename);
    dmSetAndPopupWarningDialog(displayInfo,warningString,"Ok",NULL,NULL);
    return False;
  }

  renewString(&displayInfo->dlFile->name,f1);
  dmWriteDisplayList(displayInfo,stream);
  fclose(stream);

  displayInfo->hasBeenEditedButNotSaved = False;
  displayInfo->newDisplay = False;

  dm2kSetDisplayTitle(displayInfo);
  if (!brandNewFile) {
    chmod(f1,statBuf.st_mode);
  }

  return True;
}

void dm2kExit() 
{
  char        * filename;
  char        * tmp;
  char          str[2*MAX_FILE_CHARS];
  Boolean       saveAll = False;
  Boolean       saveThis = False;
  DisplayInfo * displayInfo = displayInfoListHead;

  while (displayInfo) 
  {
    /* popup display window
     */
    XMapWindow (display, XtWindow (displayInfo->shell));
    XRaiseWindow(display, XtWindow (displayInfo->shell));

    if (displayInfo->hasBeenEditedButNotSaved) {
      if (saveAll == False) {
        filename = tmp = displayInfo->dlFile->name;
        /* strip off the path */
        while (*tmp != '\0') {
          if (*tmp == '/') 
            filename = tmp + 1;
          tmp++;
        }
        sprintf(str,"Save display \"%s\" before exit?",filename);
        if (displayInfo->next)
          dmSetAndPopupQuestionDialog(displayInfo,str,"Yes","No","All");
        else
          dmSetAndPopupQuestionDialog(displayInfo,str,"Yes","No",NULL);
        switch (displayInfo->questionDialogAnswer) {
          case 1 :
          /* Yes, save this file */
            saveThis = True;
            break;
          case 2 :
            /* No, check next file */
            saveThis = False;
            break;
          case 3 :
            /* save all files */
            saveAll = True;
            saveThis = True;
	          break;
          default :
            saveThis = False;
            break;
        }
      }

      if (saveThis == True)  
      {
	Boolean status;

	/* check versions
	 */
	if (DM2K_ADL_FILE_JUMP_VERSION > displayInfo->dlFile->versionNumber) 
	{
	  int button;

	  button = getUserChoiseViaPopupQuestionDialog 
	    (displayInfo->shell,
	     DM2K_DIALOG_MESSAGE_LABEL, 
	     "Format of original adl file differs from current one.",
	     DM2K_DIALOG_OK_BUTTON,     "Save in current format",
	     DM2K_DIALOG_CUSTEM_BUTTON, "Save in original format",
	     DM2K_DIALOG_CANCEL_BUTTON, "Cancel",
	     DM2K_DIALOG_TITLE,         "Format file notice",
	     NULL);

	  if (button == 0) 
	    {
	      /* Save in current format 
	       */
	      Dm2kUseNewFileFormat = True; 
	      status = dm2kSaveDisplay(displayInfo, 
				       displayInfo->dlFile->name,True);
	    }
	  else if (button == 1)
	    {
	      /* Save in original format 
	       */
	      Dm2kUseNewFileFormat = False;
	      status = dm2kSaveDisplay(displayInfo, 
				       displayInfo->dlFile->name,True);
	    }	      
	}
      else
	{
	  /* Save in current format 
	   */
	  Dm2kUseNewFileFormat = True; 
	  status = dm2kSaveDisplay(displayInfo, 
				   displayInfo->dlFile->name,True);
	}

        if (status == False) 
	  return;
      }
    }
    displayInfo = displayInfo->next;
  }

  XtVaSetValues(mainShell, XmNiconic, False, NULL);
  XtPopup(mainShell,XtGrabNone);
  if (dm2kPromptToExit) {
     XtManageChild(exitQD);
     XtPopup(XtParent(exitQD),XtGrabNone);
  } else {
     dm2kClearImageCache();
     dm2kCATerminate();
     dmTerminateX();
     exit(0);
  }
}

void openFaceplate( const char *fname ) 
{
   char *dir = NULL;
   char name[FULLPATHNAME_SIZE];
   int startPos;
   Boolean canAccess = False;
   char fullPathName[FULLPATHNAME_SIZE+1];

   /* T. Straumann: protect against null pointer arg */
   if (!fname) return;

   if ((canAccess = !access(fname,R_OK|F_OK))) {
      strcpy( fullPathName, fname );
   } else {
      dir = getenv(DISPLAY_LIST_ENV);
      if (dir != NULL) {
	 startPos = 0;
	 while (extractStringBetweenColons(dir,name,startPos,&startPos)) {
	    if (STRLEN(name)+STRLEN(fname)+1 < (size_t) FULLPATHNAME_SIZE) {
	       strcpy(fullPathName,name);
	       strcat(fullPathName,"/");
	       strcat(fullPathName,fname);
	       if ((canAccess = !access(fullPathName,R_OK|F_OK))) break;
	    }
	 }
      }
   }

   if (canAccess) {
      FaceplateGroup * fpg = createFaceplateGroup(fullPathName);
   
      if (fpg != NULL)
	 buildDisplayFromFaceplateGroup(fpg);
      
      destroyFaceplateGroup(fpg);
   }
}


#ifdef __cplusplus
static void palettesMenuSimpleCallback(Widget, XtPointer cd, XtPointer)
#else
static void palettesMenuSimpleCallback(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  int buttonNumber = (int) cd;
  
    switch(buttonNumber) {

	case PALETTES_OBJECT_BTN:
		/* fills in global objectMW */
		if (objectMW == NULL) createObject();
		XtPopup(objectS,XtGrabNone);
		break;

	case PALETTES_RESOURCE_BTN:
		/* fills in global resourceMW */
		if (resourceMW == NULL) createResource();
		/* (MDA) this is redundant - done at end of createResource() */
		XtPopup(resourceS,XtGrabNone);
		break;

	case PALETTES_COLOR_BTN:
		/* fills in global colorMW */
		if (colorMW == NULL)
		   setCurrentDisplayColorsInColorPalette(BCLR_RC,0);
		XtPopup(colorS,XtGrabNone);
		break;

    }

}

#ifdef __cplusplus
static void toolsMenuSimpleCallback(Widget, XtPointer cd, XtPointer)
#else
static void toolsMenuSimpleCallback(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  int buttonNumber = (int) cd;
  
  switch(buttonNumber) {
      
  case TOOLS_FACEPLATE_BTN:
    create_shell5 (mainShell);
    break;
  case TOOLS_DUMPDISPLAYINFO_BTN:
    dumpDisplayInfoList();
  }
}


static void helpMenuSimpleCallback (Widget w, XtPointer cd, XtPointer cbs)
{
  int buttonNumber = (int) cd;

  switch (buttonNumber)
    {
      case HELP_ON_VERSION_BTN:
	XtManageChild (productDescriptionShell);
	XtPopup (XtParent (productDescriptionShell), XtGrabNone);
	break;
    }
}


#ifdef __cplusplus
static void helpDialogCallback(Widget, XtPointer, XtPointer cbs)
#else
static void helpDialogCallback(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  switch(((XmAnyCallbackStruct *) cbs)->reason){
	case XmCR_OK:
	case XmCR_CANCEL:
		XtPopdown(helpS);
		break;
  }
}


static void cleanDisplay (DisplayInfo *displayInfo, DlTraversalMode mode)
{
  displayInfo->traversalMode = mode;
  if (displayInfo->fromRelatedDisplayExecution) {
    dmRemoveDisplayInfo(displayInfo);
  } else {
    dmCleanupDisplayInfo(displayInfo,False);
  }
}


void cleanDisplayModeEdit ()
{
  /* This routine must not be called if the mode was just changed */
  if ( globalDisplayListTraversalMode != DL_EDIT )
    return;

  cleanObjectID ();
  executeTimeCartesianPlotWidget = NULL;
  cleanDisplay (currentDisplayInfo, globalDisplayListTraversalMode);
}


void executeAllDisplayMode ()
{
  DisplayInfo *displayInfo;
  DlTraversalMode mode = globalDisplayListTraversalMode;

/* unselect anything that might be selected */
#if 0
  highlightAndSetSelectedElements(NULL,0,0);
#endif
  clearResourcePaletteEntries();

  disableEditFunctions();

  cleanObjectID ();
  switch(mode) {
      case DL_EDIT:
	if (relatedDisplayS) XtSetSensitive(relatedDisplayS,True);
	if (cartesianPlotS) XtSetSensitive(cartesianPlotS,True);
	if (cartesianPlotAxisS) XtSetSensitive(cartesianPlotAxisS,True);
	if (stripChartS) XtSetSensitive(stripChartS,True);
	XtSetSensitive(fileMenu[FILE_NEW_BTN].widget,True);
	XtSetSensitive(fileMenu[FILE_SAVE_BTN].widget,True);
	XtSetSensitive(fileMenu[FILE_SAVE_AS_BTN].widget,True);
	XtSetSensitive(fileMenu[FILE_FACEPLATELOAD_BTN].widget,False);

        if (dm2kWorkProcId) {
          XtRemoveWorkProc(dm2kWorkProcId);
          dm2kWorkProcId = 0;
        }
	break;

      case DL_EXECUTE:
	if (relatedDisplayS) {
	   XtSetSensitive(relatedDisplayS,False);
	   XtPopdown(relatedDisplayS);
	}
	if (cartesianPlotS) {
	   XtSetSensitive(cartesianPlotS,False);
	   XtPopdown(cartesianPlotS);
	}
	if (cartesianPlotAxisS) {
	   XtSetSensitive(cartesianPlotAxisS,False);
	   XtPopdown(cartesianPlotAxisS);
	}
	if (stripChartS) {
	   XtSetSensitive(stripChartS,False);
	   XtPopdown(stripChartS);
	}

	/* Associated Menu dialog destroy */
	destroyAMPalette();

	XtSetSensitive(fileMenu[FILE_NEW_BTN].widget,False);
	XtSetSensitive(fileMenu[FILE_SAVE_BTN].widget,False);
	XtSetSensitive(fileMenu[FILE_SAVE_AS_BTN].widget,False);
	XtSetSensitive(fileMenu[FILE_FACEPLATELOAD_BTN].widget,True);

        dm2kStartUpdateCAStudyDlg();

	
	break;

      default:
	break;
  }

  executeTimeCartesianPlotWidget = NULL;
/* no display is current */
  currentDisplayInfo = (DisplayInfo *)NULL;

  displayInfo = displayInfoListHead;

  /*
   * Go through the whole display list, if any display is
   * brought up as a related display, shut down that display
   * and remove that display from the display list, otherwise,
   * just shutdown that display.
   */

  while (displayInfo) {
    DisplayInfo *pDI = displayInfo;

    displayInfo = displayInfo->next;
    cleanDisplay (pDI, mode);
  }

  /*
   * See whether there is any display in the display list.
   * If any, enable resource palette, object palette and
   * color palette, traverse the whole display list.
   */

  if (displayInfoListHead) {
    if (globalDisplayListTraversalMode == DL_EDIT) {
      enableEditFunctions();
    }
    currentDisplayInfo = displayInfoListHead;
    enableControllerRC (currentDisplayInfo);
    dmTraverseAllDisplayLists();
    XFlush(display);
  }
  caPendEvent ("executeAllDisplayMode");
}


#ifdef __cplusplus
static void modeCallback(Widget, XtPointer cd, XtPointer cbs)
#else
static void modeCallback(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  DlTraversalMode mode = (DlTraversalMode) cd;
  XmToggleButtonCallbackStruct *call_data = (XmToggleButtonCallbackStruct *) cbs;

/*
 * since both on & off will invoke this callback, only care about transition
 * of one to ON (with real change of state)
 */
  if (call_data->set == False ||
	globalDisplayListTraversalMode == mode) return;

/*
 * set all the displayInfo->traversalMode(s) to the specified mode, and
 * then invoke the traversal
 */
  globalDisplayListTraversalMode = mode;
  executeAllDisplayMode ();
  mouseButtonHelp ();
}


static void createCursors()
{ 
  XCharStruct overall;
  int         dir, asc, desc;
  Pixmap      sourcePixmap, maskPixmap;
  XColor      colors[2];
  GC          gc;
  Dimension   hotSpotWidth = HOT_SPOT_WIDTH;
  Dimension   radius;


  /* create pixmap cursors
   */
  colors[0].pixel = BlackPixel(display,screenNum);
  colors[1].pixel = WhitePixel(display,screenNum);
  XQueryColors(display,cmap,colors,2);

  /* CLOSE cursor 
   */
  XTextExtents(fontTable[6],"Close",5,&dir,&asc,&desc,&overall);
  sourcePixmap = XCreatePixmap(display,RootWindow(display,screenNum),
			       overall.width+hotSpotWidth,asc+desc,1);
  maskPixmap = XCreatePixmap(display,RootWindow(display,screenNum),
			     overall.width+hotSpotWidth,asc+desc,1);
  gc =  XCreateGC(display,sourcePixmap,0,NULL);
  XSetBackground(display,gc,0);
  XSetFunction(display,gc,GXcopy);

  /* an arbitrary modest-sized font from the font table 
   */
  XSetFont(display,gc,fontTable[6]->fid);
  XSetForeground(display,gc,0);
  XFillRectangle(display,sourcePixmap,gc,0,0,overall.width+hotSpotWidth,
		 asc+desc);
  XFillRectangle(display,maskPixmap,gc,0,0,overall.width+hotSpotWidth,
		 asc+desc);
  radius = MIN(hotSpotWidth,(Dimension)((asc+desc)/2));
  XSetForeground(display,gc,1);
  XFillArc(display,maskPixmap,gc,hotSpotWidth/2 - radius/2,
	   (asc+desc)/2 - radius/2,radius,radius,0,360*64);
  XDrawString(display,sourcePixmap,gc,hotSpotWidth,asc,"Close",5);
  XDrawString(display,maskPixmap,gc,hotSpotWidth,asc,"Close",5);
  closeCursor = XCreatePixmapCursor(display,sourcePixmap,maskPixmap,
				    &colors[0],&colors[1],0,(asc+desc)/2);
  XFreePixmap(display,sourcePixmap);
  XFreePixmap(display,maskPixmap);

  /* SAVE cursor 
   */
  XTextExtents(fontTable[6],"Save",4,&dir,&asc,&desc,&overall);
  sourcePixmap = XCreatePixmap(display,RootWindow(display,screenNum),
			       overall.width+hotSpotWidth,asc+desc,1);
  maskPixmap = XCreatePixmap(display,RootWindow(display,screenNum),
			     overall.width+hotSpotWidth,asc+desc,1);
  XSetForeground(display,gc,0);
  XFillRectangle(display,sourcePixmap,gc,0,0,overall.width+hotSpotWidth,
		 asc+desc);
  XFillRectangle(display,maskPixmap,gc,0,0,overall.width+hotSpotWidth,
		 asc+desc);
  radius = MIN(hotSpotWidth,(Dimension)((asc+desc)/2));
  XSetForeground(display,gc,1);
  XFillArc(display,maskPixmap,gc,hotSpotWidth/2 - radius/2,
	   (asc+desc)/2 - radius/2,radius,radius,0,360*64);
  XDrawString(display,sourcePixmap,gc,hotSpotWidth,asc,"Save",4);
  XDrawString(display,maskPixmap,gc,hotSpotWidth,asc,"Save",4);
  saveCursor = XCreatePixmapCursor(display,sourcePixmap,maskPixmap,
				   &colors[0],&colors[1],0,(asc+desc)/2);
  XFreePixmap(display,sourcePixmap);
  XFreePixmap(display,maskPixmap);

  /* PRINT cursor
   */
  XTextExtents(fontTable[6],"Print",5,&dir,&asc,&desc,&overall);
  sourcePixmap = XCreatePixmap(display,RootWindow(display,screenNum),
			       overall.width+hotSpotWidth,asc+desc,1);
  maskPixmap = XCreatePixmap(display,RootWindow(display,screenNum),
			     overall.width+hotSpotWidth,asc+desc,1);
  XSetForeground(display,gc,0);
  XFillRectangle(display,sourcePixmap,gc,0,0,overall.width+hotSpotWidth,
		 asc+desc);
  XFillRectangle(display,maskPixmap,gc,0,0,overall.width+hotSpotWidth,
		 asc+desc);
  radius = MIN(hotSpotWidth,(Dimension)((asc+desc)/2));
  XSetForeground(display,gc,1);
  XFillArc(display,maskPixmap,gc,hotSpotWidth/2 - radius/2,
	   (asc+desc)/2 - radius/2,radius,radius,0,360*64);
  XDrawString(display,sourcePixmap,gc,hotSpotWidth,asc,"Print",5);
  XDrawString(display,maskPixmap,gc,hotSpotWidth,asc,"Print",5);
  printCursor = XCreatePixmapCursor(display,sourcePixmap,maskPixmap,
				    &colors[0],&colors[1],0,(asc+desc)/2);
  XFreePixmap(display,sourcePixmap);
  XFreePixmap(display,maskPixmap);

  /* no write access cursor 
   */
  colors[0].pixel = alarmColorPixel[MAJOR_ALARM];
  colors[1].pixel = WhitePixel(display,screenNum);
  XQueryColors(display,cmap,colors,2);

  sourcePixmap = XCreateBitmapFromData(display,RootWindow(display,screenNum),
				       (char *)noAccess25_bits, 
				       noAccess25_width, noAccess25_height);
  maskPixmap = XCreateBitmapFromData(display,RootWindow(display,screenNum),
				     (char *)noAccessMask25_bits, 
				     noAccessMask25_width, 
				     noAccessMask25_height);
  noWriteAccessCursor = XCreatePixmapCursor(display,sourcePixmap,maskPixmap,
					    &colors[0],&colors[1],13,13);
  XFreePixmap(display,sourcePixmap);
  XFreePixmap(display,maskPixmap);

  /* big hand cursor
   */
  if (dm2kUseBigCursor) {
    colors[0].pixel = BlackPixel(display,screenNum);
    colors[1].pixel = WhitePixel(display,screenNum);
    XQueryColors(display,cmap,colors,2);

    sourcePixmap = XCreateBitmapFromData(display,RootWindow(display,screenNum),
					 (char *)bigHand25_bits,
					 bigHand25_width, bigHand25_height);
    maskPixmap = XCreateBitmapFromData(display,RootWindow(display,screenNum),
				       (char *)bigHandMask25_bits, 
				       bigHandMask25_width,
				       bigHandMask25_height);
    rubberbandCursor = XCreatePixmapCursor(display,sourcePixmap,maskPixmap,
					   &colors[0],&colors[1],1,2);
    XFreePixmap(display,sourcePixmap);
    XFreePixmap(display,maskPixmap);
  } else {
    rubberbandCursor = XCreateFontCursor(display,XC_hand2);
  }

  /* big cross cursor 
   */
  if (dm2kUseBigCursor) {
    colors[0].pixel = BlackPixel(display,screenNum);
    colors[1].pixel = WhitePixel(display,screenNum);
    XQueryColors(display,cmap,colors,2);

    sourcePixmap = XCreateBitmapFromData(display,RootWindow(display,screenNum),
					 (char *)bigCross25_bits, 
					 bigCross25_width, bigCross25_height);
    maskPixmap = XCreateBitmapFromData(display,RootWindow(display,screenNum),
				       (char *)bigCrossMask25_bits, 
				       bigCrossMask25_width,
				       bigCrossMask25_height);
    crosshairCursor = XCreatePixmapCursor(display,sourcePixmap,maskPixmap,
					  &colors[0],&colors[1],13,13);
    XFreePixmap(display,sourcePixmap);
    XFreePixmap(display,maskPixmap);
  } else {
    crosshairCursor = XCreateFontCursor(display,XC_crosshair);
  }

  /* big 4 way pointers 
   */
  if (dm2kUseBigCursor) {
    colors[0].pixel = BlackPixel(display,screenNum);
    colors[1].pixel = WhitePixel(display,screenNum);
    XQueryColors(display,cmap,colors,2);

    sourcePixmap = XCreateBitmapFromData(display,RootWindow(display,screenNum),
					 (char *)big4WayPtr25_bits, 
					 big4WayPtr25_width, 
					 big4WayPtr25_height);
    maskPixmap = XCreateBitmapFromData(display,RootWindow(display,screenNum),
				       (char *)big4WayPtrMask25_bits, 
				       big4WayPtrMask25_width,
				       big4WayPtrMask25_height);
    dragCursor = XCreatePixmapCursor(display,sourcePixmap,maskPixmap,
				     &colors[0],&colors[1],13,13);
    XFreePixmap(display,sourcePixmap);
    XFreePixmap(display,maskPixmap);
  } else {
    dragCursor = XCreateFontCursor(display,XC_fleur);
  }

  /* big size cursor pointers 
   */
  if (dm2kUseBigCursor) {
    colors[0].pixel = BlackPixel(display,screenNum);
    colors[1].pixel = WhitePixel(display,screenNum);
    XQueryColors(display,cmap,colors,2);

    sourcePixmap = XCreateBitmapFromData(display,RootWindow(display,screenNum),
					 (char *)bigSizeCursor25_bits, 
					 bigSizeCursor25_width,
					 bigSizeCursor25_height);
    maskPixmap = XCreateBitmapFromData(display,RootWindow(display,screenNum),
				       (char *)bigSizeCursorMask25_bits, 
				       bigSizeCursorMask25_width,
				       bigSizeCursorMask25_height);
    resizeCursor = XCreatePixmapCursor(display,sourcePixmap,maskPixmap,
				       &colors[0],&colors[1],25,25);
    XFreePixmap(display,sourcePixmap);
    XFreePixmap(display,maskPixmap);
  } else {
    resizeCursor = XCreateFontCursor(display,XC_bottom_right_corner);
  }

  /* big watch cursor pointers 
   */
  if (dm2kUseBigCursor) {
    colors[0].pixel = BlackPixel(display,screenNum);
    colors[1].pixel = WhitePixel(display,screenNum);
    XQueryColors(display,cmap,colors,2);
 
    sourcePixmap = XCreateBitmapFromData(display,RootWindow(display,screenNum),
					 (char *)bigWatchCursor25_bits, 
					 bigWatchCursor25_width,
					 bigWatchCursor25_height);
    maskPixmap = XCreateBitmapFromData(display,RootWindow(display,screenNum),
				       (char *)bigWatchCursorMask25_bits, 
				       bigWatchCursorMask25_width,
				       bigWatchCursorMask25_height);
    watchCursor = XCreatePixmapCursor(display,sourcePixmap,maskPixmap,
				      &colors[0],&colors[1],25,25);
    XFreePixmap(display,sourcePixmap);
    XFreePixmap(display,maskPixmap);
  } 
  else {
    watchCursor = XCreateFontCursor(display,XC_watch);
  }

  /*  associatedMenu cursor pointers 
   */
  colors[0].pixel = BlackPixel(display,screenNum);
  colors[1].pixel = WhitePixel(display,screenNum);
  XQueryColors(display,cmap,colors,2);
 
  sourcePixmap = XCreateBitmapFromData(display,
				       RootWindow(display,screenNum),
				       (char *)associatedMenuCursor25_bits, 
				       associatedMenuCursor25_width,
				       associatedMenuCursor25_height);

  maskPixmap = XCreateBitmapFromData(display,RootWindow(display,screenNum),
				     (char *)associatedMenuCursorMask25_bits, 
				     associatedMenuCursorMask25_width,
				     associatedMenuCursorMask25_height);

  associatedMenuCursor = XCreatePixmapCursor(display,
					     sourcePixmap,maskPixmap,
					     &colors[0],&colors[1],12,0);
  XFreePixmap(display, sourcePixmap);
  XFreePixmap(display, maskPixmap);

  XFreeGC(display,gc);

  /* readOnly indication cursor
   */
  sourcePixmap = XCreateBitmapFromData(display,
				       RootWindow(display,screenNum),
				       (char *)readOnlyCursor25_bits, 
				       readOnlyCursor25_width,
				       readOnlyCursor25_height);

  maskPixmap = XCreateBitmapFromData(display,RootWindow(display,screenNum),
				     (char *)readOnlyCursorMask25_bits, 
				     readOnlyCursorMask25_width,
				     readOnlyCursorMask25_height);

  readOnlyCursor = XCreatePixmapCursor(display,
					     sourcePixmap,maskPixmap,
					     &colors[0],&colors[1],12,0);
  XFreePixmap(display, sourcePixmap);
  XFreePixmap(display, maskPixmap);

  /* now create standard font cursors
   */
  helpCursor   = XCreateFontCursor(display,  XC_question_arrow);
  xtermCursor  = XCreateFontCursor(display,  XC_xterm);
  simpleCursor = XCreateFontCursor (display, XC_top_left_arrow);
}


void enableCtrlObjMenu (Boolean sensitive, DlElementType DlType)
{
  menuEntry_t *items;
  int i;

  items = controllersObjectMenu;
  for ( i=0 ; items[i].label != NULL ; i++ ) {
    if ( (DlElementType) items[i].callbackData != DlType ) continue;
    if ( items[i].widget ) XtSetSensitive (items[i].widget, sensitive);
    break;
  }
}


Widget buildMenu (Widget parent,
		  int menuType,
		  char *menuTitle,
		  char menuMnemonic,
		  Boolean tear_off,
		  menuEntry_t *items)
{
  Widget   menu;
  Widget   cascade;
  int      i;
  XmString str;
  Arg      args[8];
  int      n;
 
  n = 0;
  XtSetArg (args[n], XmNtearOffModel, (tear_off) ? XmTEAR_OFF_ENABLED : XmTEAR_OFF_DISABLED); n++;

  if (menuType == XmMENU_PULLDOWN) {
    menu = XmCreatePulldownMenu (parent, "_pulldown",args, n);
    str = XmStringCreateSimple(menuTitle);
    cascade = XtVaCreateManagedWidget(menuTitle,
      xmCascadeButtonGadgetClass, parent,
      XmNsubMenuId, menu,
      XmNlabelString, str,
      XmNmnemonic, menuMnemonic,
      NULL);
    XmStringFree(str);
  } else {
    menu = XmCreatePopupMenu(parent, "_popup", args, n);
  }

  /* now add the menu items */
  for (i=0;items[i].label != NULL; i++) {
    /* if subitems exist, create the pull-right menu by calling this
     * function recursively. Since the function returns a cascade
     * button, the widget returned is used..
     */

    if (items[i].subItems) {
      items[i].widget = buildMenu(menu, XmMENU_PULLDOWN,
			 items[i].label,
			 items[i].mnemonic,
			 tear_off,
			 items[i].subItems);
    } else {
      items[i].widget = XtVaCreateManagedWidget(items[i].label,
			 *items[i].widgetClass, menu,
			 NULL);
    }

    /* Whether the item is a real item or a cascade button with a
     * menu, it can still have a mnemonic.
     */
    if (items[i].mnemonic) {
      XtVaSetValues(items[i].widget, XmNmnemonic, items[i].mnemonic, NULL);
    }

    /* any item can have an accelerator, execpt cascade menus. But,
     * we don't worry about that; we know better in our declarations.
     */
    if (items[i].accelerator) {
      str = XmStringCreateSimple(items[i].accText);
      XtVaSetValues(items[i].widget,
	  XmNaccelerator, items[i].accelerator,
	  XmNacceleratorText, str,
	  NULL);
      XmStringFree(str);
    }
    /* again, anyone can have a callback -- however, this is an
     * activate-callback.  This may not be appropriate for all items.
     */
    if (items[i].callback) {
      XtAddCallback(items[i].widget, XmNactivateCallback,
	items[i].callback, items[i].callbackData);
    }
  }
  return (menuType == XmMENU_POPUP) ? menu : cascade;
}


/*
 * globals needed for remote display invocation...
 *   have limited scope (main() and dmTerminateX() are only routines which care
 */
Atom DM2K_EDIT_FIXED = (Atom)NULL, DM2K_EXEC_FIXED = (Atom)NULL;
Atom DM2K_EDIT_SCALABLE = (Atom)NULL, DM2K_EXEC_SCALABLE = (Atom)NULL;


/* 
 * SIGNAL HANDLER
 *   function to perform cleanup of X root window DM2K... properties
 *   which accomodate remote display requests
 */
static void handleSignals(int sig)
{
  char        filename[80];
  DisplayInfo *displayInfo;
  int         i = 0;
  int         printreport = 0;

  char *username = 0;

#ifdef CRASH_DEBUG
  static int  being_called = 1;
#else
  static int  being_called = 0;
#endif

#ifdef CRASH_DEBUG
  if      (sig==SIGQUIT)   fprintf(stderr,"\nSIGQUIT\n");
  else if (sig==SIGINT)    fprintf(stderr,"\nSIGINT\n");
  else if (sig==SIGTERM)   fprintf(stderr,"\nSIGTERM\n");
  else if (sig==SIGSEGV)   fprintf(stderr,"\nSIGSEGV\n");
  else if (sig==SIGBUS)    fprintf(stderr,"\nSIGBUS\n");
  
  /* remove the properties on the root window 
   */
  if (DM2K_EDIT_FIXED != (Atom)NULL)
    XDeleteProperty(display,rootWindow,DM2K_EDIT_FIXED);
  
  if (DM2K_EXEC_FIXED != (Atom)NULL)
    XDeleteProperty(display,rootWindow,DM2K_EXEC_FIXED);
  
  if (DM2K_EDIT_SCALABLE != (Atom)NULL)
    XDeleteProperty(display,rootWindow,DM2K_EDIT_SCALABLE);
  
  if (DM2K_EXEC_SCALABLE != (Atom)NULL)
    XDeleteProperty(display,rootWindow,DM2K_EXEC_SCALABLE);

  /* T. Straumann: use XSync instead of XFlush; see comment
   *				 in 'dm2kWidget.c'
   */
   XSync(display,False);

#endif

  if (being_called == 0) 
  {
    being_called++;
    
    if      (sig==SIGQUIT)   fprintf(stderr,"\nSIGQUIT\n");
    else if (sig==SIGINT)    fprintf(stderr,"\nSIGINT\n");
    else if (sig==SIGTERM)   fprintf(stderr,"\nSIGTERM\n");
    else if (sig==SIGSEGV)   fprintf(stderr,"\nSIGSEGV\n");
    else if (sig==SIGBUS)    fprintf(stderr,"\nSIGBUS\n");
    
    /* remove the properties on the root window */
    if (DM2K_EDIT_FIXED != (Atom)NULL)
      XDeleteProperty(display,rootWindow,DM2K_EDIT_FIXED);
    
    if (DM2K_EXEC_FIXED != (Atom)NULL)
      XDeleteProperty(display,rootWindow,DM2K_EXEC_FIXED);
    
    if (DM2K_EDIT_SCALABLE != (Atom)NULL)
      XDeleteProperty(display,rootWindow,DM2K_EDIT_SCALABLE);
    
    if (DM2K_EXEC_SCALABLE != (Atom)NULL)
      XDeleteProperty(display,rootWindow,DM2K_EXEC_SCALABLE);
    
	/* T. Straumann: use XSync instead of XFlush; see comment
	 *				 in 'dm2kWidget.c'
	 */
    XSync(display,False);

    displayInfo = displayInfoListHead;

    username = getenv("USER");

    while(displayInfo) 
    {
      memset(filename, 80, '\0');
      sprintf(filename, "/tmp/SAVE-%s-%-d.adl", username?username:"UNKNOWN", i++);
      if (displayInfo->hasBeenEditedButNotSaved) {
	if (printreport++ == 0)
	  printf("\n\n\t============> Save edited filed <=============\n");
	dm2kSaveDisplay(displayInfo, filename, TRUE);
	fprintf(stderr, "saving display into %s \n", filename);
      }
      if (printreport)
	printf("\n\n\t===============================================\n\n");
      displayInfo = displayInfo->next;
    }
  }

  if (sig == SIGSEGV || sig == SIGBUS) {
  /* want core dump */
    abort();
  } else {
  /* and exit */
    exit(0);
  }
}


/*
 * the function to take a full path name and macro string,
 *  and send it as a series of clientMessage events to the root window
 *  with the specified Atom.
 *
 *  this allows the "smart-startup" feature to be independent of
 *  the actual property value (since properties can change faster
 *  than DM2K can handle them).
 *
 *  client message events are sent as full path names, followed by
 *  a semi-colon delimiter, followed by an optional macro string,
 *  delimited by parentheses {e.g., (/abc/def;a=b,c=d) or (/abc/def)}
 *  to allow the receiving client to figure out
 *  when it has received all the relevant information for that
 *  request.  (i.e., the string will usually be split up across
 *  events, therefore the parentheses mechanism allows the receiver to
 *  concatenate just enough client message events to properly handle the
 *  request).
 */

void sendFullPathNameAndMacroAsClientMessages(
  Window targetWindow,
  char *fullPathName,
  char *macroString,
  char *geometryString,
  Atom atom)
{
  XClientMessageEvent clientMessageEvent;
  int index, i;
  char *ptr;

#define MAX_CHARS_IN_CLIENT_MESSAGE 20	/* as defined in XClientEventMessage */

  clientMessageEvent.type = ClientMessage;
  clientMessageEvent.serial = 0;
  clientMessageEvent.send_event = True;
  clientMessageEvent.display = display;
  clientMessageEvent.window = targetWindow;
  clientMessageEvent.message_type = atom;
  clientMessageEvent.format = 8;
  ptr = fullPathName;
/* leading "(" */
  clientMessageEvent.data.b[0] = '(';
  index = 1;

/* body of full path name string */
  while (ptr[0] != '\0') {
    if (index == MAX_CHARS_IN_CLIENT_MESSAGE) {
      XSendEvent(display,targetWindow,True,NoEventMask,
		(XEvent *)&clientMessageEvent);
      index = 0;
    }
    clientMessageEvent.data.b[index++] = ptr[0];
    ptr++;
  }

  /* ; delimiter */
  if (index == MAX_CHARS_IN_CLIENT_MESSAGE) {
    XSendEvent(display,targetWindow,True,NoEventMask,
              (XEvent *)&clientMessageEvent);
    index = 0;
  }
  clientMessageEvent.data.b[index++] = ';';


/* body of macro string if one was specified */
  if ((ptr = macroString) != NULL) {
    while (ptr[0] != '\0') {
      if (index == MAX_CHARS_IN_CLIENT_MESSAGE) {
        XSendEvent(display,targetWindow,True,NoEventMask,
		(XEvent *)&clientMessageEvent);
        index = 0;
      }
      clientMessageEvent.data.b[index++] = ptr[0];
      ptr++;
    }
  }

  /* ; delimiter */
  if (index == MAX_CHARS_IN_CLIENT_MESSAGE) {
    XSendEvent(display,targetWindow,True,NoEventMask,
              (XEvent *)&clientMessageEvent);
    index = 0;
  }
  clientMessageEvent.data.b[index++] = ';';

/* body of geometry string if one was specified */
  if ((ptr = geometryString) != NULL) {
    while (ptr[0] != '\0') {
      if (index == MAX_CHARS_IN_CLIENT_MESSAGE) {
        XSendEvent(display,targetWindow,True,NoEventMask,
                (XEvent *)&clientMessageEvent);
        index = 0;
      }
      clientMessageEvent.data.b[index++] = ptr[0];
      ptr++;
    }
  }

/* trailing ")" */
  if (index == MAX_CHARS_IN_CLIENT_MESSAGE) {
    XSendEvent(display,targetWindow,True,NoEventMask,
		(XEvent *)&clientMessageEvent);
    index = 0;
  }
  clientMessageEvent.data.b[index++] = ')';
/* fill out client event with spaces just for "cleanliness" */
  for (i = index; i < MAX_CHARS_IN_CLIENT_MESSAGE; i++)
	clientMessageEvent.data.b[i] = ' ';
  XSendEvent(display,targetWindow,True,NoEventMask,
		(XEvent *)&clientMessageEvent);

}

typedef enum {FILENAME_MSG,MACROSTR_MSG,GEOMETRYSTR_MSG} msgClass_t;

static void version (char *pname, FILE *stream)
{
   if (strrchr(pname,'/')) pname=strrchr(pname,'/')+1;
   fprintf(stream, "%s version %d.%d.%d%s (built %s by %s)\n", pname, DM2K_VERSION,
	   DM2K_REVISION, DM2K_UPDATE_LEVEL, DM2K_VERSION_TAG, 
	   BUILT_AT, BUILT_BY);
}

static void usage (char *pname, FILE *stream)
{
  static const char usageText[] = "\n\
       [-displayFont <font>]\n\
       [-macro \"<name>=<value>,<name>=<value>...\"]\n\
       [display file names]\n\
\n\
   parameters:\n\
       -displayFont      select alias for scalable fonts\n\
       -macro            apply macro substitution\n\n\
   options:\n\
       -version	     	 output version info and exit\n\
       -readonly     	 run in read only mode (no CA put)\n\
       -local        	 don't participate in remote display protocol\n\
       -cleanup      	 support remote display protocol, ignore existing instances\n\
       -cmap         	 use private color map\n\
       -dump         	 dump compiled in fallback-resources to stdout and exit\n\
       -bigMousePointer  use big cursors\n\
       -promptExit       prompt user before exiting DM2K"
#if PROMPT_TO_EXIT==1
" (default)"
#endif
"\n       -dontPromptExit   dont!"
#if PROMPT_TO_EXIT==0
" (default)"
#endif
"\n       -silent           reduced error mesages\n\
       -wmPositionPolicy define the WM positioning policy : FRAME or USER\n\
       -verbose          display verbose error messages\n\
       -debug            provide debugging informations\n\
       -synchro          run X11 protocol in synchronous mode\n\
       -help             this message\n\n\
   Environment variables:\n\n\
       %s : (default unset)\n\
            open panels in execute-mode but read only\n\n\
       %s : (default unset)\n\
            value = FRAME / USER to define the WM positioning policy.\n\n\
       %s : (default: 'ColorRules')\n\
            to define the name of color rule file. If the env is not set,\n\
            then dm2k reads file ColorRules in current directory.\n\
            If there is no such file, it uses default.\n\n\
       %s : (default: 'GraphicRules')\n\
            to define the name of graphic rule file.\n\
            If the env is not set, then dm2k reads file\n\
            GraphicRules in current directory.\n\n\
       %s : (default: unset)\n\
            A default adl-file to be called on MB3-click on various elemnts.\n\
            It is given the macro 'record=<PV>' of the PV of the selected\n\
            element.\n\n\
       %s : (default: 'mail')\n\
            The mail-command to use in the Message Window\n\
            The program is called the following way:\n\
                '<mail_cmd> -s <subject> <recipient>'\n\
            and the message is given on stdin.\n\
\n\
       EPICS_DISPLAY_PATH : (default '.')\n\
            adl-files, mfp-files and gif-files are searched in these\n\
            colon-separated directories.\n\n\
       PSPRINTER : (default: unset)\n\
            PostScript printer to use for printing displays\n\n\
       EPICS_PS_PRINT_CMD : (default: unset)\n\
            complete command the PostScript output to be printed is piped through.\n\
            (e.g.: \"lp\" or \"tee LastPrintout.ps | lp\" or \"ghostview -\")\n";

  fprintf(stream, "\nUsage: %s [-x|-e] [options...] ", pname);
  fprintf(stream, usageText, READ_ONLY_ENV, DM2K_WM_POSITION_POLICY_ENV,
	  DM2K_COLOR_RULE_ENV, DM2K_GRAPHIC_RULE_ENV,
	  EPICS_DM2K_DEFAULT_MB3_DISPLAY_ENV, DM2K_MAIL_CMD_ENV);
}

/*------------------------------------------------------------------------*/
/* Routines to create / destroy the display popup menus
   ----------------------------------------------------
     They cannot be children  of a shell widget (Linux X server pb !)
*/

void destroyPopupDisplayMenu (DisplayInfo *displayInfo, Widget parent)
{
  if ( displayInfo->editPopupMenu ) {
    XtDestroyWidget (displayInfo->editPopupMenu);
    displayInfo->editPopupMenu = NULL;
  }
  if ( displayInfo->executePopupMenu ) {
    XtDestroyWidget (displayInfo->executePopupMenu);
    displayInfo->executePopupMenu = NULL;
  }
}

void createPopupDisplayMenu (
  DlTraversalMode traversalMode,
  DisplayInfo *displayInfo,
  Widget widget)
{
  Widget parent, *menu_pt;
  menuEntry_t *items;
  char *menuTitle;

#if 1
  parent = widget;      /* for Linux X server !!! */
  /* In Linux : a popup cannot be the direct child of a shell ! */
#else
  parent = XtParent(widget);
#endif

  if ( traversalMode == DL_EXECUTE ) {
    /* create the shell's EXECUTE popup menus */
    menu_pt = &(displayInfo->executePopupMenu);
    items = executeMenu;
    menuTitle = "executePopupMenu";
    }
  else if ( traversalMode == DL_EDIT ) {
    /* create the shell's EDIT popup menus */
    menu_pt = &(displayInfo->editPopupMenu);
    items = displayMenu;
    menuTitle = "displayMenu";
    }
  if ( *menu_pt ) return;
  *menu_pt = buildMenu (parent, XmMENU_POPUP, menuTitle, '\0', False, items);
  XtVaSetValues (*menu_pt, XmNuserData, displayInfo, NULL);
  displayInfo->executePropertyButton =
      ( globalDisplayListTraversalMode == DL_EXECUTE ) ?
      items[PROPERTY_IDX].widget : NULL;
}
/*------------------------------------------------------------------------*/

static void createMain()
{
  XmString     label;
  Widget       mainMB, mainBB, frame, frameLabel;
  Widget       modeRB, modeEditTB, modeExecTB;
  int          n;
  Arg          args[20];

  /* create a main window child of the main shell 
   */
  mainMW = XmCreateMainWindow(mainShell,"mainMW", NULL, 0);

  /* get default fg/bg colors from mainMW for later use 
   */
  n = 0;
  XtSetArg(args[n],XmNbackground,&defaultBackground);n++;
  XtSetArg(args[n],XmNforeground,&defaultForeground);n++;
  XtGetValues(mainMW, args, n);

  /* create the menu bar
   */
  mainMB = XmCreateMenuBar(mainMW,"mainMB",NULL,0);

  /* color mainMB properly (force so VUE doesn't interfere) 
   */
  colorMenuBar(mainMB,defaultForeground,defaultBackground);

  /* create the file pulldown menu pane
   */
  mainFilePDM = buildMenu(mainMB,XmMENU_PULLDOWN, "File", 'F', True, fileMenu);

  if (globalDisplayListTraversalMode == DL_EXECUTE) {
    XtSetSensitive(fileMenu[FILE_NEW_BTN].widget,False);
    XtSetSensitive(fileMenu[FILE_SAVE_BTN].widget,False);
    XtSetSensitive(fileMenu[FILE_SAVE_AS_BTN].widget,False);
  }
  else if (globalDisplayListTraversalMode == DL_EDIT){
    XtSetSensitive(fileMenu[FILE_FACEPLATELOAD_BTN].widget,False);
  }

  /* create the edit pulldown menu pane
   */
  mainEditPDM = buildMenu(mainMB,XmMENU_PULLDOWN, "Edit", 'E', True, editMenu);

  if (globalDisplayListTraversalMode == DL_EXECUTE)
      XtSetSensitive(mainEditPDM,False);

  /* create the view pulldown menu pane
   */
  mainViewPDM = buildMenu(mainMB,XmMENU_PULLDOWN, "View", 'V', True, viewMenu);

  /* create the palettes pulldown menu pane
   */
  mainPalettesPDM = buildMenu(mainMB,XmMENU_PULLDOWN, 
			      "Palettes", 'P', True, palettesMenu);

  if (globalDisplayListTraversalMode == DL_EXECUTE)
      XtSetSensitive(mainPalettesPDM,False);

  /* create the tools pulldown menu pane
   */
  mainToolsPDM = buildMenu(mainMB,XmMENU_PULLDOWN, 
			   "Tools", 'T', True, toolsMenu);

  /* create the help pulldown menu pane
   */
  mainHelpPDM = buildMenu(mainMB,XmMENU_PULLDOWN, "Help", 'H', True, helpMenu);

  XtVaSetValues(mainMB, XmNmenuHelpWidget, mainHelpPDM, NULL);

  n = 0;
  XtSetArg(args[n],XmNmarginHeight, 9); n++;
  XtSetArg(args[n],XmNmarginWidth,  18); n++;
  mainBB = XmCreateBulletinBoard(mainMW,"mainBB",args,n);
  XtAddCallback(mainBB,XmNhelpCallback, 
		globalHelpCallback,(XtPointer)HELP_MAIN);

  /* create mode frame
   */
  n = 0;
  XtSetArg(args[n],XmNshadowType,XmSHADOW_ETCHED_IN); n++;
  frame = XmCreateFrame(mainBB,"frame",args,n);

  label = XmStringCreateSimple("Mode");

  n = 0;
  XtSetArg(args[n],XmNlabelString,  label); n++;
  XtSetArg(args[n],XmNmarginWidth,  0); n++;
  XtSetArg(args[n],XmNmarginHeight, 0); n++;
  XtSetArg(args[n],XmNchildType,    XmFRAME_TITLE_CHILD); n++;
  frameLabel = XmCreateLabel(frame,"frameLabel",args,n);
  XmStringFree(label);

  XtManageChild(frameLabel);
  XtManageChild(frame);

  if (globalDisplayListTraversalMode == DL_EDIT) 
  {
    /* create the mode radio box and buttons
     */
    n = 0;
    XtSetArg(args[n],XmNorientation,  XmHORIZONTAL); n++;
    XtSetArg(args[n],XmNpacking,      XmPACK_COLUMN); n++;
    XtSetArg(args[n],XmNnumColumns,   1); n++;
    XtSetArg(args[n],XmNchildType,    XmFRAME_WORKAREA_CHILD); n++;

    modeRB = XmCreateRadioBox(frame,"modeRB",args,n);

    label = XmStringCreateSimple("Edit");

    n = 0;
    XtSetArg(args[n],XmNlabelString, label); n++;
    XtSetArg(args[n],XmNset,         TRUE); n++; /* start with EDIT as set */

    modeEditTB = XmCreateToggleButton(modeRB,"modeEditTB",args,n);

    XtAddCallback(modeEditTB,XmNvalueChangedCallback, 
		  modeCallback, (XtPointer)DL_EDIT);

    XmStringFree(label);

    label = XmStringCreateSimple("Execute");

    n = 0;
    XtSetArg(args[n],XmNlabelString,label); n++;
    modeExecTB = XmCreateToggleButton(modeRB,"modeExecTB",args,n);

    XtAddCallback(modeExecTB,XmNvalueChangedCallback,
		  modeCallback,(XtPointer)DL_EXECUTE);
    XmStringFree(label);

    XtManageChild(modeRB);
    XtSetSensitive (modeEditTB, !globalDm2kReadOnly);
    XtManageChild(modeEditTB);
    XtManageChild(modeExecTB);
  } 
  else 
  {
    /* if started in execute mode, then no editing allowed, therefore
     * the modeRB widget is really a frame with a label indicating
     * execute-only mode
     */

    label = XmStringCreateSimple("Execute-Only");

    n = 0;
    XtSetArg(args[n],XmNlabelString,   label); n++;
    XtSetArg(args[n],XmNmarginWidth,   2); n++;
    XtSetArg(args[n],XmNmarginHeight,  1); n++;
    XtSetArg(args[n],XmNchildType,     XmFRAME_WORKAREA_CHILD); n++;

    modeRB = XmCreateLabel(frame,"modeRB",args,n);

    XmStringFree(label);
    XtManageChild(modeRB);
  }

  /* manage the composites
   */
  XtManageChild(mainBB);
  XtManageChild(mainMB);
  XtManageChild(mainMW);


/************************************************
 ****** create main-window related dialogs ******
 ************************************************/

  if (dm2kPromptToExit) {
     /* create the Exit... warning dialog */
     exitQD = XmCreateQuestionDialog(XtParent(mainFilePDM),"exitQD",NULL,0);
     XtVaSetValues(XtParent(exitQD),XmNmwmDecorations, 
		   MWM_DECOR_ALL|MWM_DECOR_RESIZEH, NULL);
     XtUnmanageChild(XmMessageBoxGetChild(exitQD,XmDIALOG_HELP_BUTTON));
     XtAddCallback(exitQD,XmNcancelCallback,
		   fileMenuDialogCallback,(XtPointer)FILE_EXIT_BTN);
     XtAddCallback(exitQD,XmNokCallback,fileMenuDialogCallback,
		   (XtPointer)FILE_EXIT_BTN);
  }
   
   /*   create the Help information shell
    */
   n = 0;
   XtSetArg(args[n],XtNiconName,"Help"); n++;
   XtSetArg(args[n],XtNtitle,"Dm2k Help System"); n++;
   XtSetArg(args[n],XtNallowShellResize,TRUE); n++;
   XtSetArg(args[n],XmNkeyboardFocusPolicy,XmEXPLICIT); n++;

   /* map window manager menu Close function to application close... 
    */
   XtSetArg(args[n],XmNdeleteResponse,XmDO_NOTHING); n++;
   XtSetArg(args[n],XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_RESIZEH); n++;
   
   helpS = XtCreatePopupShell("helpS",topLevelShellWidgetClass,
			      mainShell,args,n);
   XmAddWMProtocolCallback(helpS,WM_DELETE_WINDOW,
			   wmCloseCallback,(XtPointer)OTHER_SHELL);
   n = 0;
   XtSetArg(args[n],XmNdialogType,XmDIALOG_INFORMATION); n++;
   helpMessageBox = XmCreateMessageBox(helpS,"helpMessageBox",
				       args,n);
   XtAddCallback(helpMessageBox,XmNcancelCallback,
		 helpDialogCallback, (XtPointer)NULL);
   XtAddCallback(helpMessageBox,XmNokCallback,
		 helpDialogCallback,(XtPointer)NULL);
   
   XtManageChild(helpMessageBox);

   /* T. Straumann: create the clipboard (used to transfer
    *			   PV names via the XA_PRIMARY selection)
    *			   as a child of the main shell.
    */
   clipbdInit(mainShell);


   /* and realize the toplevel shell widget
    */
   XtRealizeWidget(mainShell);
}


#ifdef __cplusplus
Boolean dm2kInitWorkProc(XtPointer) {
#else
Boolean dm2kInitWorkProc(XtPointer cd) {
#endif
  int i;
  for (i=0; i<LAST_INIT_C; i++) {
    if (dm2kInitTask[i].init == False) {
       dm2kInitTask[i].init = dm2kInitTask[i].initTask();
       return False;
    }
  }
  return True;
}

void enableEditFunctions()
 {
  if ( globalDm2kReadOnly ) {
    disableEditFunctions();
    return;
  }

  if (objectS)   XtSetSensitive(objectS,True);
  if (resourceS) XtSetSensitive(resourceS,True);
  if (colorS)    XtSetSensitive(colorS,True);
  if (channelS)  XtSetSensitive(channelS,True);
  if (variableS) XtSetSensitive(variableS,True);
  XtSetSensitive(mainEditPDM,True);
  XtSetSensitive(mainPalettesPDM,True);
}

void disableEditFunctions() {
  if (objectS)   XtSetSensitive(objectS,False);
  if (resourceS) XtSetSensitive(resourceS,False);
  if (colorS)    XtSetSensitive(colorS,False);
  if (channelS)  XtSetSensitive(channelS,False);
  if (variableS) XtSetSensitive(variableS,False);
  XtSetSensitive(mainEditPDM,False);
  XtSetSensitive(mainPalettesPDM,False);
}

/*-------------------------------------------------------------*/
#define	done(type, value)                                       \
	{							\
	    if (toVal->addr != NULL) {				\
		if (toVal->size < sizeof(type)) {		\
		    toVal->size = sizeof(type);			\
		    return False;				\
		}						\
		*(type*)(toVal->addr) = (type)(value);          \
	    }							\
	    else {						\
		static type static_val;				\
		static_val = (value);				\
		toVal->addr = (XPointer)&static_val;		\
	    }							\
	    toVal->size = sizeof(type);				\
	    return True;					\
	}                                                       \
/*-------------------------------------------------------------*/


static Boolean Dm2kCvtStringToBitmap(Display      * dpy,
				     XrmValuePtr    args,
				     Cardinal     * num_args,
				     XrmValuePtr    fromVal,
				     XrmValuePtr    toVal,
				     XtPointer    * data)
{
  static Pixmap  pixmap;		/*** static for cvt magic ***/
  Window         drawable = XDefaultRootWindow(dpy);
  char*          system_env;
  /* T. Straumann: protect against null ptr */
  char         * name     = (fromVal->addr ? (char *)fromVal->addr : "");
  static char    sufix[]  = ".xbm";
  char           sepr     = ':';
  char           file_name[256];
  unsigned int   w, h;
  int            x_hot,y_hot;

  if (*num_args != 0)
    XtErrorMsg("wrongParameters","cvtStringToBitmap","XtToolkitError",
	       "String to Pixmap conversion needs no any arguments",
	       (String *)NULL, (Cardinal *)NULL);
  
  system_env = (char *) getenv("PIXMAP_PATH");
  
  if (system_env != NULL ) {
    register int    i;
    register char * ch;
    
    i = 0;
    for(ch = system_env;  *ch != '\0' ;ch++) 
      {
	if ( *ch != sepr )
	  {
	    file_name[i++] = *ch;
	  } 
	else 
	  {
	    file_name[i++] = '/';
	    file_name[i=0] = '\0'; 
	    strcat(file_name, name);	
	    
	    (void)XReadBitmapFile(dpy, drawable, file_name, &w,&h,
				  &pixmap, &x_hot, &y_hot);
	    
	    if (pixmap != None)
	      break;
	    
	    strcat(file_name, sufix);
	    
	    (void)XReadBitmapFile(dpy, drawable, file_name, &w,&h,
				  &pixmap, &x_hot, &y_hot);
	    
	    if (pixmap != None)
	      break;
	  }
      }
    
    /* last diectory in path
     */
    if ( *ch == '\0' ) 
      {
	file_name[i++] = '/';
	file_name[i]   = '\0';
	strcat(file_name, name);
	
	(void)XReadBitmapFile(dpy, drawable, file_name, &w,&h,
			      &pixmap, &x_hot, &y_hot);
	
	if ( pixmap == None ) 
	  {
	    strcat(file_name, sufix);
	    
	    (void)XReadBitmapFile(dpy, drawable, file_name, &w,&h,
				  &pixmap, &x_hot, &y_hot);
	  }
      }
  }

  if (system_env == NULL) 
    {
      XtErrorMsg("wrongParameters","cvtStringToPixmap","XtToolkitError",
		 "PIXMAP_PATH environment is not set",
		 (String *)NULL, (Cardinal *)NULL);
      return False;
    }
  
  if (pixmap != None) 
  {
    done(Pixmap, pixmap)
  } 
  else 
  {
    XtStringConversionWarning (name, "Pixmap");
    return False;
  }
}

/*ARGSUSED*/
static Boolean  Dm2kCvtStringToPixmap(Display      * dpy,
				      XrmValuePtr    args,
				      Cardinal     * num_args,
				      XrmValuePtr    fromVal,
				      XrmValuePtr    toVal,
				      XtPointer    * data)
{
  static Pixmap  pixmap = None;		/*** static for cvt magic ***/
  Window         drawable = XDefaultRootWindow(dpy);
  char*          system_env;
  /* T. Straumann: protect against null ptr */
  char         * name     = (fromVal->addr ? (char *)fromVal->addr : "");
  char	       * sufix    = ".xpm";
  char           sepr     = ':';
  char           file_name[256];


  if (*num_args != 0)
    XtErrorMsg("wrongParameters","cvtStringToPixmap","XtToolkitError",
	       "String to Pixmap conversion needs no any arguments",
	       (String *)NULL, (Cardinal *)NULL);
  
  system_env = (char *) getenv("PIXMAP_PATH");
  
  if (system_env != NULL ) 
  {
    register int    i;
    register char * ch;
    
    i = 0;
    for(ch = system_env;  *ch != '\0' ;ch++) 
      {
	if ( *ch != sepr )
	  {
	    file_name[i++] = *ch;
	  } 
	else
	  {
	    file_name[i++] = '/';
	    file_name[i]   = '\0'; 
	    i = 0;

	    strcat(file_name, name);	
#ifdef USE_XPM	    
	    (void)XpmReadFileToPixmap(dpy, drawable,
				      file_name, &pixmap, NULL, NULL);
#endif
	    if (pixmap != None)
	      break;
	    
	    strcat(file_name, sufix);
#ifdef USE_XPM	    
	    (void)XpmReadFileToPixmap(dpy, drawable,
				      file_name, &pixmap, NULL, NULL);
#endif
	    if (pixmap != None)
	      break;
	  }
      }
    
    /* last diectory in path
     */
    if ( *ch == '\0' ) 
      {
	file_name[i++] = '/';
	file_name[i]   = '\0';
	strcat(file_name, name);
#ifdef USE_XPM
	(void)XpmReadFileToPixmap(dpy, drawable,
				  file_name, &pixmap, NULL, NULL);
#endif
	if (pixmap == None ) 
	  {
	    strcat(file_name, sufix);
#ifdef USE_XPM
	    (void)XpmReadFileToPixmap(dpy, drawable,
				      file_name, &pixmap, NULL, NULL);
#endif
	  }
      }
  } 

  if (system_env == NULL) 
    {
      XtErrorMsg("wrongParameters","cvtStringToPixmap","XtToolkitError",
		 "PIXMAP_PATH environment is not set",
		 (String *)NULL, (Cardinal *)NULL);
      XtStringConversionWarning (name, "Pixmap");
      return False;
    }
  
  if (pixmap != None) 
    {
      done(Pixmap, pixmap)
    } 
  else 
    {
      return Dm2kCvtStringToBitmap(dpy,args, num_args, fromVal, toVal, data);
    }
}


/*ARGSUSED*/
static void Dm2kPixmapDestructor(XtAppContext  app,
				 XrmValuePtr   to,
				 XtPointer     converter_data,
				 XrmValuePtr   args,
				 Cardinal    * num_args)
{
  XFreePixmap(display, *(Pixmap*)to->addr);
}
     
static void RegisterCvtStringToPixmap(void)
{
  
  XtSetTypeConverter("String", "Pixmap", Dm2kCvtStringToPixmap,
                    (XtConvertArgList)NULL, 0,
		     XtCacheAll, Dm2kPixmapDestructor );
  XtSetTypeConverter("String", "Bitmap", Dm2kCvtStringToBitmap,
                    (XtConvertArgList)NULL, 0,
		     XtCacheAll,Dm2kPixmapDestructor );
}

/*ARGSUSED*/
static void QuestionDialogCB(Widget    widget, 
			     XtPointer clientData, 
			     XtPointer callData)
{
  int   button = (int) clientData;
  int * result;
  
  XtVaGetValues(widget, XmNuserData, &result, NULL);

  if (result) 
    *result = button;
  else
    INFORM_INTERNAL_ERROR();
}

int getUserChoiseViaPopupQuestionDialog (Widget parentWidget, ...)
{
#if defined(va_dcl)
  va_dcl
#endif
  va_list     ap;
  int         arg;
  Widget      dialog;
  Widget      widget;
  XmString    messageXmString = NULL;
  XmString    okXmString      = NULL;
  XmString    cancelXmString  = NULL;
  XmString    helpXmString    = NULL;
  XmString    customXmString[DM2K_DIALOG_MAX_CUSTEM_BUTTON];
  int         customButton = 0;
  Arg         args[10];
  int         n;
  int         i;
  char      * title = NULL;  
  int         startNumberForCustomButton = 0;
  int       * result = NULL;
  XEvent      event;


  for (i = 0; i < DM2K_DIALOG_MAX_CUSTEM_BUTTON; i++)
    customXmString[i] = NULL;

  va_start(ap, parentWidget);
  arg = va_arg(ap,int);


  while (arg != 0) 
    {
      switch (arg) 
	{
	case DM2K_DIALOG_OK_BUTTON :
	  if (okXmString)  /* if ``OK'' was defined more than once.. */
	    XmStringFree (okXmString);

	  okXmString = XmStringCreateLocalized (va_arg(ap,char *));

	  startNumberForCustomButton = 1;

	  break;
	  
	case DM2K_DIALOG_CANCEL_BUTTON :
	  if (cancelXmString)  /* if ``CANCEL'' was defined more than once.. */
	    XmStringFree (cancelXmString);

	  cancelXmString = XmStringCreateLocalized (va_arg(ap,char *));

	  break;
	  
	case DM2K_DIALOG_HELP_BUTTON :
	  if (helpXmString)  /* if ``HELP'' was defined more than once.. */
	    XmStringFree (helpXmString);

	  helpXmString = XmStringCreateLocalized (va_arg(ap,char *));

	  break;

	case DM2K_DIALOG_MESSAGE_LABEL :
	  if (messageXmString) /* if ``MESSAGE'' was defined more than once..*/
	    XmStringFree (messageXmString);

	  messageXmString = XmStringCreateLtoR (va_arg(ap,char *),
						XmFONTLIST_DEFAULT_TAG);

	  break;
	  
	case DM2K_DIALOG_CUSTEM_BUTTON :
	  if (customButton < DM2K_DIALOG_MAX_CUSTEM_BUTTON) {
	    customXmString[customButton] = 
	      XmStringCreateLocalized (va_arg(ap,char *));
	  }

	  customButton++;

	  break;

	case DM2K_DIALOG_TITLE:
	  title = va_arg(ap,char *);

	  break;

	default :
	  INFORM_INTERNAL_ERROR();

	  /* free all allocated memory
	   */
	  if (okXmString)      XmStringFree (okXmString);
	  if (cancelXmString)  XmStringFree (cancelXmString);
	  if (helpXmString)    XmStringFree (helpXmString);
	  if (messageXmString) XmStringFree (messageXmString);

	  for (i = 0; i < DM2K_DIALOG_MAX_CUSTEM_BUTTON; i++)
	    {
	      if (customXmString[i])
		XmStringFree(customXmString[i]);
	      else
		break;
	    }

	  return -1;

	  break;
	}

      arg = va_arg(ap,int);
    }

  result = DM2KALLOC(int);

  if (result == NULL) {
    fprintf(stderr,"cannot allocate memory\n");
    return -1;
  }


  /* set resource for Dialog
   */
  n = 0;
  XtSetArg(args[n],XmNautoUnmanage,      True); n++;
  XtSetArg(args[n],XmNdialogStyle,       XmDIALOG_FULL_APPLICATION_MODAL); n++;

  if (okXmString){     
    XtSetArg(args[n],XmNokLabelString,     okXmString); n++;
  }

  if (cancelXmString) {
    XtSetArg(args[n],XmNcancelLabelString, cancelXmString); n++;
  }

  if (helpXmString) {
    XtSetArg(args[n],XmNhelpLabelString,   helpXmString); n++;
  }

  if (messageXmString) {
    XtSetArg(args[n],XmNmessageString,     messageXmString); n++;
  }

  dialog = XmCreateQuestionDialog (parentWidget,
				   "questionDialog",
				   args, n);

  if (title)
    XtVaSetValues (XtParent(dialog), XmNtitle, title, NULL);


  /* if some of common widget was not defined, than unmanage particular widget.
   * if not, than set callback.
   */
  widget=XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON);
  if (okXmString == NULL) 
    {
      if (widget)
	XtUnmanageChild(widget);
    }
  else
    {
      XtVaSetValues(widget, XmNuserData,       result, NULL);
      XtAddCallback(widget, XmNactivateCallback, QuestionDialogCB, 
		    (XtPointer)0);
    }

  widget=XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON);
  if (cancelXmString == NULL) 
    {
      if (widget)
	XtUnmanageChild(widget);
    }
  else
    {
      XtVaSetValues(widget, XmNuserData,       result, NULL);
      XtAddCallback(widget, XmNactivateCallback, QuestionDialogCB, 
		    (XtPointer)(startNumberForCustomButton + customButton));
    }

  widget=XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON);
  if (helpXmString == NULL) 
    {
      if (widget)
	XtUnmanageChild(widget);
    }
  else
    {
      XtVaSetValues(widget, XmNuserData,       result, NULL);
      XtAddCallback(widget, XmNactivateCallback, QuestionDialogCB,
		  (XtPointer)(startNumberForCustomButton + customButton +
			      (cancelXmString ? 1 : 0)));  
    }

  if (messageXmString == NULL) 
    {
      if (NULL !=(widget=XmMessageBoxGetChild(dialog, XmDIALOG_MESSAGE_LABEL)))
	XtUnmanageChild(widget);
    }


  /* create custom buttons
   */
  for (i = 0; i < DM2K_DIALOG_MAX_CUSTEM_BUTTON; i++)
    {
      if (customXmString[i]) 
	{
	  widget = XtVaCreateManagedWidget ("customButton", 
					    xmPushButtonWidgetClass,
					    dialog,
					    XmNlabelString, customXmString[i],
					    XmNuserData,    result,
					    NULL);

	  XtAddCallback(widget, XmNactivateCallback, QuestionDialogCB,
			(XtPointer)(customButton + i));
	}
      else
	break;
    }

  XtManageChild(dialog);

  XtPopup(XtParent(dialog), XtGrabExclusive);

  XmUpdateDisplay(XtParent(dialog));

  /* Process events
   */
  *result = -10;

  while (*result == -10 || XtAppPending(appContext)) {
    XtAppNextEvent(appContext,&event);
    XtDispatchEvent(&event);
  }

  XtDestroyWidget(XtParent(dialog));

  if (result)
    i = *result;
  else
    i = -1;

  DM2KFREE(result);

  return i;
}

static Boolean InitializeWorkProc(XtPointer client_data);

static void openCommandLineDisplays(request_t * request)
{ 
  int    i;
  FILE * filePtr;
  /* start any command-line specified displays
   */
  for (i = 0; i < request->fileCnt; i++) {
    char * fileStr = request->fileList[i];
      
    if (fileStr != NULL) 
      {
	 if (!strncmp(fileStr+STRLEN(fileStr)-STRLEN(DISPLAY_FILE_FACEPLATE_SUFFIX),
		      DISPLAY_FILE_FACEPLATE_SUFFIX, STRLEN(DISPLAY_FILE_FACEPLATE_SUFFIX))) {
	    if ( request->opMode == EXECUTE ) {
	       openFaceplate( fileStr );
	    } else {
	       create_shell5 (mainShell);
/*
	       FaceplateGUI * fGUI = getFaceplateGUI(w);
	       fGUI->fpg = createFaceplateGroup(filStr);
	       setFaceplateSensitivity(fGUI);
	       transferFromDataIntoWidgets(fGUI);
*/
	    }

	 } else {
#if 1
	    filePtr = dmOpenUseableFile(fileStr);
#else
	    filePtr = fopen(fileStr,"r");
#endif
	    if (filePtr != NULL) {
	       dmDisplayListParse(NULL,filePtr,request->macroString,fileStr,
				  request->displayGeometry,(Boolean)False);
	       fclose(filePtr);
	       enableEditFunctions();
	    } else {
	       dm2kPrintf("\ndm2k: can't open display file: \"%s\"",fileStr);
	    }
	 }
      }
  }
}

NameValueTable defaultNameValueTable[] = {
   { "DISPLAY",		0 },
   { "HOST",		0 },
   { "USER",		0 }   
};
int defaultNameValueTableSize = sizeof(defaultNameValueTable)/sizeof(NameValueTable);

typedef struct vResourcesRec_ {
	String version;
} vResourcesRec;

static vResourcesRec vResources;

static XtResource vXtResources[]={
	{ "version", "Version",
	  XtRString, sizeof(String),
	  XtOffsetOf(vResourcesRec, version),
	  XtRImmediate, (XtPointer) "x.y.z" },
};

static int atExitMsgFlag = 1;
static void atExitMsg() 
{
   if (atExitMsgFlag) {
      fprintf(stderr, "\n    (use -local for private copy or ");
      fprintf(stderr, "-cleanup to ignore existing DM2K)\n");
   }
}


int main(int argc, char *argv[])
{
  int             i, j, n, index;
  Arg             args[5];
  FILE          * filePtr;
  XColor          color;
  XEvent          event;
  char            versionString[128];
  char            rscVersion[60];

  Boolean         dm2kAlreadyRunning;
  Boolean         completeClientMessage;
  char            fullPathName[FULLPATHNAME_SIZE+1];
  char            name[FULLPATHNAME_SIZE+1];
  unsigned char * propertyData;
  int             status, format;
  unsigned long   nitems, left;
  Atom            type;
  char          * ptr;
  XColor          colors[2];
  request_t     * request;
  msgClass_t      msgClass;
  Window          dm2kHostWindow = (Window)0;
  char buf[40];
  char dm2kvs[40];

  char fbr0[60];

  char *pname;

#if (!((EPICS_VERSION == 3) && (EPICS_REVISION < 14))) && defined(HP_UX)
  /* we are being linked with a C++-linker on HP-UX, so we have to call
   *  _main() to initialize global objects
   */
  _main();
#endif

  pname = strrchr(argv[0], '/');

  MAIN_name = "dm2k";
  MAIN_Name = "Dm2k";
  MAIN_NAME = "DM2K";

  if (!pname++) pname = argv[0];
  if (!strncmp( pname, "medm", 4)) {
     MAIN_name = "medm";
     MAIN_Name = "Medm";
     MAIN_NAME = "MEDM";
  }

  sprintf( READ_ONLY_ENV, "%s_READ_ONLY", MAIN_NAME );
  sprintf( DM2K_WM_POSITION_POLICY_ENV, "%s_WM_POSITION_POLICY", MAIN_NAME );
  sprintf( DM2K_COLOR_RULE_ENV, "%s_COLOR_RULE", MAIN_NAME );
  sprintf( DM2K_GRAPHIC_RULE_ENV, "%s_GRAPHIC_RULE", MAIN_NAME );
  sprintf( EPICS_DM2K_DEFAULT_MB3_DISPLAY_ENV, "EPICS_%s_DEFAULT_MB3_DISPLAY", MAIN_NAME );
  sprintf( DM2K_MAIL_CMD_ENV, "%s_MAIL_CMD", MAIN_NAME );
  sprintf( DM2K_HELP_ENV, "%s_HELP", MAIN_NAME );
  sprintf( DM2K_HELP_PATH_ENV, "%s_HELP_PATH", MAIN_NAME );

  globalDm2kReadOnly = False;
  debugFlag = verboseFlag = synchroFlag = False;
  positionPolicyFlag = False;   /* default : WM positionning policy not defined by user */
  programName = argv[0];

  sprintf( dm2kvs, "%d.%d.%d%s", DM2K_VERSION, DM2K_REVISION, DM2K_UPDATE_LEVEL, DM2K_VERSION_TAG );

  fallbackResources[0] = fbr0;
  sprintf(fallbackResources[0], "%s.version: %s", MAIN_Name, dm2kvs );

  j=0;
  for( i = 1; i < argc; i++ )
     if (!strcmp(argv[i], "-dump")) {
	while (fallbackResources[j])
	   printf("%s\n", fallbackResources[j++]);
	exit(0);
     }
  

  /*  channel status 
   */
  dm2kWorkProcId         = 0;
  dm2kUpdateRequestCount = 0;
  dm2kCAEventCount       = 0;
  dm2kScreenUpdateCount  = 0;
  dm2kUpdateMissedCount  = 0;
  Dm2kUseNewFileFormat   = True;

  /* initialize channel access here (to get around orphaned windows)
   */
  request = requestCreate(argc,argv);

  if (request->opMode == HELP) {
    version (argv[0], stdout);
    usage (argv[0], stdout);
    exit(0);
  }
  
  if (request->opMode == VERSION) {
    version (argv[0], stdout);
    exit(0);
  }
  
  /* allow for quick startup (using existing DM2K)
   * open display, check for atoms, etc
   */
  if (request->macroString != NULL && request->opMode != EXECUTE) {
    fprintf(stderr,"\ndm2k: %s %s","-macro command line option only valid",
	    "for execute (-x) mode operation");
    free(request->macroString);
    request->macroString = NULL;
  }
  
  if (request->opMode == ERROR) {
    fprintf(stderr,"\nusage: dm2k -x  files... for execution  ");
    fprintf(stderr,"or  dm2k -e  files... for edit");
    fprintf(stderr,"\n       -local  for forced local display/execution...\n");
    exit(2);
  }

  {
     int nv;
     struct utsname u_name;
     uname( &u_name );
     
     for (nv = 0; nv < defaultNameValueTableSize; nv++ ) {

	char *p = getenv(defaultNameValueTable[nv].name);
	if (!strcmp( defaultNameValueTable[nv].name, "DISPLAY" )) {
	   int i = 0, skip = 0, colon = 0;

           if (!p) p="";

	   int useNodename= ! ( p[0] != ':') ;

	   if ( !p ) p = ":0.0";
	   defaultNameValueTable[nv].value = calloc(STRLEN(p) +
						    (useNodename ?
						     STRLEN(u_name.nodename) : 0 ) + 3,
						    sizeof(char));
	   if (useNodename) {
	      while (u_name.nodename[i]) {
		 defaultNameValueTable[nv].value[i] = tolower(u_name.nodename[i]); i++;
	      }
	   }

	   while (*p) {
	      if (*p == '.') skip = 1;
	      if (*p == ':') colon = 1, skip = 0;
	      if (!skip || colon)
		 defaultNameValueTable[nv].value[i++] =
		    tolower(((*p == '.') || (*p == ':'))?'_':*p);
	      p++;
	   }
	   if (!skip) strcat( defaultNameValueTable[nv].value, "_0" );
	} else if (!strcmp( defaultNameValueTable[nv].name, "HOST" )) {
	   defaultNameValueTable[nv].value = calloc(STRLEN(u_name.nodename)+1, sizeof(char));
	   strcpy( defaultNameValueTable[nv].value, u_name.nodename ? u_name.nodename : "" );
	} else {
	   int i = 0;
	   defaultNameValueTable[nv].value = calloc(STRLEN(p)+1, sizeof(char));
	   while (*p) defaultNameValueTable[nv].value[i++] = *p++;
	}
     }
  }
  
  if (request->dm2kMode != LOCAL) {
    /* do remote protocol stuff
     */

    display = XOpenDisplay(request->displayName);
    if (display == NULL) {
      fprintf(stderr,"\ndm2k: could not open Display!\n");
      exit(0);
    }
    screenNum = DefaultScreen(display);
    rootWindow = RootWindow(display,screenNum);

    /*  don't create the atom if it doesn't exist - this tells us if another
     *  instance of DM2K is already running in proper startup mode (-e or -x)
     */
    if (request->fontStyle == FIXED) {
      if (request->opMode == EXECUTE) {
	sprintf(buf, "%s020400_EXEC_FIXED", MAIN_NAME );
        DM2K_EXEC_FIXED = XInternAtom(display,buf,False);
        status = XGetWindowProperty(display,rootWindow,DM2K_EXEC_FIXED,
		0,FULLPATHNAME_SIZE,(Bool)False,AnyPropertyType,&type,
		&format,&nitems,&left,&propertyData);
      } else {
	sprintf(buf, "%s020400_EDIT_FIXED", MAIN_NAME );
        DM2K_EDIT_FIXED = XInternAtom(display,buf,False);
        status = XGetWindowProperty(display,rootWindow,DM2K_EDIT_FIXED,
		0,FULLPATHNAME_SIZE,(Bool)False,AnyPropertyType,&type,
		&format,&nitems,&left,&propertyData);
      }
    }
    else if (request->fontStyle == SCALABLE) {
      if (request->opMode == EXECUTE) {
	sprintf(buf, "%s020400_EDIT_SCALABLE", MAIN_NAME );
        DM2K_EXEC_SCALABLE = XInternAtom(display,buf,False);
        status = XGetWindowProperty(display,rootWindow,DM2K_EXEC_SCALABLE,
				    0,FULLPATHNAME_SIZE,(Bool)False,
				    AnyPropertyType,&type,
				    &format,&nitems,&left,&propertyData);
      } else {
	sprintf(buf, "%s020400_EDIT_SCALABLE", MAIN_NAME );
        DM2K_EDIT_SCALABLE = XInternAtom(display,buf,False);
        status = XGetWindowProperty(display,rootWindow,DM2K_EDIT_SCALABLE,
				    0,FULLPATHNAME_SIZE,(Bool)False,
				    AnyPropertyType,&type,
				    &format,&nitems,&left,&propertyData);
      }
    } 
    
    if (type != None) {
      dm2kHostWindow = *((Window *)propertyData);
      dm2kAlreadyRunning = (request->dm2kMode == CLEANUP) ? False : True;
      XFree(propertyData);
    } else {
      dm2kAlreadyRunning = False;
    }

    /* go get the requested files (*.adl), convert to full path name 
     * if necessary, and change the property to initiate the display 
     * request to the remote DM2K
     */

    if (dm2kAlreadyRunning) {
      char *fileStr;
      int i;
      atexit(atExitMsg);
      if (request->fileCnt > 0) {
        fprintf(stderr,"  - dispatched display request%s to remote %s:\n", 
		(request->fileCnt>1?"s":""), MAIN_name);
        for (i=0; i<request->fileCnt; i++) {
          if ((fileStr = request->fileList[i])) {
	    fprintf(stderr,"\t\"%s\"\n",fileStr);

	    if (request->opMode == EXECUTE) {
	      if (request->fontStyle == FIXED)
	        sendFullPathNameAndMacroAsClientMessages
		  (dm2kHostWindow,fileStr,
		   request->macroString,
		   request->displayGeometry,
		   DM2K_EXEC_FIXED);
	      else if (request->fontStyle == SCALABLE)
	        sendFullPathNameAndMacroAsClientMessages
		  (dm2kHostWindow,fileStr,
		   request->macroString,
		   request->displayGeometry,
		   DM2K_EXEC_SCALABLE);
            } 
	    else if (request->opMode == EDIT) {
	      if (request->fontStyle == FIXED)
	        sendFullPathNameAndMacroAsClientMessages
		  (dm2kHostWindow,fileStr,
		   request->macroString,
		   request->displayGeometry,
		   DM2K_EDIT_FIXED);
	      else if (request->fontStyle == SCALABLE)
	        sendFullPathNameAndMacroAsClientMessages
		  (dm2kHostWindow,fileStr,
	  	  request->macroString,
		   request->displayGeometry,
		   DM2K_EDIT_SCALABLE);
	    }	
	    
            XFlush(display);
          }
	}
      } else {
        fprintf(stderr,"\n  - no display to dispatch, ");
	fprintf(stderr,"and already a remote DM2K running:");
	fprintf(stderr,"\n    (use -local for private copy or ");
	fprintf(stderr,"-cleanup to ignore existing DM2K)\n");
      }

      /********* we leave here if another DM2K can do our work  *************
       */
      XCloseDisplay(display);

      atExitMsgFlag = 0;
      exit(0);
    }  
    
    XCloseDisplay(display);
  } /* end if (!request->dm2kMode) */
  
  /*
   * initialize the Intrinsics..., create main shell
   */

  /* map window manager menu Close function to application close... 
   */
  n = 0;
  XtSetArg(args[n],XmNdeleteResponse,XmDO_NOTHING); n++;
  XtSetArg(args[n],XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_RESIZEH); n++;
  mainShell = XtAppInitialize(&appContext, MAIN_Name, NULL, 0, &argc, argv,
			      fallbackResources, args, n);

  display = XtDisplay(mainShell);
  if (display == NULL) {
    XtWarning("cannot open display");
    exit(-1);
  }

  screenNum = DefaultScreen(display);
  rootWindow = RootWindow(display,screenNum);
  cmap = DefaultColormap(display,screenNum);	/* X default colormap */

  if (request->privateDataCmap) {
    /* cheap/easy way to get colormap - do real PseudoColor cmap alloc later
     *  note this really creates a colormap for default visual with no
     *  entries
     */
    cmap = XCopyColormapAndFree(display,cmap);
  }
  XtVaSetValues(mainShell,XmNcolormap,cmap,NULL);

  XtVaGetApplicationResources(mainShell,&vResources,
			      vXtResources, XtNumber(vXtResources),
			      NULL);

  if (0 && strcmp(vResources.version, dm2kvs)) {
     fprintf(stderr, "Version strings differ:\n  binary is %s, resources are %s\n",
	     dm2kvs, vResources.version);
  }

  /* add necessary Motif resource converters 
   */
  XmRegisterConverters();
  XmRepTypeInstallTearOffModelConverter();
  
  if ( synchroFlag ) {
    XSynchronize(display,TRUE);
    fprintf(stderr,"\nRunning in SYNCHRONOUS mode!!");
  }
  
  WM_DELETE_WINDOW = XmInternAtom(display,"WM_DELETE_WINDOW",False);
  WM_TAKE_FOCUS    = XmInternAtom(display,"WM_TAKE_FOCUS",False);
  COMPOUND_TEXT    = XmInternAtom(display,"COMPOUND_TEXT",False);


  /*
   * for remote display start-up:
   *  register signal handlers to assure property states (unfortunately
   *    SIGKILL, SIGSTOP can't be caught...)
   */
#if defined(__cplusplus) && !defined(__GNUG__)
  signal(SIGQUIT,(SIG_PF)handleSignals);
  signal(SIGINT, (SIG_PF)handleSignals);
  signal(SIGTERM,(SIG_PF)handleSignals);
  signal(SIGSEGV,(SIG_PF)handleSignals);
  signal(SIGBUS,(SIG_PF)handleSignals);
#else
  signal(SIGQUIT,handleSignals);
  signal(SIGINT, handleSignals);
  signal(SIGTERM,handleSignals);
  signal(SIGSEGV,handleSignals);
  signal(SIGBUS, handleSignals);
#endif


  if (request->opMode == EDIT) {
    globalDisplayListTraversalMode = DL_EDIT;
  } 
  else if (request->opMode == EXECUTE) {
    globalDisplayListTraversalMode = DL_EXECUTE;
    if (request->fileCnt > 0) {	/* assume .adl file names follow */
        XtVaSetValues(mainShell, 
		      XmNinitialState, IconicState,
		      NULL);
    }
  } else {
    globalDisplayListTraversalMode = DL_EDIT;
  }

  /* initialize some globals
   */
  globalModifiedFlag             = False;
  mainMW                         = NULL;
  objectS                        = NULL; 
  colorS                         = NULL; 
  resourceS                      = NULL; 
  channelS                       = NULL;
  variableS                      = NULL;
  relatedDisplayS                = NULL; 
  shellCommandS                  = NULL;
  cartesianPlotS                 = NULL; 
  cartesianPlotAxisS             = NULL; 
  stripChartS                    = NULL;

  objectMW                       = NULL;
  colorMW                        = NULL;
  resourceMW                     = NULL,
  channelMW                      = NULL;

  cpAxisForm                     = NULL; 
  executeTimeCartesianPlotWidget = NULL;
  currentDisplayInfo             = NULL;
  pointerInDisplayInfo           = NULL;
  resourceBundleCounter          = 0;
  currentElementType             = DL_Element;

  /* not really unphysical, but being used for unallocable color cells 
   */
  unphysicalPixel = BlackPixel(display,screenNum);

  /* initialize the default colormap 
   */
  if (request->privateDataCmap) {
    /* first add in black and white pixels to match 
     * [Black/White]Pixel(dpy,scr) 
     */
    colors[0].pixel = BlackPixel(display,screenNum);
    colors[1].pixel = WhitePixel(display,screenNum);

    XQueryColors(display,DefaultColormap(display,screenNum),colors,2);

    /* need to allocate 0 pixel first, then 1 pixel, usually Black, White...
     *  note this is slightly risky in case of non Pseudo-Color visuals 
     * I think,
     *  but the preallocated colors of Black=0, White=1 for Psuedo-Color
     *  visuals is common, and only for Direct/TrueColor visuals will
     *  this be way off, but then we won't be using the privateData colormap
     *  since we won't run out of colors in that instance...
     */
    if (colors[0].pixel == 0) XAllocColor(display,cmap,&(colors[0]));
    else                      XAllocColor(display,cmap,&(colors[1]));

    if (colors[1].pixel == 1) XAllocColor(display,cmap,&(colors[1]));
    else                      XAllocColor(display,cmap,&(colors[0]));
  }

  for (i = 0; i < DL_MAX_COLORS; i++) 
  {
    /* scale [0,255] to [0,65535] 
     */
    color.red  =(unsigned short)COLOR_SCALE*(defaultDlColormap.dl_color[i].r);
    color.green=(unsigned short)COLOR_SCALE*(defaultDlColormap.dl_color[i].g);
    color.blue =(unsigned short)COLOR_SCALE*(defaultDlColormap.dl_color[i].b);

    /* allocate a shareable color cell with closest RGB value 
     */
    if (XAllocColor(display,cmap,&color)) {
      defaultColormap[i] = color.pixel;
    }
    else {
      fprintf(stderr,"\nmain: couldn't allocate requested color");
      /* put unphysical pixmap value in there as tag it was invalid 
       */
      defaultColormap[i] = unphysicalPixel;
    }
    
  }

  currentColormap = defaultColormap;
  currentColormapSize = DL_MAX_COLORS;
  
  /* and initialize the global resource bundle 
   */
  initializeGlobalResourceBundle();
  globalResourceBundle.next = NULL;
  globalResourceBundle.prev = NULL;

  /* default action for MB in display is select 
   * (regulated by object palette) 
   */
  currentActionType = SELECT_ACTION;

  /* initialization in background procedure
   */
  XtAppAddWorkProc(appContext,InitializeWorkProc,(XtPointer)request);

  request->productPresentationRun = 1;
  
  /* create and popup the product description shell
   *  (use defaults for fg/bg)
   */
  {
    Pixmap         pixmap = None;
#ifdef USE_XPM
    XpmAttributes xpmAttr;

    xpmAttr.valuemask = XpmColormap;
    xpmAttr.colormap = cmap;
    
    /* create dm2k logo pixmap 
     * if color pixmap is not made, than make black and white
     */
    (void)XpmCreatePixmapFromData (display, 
				     XDefaultRootWindow(display),
				     dm2k_logo_xpm,
				     &pixmap, 
				     NULL, 
				     &xpmAttr);

    if (pixmap == None)
      {
	XpmCreatePixmapFromData (display, 
				 XDefaultRootWindow(display),
				 dm2k_logo_bw_xpm,
				 &pixmap, 
				 NULL, 
				 NULL);
      }
#endif

    sprintf(versionString,"%s Version %s \nfor\n %s\nusing\n%s\n%s\n~\n",
	    MAIN_NAME,
	    dm2kvs,
	    EPICS_VERSION_STRING,
	    XmVERSION_STRING,
	    XRT_DOCSTR);
    
    productDescriptionShell = ProductDescriptionCreatePopupDialogShell
      (mainShell,
       "DM2K",
       pixmap,
       "Motif-based Editor & Display Manager\n~\n",
       versionString,
       DEVELOPED_BY,
       -1, -1);

    ProductDescriptionPopupShell
      (appContext,
       mainShell,
       "DM2K",
       pixmap,
       "Motif-based Editor & Display Manager\n~\n",
       versionString,
       DEVELOPED_BY,
       -1, -1, &request->productPresentationRun);

#ifdef USE_XPM
    if (XpmSuccess == XpmCreatePixmapFromData (display,
					       XDefaultRootWindow(display),
					       dm2k_icon_xpm, 
					       &pixmap,
					       NULL,
					       NULL))
      {
	XtVaSetValues(mainShell, XtNiconPixmap, pixmap, NULL);
      }
#endif
  }


  /* process some type of events for redrawing productDescription window
   */
  while (request->productPresentationRun) 
    {
      XtAppNextEvent(appContext,&event);
      switch (event.type) 
	{
	case ConfigureNotify:
	case Expose:
	  XtDispatchEvent(&event);
	  break;
	default :
	  break;
	}
    }

  /* add translations/actions for drag-and-drop 
   */
  parsedTranslations = XtParseTranslationTable(dragTranslations);
  XtAppAddActions(appContext,dragActions,XtNumber(dragActions));

  /* add translations/actions for object palette information 
   */
  objectPaletteTranstable = XtParseTranslationTable (objectPaletteTranslation);
  XtAppAddActions (appContext, objectPaletteActions, XtNumber (objectPaletteActions));

  /* need this later than shell creation for some reason (?) 
   */
  XmAddWMProtocolCallback(mainShell,WM_DELETE_WINDOW,
			  wmCloseCallback, (XtPointer) OTHER_SHELL);
  
  
  XtAppAddWorkProc(appContext,dm2kInitWorkProc,NULL);

  motifWMRunning = XmIsMotifWMRunning (mainShell);

  /* now go into event loop - formerly XtAppMainLoop(appContext);
   */

#ifdef __TED__
  GetWorkSpaceList(mainMW);
#endif


  openCommandLineDisplays(request);


  while (True) {
    static double t0 = 0.0, t1 = 0.0;

    if ( debugFlag ) {
      t0 = dm2kTime ();

      if ( (t0 - t1) > 1.0 ) 
	printf("\n<==== processing time %f ==== event type %d\n", 
	       (t0 -t1), event.type);

      printf("\r<== waiting for events ...                      "); 
      fflush(stdout);
    }

    XtAppNextEvent(appContext,&event);

    if ( debugFlag ) {
      printf("\r<== New event, type %d          ", event.type); 
      fflush(stdout);
      t1 = dm2kTime ();
    }

    switch (event.type) {
      case ClientMessage:
      if ( (event.xclient.message_type == DM2K_EDIT_FIXED &&
	    request->opMode == EDIT && request->fontStyle == FIXED) ||
	  (event.xclient.message_type == DM2K_EXEC_FIXED &&
	   request->opMode == EXECUTE && request->fontStyle == FIXED) ||
	  (event.xclient.message_type == DM2K_EDIT_SCALABLE &&
	   request->opMode == EDIT && request->fontStyle == SCALABLE) ||
	  (event.xclient.message_type == DM2K_EXEC_SCALABLE &&
	   request->opMode == EXECUTE && request->fontStyle == SCALABLE) ) {
	char geometryString[256];
	
	/* concatenate clientMessage events to get full name from form: 
	 * (xyz) 
	 */
	  completeClientMessage = False;
	  for (i = 0; i < MAX_CHARS_IN_CLIENT_MESSAGE; i++) {
	    switch (event.xclient.data.b[i]) {
	      /* start with filename */
	    case '(':  index = 0;
	      ptr = fullPathName;
	      msgClass = FILENAME_MSG;
	      break;
	      /* keep filling in until ';', then start macro string if any */
	    case ';':  ptr[index++] = '\0';
	      if (msgClass == FILENAME_MSG) {
		msgClass = MACROSTR_MSG;
		ptr = name;
	      } else {
		msgClass = GEOMETRYSTR_MSG;
		ptr = geometryString;
	      }
	      index = 0;
	      break;
	      /* terminate whatever string is being filled in */
	    case ')':  completeClientMessage = True;
	      ptr[index++] = '\0';
	      break;
	    default:   ptr[index++] = event.xclient.data.b[i];
	      break;
	    }
	  }
	
	if (completeClientMessage) {
	   if (!strncmp(fullPathName+STRLEN(fullPathName)-STRLEN(DISPLAY_FILE_FACEPLATE_SUFFIX),
			DISPLAY_FILE_FACEPLATE_SUFFIX, STRLEN(DISPLAY_FILE_FACEPLATE_SUFFIX))) {
	      if ( request->opMode == EXECUTE ) {
		 openFaceplate( fullPathName );
	      } else {
		 create_shell5 (mainShell);
	      }
	    } else {

             char *slash=rindex(fullPathName,'/');
             if (slash==NULL) slash=fullPathName; else slash+=1;
             printf("fullpath=%s\nslash=%s\ngeometry=%s\n", fullPathName, slash, (geometryString?geometryString:"NULL"));
             if (strncmp(slash,"dumpDisplayInfoList",min(19,strlen(slash)))==0) {
               
               dumpDisplayInfoList();
             } else {
	       filePtr = fopen(fullPathName,"r");
	       if (filePtr) {
                 dmDisplayListParse(NULL,filePtr,name,fullPathName,geometryString,
                                    (Boolean)False);
                 if (globalDisplayListTraversalMode == DL_EDIT) {
                   enableEditFunctions();
                 }
                 if (geometryString[0] != '\0')
                   dm2kPrintf("    geometry = %s\n\n",geometryString);
                 if (name[0] != '\0')
                   dm2kPrintf("    macro = %s\n",name);
                 if (fullPathName[0] != '\0')
                   dm2kPrintf("    filename = %s\n",fullPathName);
                 dm2kPrintf("File Dispatch Request :\n");
                 dm2kPostTime();
                 fclose(filePtr);
	       } else {
                 dm2kPrintf("\nDM2K: could not open requested file\n"
                            "\t\"%s\"\n  from remote DM2K request\n",
                            fullPathName);
	       }
             }
	    }
	}
	
      } else
	XtDispatchEvent(&event);
      break;
      
    default:
      if ( debugFlag ) {
	printf("\r<== Process new event, type %d          ", event.type); 
	fflush(stdout);
	t1 = dm2kTime ();
      }
      XtDispatchEvent(&event);
    }
  }
  exit(0);
  return 0;
}


static Boolean InitializeWorkProc(XtPointer client_data)
{
  request_t * request = (request_t *) client_data;
  static int num = 0;

  switch(num) 
  {
  case 0:   setbuf(stderr,NULL); 
            setbuf(stdout,NULL);
            dm2kInit(request->displayFont);
    break;
    
  case 1:    dm2kInitializeImageCache();
    break;
    
  case 2:    createCursors();
    break;
    
  case 3:    initializeRubberbanding();
    break;
    
  case 4:    createMain();
    break;
    
  case 5:    disableEditFunctions();
    break;
    
  case 6:    initDm2kCommon();
    break;
    
  case 7:    (void) initEventHandlers();
    break;
    
  case 8:    initDm2kWidget();
    break;

  case 9:    RegisterCvtStringToPixmap();
    break;
    
  case 10:   readColorRulesFromFile(getenv (DM2K_COLOR_RULE_ENV));
             currentDisplayInfo = NULL;
    break;
   
  case 11:   readGraphicRulesFromFile(getenv (DM2K_GRAPHIC_RULE_ENV));
             currentDisplayInfo = NULL;
    break;
    
  case 12: {
    Window targetWindow;

    /* ...we're the first DM2K around in this mode - proceed with 
     *  full execution but store dummy property first
     */
    targetWindow = XtWindow(mainShell);
      
    if (request->opMode == EXECUTE) 
      {
	if (request->fontStyle == FIXED) 
	  {
	    if (DM2K_EXEC_FIXED != (Atom)NULL)
	      XChangeProperty(display,rootWindow,DM2K_EXEC_FIXED,
			      XA_WINDOW,32,PropModeReplace,
			      (unsigned char *)&targetWindow,1);
	  } 
	else if (request->fontStyle == SCALABLE) 
	  {
	    if (DM2K_EXEC_SCALABLE != (Atom)NULL)
	      XChangeProperty(display,rootWindow,DM2K_EXEC_SCALABLE,
			      XA_WINDOW,32,PropModeReplace,
			      (unsigned char *)&targetWindow,1);
	  }
      } 
    else if (request->opMode == EDIT) 
      {
	if (request->fontStyle == FIXED) {
	  if (DM2K_EDIT_FIXED != (Atom)NULL)
	    XChangeProperty(display,rootWindow,DM2K_EDIT_FIXED,
			    XA_WINDOW,32,PropModeReplace,
			    (unsigned char *)&targetWindow,1);
	} 
	else if (request->fontStyle == SCALABLE) 
	  {
	    if (DM2K_EDIT_SCALABLE != (Atom)NULL)
	      XChangeProperty(display,rootWindow,DM2K_EDIT_SCALABLE,
			      XA_WINDOW,32,PropModeReplace, 
			      (unsigned char *)&targetWindow,1);
	  }
      }
  }
  break;
    
  case 13:
    if (displayInfoListHead && (globalDisplayListTraversalMode == DL_EDIT)) {
      enableEditFunctions();
    }
    break;
    
  case 14:
    /* need this later than shell creation for some reason (?) 
     */
    XmAddWMProtocolCallback(mainShell,WM_DELETE_WINDOW,
			    wmCloseCallback, (XtPointer) OTHER_SHELL);
    
    break;

  default:
    request->productPresentationRun = 0;
    return True;
  }    

  num++;
  return False;
}

