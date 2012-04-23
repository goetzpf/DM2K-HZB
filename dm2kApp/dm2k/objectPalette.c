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
 * .03  09-12-95        vong    conform to c++ syntax
 *
 *****************************************************************************
*/

#include "dm2k.h"

#include <Xm/ToggleBG.h>
#include <Xm/MwmUtil.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <cvtFast.h>
#ifdef __cplusplus
}
#endif

#define N_MAIN_MENU_ELES 2
#define HELP_BTN_POSN 1

#define N_FILE_MENU_ELES 1
#define FILE_BTN_POSN 0
#undef FILE_CLOSE_BTN
#define FILE_CLOSE_BTN 0
#define N_HELP_MENU_ELES 1


/* the object element currently displayed in the resource palette */
static DlElement *resourcePaletteDlElement = NULL;


typedef struct {
  char *         pixmapName;
  Widget         widget;
  XtCallbackProc callback;
  XtPointer      clientData;
} buttons_t;

static void resetGlobalResourceBundleAndResourcePalette();
static void objectToggleCallback(Widget, XtPointer, XtPointer);
static void objectPushCallback(Widget, XtPointer, XtPointer);

buttons_t paletteGraphicsButton[] = {
  {"rectangle25",NULL,objectToggleCallback,(XtPointer) DL_Rectangle},
  {"oval25",NULL,objectToggleCallback,(XtPointer) DL_Oval},
  {"arc25",NULL,objectToggleCallback,(XtPointer) DL_Arc},
  {"text25",NULL,objectToggleCallback,(XtPointer) DL_Text},
  {"polyline25",NULL,objectToggleCallback,(XtPointer) DL_Polyline},
  {"line25",NULL,objectToggleCallback,(XtPointer) DL_Line},
  {"polygon25",NULL,objectToggleCallback,(XtPointer) DL_Polygon},
  {"image25",NULL,objectToggleCallback,(XtPointer) DL_Image},
  {NULL,NULL,NULL,NULL}};
/* not implement
  {"bezierCurve25",NULL,objectToggleCallback,(XtPointer) DL_BezierCurve},
*/
buttons_t paletteMonitorButton[] = {
  {"meter25",NULL,objectToggleCallback,(XtPointer) DL_Meter},
  {"bar25",NULL,objectToggleCallback,(XtPointer) DL_Bar},
  {"stripChart25",NULL,objectToggleCallback,(XtPointer) DL_StripChart},
  {"textUpdate25",NULL,objectToggleCallback,(XtPointer) DL_TextUpdate},
  {"indicator25",NULL,objectToggleCallback,(XtPointer) DL_Indicator},
  {"cartesianPlot25",NULL,objectToggleCallback,(XtPointer) DL_CartesianPlot},
/* not implement
  {"surfacePlot25",NULL,objectToggleCallback,(XtPointer) DL_SurfacePlot},
*/
  {"byte25",NULL,objectToggleCallback,(XtPointer) DL_Byte},
  {"dynSymbol25",NULL,objectToggleCallback,(XtPointer) DL_DynSymbol},
  {NULL,NULL,NULL,NULL}};

buttons_t paletteControllerButton[] = {
  {"choiceButton25",NULL, objectToggleCallback,(XtPointer) DL_ChoiceButton},
  {"textEntry25",NULL, objectToggleCallback,(XtPointer) DL_TextEntry},
  {"messageButton25",NULL, objectToggleCallback,(XtPointer) DL_MessageButton},
  {"menu25",NULL, objectToggleCallback,(XtPointer) DL_Menu},
  {"valuator25",NULL, objectToggleCallback,(XtPointer) DL_Valuator},
  {"relatedDisplay25",NULL, objectToggleCallback,(XtPointer) DL_RelatedDisplay},
  {"shellCommand25",NULL, objectToggleCallback,(XtPointer) DL_ShellCommand},
  {NULL,NULL,NULL,NULL}};
#define relatedDisplayIdx 5
#define shellCommandIdx 6

/* controllers which cannot be used in a dialog dysplay */
int controllerEnb[] = {
  relatedDisplayIdx,
  shellCommandIdx,
  -1
  };


buttons_t paletteMiscButton[] = {
  {"select25",NULL, objectPushCallback,NULL},
  {NULL,NULL,NULL,NULL}};

buttons_t *buttonList[] = {
  paletteGraphicsButton,
  paletteMonitorButton,
  paletteControllerButton,
  NULL};

static Widget lastButton = NULL;

Widget objectFilePDM;
extern Widget importFSD;

static XmString xmstringSelect = NULL;
static XmString xmstringCreate = NULL;

/* last mouse position of the display before popup the menu */
extern XButtonPressedEvent lastEvent;


/*
 * global widget for ObjectPalette's SELECT toggle button (needed for
 *	programmatic toggle/untoggle of SELECT/CREATE modes
 */
Widget objectPaletteSelectToggleButton;

static void updateGlobalResourceBundleObjectAttribute(DlObject *object) 
{
  globalResourceBundle.x = object->x;
  globalResourceBundle.y = object->y;
  globalResourceBundle.width = object->width;
  globalResourceBundle.height= object->height;
}

void updateElementObjectAttribute(ResourceBundle *pRCB, DlObject *object) 
{
  dm2kGetValues (pRCB,
    X_RC,      &object->x,
    Y_RC,      &object->y,
    WIDTH_RC,  &object->width,
    HEIGHT_RC, &object->height,
    -1);
}

static void updateResourcePaletteObjectAttribute() {
  char string[MAX_TOKEN_LENGTH];
  sprintf(string,"%d",globalResourceBundle.x);
  XmTextFieldSetString(resourceEntryElement[X_RC],string);
  sprintf(string,"%d",globalResourceBundle.y);
  XmTextFieldSetString(resourceEntryElement[Y_RC],string);
  sprintf(string,"%d",globalResourceBundle.width);
  XmTextFieldSetString(resourceEntryElement[WIDTH_RC],string);
  sprintf(string,"%d",globalResourceBundle.height);
  XmTextFieldSetString(resourceEntryElement[HEIGHT_RC],string);
}

static void updateGlobalResourceBundleBasicAttribute(DlBasicAttribute *attr) {
  globalResourceBundle.clr = attr->clr;
  globalResourceBundle.style = attr->style;
  globalResourceBundle.fill = attr->fill;
  globalResourceBundle.lineWidth = attr->width;
}

#if 0
static void updateElementBasicAttribute(ResourceBundle *pRCB, DlBasicAttribute *attr) {
  dm2kGetValues (pRCB,
    CLR_RC,    &attr->clr,
    STYLE_RC,  &attr->style,
    FILL_RC,   &attr->fill,
    WIDTH_RC,  &attr->width,
    -1);
}
#endif

static void updateResourcePaletteBasicAttribute() {
  char string[MAX_TOKEN_LENGTH];
  XtVaSetValues(resourceEntryElement[CLR_RC],XmNbackground,
    currentDisplayInfo->colormap[globalResourceBundle.clr],NULL);
  optionMenuSet(resourceEntryElement[STYLE_RC],
    globalResourceBundle.style - FIRST_EDGE_STYLE);
  optionMenuSet(resourceEntryElement[FILL_RC],
    globalResourceBundle.fill - FIRST_FILL_STYLE);
  sprintf(string,"%d",globalResourceBundle.lineWidth);
  XmTextFieldSetString(resourceEntryElement[LINEWIDTH_RC],string);
}

static void updateGlobalResourceBundleDynamicAttribute
(DlDynamicAttribute *dynAttr) 
{
  globalResourceBundle.clrmod    = dynAttr->clr;
  globalResourceBundle.vis       = dynAttr->vis;
  globalResourceBundle.colorRule = dynAttr->colorRule;
  renewString(&globalResourceBundle.chan, dynAttr->chan);
}

#if 0
static void updateElementDynamicAttribute(ResourceBundle *pRCB,
					  DlDynamicAttribute *dynAttr)
{
  dm2kGetValues (pRCB,
		 CLRMOD_RC,      &dynAttr->clr,
		 VIS_RC,         &dynAttr->vis,
		 COLOR_RULE_RC,  &dynAttr->colorRule,
		 CHAN_RC,        &dynAttr->chan,
		 -1);
}
#endif

static void updateResourcePaletteDynamicAttribute() 
{
  ColorRule * colorRule;
  int         i;
  Boolean     trueFalse;

  optionMenuSet(resourceEntryElement[CLRMOD_RC],
    globalResourceBundle.clrmod - FIRST_COLOR_MODE);

  optionMenuSet(resourceEntryElement[VIS_RC],
    globalResourceBundle.vis - FIRST_VISIBILITY_MODE);

  for (i = 0, colorRule = colorRuleHead; 
       i < colorRuleCounts && colorRule; 
       i++, colorRule = colorRule->next)
  {
    if (colorRule == globalResourceBundle.colorRule)
      break;
  }
 
  if (resourceEntryElement[COLOR_RULE_RC])
    optionMenuSet(resourceEntryElement[COLOR_RULE_RC], i);
  else
    printf("%s:line %d: resourceEntryElement[COLOR_RULE_RC] is NULL!\n",
 	   __FILE__,__LINE__);
    
  XmTextFieldSetString(resourceEntryElement[CHAN_RC],
		       CARE_PRINT(globalResourceBundle.chan));

  trueFalse = globalResourceBundle.chan != NULL;

  /**2000/12 G.Lei add "if (resourceEntryRC[CLRMOD_RC])",
   **"if (resourceEntryRC[VIS_RC])" and "if (resourceEntryRC[COLOR_RULE_RC])",
   ** since core dump was found when XtSetSensitive has a NULL widget as 
   ** parameter, on alpha machines
   */
  if (resourceEntryRC[CLRMOD_RC]) 
  XtSetSensitive(resourceEntryRC[CLRMOD_RC], trueFalse);
  else printf("updateResourcePaletteDynamicAttribute: resourceEntryRC[CLRMOD_RC] is NULL\n");
  if (resourceEntryRC[VIS_RC])
  XtSetSensitive(resourceEntryRC[VIS_RC], trueFalse);
  else printf("updateResourcePaletteDynamicAttribute: resourceEntryRC[VIS_RC] is NULL\n");
  if (resourceEntryRC[COLOR_RULE_RC])
  XtSetSensitive(resourceEntryRC[COLOR_RULE_RC], trueFalse);
  else printf("updateResourcePaletteDynamicAttribute: resourceEntryRC[COLOR_RULE_RC] is NULL\n");
}

