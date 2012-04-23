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
 *
 *      24-02-97    Fabien  initializeXmStringValueTables no more static
 *
 *****************************************************************************
*/

/****************************************************************************
 * resourcePalette.c - Resource Palette                                     *
 * Mods: MDA - Creation                                                     *
 *       DMW - Tells resource palette which global resources Byte needs     *
 ****************************************************************************/
#include <ctype.h>
#include "dm2k.h"
#include <Xm/MwmUtil.h>
#include "dm2kCartesianPlot.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <cvtFast.h>
#ifdef __cplusplus
}
#endif

#ifndef FLDNAME_SZ
# define FLDNAME_SZ   4
#endif
#ifndef PVNAME_STRINGSZ
# define PVNAME_STRINGSZ   29
#endif


#define N_MAX_MENU_ELES 5
#define N_MAIN_MENU_ELES 2
#define N_FILE_MENU_ELES 1
#define FILE_BTN_POSN 0
#undef FILE_CLOSE_BTN
#define FILE_CLOSE_BTN	 0

#define N_HELP_MENU_ELES 1
#define HELP_BTN_POSN 1

#define CMD_APPLY_BTN	0
#define CMD_CLOSE_BTN	1

#define SC_CHANNEL_COLUMN	0
#define SC_ARCH_CHANNEL_COLUMN  1	
#define SC_COLOR_COLUMN		2	

#define SC_APPLY_BTN	0
#define SC_CLOSE_BTN	1

static Dimension maxLabelWidth = 0;
static Dimension maxLabelHeight = 0;

static XmString xmstringSelect = NULL;

extern void createAmDialog(Widget);

/*********************************************************************
 * STRIP CHART DATA
 *********************************************************************/
static Widget scMatrix = NULL, scForm;
static String scColumnLabels[] = {"Channel", "Util Channel", "Color",};
static int scColumnMaxLengths[] = {MAX_TOKEN_LENGTH-1,MAX_TOKEN_LENGTH-1, 6,};
static short scColumnWidths[] = {29,29,6,};
static unsigned char scColumnLabelAlignments[] = 
{XmALIGNMENT_CENTER, XmALIGNMENT_CENTER, XmALIGNMENT_CENTER};
/* and the scCells array of strings (filled in from globalResourceBundle...) 
 */
static String scRows[MAX_PENS][3];
static String *scCells[MAX_PENS];
static Pixel scColorRows[MAX_PENS][3];
static Pixel *scColorCells[MAX_PENS];


/*********************************************************************
 * SHELL COMMAND DATA
 *********************************************************************/
static Widget cmdMatrix = NULL, cmdForm = NULL;
static String cmdColumnLabels[] = {"Command Label","Command","Arguments",};
static int cmdColumnMaxLengths[] = {MAX_TOKEN_LENGTH-1,MAX_TOKEN_LENGTH-1,
					MAX_TOKEN_LENGTH-1,};
static short cmdColumnWidths[] = {36,36,36,};
static unsigned char cmdColumnLabelAlignments[] = {XmALIGNMENT_CENTER,
					XmALIGNMENT_CENTER,XmALIGNMENT_CENTER,};
/* and the cmdCells array of strings (filled in from globalResourceBundle...) */
static String cmdRows[MAX_SHELL_COMMANDS][3];
static String *cmdCells[MAX_SHELL_COMMANDS];

static void createResourceEntries(Widget entriesSW);
static void initializeResourcePaletteElements();
static void createEntryRC( Widget parent, int rcType);

static void createBundleButtons( Widget messageF) {
/****************************************************************************
 * Create Bundle Buttons: Create the control panel at bottom of resource    *
 *   and bundle editor.                                                     *
 ****************************************************************************/
    Widget separator;
    Arg args[4];
    int n;

    n = 0;
    XtSetArg(args[n],XmNlabelString,xmstringSelect); n++;
    resourceElementTypeLabel = XmCreateLabel(messageF,
      "resourceElementTypeLabel",args,n);

    n = 0;
    XtSetArg(args[n],XmNseparatorType,XmNO_LINE); n++;
    XtSetArg(args[n],XmNshadowThickness,0); n++;
    XtSetArg(args[n],XmNheight,1); n++;
    separator = XmCreateSeparator(messageF,"separator",args,n);

/****** Label - message */
    XtVaSetValues(resourceElementTypeLabel,XmNtopAttachment,XmATTACH_FORM,
      XmNleftAttachment,XmATTACH_FORM,XmNrightAttachment,XmATTACH_FORM, NULL);

/****** Separator*/
    XtVaSetValues(separator,XmNtopAttachment,XmATTACH_WIDGET,
      XmNtopWidget,resourceElementTypeLabel,
      XmNbottomAttachment,XmATTACH_FORM, XmNleftAttachment,XmATTACH_FORM,
      XmNrightAttachment,XmATTACH_FORM, NULL);

    XtManageChild(resourceElementTypeLabel);
    XtManageChild(separator);
}

#ifdef __cplusplus
static void pushButtonActivateCallback(Widget w, XtPointer cd, XtPointer)
#else
static void pushButtonActivateCallback(Widget w, XtPointer cd, XtPointer cbs) 
#endif
{
  int rcType = (int) cd;

  switch (rcType) {
    case ASSOM_RC: 
      createAmDialog(w);
      break;
    case RDDATA_RC:
      relatedDisplayDataDialogPopup(w);
      break;
    case SHELLDATA_RC:
      if (!shellCommandS) {
        shellCommandS = createShellCommandDataDialog(w);
      }
      /* update shell command data from globalResourceBundle */
      updateShellCommandDataDialog();
      XtManageChild(cmdForm);
      XtPopup(shellCommandS,XtGrabNone);
      break;
    case CPDATA_RC:
      if (!cartesianPlotS) {
        cartesianPlotS = createCartesianPlotDataDialog(w);
      }
      /* update cartesian plot data from globalResourceBundle */
      updateCartesianPlotDataDialog();
      XtManageChild(cpForm);
      XtPopup(cartesianPlotS,XtGrabNone);
      break;
    case SCDATA_RC:
      if (!stripChartS) {
        stripChartS = createStripChartDataDialog(w);
      }
      /* update strip chart data from globalResourceBundle */
      updateStripChartDataDialog();
      XtManageChild(scForm);
      XtPopup(stripChartS,XtGrabNone);
      break;
    case CPAXIS_RC:
      if (!cartesianPlotAxisS) {
        cartesianPlotAxisS = createCartesianPlotAxisDialog(w);
      }
      /* update cartesian plot axis data from globalResourceBundle */
      updateCartesianPlotAxisDialog();
      if (currentDisplayInfo->hasBeenEditedButNotSaved == False) 
        dm2kMarkDisplayBeingEdited(currentDisplayInfo);
      XtManageChild(cpAxisForm);
      XtPopup(cartesianPlotAxisS,XtGrabNone);
      break;
    default:
      dm2kPrintf("\npushButtonActivate...: invalid type = %d",rcType);
      break;
  }
}