static void updateGlobalResourceBundleControlAttribute(DlControl *control) 
{
  globalResourceBundle.clr  = control->clr;
  globalResourceBundle.bclr = control->bclr;
  renewString(&globalResourceBundle.chan, control->ctrl);
}

void updateElementControlAttribute(ResourceBundle *pRCB, DlControl *control) 
{
  dm2kGetValues (pRCB,
		 CTRL_RC,   &control->ctrl,
		 CLR_RC,    &control->clr,
		 BCLR_RC,   &control->bclr,
		 -1);
}

static void updateResourcePaletteControlAttribute() 
{
  XmTextFieldSetString(resourceEntryElement[CTRL_RC],CARE_PRINT(globalResourceBundle.chan));
  XtVaSetValues(resourceEntryElement[CLR_RC],XmNbackground,
		currentDisplayInfo->colormap[globalResourceBundle.clr],NULL);
  XtVaSetValues(resourceEntryElement[BCLR_RC],XmNbackground,
		currentDisplayInfo->colormap[globalResourceBundle.bclr],NULL);
}

static void updateGlobalResourceBundleMonitorAttribute(DlMonitor *monitor) 
{
  globalResourceBundle.clr  = monitor->clr;
  globalResourceBundle.bclr = monitor->bclr;
  renewString(&globalResourceBundle.chan, monitor->rdbk);
}

#if 0
static void updateElementMonitorAttribute(ResourceBundle *pRCB, 
					  DlMonitor *monitor)
{
  dm2kGetValues (pRCB,
		 RDBK_RC,   &monitor->rdbk,
		 CLR_RC,    &monitor->clr,
		 BCLR_RC,   &monitor->bclr,
		 -1);
}
#endif

static void updateResourcePaletteMonitorAttribute() 
{
  XmTextFieldSetString(resourceEntryElement[RDBK_RC],
		       CARE_PRINT(globalResourceBundle.chan));

  XtVaSetValues(resourceEntryElement[CLR_RC],XmNbackground,
	currentDisplayInfo->colormap[globalResourceBundle.clr],NULL);

  XtVaSetValues(resourceEntryElement[BCLR_RC],XmNbackground,
	currentDisplayInfo->colormap[globalResourceBundle.bclr],NULL);
}

/* controllers sensitivity attributes */

static void updateGlobalResourceBundleSensitiveAttribute
(DlSensitive *sensitive) 
{
  globalResourceBundle.sensitive_mode = sensitive->mode;
  renewString(&globalResourceBundle.sensitive_chan, sensitive->chan);
}

void updateElementSensitiveAttribute(ResourceBundle *pRCB,
				     DlSensitive *sensitive)
{
  dm2kGetValues (pRCB,
		 SENSITIVE_MODE_RC, &sensitive->mode,
		 SENSITIVE_CHAN_RC, &sensitive->chan,
		 -1);
}

void updateResourcePaletteSensitiveAttribute() 
{
  Boolean truefalse;

  XmTextFieldSetString(resourceEntryElement[SENSITIVE_CHAN_RC],
		       CARE_PRINT(globalResourceBundle.sensitive_chan));

  optionMenuSet(resourceEntryElement[SENSITIVE_MODE_RC],
    globalResourceBundle.sensitive_mode - FIRST_SENSITIVE_MODE);

  truefalse = ( STRLEN(globalResourceBundle.sensitive_chan) > (size_t) 0 );
  XtSetSensitive(resourceEntryRC[SENSITIVE_MODE_RC],truefalse);

  if ( truefalse ) XtManageChild (resourceEntryElement[SENSITIVE_MODE_RC]);
  else  XtUnmanageChild (resourceEntryElement[SENSITIVE_MODE_RC]);
}

/********************************************
 **************** Actions *******************
 ********************************************/

static Widget objectINF = NULL, infoForm = NULL;

void destroyObjectINF (
  Widget w,
  XtPointer clientData,
  XtPointer callbackStruct)
{
  objectINF = NULL;
}

void resetPaletteButton ()
{
  if ( ! lastButton ) return;
  XmToggleButtonSetState (lastButton, False, False);
  lastButton = NULL;
}

void setPaletteButton (DlElement *dlElement)
{
  Widget w;

  w = elementWidgetTable[dlElement->type -MIN_DL_ELEMENT_TYPE];
  if ( w == NULL ) return;

  /* allow only one button pressed at a time */
  if ( lastButton != w ) resetPaletteButton ();
  XmToggleButtonSetState (w, True, False);
  lastButton = w;
}

extern Boolean debugFlag;

void updateLabelObjectINF (DlElement *dlElement, char *strg)
{
  Arg      args[1];
  XmString xmString;
  char     label[256];

  if ( (globalDisplayListTraversalMode != DL_EDIT)
       || (currentActionType != SELECT_ACTION) ) return;

  if ( ! objectINF ) return;
  if ( ! XtIsManaged (objectINF) ) return;

  setPaletteButton (dlElement);
  strcpy (label, elementStringTable[dlElement->type - MIN_DL_ELEMENT_TYPE]);

  if ( debugFlag ) {
    sprintf(&label[STRLEN(label)], " (%x)", (int)dlElement);
  }

  if ( strg ) 
    strcat (label, strg);

  label[sizeof(label)-1] ='\0';

  xmString = XmStringCreateSimple (label);
  XtSetArg (args[0],  XmNlabelString, xmString);
  XtSetValues (objectINF, args ,1);

  XmStringFree(xmString);
}

void displayLeaveWindow (Widget w)
{
  Arg args[1];

  if ( (globalDisplayListTraversalMode != DL_EDIT)
       || (currentActionType != SELECT_ACTION) ) return;

  if ( ! objectINF ) return;
  if ( ! XtIsManaged (objectINF) ) return;

  resetPaletteButton ();
  XtSetArg (args[0], XmNlabelString, xmstringSelect);
  XtSetValues (objectINF, args ,1);
}


DlElement *getDisplayObject (DisplayInfo *displayInfo, Widget w)
/* not yet in use */
{
  DlElement *dlElement;

  if ( displayInfo->dlElementList->count <= 0 ) return (NULL);
  for ( dlElement = displayInfo->dlElementList->head; dlElement ; dlElement = dlElement->next) {
    if ( dlElement->widget == w ) return (dlElement);
  }
  return (NULL);
}

void displayEnterWindow (Widget w)
/* not yet in use */
{
  int idl;
  DisplayInfo *displayInfo;
  DlElement *dlElement;

  if ( globalDisplayListTraversalMode != DL_EDIT ) return;
  if ( ! objectINF ) return;
  if ( ! XtIsManaged (objectINF) ) return;

  /* search for a display object */
  for ( displayInfo = displayInfoListHead; displayInfo; displayInfo = displayInfo->next ) {
    dlElement = getDisplayObject (displayInfo, w);
    if ( dlElement ) break;
  }
  if ( dlElement ) {
    idl = dlElement->type;
    /*
    XtSetArg (args[0],  XmNlabelString, elementXmStringTable[idl -MIN_DL_ELEMENT_TYPE]);
    XtSetValues (objectINF, args ,1);
    */
    return;
  }
}


/*ARGSUSED*/
#ifdef __cplusplus
void objectPaletteEnterWindow (Widget w, XEvent* , String*, Cardinal*)
#else
void objectPaletteEnterWindow (Widget w, XEvent* ev, String* pr, Cardinal* n)
#endif
{
  Arg args[1];
  int i, j, idl;

  if ( globalDisplayListTraversalMode != DL_EDIT ) return;
  if ( ! objectINF ) return;
  if ( ! XtIsManaged (objectINF) ) return;

  /* search for an icon in the object palette */
  for (i = 0; buttonList[i] != NULL; i++) {
    for (j = 0; buttonList[i][j].pixmapName != NULL; j++) {
      if (buttonList[i][j].widget == w) {
      idl = (int) buttonList[i][j].clientData;
      XtSetArg (args[0],  XmNlabelString, elementXmStringTable[idl -MIN_DL_ELEMENT_TYPE]);
      XtSetValues (objectINF, args ,1);
      return;
      }
    }
  }
  if ( paletteMiscButton[0].widget == w ) {
    if ( xmstringSelect == NULL )
      xmstringSelect = XmStringCreateSimple ("Select mode");
    XtSetArg (args[0],  XmNlabelString, xmstringSelect);
    XtSetValues (objectINF, args ,1);
    return;
  }
}

static void setObjectPaletteInfo ()
{
  Arg args[1];

  if ( globalDisplayListTraversalMode != DL_EDIT ) return;
  if ( ! objectINF ) return;
  if ( ! XtIsManaged (objectINF) ) return;

  if ( xmstringSelect == NULL )
    xmstringSelect = XmStringCreateSimple ("Select mode");
  if ( xmstringCreate == NULL )
    xmstringCreate = XmStringCreateSimple ("Create mode");

  XtSetArg (args[0], XmNlabelString, ( currentActionType == SELECT_ACTION )
				   ? xmstringSelect
				   : xmstringCreate);
  XtSetValues (objectINF, args ,1);
}


/*ARGSUSED*/
#ifdef __cplusplus
void objectPaletteLeaveWindow (Widget , XEvent* , String*, Cardinal*)
#else
void objectPaletteLeaveWindow (Widget w, XEvent* ev, String* pr, Cardinal* n)
#endif
{
  setObjectPaletteInfo ();
}


/********************************************
 **************** Callbacks *****************
 ********************************************/


/*
 * object palette's state transition callback - updates resource palette
 */
#ifdef __cplusplus
 void objectMenuCallback(
  Widget,
  XtPointer clientData,
  XtPointer)
#else
 void objectMenuCallback(
  Widget w,
  XtPointer clientData,
  XtPointer callbackStruct)
#endif
{
  DlElementType elementType = (DlElementType) clientData;
  DisplayInfo *di;

  /* move the pointer back the original location */
  di = currentDisplayInfo;
  XWarpPointer(display,None,XtWindow(di->drawingArea),0,0,
    0,0,lastEvent.x, lastEvent.y);

/* unhighlight and unselect any selected elements */
  unhighlightSelectedElements();
  unselectSelectedElements();
  clearResourcePaletteEntries();

/* set global action (from object palette) to CREATE, & global element type */
  currentActionType = CREATE_ACTION;
  mouseButtonHelp ();
  currentElementType = elementType;
  setResourcePaletteEntries();
  di = displayInfoListHead;
  while(di != NULL) {
    XDefineCursor(display,XtWindow(di->drawingArea),crosshairCursor);
    di = di->next;
  }

  if (objectS) {
    int i,j;

    /* allow only one button pressed at a time */
    resetPaletteButton ();
    for (i = 0; buttonList[i] != NULL; i++) {
      for (j = 0; buttonList[i][j].pixmapName != NULL; j++) {
        if (buttonList[i][j].clientData == (XtPointer) elementType) {
	  lastButton = buttonList[i][j].widget;
	  XmToggleButtonSetState (lastButton, True, False);
	  return;
        }
      }
    }
  }

  return;
}

void setActionToSelect() 
{
  DisplayInfo *di;
  /* set global action (from object palette) to SELECT, not CREATE... */
  currentActionType = SELECT_ACTION;
  mouseButtonHelp ();
  setObjectPaletteInfo ();

  /* since currentElementType is not really reset yet (don't know what is
   *	selected yet), clearResourcePaletteEntries() may not popdown
   *	these associated shells  - therefore use brute force */

  if (relatedDisplayS)    XtPopdown(relatedDisplayS);
  if (cartesianPlotS)     XtPopdown(cartesianPlotS);
  if (cartesianPlotAxisS) XtPopdown(cartesianPlotAxisS);
  if (stripChartS)        XtPopdown(stripChartS);

  /* clear out the resource palette to reflect empty/unselected object */
  if (!currentDisplayInfo) {
    clearResourcePaletteEntries();
  } else {
    if (currentDisplayInfo->selectedDlElementList->count == 1) {
      clearResourcePaletteEntries();
    }
  }

  if (objectS) 
    XDefineCursor(display,XtWindow(objectS),rubberbandCursor);
  di = displayInfoListHead;
  while(di) {
    XDefineCursor(display,XtWindow(di->drawingArea),rubberbandCursor);
    di = di->next;
  }
  return;
}

/*
 * object palette's state transition callback - updates resource palette
 */
static void objectPushCallback (
  Widget w,
  XtPointer clientData,
  XtPointer callbackStruct)
{
  if (w == objectPaletteSelectToggleButton) {
    resetPaletteButton ();
    setActionToSelect();
    /* unhighlight and unselect any selected elements */
    unhighlightSelectedElements();
    unselectSelectedElements();
    clearResourcePaletteEntries();
  }
}

static void objectToggleCallback(
  Widget w,
  XtPointer clientData,
  XtPointer callbackStruct)
{
  char strg[128];
  DisplayInfo *di;
  DlElementType elementType = (DlElementType) clientData;
  XmToggleButtonCallbackStruct *call_data = (XmToggleButtonCallbackStruct *) callbackStruct;

  if (w == objectPaletteSelectToggleButton) {
    /* The "Select" push button must be handle by objectPushCallback !" */
    return;
  }

  /* pushing one of these toggles implies create object of this type,
   *      and MB in a display now takes on CREATE semantics
   */

  if (call_data->set == False) {
    if ( lastButton != w ) resetPaletteButton ();
    lastButton = NULL;
    setActionToSelect();
    return;
  }

  /* allow only one button pressed at a time */
  if ( lastButton != w ) resetPaletteButton ();

  lastButton = w;

  /* Object button */
  /* unhighlight and unselect any selected elements */
  unhighlightSelectedElements();
  unselectSelectedElements();
  clearResourcePaletteEntries();

  /* set global action (from object palette) to CREATE, & global element type */
  currentActionType = CREATE_ACTION;
  mouseButtonHelp ();
  currentElementType = elementType;
  setResourcePaletteEntries();
  XDefineCursor(display,XtWindow(objectS),crosshairCursor);
  di = displayInfoListHead;

  while(di) {
    XDefineCursor(display,XtWindow(di->drawingArea),crosshairCursor);
    di = di->next;
  }

  strcpy (strg, "Create ");
  strcat (strg, elementStringTable[currentElementType -MIN_DL_ELEMENT_TYPE]);

  if ( xmstringCreate ) 
    XmStringFree(xmstringCreate);

  xmstringCreate = XmStringCreateSimple (strg);
  setObjectPaletteInfo ();

  return;
}


#ifdef __cplusplus
static void fileMenuSimpleCallback(
  Widget,
  XtPointer clientData,
  XtPointer)
#else
static void fileMenuSimpleCallback(
  Widget w,
  XtPointer clientData,
  XtPointer callbackStruct)
#endif
{
  int buttonNumber = (int) clientData;
  switch(buttonNumber) {
    case FILE_CLOSE_BTN:
      XtPopdown(objectS);
  }
}



/*ARGSUSED*/
static void 
#ifdef __cplusplus
RadioButtonPanelDestroyCallback(Widget w, XtPointer, XtPointer)
#else
RadioButtonPanelDestroyCallback(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  if ( w == lastButton ) lastButton = NULL;
}


Widget createRadioButtonPanel(
  Widget parent, char* name, buttons_t *b, Boolean toggle)
{
  Widget   background, label, rowCol;
  Pixel    fg, bg;
  Arg      args[10];
  int      n;
  int      i, col;
  XmString xmString;
  DlElementType type;

  extern XtTranslations objectPaletteTranstable;

  n = 0;
  XtSetArg(args[n],XmNshadowType,XmSHADOW_ETCHED_IN); n++;
  background = XmCreateFrame(parent,"background", args,n);

  xmString = XmStringCreateSimple(name);

  n = 0;
  XtSetArg(args[n],XmNlabelString,  xmString); n++;
  XtSetArg(args[n],XmNmarginWidth,  0); n++;
  XtSetArg(args[n],XmNmarginHeight, 0); n++;
  XtSetArg(args[n],XmNchildType,    XmFRAME_TITLE_CHILD); n++;

  label = XmCreateLabel(background, name, args, n);
  XmStringFree(xmString);

/* it was before VTR
  background = XtVaCreateWidget("background",
	    xmRowColumnWidgetClass, parent,
	    XmNpacking, XmPACK_TIGHT,
	    NULL);

  label = XtVaCreateWidget(name,
	    xmLabelWidgetClass, background,
	    NULL);
*/
  rowCol = XtVaCreateWidget("rowCol",
			    xmRowColumnWidgetClass, background,
			    XmNorientation,         XmVERTICAL,
			    XmNpacking,             XmPACK_COLUMN,
			    XmNchildType,           XmFRAME_WORKAREA_CHILD,
			    NULL);

  XtVaGetValues(rowCol,
		XmNforeground, &fg,
		XmNbackground, &bg,
		NULL);

  for (i=0; b[i].pixmapName != NULL; i++) 
  {
    Pixmap pixmap;

    pixmap = XmGetPixmap(XtScreen(rowCol),
			 b[i].pixmapName,
			 fg, bg);

    b[i].widget = XtVaCreateManagedWidget(
			b[i].pixmapName,
			(toggle) ? xmToggleButtonWidgetClass : xmPushButtonWidgetClass,
			rowCol,
					  XmNlabelType, XmPIXMAP,
					  XmNmarginTop, 0,
					  XmNmarginBottom, 0,
					  XmNmarginLeft, 0,
					  XmNmarginRight, 0,
					  XmNmarginWidth, 0,
					  XmNmarginHeight, 0,
					  XmNwidth, 29,
					  XmNheight, 29,
					  XmNpushButtonEnabled, True,
					  XmNhighlightThickness, 0,
					  XmNalignment, XmALIGNMENT_CENTER,
					  XmNlabelPixmap, pixmap,
					  XmNindicatorOn, False,
					  XmNrecomputeSize, False,
					  NULL);
    type = (DlElementType) b[i].clientData;
    if ( type >= MIN_DL_ELEMENT_TYPE )
      elementWidgetTable[type -MIN_DL_ELEMENT_TYPE] = b[i].widget;

    XtAddCallback(b[i].widget,
		  (toggle) ? XmNvalueChangedCallback : XmNactivateCallback,
		  b[i].callback,b[i].clientData);
    XtAddCallback(b[i].widget,
		  XmNdestroyCallback, RadioButtonPanelDestroyCallback,
		  b[i].clientData);

    XtOverrideTranslations (b[i].widget, objectPaletteTranstable);
  }

  if (((i>>1)<<1) == i) { /* check for odd/even */
    col = i >> 1;
  } else {
    col = (i >> 1) + 1;
  }
  if (col <= 0) col = 1;
  XtVaSetValues(rowCol, XmNnumColumns, col, NULL);
  XtManageChild(rowCol);
  XtManageChild(background);
  XtManageChild(label);
  return rowCol;
}

/* create an information frame and label */

static Widget createObjectInfo (Widget objectMW)
{
  Widget separator;
  int n;
  Arg args[10];
  XmString xmstring;

  initializeXmStringValueTables ();

  n = 0;
  XtSetArg (args[n], XmNtopOffset, 0); n++;
  XtSetArg (args[n], XmNbottomOffset, 0); n++;
  XtSetArg (args[n], XmNshadowThickness, 0); n++;
  infoForm = XmCreateForm (objectMW,"infoFrame",args,n);

  xmstring = XmStringCreateSimple ("Object type ...");
  n = 0;
  XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
  XtSetArg (args[n], XmNlabelString, xmstring); n++;
  objectINF = XmCreateLabel (infoForm, "infoLabel", args, n);
  XmStringFree (xmstring);

  XtAddCallback(objectINF, XmNdestroyCallback, destroyObjectINF, NULL);

  n = 0;
  XtSetArg (args[n], XmNseparatorType, XmNO_LINE); n++;
  XtSetArg (args[n], XmNshadowThickness, 0); n++;
  XtSetArg (args[n], XmNheight, 1); n++;
  separator = XmCreateSeparator (infoForm, "separator", args, n);

  /* Label - message */
  XtVaSetValues (objectINF,
		 XmNtopAttachment, XmATTACH_FORM,
		 XmNleftAttachment, XmATTACH_FORM,
		 XmNrightAttachment, XmATTACH_FORM,
		 NULL);

  /* Separator*/
  XtVaSetValues (separator,
		 XmNtopAttachment, XmATTACH_WIDGET,
		 XmNtopWidget, resourceElementTypeLabel,
		 XmNbottomAttachment, XmATTACH_FORM,
		 XmNleftAttachment, XmATTACH_FORM,
		 XmNrightAttachment, XmATTACH_FORM,
		 NULL);

  XtManageChild (objectINF);
  XtManageChild (separator);

  return (infoForm);
}