#ifdef __cplusplus
static void optionMenuSimpleCallback(Widget w, XtPointer cd, XtPointer) {
#else
static void optionMenuSimpleCallback(Widget w, XtPointer cd, XtPointer cbs) {
#endif
  int buttonId = (int) cd;
  int i, rcType;
  DlElement *elementPtr;
  Boolean truefalse;

/****** rcType (which option menu) is stored in userData */
  XtVaGetValues(XtParent(w),XmNuserData,&rcType,NULL);
  switch (rcType) {
    case ALIGNMENT_RC: 
      globalResourceBundle.alignment = (Alignment) (FIRST_ALIGNMENT + buttonId);
      break;
    case FORMAT_RC: 
      globalResourceBundle.format = (TextFormat) (FIRST_TEXT_FORMAT + buttonId);
      break;
    case LABEL_RC: 
      globalResourceBundle.label = (LabelType) (FIRST_LABEL_TYPE + buttonId);
      break;
    case DIRECTION_RC: 
      globalResourceBundle.direction = (Direction) (FIRST_DIRECTION + buttonId);
      break;
    case CLRMOD_RC:
      globalResourceBundle.clrmod = (ColorMode) (FIRST_COLOR_MODE + buttonId);
      if (globalResourceBundle.clrmod == DISCRETE) {
        XtSetSensitive(resourceEntryRC[COLOR_RULE_RC],True);
      } else {
        XtSetSensitive(resourceEntryRC[COLOR_RULE_RC],False);
      }
      break;
    case COLOR_RULE_RC:
      globalResourceBundle.colorRule = colorRuleHead;

      for (i = 0; i < buttonId; i ++)
	globalResourceBundle.colorRule = 
	  globalResourceBundle.colorRule->next;

      break;
    case GRAPHIC_RULE_RC: 

      if (buttonId == 0)
	globalResourceBundle.graphicRule = NULL;
      else {
	globalResourceBundle.graphicRule = graphicRuleHead;

	/* we have <not set> button, so i = 1, rather than 0 */
	for (i = 1; i < buttonId; i ++)
	  globalResourceBundle.graphicRule = 
	    globalResourceBundle.graphicRule->next;
      }
      break;
    case FIT_RC:
      globalResourceBundle.fit = (int) (FIRST_FIT_TYPE + buttonId);
      break;
    case FILLMOD_RC: 
      globalResourceBundle.fillmod = (FillMode) (FIRST_FILL_MODE + buttonId);
      break;
    case STYLE_RC: 
      globalResourceBundle.style = (EdgeStyle) (FIRST_EDGE_STYLE + buttonId);
      break;
    case FILL_RC: 
      globalResourceBundle.fill = (FillStyle) (FIRST_FILL_STYLE + buttonId);
      break;
    case VIS_RC: 
      globalResourceBundle.vis = (VisibilityMode) (FIRST_VISIBILITY_MODE + buttonId);
      break;
    case UNITS_RC: 
      globalResourceBundle.units = (TimeUnits) (FIRST_TIME_UNIT + buttonId);
      break;
    case CSTYLE_RC: 
      globalResourceBundle.cStyle = (CartesianPlotStyle) (FIRST_CARTESIAN_PLOT_STYLE + buttonId);
      break;
    case ERASE_OLDEST_RC:
      globalResourceBundle.erase_oldest = (EraseOldest) (FIRST_ERASE_OLDEST + buttonId);
      break;
    case STACKING_RC: 
      globalResourceBundle.stacking = (Stacking) (FIRST_STACKING + buttonId);
      break;
    case IMAGETYPE_RC: 
      globalResourceBundle.imageType = (ImageType) (FIRST_IMAGE_TYPE + buttonId);
      break;
    case ERASE_MODE_RC:
      globalResourceBundle.eraseMode = (eraseMode_t) (FIRST_ERASE_MODE + buttonId);
      break;
    case RD_VISUAL_RC :
      globalResourceBundle.rdVisual = 
          (relatedDisplayVisual_t) (FIRST_RD_VISUAL + buttonId);
      break;

    /* addition for Message Button type and sensitivity */
    case BUTTON_TYPE_RC:
       globalResourceBundle.messageButtonType = (MessageButtonType) (FIRST_BUTTON_TYPE + buttonId);
       truefalse = (globalResourceBundle.messageButtonType == TOGGLE_BUTTON);
       XtSetSensitive (resourceEntryRC[ACTIVE_LABEL_RC], truefalse);
       XtSetSensitive (resourceEntryRC[ACTIVE_COLOR_RC], truefalse);
       if ( truefalse ) {
       XtManageChild (resourceEntryElement[ACTIVE_LABEL_RC]);
       XtManageChild (resourceEntryElement[ACTIVE_COLOR_RC]);
       } else {
       XtUnmanageChild (resourceEntryElement[ACTIVE_LABEL_RC]);
       XtUnmanageChild (resourceEntryElement[ACTIVE_COLOR_RC]);
       }
       break;

    case SENSITIVE_MODE_RC:
       globalResourceBundle.sensitive_mode = 
	 (SensitivityMode) (FIRST_SENSITIVE_MODE + buttonId);
       break;

    case DISPLAY_TYPE_RC:
       globalResourceBundle.displayType = 
	 (DisplayType) (FIRST_DISPLAY_TYPE + buttonId);
       break;

    case SCALE_TYPE_RC: 
      globalResourceBundle.scaleType = 
	(ScaleType) (FIRST_SCALE_TYPE + buttonId);
      break;

    case BAR_ONLY_RC: 
      globalResourceBundle.barOnly = (ShowBar) (FIRST_SHOW_BAR + buttonId);
      break;

    case SHOW_ALARM_LIMIT_RC: 
      globalResourceBundle.showAlarmLimits = 
      (ShowAlarmLimits) (FIRST_SHOW_ALARM_LIMITS_TYPE + buttonId);
      break;

    case SHOW_SCALE_RC: 
      globalResourceBundle.showScale = 
      (ShowScale) (FIRST_SHOW_SCALE_TYPE + buttonId);
      break;

    default:
      dm2kPrintf("\noptionMenuSimpleCallback: unknown rcType = %d",rcType);
      break;
  }

/****** Update elements (this is overkill, but okay for now)
 *	-- not as efficient as it should be (don't update EVERYTHING if only
 *	   one item changed!) */
  if (currentDisplayInfo) {
    DlElement *dlElement = FirstDlElement(currentDisplayInfo->selectedDlElementList);
    /****** Unhighlight (since objects may move) */
    unhighlightSelectedElements();

    while (dlElement) {
      elementPtr = dlElement->structure.element;
      updateElementFromGlobalResourceBundle(elementPtr);
      dlElement = dlElement->next;
    }

    dmTraverseNonWidgetsInDisplayList(currentDisplayInfo);
    if (currentDisplayInfo->hasBeenEditedButNotSaved == False) 
      dm2kMarkDisplayBeingEdited(currentDisplayInfo);
    /* highlight */
    highlightSelectedElements();

    if ( rcType == DISPLAY_TYPE_RC )
      enableControllerRC (currentDisplayInfo);
  }
}


/** set Cartesian Plot Axis attributes
 * (complex - has to handle both EDIT and EXECUTE time interactions)
 */
#ifdef __cplusplus
static void cpAxisOptionMenuSimpleCallback(Widget w, XtPointer cd, XtPointer) {
#else
static void cpAxisOptionMenuSimpleCallback(Widget w, XtPointer cd, XtPointer cbs) {
#endif

#ifdef USE_XRT
  int               buttonId = (int) cd;
  int               k, n, rcType, iPrec;
  char              string[24];
  Arg               args[10];
  float             minF, maxF, tickF;
  XtPointer         userData;
  CartesianPlot   * pcp = NULL;
  DlCartesianPlot * dlCartesianPlot = NULL;

  /* Get current cartesian plot 
   */
  if (globalDisplayListTraversalMode == DL_EXECUTE) {
    if (executeTimeCartesianPlotWidget) {
      XtVaGetValues(executeTimeCartesianPlotWidget,
		    XmNuserData, &userData, NULL);
      if ((pcp = (CartesianPlot *) (((WidgetUserData*)userData)->privateData)))
	dlCartesianPlot = (DlCartesianPlot *) 
	  pcp->dlElement->structure.cartesianPlot;
    }
  }
  /* rcType (and therefore which option menu...) is stored in userData 
   */
  XtVaGetValues(XtParent(w),XmNuserData,&rcType,NULL);
  n = 0;

  switch (rcType) 
    {
    case CP_X_AXIS_STYLE: 
    case CP_Y_AXIS_STYLE: 
    case CP_Y2_AXIS_STYLE: {

      CartesianPlotAxisStyle style 
        = (CartesianPlotAxisStyle) (FIRST_CARTESIAN_PLOT_AXIS_STYLE+buttonId);
      globalResourceBundle.axis[rcType - CP_X_AXIS_STYLE].axisStyle = style;
      switch (rcType) 
	{
	case CP_X_AXIS_STYLE :
	  if (style == TIME_AXIS) {
	    XtSetSensitive(axisTimeFormat,True);
	  } else {
	    XtSetArg(args[n],XtNxrtXAxisLogarithmic,
		     (style == LOG10_AXIS) ? True : False); n++;
	    XtSetSensitive(axisTimeFormat,False);
	  }
	  break;
	case CP_Y_AXIS_STYLE :
	  XtSetArg(args[n],XtNxrtYAxisLogarithmic,
		   (style == LOG10_AXIS) ? True : False); n++;
	  break;
	case CP_Y2_AXIS_STYLE :
	  XtSetArg(args[n],XtNxrtY2AxisLogarithmic,
		   (style == LOG10_AXIS) ? True : False); n++;
	  break;
	}
      break;
    }

    case CP_X_RANGE_STYLE :
    case CP_Y_RANGE_STYLE:
    case CP_Y2_RANGE_STYLE:
      globalResourceBundle.axis[rcType-CP_X_RANGE_STYLE].rangeStyle
	= (CartesianPlotRangeStyle) (FIRST_CARTESIAN_PLOT_RANGE_STYLE
				     + buttonId);

      switch(globalResourceBundle.axis[rcType%3].rangeStyle) 
	{
	case USER_SPECIFIED_RANGE :
	  XtSetSensitive(axisRangeMinRC[rcType%3],True);
	  XtSetSensitive(axisRangeMaxRC[rcType%3],True);
	  if (globalDisplayListTraversalMode == DL_EXECUTE) {
	    if (dlCartesianPlot && dlCartesianPlot->axis) { /* get min from element if possible */
	       minF = dlCartesianPlot->axis[rcType%3].minRange;
	    } else {
	       minF = globalResourceBundle.axis[rcType%3].minRange;
	    }
 	    sprintf(string,"%f",minF);
	    XmTextFieldSetString(axisRangeMin[rcType%3],string);
	    if (dlCartesianPlot) /* get max from element if possible */
	      maxF = dlCartesianPlot->axis[rcType%3].maxRange;
	    else
	      maxF = globalResourceBundle.axis[rcType%3].maxRange;
	    
	    sprintf(string,"%f",maxF);
	    XmTextFieldSetString(axisRangeMax[rcType%3],string);
	    tickF = (maxF - minF)/4.0;
	    sprintf(string,"%f",tickF);
	    k = STRLEN(string)-1;

	    while (string[k] == '0')
	      k--; /* strip off trailing zeroes */

	    iPrec = k;

	    while (string[k] != '.' && k >= 0) 
	      k--;

	    iPrec = iPrec - k;

	    switch(rcType%3) 
	      {
	      case X_AXIS_ELEMENT:
		XtSetArg(args[n],XtNxrtXMin,XrtFloatToArgVal(minF)); n++;
		XtSetArg(args[n],XtNxrtXMax,XrtFloatToArgVal(maxF)); n++;
		XtSetArg(args[n],XtNxrtXTick,XrtFloatToArgVal(tickF)); n++;
		XtSetArg(args[n],XtNxrtXNum,XrtFloatToArgVal(tickF)); n++;
		XtSetArg(args[n],XtNxrtXPrecision,iPrec); n++;
		break;
	      case Y1_AXIS_ELEMENT:
		XtSetArg(args[n],XtNxrtYMin,XrtFloatToArgVal(minF)); n++;
		XtSetArg(args[n],XtNxrtYMax,XrtFloatToArgVal(maxF)); n++;
		XtSetArg(args[n],XtNxrtYTick,XrtFloatToArgVal(tickF)); n++;
		XtSetArg(args[n],XtNxrtYNum,XrtFloatToArgVal(tickF)); n++;
		XtSetArg(args[n],XtNxrtYPrecision,iPrec); n++;
		break;
	      case Y2_AXIS_ELEMENT:
		XtSetArg(args[n],XtNxrtY2Min,XrtFloatToArgVal(minF)); n++;
		XtSetArg(args[n],XtNxrtY2Max,XrtFloatToArgVal(maxF)); n++;
		XtSetArg(args[n],XtNxrtY2Tick,XrtFloatToArgVal(tickF)); n++;
		XtSetArg(args[n],XtNxrtY2Num,XrtFloatToArgVal(tickF)); n++;
		XtSetArg(args[n],XtNxrtY2Precision,iPrec); n++;
		break;
	      }
	  }
	  if (pcp)
	    pcp->axisRange[rcType%3].isCurrentlyFromChannel = False;
	  break;
#if 0
	  CHANNEL_X_RANGE : 
	  CHANNEL_Y_RANGE : 
	  CHANNEL_Y2_RANGE : 
#endif
	case CHANNEL_RANGE :
	  XtSetSensitive(axisRangeMinRC[rcType%3],False);
	  XtSetSensitive(axisRangeMaxRC[rcType%3],False);
	  if (pcp) {
	    /* get channel-based range specifiers - NB: these are
	     *   different than the display element version of these
	     *   which are the user-specified values
	     */
	    minF = pcp->axisRange[rcType%3].axisMin;
	    maxF = pcp->axisRange[rcType%3].axisMax;
	  }

	  switch(rcType%3) {
	  case X_AXIS_ELEMENT:
	    XtSetArg(args[n],XtNxrtXMin,XrtFloatToArgVal(minF)); n++;
	    XtSetArg(args[n],XtNxrtXMax,XrtFloatToArgVal(maxF)); n++;
	    XtSetArg(args[n],XtNxrtXTickUseDefault,True); n++;
	    XtSetArg(args[n],XtNxrtXNumUseDefault,True); n++;
	    XtSetArg(args[n],XtNxrtXPrecisionUseDefault,True); n++;
	    break;
	  case Y1_AXIS_ELEMENT:
	    XtSetArg(args[n],XtNxrtYMin,XrtFloatToArgVal(minF)); n++;
	    XtSetArg(args[n],XtNxrtYMax,XrtFloatToArgVal(maxF)); n++;
	    XtSetArg(args[n],XtNxrtYTickUseDefault,True); n++;
	    XtSetArg(args[n],XtNxrtYNumUseDefault,True); n++;
	    XtSetArg(args[n],XtNxrtYPrecisionUseDefault,True); n++;
	    break;
	  case Y2_AXIS_ELEMENT:
	    XtSetArg(args[n],XtNxrtY2Min,XrtFloatToArgVal(minF)); n++;
	    XtSetArg(args[n],XtNxrtY2Max,XrtFloatToArgVal(maxF)); n++;
	    XtSetArg(args[n],XtNxrtY2TickUseDefault,True); n++;
	    XtSetArg(args[n],XtNxrtY2NumUseDefault,True); n++;
	    XtSetArg(args[n],XtNxrtY2PrecisionUseDefault,True); n++;
	    break;
	  }
	  if (pcp) pcp->axisRange[rcType%3].isCurrentlyFromChannel = True;
	  break;
	case AUTO_SCALE_RANGE :
	  XtSetSensitive(axisRangeMinRC[rcType%3],False);
	  XtSetSensitive(axisRangeMaxRC[rcType%3],False);
	  if (globalDisplayListTraversalMode == DL_EXECUTE) {
	    switch(rcType%3) {
	    case X_AXIS_ELEMENT:
	      XtSetArg(args[n],XtNxrtXMinUseDefault,True);n++;
	      XtSetArg(args[n],XtNxrtXMaxUseDefault,True);n++;
	      XtSetArg(args[n],XtNxrtXTickUseDefault,True);n++;
	      XtSetArg(args[n],XtNxrtXNumUseDefault,True);n++;
	      XtSetArg(args[n],XtNxrtXPrecisionUseDefault,True);n++;
	      break;
	    case Y1_AXIS_ELEMENT:
	      XtSetArg(args[n],XtNxrtYMinUseDefault,True);n++;
	      XtSetArg(args[n],XtNxrtYMaxUseDefault,True);n++;
	      XtSetArg(args[n],XtNxrtYTickUseDefault,True);n++;
	      XtSetArg(args[n],XtNxrtYNumUseDefault,True);n++;
	      XtSetArg(args[n],XtNxrtYPrecisionUseDefault,True);n++;
	      break;
	    case Y2_AXIS_ELEMENT:
	      XtSetArg(args[n],XtNxrtY2MinUseDefault,True);n++;
	      XtSetArg(args[n],XtNxrtY2MaxUseDefault,True);n++;
	      XtSetArg(args[n],XtNxrtY2TickUseDefault,True);n++;
	      XtSetArg(args[n],XtNxrtY2NumUseDefault,True);n++;
	      XtSetArg(args[n],XtNxrtY2PrecisionUseDefault,True);n++;
	      break;
	    }
	  }
	  if (pcp) pcp->axisRange[rcType%3].isCurrentlyFromChannel = False;
	  break;
	}
      break;
    case CP_X_TIME_FORMAT :
      globalResourceBundle.axis[0].timeFormat =
	(CartesianPlotTimeFormat_t) (FIRST_CP_TIME_FORMAT + buttonId);
      break;

    case CP_PLOT_STYLE:
      XtSetArg(args[n], XtNxrtType, 
	       (buttonId == 0 ? XRT_TYPE_PLOT : XRT_TYPE_BAR)); n++;
      XtSetArg(args[n],XtNxrtType2,
	       (buttonId == 0 ? XRT_TYPE_PLOT : XRT_TYPE_BAR)); n++;
      break;

    default:
       printf("\ncpAxisptionMenuSimpleCallback: unknown rcType = %d",rcType/3);
      dm2kPrintf("\ncpAxisptionMenuSimpleCallback: unknown rcType = %d",rcType/3);
      break;
    }

  /****** Update for EDIT or EXECUTE mode */
  switch(globalDisplayListTraversalMode) {
    case DL_EDIT:
      if (currentDisplayInfo) {
        DlElement *dlElement = FirstDlElement(
            currentDisplayInfo->selectedDlElementList);
        unhighlightSelectedElements();
        while (dlElement) {
          updateElementFromGlobalResourceBundle(dlElement->structure.element);
          dlElement = dlElement->next;
        }
        dmTraverseNonWidgetsInDisplayList(currentDisplayInfo);
        highlightSelectedElements();
      }
      break;
    case DL_EXECUTE:
      if (executeTimeCartesianPlotWidget)
        XtSetValues(executeTimeCartesianPlotWidget,args,n);
      break;
  }
#endif
}

#ifdef __cplusplus
static void colorSelectCallback(Widget, XtPointer cd, XtPointer) {
#else
static void colorSelectCallback(Widget w, XtPointer cd, XtPointer cbs) {
#endif
    int rcType = (int) cd;

    if (colorMW != NULL) {
      setCurrentDisplayColorsInColorPalette(rcType,0);
      XtPopup(colorS,XtGrabNone);
    } else {
      createColor();
      setCurrentDisplayColorsInColorPalette(rcType,0);
      XtPopup(colorS,XtGrabNone);
    }
}

#ifdef __cplusplus
static void fileMenuSimpleCallback(Widget, XtPointer cd, XtPointer) {
#else
static void fileMenuSimpleCallback(Widget w, XtPointer cd, XtPointer cbs) {
#endif
    int buttonNumber = (int) cd;

    switch(buttonNumber) {
	    case FILE_CLOSE_BTN:
		XtPopdown(resourceS);
		break;
    }
}

/****** Text field verify callback  (verify numeric input) */
void textFieldNumericVerifyCallback(
  Widget w,
  XtPointer clientData,
  XtPointer callbackData)
{
   int rcType = (int) clientData;
   XmTextVerifyCallbackStruct *cbs = (XmTextVerifyCallbackStruct *) callbackData;
   int len;
   len = 0;
   if (cbs->startPos < cbs->currInsert) return;

   len = (int) XmTextFieldGetLastPosition(w);

/****** If no (new) data {e.g., setting to NULL string}, simply return */
   if (cbs->text->ptr == NULL) return;

/****** Check for leading sign (+/-) */
   if (len == 0) {	/* nothing there yet... therefore can add sign */
   if ((!isdigit(cbs->text->ptr[0]) && cbs->text->ptr[0] != '+'
      && cbs->text->ptr[0] != '-') ||
/****** Not a digit or +/-, move all chars down one and decrement length */
     (!isdigit(cbs->text->ptr[0]) && ((rcType == X_RC || rcType == Y_RC)
     && cbs->text->ptr[0] == '-')) ) {
     int i;
     for (i = len; (i+1) < cbs->text->length; i++)
       cbs->text->ptr[i] = cbs->text->ptr[i+1];
     cbs->text->length--;
     len--;
    }
  } else {
/****** Already a first character, therefore only digits allowed */
    for (len = 0; len < cbs->text->length; len++) {
/****** Not a digit - move all chars down one and decrement length */
      if (!isdigit(cbs->text->ptr[len])) {
	int i;
	for (i = len; (i+1) < cbs->text->length; i++)
	    cbs->text->ptr[i] = cbs->text->ptr[i+1];
	cbs->text->length--;
	len--;
      }
    }
  }
  if (cbs->text->length <= 0)
    cbs->doit = False;
}

#ifdef __cplusplus
void textFieldFloatVerifyCallback(Widget w, XtPointer, XtPointer pcbs) {
#else
void textFieldFloatVerifyCallback(Widget w, XtPointer cd, XtPointer pcbs) {
#endif
  XmTextVerifyCallbackStruct *cbs = (XmTextVerifyCallbackStruct *) pcbs;
    
  int len;
  Boolean haveDecimalPoint;
  char *textString;

  len = 0;
  if (cbs->startPos < cbs->currInsert) return;

  len = (int) XmTextFieldGetLastPosition(w);

/* see if there is already a decimal point in the string */
  textString = XmTextFieldGetString(w);
  if (strchr(textString,(int)'.'))
     haveDecimalPoint = True;
  else
     haveDecimalPoint = False;
  XtFree(textString);

/* odd behavior for programmatic reset - if NULL event, then programmatic
	and bypass previous determinations regarding decimal point, etc */
  if (cbs->event == NULL) {
    len = 0;
    haveDecimalPoint = False;
  }

/* if no (new) data {e.g., setting to NULL string, simply return */
  if (cbs->text->ptr == NULL) return;

/* check for leading sign (+/-) */
  if (len == 0) {	/* nothing there yet... therefore can add sign */
    if (!isdigit(cbs->text->ptr[0]) && cbs->text->ptr[0] != '+'
		&& cbs->text->ptr[0] != '-' && cbs->text->ptr[0] != '.') {
/* not a digit or +/-/.,  move all chars down one and decrement length */
      int i;
      for (i = len; (i+1) < cbs->text->length; i++)
	cbs->text->ptr[i] = cbs->text->ptr[i+1];
      cbs->text->length--;
      len--;
    } else if (cbs->text->ptr[0] != '.') {
      haveDecimalPoint = True;
    }
  } else {
/* already a first character, therefore only digits or potential
 * decimal point allowed */

    for (len = 0; len < cbs->text->length; len++) {
    /* make sure all additions are digits (or the first decimal point) */
      if (!isdigit(cbs->text->ptr[len])) {
	 if (cbs->text->ptr[len] == '.' && !haveDecimalPoint) {
	  haveDecimalPoint = True;
	 } else {
	/* not a digit (or is another decimal point)  
	   - move all chars down one and decrement length */
	  int i;
	  for (i = len; (i+1) < cbs->text->length; i++)
	    cbs->text->ptr[i] = cbs->text->ptr[i+1];
	  cbs->text->length--;
	  len--;
	 }
      }
    }
  }
  if (cbs->text->length <= 0)
    cbs->doit = False;
}

#ifdef __cplusplus
void scaleCallback(Widget, XtPointer cd, XtPointer pcbs) {
#else
void scaleCallback(Widget w, XtPointer cd, XtPointer pcbs) {
#endif
  int rcType = (int) cd;  /* the resource element type */
  XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *) pcbs;

/****** Show users degrees, but internally use degrees*64 as Xlib requires */
    switch(rcType) {
      case BEGIN_RC:
	globalResourceBundle.begin = 64*cbs->value;
	if (currentDisplayInfo->hasBeenEditedButNotSaved == False) 
	  dm2kMarkDisplayBeingEdited(currentDisplayInfo);
	break;
      case PATH_RC:
	globalResourceBundle.path = 64*cbs->value;
        if (currentDisplayInfo->hasBeenEditedButNotSaved == False) 
          dm2kMarkDisplayBeingEdited(currentDisplayInfo);
	break;
      default:
	break;
    }

/****** Update elements (this is overkill, but okay for now) */
    if (currentDisplayInfo != NULL) {
      DlElement *dlElement = FirstDlElement(
          currentDisplayInfo->selectedDlElementList);
      unhighlightSelectedElements();
      while (dlElement) {
        updateElementFromGlobalResourceBundle(dlElement->structure.element);
        dlElement = dlElement->next;
      }
      dmTraverseNonWidgetsInDisplayList(currentDisplayInfo);
      highlightSelectedElements();
    }
}

#ifdef __cplusplus
void textFieldActivateCallback(Widget w, XtPointer cd, XtPointer) {
#else
void textFieldActivateCallback(Widget w, XtPointer cd, XtPointer cbs) {
#endif
  int rcType = (int) cd;
  char *stringValue;
  Boolean truefalse;

  stringValue = XmTextFieldGetString(w);
  switch(rcType) {
     case X_RC:
	globalResourceBundle.x = atoi(stringValue);
	break;
     case Y_RC:
	globalResourceBundle.y = atoi(stringValue);
	break;
     case WIDTH_RC:
	globalResourceBundle.width = atoi(stringValue);
	break;
     case HEIGHT_RC:
	globalResourceBundle.height = atoi(stringValue);
	break;
     case RDBK_RC:
	renewString(&globalResourceBundle.chan,stringValue);
	break;
     case CTRL_RC:
	renewString(&globalResourceBundle.chan,stringValue);
	break;

     /* addition for Controller sensitivity */
     case SENSITIVE_CHAN_RC:
	renewString(&globalResourceBundle.sensitive_chan,stringValue);
	truefalse = (STRLEN(stringValue) > (size_t) 0) ;
	XtSetSensitive (resourceEntryRC[SENSITIVE_MODE_RC], truefalse);
	if ( truefalse ) XtManageChild (resourceEntryElement[SENSITIVE_MODE_RC]);
	else  XtUnmanageChild (resourceEntryElement[SENSITIVE_MODE_RC]);
	break;

     case TITLE_RC:
	renewString(&globalResourceBundle.title,stringValue);
	break;
     case XLABEL_RC:
	renewString(&globalResourceBundle.xlabel,stringValue);
	break;
     case YLABEL_RC:
	renewString(&globalResourceBundle.ylabel,stringValue);
	break;
     case LINEWIDTH_RC:
	globalResourceBundle.lineWidth = atoi(stringValue);
	break;
     case SBIT_RC: 
       {
	 int value = atoi(stringValue);
         if (value >= 0 && value <= 15) {
	   globalResourceBundle.sbit = value;
	 } else {
	   char tmp[32];
	   sprintf(tmp,"%d",globalResourceBundle.sbit);
	   XmTextFieldSetString(w,tmp);
	 }
       }
       break;
     case EBIT_RC:
       {
	 int value = atoi(stringValue);
         if (value >= 0 && value <= 15) {
	   globalResourceBundle.ebit = value;
	 } else {
	   char tmp[32];
	   sprintf(tmp,"%d",globalResourceBundle.ebit);
	   XmTextFieldSetString(w,tmp);
	 }
       }
       break;

/****** Since a non-NULL string value for the dynamics channel means that VIS 
        and CLRMOD must be visible */
     case CHAN_RC:
	renewString(&globalResourceBundle.chan,stringValue);
	if (STRLEN(stringValue) > (size_t) 0) {
          XtSetSensitive(resourceEntryRC[CLRMOD_RC],True);
          XtSetSensitive(resourceEntryRC[VIS_RC],True);
          if (globalResourceBundle.clrmod == DISCRETE) {
            XtSetSensitive(resourceEntryRC[COLOR_RULE_RC],True);
          } else {
            XtSetSensitive(resourceEntryRC[COLOR_RULE_RC],False);
          }
	} else {
          XtSetSensitive(resourceEntryRC[CLRMOD_RC],False);
          XtSetSensitive(resourceEntryRC[VIS_RC],False);
          XtSetSensitive(resourceEntryRC[COLOR_RULE_RC],False);
        }
        break;

     case DIS_RC:
	globalResourceBundle.dis = atoi(stringValue);
	break;
     case XYANGLE_RC:
	globalResourceBundle.xyangle = atoi(stringValue);
	break;
     case ZANGLE_RC:
	globalResourceBundle.zangle = atoi(stringValue);
	break;
     case PERIOD_RC:
	globalResourceBundle.period = atof(stringValue);
	break;
     case COUNT_RC:
	globalResourceBundle.count = atoi(stringValue);
	break;
     case TEXTIX_RC:
	renewString(&globalResourceBundle.textix,stringValue);
	break;
     case MSG_LABEL_RC:
	renewString(&globalResourceBundle.messageLabel,stringValue);
	break;
     case PRESS_MSG_RC:
	renewString(&globalResourceBundle.press_msg,stringValue);
	break;
     case RELEASE_MSG_RC:
	renewString(&globalResourceBundle.release_msg,stringValue);
	break;
     case IMAGENAME_RC:
	renewString(&globalResourceBundle.imageName,stringValue);
	break;
     case COMPOSITE_NAME_RC:
	renewString(&globalResourceBundle.compositeName,stringValue);
	break;
     case DATA_RC:
	renewString(&globalResourceBundle.data,stringValue);
	break;
     case CMAP_RC:
	renewString(&globalResourceBundle.cmap,stringValue);
	break;
     case PRECISION_RC:
	globalResourceBundle.dPrecision = atof(stringValue);
	break;
     case TRIGGER_RC:
	renewString(&globalResourceBundle.trigger,stringValue);
	break;
     case ERASE_RC:
        renewString(&globalResourceBundle.erase,stringValue);
        if (STRLEN(stringValue) > (size_t) 0) {
	  XtSetSensitive(resourceEntryRC[ERASE_MODE_RC],True);
	} else {
	  XtSetSensitive(resourceEntryRC[ERASE_MODE_RC],False);
	}
	break;
      case RD_LABEL_RC:
        renewString(&globalResourceBundle.rdLabel,stringValue);
        break;

  /* addition for Message Button type */
  case ACTIVE_LABEL_RC:
    renewString(&globalResourceBundle.toggleOnLabel, stringValue);
    break;

  case VAL_PRECISION_RC:
     if (stringValue[0])
	globalResourceBundle.valPrecision = atoi(stringValue);
     else
	globalResourceBundle.valPrecision = -1;
    break;

  case LOW_LIMIT_RC:
     if (stringValue[0])
	globalResourceBundle.dispayLowLimit = atof(stringValue);
     else
	globalResourceBundle.dispayLowLimit = MAXFLOAT;
    break;

  case HIGH_LIMIT_RC:
     if (stringValue[0])
	globalResourceBundle.dispayHighLimit = atof(stringValue);
     else
	globalResourceBundle.dispayHighLimit = MAXFLOAT;
    
    break;
  }

  XtFree(stringValue);

/****** Update elements (this is overkill, but okay for now) */
/* unhighlight (since objects may move) */
  if (currentDisplayInfo != NULL) {
     DlElement *dlElement = FirstDlElement(currentDisplayInfo->selectedDlElementList);
     unhighlightSelectedElements();
     while (dlElement) {
	updateElementFromGlobalResourceBundle(dlElement->structure.element);
	dlElement = dlElement->next;
     }
     dmTraverseNonWidgetsInDisplayList(currentDisplayInfo);
     /* highlight */
     highlightSelectedElements();
     if (currentDisplayInfo->hasBeenEditedButNotSaved == False) 
	dm2kMarkDisplayBeingEdited(currentDisplayInfo);
  }
}

#ifdef __cplusplus
void cpAxisTextFieldActivateCallback(Widget w, XtPointer cd, XtPointer) {
#else
void cpAxisTextFieldActivateCallback(Widget w, XtPointer cd, XtPointer cbs) {
#endif

#ifdef USE_XRT
    int rcType = (int) cd;
    char *stringValue, string[24];
    int k, n, iPrec;
    Arg args[6];
    XcVType valF, minF, maxF, tickF;
    String resourceName;

    stringValue = XmTextFieldGetString(w);

/****** For the strcpy() calls, note that the textField has a maxLength 
        resource set such that the strcpy always succeeds */
    n = 0;
    switch(rcType) {
      case CP_X_RANGE_MIN:
      case CP_Y_RANGE_MIN:
      case CP_Y2_RANGE_MIN:
	globalResourceBundle.axis[rcType%3].minRange= atof(stringValue);
	if (globalDisplayListTraversalMode == DL_EXECUTE) {
	    valF.fval = globalResourceBundle.axis[rcType%3].minRange;
	    switch(rcType) {
		case CP_X_RANGE_MIN: resourceName = XtNxrtXMin; break;
		case CP_Y_RANGE_MIN: resourceName = XtNxrtYMin; break;
		case CP_Y2_RANGE_MIN: resourceName = XtNxrtY2Min; break;
	    }
	    XtSetArg(args[n],resourceName,valF.lval); n++;
	}
	break;
     case CP_X_RANGE_MAX:
     case CP_Y_RANGE_MAX:
     case CP_Y2_RANGE_MAX:
	globalResourceBundle.axis[rcType%3].maxRange= atof(stringValue);
	if (globalDisplayListTraversalMode == DL_EXECUTE) {
	    valF.fval = globalResourceBundle.axis[rcType%3].maxRange;
	    switch(rcType) {
		case CP_X_RANGE_MAX: resourceName = XtNxrtXMax; break;
		case CP_Y_RANGE_MAX: resourceName = XtNxrtYMax; break;
		case CP_Y2_RANGE_MAX: resourceName = XtNxrtY2Max; break;
	    }
	    XtSetArg(args[n],resourceName,valF.lval); n++;
	}
	break;
     default:
	dm2kPrintf("\ncpAxisTextFieldActivateCallback: unknown rcType = %d",rcType/3);
	break;

  }

  minF.fval = globalResourceBundle.axis[rcType%3].minRange;
  maxF.fval = globalResourceBundle.axis[rcType%3].maxRange;
  tickF.fval = (maxF.fval - minF.fval)/4.0;
  switch(rcType%3) {
     case X_AXIS_ELEMENT: resourceName = XtNxrtXTick; break;
     case Y1_AXIS_ELEMENT: resourceName = XtNxrtYTick; break;
     case Y2_AXIS_ELEMENT: resourceName = XtNxrtY2Tick; break;
  }

  XtSetArg(args[n],resourceName,tickF.lval); n++;
  switch(rcType%3) {
     case X_AXIS_ELEMENT: resourceName = XtNxrtXNum; break;
     case Y1_AXIS_ELEMENT: resourceName = XtNxrtYNum; break;
     case Y2_AXIS_ELEMENT: resourceName = XtNxrtY2Num; break;
  }

  XtSetArg(args[n],resourceName,tickF.lval); n++;
  switch(rcType%3) {
     case X_AXIS_ELEMENT: resourceName = XtNxrtXPrecision; break;
     case Y1_AXIS_ELEMENT: resourceName = XtNxrtYPrecision; break;
     case Y2_AXIS_ELEMENT: resourceName = XtNxrtY2Precision; break;
  }

  XtSetArg(args[n],resourceName,tickF.lval); n++;
  sprintf(string,"%f",tickF.fval);
  k = STRLEN(string)-1;
  while (string[k] == '0') k--;	/* strip off trailing zeroes */
  iPrec = k;
  while (string[k] != '.' && k >= 0) k--;
  iPrec = iPrec - k;
  XtSetArg(args[n],resourceName,iPrec); n++;

  XtFree(stringValue);

/*
 *  update for EDIT or EXECUTE mode
 */

  switch(globalDisplayListTraversalMode) {

   case DL_EDIT:
/*
 * update elements (this is overkill, but okay for now)
 *	-- not as efficient as it should be (don't update EVERYTHING if only
 *	   one item changed!)
 */
    if (currentDisplayInfo != NULL) {
      DlElement *dlElement = FirstDlElement(
          currentDisplayInfo->selectedDlElementList);
      unhighlightSelectedElements();
      while (dlElement) {
        updateElementFromGlobalResourceBundle(dlElement->structure.element);
        dlElement = dlElement->next;
      }
      dmTraverseNonWidgetsInDisplayList(currentDisplayInfo);
      highlightSelectedElements();
    }
    break;

   case DL_EXECUTE:
        if (executeTimeCartesianPlotWidget != NULL)
           XtSetValues(executeTimeCartesianPlotWidget,args,n);
        break;
  }
#endif
}

#ifdef __cplusplus
static void textFieldLosingFocusCallback(Widget, XtPointer cd, XtPointer) {
#else
static void textFieldLosingFocusCallback(Widget w, XtPointer cd, XtPointer cbs) {
#endif
  int    rcType = (int) cd;
  char * newString = NULL;
  char   string[MAX_TOKEN_LENGTH];
  int    tail;
  
  string[0] = '\0';

  /* losing focus - make sure that the text field remains accurate
   * wrt globalResourceBundle
   */
  switch(rcType) {
    case X_RC:
      sprintf(string,"%d",globalResourceBundle.x);
      break;
    case Y_RC:
      sprintf(string,"%d",globalResourceBundle.y);
      break;
    case WIDTH_RC:
      sprintf(string,"%d",globalResourceBundle.width);
      break;
    case HEIGHT_RC:
      sprintf(string,"%d",globalResourceBundle.height);
      break;
    case LINEWIDTH_RC:
      sprintf(string,"%d",globalResourceBundle.lineWidth);
      break;
    case RDBK_RC:
      newString = globalResourceBundle.chan;
      break;
    case CTRL_RC:
      newString = globalResourceBundle.chan;
      break;

    case SENSITIVE_CHAN_RC:
      newString = globalResourceBundle.sensitive_chan;
      break;

    case TITLE_RC:
      newString = globalResourceBundle.title;
      break;
    case XLABEL_RC:
      newString = globalResourceBundle.xlabel;
      break;
    case YLABEL_RC:
      newString = globalResourceBundle.ylabel;
      break;
    case CHAN_RC:
      newString = globalResourceBundle.chan;
      break;
    case DIS_RC:
      sprintf(string,"%d",globalResourceBundle.dis);
      break;
    case XYANGLE_RC:
      sprintf(string,"%d",globalResourceBundle.xyangle);
      break;
    case ZANGLE_RC:
      sprintf(string,"%d",globalResourceBundle.zangle);
      break;
    case PERIOD_RC:
      cvtDoubleToString(globalResourceBundle.period,string,0);
      break;
    case COUNT_RC:
      sprintf(string,"%d",globalResourceBundle.count);
      break;
    case TEXTIX_RC:
      newString = globalResourceBundle.textix;
      break;
    case MSG_LABEL_RC:
      newString = globalResourceBundle.messageLabel;
      break;
    case PRESS_MSG_RC:
      newString = globalResourceBundle.press_msg;
      break;
    case RELEASE_MSG_RC:
      newString = globalResourceBundle.release_msg;
      break;
    case IMAGENAME_RC:
      newString = globalResourceBundle.imageName;
      break;
    case DATA_RC:
      newString = globalResourceBundle.data;
      break;
    case CMAP_RC:
      newString = globalResourceBundle.cmap;
      break;
    case  VAL_PRECISION_RC:
       if (globalResourceBundle.valPrecision >= 0)
	  sprintf(string,"%d",globalResourceBundle.valPrecision);
       else
	  strcpy(string,"");
       break;
    case  LOW_LIMIT_RC:
       if (globalResourceBundle.dispayLowLimit != MAXFLOAT)
	  sprintf(string,"%g",globalResourceBundle.dispayLowLimit);
       else
	  strcpy(string,"");
      break;
    case  HIGH_LIMIT_RC:
       if (globalResourceBundle.dispayHighLimit != MAXFLOAT)
	  sprintf(string,"%g",globalResourceBundle.dispayHighLimit);
       else
	  strcpy(string,"");
      break;
    case PRECISION_RC:
      sprintf(string,"%f",globalResourceBundle.dPrecision);
      /* strip trailing zeroes */
      tail = STRLEN(string);
      while (string[--tail] == '0') string[tail] = '\0';
      break;
    case SBIT_RC:
      sprintf(string,"%d",globalResourceBundle.sbit);
      break;
    case EBIT_RC:
      sprintf(string,"%d",globalResourceBundle.ebit);
      break;
    case TRIGGER_RC:
      newString= globalResourceBundle.trigger;
      break;
    case ERASE_RC :
      newString= globalResourceBundle.erase;
      break;
    case RD_LABEL_RC :
      newString = globalResourceBundle.rdLabel;
      break;
      
    /* addition for Message Button type */
    case ACTIVE_LABEL_RC:
      newString= globalResourceBundle.toggleOnLabel;
      break;
    case COMPOSITE_NAME_RC:
      newString= globalResourceBundle.compositeName;
      break;
  default :
    INFORM_INTERNAL_ERROR();
    break;
  }

  if (newString == NULL)
    newString = string;

  XmTextFieldSetString(resourceEntryElement[rcType],CARE_PRINT(newString));
}

#ifdef __cplusplus
void cpAxisTextFieldLosingFocusCallback(Widget, XtPointer cd, XtPointer) {
#else
void cpAxisTextFieldLosingFocusCallback(Widget w, XtPointer cd, XtPointer cbs) {
#endif
#ifdef USE_XRT
    int rcType = (int) cd;
    char string[MAX_TOKEN_LENGTH], *currentString;
    int tail;
    float minF[3], maxF[3];

/****** Losing focus - make sure that the text field remains accurate wrt 
        values stored in widget (not necessarily what is in 
        globalResourceBundle) */
  if (executeTimeCartesianPlotWidget != NULL)
     XtVaGetValues(executeTimeCartesianPlotWidget,
		    XtNxrtXMin,&minF[X_AXIS_ELEMENT],
		    XtNxrtYMin,&minF[Y1_AXIS_ELEMENT],
		    XtNxrtY2Min,&minF[Y2_AXIS_ELEMENT],
		    XtNxrtXMax,&maxF[X_AXIS_ELEMENT],
		    XtNxrtYMax,&maxF[Y1_AXIS_ELEMENT],
		    XtNxrtY2Max,&maxF[Y2_AXIS_ELEMENT], NULL);
  else
     return;
/*
 * losing focus - make sure that the text field remains accurate
 *	wrt values stored in widget (not necessarily what is in
 *	globalResourceBundle)
 */
  switch(rcType) {
     case CP_X_RANGE_MIN:
     case CP_Y_RANGE_MIN:
     case CP_Y2_RANGE_MIN:
	sprintf(string,"%f", minF[rcType%3]);
	break;
     case CP_X_RANGE_MAX:
     case CP_Y_RANGE_MAX:
     case CP_Y2_RANGE_MAX:
	sprintf(string,"%f", maxF[rcType%3]);
	break;
     default:
	fprintf(stderr,
	"\ncpAxisTextFieldLosingFocusCallback: unknown rcType = %d",
		rcType/3);
	return;
  }
  /* strip trailing zeroes */
  tail = STRLEN(string);
  while (string[--tail] == '0') string[tail] = '\0';
  switch(rcType) {
     case CP_X_RANGE_MIN:
     case CP_Y_RANGE_MIN:
     case CP_Y2_RANGE_MIN:
	currentString = XmTextFieldGetString(axisRangeMin[rcType%3]);
	if (strcmp(string,currentString))
	   XmTextFieldSetString(axisRangeMin[rcType%3],string);
	XtFree(currentString);
	break;
     case CP_X_RANGE_MAX:
     case CP_Y_RANGE_MAX:
     case CP_Y2_RANGE_MAX:
	currentString = XmTextFieldGetString(axisRangeMax[rcType%3]);
	if (strcmp(string,currentString))
	   XmTextFieldSetString(axisRangeMax[rcType%3],string);
	XtFree(currentString);
	break;
     default:
	break;
  }
#endif
}

/*
 * initialize globalResourceBundle with (semi-arbitrary) values
 */
void initializeGlobalResourceBundle()
{
 int i;

 globalResourceBundle.x = 0;
 globalResourceBundle.y = 0;
 globalResourceBundle.width = 10;
 globalResourceBundle.height = 10;
 globalResourceBundle.sbit = 15;
 globalResourceBundle.ebit = 0;
 globalResourceBundle.rdLabel = NULL;
 globalResourceBundle.rdVisual = RD_MENU;
#if 0
 globalResourceBundle.rdbk = NULL;
 globalResourceBundle.ctrl = NULL;
#endif
 globalResourceBundle.title = NULL;
 globalResourceBundle.xlabel = NULL;
 globalResourceBundle.ylabel = NULL;

 if (currentDisplayInfo) {

   /*
    * (MDA) hopefully this will work in the general case (with displays being
    *	made current and un-current)
    */
   globalResourceBundle.clr = currentDisplayInfo->drawingAreaForegroundColor;
   globalResourceBundle.bclr = currentDisplayInfo->drawingAreaBackgroundColor;
 } else {

   /*
    * (MDA) this isn't safe if the non-standard colormap is loaded, but this
    *	shouldn't get called unless starting from scratch...
    */
   globalResourceBundle.clr = 14;	/* black */
   globalResourceBundle.bclr = 4;	/* grey  */
 }

 globalResourceBundle.begin = 0;
 globalResourceBundle.path = 64*90;		/* arc in first quadrant */
 globalResourceBundle.alignment= ALIGNMENT_NW;
 globalResourceBundle.format = DECIMAL;
 globalResourceBundle.label = LABEL_NONE;
 globalResourceBundle.direction = RIGHT;
 globalResourceBundle.clrmod = STATIC;
 globalResourceBundle.colorRule = NULL;
 globalResourceBundle.fillmod = FROM_EDGE;
 globalResourceBundle.style = SOLID;
 globalResourceBundle.fill = F_SOLID;
 globalResourceBundle.lineWidth = 0;
 globalResourceBundle.dPrecision = 1.;
 globalResourceBundle.vis = V_STATIC;
 globalResourceBundle.chan = NULL;
 globalResourceBundle.data_clr = 0;
 globalResourceBundle.dis = 10;
 globalResourceBundle.xyangle = 45;
 globalResourceBundle.zangle = 45;
 globalResourceBundle.period = 60.0;
 globalResourceBundle.units = SECONDS;
 globalResourceBundle.cStyle = POINT_PLOT;
 globalResourceBundle.erase_oldest = ERASE_OLDEST_OFF;
 globalResourceBundle.count = 1;
 globalResourceBundle.stacking = ROW;
 globalResourceBundle.imageType= NO_IMAGE;
 globalResourceBundle.name = NULL;
 globalResourceBundle.textix = NULL;
 globalResourceBundle.messageLabel = NULL;
 globalResourceBundle.press_msg = NULL;
 globalResourceBundle.release_msg = NULL;
 globalResourceBundle.imageName = NULL;
 globalResourceBundle.compositeName = NULL;
 globalResourceBundle.relDisplay = NULL; 
 globalResourceBundle.ami = NULL; 
 globalResourceBundle.data = NULL;
 globalResourceBundle.cmap = NULL;

 for (i = 0; i < MAX_RELATED_DISPLAYS; i++) {
   globalResourceBundle.rdData[i].label = NULL;
   globalResourceBundle.rdData[i].name = NULL;
   globalResourceBundle.rdData[i].args = NULL;
   globalResourceBundle.rdData[i].mode = ADD_NEW_DISPLAY;
 }

 for (i = 0; i < MAX_SHELL_COMMANDS; i++) {
   globalResourceBundle.cmdData[i].label = NULL;
   globalResourceBundle.cmdData[i].command = NULL;
   globalResourceBundle.cmdData[i].args = NULL;
 }

 for (i = 0; i < MAX_TRACES; i++) {
   globalResourceBundle.cpData[i].xdata = NULL;
   globalResourceBundle.cpData[i].ydata = NULL;
   globalResourceBundle.cpData[i].data_clr = 0;
 }

 for (i = 0; i < MAX_PENS; i++) {
    globalResourceBundle.scData[i].chan = NULL; 
    globalResourceBundle.scData[i].utilChan = NULL;
    globalResourceBundle.scData[i].clr = 0;
 }

  plotAxisDefinitionInit(&(globalResourceBundle.axis[X_AXIS_ELEMENT]));
  /* structure copy for other two axis definitions */
  globalResourceBundle.axis[Y1_AXIS_ELEMENT]
	= globalResourceBundle.axis[X_AXIS_ELEMENT];
  globalResourceBundle.axis[Y2_AXIS_ELEMENT]
	= globalResourceBundle.axis[X_AXIS_ELEMENT];
  globalResourceBundle.trigger = NULL;
  globalResourceBundle.erase = NULL;
  globalResourceBundle.eraseMode = ERASE_IF_NOT_ZERO;

  /* extension for Button type and sensitivity */
  globalResourceBundle.sensitive_chan    = NULL;
  globalResourceBundle.sensitive_mode    = IF_NOT_ZERO;
  globalResourceBundle.messageButtonType = PUSH_BUTTON;
  globalResourceBundle.toggleOnLabel     = NULL;
  globalResourceBundle.abclr             = globalResourceBundle.abclr;

  /* extension for Display type */
  globalResourceBundle.displayType = NORMAL_DISPLAY;

  /* extension for Bar type */
  globalResourceBundle.scaleType        = FIRST_SCALE_TYPE;
  globalResourceBundle.barOnly          = FIRST_SHOW_BAR;
  globalResourceBundle.showAlarmLimits  = FIRST_SHOW_ALARM_LIMITS_TYPE;
  globalResourceBundle.showScale        = FIRST_SHOW_SCALE_TYPE;
/*  globalResourceBundle.displayLimitType = FIRST_DISPLAY_LIMIT_TYPE;*/
  globalResourceBundle.dispayLowLimit   = MAXFLOAT;
  globalResourceBundle.dispayHighLimit  = MAXFLOAT;
  globalResourceBundle.valPrecision     = -1;
}

/*
 * Initialize XmStrintg Value Tables: ResourceBundle and related widgets. 
 */
void initializeXmStringValueTables() 
{
    int i;
    static Boolean initialized = False;

/****** Initialize XmString table for element types */
    if (!initialized) {
      initialized = True;
      for (i = 0; i <NUM_DL_ELEMENT_TYPES; i++) {
        elementXmStringTable[i] = XmStringCreateSimple(elementStringTable[i]);
      }

/****** Initialize XmString table for value types (format, alignment types) */
      for (i = 0; i < NUMBER_STRING_VALUES; i++) {
        xmStringValueTable[i] = XmStringCreateSimple(stringValueTable[i]);
      }
      xmstringSelect = XmStringCreateSimple("Select...");
    }
}

/*
 * Create Resource: Create and initialize the resourcePalette,
 *   resourceBundle and related widgets. 
 */
void createResource() 
{
  
  Widget       entriesSW;
  Widget       resourceMB;
  Widget       messageF; 
  Widget       resourceHelpPDM;
  Widget       menuHelpWidget;
  XmString     buttons[N_MAX_MENU_ELES];
  KeySym       keySyms[N_MAX_MENU_ELES];
  XmButtonType buttonType[N_MAX_MENU_ELES];
  int          i, n;
  Arg          args[10];

  /* If resource palette has already been created, simply return 
   */
  if (resourceMW != NULL) return;

  /* This make take a second... give user some indication 
   */
  if (currentDisplayInfo != NULL) 
    XDefineCursor(display, XtWindow(currentDisplayInfo->drawingArea), 
		  watchCursor);
  
  /* Initialize XmString tables 
   */
  initializeXmStringValueTables();
  xmstringSelect = XmStringCreateSimple("Select...");
  
  /* Create a main window in a dialog 
   */
  n = 0;
  XtSetArg(args[n],XtNiconName,            "Resources"); n++;
  XtSetArg(args[n],XmNautoUnmanage,        False); n++;
  XtSetArg(args[n],XtNtitle,               "Resource Palette"); n++;
  XtSetArg(args[n],XtNallowShellResize,    TRUE); n++;
  XtSetArg(args[n],XmNkeyboardFocusPolicy, XmEXPLICIT); n++;
  
  /****** Map window manager menu Close function to application close... */
  XtSetArg(args[n],XmNdeleteResponse,   XmDO_NOTHING); n++;
  XtSetArg(args[n],XmNmwmDecorations,   MWM_DECOR_ALL|MWM_DECOR_RESIZEH); n++;
  resourceS = XtCreatePopupShell("resourceS",topLevelShellWidgetClass,
				 mainShell,args,n);

  XmAddWMProtocolCallback(resourceS,WM_DELETE_WINDOW,
			  wmCloseCallback,(XtPointer)OTHER_SHELL);
  
  resourceMW = XmCreateMainWindow(resourceS,"resourceMW",NULL,0);
  
  /* Create the menu bar 
   */
  buttons[0] = XmStringCreateSimple("File");
  
  buttons[1] = XmStringCreateSimple("Help");
  keySyms[1] = 'H';

  keySyms[0] = 'F';
  n = 0;
  
#if 0
  XtSetArg(args[n],XmNbuttonCount,N_MAIN_MENU_ELES); n++;
  XtSetArg(args[n],XmNbuttons,buttons); n++;
  XtSetArg(args[n],XmNbuttonMnemonics,keySyms); n++;
  XtSetArg(args[n],XmNforeground,defaultForeground); n++;
  XtSetArg(args[n],XmNbackground,defaultBackground); n++;
  resourceMB = XmCreateSimpleMenuBar(resourceMW, "resourceMB",args,n);
#endif
  
  resourceMB = XmVaCreateSimpleMenuBar(resourceMW, "resourceMB",
				       XmVaCASCADEBUTTON, buttons[0], 'F',
				       XmVaCASCADEBUTTON, buttons[1], 'H',
				       NULL);
  
  /* Color resourceMB properly (force so VUE doesn't interfere) 
   */
  colorMenuBar(resourceMB,defaultForeground,defaultBackground);
  
  
  /* set the Help cascade button in the menu bar 
   */
  menuHelpWidget = XtNameToWidget(resourceMB,"*button_1");
  
  XtVaSetValues(resourceMB,XmNmenuHelpWidget,menuHelpWidget, NULL);
  for (i = 0; i < N_MAIN_MENU_ELES; i++) 
    XmStringFree(buttons[i]);
  
  /* create the file pulldown menu pane 
   */
  buttons[0] = XmStringCreateSimple("Close");
  keySyms[0] = 'C';
  buttonType[0] = XmPUSHBUTTON;

  n = 0;
  XtSetArg(args[n],XmNbuttonCount,     N_FILE_MENU_ELES); n++;
  XtSetArg(args[n],XmNbuttons,         buttons); n++;
  XtSetArg(args[n],XmNbuttonType,      buttonType); n++;
  XtSetArg(args[n],XmNbuttonMnemonics, keySyms); n++;
  XtSetArg(args[n],XmNpostFromButton,  FILE_BTN_POSN); n++;
  XtSetArg(args[n],XmNsimpleCallback,  fileMenuSimpleCallback); n++;

  XmCreateSimplePulldownMenu(resourceMB,"resourceFilePDM", args,n);

  for (i = 0; i < N_FILE_MENU_ELES; i++) 
    XmStringFree(buttons[i]);

  /* create the help pulldown menu pane 
   */
  buttons[0] = XmStringCreateSimple("On Resource Palette...");
  keySyms[0] = 'C';
  buttonType[0] = XmPUSHBUTTON;

  n = 0;
  XtSetArg(args[n],XmNbuttonCount,     N_HELP_MENU_ELES); n++;
  XtSetArg(args[n],XmNbuttons,         buttons); n++;
  XtSetArg(args[n],XmNbuttonType,      buttonType); n++;
  XtSetArg(args[n],XmNbuttonMnemonics, keySyms); n++;
  XtSetArg(args[n],XmNpostFromButton,  HELP_BTN_POSN); n++;
  resourceHelpPDM = XmCreateSimplePulldownMenu(resourceMB,
					       "resourceHelpPDM",args,n);
  XmStringFree(buttons[0]);

  /* (MDA) for now, disable this menu 
   */
  XtSetSensitive(resourceHelpPDM,False);

  /* Add the resource entry scrolled window and contents 
   */
  n = 0;
  XtSetArg(args[n],XmNscrollingPolicy,          XmAUTOMATIC); n++;
  XtSetArg(args[n],XmNscrollBarDisplayPolicy,   XmAS_NEEDED); n++;
  entriesSW = XmCreateScrolledWindow(resourceMW,"entriesSW",args,n);

  createResourceEntries(entriesSW);
  
  /* add a message/status and dispatch area (this is clumsier than need-be,
   * but perhaps necessary (at least for now)) 
   */
  n = 0;
  XtSetArg(args[n],XmNtopOffset,                0); n++;
  XtSetArg(args[n],XmNbottomOffset,             0); n++;
  XtSetArg(args[n],XmNshadowThickness,          0); n++;
  messageF = XmCreateForm(resourceMW,"messageF",args,n);

  createBundleButtons(messageF);

  /* Manage the composites 
   */
  XtManageChild(messageF);
  XtManageChild(resourceMB);
  XtManageChild(entriesSW);
  XtManageChild(resourceMW);
  
  XmMainWindowSetAreas(resourceMW,resourceMB,NULL,NULL,NULL,entriesSW);
  XtVaSetValues(resourceMW,XmNmessageWindow,messageF, NULL);
  
  /* Now popup the dialog and restore cursor 
   */
  XtPopup(resourceS,XtGrabNone);
  
  /* change drawingArea's cursor back to the appropriate cursor 
   */
  if (currentDisplayInfo != NULL)
    XDefineCursor(display,XtWindow(currentDisplayInfo->drawingArea),
		  (currentActionType == SELECT_ACTION ? 
		 rubberbandCursor: crosshairCursor));
}


/*
 * Create Resource Entries: Create resource entries in scrolled window      
 */
static void createResourceEntries(Widget entriesSW) 
{
  Widget entriesRC;
  Arg    args[12];
  int    i, n;

  n = 0;
  XtSetArg(args[n],XmNnumColumns,   1); n++;
  XtSetArg(args[n],XmNorientation,  XmVERTICAL); n++;
  XtSetArg(args[n],XmNpacking,      XmPACK_COLUMN); n++;
  entriesRC = XmCreateRowColumn(entriesSW, "entriesRC", args, n);
  
  /* Create the row-columns which are entries into overall row-column
   *	these entries are specific to resource bundle elements, and
   *	are managed/unmanaged according to the selected widgets being
   *	editted...  (see WidgetDM.h for details on this) 
   */
  for (i = MIN_RESOURCE_ENTRY; i < MAX_RESOURCE_ENTRY; i++) 
    createEntryRC(entriesRC,i);
  
  initializeResourcePaletteElements();

  /* now resize the labels and elements (to maximum's width)
   *	for uniform appearance 
   */
  XtSetArg(args[0],XmNrecomputeSize, False);
  XtSetArg(args[1],XmNalignment,     XmALIGNMENT_END);

  XtSetArg(args[2],XmNwidth,         maxLabelWidth);
  XtSetArg(args[3],XmNheight,        maxLabelHeight);

  XtSetArg(args[4],XmNx,             (Position)maxLabelWidth);
  XtSetArg(args[5],XmNrecomputeSize, False);
  XtSetArg(args[6],XmNresizeWidth,   False);
  XtSetArg(args[7],XmNmarginWidth,   0);

  for (i = MIN_RESOURCE_ENTRY; i < MAX_RESOURCE_ENTRY; i++) 
    {
      /* Set label 
       */
      /**2000/12 G.Lei add "if (resourceEntryLabel[i])" and
       ** "if (!resourceEntryElement[i])"
       */
      if (resourceEntryLabel[i])
      XtSetValues(resourceEntryLabel[i], args, 4);
      
      /* Set element 
       */
      if (!resourceEntryElement[i])
	printf("resourceEntryElement %d is NULL! \n", i);
      else
      {
      if (XtClass(resourceEntryElement[i]) == xmRowColumnWidgetClass) {
	/* must be option menu - unmanage label widget 
	 */
	XtUnmanageChild(XmOptionLabelGadget(resourceEntryElement[i]));
	XtSetValues(XmOptionButtonGadget(resourceEntryElement[i]),
		    &(args[2]),6);
      }

      XtSetValues(resourceEntryElement[i], &(args[2]), 6);

      /* restrict size of CA PV name entry 
       */
      if (i == CHAN_RC || 
	  i == RDBK_RC || 
	  i == CTRL_RC)
      {
	XtVaSetValues(resourceEntryElement[i],
		      XmNcolumns,(short)(PVNAME_STRINGSZ + FLDNAME_SZ+1),
		      /* since can have macro-substituted strings, 
		       * need longer length 
		       */
		      XmNmaxLength,(int)MAX_TOKEN_LENGTH-1,
		      NULL);
      } 
      else if (i == MSG_LABEL_RC   || 
	       i == PRESS_MSG_RC   ||
	       i == RELEASE_MSG_RC ||
	       i == TEXTIX_RC      ||
	       i == TITLE_RC       ||
	       i == XLABEL_RC 	   ||
	       i == YLABEL_RC) 
	{
	  /* use size of CA PV name entry for other text-oriented fields 
	   */
	  XtVaSetValues(resourceEntryElement[i],
			XmNcolumns,(short)(PVNAME_STRINGSZ + FLDNAME_SZ+1),
			NULL);
	}
    }
    }
  
  XtManageChild(entriesRC);
}

/* ---------------------------------------------------------------------- */
#define SIMPLE_OPTION_MENU(firstButton,count)                              \
  XtSetArg(args[0],XmNbuttonType,     buttonType); 		           \
  XtSetArg(args[1],XmNbuttons,        &(xmStringValueTable[firstButton])); \
  XtSetArg(args[2],XmNbuttonCount,    count); 		                   \
  XtSetArg(args[3],XmNsimpleCallback, optionMenuSimpleCallback); 	   \
  XtSetArg(args[4],XmNuserData,       rcType); 		                   \
  localElement = XmCreateSimpleOptionMenu(localRC,"localElement", args, 5) \
/* ---------------------------------------------------------------------- */



/*
 * Create Entry RC: Create the various row-columns for each resource entry
 * rcType = {X_RC,Y_RC,...}. 
 */
static void createEntryRC( Widget parent, int rcType) 
{
  Widget localRC, localLabel, localElement;
  XmString labelString;
  Dimension width, height;
  Arg args[10];
  int n;

  XmButtonType buttonType[MAX_OPTIONS];

  for (n = 0; n < MAX_OPTIONS; n++) {
    buttonType[n] = XmPUSHBUTTON;
  }

  n = 0;
  XtSetArg(args[n],XmNorientation, XmVERTICAL); n++;
  XtSetArg(args[n],XmNpacking,     XmPACK_NONE); n++;
  localRC = XmCreateRowColumn(parent,"entryRC",args,n);

  /* Create the label element 
   */
  labelString = XmStringCreateSimple(resourceEntryStringTable[rcType]);

  n = 0;
  XtSetArg(args[n],XmNalignment,     XmALIGNMENT_END); n++;
  XtSetArg(args[n],XmNlabelString,   labelString); n++;
  XtSetArg(args[n],XmNrecomputeSize, False); n++;
  localLabel = XmCreateLabel(localRC,"localLabel",args,n);
  XmStringFree(labelString);

  /* Create the selection element (text entry, option menu, etc) 
   */
  switch(rcType) 
    {
    /* numeric text field types */
    case X_RC:
    case Y_RC:
    case WIDTH_RC:
    case HEIGHT_RC:
    case SBIT_RC:
    case EBIT_RC:
    case DIS_RC:
    case XYANGLE_RC:
    case ZANGLE_RC:
    case PERIOD_RC:
    case COUNT_RC:
    case LINEWIDTH_RC:
    case VAL_PRECISION_RC:
      n = 0;
      XtSetArg(args[n],XmNmaxLength,MAX_TOKEN_LENGTH-1); n++;
      localElement = XmCreateTextField(localRC,"localElement",args,n);
      XtAddCallback(localElement,XmNactivateCallback,
                    textFieldActivateCallback,(XtPointer)rcType);
      XtAddCallback(localElement,XmNlosingFocusCallback,
                    textFieldLosingFocusCallback,(XtPointer)rcType);

      if (rcType != VAL_PRECISION_RC) {
	 XtAddCallback(localElement,XmNmodifyVerifyCallback,
		       textFieldNumericVerifyCallback,(XtPointer)rcType); /* !!!!! */
      }
      break;

    case LOW_LIMIT_RC:
    case HIGH_LIMIT_RC:
    case PRECISION_RC:
      n = 0;
      XtSetArg(args[n],XmNmaxLength,MAX_TOKEN_LENGTH-1); n++;
      localElement = XmCreateTextField(localRC,"localElement",args,n);
      XtAddCallback(localElement,XmNactivateCallback,
                    textFieldActivateCallback,(XtPointer)rcType);
      XtAddCallback(localElement,XmNlosingFocusCallback,
                    textFieldLosingFocusCallback,(XtPointer)rcType);

      if (rcType != LOW_LIMIT_RC || rcType == HIGH_LIMIT_RC) {
	 XtAddCallback(localElement,XmNmodifyVerifyCallback,
		       textFieldFloatVerifyCallback,(XtPointer)NULL); /* !!!!! */
      }
      
      break;

      /* alpha-numeric text field types 
       */
    case RDBK_RC:
    case CTRL_RC:
    case TITLE_RC:
    case SENSITIVE_CHAN_RC:
    case XLABEL_RC:
    case YLABEL_RC:
    case CHAN_RC:
    case TEXTIX_RC:
    case MSG_LABEL_RC:
    case PRESS_MSG_RC:
    case RELEASE_MSG_RC:
    case IMAGENAME_RC:
    case COMPOSITE_NAME_RC: 
    case DATA_RC:
    case CMAP_RC:
    case NAME_RC:
    case TRIGGER_RC:
    case ERASE_RC:
    case RD_LABEL_RC:
    case ACTIVE_LABEL_RC:
      n = 0;
      XtSetArg(args[n],XmNmaxLength,MAX_TOKEN_LENGTH-1); n++;
      localElement = XmCreateTextField(localRC,"localElement",args,n);
      XtAddCallback(localElement,XmNactivateCallback,
                    textFieldActivateCallback,(XtPointer)rcType);
      XtAddCallback(localElement,XmNlosingFocusCallback,
                    textFieldLosingFocusCallback,(XtPointer)rcType);

      break;

      /* scale (slider) types 
       */
    case BEGIN_RC:
    case PATH_RC:
	n = 0;
	XtSetArg(args[n],XmNminimum,0); n++;
	XtSetArg(args[n],XmNmaximum,360); n++;
	XtSetArg(args[n],XmNorientation,XmHORIZONTAL); n++;
	XtSetArg(args[n],XmNshowValue,True); n++;
	XtSetArg(args[n],XmNorientation,XmHORIZONTAL); n++;
	XtSetArg(args[n],XmNscaleMultiple,15); n++;
	localElement = XmCreateScale(localRC,"localElement",args,n);
	XtAddCallback(localElement,XmNvalueChangedCallback,
			scaleCallback,(XtPointer)rcType);
	XtAddCallback(localElement,XmNdragCallback,
			scaleCallback,(XtPointer)rcType);
	break;

      /* option menu types 
       */
     case COLOR_RULE_RC:
       {
	 XmButtonType * buttonTypes;
	 XmString     * xmStringButtons;
	 ColorRule    * colorRule;

	 /** 2000/12 G.Lei **/
	 if (colorRuleCounts<=0) {
	   localElement = NULL;
	   break;
	 }
	 buttonTypes = (XmButtonType *) 
	   calloc (colorRuleCounts, sizeof(XmButtonType));
	 
	 xmStringButtons = (XmString*)
	   calloc (colorRuleCounts, sizeof(XmString));

	 if (buttonTypes == NULL || xmStringButtons == NULL) {
	   DM2KFREE(buttonTypes);
	   DM2KFREE(xmStringButtons);
	   localElement = NULL;
	   break;
	 }

	 for (n = 0, colorRule = colorRuleHead;
	      n < colorRuleCounts && colorRule != NULL;
	      n++, colorRule = colorRule->next) 
	   {
	     buttonTypes[n] = XmPUSHBUTTON;
	     xmStringButtons[n] = XmStringCreateSimple(colorRule->name);
	   }

	 colorRuleCounts = n;

	 n = 0;
	 XtSetArg(args[n],XmNbuttonType,  buttonTypes); n++;
	 XtSetArg(args[n],XmNbuttons,     xmStringButtons); n++;
	 XtSetArg(args[n],XmNbuttonCount, colorRuleCounts); n++;
	 XtSetArg(args[n],XmNsimpleCallback,
		  (XtCallbackProc)optionMenuSimpleCallback); n++;
	 XtSetArg(args[n],XmNuserData,rcType); n++;
	 XtSetArg(args[n],XmNnumColumns,4); n++;
	 XtSetArg(args[n],XmNpacking,XmPACK_COLUMN); n++;
	 localElement =XmCreateSimpleOptionMenu(localRC,"localElement",args,n);

	 for (n = 0; n < colorRuleCounts; n++) {
	   XmStringFree(xmStringButtons[n]);
	 }

	 DM2KFREE(buttonTypes);
	 DM2KFREE(xmStringButtons);
       }
     
     if (globalResourceBundle.clrmod != DISCRETE)
       XtSetSensitive(localRC, False);

     break;

     case GRAPHIC_RULE_RC:
       {
	 XmButtonType * buttonTypes;
	 XmString     * xmStringButtons;
	 GraphicRule  * graphicRule;

	 buttonTypes = (XmButtonType *) 
	   calloc (graphicRuleCounts+1, sizeof(XmButtonType));
	 
	 xmStringButtons = (XmString*)
	   calloc (graphicRuleCounts+1, sizeof(XmString));

	 if (buttonTypes == NULL || xmStringButtons == NULL) {
	   DM2KFREE(buttonTypes);
	   DM2KFREE(xmStringButtons);
	   break;
	 }

	 
	 buttonTypes[0]     = XmPUSHBUTTON;
	 xmStringButtons[0] = XmStringCreateSimple("<not set>");

	 for (n = 0, graphicRule = graphicRuleHead;
	      n < graphicRuleCounts && graphicRule != NULL;
	      n++, graphicRule = graphicRule->next) 
	   {
	     buttonTypes[n+1]     = XmPUSHBUTTON;
	     xmStringButtons[n+1] = XmStringCreateSimple(graphicRule->name);
	   }

	 graphicRuleCounts = n+1;

	 n = 0;
	 XtSetArg(args[n],XmNbuttonType,  buttonTypes); n++;
	 XtSetArg(args[n],XmNbuttons,     xmStringButtons); n++;
	 XtSetArg(args[n],XmNbuttonCount, graphicRuleCounts); n++;
	 XtSetArg(args[n],XmNsimpleCallback,
		  (XtCallbackProc)optionMenuSimpleCallback); n++;
	 XtSetArg(args[n],XmNuserData,rcType); n++;
	 XtSetArg(args[n],XmNnumColumns,4); n++;
	 XtSetArg(args[n],XmNpacking,XmPACK_COLUMN); n++;
	 localElement =XmCreateSimpleOptionMenu(localRC,"localElement",args,n);

	 for (n = 0; n < graphicRuleCounts; n++) {
	   XmStringFree(xmStringButtons[n]);
	 }

	 DM2KFREE(buttonTypes);
	 DM2KFREE(xmStringButtons);
       }

     /*
     if (globalResourceBundle.clrmod != DISCRETE)
       XtSetSensitive(localRC, False);
       */
     break;

     case ALIGNMENT_RC:
        SIMPLE_OPTION_MENU(FIRST_ALIGNMENT, NUM_ALIGNMENTS);
	break;

     case FORMAT_RC:
        SIMPLE_OPTION_MENU(FIRST_TEXT_FORMAT, NUM_TEXT_FORMATS);
	break;

     case LABEL_RC:
        SIMPLE_OPTION_MENU(FIRST_LABEL_TYPE, NUM_LABEL_TYPES);
	break;

     case DIRECTION_RC:
        SIMPLE_OPTION_MENU(FIRST_DIRECTION, NUM_DIRECTIONS);
	break;

     case CLRMOD_RC:
        SIMPLE_OPTION_MENU(FIRST_COLOR_MODE, NUM_COLOR_MODES);
	break;

     case FILLMOD_RC:
        SIMPLE_OPTION_MENU(FIRST_FILL_MODE, NUM_FILL_MODES);
	break;

     case STYLE_RC:
        SIMPLE_OPTION_MENU(FIRST_EDGE_STYLE, NUM_EDGE_STYLES);
	break;

     case FILL_RC:
        SIMPLE_OPTION_MENU(FIRST_FILL_STYLE, NUM_FILL_STYLES);
	break;

     case VIS_RC:
        SIMPLE_OPTION_MENU(FIRST_VISIBILITY_MODE, NUM_VISIBILITY_MODES);
	break;

     case BUTTON_TYPE_RC:
        SIMPLE_OPTION_MENU(FIRST_BUTTON_TYPE, NUM_BUTTON_TYPES);
	break;

     case SENSITIVE_MODE_RC:
        SIMPLE_OPTION_MENU(FIRST_SENSITIVE_MODE, NUM_SENSITIVE_MODES);
	break;

     case DISPLAY_TYPE_RC:
        SIMPLE_OPTION_MENU(FIRST_DISPLAY_TYPE, NUM_DISPLAY_TYPES);
	break;

     case UNITS_RC:
        SIMPLE_OPTION_MENU(FIRST_TIME_UNIT, NUM_TIME_UNITS);
	break;

     case CSTYLE_RC:
        SIMPLE_OPTION_MENU(FIRST_CARTESIAN_PLOT_STYLE, NUM_CARTESIAN_PLOT_STYLES);
	break;

     case ERASE_OLDEST_RC:
        SIMPLE_OPTION_MENU(FIRST_ERASE_OLDEST, NUM_ERASE_OLDESTS);
	break;

     case ERASE_MODE_RC:
        SIMPLE_OPTION_MENU(FIRST_ERASE_MODE, NUM_ERASE_MODES);
	break;

     case STACKING_RC:
        SIMPLE_OPTION_MENU(FIRST_STACKING, NUM_STACKINGS);
	break;

     case IMAGETYPE_RC:
       /* MDA - when TIFF is implemented: */
       /* SIMPLE_OPTION_MENU(FIRST_IMAGE_TYPE, NUM_IMAGE_TYPES); */

        SIMPLE_OPTION_MENU(FIRST_IMAGE_TYPE, 2);
	break;

     case RD_VISUAL_RC:
        SIMPLE_OPTION_MENU(FIRST_RD_VISUAL, NUM_RD_VISUALS);
       break;

     case SCALE_TYPE_RC:
        SIMPLE_OPTION_MENU(FIRST_SCALE_TYPE, NUM_SCALE_TYPES);
	break;

     case BAR_ONLY_RC:
        SIMPLE_OPTION_MENU(FIRST_SHOW_BAR,  NUM_SHOW_BARS);
	break;
	
    case  SHOW_ALARM_LIMIT_RC:
        SIMPLE_OPTION_MENU(FIRST_SHOW_ALARM_LIMITS_TYPE, NUM_SHOW_ALARM_LIMITS_TYPES);
	break;

    case  SHOW_SCALE_RC:
        SIMPLE_OPTION_MENU(FIRST_SHOW_SCALE_TYPE, NUM_SHOW_SCALE_TYPES);
	break;

    case  FIT_RC:
        SIMPLE_OPTION_MENU(FIRST_FIT_TYPE, NUM_FIT_TYPES);
        break;

      /* color types 
       */
     case CLR_RC:
     case BCLR_RC:
     case DATA_CLR_RC:
     case ACTIVE_COLOR_RC:
	n = 0;
	if (rcType == CLR_RC) {
	    XtSetArg(args[n],XmNbackground,
		(currentDisplayInfo == NULL) ? BlackPixel(display,screenNum) :
			currentDisplayInfo->colormap[
			currentDisplayInfo->drawingAreaForegroundColor]); n++;
	} else {
	    XtSetArg(args[n],XmNbackground,
		(currentDisplayInfo == NULL) ? WhitePixel(display,screenNum) :
		currentDisplayInfo->colormap[
			currentDisplayInfo->drawingAreaBackgroundColor]); n++;
	}
	localElement = XmCreateDrawnButton(localRC,"localElement",args,n);
	XtAddCallback(localElement,XmNactivateCallback,
		      colorSelectCallback,(XtPointer)rcType);
	break;


     case ASSOM_RC:
     case RDDATA_RC:
     case CPDATA_RC:
     case SCDATA_RC:
     case SHELLDATA_RC:
     case CPAXIS_RC:
	n = 0;
	XtSetArg(args[n],XmNlabelString,   dlXmStringMoreToComeSymbol); n++;
	XtSetArg(args[n],XmNalignment,     XmALIGNMENT_CENTER); n++;
	XtSetArg(args[n],XmNrecomputeSize, False); n++;
	localElement = XmCreatePushButton(localRC,"localElement",args,n);
	XtAddCallback(localElement,XmNactivateCallback,
		pushButtonActivateCallback,(XtPointer)rcType);
	break;

     default:
	printf("\n------MISSED TYPE  %d  IN createEntryRC()!!--------\n",rcType);
	break;
  }

  XtVaGetValues(localLabel,XmNwidth,&width,XmNheight,&height,NULL);
  maxLabelWidth = MAX(maxLabelWidth,width);
  maxLabelHeight = MAX(maxLabelHeight,height);

#if 0
  XtVaGetValues(localElement,XmNwidth,&width,XmNheight,&height,NULL);
  maxLabelWidth = MAX(maxLabelWidth,width);
  maxLabelHeight = MAX(maxLabelHeight,height);
#endif

  /** 2000/12 G.Lei found application crashes if XtManageChild(widget)
  *** has a NULL widget. So "if (localLabel)" and "if (localElement)"
  *** were added.
  **/
  if (localLabel) 
  XtManageChild(localLabel);
  else printf("createEntryRC %d: localLabel is NULL\n", rcType);
  if (localElement)
  XtManageChild(localElement);

  /* update global variables 
   */
  resourceEntryRC[rcType] = localRC;
  resourceEntryLabel[rcType] = localLabel;
  resourceEntryElement[rcType] = localElement;
}

static int table[] = {
  DL_Display,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, CLR_RC, BCLR_RC, CMAP_RC,
    DISPLAY_TYPE_RC,
    -1,

  DL_ChoiceButton,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, CTRL_RC, CLR_RC, BCLR_RC, CLRMOD_RC,
    STACKING_RC,
    SENSITIVE_CHAN_RC, SENSITIVE_MODE_RC,
    -1,

  DL_Menu,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, CTRL_RC, CLR_RC, BCLR_RC, CLRMOD_RC,
    SENSITIVE_CHAN_RC, SENSITIVE_MODE_RC,
    -1,

  DL_MessageButton,
    BUTTON_TYPE_RC,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, CTRL_RC, CLR_RC, BCLR_RC, MSG_LABEL_RC,
    PRESS_MSG_RC, RELEASE_MSG_RC, CLRMOD_RC,
    ACTIVE_LABEL_RC, ACTIVE_COLOR_RC,
    SENSITIVE_CHAN_RC, SENSITIVE_MODE_RC,
    -1,

  DL_Valuator,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, CTRL_RC, CLR_RC, BCLR_RC, LABEL_RC,
    CLRMOD_RC, DIRECTION_RC, PRECISION_RC,
    SENSITIVE_CHAN_RC, SENSITIVE_MODE_RC,
    LOW_LIMIT_RC, HIGH_LIMIT_RC, VAL_PRECISION_RC,
    -1,

  DL_TextEntry,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, CTRL_RC, CLR_RC, BCLR_RC, CLRMOD_RC,
    FORMAT_RC,
    SENSITIVE_CHAN_RC, SENSITIVE_MODE_RC,
    -1,

  DL_Meter,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, RDBK_RC, CLR_RC, BCLR_RC, LABEL_RC,
  CLRMOD_RC, LOW_LIMIT_RC, HIGH_LIMIT_RC, VAL_PRECISION_RC,
    -1,

  DL_TextUpdate,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, RDBK_RC, CLR_RC, BCLR_RC, CLRMOD_RC,
    ALIGNMENT_RC, FORMAT_RC,
    VAL_PRECISION_RC,
    -1,

  DL_Bar,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, RDBK_RC, CLR_RC, BCLR_RC, LABEL_RC,
    CLRMOD_RC, COLOR_RULE_RC, DIRECTION_RC, FILLMOD_RC, SCALE_TYPE_RC, 
    BAR_ONLY_RC,  SHOW_ALARM_LIMIT_RC, SHOW_SCALE_RC,
    LOW_LIMIT_RC, HIGH_LIMIT_RC,  VAL_PRECISION_RC,
    -1,

  DL_Byte,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, RDBK_RC, CLR_RC, BCLR_RC, SBIT_RC,
    EBIT_RC, CLRMOD_RC, DIRECTION_RC, 
    -1,

  DL_Indicator,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, RDBK_RC, CLR_RC, BCLR_RC, LABEL_RC,
    CLRMOD_RC, DIRECTION_RC, 
    LOW_LIMIT_RC, HIGH_LIMIT_RC,  VAL_PRECISION_RC,
    -1,

  DL_StripChart,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, TITLE_RC, XLABEL_RC, YLABEL_RC, CLR_RC,
    BCLR_RC, PERIOD_RC, UNITS_RC, SCDATA_RC, 
    -1,

  DL_CartesianPlot,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, TITLE_RC, XLABEL_RC, YLABEL_RC, CLR_RC,
    BCLR_RC, CSTYLE_RC, ERASE_OLDEST_RC, COUNT_RC, CPDATA_RC, CPAXIS_RC,
    TRIGGER_RC, ERASE_RC, ERASE_MODE_RC, -1,

  DL_Rectangle,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, CLR_RC, STYLE_RC, FILL_RC, LINEWIDTH_RC,
    CLRMOD_RC, COLOR_RULE_RC, VIS_RC, CHAN_RC, 
    -1,

  DL_Oval,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, CLR_RC, STYLE_RC, FILL_RC, LINEWIDTH_RC,
    CLRMOD_RC, COLOR_RULE_RC, VIS_RC, CHAN_RC, 
    -1,

  DL_Arc,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, BEGIN_RC, PATH_RC, CLR_RC, STYLE_RC,
    FILL_RC, LINEWIDTH_RC, CLRMOD_RC, COLOR_RULE_RC, VIS_RC, CHAN_RC, 
    -1,

  DL_Text,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, TEXTIX_RC, ALIGNMENT_RC, CLR_RC,
    CLRMOD_RC, COLOR_RULE_RC, VIS_RC, CHAN_RC, 
    -1,

  DL_RelatedDisplay,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, CLR_RC, BCLR_RC,
/*    COLOR_RULE_RC, VIS_RC, CHAN_RC, */
    RD_LABEL_RC, RD_VISUAL_RC, RDDATA_RC, 
  -1,

  DL_ShellCommand,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, CLR_RC, BCLR_RC, SHELLDATA_RC, 
    -1,

  DL_Image,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, IMAGETYPE_RC, IMAGENAME_RC, 
    -1,

  DL_Composite,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, CLR_RC, ASSOM_RC, COMPOSITE_NAME_RC,
    -1,

  DL_Line,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, CLR_RC, STYLE_RC, LINEWIDTH_RC,
    CLRMOD_RC, COLOR_RULE_RC, VIS_RC, CHAN_RC, 
    -1,

  DL_Polyline,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, CLR_RC, STYLE_RC, LINEWIDTH_RC,
    CLRMOD_RC, COLOR_RULE_RC, VIS_RC, CHAN_RC, 
    -1,

  DL_Polygon,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, CLR_RC, STYLE_RC, FILL_RC, LINEWIDTH_RC,
    CLRMOD_RC, COLOR_RULE_RC, VIS_RC, CHAN_RC, 
    -1,

  DL_DynSymbol,
    X_RC, Y_RC, WIDTH_RC, HEIGHT_RC, RDBK_RC, FIT_RC,
    BCLR_RC, CLRMOD_RC, COLOR_RULE_RC, GRAPHIC_RULE_RC,
    -1,
};

static void initializeResourcePaletteElements() 
{
  int i, j, index;
  int tableSize = XtNumber(table);

  index = -1;
  for (i = 0; i < tableSize; i++) 
    {
      if (index < 0) 
	{
	  /* start a new element, get the new index 
	   */
	  index = table[i] - MIN_DL_ELEMENT_TYPE;
	  j = 0;
	} 
      else 
	{
	  if (table[i] >= 0) 
	    {
	      /* copy RC resource from table until it meet -1 
	       */
	      resourcePaletteElements[index].childIndexRC[j] = table[i];
	      resourcePaletteElements[index].children[j] =
		resourceEntryRC[table[i]];
	      j++;
	    } 
	  else 
	    {
	      int k;

	      /* reset the index, fill the rest with zero 
	       */
	      for (k = j; k < MAX_RESOURCES_FOR_DL_ELEMENT; k++) 
		{
		  resourcePaletteElements[index].childIndexRC[k] = 0;
		  resourcePaletteElements[index].children[k] = NULL;
		}
	      resourcePaletteElements[index].numChildren = j;
	      index = -1;
	    }
	}
    }  
}

#ifdef __cplusplus
static void shellCommandActivate(Widget, XtPointer cd, XtPointer)
#else
static void shellCommandActivate(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  int buttonType = (int) cd;
  String **newCells;
  int i;

  switch (buttonType) {
    case CMD_APPLY_BTN:
  /* commit changes in matrix to global matrix array data */
     XbaeMatrixCommitEdit(cmdMatrix,False);
     XtVaGetValues(cmdMatrix,XmNcells,&newCells,NULL);
  /* now update globalResourceBundle...*/
      for (i = 0; i < MAX_SHELL_COMMANDS; i++) {
        renewString(&globalResourceBundle.cmdData[i].label, newCells[i][0]);
        renewString(&globalResourceBundle.cmdData[i].command, newCells[i][1]);
        renewString(&globalResourceBundle.cmdData[i].args, newCells[i][2]);
      }
  /* and update the elements (since this level of "Ok" is analogous
   *	to changing text in a text field in the resource palette
   *	(don't need to traverse the display list since these changes
   *	 aren't visible at the first level)
   */
      if (currentDisplayInfo) {
        DlElement *dlElement = FirstDlElement(
          currentDisplayInfo->selectedDlElementList);
        unhighlightSelectedElements();
        while (dlElement) {
          if ((dlElement->structure.element->type = DL_ShellCommand))
            updateElementFromGlobalResourceBundle(dlElement->structure.element);
          dlElement = dlElement->next;
        }
      }
      XtPopdown(shellCommandS);
      if (currentDisplayInfo->hasBeenEditedButNotSaved == False) 
        dm2kMarkDisplayBeingEdited(currentDisplayInfo);
      break;

    case CMD_CLOSE_BTN:
      XtPopdown(shellCommandS);
      break;
  }
}

/*
 * create shell command data dialog
 */
Widget createShellCommandDataDialog(
  Widget parent)
{
  Widget shell, applyButton, closeButton, sw;
  Dimension cWidth, cHeight, aWidth, aHeight;
  Arg args[12];
  XmString xmString;
  int i, j, n;
  static Boolean first = True;


/* initialize those file-scoped globals */
  if (first) {
    first = False;
    for (i = 0; i < MAX_SHELL_COMMANDS; i++) {
      for (j = 0; j < 3; j++) cmdRows[i][j] = NULL;
      cmdCells[i] = &cmdRows[i][0];
    }
  }


/*
 * now create the interface
 *
 *	       label | cmd | args
 *	       -------------------
 *	    1 |  A      B      C
 *	    2 | 
 *	    3 | 
 *		     ...
 *		 OK     CANCEL
 */

  n = 0;
  XtSetArg(args[n],XmNautoUnmanage,False); n++;
  XtSetArg(args[n],XmNmarginHeight,8); n++;
  XtSetArg(args[n],XmNmarginWidth,8); n++;
  cmdForm = XmCreateFormDialog(parent,"shellCommandDataF",args,n);
  shell = XtParent(cmdForm);
  n = 0;
  XtSetArg(args[n],XmNtitle,"Shell Command Data"); n++;
  XtSetArg(args[n],XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_RESIZEH); n++;
  XtSetValues(shell,args,n);
  XmAddWMProtocolCallback(shell,WM_DELETE_WINDOW,
		shellCommandActivate,(XtPointer)CMD_CLOSE_BTN);

  n = 0;
  XtSetArg(args[n],XmNscrollingPolicy,          XmAUTOMATIC); n++;
  XtSetArg(args[n],XmNscrollBarDisplayPolicy,   XmAS_NEEDED); n++;
  XtSetArg(args[n],XmNwidth,   1050); n++;
  XtSetArg(args[n],XmNheight,   600); n++;
  sw = XmCreateScrolledWindow(cmdForm, "scrollScMatrix", args, n);

  n = 0;
  XtSetArg(args[n],XmNrows,MAX_RELATED_DISPLAYS); n++;
  XtSetArg(args[n],XmNcolumns,3); n++;
  XtSetArg(args[n],XmNcolumnMaxLengths,cmdColumnMaxLengths); n++;
  XtSetArg(args[n],XmNcolumnWidths,cmdColumnWidths); n++;
  XtSetArg(args[n],XmNcolumnLabels,cmdColumnLabels); n++;
  XtSetArg(args[n],XmNcolumnMaxLengths,cmdColumnMaxLengths); n++;
  XtSetArg(args[n],XmNcolumnWidths,cmdColumnWidths); n++;
  XtSetArg(args[n],XmNcolumnLabelAlignments,cmdColumnLabelAlignments); n++;
  XtSetArg(args[n],XmNboldLabels,False); n++;
  cmdMatrix = XtCreateManagedWidget("cmdMatrix",
			xbaeMatrixWidgetClass,sw,args,n);


  xmString = XmStringCreateSimple("Close");
  n = 0;
  XtSetArg(args[n],XmNlabelString,xmString); n++;
  closeButton = XmCreatePushButton(cmdForm,"closeButton",args,n);
  XtAddCallback(closeButton,XmNactivateCallback,
		shellCommandActivate,(XtPointer)CMD_CLOSE_BTN);
  XtManageChild(closeButton);
  XmStringFree(xmString);

  xmString = XmStringCreateSimple("Apply");
  n = 0;
  XtSetArg(args[n],XmNlabelString,xmString); n++;
  applyButton = XmCreatePushButton(cmdForm,"applyButton",args,n);
  XtAddCallback(applyButton,XmNactivateCallback,
		shellCommandActivate,(XtPointer)CMD_APPLY_BTN);
  XtManageChild(applyButton);
  XmStringFree(xmString);

/* make APPLY and CLOSE buttons same size */
  XtVaGetValues(closeButton,XmNwidth,&cWidth,XmNheight,&cHeight,NULL);
  XtVaGetValues(applyButton,XmNwidth,&aWidth,XmNheight,&aHeight,NULL);
  XtVaSetValues(closeButton,XmNwidth,MAX(cWidth,aWidth),
			XmNheight,MAX(cHeight,aHeight),NULL);

/* and make the APPLY button the default for the form */
  XtVaSetValues(cmdForm,XmNdefaultButton,applyButton,NULL);

/*
 * now do form layout 
 */

/* cmdMatrix */
  n = 0;
  XtSetArg(args[n],XmNtopAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNleftAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNrightAttachment,XmATTACH_FORM); n++;
  XtSetValues(sw,args,n);
/* apply */
  n = 0;
  XtSetArg(args[n],XmNleftAttachment,XmATTACH_POSITION); n++;
  XtSetArg(args[n],XmNleftPosition,30); n++;
  XtSetArg(args[n],XmNbottomAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNbottomOffset,12); n++;
  XtSetValues(applyButton,args,n);
/* close */
  n = 0;
  XtSetArg(args[n],XmNrightAttachment,XmATTACH_POSITION); n++;
  XtSetArg(args[n],XmNrightPosition,70); n++;
  XtSetArg(args[n],XmNbottomAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNbottomOffset,12); n++;
  XtSetValues(closeButton,args,n);

/* scrolled window */
  n = 0;
  XtSetArg(args[n],XmNbottomAttachment,   XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNbottomWidget,       applyButton); n++;
  XtSetArg(args[n],XmNbottomOffset,       12); n++;
  XtSetValues(sw,args,n);
 

  XtManageChild(sw);
  XtManageChild(cmdForm);

  return shell;
}

/*
 * access function (for file-scoped globals, etc) to udpate the
 *	shell command data dialog with the values currently in
 *	globalResourceBundle
 */
void updateShellCommandDataDialog() {
  int i;

  for (i = 0; i < MAX_SHELL_COMMANDS; i++) {
    cmdRows[i][0] = globalResourceBundle.cmdData[i].label;
    cmdRows[i][1] = globalResourceBundle.cmdData[i].command;
    cmdRows[i][2] = globalResourceBundle.cmdData[i].args;
  }
  if (cmdMatrix != NULL) XtVaSetValues(cmdMatrix,XmNcells,cmdCells,NULL);
  
}

#ifdef __cplusplus
static void cartesianPlotActivate(Widget, XtPointer cd, XtPointer)
#else
static void cartesianPlotActivate(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  int buttonType = (int) cd;
  String **newCells;
  int i;

  switch (buttonType) {
    case CP_APPLY_BTN:
      /* commit changes in matrix to global matrix array data */
      XbaeMatrixCommitEdit(cpMatrix,False);
      XtVaGetValues(cpMatrix,XmNcells,&newCells,NULL);
      /* now update globalResourceBundle...*/
      for (i = 0; i < MAX_TRACES; i++) {
        renewString(&globalResourceBundle.cpData[i].xdata,
		    newCells[i][CP_XDATA_COLUMN]);
        renewString(&globalResourceBundle.cpData[i].ydata,
		    newCells[i][CP_YDATA_COLUMN]);
        globalResourceBundle.cpData[i].data_clr = 
	  (int) cpColorRows[i][CP_COLOR_COLUMN];
      }
      /* and update the elements (since this level of "Apply" is analogous
       *	to changing text in a text field in the resource palette
       *	(don't need to traverse the display list since these changes
       *	 aren't visible at the first level)
       */
      if (currentDisplayInfo) {
        DlElement *dlElement = FirstDlElement(
          currentDisplayInfo->selectedDlElementList);
        unhighlightSelectedElements();
        while (dlElement) {
          if ((dlElement->structure.element->type = DL_CartesianPlot))
            updateElementFromGlobalResourceBundle(dlElement->structure.element);
          dlElement = dlElement->next;
        }
      }
      XtPopdown(cartesianPlotS);
      if (currentDisplayInfo->hasBeenEditedButNotSaved == False) 
        dm2kMarkDisplayBeingEdited(currentDisplayInfo);
      break;

    case CP_CLOSE_BTN:
      XtPopdown(cartesianPlotS);
      break;
  }
}


#ifdef __cplusplus
static void cartesianPlotAxisActivate(Widget, XtPointer cd, XtPointer)
#else
static void cartesianPlotAxisActivate(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  int buttonType = (int) cd;

  switch (buttonType) {

   case CP_CLOSE_BTN:
     XtPopdown(cartesianPlotAxisS);
    /* since done with CP Axis dialog, reset that selected widget */
     executeTimeCartesianPlotWidget  = NULL;
     break;
  }
}

/*
 * function to handle cell selection in the matrix
 *	mostly it passes through for the text field entry
 *	but pops up the color editor for the color field selection
 */
#ifdef __cplusplus
void cpEnterCellCallback(Widget, XtPointer, XtPointer cbs)
#else
void cpEnterCellCallback(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  XbaeMatrixEnterCellCallbackStruct *call_data = (XbaeMatrixEnterCellCallbackStruct *) cbs;
  int row;
  if (call_data->column == CP_COLOR_COLUMN) {
    /* set this cell non-editable */
    call_data->doit = False;
    /* update the color palette, set index of the color vector element to set */
    row = call_data->row;
    setCurrentDisplayColorsInColorPalette(CPDATA_RC,row);
    XtPopup(colorS,XtGrabNone);
  }
}


/*
 * function to actually update the colors in the COLOR_COLUMN of the matrix
 */
void cpUpdateMatrixColors()
{
  int i;

/* XmNcolors needs pixel values */
  for (i = 0; i < MAX_TRACES; i++) {
    cpColorRows[i][CP_COLOR_COLUMN] = currentColormap[
	globalResourceBundle.cpData[i].data_clr];
  }
  if (cpMatrix != NULL) XtVaSetValues(cpMatrix,XmNcolors,cpColorCells,NULL);

/* but for resource editing cpData should contain indexes into colormap */
/* this resource is copied, hence this is okay to do */
  for (i = 0; i < MAX_TRACES; i++) {
    cpColorRows[i][CP_COLOR_COLUMN] = globalResourceBundle.cpData[i].data_clr;
  }
}

/*
 * create data dialog
 */
Widget createCartesianPlotDataDialog(
  Widget parent)
{
  Widget shell, applyButton, closeButton;
  Dimension cWidth, cHeight, aWidth, aHeight;
  Arg args[12];
  XmString xmString;
  int i, j, n;
  static Boolean first = True;


/* initialize those file-scoped globals */
  if (first) {
    first = False;
    for (i = 0; i < MAX_TRACES; i++) {
      for (j = 0; j < 2; j++) cpRows[i][j] = NULL;
      cpRows[i][2] = dashes;
      cpCells[i] = &cpRows[i][0];
      cpColorCells[i] = &cpColorRows[i][0];
    }
  }


/*
 * now create the interface
 *
 *	       xdata | ydata | color
 *	       ---------------------
 *	    1 |  A      B      C
 *	    2 | 
 *	    3 | 
 *		     ...
 *		 OK     CANCEL
 */

  n = 0;
  XtSetArg(args[n],XmNautoUnmanage,False); n++;
  XtSetArg(args[n],XmNmarginHeight,8); n++;
  XtSetArg(args[n],XmNmarginWidth,8); n++;
  XtSetArg(args[n],XmNdialogStyle,XmDIALOG_PRIMARY_APPLICATION_MODAL); n++;
  cpForm = XmCreateFormDialog(parent,"cartesianPlotDataF",args,n);
  shell = XtParent(cpForm);
  n = 0;
  XtSetArg(args[n],XmNtitle,"Cartesian Plot Data"); n++;
  XtSetArg(args[n],XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_RESIZEH); n++;
  XtSetValues(shell,args,n);
  XmAddWMProtocolCallback(shell,WM_DELETE_WINDOW,
		cartesianPlotActivate,(XtPointer)CP_CLOSE_BTN);
  n = 0;
  XtSetArg(args[n],XmNrows,MAX_TRACES); n++;
  XtSetArg(args[n],XmNcolumns,3); n++;
  XtSetArg(args[n],XmNcolumnMaxLengths,cpColumnMaxLengths); n++;
  XtSetArg(args[n],XmNcolumnWidths,cpColumnWidths); n++;
  XtSetArg(args[n],XmNcolumnLabels,cpColumnLabels); n++;
  XtSetArg(args[n],XmNcolumnMaxLengths,cpColumnMaxLengths); n++;
  XtSetArg(args[n],XmNcolumnWidths,cpColumnWidths); n++;
  XtSetArg(args[n],XmNcolumnLabelAlignments,cpColumnLabelAlignments); n++;
  XtSetArg(args[n],XmNboldLabels,False); n++;
  cpMatrix = XtCreateManagedWidget("cpMatrix",
			xbaeMatrixWidgetClass,cpForm,args,n);
  cpUpdateMatrixColors();
  XtAddCallback(cpMatrix,XmNenterCellCallback,
			cpEnterCellCallback,(XtPointer)NULL);


  xmString = XmStringCreateSimple("Close");
  n = 0;
  XtSetArg(args[n],XmNlabelString,xmString); n++;
  closeButton = XmCreatePushButton(cpForm,"closeButton",args,n);
  XtAddCallback(closeButton,XmNactivateCallback,
		cartesianPlotActivate,(XtPointer)CP_CLOSE_BTN);
  XtManageChild(closeButton);
  XmStringFree(xmString);

  xmString = XmStringCreateSimple("Apply");
  n = 0;
  XtSetArg(args[n],XmNlabelString,xmString); n++;
  applyButton = XmCreatePushButton(cpForm,"applyButton",args,n);
  XtAddCallback(applyButton,XmNactivateCallback,
		cartesianPlotActivate,(XtPointer)CP_APPLY_BTN);
  XtManageChild(applyButton);
  XmStringFree(xmString);

/* make APPLY and CLOSE buttons same size */
  XtVaGetValues(closeButton,XmNwidth,&cWidth,XmNheight,&cHeight,NULL);
  XtVaGetValues(applyButton,XmNwidth,&aWidth,XmNheight,&aHeight,NULL);
  XtVaSetValues(closeButton,XmNwidth,MAX(cWidth,aWidth),
			XmNheight,MAX(cHeight,aHeight),NULL);

/* and make the APPLY button the default for the form */
  XtVaSetValues(cpForm,XmNdefaultButton,applyButton,NULL);

/*
 * now do form layout 
 */

/* cpMatrix */
  n = 0;
  XtSetArg(args[n],XmNtopAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNleftAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNrightAttachment,XmATTACH_FORM); n++;
  XtSetValues(cpMatrix,args,n);
/* apply */
  n = 0;
  XtSetArg(args[n],XmNtopAttachment,XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNtopWidget,cpMatrix); n++;
  XtSetArg(args[n],XmNtopOffset,12); n++;
  XtSetArg(args[n],XmNleftAttachment,XmATTACH_POSITION); n++;
  XtSetArg(args[n],XmNleftPosition,25); n++;
  XtSetArg(args[n],XmNbottomAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNbottomOffset,12); n++;
  XtSetValues(applyButton,args,n);
/* close */
  n = 0;
  XtSetArg(args[n],XmNtopAttachment,XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNtopWidget,cpMatrix); n++;
  XtSetArg(args[n],XmNtopOffset,12); n++;
  XtSetArg(args[n],XmNrightAttachment,XmATTACH_POSITION); n++;
  XtSetArg(args[n],XmNrightPosition,75); n++;
  XtSetArg(args[n],XmNbottomAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNbottomOffset,12); n++;
  XtSetValues(closeButton,args,n);


  XtManageChild(cpForm);

  return shell;
}

/*
 * access function (for file-scoped globals, etc) to udpate the
 *	cartesian plot data dialog with the values currently in
 *	globalResourceBundle
 */
void updateCartesianPlotDataDialog()
{
  int i;

  for (i = 0; i < MAX_TRACES; i++) {
    cpRows[i][0] = globalResourceBundle.cpData[i].xdata;
    cpRows[i][1] = globalResourceBundle.cpData[i].ydata;
    cpRows[i][2] =  dashes;
  }
  /* handle data_clr in here */
  cpUpdateMatrixColors();
  if (cpMatrix)
    XtVaSetValues(cpMatrix,XmNcells,cpCells,NULL);
}

#ifdef __cplusplus
static void stripChartActivate(Widget, XtPointer cd, XtPointer) 
#else
static void stripChartActivate(Widget w, XtPointer cd, XtPointer cbs) 
#endif
{
  int       buttonType = (int) cd;
  String ** newCells;
  int       i;

  switch (buttonType) {

    case SC_APPLY_BTN:
      /* commit changes in matrix to global matrix array data 
       */
      XbaeMatrixCommitEdit(scMatrix,False);
      XtVaGetValues(scMatrix,XmNcells,&newCells,NULL);

      /* now update globalResourceBundle...
       */
      for (i = 0; i < MAX_PENS; i++) {
        renewString(&globalResourceBundle.scData[i].chan,
               newCells[i][SC_CHANNEL_COLUMN]);
        renewString(&globalResourceBundle.scData[i].utilChan,
               newCells[i][SC_ARCH_CHANNEL_COLUMN]);
        globalResourceBundle.scData[i].clr =
	  (int) scColorRows[i][SC_COLOR_COLUMN];
      }

      /* and update the elements (since this level of "Apply" is analogous
       *	to changing text in a text field in the resource palette
       *	(don't need to traverse the display list since these changes
       *	 aren't visible at the first level)
       */
      if (currentDisplayInfo != NULL) {
        DlElement *dlElement = FirstDlElement(
          currentDisplayInfo->selectedDlElementList);

        unhighlightSelectedElements();
        while (dlElement) {
          if ((dlElement->structure.element->type = DL_StripChart))
            updateElementFromGlobalResourceBundle
	      (dlElement->structure.element);
          dlElement = dlElement->next;
        }
      }

      XtPopdown(stripChartS);

      if (currentDisplayInfo->hasBeenEditedButNotSaved == False) 
        dm2kMarkDisplayBeingEdited(currentDisplayInfo);

      break;

    case SC_CLOSE_BTN:
      XtPopdown(stripChartS);
      break;
  }
}

/*
 * function to handle cell selection in the matrix
 *	mostly it passes through for the text field entry
 *	but pops up the color editor for the color field selection
 */
#ifdef __cplusplus
void scEnterCellCallback(Widget, XtPointer, XtPointer cbs)
#else
void scEnterCellCallback(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  int row;
  XbaeMatrixEnterCellCallbackStruct *call_data = 
    (XbaeMatrixEnterCellCallbackStruct *) cbs;

  if (call_data->column == SC_COLOR_COLUMN) {
    /* set this cell non-editable */
    call_data->doit = False;
    /* update the color palette, 
     * set index of the color vector element to set 
     */
    row = call_data->row;
    setCurrentDisplayColorsInColorPalette(SCDATA_RC,row);
    XtPopup(colorS,XtGrabNone);
  }
}


/*
 * function to actually update the colors in the COLOR_COLUMN of the matrix
 */
void scUpdateMatrixColors()
{
  int i;

  /* XmNcolors needs pixel values 
   */
  for (i = 0; i < MAX_PENS; i++) {
    scColorRows[i][SC_COLOR_COLUMN] =
		currentColormap[globalResourceBundle.scData[i].clr];
  }
  
  if (scMatrix != NULL) XtVaSetValues(scMatrix,XmNcolors,scColorCells,NULL);

  /* but for resource editing scData should contain indexes into colormap 
   * this resource is copied, hence this is okay to do 
   */
  for (i = 0; i < MAX_PENS; i++) {
    scColorRows[i][SC_COLOR_COLUMN] = globalResourceBundle.scData[i].clr;
  }

}


/*
 * create strip chart data dialog
 */
Widget createStripChartDataDialog(Widget parent)
{
  Widget shell, applyButton, closeButton;
  Dimension cWidth, cHeight, aWidth, aHeight;
  Arg args[14];
  XmString xmString;
  int i, n;
  static Boolean first = True;
  Pixel foreground;

  XtVaGetValues(parent,XmNforeground,&foreground,NULL);

  /* initialize those file-scoped globals 
   */
  if (first) {
    first = False;
    for (i = 0; i < MAX_PENS; i++) {
      scRows[i][0] = NULL;
      scRows[i][1] = NULL;
      scRows[i][2] = NULL;
      scColorRows[i][SC_CHANNEL_COLUMN] = foreground;
      scColorRows[i][SC_ARCH_CHANNEL_COLUMN] = foreground;
      scColorRows[i][SC_COLOR_COLUMN] = foreground;
      scRows[i][2] = dashes;
      scCells[i] = &scRows[i][0];
      scColorCells[i] = &scColorRows[i][0];
    }
  }


/*
 * now create the interface
 *
 *	       channel | utilChannel | color
 *	       -----------------------------
 *	    1 |  A         B            C  
 *	    2 | 
 *	    3 | 
 *		            ...
 *		        OK     CANCEL
 */

  n = 0;
  XtSetArg(args[n],XmNautoUnmanage,False); n++;
  XtSetArg(args[n],XmNmarginHeight,8); n++;
  XtSetArg(args[n],XmNmarginWidth,8); n++;
  XtSetArg(args[n],XmNdialogStyle,XmDIALOG_PRIMARY_APPLICATION_MODAL); n++;
  scForm = XmCreateFormDialog(parent,"stripChartDataF",args,n);
  shell = XtParent(scForm);
  XmAddWMProtocolCallback(shell,WM_DELETE_WINDOW,
		stripChartActivate,(XtPointer)SC_CLOSE_BTN);
  n = 0;
  XtSetArg(args[n],XmNtitle,"Strip Chart Data"); n++;
  XtSetArg(args[n],XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_RESIZEH); n++;
  XtSetValues(shell,args,n);
  n = 0;
  XtSetArg(args[n],XmNrows,                  MAX_PENS); n++;
  XtSetArg(args[n],XmNcolumns,               3); n++;
  XtSetArg(args[n],XmNcolumnMaxLengths,      scColumnMaxLengths); n++;
  XtSetArg(args[n],XmNcolumnWidths,          scColumnWidths); n++;
  XtSetArg(args[n],XmNcolumnLabels,          scColumnLabels); n++;
  XtSetArg(args[n],XmNcolumnMaxLengths,      scColumnMaxLengths); n++;
  XtSetArg(args[n],XmNcolumnWidths,          scColumnWidths); n++;
  XtSetArg(args[n],XmNcolumnLabelAlignments, scColumnLabelAlignments); n++;
  XtSetArg(args[n],XmNboldLabels,            False); n++;
  scMatrix = XtCreateManagedWidget("scMatrix",
				   xbaeMatrixWidgetClass,scForm,args,n);
  scUpdateMatrixColors();

  XtAddCallback(scMatrix,XmNenterCellCallback,
		scEnterCellCallback,(XtPointer)NULL);

  xmString = XmStringCreateSimple("Close");
  n = 0;
  XtSetArg(args[n],XmNlabelString,xmString); n++;
  closeButton = XmCreatePushButton(scForm,"closeButton",args,n);
  XtAddCallback(closeButton,XmNactivateCallback,
		stripChartActivate,(XtPointer)SC_CLOSE_BTN);
  XtManageChild(closeButton);
  XmStringFree(xmString);

  xmString = XmStringCreateSimple("Apply");
  n = 0;
  XtSetArg(args[n],XmNlabelString,xmString); n++;
  applyButton = XmCreatePushButton(scForm,"applyButton",args,n);
  XtAddCallback(applyButton,XmNactivateCallback,
		stripChartActivate,(XtPointer)SC_APPLY_BTN);
  XtManageChild(applyButton);
  XmStringFree(xmString);

/* make APPLY and CLOSE buttons same size */
  XtVaGetValues(closeButton,XmNwidth,&cWidth,XmNheight,&cHeight,NULL);
  XtVaGetValues(applyButton,XmNwidth,&aWidth,XmNheight,&aHeight,NULL);
  XtVaSetValues(closeButton,XmNwidth,MAX(cWidth,aWidth),
			XmNheight,MAX(cHeight,aHeight),NULL);

/* and make the APPLY button the default for the form */
  XtVaSetValues(scForm,XmNdefaultButton,applyButton,NULL);

/*
 * now do form layout 
 */

/* scMatrix */
  n = 0;
  XtSetArg(args[n],XmNtopAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNleftAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNrightAttachment,XmATTACH_FORM); n++;
  XtSetValues(scMatrix,args,n);
/* apply */
  n = 0;
  XtSetArg(args[n],XmNtopAttachment,XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNtopWidget,scMatrix); n++;
  XtSetArg(args[n],XmNtopOffset,12); n++;
  XtSetArg(args[n],XmNleftAttachment,XmATTACH_POSITION); n++;
  XtSetArg(args[n],XmNleftPosition,20); n++;
  XtSetArg(args[n],XmNbottomAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNbottomOffset,12); n++;
  XtSetValues(applyButton,args,n);
/* close */
  n = 0;
  XtSetArg(args[n],XmNtopAttachment,XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNtopWidget,scMatrix); n++;
  XtSetArg(args[n],XmNtopOffset,12); n++;
  XtSetArg(args[n],XmNrightAttachment,XmATTACH_POSITION); n++;
  XtSetArg(args[n],XmNrightPosition,80); n++;
  XtSetArg(args[n],XmNbottomAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNbottomOffset,12); n++;
  XtSetValues(closeButton,args,n);


  XtManageChild(scForm);

  return shell;
}

/*
 * access function (for file-scoped globals, etc) to udpate the
 *	strip chart data dialog with the values currently in
 *	globalResourceBundle
 */
void updateStripChartDataDialog()
{
  int i;

  for (i = 0; i < MAX_PENS; i++) {
    scRows[i][SC_CHANNEL_COLUMN] = globalResourceBundle.scData[i].chan;
    scRows[i][SC_ARCH_CHANNEL_COLUMN] = globalResourceBundle.scData[i].utilChan;
    scRows[i][SC_COLOR_COLUMN] =  dashes;
  }

  /* handle clr in here */
  scUpdateMatrixColors();
  if (scMatrix != NULL) XtVaSetValues(scMatrix,XmNcells,scCells,NULL);
  
}




/*****************
 * for the Cartesian Plot Axis Dialog...
 *****************/

void createCartesianPlotAxisDialogMenuEntry(
     Widget         parentRC,
     XmString       axisLabelXmString,
     Widget       * thelabel,
     Widget       * themenu,
     XmString     * menuLabelXmStrings,
     XmButtonType * buttonType,
     int            numberOfLabels,
     XtPointer      clientData) 
{
  Arg    args[10];
  int    n = 0;
  Widget rowColumn;
  Widget label, menu;

  /* create rowColumn widget to hold the label and menu widgets */
  XtSetArg(args[n],XmNorientation,XmVERTICAL); n++;
  XtSetArg(args[n],XmNpacking,XmPACK_NONE); n++;
  rowColumn = XmCreateRowColumn(parentRC,"entryRC",args,n);
 
  /* create the label widget */
  n = 0;
  XtSetArg(args[n],XmNalignment,XmALIGNMENT_END); n++;
  XtSetArg(args[n],XmNlabelString,axisLabelXmString); n++;
  XtSetArg(args[n],XmNrecomputeSize,False); n++;
  label = XmCreateLabel(rowColumn,"localLabel",args,n);
 
  /* create the text widget */
  n = 0;
  XtSetArg(args[n],XmNbuttonType,buttonType); n++;
  XtSetArg(args[n],XmNbuttons,menuLabelXmStrings); n++;
  XtSetArg(args[n],XmNbuttonCount,numberOfLabels); n++;
  XtSetArg(args[n],XmNsimpleCallback,cpAxisOptionMenuSimpleCallback); n++;
  XtSetArg(args[n],XmNuserData,clientData); n++;
  menu = XmCreateSimpleOptionMenu(rowColumn,"localElement",args,n);
  XtUnmanageChild(XmOptionLabelGadget(menu));
  XtManageChild(rowColumn);
 
  if (thelabel) *thelabel = label;
  if (themenu)  *themenu = menu;
}

void createCartesianPlotAxisDialogTextEntry(
     Widget parentRC,
     XmString axisLabelXmString,
     Widget *rowColumn,
     Widget *label,
     Widget *text,
     XtPointer clientData)
{
  Arg args[10];
  int n = 0;
  /* create a row column widget to hold the label and textfield widget */
  XtSetArg(args[n],XmNorientation,XmVERTICAL); n++;
  XtSetArg(args[n],XmNpacking,XmPACK_NONE); n++;
  *rowColumn = XmCreateRowColumn(parentRC,"entryRC",args,n);
 
  /* create the label */
  n = 0;
  XtSetArg(args[n],XmNalignment,XmALIGNMENT_END); n++;
  XtSetArg(args[n],XmNlabelString,axisLabelXmString); n++;
  XtSetArg(args[n],XmNrecomputeSize,False); n++;
  *label = XmCreateLabel(*rowColumn,"localLabel",args,n);
 
  /* create the text field */
  n = 0;
  XtSetArg(args[n],XmNmaxLength,MAX_TOKEN_LENGTH-1); n++;
  *text = XmCreateTextField(*rowColumn,"localElement",args,n);
  XtAddCallback(*text,XmNactivateCallback,cpAxisTextFieldActivateCallback,
                clientData);
  XtAddCallback(*text,XmNlosingFocusCallback,cpAxisTextFieldLosingFocusCallback,
                clientData);
  XtAddCallback(*text,XmNmodifyVerifyCallback, textFieldFloatVerifyCallback,
                NULL);
  XtManageChild(*rowColumn);
}
     
/*
 * create axis dialog
 */
#ifdef __cplusplus
Widget createCartesianPlotAxisDialog(Widget)
#else
Widget createCartesianPlotAxisDialog(Widget parent)
#endif
{
  Widget         shell;
  Widget         closeButton;
  Arg            args[12];
  int            counter;
  XmString       xmString;
  XmString       axisStyleXmString;
  XmString       axisRangeXmString;
  XmString       axisMinXmString;
  XmString       axisMaxXmString;
  XmString       axisTimeFmtXmString;
  XmString       frameLabelXmString;
  int            i, n;
  XmButtonType   buttonType[MAX_CP_AXIS_BUTTONS];
  Widget         frame;
  Widget         localLabel;
  Widget         parentRC;
  Widget         entryLabel[MAX_CP_AXIS_ELEMENTS];   /* for keeping list */
  Widget         entryElement[MAX_CP_AXIS_ELEMENTS]; /* of widgets around */
  Dimension      width;
  Dimension      height;

  /* indexed like dlCartesianPlot->axis[]: X_ELEMENT_AXIS, Y1_ELEMENT_AXIS... 
   */
  static char *frameLabelString[3] = {"X Axis", "Y1 Axis", "Y2 Axis",};



  /* initialize XmString value tables (since this can be edit or execute time)
   */
  initializeXmStringValueTables();

  for (i = 0; i < MAX_CP_AXIS_BUTTONS; i++) 
    buttonType[i] = XmPUSHBUTTON;


  n = 0;
  XtSetArg(args[n],XmNdeleteResponse, XmDO_NOTHING); n++;
  XtSetArg(args[n],XmNautoUnmanage,   False); n++;
  XtSetArg(args[n],XmNtitle,          "Cartesian Plot Axis Data"); n++;
  shell = XtCreatePopupShell("cartesianPlotAxisS",
			     topLevelShellWidgetClass,mainShell,args,n);
  XmAddWMProtocolCallback(shell,WM_DELETE_WINDOW,
			  cartesianPlotAxisActivate,
			  (XtPointer)CP_CLOSE_BTN);

  n = 0;
  XtSetArg(args[n],XmNautoUnmanage, False); n++;
  XtSetArg(args[n],XmNmarginHeight, 8); n++;
  XtSetArg(args[n],XmNmarginWidth,  8); n++;
  XtSetArg(args[n],XmNdialogStyle,  XmDIALOG_PRIMARY_APPLICATION_MODAL); n++;
  cpAxisForm = XmCreateForm(shell,"cartesianPlotAxisF",args,n);

  axisStyleXmString   = XmStringCreateSimple("Axis Style");
  axisRangeXmString   = XmStringCreateSimple("Axis Range");
  axisMinXmString     = XmStringCreateSimple("Minimum Value");
  axisMaxXmString     = XmStringCreateSimple("Maximum Value");
  axisTimeFmtXmString = XmStringCreateSimple("Time format");

  counter = 0;

  for (i = X_AXIS_ELEMENT; i <= Y2_AXIS_ELEMENT; i++) 
    {
      n = 0;

      if (i == X_AXIS_ELEMENT) {
	XtSetArg(args[n],XmNtopAttachment,   XmATTACH_FORM); n++;
      } else {
	/* frame widget will be used in first loop!!! */
	XtSetArg(args[n],XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n],XmNtopWidget,     frame); n++;
      }
	
      XtSetArg(args[n],XmNleftAttachment,  XmATTACH_FORM); n++;
      XtSetArg(args[n],XmNrightAttachment, XmATTACH_FORM); n++;
      XtSetArg(args[n],XmNshadowType,      XmSHADOW_ETCHED_IN); n++;
      frame = XmCreateFrame(cpAxisForm,"frame",args,n);
      XtManageChild(frame);
      
      frameLabelXmString = XmStringCreateSimple(frameLabelString[i]);

      n = 0;
      XtSetArg(args[n],XmNlabelString,  frameLabelXmString); n++;
      XtSetArg(args[n],XmNmarginWidth,  0); n++;
      XtSetArg(args[n],XmNmarginHeight, 0); n++;
      XtSetArg(args[n],XmNchildType,    XmFRAME_TITLE_CHILD); n++;
      
      localLabel = XmCreateLabel(frame,"label",args,n);

      XtManageChild(localLabel);
      XmStringFree(frameLabelXmString);


      /* parent RC within frame 
       */
      n = 0;
      XtSetArg(args[n],XmNnumColumns,   1); n++;
      XtSetArg(args[n],XmNorientation,  XmVERTICAL); n++;
      XtSetArg(args[n],XmNpacking,      XmPACK_COLUMN); n++;
      parentRC = XmCreateRowColumn(frame,"parentRC",args,n);
      XtManageChild(parentRC);


      /* create Axis Style Entry 
       */
      createCartesianPlotAxisDialogMenuEntry
	(parentRC,
	 axisStyleXmString,
	 &(entryLabel[counter]),
	 &(entryElement[counter]),
	 &(xmStringValueTable[FIRST_CARTESIAN_PLOT_AXIS_STYLE]),
	 buttonType,
	 (!i)?NUM_CARTESIAN_PLOT_AXIS_STYLES:NUM_CARTESIAN_PLOT_AXIS_STYLES-1,
	 (XtPointer)(CP_X_AXIS_STYLE+i));

      axisStyleMenu[i] =  entryElement[counter];
      counter++;
      

      /* create Range Style Entry 
       */
      createCartesianPlotAxisDialogMenuEntry
	(parentRC,
	 axisRangeXmString,
	 &(entryLabel[counter]),
	 &(entryElement[counter]),
	 &(xmStringValueTable[FIRST_CARTESIAN_PLOT_RANGE_STYLE]),
	 buttonType,
	 NUM_CARTESIAN_PLOT_RANGE_STYLES,
	 (XtPointer)(CP_X_RANGE_STYLE+i));

      axisRangeMenu[i] =  entryElement[counter];
      counter++;


      /* create Min text field entry 
       */
      createCartesianPlotAxisDialogTextEntry
	(parentRC, 
	 axisMinXmString,
	 &(axisRangeMinRC[i]), 
	 &(entryLabel[counter]),
	 &(entryElement[counter]),
	 (XtPointer)(CP_X_RANGE_MIN+i));

      axisRangeMin[i] = entryElement[counter];
      counter++;
 

      /* create Max text field entry 
       */
      createCartesianPlotAxisDialogTextEntry
	(parentRC, 
	 axisMaxXmString,
	 &(axisRangeMaxRC[i]),
	 &(entryLabel[counter]),
	 &(entryElement[counter]),
	 (XtPointer)(CP_X_RANGE_MAX+i));

      axisRangeMax[i] = entryElement[counter];
      counter++;


      if (i == X_AXIS_ELEMENT) 
	{
	  /* create time format menu entry 
	   */
	  createCartesianPlotAxisDialogMenuEntry
	    (parentRC,
	     axisTimeFmtXmString,
	     &(entryLabel[counter]),
	     &(entryElement[counter]),
	     &(xmStringValueTable[FIRST_CP_TIME_FORMAT]),
	     buttonType,
	     NUM_CP_TIME_FORMATS,
	     (XtPointer)(CP_X_TIME_FORMAT));

	  axisTimeFormat =  entryElement[counter];
	  counter++;
	}
    }

  for (i = 0; i < counter; i++) 
    {
      XtVaGetValues(entryLabel[i],XmNwidth,&width,XmNheight,&height,NULL);
      maxLabelWidth = MAX(maxLabelWidth,width);
      maxLabelHeight = MAX(maxLabelHeight,height);
      XtVaGetValues(entryElement[i],XmNwidth,&width,XmNheight,&height,NULL);
      maxLabelWidth = MAX(maxLabelWidth,width);
      maxLabelHeight = MAX(maxLabelHeight,height);
      XtManageChild(entryLabel[i]);
      XtManageChild(entryElement[i]);
    }

  /* now resize the labels and elements (to maximum's width)
   * for uniform appearance
   */
  for (i = 0; i < counter; i++) 
    {
      /* set label */
      XtVaSetValues(entryLabel[i],
		    XmNwidth,         maxLabelWidth,
		    XmNheight,        maxLabelHeight,
		    XmNrecomputeSize, False,
		    XmNalignment,     XmALIGNMENT_END,
		    NULL);

      /* set element */
      if (XtClass(entryElement[i]) == xmRowColumnWidgetClass) {
        /* must be option menu - unmanage label widget 
	 */
	XtVaSetValues(XmOptionButtonGadget(entryElement[i]),
		      XmNx,             (Position)maxLabelWidth, 
		      XmNwidth,         maxLabelWidth,
		      XmNheight,        maxLabelHeight,
		      XmNrecomputeSize, False,
		      XmNresizeWidth,   True,
		      XmNmarginWidth,   0,
		      NULL);
      }

      XtVaSetValues(entryElement[i],
		    XmNx,             (Position)maxLabelWidth,
		    XmNwidth,         maxLabelWidth,
		    XmNheight,        maxLabelHeight,
		    XmNrecomputeSize, False, 
		    XmNresizeWidth,   True,
		    XmNmarginWidth,   0,
		    NULL);
      XtManageChild(entryLabel[i]);
      XtManageChild(entryElement[i]);
    }

  /* free XmStrings
   */
  XmStringFree(axisStyleXmString);
  XmStringFree(axisRangeXmString);
  XmStringFree(axisMinXmString);
  XmStringFree(axisMaxXmString);

  /* create Plot Style Menu
   */
  {
    XmString plotStyleXmString =  XmStringCreateSimple("Plot Style");
    Widget   plotStyleLabel;
    Widget   plotStyleMenu;
    XmString plotStyleItems[2];

    n = 0;
    XtSetArg(args[n],XmNtopAttachment,   XmATTACH_WIDGET); n++;
    XtSetArg(args[n],XmNtopWidget,       frame); n++;
    XtSetArg(args[n],XmNleftAttachment,  XmATTACH_FORM); n++;
    XtSetArg(args[n],XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n],XmNshadowType,      XmSHADOW_ETCHED_IN); n++;
    frame = XmCreateFrame(cpAxisForm,"frame",args,n);

    frameLabelXmString = XmStringCreateSimple("General setup");
    n = 0;
    XtSetArg(args[n],XmNlabelString,     frameLabelXmString); n++;
    XtSetArg(args[n],XmNmarginWidth,     0); n++;
    XtSetArg(args[n],XmNmarginHeight,    0); n++;
    XtSetArg(args[n],XmNchildType,       XmFRAME_TITLE_CHILD); n++;
    localLabel = XmCreateLabel(frame,"label",args,n);
    XmStringFree(frameLabelXmString);

    /* parent RC within frame 
     */
    n = 0;
    XtSetArg(args[n],XmNnumColumns,      1); n++;
    XtSetArg(args[n],XmNorientation,     XmVERTICAL); n++;
    XtSetArg(args[n],XmNpacking,         XmPACK_COLUMN); n++;
    parentRC = XmCreateRowColumn(frame,"parentRC",args,n);

    plotStyleItems[0] = XmStringCreateSimple("plot");
    plotStyleItems[1] = XmStringCreateSimple("bar");
    
    /* create Plot Style Entry 
     */
    createCartesianPlotAxisDialogMenuEntry
      (parentRC,
       plotStyleXmString,
       &(plotStyleLabel),
       &(plotStyleMenu),
       plotStyleItems,
       buttonType,
       2,
       (XtPointer)(CP_PLOT_STYLE));

    XmStringFree(plotStyleXmString);

    /* set label */
    XtVaSetValues(plotStyleLabel,
		  XmNwidth,         maxLabelWidth,
		  XmNheight,        maxLabelHeight,
		  XmNrecomputeSize, False,
		  XmNalignment,     XmALIGNMENT_END,
		  NULL);

    /* set element */
    if (XtClass(plotStyleMenu) == xmRowColumnWidgetClass) {
      /* must be option menu - unmanage label widget */
      XtVaSetValues(XmOptionButtonGadget(plotStyleMenu),
		    XmNx,             (Position)maxLabelWidth, 
		    XmNwidth,         maxLabelWidth,
		    XmNheight,        maxLabelHeight,
		    XmNrecomputeSize, False, 
		    XmNresizeWidth,   True,
		    XmNmarginWidth,   0,
		    NULL);
    }

    XtVaSetValues(plotStyleMenu,
		  XmNx,             (Position)maxLabelWidth, 
		  XmNwidth,         maxLabelWidth,
		  XmNheight,        maxLabelHeight,
		  XmNrecomputeSize, False, 
		  XmNresizeWidth,   True,
		  XmNmarginWidth,   0,
		  NULL);

    XtManageChild(plotStyleLabel);
    XtManageChild(plotStyleMenu);
    XtManageChild(localLabel);
    XtManageChild(parentRC);
    XtManageChild(frame);
  }

  xmString = XmStringCreateSimple("Close");
  n = 0;
  XtSetArg(args[n],XmNtopAttachment,    XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNleftAttachment,   XmATTACH_POSITION); n++;
  XtSetArg(args[n],XmNrightAttachment,  XmATTACH_POSITION); n++;
  XtSetArg(args[n],XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNtopWidget,        frame); n++;
  XtSetArg(args[n],XmNtopOffset,        12); n++;
  XtSetArg(args[n],XmNleftPosition,     (Position)20); n++;
  XtSetArg(args[n],XmNrightPosition,    (Position)80); n++;
  XtSetArg(args[n],XmNbottomOffset,     12); n++;
  XtSetArg(args[n],XmNlabelString,      xmString); n++;

  closeButton = XmCreatePushButton(cpAxisForm,"closeButton",args,n);
  XtAddCallback(closeButton,XmNactivateCallback,
		cartesianPlotAxisActivate, (XtPointer)CP_CLOSE_BTN);
  XtManageChild(closeButton);
  XmStringFree(xmString);

  return (shell);
}


/*
 * access function (for file-scoped globals, etc) to udpate the
 *	cartesian plot axis dialog with the values currently in
 *	globalResourceBundle
 */
void updateCartesianPlotAxisDialog()
{
  int i, tail;
  char string[MAX_TOKEN_LENGTH];

  for (i = X_AXIS_ELEMENT; i <= Y2_AXIS_ELEMENT; i++) {
    optionMenuSet(axisStyleMenu[i], globalResourceBundle.axis[i].axisStyle
		- FIRST_CARTESIAN_PLOT_AXIS_STYLE);
    optionMenuSet(axisRangeMenu[i], globalResourceBundle.axis[i].rangeStyle
		- FIRST_CARTESIAN_PLOT_RANGE_STYLE);
    if (globalResourceBundle.axis[i].rangeStyle == USER_SPECIFIED_RANGE) {
      sprintf(string,"%f",globalResourceBundle.axis[i].minRange);
  /* strip trailing zeroes */
      tail = STRLEN(string);
      while (string[--tail] == '0') string[tail] = '\0';
      XmTextFieldSetString(axisRangeMin[i],string);
      sprintf(string,"%f",globalResourceBundle.axis[i].maxRange);
  /* strip trailing zeroes */
      tail = STRLEN(string);
      while (string[--tail] == '0') string[tail] = '\0';
      XmTextFieldSetString(axisRangeMax[i],string);
      XtSetSensitive(axisRangeMinRC[i],True);
      XtSetSensitive(axisRangeMaxRC[i],True);
    } else {
      XtSetSensitive(axisRangeMinRC[i],False);
      XtSetSensitive(axisRangeMaxRC[i],False);
    }
  }
  if (globalResourceBundle.axis[0].axisStyle == TIME_AXIS) {
    XtSetSensitive(axisTimeFormat,True);
    optionMenuSet(axisTimeFormat,globalResourceBundle.axis[0].timeFormat
        - FIRST_CP_TIME_FORMAT);
  } else {
    XtSetSensitive(axisTimeFormat,False);
  }
}

/*
 * access function (for file-scoped globals, etc) to udpate the
 *      cartesian plot axis dialog with the values currently in
 *      the subject cartesian plot
 */
void updateCartesianPlotAxisDialogFromWidget(Widget cp)
{
#ifdef USE_XRT
  int tail, buttonId;
  char string[MAX_TOKEN_LENGTH];
  XtPointer userData;
  Boolean xAxisIsLog, y1AxisIsLog, y2AxisIsLog,
        xMinUseDef, y1MinUseDef, y2MinUseDef,
        xIsCurrentlyFromChannel, y1IsCurrentlyFromChannel,
        y2IsCurrentlyFromChannel;
  XrtAnnoMethod xAnnoMethod;
  XcVType xMinF, xMaxF, y1MinF, y1MaxF, y2MinF, y2MaxF;

  if (globalDisplayListTraversalMode != DL_EXECUTE) return;
  XtVaGetValues(cp,
          XmNuserData,&userData,
          XtNxrtXAnnotationMethod, &xAnnoMethod,
          XtNxrtXAxisLogarithmic,&xAxisIsLog,
          XtNxrtYAxisLogarithmic,&y1AxisIsLog,
          XtNxrtY2AxisLogarithmic,&y2AxisIsLog,
          XtNxrtXMin,&xMinF.lval,
          XtNxrtYMin,&y1MinF.lval,
          XtNxrtY2Min,&y2MinF.lval,
          XtNxrtXMax,&xMaxF.lval,
          XtNxrtYMax,&y1MaxF.lval,
          XtNxrtY2Max,&y2MaxF.lval,
          XtNxrtXMinUseDefault,&xMinUseDef,
          XtNxrtYMinUseDefault,&y1MinUseDef,
          XtNxrtY2MinUseDefault,&y2MinUseDef,
          NULL);

  if (userData != NULL) {
    WidgetUserData * wud = (WidgetUserData *) userData;
    CartesianPlot  * pcp = (CartesianPlot *) wud->privateData;

    if (pcp != NULL) {
      xIsCurrentlyFromChannel =
        pcp->axisRange[X_AXIS_ELEMENT].isCurrentlyFromChannel;
      y1IsCurrentlyFromChannel =
        pcp->axisRange[Y1_AXIS_ELEMENT].isCurrentlyFromChannel;
      y2IsCurrentlyFromChannel =
        pcp->axisRange[Y2_AXIS_ELEMENT].isCurrentlyFromChannel;
    }
  }

  /* X Axis */
  if (xAnnoMethod == XRT_ANNO_TIME_LABELS)  {
    optionMenuSet(axisStyleMenu[X_AXIS_ELEMENT],
                  TIME_AXIS - FIRST_CARTESIAN_PLOT_AXIS_STYLE);
    optionMenuSet(axisRangeMenu[X_AXIS_ELEMENT],
                  AUTO_SCALE_RANGE - FIRST_CARTESIAN_PLOT_AXIS_STYLE);
    XtSetSensitive(axisTimeFormat,True);
  } else {
    optionMenuSet(axisStyleMenu[X_AXIS_ELEMENT],
        (xAxisIsLog ? LOG10_AXIS : LINEAR_AXIS)
                - FIRST_CARTESIAN_PLOT_AXIS_STYLE);
    buttonId = (xMinUseDef ? AUTO_SCALE_RANGE :
        (xIsCurrentlyFromChannel ? CHANNEL_RANGE : USER_SPECIFIED_RANGE)
                - FIRST_CARTESIAN_PLOT_RANGE_STYLE);
    optionMenuSet(axisRangeMenu[X_AXIS_ELEMENT], buttonId);
    if (buttonId == USER_SPECIFIED_RANGE - FIRST_CARTESIAN_PLOT_RANGE_STYLE) {
      sprintf(string,"%f",xMinF.fval);
      /* strip trailing zeroes */
      tail = STRLEN(string);
      while (string[--tail] == '0') string[tail] = '\0';
      XmTextFieldSetString(axisRangeMin[X_AXIS_ELEMENT],string);
      sprintf(string,"%f",xMaxF.fval);
      /* strip trailing zeroes */
      tail = STRLEN(string);
      while (string[--tail] == '0') string[tail] = '\0';
      XmTextFieldSetString(axisRangeMax[X_AXIS_ELEMENT],string);
    }
    XtSetSensitive(axisTimeFormat,False);
  }
  if ((!xMinUseDef && !xIsCurrentlyFromChannel) ||
      (xAnnoMethod == XRT_ANNO_TIME_LABELS)) {
    XtSetSensitive(axisRangeMinRC[X_AXIS_ELEMENT],True);
    XtSetSensitive(axisRangeMaxRC[X_AXIS_ELEMENT],True);
  } else {
    XtSetSensitive(axisRangeMinRC[X_AXIS_ELEMENT],False);
    XtSetSensitive(axisRangeMaxRC[X_AXIS_ELEMENT],False);
  }


/* Y1 Axis */
    optionMenuSet(axisStyleMenu[Y1_AXIS_ELEMENT],
        (y1AxisIsLog ? LOG10_AXIS : LINEAR_AXIS)
                - FIRST_CARTESIAN_PLOT_AXIS_STYLE);
    buttonId =  (y1MinUseDef ? AUTO_SCALE_RANGE :
        (y1IsCurrentlyFromChannel ? CHANNEL_RANGE : USER_SPECIFIED_RANGE)
                - FIRST_CARTESIAN_PLOT_RANGE_STYLE);
    optionMenuSet(axisRangeMenu[Y1_AXIS_ELEMENT], buttonId);
    if (buttonId == USER_SPECIFIED_RANGE - FIRST_CARTESIAN_PLOT_RANGE_STYLE) {
      sprintf(string,"%f",y1MinF.fval);
  /* strip trailing zeroes */
      tail = STRLEN(string);
      while (string[--tail] == '0') string[tail] = '\0';
      XmTextFieldSetString(axisRangeMin[Y1_AXIS_ELEMENT],string);
      sprintf(string,"%f",y1MaxF.fval);
  /* strip trailing zeroes */
      tail = STRLEN(string);
      while (string[--tail] == '0') string[tail] = '\0';
      XmTextFieldSetString(axisRangeMax[Y1_AXIS_ELEMENT],string);
    }
    if (!y1MinUseDef && !y1IsCurrentlyFromChannel) {
      XtSetSensitive(axisRangeMinRC[Y1_AXIS_ELEMENT],True);
      XtSetSensitive(axisRangeMaxRC[Y1_AXIS_ELEMENT],True);
    } else {
      XtSetSensitive(axisRangeMinRC[Y1_AXIS_ELEMENT],False);
      XtSetSensitive(axisRangeMaxRC[Y1_AXIS_ELEMENT],False);
    }


/* Y2 Axis */
    optionMenuSet(axisStyleMenu[Y2_AXIS_ELEMENT],
                (y1AxisIsLog ? LOG10_AXIS : LINEAR_AXIS)
                - FIRST_CARTESIAN_PLOT_AXIS_STYLE);
    buttonId = (y2MinUseDef ? AUTO_SCALE_RANGE :
        (y2IsCurrentlyFromChannel ? CHANNEL_RANGE : USER_SPECIFIED_RANGE)
                - FIRST_CARTESIAN_PLOT_RANGE_STYLE);
    optionMenuSet(axisRangeMenu[Y2_AXIS_ELEMENT], buttonId);
    if (buttonId == USER_SPECIFIED_RANGE - FIRST_CARTESIAN_PLOT_RANGE_STYLE) {
      sprintf(string,"%f",y2MinF.fval);
  /* strip trailing zeroes */
      tail = STRLEN(string);
      while (string[--tail] == '0') string[tail] = '\0';
      XmTextFieldSetString(axisRangeMin[Y2_AXIS_ELEMENT],string);
      sprintf(string,"%f",y2MaxF.fval);
  /* strip trailing zeroes */
      tail = STRLEN(string);
      while (string[--tail] == '0') string[tail] = '\0';
      XmTextFieldSetString(axisRangeMax[Y2_AXIS_ELEMENT],string);
    }
    if (!y2MinUseDef && !y2IsCurrentlyFromChannel) {
      XtSetSensitive(axisRangeMinRC[Y2_AXIS_ELEMENT],True);
      XtSetSensitive(axisRangeMaxRC[Y2_AXIS_ELEMENT],True);
    } else {
      XtSetSensitive(axisRangeMinRC[Y2_AXIS_ELEMENT],False);
      XtSetSensitive(axisRangeMaxRC[Y2_AXIS_ELEMENT],False);
    }
#endif
}


void dm2kGetValues(ResourceBundle *pRB, ...) 
{ 
#if defined(va_dcl)
  va_dcl
#endif
  va_list ap;
  int arg;
  va_start(ap, pRB);
  arg = va_arg(ap,int);

/* ------------------------------------- */
#define GET_VALUE(TYPE,FIELD)             \
        TYPE pvalue = va_arg(ap,TYPE);    \
	(*pvalue) = pRB->FIELD            \
/* -------------------------------------- */

/* ---------------------------------------- */
#define GET_STRING_VALUE(FIELD)              \
        char ** pvalue = va_arg(ap,char **); \
        renewString(pvalue,pRB->FIELD)       \
/* ---------------------------------------- */

  while (arg >= 0) 
  {
    switch (arg) 
    {
      case X_RC              : { GET_VALUE(int *         , x);            } break;
      case Y_RC              : { GET_VALUE(int *         , y);            } break;
      case WIDTH_RC          : { GET_VALUE(unsigned int *, width);        } break;
      case HEIGHT_RC         : { GET_VALUE(unsigned int *, height);       } break;
      case CLR_RC            : { GET_VALUE(int *         , clr);          } break;
      case BCLR_RC           : { GET_VALUE(int *         , bclr);         } break;
      case BEGIN_RC          : { GET_VALUE(int *         , begin);        } break;
      case PATH_RC           : { GET_VALUE(int *         , path);         } break;
      case ALIGNMENT_RC      : { GET_VALUE(Alignment *   , alignment);    } break;
      case FORMAT_RC         : { GET_VALUE(TextFormat *  , format);       } break;
      case LABEL_RC          : { GET_VALUE(LabelType *   , label);        } break;
      case DIRECTION_RC      : { GET_VALUE(Direction *   , direction);    } break;
      case FILLMOD_RC        : { GET_VALUE(FillMode *    , fillmod);      } break;
      case FIT_RC            : { GET_VALUE(int *         , fit);          } break;
      case STYLE_RC          : { GET_VALUE(EdgeStyle *   , style);        } break;
      case FILL_RC           : { GET_VALUE(FillStyle *   , fill);         } break;
      case CLRMOD_RC         : { GET_VALUE(ColorMode *   , clrmod);       } break;
      case ERASE_OLDEST_RC   : { GET_VALUE(EraseOldest * , erase_oldest); } break;
      case DATA_CLR_RC       : { GET_VALUE(int *         , data_clr);     } break;
      case DIS_RC            : { GET_VALUE(int *         , dis);          } break;
      case XYANGLE_RC        : { GET_VALUE(int *         , xyangle);      } break;
      case ZANGLE_RC         : { GET_VALUE(int *         , zangle);       } break;
      case PERIOD_RC         : { GET_VALUE(double *      , period);       } break;
      case UNITS_RC          : { GET_VALUE(TimeUnits *   , units);        } break;
      case COUNT_RC          : { GET_VALUE(int *         , count);        } break;
      case STACKING_RC       : { GET_VALUE(Stacking *    , stacking);     } break;
      case IMAGETYPE_RC      : { GET_VALUE(ImageType *   , imageType);    } break;
      case LINEWIDTH_RC      : { GET_VALUE(int *         , lineWidth);    } break;
      case PRECISION_RC      : { GET_VALUE(double *      , dPrecision);   } break;
      case SBIT_RC           : { GET_VALUE(int *         , sbit);         } break;
      case EBIT_RC           : { GET_VALUE(int *         , ebit);         } break;
      case ACTIVE_COLOR_RC   : { GET_VALUE(int *         , abclr);        } break;
      case DISPLAY_TYPE_RC   : { GET_VALUE(DisplayType * , displayType);  } break;
      case ERASE_MODE_RC     : { GET_VALUE(eraseMode_t * , eraseMode);    } break;


      case SCALE_TYPE_RC     : { GET_VALUE(ScaleType*    , scaleType);            } break;
      case BAR_ONLY_RC       : { GET_VALUE(ShowBar*      , barOnly);              } break;
      case SHOW_ALARM_LIMIT_RC:{ GET_VALUE(ShowAlarmLimits*, showAlarmLimits);    } break;
      case SHOW_SCALE_RC     : { GET_VALUE(ShowScale*    , showScale);    } break;
      case LOW_LIMIT_RC      : { GET_VALUE(double *      , dispayLowLimit);       } break;
      case HIGH_LIMIT_RC     : { GET_VALUE(double *      , dispayHighLimit);      } break;
      case VAL_PRECISION_RC  : { GET_VALUE(int *         , valPrecision);      } break;

      case COLOR_RULE_RC     : { GET_VALUE(ColorRule **  , colorRule);    } break;
      case GRAPHIC_RULE_RC   : { GET_VALUE(GraphicRule **, graphicRule);    } break;
      case VIS_RC            : { GET_VALUE(VisibilityMode *        , vis);               } break;
      case CSTYLE_RC         : { GET_VALUE(CartesianPlotStyle *    , cStyle);            } break;
      case ASSOM_RC          : { GET_VALUE(AssociatedMenuItem **   , ami);               } break;
      case RD_VISUAL_RC      : { GET_VALUE(relatedDisplayVisual_t *, rdVisual);          } break;
      case SENSITIVE_MODE_RC : { GET_VALUE(SensitivityMode *       , sensitive_mode);    } break;
      case BUTTON_TYPE_RC    : { GET_VALUE(MessageButtonType *     , messageButtonType); } break;


      case RD_LABEL_RC       : { GET_STRING_VALUE(rdLabel);        } break;
      case RDBK_RC           : { GET_STRING_VALUE(chan);           } break;
      case CTRL_RC           : { GET_STRING_VALUE(chan);           } break;
      case CHAN_RC           : { GET_STRING_VALUE(chan);           } break;
      case TITLE_RC          : { GET_STRING_VALUE(title);          } break;
      case XLABEL_RC         : { GET_STRING_VALUE(xlabel);         } break;
      case YLABEL_RC         : { GET_STRING_VALUE(ylabel);         } break;
      case TEXTIX_RC         : { GET_STRING_VALUE(textix);         } break;
      case MSG_LABEL_RC      : { GET_STRING_VALUE(messageLabel);   } break;
      case PRESS_MSG_RC      : { GET_STRING_VALUE(press_msg);      } break;
      case RELEASE_MSG_RC    : { GET_STRING_VALUE(release_msg);    } break;
      case IMAGENAME_RC      : { GET_STRING_VALUE(imageName);      } break;
      case COMPOSITE_NAME_RC   : { GET_STRING_VALUE(compositeName);  } break;
      case DATA_RC           : { GET_STRING_VALUE(data);           } break;
      case CMAP_RC           : { GET_STRING_VALUE(cmap);           } break;
      case NAME_RC           : { GET_STRING_VALUE(name);           } break;
      case TRIGGER_RC        : { GET_STRING_VALUE(trigger);        } break;
      case ERASE_RC          : { GET_STRING_VALUE(erase);          } break;
      case SENSITIVE_CHAN_RC : { GET_STRING_VALUE(sensitive_chan); } break;
      case ACTIVE_LABEL_RC   : { GET_STRING_VALUE(toggleOnLabel);  } break;

      case RDDATA_RC : {
        DlRelatedDisplayEntry *pDisplay = va_arg(ap,DlRelatedDisplayEntry *);
        int i;
        for (i = 0; i < MAX_RELATED_DISPLAYS; i++){
          renewString(&pDisplay[i].label,globalResourceBundle.rdData[i].label);
          renewString(&pDisplay[i].name,globalResourceBundle.rdData[i].name);
          renewString(&pDisplay[i].args,globalResourceBundle.rdData[i].args);
          pDisplay[i].mode = globalResourceBundle.rdData[i].mode;
        }
        break;
      }
      case CPDATA_RC : {
        DlTrace* ptrace = va_arg(ap,DlTrace *);
        int i;
        for (i = 0; i < MAX_TRACES; i++){
          renewString(&ptrace[i].xdata,globalResourceBundle.cpData[i].xdata);
          renewString(&ptrace[i].ydata,globalResourceBundle.cpData[i].ydata);
          ptrace[i].data_clr = globalResourceBundle.cpData[i].data_clr;
        }
        break;
      }
      case SCDATA_RC : {
        DlPen *pPen = va_arg(ap, DlPen *);
        int i;
        for (i = 0; i < MAX_PENS; i++){
          renewString(&pPen[i].chan,pRB->scData[i].chan);
          renewString(&pPen[i].utilChan,pRB->scData[i].utilChan);
          pPen[i].clr = pRB->scData[i].clr;
        }
        break;
      }
      case SHELLDATA_RC : {
        DlShellCommandEntry *pCommand = va_arg(ap, DlShellCommandEntry *);
        int i;
        for (i = 0; i < MAX_SHELL_COMMANDS; i++){
          renewString(&pCommand[i].label,globalResourceBundle.cmdData[i].label);
          renewString(&pCommand[i].command,globalResourceBundle.cmdData[i].command);
          renewString(&pCommand[i].args,globalResourceBundle.cmdData[i].args);
        }
        break;
      }
      case CPAXIS_RC : {
        DlPlotAxisDefinition *paxis = va_arg(ap,DlPlotAxisDefinition *);
        paxis[X_AXIS_ELEMENT] = globalResourceBundle.axis[X_AXIS_ELEMENT];
        paxis[Y1_AXIS_ELEMENT] = globalResourceBundle.axis[Y1_AXIS_ELEMENT];
        paxis[Y2_AXIS_ELEMENT] = globalResourceBundle.axis[Y2_AXIS_ELEMENT];
        break;
      }

      default :
        break;
    }

    arg = va_arg(ap,int);
  }

  va_end(ap);
  return;
}