static menuEntry_t fileMenu[] = {
  { "Close",  &xmPushButtonGadgetClass, 'C', NULL, NULL, NULL,
    fileMenuSimpleCallback, (XtPointer) FILE_CLOSE_BTN,  NULL},
  {NULL}
};

static menuEntry_t helpMenu[] = {
  { "On Object Palette...",  &xmPushButtonGadgetClass, 'O', NULL, NULL, NULL,
    NULL, NULL, NULL},
  {NULL}
};

void createObject()
{
  Widget objectRC;
  Widget graphicsRC, monitorRC, controllerRC, miscRC;
  Widget objectMB;
  Widget objectHelpPDM;
  Widget objectLblfr;

/*
 * initialize local static globals
 */
 importFSD = NULL;

/*
 * create a MainWindow in a shell, and then the palette radio box
 */
/* map window manager menu Close function to application close... */

 objectS = XtVaCreatePopupShell("objectS",
	     topLevelShellWidgetClass,mainShell,
	     XtNiconName,"Objects",
	     XtNtitle,"Object Palette",
	     XtNallowShellResize,TRUE,
	     XmNkeyboardFocusPolicy,XmEXPLICIT,
	     XmNdeleteResponse,XmDO_NOTHING,
	     XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_RESIZEH,
	     NULL);

 XmAddWMProtocolCallback(objectS,WM_DELETE_WINDOW,
		(XtCallbackProc)wmCloseCallback,(XtPointer)OTHER_SHELL);

 objectMW = XmCreateMainWindow(objectS,"objectMW",NULL,0);


/*
 * create the menu bar
 */
  objectMB = XmCreateMenuBar(objectMW, "objectMB",NULL,0);

/* color objectMB properly (force so VUE doesn't interfere) */
  colorMenuBar(objectMB,defaultForeground,defaultBackground);

/*
 * create the file pulldown menu pane
 */
  objectFilePDM = buildMenu(objectMB,XmMENU_PULLDOWN,
		    "File", 'F', True, fileMenu);


/*
 * create the help pulldown menu pane
 */
  objectHelpPDM = buildMenu(objectMB,XmMENU_PULLDOWN,
		    "Help", 'H', True, helpMenu);
  XtVaSetValues(objectMB, XmNmenuHelpWidget, objectHelpPDM, NULL);
  /* (MDA) for now, disable this menu */
  XtSetSensitive(objectHelpPDM,False);

/*
 * create work area Row Column
 */
  objectRC = XtVaCreateWidget("objectRC",
			      xmRowColumnWidgetClass, objectMW,
			      XmNorientation,         XmHORIZONTAL,
			      XmNspacing,             12,
			      NULL);

  /* create the information label */
  objectLblfr = createObjectInfo (objectMW);

/* set main window areas */
  XmMainWindowSetAreas(objectMW,objectMB,NULL,NULL,NULL,objectRC);
  XtVaSetValues (objectMW,
	       XmNmessageWindow, objectLblfr,
	       NULL);

  memset (elementWidgetTable, 0, sizeof(elementWidgetTable));
  graphicsRC = createRadioButtonPanel(objectRC,"Graphics",paletteGraphicsButton, True);
  monitorRC = createRadioButtonPanel(objectRC,"Monitors",paletteMonitorButton, True);
  controllerRC = createRadioButtonPanel(objectRC,"Controls",paletteControllerButton, True);
  miscRC = createRadioButtonPanel(objectRC,"Misc",paletteMiscButton, False);

  objectPaletteSelectToggleButton = paletteMiscButton[0].widget;
  lastButton = objectPaletteSelectToggleButton;
  XtVaSetValues(objectPaletteSelectToggleButton,
     XmNset, True,
     NULL);
     
  enableControllerRC (currentDisplayInfo);

  XtManageChild (objectLblfr);
  XtManageChild(objectMB);
  XtManageChild(objectRC);
  XtManageChild(objectMW);
}


/*
 * Routines to enable, check ... controller objects for dialog display
 * -------------------------------------------------------------------
 *   For a check these routines return True if the object type
 *       is a valid type for the display type
 */

void enableControllerRC (DisplayInfo *displayInfo)
{
  Boolean sensitive;
  int i, idx;

  if ( globalDisplayListTraversalMode != DL_EDIT ) return;
  if ( !displayInfo || !objectS ) return;

  sensitive = (displayInfo->displayType == NORMAL_DISPLAY);
  for ( i=0 ; (idx = controllerEnb[i]) >= 0 ; i++ ) {
    if ( ! paletteControllerButton[idx].widget ) continue;
    XtSetSensitive (paletteControllerButton[idx].widget, sensitive);
  }
}


void enableControllerMenu (DisplayInfo *displayInfo)
{
  Boolean sensitive;
  int i, idx;

  if ( globalDisplayListTraversalMode != DL_EDIT ) return;
  if ( !displayInfo ) return;

  sensitive = (displayInfo->displayType == NORMAL_DISPLAY);
  for ( i=0 ; (idx = controllerEnb[i]) >= 0 ; i++ ) {
    enableCtrlObjMenu (sensitive, (DlElementType) paletteControllerButton[idx].clientData);
  }
}


static Boolean checkController (DisplayInfo *displayInfo, DlElementType elementType)
{
  int i, idx;

  if ( ! ELEMENT_IS_CONTROLLER (elementType) ) return (True);
  if ( !displayInfo ) return (False);
  if ( displayInfo->displayType == NORMAL_DISPLAY ) return (True);

  for ( i=0 ; (idx = controllerEnb[i]) >= 0 ; i++ ) {
    if ( (DlElementType) paletteControllerButton[idx].clientData != elementType ) continue;
    return (False);
  }
  return (True);
}


Boolean checkControllerObjectType (
	DisplayInfo *displayInfo,
	DlElementType elementType,
	MessageButtonType buttonType)
{
  if ( ! checkController (displayInfo, elementType) ) return (False);
  if ( displayInfo->displayType == NORMAL_DISPLAY ) return (True);

  if ( elementType == DL_MessageButton ) {
    /* special case : only TOGGLE button is allowed in dialog display */
    if ( buttonType != TOGGLE_BUTTON ) {
      return (False);
    }
  }
  return (True);
}


Boolean checkControllerObject (DisplayInfo *displayInfo, DlElement *dlElement)
{
  return ( checkControllerObjectType (displayInfo, dlElement->type,
		(dlElement->type == DL_MessageButton)
		  ? dlElement->structure.messageButton->buttonType
		  : (MessageButtonType) -1) );
}


/* check for an active controller object */
Boolean checkActiveObject (DlElement *dlElement)
{
  int i, idx;

  if ( ! ELEMENT_IS_CONTROLLER (dlElement->type) ) return (True);

  for ( i=0 ; (idx = controllerEnb[i]) >= 0 ; i++ ) {
    if ( (DlElementType) paletteControllerButton[idx].clientData != dlElement->type ) continue;
    if ( dlElement->widget ) return (True);
    else return (dlElement->actif);
  }
  if ( dlElement->type == DL_MessageButton ) {
    /* special case : PUSH_CLOSE_BUTTON are not allowed in dialog display */
    if ( dlElement->structure.messageButton->buttonType == PUSH_CLOSE_BUTTON ) {
      return (dlElement->widget != NULL);
    }
  }
  return (True);
}


void invalidObjectWarning (DisplayInfo *displayInfo, DlElementType dlElementType)
{
  char warningString[128];
  extern Boolean verboseFlag, debugFlag;

  invalidObjectMessage (warningString, displayInfo, dlElementType);
  if ( verboseFlag || debugFlag ) {
    fprintf(stderr, "\n<== %s ==>\n", warningString);
  }
  dmSetAndPopupWarningDialog (displayInfo, warningString, "Ok", NULL, NULL);
}




/*
 * clear current resourcePalette entries
 */
void clearResourcePaletteEntries()
{
  /* if no resource palette yet, simply return */
  if (!resourceMW) return;
 
  resourcePaletteDlElement = NULL;

  /* popdown any of the associated shells */
  if (relatedDisplayS)    XtPopdown(relatedDisplayS);
  if (shellCommandS)      XtPopdown(shellCommandS);
  if (cartesianPlotS)     XtPopdown(cartesianPlotS);
  if (cartesianPlotAxisS) XtPopdown(cartesianPlotAxisS);
  if (stripChartS)        XtPopdown(stripChartS);

  /* Associated Menu dialog should be destroy to do not occupy memory
   */
  destroyAMPalette();
 
  /*
   * unsetting the current button: unmanage previous resource entries
   * and update current element type label in resourceMW (to Select...)
   * by default
   */
 
  if ( xmstringSelect == NULL )
    xmstringSelect = XmStringCreateSimple ("Select mode");
  XtVaSetValues(resourceElementTypeLabel,XmNlabelString,xmstringSelect,NULL);
 
  /* must normalize back to 0 as index into array for element type */
  if (currentElementType >= MIN_DL_ELEMENT_TYPE &&
      currentElementType <= MAX_DL_ELEMENT_TYPE) {
    int i = currentElementType-MIN_DL_ELEMENT_TYPE;
    XtUnmanageChildren(
      resourcePaletteElements[i].children,
      resourcePaletteElements[i].numChildren);
  }
}


/*
 * set resourcePalette entries based on current type
 */

static void setResourcePaletteEntriesPopup (Boolean popupFlag)
{
  extern Boolean debugFlag;

/* must normalize back to 0 as index into array for element type */
  XmString xmStrg, xmlabel;
  Boolean objectDataOnly;
  DlElementType displayType;
  DlElement *elementPtr;

  /* if no display available, dm2k crash ! */
  if ( ! currentDisplayInfo ) return;

  /* if no resource palette yet, create it */
  if (!resourceMW) createResource();

  /* make sure the resource palette shell is popped-up */
  if ( popupFlag ) XtPopup(resourceS,XtGrabNone);

  /* make these sensitive in case they are managed 
   */
  /**2000/12 G.Lei add "if (resourceEntryRC[CLRMOD_RC])",
   **"if (resourceEntryRC[VIS_RC])" and "if (resourceEntryRC[COLOR_RULE_RC])",
   ** since core dump was found when XtSetSensitive has a NULL widget as 
   ** parameter, on alpha machines
   */
  if (resourceEntryRC[VIS_RC])
  XtSetSensitive(resourceEntryRC[VIS_RC],True);
  else 
    printf("setResourcePaletteEntriesPopup: resourceEntryRC[VIS_RC] NULL\n");
  if (resourceEntryRC[CLRMOD_RC])
  XtSetSensitive(resourceEntryRC[CLRMOD_RC],True);
  else
    printf("setResourcePaletteEntriesPopup: resourceEntryRC[CLRMOD_RC] NULL\n");
  if (resourceEntryRC[COLOR_RULE_RC])
  XtSetSensitive(resourceEntryRC[COLOR_RULE_RC],True);

  /* setting the new button: manage new resource entries 
   */
  XtManageChildren(
      resourcePaletteElements[currentElementType -
				MIN_DL_ELEMENT_TYPE].children,
      resourcePaletteElements[currentElementType -
				MIN_DL_ELEMENT_TYPE].numChildren);

  /* update current element type label in resourceMW 
   * if polyline with 2 points display Line as label, not Polyline 
   */
  displayType = currentElementType;
  if ((currentDisplayInfo->selectedDlElementList->count == 1) &&
      (currentElementType == DL_Polyline) &&
      (FirstDlElement(currentDisplayInfo->selectedDlElementList)->
       structure.element->structure.polyline->nPoints == 2))
    {
      displayType = DL_Line;
    }

  if (IsEmpty(currentDisplayInfo->selectedDlElementList)) {
    elementPtr = NULL;
  }
  else {
    elementPtr = FirstDlElement(currentDisplayInfo->selectedDlElementList);
    elementPtr = elementPtr->structure.element;
  }
  resourcePaletteDlElement = elementPtr;

  xmStrg = elementXmStringTable[displayType-MIN_DL_ELEMENT_TYPE];

  if ( debugFlag ) {
    XmString xmt1;
    char strg[48];
    sprintf(strg, " (element %x)", (int)resourcePaletteDlElement);
    xmt1 = XmStringCreateLtoR (strg, XmFONTLIST_DEFAULT_TAG);
    xmlabel = XmStringConcat (xmStrg, xmt1);
    XmStringFree (xmt1);
  }
  else 
    xmlabel = XmStringCopy (xmStrg);

  XtVaSetValues(resourceElementTypeLabel, XmNlabelString, xmlabel, NULL);
  XmStringFree (xmlabel);

  if ( !elementPtr ) {
    /* restore globalResourceBundle and resource palette
     * x/y/width/height to defaults (as in initializeResourceBundle)
     */
    resetGlobalResourceBundleAndResourcePalette();
  }
  else {
    objectDataOnly = False;
    updateGlobalResourceBundleAndResourcePalette(objectDataOnly);
  }

  /* if not a monitor or controller type object, and no  dynamics channel
   * specified, then insensitize the related entries
   */
  if (globalResourceBundle.chan == NULL) 
  {
    XtSetSensitive(resourceEntryRC[VIS_RC],False);

  /**2000/12 G.Lei add "if (resourceEntryRC[CLRMOD_RC])",
   **"if (resourceEntryRC[VIS_RC])" and "if (resourceEntryRC[COLOR_RULE_RC])",
   ** since core dump was found when XtSetSensitive has a NULL widget as 
   ** parameter, on alpha machines
   */
    if ( (!ELEMENT_HAS_WIDGET(currentElementType)) &&
	(currentElementType != DL_TextUpdate))
      if (resourceEntryRC[CLRMOD_RC])
      XtSetSensitive(resourceEntryRC[CLRMOD_RC],False);
    
    if (globalResourceBundle.clrmod != DISCRETE)
      if (resourceEntryRC[COLOR_RULE_RC])
      XtSetSensitive(resourceEntryRC[COLOR_RULE_RC],False);
  }
  else {
    if (globalResourceBundle.clrmod != DISCRETE)
      XtSetSensitive(resourceEntryRC[COLOR_RULE_RC],False);
    else
      XtSetSensitive(resourceEntryRC[COLOR_RULE_RC],True);
  }

  /* make these sensitive in case they are managed */
  if (globalResourceBundle.erase == NULL)
    XtSetSensitive(resourceEntryRC[ERASE_MODE_RC],False);
  else
    XtSetSensitive(resourceEntryRC[ERASE_MODE_RC],True);
}

void setResourcePaletteEntries()
{
  setResourcePaletteEntriesPopup (True);
}


void updateGlobalResourceBundleFromElement(DlElement *element) 
{
  DlCartesianPlot *p;
  int i;

  if (!element || (element->type != DL_CartesianPlot)) 
    return;

  p = element->structure.cartesianPlot;

  for (i = X_AXIS_ELEMENT; i <= Y2_AXIS_ELEMENT; i++) {
    globalResourceBundle.axis[i].axisStyle = p->axis[i].axisStyle;
    globalResourceBundle.axis[i].rangeStyle = p->axis[i].rangeStyle;
    globalResourceBundle.axis[i].minRange = p->axis[i].minRange;
    globalResourceBundle.axis[i].maxRange = p->axis[i].maxRange;
  }
}

void updateGlobalResourceBundleDisplayPosition ()
{
  DlElement *elementPtr;

/* simply return if not valid to update */
  if (currentDisplayInfo->selectedDlElementList->count != 1) return;

  elementPtr = FirstDlElement(currentDisplayInfo->selectedDlElementList);
  elementPtr = elementPtr->structure.element;

  if ( elementPtr->type != DL_Display ) return;

  positionDisplayRead (currentDisplayInfo);
  elementPtr->structure.display->object.x = currentDisplayInfo->xPosition;
  elementPtr->structure.display->object.y = currentDisplayInfo->yPosition;

  /*
  globalResourceBundle.x = currentDisplayInfo->xPosition;
  globalResourceBundle.y = currentDisplayInfo->yPosition;
  updateResourcePaletteObjectAttribute();
  */
}


void setResourcePaletteEntriesIfVisible (DlElement *dlElement)
{
  if ( !resourceMW ) return;
  if ( !XtIsManaged (resourceMW) ) return;
  if ( !currentDisplayInfo ) return;

  currentElementType = dlElement->type;
  setResourcePaletteEntriesPopup (False);
}

/* ----------------------------------------------------------------- */
#define SET_TEXT(res,field)                                           \
       XmTextFieldSetString(resourceEntryElement[res],                \
                            CARE_PRINT(globalResourceBundle.field))   \
/* ----------------------------------------------------------------- */

void updateGlobalResourceBundleAndResourcePalette(Boolean objectDataOnly) 
{
  DlElement *elementPtr;
  char string[MAX_TOKEN_LENGTH];
  int i, tail;
  DlOverrideFields *povrr = 0;

  /* simply return if not valid to update */
  if (currentDisplayInfo->selectedDlElementList->count != 1) 
    return;

  elementPtr = FirstDlElement(currentDisplayInfo->selectedDlElementList);
  elementPtr = elementPtr->structure.element;

  /* if no resource palette yet, create it 
   */
  if (!resourceMW) {
     currentElementType = elementPtr->type;
     setResourcePaletteEntries();
     return;
  }

  if ( resourcePaletteDlElement != elementPtr ) return;

  switch (elementPtr->type) {
    case DL_Display:
      updateGlobalResourceBundleObjectAttribute(
                &(elementPtr->structure.display->object));
      updateResourcePaletteObjectAttribute();

      if (objectDataOnly) break;

      /* foreground 
       */
      globalResourceBundle.clr = elementPtr->structure.display->clr;
      XtVaSetValues(resourceEntryElement[CLR_RC],XmNbackground,
		currentDisplayInfo->colormap[globalResourceBundle.clr],NULL);

      /* background
       */
      globalResourceBundle.bclr = elementPtr->structure.display->bclr;
      XtVaSetValues(resourceEntryElement[BCLR_RC],XmNbackground,
		currentDisplayInfo->colormap[globalResourceBundle.bclr],NULL);

      /* colormap
       */
      renewString(&globalResourceBundle.cmap,
		  elementPtr->structure.display->cmap);
      XmTextFieldSetString(resourceEntryElement[CMAP_RC],
			   CARE_PRINT(globalResourceBundle.cmap));

      /* display type
       */
      globalResourceBundle.displayType = elementPtr->structure.display->displayType;
      optionMenuSet(resourceEntryElement[DISPLAY_TYPE_RC],
	       globalResourceBundle.displayType - FIRST_DISPLAY_TYPE);
      break;

    case DL_Valuator: {
      DlValuator *p = elementPtr->structure.valuator;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      updateGlobalResourceBundleControlAttribute(&(p->control));
      updateResourcePaletteControlAttribute();
      updateGlobalResourceBundleSensitiveAttribute(&(p->sensitive));
      updateResourcePaletteSensitiveAttribute();

      /* label
       */
      globalResourceBundle.label = p->label;
      optionMenuSet(resourceEntryElement[LABEL_RC],
		    globalResourceBundle.label - FIRST_LABEL_TYPE);

      /* color mode 
       */
      globalResourceBundle.clrmod = p->clrmod;
      optionMenuSet(resourceEntryElement[CLRMOD_RC],
		    globalResourceBundle.clrmod - FIRST_COLOR_MODE);

      /* direction
       */
      globalResourceBundle.direction = p->direction;
      optionMenuSet(resourceEntryElement[DIRECTION_RC],
		globalResourceBundle.direction - FIRST_DIRECTION);

      /* precision
       */
      globalResourceBundle.dPrecision = p->dPrecision;
      sprintf(string,"%f",globalResourceBundle.dPrecision);

      /* strip trailing zeroes */
      tail = STRLEN(string);
      while (string[--tail] == '0') string[tail] = '\0';
      XmTextFieldSetString(resourceEntryElement[PRECISION_RC],string);
      break;
    }
    case DL_ChoiceButton: {
      DlChoiceButton *p = elementPtr->structure.choiceButton;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      updateGlobalResourceBundleControlAttribute(&(p->control));
      updateResourcePaletteControlAttribute();
      updateGlobalResourceBundleSensitiveAttribute(&(p->sensitive));
      updateResourcePaletteSensitiveAttribute();

      /* color mode
       */
      globalResourceBundle.clrmod = p->clrmod;
      optionMenuSet(resourceEntryElement[CLRMOD_RC],
		    globalResourceBundle.clrmod - FIRST_COLOR_MODE);
      /* staking
       */
      globalResourceBundle.stacking = p->stacking;
      optionMenuSet(resourceEntryElement[STACKING_RC],
		    globalResourceBundle.stacking - FIRST_STACKING);
      break;
    }
    case DL_MessageButton: {
      Boolean truefalse;

      DlMessageButton *p = elementPtr->structure.messageButton;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      updateGlobalResourceBundleControlAttribute(&(p->control));
      updateResourcePaletteControlAttribute();
      updateGlobalResourceBundleSensitiveAttribute(&(p->sensitive));
      updateResourcePaletteSensitiveAttribute();

      /* label
       */
      renewString(&globalResourceBundle.messageLabel, p->label);
      SET_TEXT(MSG_LABEL_RC,messageLabel);
      
      /* press message
       */
      renewString(&globalResourceBundle.press_msg, p->press_msg);
      SET_TEXT(PRESS_MSG_RC,press_msg);

      /* release message
       */ 
      renewString(&globalResourceBundle.release_msg, p->release_msg);
      SET_TEXT(RELEASE_MSG_RC,release_msg);

      /* color mode
       */
      globalResourceBundle.clrmod = p->clrmod;
      optionMenuSet(resourceEntryElement[CLRMOD_RC],
		globalResourceBundle.clrmod - FIRST_COLOR_MODE);

      /* button type 
       */
      globalResourceBundle.messageButtonType = p->buttonType;
      optionMenuSet(resourceEntryElement[BUTTON_TYPE_RC],
		    globalResourceBundle.messageButtonType - FIRST_BUTTON_TYPE);

      /* active label
       */
      renewString(&globalResourceBundle.toggleOnLabel, p->alabel);
      SET_TEXT(ACTIVE_LABEL_RC,toggleOnLabel);

      /* active background color todo?
       */
      globalResourceBundle.abclr = p->abclr;

      /* display type dependencies
       */
      truefalse = ( currentDisplayInfo->displayType == NORMAL_DISPLAY );
      optionMenuSensitive (resourceEntryElement[BUTTON_TYPE_RC],
			   PUSH_CLOSE_BUTTON - FIRST_BUTTON_TYPE, truefalse);

      truefalse = ( globalResourceBundle.messageButtonType == TOGGLE_BUTTON );
      XtSetSensitive(resourceEntryRC[ACTIVE_LABEL_RC],truefalse);
      XtSetSensitive(resourceEntryRC[ACTIVE_COLOR_RC],truefalse);

      if ( truefalse ) {
	XtManageChild (resourceEntryElement[ACTIVE_LABEL_RC]);
	XtManageChild (resourceEntryElement[ACTIVE_COLOR_RC]);
      }
      else {
	XtUnmanageChild (resourceEntryElement[ACTIVE_LABEL_RC]);
	XtUnmanageChild (resourceEntryElement[ACTIVE_COLOR_RC]);
      }
      break;
    }
    case DL_TextEntry: {
      DlTextEntry *p = elementPtr->structure.textEntry;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      updateGlobalResourceBundleControlAttribute(&(p->control));
      updateResourcePaletteControlAttribute();
      updateGlobalResourceBundleSensitiveAttribute(&(p->sensitive));
      updateResourcePaletteSensitiveAttribute();

      /* color mode
       */
      globalResourceBundle.clrmod = p->clrmod;
      optionMenuSet(resourceEntryElement[CLRMOD_RC],
		globalResourceBundle.clrmod - FIRST_COLOR_MODE);

      /* format
       */
      globalResourceBundle.format = p->format;
      optionMenuSet(resourceEntryElement[FORMAT_RC],
		    globalResourceBundle.format - FIRST_TEXT_FORMAT);
      break;
    }
    case DL_Menu: {
      DlMenu *p = elementPtr->structure.menu;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      updateGlobalResourceBundleControlAttribute(&(p->control));
      updateResourcePaletteControlAttribute();
      updateGlobalResourceBundleSensitiveAttribute(&(p->sensitive));
      updateResourcePaletteSensitiveAttribute();

      /* color mode
       */
      globalResourceBundle.clrmod = p->clrmod;
      optionMenuSet(resourceEntryElement[CLRMOD_RC],
		    globalResourceBundle.clrmod - FIRST_COLOR_MODE);
      break;
    }
    case DL_Meter: {
      DlMeter *p = elementPtr->structure.meter;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly)  break;

      updateGlobalResourceBundleMonitorAttribute(&(p->monitor));
      updateResourcePaletteMonitorAttribute();

      /* label
       */
      globalResourceBundle.label = p->label;
      optionMenuSet(resourceEntryElement[LABEL_RC],
		    globalResourceBundle.label - FIRST_LABEL_TYPE);

      /* color mode 
       */
      globalResourceBundle.clrmod = p->clrmod;
      optionMenuSet(resourceEntryElement[CLRMOD_RC],
		    globalResourceBundle.clrmod - FIRST_COLOR_MODE);

      break;
    }
    case DL_TextUpdate: {
      DlTextUpdate *p = elementPtr->structure.textUpdate;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      updateGlobalResourceBundleMonitorAttribute(&(p->monitor));
      updateResourcePaletteMonitorAttribute();

      /* color mode
       */
      globalResourceBundle.clrmod = p->clrmod;
      optionMenuSet(resourceEntryElement[CLRMOD_RC],
		    globalResourceBundle.clrmod - FIRST_COLOR_MODE);

      /* alignment
       */
      globalResourceBundle.alignment = p->alignment;
      optionMenuSet(resourceEntryElement[ALIGNMENT_RC],
		    globalResourceBundle.alignment - FIRST_ALIGNMENT);

      /* format
       */
      globalResourceBundle.format = p->format;
      optionMenuSet(resourceEntryElement[FORMAT_RC],
		    globalResourceBundle.format - FIRST_TEXT_FORMAT);
      break;
    }
    case DL_Bar: {
      DlBar *p = elementPtr->structure.bar;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      updateGlobalResourceBundleDynamicAttribute(&(p->dynAttr));
      updateGlobalResourceBundleMonitorAttribute(&(p->monitor));
      updateResourcePaletteMonitorAttribute();

      /* label
       */
      globalResourceBundle.label = p->label;
      optionMenuSet(resourceEntryElement[LABEL_RC],
		    globalResourceBundle.label - FIRST_LABEL_TYPE);

      /* color mode
       */
      globalResourceBundle.clrmod = p->dynAttr.clr;
      optionMenuSet(resourceEntryElement[CLRMOD_RC],
		    globalResourceBundle.clrmod - FIRST_COLOR_MODE);

      /* direction
       */
      globalResourceBundle.direction = p->direction;
      optionMenuSet(resourceEntryElement[DIRECTION_RC],
		    globalResourceBundle.direction - FIRST_DIRECTION);

      /* fill mode
       */
      globalResourceBundle.fillmod = p->fillmod;
      optionMenuSet(resourceEntryElement[FILLMOD_RC],
		    globalResourceBundle.fillmod - FIRST_FILL_MODE);

      /* scale type
       */
      globalResourceBundle.scaleType = p->scaleType;
      optionMenuSet(resourceEntryElement[SCALE_TYPE_RC],
		    p->scaleType - FIRST_SCALE_TYPE);

      /* bar only
       */
      globalResourceBundle.barOnly = p->barOnly;
      optionMenuSet(resourceEntryElement[BAR_ONLY_RC],
		    p->barOnly - FIRST_SHOW_BAR);

      /* show alarm limits
       */
      globalResourceBundle.showAlarmLimits = p->showAlarmLimits;
      optionMenuSet(resourceEntryElement[SHOW_ALARM_LIMIT_RC],
		    p->showAlarmLimits - FIRST_SHOW_ALARM_LIMITS_TYPE);

      /* show scale
       */
      globalResourceBundle.showScale = p->showScale;
      optionMenuSet(resourceEntryElement[SHOW_SCALE_RC],
		    p->showScale - FIRST_SHOW_SCALE_TYPE);

      break;
    }
    case DL_Byte: {
      DlByte *p = elementPtr->structure.byte;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      updateGlobalResourceBundleMonitorAttribute(&(p->monitor));
      updateResourcePaletteMonitorAttribute();

      /* color mode
       */
      globalResourceBundle.clrmod = p->clrmod;
      optionMenuSet(resourceEntryElement[CLRMOD_RC],
		    globalResourceBundle.clrmod - FIRST_COLOR_MODE);

      /* direction
       */
      globalResourceBundle.direction = p->direction;
      optionMenuSet(resourceEntryElement[DIRECTION_RC],
		    globalResourceBundle.direction - FIRST_DIRECTION);
      /*  start bit
       */
      globalResourceBundle.sbit = p->sbit;
      sprintf(string,"%d",globalResourceBundle.sbit);
      XmTextFieldSetString(resourceEntryElement[SBIT_RC],string);

      /* end bit
       */
      globalResourceBundle.ebit = p->ebit;
      sprintf(string,"%d",globalResourceBundle.ebit);
      XmTextFieldSetString(resourceEntryElement[EBIT_RC],string);
      break;
    }
    case DL_Indicator: {
      DlIndicator *p = elementPtr->structure.indicator;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      updateGlobalResourceBundleMonitorAttribute(&(p->monitor));
      updateResourcePaletteMonitorAttribute();

      /* label
       */
      globalResourceBundle.label = p->label;
      optionMenuSet(resourceEntryElement[LABEL_RC],
		    globalResourceBundle.label - FIRST_LABEL_TYPE);
      
      /* color mode
       */
      globalResourceBundle.clrmod = p->clrmod;
      optionMenuSet(resourceEntryElement[CLRMOD_RC],
		    globalResourceBundle.clrmod - FIRST_COLOR_MODE);
      /* direction
       */
      globalResourceBundle.direction = p->direction;
      optionMenuSet(resourceEntryElement[DIRECTION_RC],
		globalResourceBundle.direction - FIRST_DIRECTION);
      break;
    }
    case DL_StripChart: {
      DlStripChart *p = elementPtr->structure.stripChart;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      /* title
       */
      renewString(&globalResourceBundle.title, p->plotcom.title);
      SET_TEXT(TITLE_RC,title);

      /* x label
       */
      renewString(&globalResourceBundle.xlabel, p->plotcom.xlabel);
      SET_TEXT(XLABEL_RC,xlabel);

      /* y label
       */
      renewString(&globalResourceBundle.ylabel, p->plotcom.ylabel);
      SET_TEXT(YLABEL_RC,ylabel);

      /* foreground 
       */
      globalResourceBundle.clr = p->plotcom.clr;
      XtVaSetValues(resourceEntryElement[CLR_RC],XmNbackground,
	       currentDisplayInfo->colormap[globalResourceBundle.clr],NULL);
      /* background
       */
      globalResourceBundle.bclr = p->plotcom.bclr;
      XtVaSetValues(resourceEntryElement[BCLR_RC],XmNbackground,
	       currentDisplayInfo->colormap[globalResourceBundle.bclr],NULL);

      /* period
       */
      globalResourceBundle.period = p->period;
      cvtDoubleToString(globalResourceBundle.period,string,0);
      XmTextFieldSetString(resourceEntryElement[PERIOD_RC],string);

      /* units
       */
      globalResourceBundle.units = p->units;
      optionMenuSet(resourceEntryElement[UNITS_RC],
		    globalResourceBundle.units - FIRST_TIME_UNIT);

      for (i = 0; i < MAX_PENS; i++){
        renewString(&globalResourceBundle.scData[i].chan,p->pen[i].chan);  
        renewString(&globalResourceBundle.scData[i].utilChan,p->pen[i].utilChan);  
	globalResourceBundle.scData[i].clr = p->pen[i].clr;
      }
      break;
    }
    case DL_CartesianPlot: {
      DlCartesianPlot *p = elementPtr->structure.cartesianPlot;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      /* title
       */
      renewString(&globalResourceBundle.title, p->plotcom.title);
      SET_TEXT(TITLE_RC,title);

      /* x label
       */
      renewString(&globalResourceBundle.xlabel, p->plotcom.xlabel);
      SET_TEXT(XLABEL_RC,xlabel);

      /* y label
       */
      renewString(&globalResourceBundle.ylabel, p->plotcom.ylabel);
      SET_TEXT(YLABEL_RC,ylabel);

      /* foreground
       */
      globalResourceBundle.clr = p->plotcom.clr;
      XtVaSetValues(resourceEntryElement[CLR_RC],XmNbackground,
		currentDisplayInfo->colormap[globalResourceBundle.clr],NULL);

      /* background
       */
      globalResourceBundle.bclr = p->plotcom.bclr;
      XtVaSetValues(resourceEntryElement[BCLR_RC],XmNbackground,
	  currentDisplayInfo->colormap[globalResourceBundle.bclr],NULL);

      /* count
       */
      globalResourceBundle.count = p->count;
      sprintf(string,"%d",globalResourceBundle.count);
      XmTextFieldSetString(resourceEntryElement[COUNT_RC],string);

      /* style
       */
      globalResourceBundle.cStyle = p->style;
      optionMenuSet(resourceEntryElement[CSTYLE_RC],
		globalResourceBundle.cStyle - FIRST_CARTESIAN_PLOT_STYLE);

      /* erase oldest
       */
      globalResourceBundle.erase_oldest = p->erase_oldest;
      optionMenuSet(resourceEntryElement[ERASE_OLDEST_RC],
		globalResourceBundle.erase_oldest - FIRST_ERASE_OLDEST);

      for (i = 0; i < MAX_TRACES; i++){
        renewString(&globalResourceBundle.cpData[i].xdata, p->trace[i].xdata);  
	renewString(&globalResourceBundle.cpData[i].ydata, p->trace[i].ydata);  
	globalResourceBundle.cpData[i].data_clr = p->trace[i].data_clr;
      }
      for (i = X_AXIS_ELEMENT; i <= Y2_AXIS_ELEMENT; i++) {
        globalResourceBundle.axis[i] = p->axis[i];
      }

      /* trigger
       */
      renewString(&globalResourceBundle.trigger, p->trigger);
      SET_TEXT(TRIGGER_RC,trigger);

      /* erase
       */
      renewString(&globalResourceBundle.erase, p->erase);
      SET_TEXT(ERASE_RC,erase);

      /* erase mode
       */
      globalResourceBundle.eraseMode = p->eraseMode;
      optionMenuSet(resourceEntryElement[ERASE_MODE_RC],
		globalResourceBundle.eraseMode - FIRST_ERASE_MODE);
      break;
    }
    case DL_Rectangle: {
      DlRectangle *p = elementPtr->structure.rectangle;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      updateGlobalResourceBundleBasicAttribute(&(p->attr));
      updateResourcePaletteBasicAttribute();
      updateGlobalResourceBundleDynamicAttribute(&(p->dynAttr));
      updateResourcePaletteDynamicAttribute();
      break;
    }
    case DL_Oval: {
      DlOval *p = elementPtr->structure.oval;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      updateGlobalResourceBundleBasicAttribute(&(p->attr));
      updateResourcePaletteBasicAttribute();
      updateGlobalResourceBundleDynamicAttribute(&(p->dynAttr));
      updateResourcePaletteDynamicAttribute();
      break;
    }
    case DL_Arc: {
      DlArc *p = elementPtr->structure.arc;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      updateGlobalResourceBundleBasicAttribute(&(p->attr));
      updateResourcePaletteBasicAttribute();
      updateGlobalResourceBundleDynamicAttribute(&(p->dynAttr));
      updateResourcePaletteDynamicAttribute();

      /* want user to see degrees, but internally use
       * degrees*64 as Xlib requires
       */
      globalResourceBundle.begin = p->begin;
      XmScaleSetValue(resourceEntryElement[BEGIN_RC],
			globalResourceBundle.begin/64);
      globalResourceBundle.path = p->path;
      XmScaleSetValue(resourceEntryElement[PATH_RC],
			globalResourceBundle.path/64);
      break;
    }
    case DL_Text: {
      DlText *p = elementPtr->structure.text;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();

      if (objectDataOnly) break;

      updateGlobalResourceBundleBasicAttribute(&(p->attr));
      updateResourcePaletteBasicAttribute();
      updateGlobalResourceBundleDynamicAttribute(&(p->dynAttr));
      updateResourcePaletteDynamicAttribute();

      /* text
       */
      renewString(&globalResourceBundle.textix, p->textix);
      SET_TEXT(TEXTIX_RC,textix);

      /* alignment
       */
      globalResourceBundle.alignment = p->alignment;
      optionMenuSet(resourceEntryElement[ALIGNMENT_RC],
		globalResourceBundle.alignment - FIRST_ALIGNMENT);

      break;
    }
    case DL_RelatedDisplay: {
      DlRelatedDisplay *p = elementPtr->structure.relatedDisplay;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      /* foreground
       */
      globalResourceBundle.clr = p->clr;
      XtVaSetValues(resourceEntryElement[CLR_RC],XmNbackground,
		    currentDisplayInfo->colormap[globalResourceBundle.clr],
		    NULL);

      /* background
       */
      globalResourceBundle.bclr = p->bclr;
      XtVaSetValues(resourceEntryElement[BCLR_RC],XmNbackground,
		    currentDisplayInfo->colormap[globalResourceBundle.bclr],
		    NULL);

      /* label
       */
      renewString(&globalResourceBundle.rdLabel,p->label);
      SET_TEXT(RD_LABEL_RC,rdLabel);

      /* visual
       */
      globalResourceBundle.rdVisual = p->visual;
      optionMenuSet(resourceEntryElement[RD_VISUAL_RC],
		    globalResourceBundle.rdVisual - FIRST_RD_VISUAL);

      for (i = 0; i < MAX_RELATED_DISPLAYS; i++){
        renewString(&globalResourceBundle.rdData[i].label, p->display[i].label);  
        renewString(&globalResourceBundle.rdData[i].name, p->display[i].name);  
        renewString(&globalResourceBundle.rdData[i].args, p->display[i].args);  
        globalResourceBundle.rdData[i].mode = p->display[i].mode;
      }

      /* update the related display dialog (matrix of values) if appr. 
       */
      updateRelatedDisplayDataDialog();
      break;
    }
    case DL_ShellCommand: {
      DlShellCommand *p = elementPtr->structure.shellCommand;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      /* foreground
       */
      globalResourceBundle.clr = p->clr;
      XtVaSetValues(resourceEntryElement[CLR_RC],XmNbackground,
		currentDisplayInfo->colormap[globalResourceBundle.clr],NULL);

      /* background
       */
      globalResourceBundle.bclr = p->bclr;
      XtVaSetValues(resourceEntryElement[BCLR_RC],XmNbackground,
		    currentDisplayInfo->colormap[globalResourceBundle.bclr],NULL);

      for (i = 0; i < MAX_SHELL_COMMANDS; i++){
        renewString(&globalResourceBundle.cmdData[i].label, p->command[i].label);  
	renewString(&globalResourceBundle.cmdData[i].command, p->command[i].command);
	renewString(&globalResourceBundle.cmdData[i].args, p->command[i].args);  
      }

      /* update the shell command dialog (matrix of values) if appr. */
      updateShellCommandDataDialog();
      break;
    }
    case DL_Image: {
      DlImage *p = elementPtr->structure.image;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      /* image type
       */
      globalResourceBundle.imageType = p->imageType;
      optionMenuSet(resourceEntryElement[IMAGETYPE_RC],
		globalResourceBundle.imageType - FIRST_IMAGE_TYPE);

      /* image name
       */
      renewString(&globalResourceBundle.imageName, p->imageName);
      SET_TEXT(IMAGENAME_RC,imageName);

      break;
    }
    case DL_Composite: {
      DlComposite *p = elementPtr->structure.composite;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      /* visibility
       */
      globalResourceBundle.vis = p->vis;
      optionMenuSet(resourceEntryElement[VIS_RC],
		globalResourceBundle.vis - FIRST_VISIBILITY_MODE);

      /* channel
       */
      renewString(&globalResourceBundle.chan,p->chan);
      SET_TEXT(CHAN_RC,chan);

      /* composite name
       */
      renewString(&globalResourceBundle.compositeName, p->compositeName);
      SET_TEXT(COMPOSITE_NAME_RC,compositeName);

      /* associated menu
       */
      globalResourceBundle.ami = p->ami;

      break;
    }
    case DL_Polyline: {
      DlPolyline *p = elementPtr->structure.polyline;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      updateGlobalResourceBundleBasicAttribute(&(p->attr));
      updateResourcePaletteBasicAttribute();
      updateGlobalResourceBundleDynamicAttribute(&(p->dynAttr));
      updateResourcePaletteDynamicAttribute();
      break;
    }
    case DL_Polygon: {
      DlPolygon *p = elementPtr->structure.polygon;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      updateGlobalResourceBundleBasicAttribute(&(p->attr));
      updateResourcePaletteBasicAttribute();
      updateGlobalResourceBundleDynamicAttribute(&(p->dynAttr));
      updateResourcePaletteDynamicAttribute();
      break;
    }
    case DL_DynSymbol: {
      DlDynSymbol * p = elementPtr->structure.dynSymbol;
      GraphicRule * graphicRule;
      int           i;
      Boolean       found = False;

      updateGlobalResourceBundleObjectAttribute(&(p->object));
      updateResourcePaletteObjectAttribute();
      if (objectDataOnly) break;

      updateGlobalResourceBundleDynamicAttribute(&(p->dynAttr));
      updateResourcePaletteDynamicAttribute();

      /* channel
       */
      renewString(&globalResourceBundle.chan,p->dynAttr.chan);
      SET_TEXT(RDBK_RC,chan);

      /* fit within
       */
      globalResourceBundle.fit = p->fit;
      optionMenuSet(resourceEntryElement[FIT_RC], 
		    globalResourceBundle.fit-FIRST_FIT_TYPE);

      /* graphic rule
       */
      globalResourceBundle.graphicRule = p->graphicRule;
      for (i = 0, graphicRule = graphicRuleHead; 
	   i < graphicRuleCounts && graphicRule; 
	   i++, graphicRule = graphicRule->next)
	{
	  if (graphicRule == p->graphicRule) {
	    found = True;
	    break;
	  }
	}

      if (found)
	/* +1 becouse of <not set> item is first one */
	optionMenuSet(resourceEntryElement[GRAPHIC_RULE_RC], i+1);
      else
	optionMenuSet(resourceEntryElement[GRAPHIC_RULE_RC], 0);


      /* background
       */
      globalResourceBundle.bclr = p->bclr;
      XtVaSetValues(resourceEntryElement[BCLR_RC],XmNbackground,
		    currentDisplayInfo->colormap[globalResourceBundle.bclr],
		    NULL);

      break;
    }

    default:
      dm2kPrintf("\n unknown element type(%s:%d) %d", 
		 __FILE__,__LINE__,elementPtr->type);
      break;
  }

  switch (elementPtr->type ) {
  case DL_Meter:
  case DL_Indicator:
  case DL_Bar:
  case DL_TextUpdate:
     povrr = &(elementPtr->structure.meter->override);
     break;
  case DL_Valuator:
     povrr = &(elementPtr->structure.valuator->override);
     break;
  default:
     break;
  }

  if (povrr) {
     char string[20];
     globalResourceBundle.dispayLowLimit = povrr->displayLowLimit;
     globalResourceBundle.dispayHighLimit = povrr->displayHighLimit;
     globalResourceBundle.valPrecision = povrr->displayPrecision;

     if (globalResourceBundle.dispayLowLimit != MAXFLOAT)
	sprintf(string,"%g",globalResourceBundle.dispayLowLimit);
     else
	strcpy(string,"");
     XmTextFieldSetString(resourceEntryElement[LOW_LIMIT_RC],string);
     if (globalResourceBundle.dispayHighLimit != MAXFLOAT)
	sprintf(string,"%g",globalResourceBundle.dispayHighLimit);
     else
	strcpy(string,"");
     XmTextFieldSetString(resourceEntryElement[HIGH_LIMIT_RC],string);
     if (globalResourceBundle.valPrecision >= 0)
	sprintf(string,"%d",globalResourceBundle.valPrecision);
     else
	strcpy(string,"");
     XmTextFieldSetString(resourceEntryElement[VAL_PRECISION_RC],string);
  }
}

static void getGeometryFeedbackFromWidget(Widget widget, DlObject *object)
{
  Position  x;
  Position  y;
  Dimension width;
  Dimension height;
  Arg       args[4];
    
  if (widget && object) 
    {
      XtSetArg(args[0],XmNx,      &x);
      XtSetArg(args[1],XmNy,      &y);
      XtSetArg(args[2],XmNwidth,  &width);
      XtSetArg(args[3],XmNheight, &height);
      
      XtGetValues(widget, args, 4);
      
      object->x      = x;
      object->y      = y;
      object->width  = width;
      object->height = height;
    }
}

void updateElementFromGlobalResourceBundle(DlElement *elementPtr)
{
  DlObject * object = NULL;

  /* simply return if not valid to update 
   */
  if (!elementPtr || !currentDisplayInfo) 
    return;

  if (elementPtr->run->getValues) 
    (*elementPtr->run->getValues)(&globalResourceBundle,elementPtr);

  if (elementPtr->widget)
    (*elementPtr->run->execute)(currentDisplayInfo,elementPtr);

  /* let's get back a feedback about a real position and a real size of the 
   * element's widget 
   */
  switch (elementPtr->type) {
    case DL_Display:
      object = &(elementPtr->structure.display->object);
      break;

    case DL_Valuator: 
      object = &(elementPtr->structure.valuator->object);
      break;

    case DL_ChoiceButton: 
      object = &(elementPtr->structure.choiceButton->object);
      break;

    case DL_MessageButton:
      object = &(elementPtr->structure.messageButton->object);
      break;

    case DL_TextEntry: 
      object = &(elementPtr->structure.textEntry->object);
      break;

    case DL_Menu: 
      object = &(elementPtr->structure.menu->object);
      break;

    case DL_Meter: 
      object = &(elementPtr->structure.meter->object);
      break;

    case DL_TextUpdate: 
      object = &(elementPtr->structure.textUpdate->object);
      break;

    case DL_Bar: 
      object = &(elementPtr->structure.bar->object);
      break;

    case DL_Byte: 
      object = &(elementPtr->structure.byte->object);
      break;

    case DL_Indicator: 
      object = &(elementPtr->structure.indicator->object);
      break;

    case DL_StripChart: 
      object = &(elementPtr->structure.stripChart->object);
      break;

    case DL_CartesianPlot: 
      object = &(elementPtr->structure.cartesianPlot->object);
      break;

    case DL_Rectangle: 
      object = &(elementPtr->structure.rectangle->object);
      break;

    case DL_Oval: 
      object = &(elementPtr->structure.oval->object);
      break;

    case DL_Arc: 
      object = &(elementPtr->structure.arc->object);
      break;

    case DL_Text:
      object = &(elementPtr->structure.text->object);
      break;

    case DL_RelatedDisplay: 
      object = &(elementPtr->structure.relatedDisplay->object);
      break;

    case DL_ShellCommand: 
      object = &(elementPtr->structure.shellCommand->object);
      break;

    case DL_Image: 
      object = &(elementPtr->structure.image->object);
      break;

    case DL_Composite: 
      object = &(elementPtr->structure.composite->object);
      break;

    case DL_Polyline: 
      object = &(elementPtr->structure.polyline->object);
      break;

    case DL_Polygon: 
      object = &(elementPtr->structure.polygon->object);
      break;

    case DL_DynSymbol: 
      object = &(elementPtr->structure.dynSymbol->object);
      break;

    default:
      dm2kPrintf( "\n %s:%d: unknown element type %d",
		  __FILE__, __LINE__, elementPtr->type);
      break;
  }

  getGeometryFeedbackFromWidget(elementPtr->widget, object);
  updateGlobalResourceBundleAndResourcePalette(True);
}

/*
 * function to clear/reset the global resource bundle data structure
 *	and to put those resource values into the resourcePalette
 *	elements (for the specified element type)
 */

static void resetGlobalResourceBundleAndResourcePalette()
{
  char string[MAX_TOKEN_LENGTH];


  if (ELEMENT_IS_RENDERABLE(currentElementType) ) {

/* get object data: must have object entry  - use rectangle type (arbitrary) */
   globalResourceBundle.x = 0;
   globalResourceBundle.y = 0;

/*
 * special case for text -
 *   since can type to input, want to inherit width/height
 */
   if (currentElementType != DL_Text) {
     globalResourceBundle.width = 10;
     globalResourceBundle.height = 10;
   }

   sprintf(string,"%d",globalResourceBundle.x);
   XmTextFieldSetString(resourceEntryElement[X_RC],string);
   sprintf(string,"%d",globalResourceBundle.y);
   XmTextFieldSetString(resourceEntryElement[Y_RC],string);
   sprintf(string,"%d",globalResourceBundle.width);
   XmTextFieldSetString(resourceEntryElement[WIDTH_RC],string);
   sprintf(string,"%d",globalResourceBundle.height);
   XmTextFieldSetString(resourceEntryElement[HEIGHT_RC],string);

  }

}
