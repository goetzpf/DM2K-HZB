/******************************************************************
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
 *                              - using new screen update dispatch mechanism
 * .03  09-11-95        vong    conform to c++ syntax
 *
 *****************************************************************************
 *
 *      18-03-97        Fabien  Add object info and enter leave window
 *      13-10-97        Thomas  Add XRT/graph 3.0 compatibility
 *
 *****************************************************************************
*/

/*
 * TODO:
 *   - make use of XRT_POINT_* configurable
 */

#include "dm2k.h"
#define DM2K_CARTPLOT_EXCL_GLOBALS
#include "dm2kCartesianPlot.h"
#undef DM2K_CARTPLOT_EXCL_GLOBALS
#include <time.h>

#ifdef __cplusplus     
extern "C" time_t mktime (struct tm *);
#else
extern time_t mktime (struct tm *);
#endif

#ifdef USE_XRT
static void cartesianPlotCreateEditInstance(DisplayInfo *, DlElement *);
static void cartesianPlotUpdateGraphicalInfoCb(XtPointer cd);
static void cartesianPlotUpdateScreenFirstTime(XtPointer cd);
static void cartesianPlotDraw(XtPointer cd);
static void cartesianPlotUpdateValueCb(XtPointer cd);
static void cartesianPlotDestroyCb(XtPointer cd);
static void cartesianPlotName(XtPointer, char **, short *, int *);
static void cartesianPlotInheritValues(ResourceBundle *pRCB, DlElement *p);
static void cartesianPlotGetValues(ResourceBundle *pRCB, DlElement *p);
#endif

/* ===================================================================== */
/*            To emulate the XRT package       */

#ifndef USE_XRT
#define xtXrtGraphWidgetClass ((WidgetClass) 255)
void XrtSetNthDataStyle2() { }
XrtData * XrtMakeData (){return NULL;}
void XrtDestroyData (){}
void XrtSetNthDataStyle () { }
#endif

/* ===================================================================== */
static time_t time900101 = 0;
static time_t time700101 = 0;
static time_t timeOffset = 0;
static const char *timeFormatString[NUM_CP_TIME_FORMATS] = {
         "%H:%M:%S",
         "%H:%M",
         "%H:00",
         "%b %d %Y",
         "%b %d",
         "%b %d %H:00",
         "%a %H:00"};

#ifdef USE_XRT
static XrtData *nullData = NULL;
static void destroyDlElement (DlElement * element);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);

static DlDispatchTable cartesianPlotDlDispatchTable = {
         createDlCartesianPlot,
         destroyDlElement,
         executeMethod,
         writeDlCartesianPlot,
         NULL,
         cartesianPlotGetValues,
         cartesianPlotInheritValues,
         NULL,
         NULL,
         genericMove,
         genericScale,
	 NULL,
	 genericObjectInfo
};

static void destroyDlCartesianPlot (DlCartesianPlot * dlCartesianPlot)
{
  int traceNumber;
  
  if (dlCartesianPlot == NULL)
    return;

  objectAttributeDestroy(&(dlCartesianPlot->object));

  plotAxisDefinitionDestroy(&(dlCartesianPlot->axis[X_AXIS_ELEMENT]));
  plotAxisDefinitionDestroy(&(dlCartesianPlot->axis[Y1_AXIS_ELEMENT]));
  plotAxisDefinitionDestroy(&(dlCartesianPlot->axis[Y2_AXIS_ELEMENT]));
  
  for (traceNumber = 0; traceNumber < MAX_TRACES; traceNumber++)
    traceAttributeDestroy((DlTrace *)&(dlCartesianPlot->trace[traceNumber]));

  DM2KFREE(dlCartesianPlot->trigger);
  DM2KFREE(dlCartesianPlot->erase);

  free((char*)dlCartesianPlot);
}

static void destroyDlElement (DlElement * element)
{
  if (element == NULL || element->type != DL_CartesianPlot) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  destroyDlCartesianPlot (element->structure.cartesianPlot);
  free((char*)element);
}


static UpdateTask * cartesianPlotCreateRunTimeInstance
  (DisplayInfo *displayInfo,
   DlElement *dlElement)
{
  CartesianPlot   * pcp;
  int               i, k, n, validTraces, iPrec;
  Arg               args[46];
  Widget            localWidget;
  XColor            xColors[2];
  char            * headerStrings[2];
  char              rgb[2][16], string[24];
  int               usedHeight, usedCharWidth, bestSize, preferredHeight;
  float             minF, maxF, tickF;
  DlCartesianPlot * dlCartesianPlot = dlElement->structure.cartesianPlot;
  WidgetUserData  * userData;
  UpdateTask * updateTask = NULL;

  if (!nullData) 
  {
    struct tm ltime;
    nullData = XrtMakeData(XRT_GENERAL,1,1,TRUE);
    nullData->g.hole = 0.0;
    nullData->g.data[0].npoints = 1;
    nullData->g.data[0].xp[0] = 0.0;
    nullData->g.data[0].yp[0] = 0.0;

    ltime.tm_year  = 70;  /* year */
    ltime.tm_mon   = 0;   /* January */
    ltime.tm_mday  = 1;   /* day */
    ltime.tm_hour  = 0;   /* hour */
    ltime.tm_min   = 0;   /* min */
    ltime.tm_sec   = 0;   /* sec */
    ltime.tm_isdst = -1;  /* mktime figures out TZ effect */

    time700101 = mktime(&ltime);
    ltime.tm_year = 90;  /* year */
    ltime.tm_mon = 0;    /* January */
    ltime.tm_mday = 1;   /* day */
    ltime.tm_hour = 0;   /* day */
    ltime.tm_min  = 0;   /* day */
    ltime.tm_sec  = 0;   /* day */
    ltime.tm_isdst = -1; /* mktime figures out TZ effect */
    time900101 = mktime(&ltime);
    timeOffset = time900101 - time700101;
  }

  validTraces = 0;

  /* alloc servise structures
   */
  userData = DM2KALLOC (WidgetUserData);
  pcp = DM2KALLOC (CartesianPlot);

  if (pcp == NULL || userData == NULL) {
    DM2KFREE (pcp);
    DM2KFREE (userData);
    dm2kPrintf("cartesianPlotCreateRunTimeInstance : memory alloc error\n");
    return updateTask;
  }

  pcp->xrtData1  = pcp->xrtData2 = NULL;
  pcp->dirty1    = pcp->dirty2 = False;
  pcp->timeScale = False;
  pcp->startTime.secPastEpoch = 0;
  pcp->startTime.nsec = 0;
  pcp->eraseMode = dlCartesianPlot->eraseMode;
  pcp->dlElement = dlElement;
  dlCartesianPlot->object.runtimeDescriptor = (XtPointer) pcp;

  updateTask = pcp->updateTask = updateTaskAddTask(displayInfo,
						   &(dlCartesianPlot->object),
						   cartesianPlotDraw,
						   (XtPointer)pcp);
  
  if (pcp->updateTask == NULL) {
    dm2kPrintf("cartesianPlotCreateRunTimeInstance : memory allocation error\n");
  } else {
    updateTaskAddDestroyCb(pcp->updateTask,cartesianPlotDestroyCb);
    updateTaskAddNameCb(pcp->updateTask,cartesianPlotName);
  }

  for (i = 0; i < MAX_TRACES; i++) {
    Boolean validTrace = False;
    /* X data */
    if ((dlCartesianPlot->trace[i].xdata != NULL) && dlCartesianPlot->trace[i].xdata[0]) {
      pcp->xyTrace[validTraces].recordX =
              dm2kAllocateRecord(dlCartesianPlot->trace[i].xdata,
                cartesianPlotUpdateScreenFirstTime,
                cartesianPlotUpdateGraphicalInfoCb,
                (XtPointer) &(pcp->xyTrace[validTraces]));
      validTrace = True;
    } else {
      pcp->xyTrace[validTraces].recordX = NULL;
    }
    /* Y data */
    if ((dlCartesianPlot->trace[i].ydata != NULL) && dlCartesianPlot->trace[i].ydata[0]) {
      pcp->xyTrace[validTraces].recordY =
              dm2kAllocateRecord(dlCartesianPlot->trace[i].ydata,
                cartesianPlotUpdateScreenFirstTime,
                cartesianPlotUpdateGraphicalInfoCb,
                (XtPointer) &(pcp->xyTrace[validTraces]));
      validTrace = True;
    } else {
      pcp->xyTrace[validTraces].recordY = NULL;
    }
    if (validTrace) {
      pcp->xyTrace[validTraces].cartesianPlot = pcp;
      validTraces++;
    }
  }

  if (validTraces == 0) {
    /* if no channel, create a fake channel */
    validTraces = 1;
    pcp->xyTrace[0].recordX = dm2kAllocateRecord(" ",
                cartesianPlotUpdateScreenFirstTime,
                cartesianPlotUpdateGraphicalInfoCb,
                (XtPointer) &(pcp->xyTrace[0]));
    pcp->xyTrace[0].recordY = NULL;
  }

  /* now add monitor for trigger channel if appropriate */
  if ((dlCartesianPlot->trigger != NULL) && (validTraces > 0) && 
      dlCartesianPlot->trigger[0] ) {
    pcp->triggerCh.recordX = 
              dm2kAllocateRecord(dlCartesianPlot->trigger,
                cartesianPlotUpdateScreenFirstTime,
                cartesianPlotUpdateGraphicalInfoCb,
                (XtPointer) &(pcp->triggerCh));
    pcp->triggerCh.cartesianPlot = pcp;
  } else {
    pcp->triggerCh.recordX = NULL;
  }

  /* now add monitor for erase channel if appropriate 
   */
  if ((dlCartesianPlot->erase != NULL) && (validTraces > 0) &&
      dlCartesianPlot->erase[0] ) 
  {
    pcp->eraseCh.recordX =
      dm2kAllocateRecord(dlCartesianPlot->erase,
			 cartesianPlotUpdateScreenFirstTime,
			 cartesianPlotUpdateGraphicalInfoCb,
			 (XtPointer) &(pcp->eraseCh));
    pcp->eraseCh.cartesianPlot = pcp;
  }
  else {
    pcp->eraseCh.recordX = NULL;
  }

  drawWhiteRectangle(pcp->updateTask);

  /* from the cartesianPlot structure, we've got CartesianPlot's specifics */
  n = 0;
  XtSetArg(args[n],XmNx,(Position)dlCartesianPlot->object.x); n++;
  XtSetArg(args[n],XmNy,(Position)dlCartesianPlot->object.y); n++;
  XtSetArg(args[n],XmNwidth,(Dimension)dlCartesianPlot->object.width); n++;
  XtSetArg(args[n],XmNheight,(Dimension)dlCartesianPlot->object.height); n++;
  XtSetArg(args[n],XmNhighlightThickness,0); n++;
  XtSetArg(args[n],XmNcolormap,cmap); n++;

  /* long way around for color handling... 
   * but XRT/Graph insists on strings! 
   */
  xColors[0].pixel = displayInfo->colormap[dlCartesianPlot->plotcom.clr];
  xColors[1].pixel = displayInfo->colormap[dlCartesianPlot->plotcom.bclr];
  XQueryColors(display,cmap,xColors,2);

  sprintf(rgb[0],"#%2.2x%2.2x%2.2x",xColors[0].red>>8, xColors[0].green>>8,
		xColors[0].blue>>8);
  sprintf(rgb[1],"#%2.2x%2.2x%2.2x",xColors[1].red>>8, xColors[1].green>>8,
		xColors[1].blue>>8);

  XtSetArg(args[n],XtNxrtGraphForegroundColor,rgb[0]); n++;
  XtSetArg(args[n],XtNxrtGraphBackgroundColor,rgb[1]); n++;
  XtSetArg(args[n],XtNxrtDataAreaBackgroundColor,rgb[1]); n++;
  XtSetArg(args[n],XtNxrtForegroundColor,rgb[0]); n++;
  XtSetArg(args[n],XtNxrtBackgroundColor,rgb[1]); n++;
  XtSetArg(args[n],XmNcolormap,cmap); n++;

  preferredHeight = MIN(dlCartesianPlot->object.width,
			dlCartesianPlot->object.height)/TITLE_SCALE_FACTOR;
  
  bestSize = dmGetBestFontWithInfo(fontTable,MAX_FONTS,NULL,
				   preferredHeight,0,
				   &usedHeight,&usedCharWidth,FALSE);

  if ( XRT_VERSION >= 3 )
     XtSetArg(args[n],XtNxrtHeaderFont,
	      XmFontListCreate( fontTable[bestSize], XmSTRING_DEFAULT_CHARSET ));
  else
     XtSetArg(args[n],XtNxrtHeaderFont,fontTable[bestSize]->fid); 
  n++;

  headerStrings[0] = dlCartesianPlot->plotcom.title;
  headerStrings[1] = NULL;

  XtSetArg(args[n],XtNxrtHeaderStrings,headerStrings); n++;
  XtSetArg(args[n],XtNxrtXTitle,dlCartesianPlot->plotcom.xlabel); n++;
  XtSetArg(args[n],XtNxrtYTitle,dlCartesianPlot->plotcom.ylabel); n++;

  preferredHeight = MIN(dlCartesianPlot->object.width,
		dlCartesianPlot->object.height)/AXES_SCALE_FACTOR;
  bestSize = dmGetBestFontWithInfo(fontTable,MAX_FONTS,NULL,
		preferredHeight,0,&usedHeight,&usedCharWidth,FALSE);

  
  if ( XRT_VERSION >= 3 )
     XtSetArg(args[n],XtNxrtAxisFont,
	      XmFontListCreate( fontTable[bestSize], XmSTRING_DEFAULT_CHARSET ));
  else
     XtSetArg(args[n],XtNxrtAxisFont,fontTable[bestSize]->fid); 
  n++;

  switch (dlCartesianPlot->style) 
  {
    case POINT_PLOT:
    case PURELINE_PLOT:
    case LINE_PLOT:
	XtSetArg(args[n],XtNxrtType,XRT_PLOT); n++;
	if (validTraces > 1) {
	    XtSetArg(args[n],XtNxrtType2,XRT_PLOT); n++;
	}
	break;
    case FILL_UNDER_PLOT:
	XtSetArg(args[n],XtNxrtType,XRT_AREA); n++;
	if (validTraces > 1) {
	    XtSetArg(args[n],XtNxrtType2,XRT_AREA); n++;
	}
	break;
  }

  if (validTraces > 1) {
    XtSetArg(args[n],XtNxrtY2AxisShow,TRUE); n++;
  }

  /******* Axis Definitions *******/

  /* X Axis definition 
   */
  switch (dlCartesianPlot->axis[X_AXIS_ELEMENT].axisStyle) 
  {
    case LINEAR_AXIS:
      break;
    case LOG10_AXIS:
      XtSetArg(args[n],XtNxrtXAxisLogarithmic,True); n++;
      break;
    case TIME_AXIS: {
      time_t tval;
      struct tm ltime;
      ltime.tm_year = 90;  /* year */
      ltime.tm_mon = 0;    /* January */
      ltime.tm_mday = 1;   /* day */
      ltime.tm_hour = 0;   /* day */
      ltime.tm_min  = 0;   /* day */
      ltime.tm_sec  = 0;   /* day */
      ltime.tm_isdst = -1; /* mktime figures out TZ effect */
      tval = mktime(&ltime);
      XtSetArg(args[n],XtNxrtXAnnotationMethod,XRT_ANNO_TIME_LABELS);n++;
      XtSetArg(args[n],XtNxrtTimeBase,tval);n++;
      XtSetArg(args[n],XtNxrtTimeUnit,XRT_TMUNIT_SECONDS);n++;
      XtSetArg(args[n],XtNxrtTimeFormatUseDefault,False);n++;
      XtSetArg(args[n],XtNxrtTimeFormat,
               timeFormatString[dlCartesianPlot->axis[0].timeFormat
               -FIRST_CP_TIME_FORMAT]);n++;
      pcp->timeScale = True;
      }
      break;
    default:
      dm2kPrintf("\nexecuteDlCartesianPlot: unknown X axis style");
    break;
  }

  switch (dlCartesianPlot->axis[X_AXIS_ELEMENT].rangeStyle) 
  {
    case CHANNEL_RANGE:		/* handle as default until connected */
    case AUTO_SCALE_RANGE:
      XtSetArg(args[n],XtNxrtXNumUseDefault,True); n++;
      XtSetArg(args[n],XtNxrtXTickUseDefault,True); n++;
      XtSetArg(args[n],XtNxrtXPrecisionUseDefault,True); n++;
      break;
    case USER_SPECIFIED_RANGE:
      minF = dlCartesianPlot->axis[X_AXIS_ELEMENT].minRange;
      maxF = dlCartesianPlot->axis[X_AXIS_ELEMENT].maxRange;
      tickF = (maxF - minF)/4.0;
      XtSetArg(args[n],XtNxrtXMin,XrtFloatToArgVal(minF)); n++;
      XtSetArg(args[n],XtNxrtXMax,XrtFloatToArgVal(maxF)); n++;
      XtSetArg(args[n],XtNxrtXTick,XrtFloatToArgVal(tickF)); n++;
      XtSetArg(args[n],XtNxrtXNum,XrtFloatToArgVal(tickF)); n++;
      sprintf(string,"%f",tickF);
      k = STRLEN(string)-1;
      while (string[k] == '0') k--; /* strip off trailing zeroes */
      iPrec = k;
      while (string[k] != '.' && k >= 0) k--;
      iPrec = iPrec - k;
      XtSetArg(args[n],XtNxrtXPrecision,iPrec); n++;
      break;
    default:
      dm2kPrintf("\nexecuteDlCartesianPlot: unknown X range style");
      break;
  }

  /* Y1 Axis definition 
   */
  switch (dlCartesianPlot->axis[Y1_AXIS_ELEMENT].axisStyle) {
    case LINEAR_AXIS:
      break;
    case LOG10_AXIS:
      XtSetArg(args[n],XtNxrtYAxisLogarithmic,True); n++;
      break;
    default:
      dm2kPrintf("\nexecuteDlCartesianPlot: unknown Y1 axis style");
      break;
  }

  switch (dlCartesianPlot->axis[Y1_AXIS_ELEMENT].rangeStyle) 
  {
    case CHANNEL_RANGE:		/* handle as default until connected */
    case AUTO_SCALE_RANGE:
      XtSetArg(args[n],XtNxrtYNumUseDefault,True); n++;
      XtSetArg(args[n],XtNxrtYTickUseDefault,True); n++;
      XtSetArg(args[n],XtNxrtYPrecisionUseDefault,True); n++;
      break;
    case USER_SPECIFIED_RANGE:
      minF = dlCartesianPlot->axis[Y1_AXIS_ELEMENT].minRange;
      maxF = dlCartesianPlot->axis[Y1_AXIS_ELEMENT].maxRange;
      tickF = (maxF - minF)/4.0;
      XtSetArg(args[n],XtNxrtYMin,XrtFloatToArgVal(minF)); n++;
      XtSetArg(args[n],XtNxrtYMax,XrtFloatToArgVal(maxF)); n++;
      XtSetArg(args[n],XtNxrtYTick,XrtFloatToArgVal(tickF)); n++;
      XtSetArg(args[n],XtNxrtYNum,XrtFloatToArgVal(tickF)); n++;
      sprintf(string,"%f",tickF);
      k = STRLEN(string)-1;
      while (string[k] == '0') k--; /* strip off trailing zeroes */
      iPrec = k;
      while (string[k] != '.' && k >= 0) k--;
      iPrec = iPrec - k;
      XtSetArg(args[n],XtNxrtYPrecision,iPrec); n++;
      break;
    default:
      dm2kPrintf("\nexecuteDlCartesianPlot: unknown Y1 range style");
      break;
  }

  /* Y2 Axis definition 
   */
  switch (dlCartesianPlot->axis[Y2_AXIS_ELEMENT].axisStyle) {
	case LINEAR_AXIS:
		break;
	case LOG10_AXIS:
		XtSetArg(args[n],XtNxrtY2AxisLogarithmic,True); n++;
		break;
	default:
		dm2kPrintf(
		"\nexecuteDlCartesianPlot: unknown Y2 axis style");
		break;
  }

  switch (dlCartesianPlot->axis[Y2_AXIS_ELEMENT].rangeStyle) 
  {
	case CHANNEL_RANGE: /* handle as default until connected */
	case AUTO_SCALE_RANGE:
		XtSetArg(args[n],XtNxrtY2NumUseDefault,True); n++;
		XtSetArg(args[n],XtNxrtY2TickUseDefault,True); n++;
		XtSetArg(args[n],XtNxrtY2PrecisionUseDefault,True); n++;
		break;
	case USER_SPECIFIED_RANGE:
		minF = dlCartesianPlot->axis[Y2_AXIS_ELEMENT].minRange;
		maxF = dlCartesianPlot->axis[Y2_AXIS_ELEMENT].maxRange;
		tickF = (maxF - minF)/4.0;
		XtSetArg(args[n],XtNxrtY2Min,XrtFloatToArgVal(minF)); n++;
		XtSetArg(args[n],XtNxrtY2Max,XrtFloatToArgVal(maxF)); n++;
		XtSetArg(args[n],XtNxrtY2Tick,XrtFloatToArgVal(tickF)); n++;
		XtSetArg(args[n],XtNxrtY2Num,XrtFloatToArgVal(tickF)); n++;
		sprintf(string,"%f",tickF);
		k = STRLEN(string)-1;
		while (string[k] == '0') k--; /* strip off trailing zeroes */
		iPrec = k;
		while (string[k] != '.' && k >= 0) k--;
		iPrec = iPrec - k;
		XtSetArg(args[n],XtNxrtY2Precision,iPrec); n++;
		break;
	default:
		dm2kPrintf(
		"\nexecuteDlCartesianPlot: unknown Y2 range style");
		break;
  }
  


  XtSetArg(args[n],XmNtraversalOn,False); n++;

  /* add the pointer to the CartesianPlot structure as userData to widget
   */
  userData->privateData    = (char*) pcp;
  userData->updateTask = (pcp ? pcp->updateTask : NULL);
  
  XtSetArg(args[n], XmNuserData, (XtPointer) userData); n++;
  XtSetArg(args[n], XtNxrtDoubleBuffer, True); n++;

  localWidget = XtCreateWidget((headerStrings[0]&&headerStrings[0][0])?headerStrings[0]:"cartesianPlot", 
			       xtXrtGraphWidgetClass,
			       displayInfo->drawingArea, args, n);
  /* destroy callback should free allocated memory
   */
  XtAddCallback (localWidget, XmNdestroyCallback, freeUserDataCB, NULL);

  dlElement->widget = localWidget;

  /* record the number of traces in the cartesian plot 
   */
  pcp->nTraces = validTraces;

  /* add in drag/drop translations 
   */
  XtOverrideTranslations(localWidget,parsedTranslations);

  return updateTask;
}

static void cartesianPlotCreateEditInstance
   (DisplayInfo *displayInfo,
    DlElement *dlElement)
{
  int               k, n, iPrec;
  int               validTraces = 0;
  Arg               args[42];
  Widget            localWidget;
  XColor            xColors[2];
  char            * headerStrings[2];
  char              rgb[2][16], string[24];
  int               usedHeight, usedCharWidth, bestSize, preferredHeight;
  float             minF, maxF, tickF;
  DlCartesianPlot * dlCartesianPlot = dlElement->structure.cartesianPlot;


  /* from the cartesianPlot structure, we've got CartesianPlot's specifics 
   */
  n = 0;
  XtSetArg(args[n],XmNx,(Position)dlCartesianPlot->object.x); n++;
  XtSetArg(args[n],XmNy,(Position)dlCartesianPlot->object.y); n++;
  XtSetArg(args[n],XmNwidth,(Dimension)dlCartesianPlot->object.width); n++;
  XtSetArg(args[n],XmNheight,(Dimension)dlCartesianPlot->object.height); n++;
  XtSetArg(args[n],XmNhighlightThickness,0); n++;

  /* long way around for color handling... 
   * but XRT/Graph insists on strings! 
   */
  xColors[0].pixel = displayInfo->colormap[dlCartesianPlot->plotcom.clr];
  xColors[1].pixel = displayInfo->colormap[dlCartesianPlot->plotcom.bclr];
  XQueryColors(display,cmap,xColors,2);

  sprintf(rgb[0],"#%2.2x%2.2x%2.2x",xColors[0].red>>8, xColors[0].green>>8,
		xColors[0].blue>>8);
  sprintf(rgb[1],"#%2.2x%2.2x%2.2x",xColors[1].red>>8, xColors[1].green>>8,
	  xColors[1].blue>>8);

  XtSetArg(args[n],XtNxrtGraphForegroundColor,rgb[0]); n++;
  XtSetArg(args[n],XtNxrtGraphBackgroundColor,rgb[1]); n++;
  XtSetArg(args[n],XtNxrtForegroundColor,rgb[0]); n++;
  XtSetArg(args[n],XtNxrtBackgroundColor,rgb[1]); n++;

  preferredHeight = MIN(dlCartesianPlot->object.width,
			dlCartesianPlot->object.height)/TITLE_SCALE_FACTOR;

  bestSize = dmGetBestFontWithInfo(fontTable,MAX_FONTS,NULL,
				   preferredHeight,0,
				   &usedHeight,&usedCharWidth,FALSE);

  if ( XRT_VERSION >= 3 )
     XtSetArg(args[n],XtNxrtHeaderFont,
	      XmFontListCreate( fontTable[bestSize], XmSTRING_DEFAULT_CHARSET ));
  else
     XtSetArg(args[n],XtNxrtHeaderFont,fontTable[bestSize]->fid); 
  n++;

  headerStrings[0] = dlCartesianPlot->plotcom.title;
  headerStrings[1] = NULL;

  XtSetArg(args[n],XtNxrtHeaderStrings,headerStrings); n++;
  XtSetArg(args[n],XtNxrtXTitle,dlCartesianPlot->plotcom.xlabel); n++;
  XtSetArg(args[n],XtNxrtYTitle,dlCartesianPlot->plotcom.ylabel); n++;

  preferredHeight = MIN(dlCartesianPlot->object.width,
		dlCartesianPlot->object.height)/AXES_SCALE_FACTOR;

  bestSize = dmGetBestFontWithInfo(fontTable,MAX_FONTS,NULL,
				   preferredHeight,0,
				   &usedHeight,&usedCharWidth,FALSE);

  if ( XRT_VERSION >= 3 )
     XtSetArg(args[n],XtNxrtAxisFont,
	      XmFontListCreate( fontTable[bestSize], XmSTRING_DEFAULT_CHARSET ));
  else
     XtSetArg(args[n],XtNxrtAxisFont,fontTable[bestSize]->fid);
  n++;

  switch (dlCartesianPlot->style) {
    case POINT_PLOT:
    case PURELINE_PLOT:
    case LINE_PLOT:
	XtSetArg(args[n],XtNxrtType,XRT_PLOT); n++;
	if (validTraces > 1) {
	    XtSetArg(args[n],XtNxrtType2,XRT_PLOT); n++;
	}
	break;
    case FILL_UNDER_PLOT:
	XtSetArg(args[n],XtNxrtType,XRT_AREA); n++;
	if (validTraces > 1) {
	    XtSetArg(args[n],XtNxrtType2,XRT_AREA); n++;
	}
	break;
  }

  if (validTraces > 1) {
    XtSetArg(args[n],XtNxrtY2AxisShow,TRUE); n++;
  }

  /******* Axis Definitions *******/

  /* X Axis definition 
   */
  switch (dlCartesianPlot->axis[X_AXIS_ELEMENT].axisStyle) 
  {
    case LINEAR_AXIS:
      break;
    case LOG10_AXIS:
      XtSetArg(args[n],XtNxrtYAxisLogarithmic,True); n++;
      break;
    case TIME_AXIS:
      break;
    default:
      dm2kPrintf("\nexecuteDlCartesianPlot: unknown X axis style"); 
      break;
  }

  switch (dlCartesianPlot->axis[X_AXIS_ELEMENT].rangeStyle) 
  {
  case CHANNEL_RANGE: /* handle as default until connected */
  case AUTO_SCALE_RANGE:
    XtSetArg(args[n],XtNxrtXNumUseDefault,True); n++;
    XtSetArg(args[n],XtNxrtXTickUseDefault,True); n++;
    XtSetArg(args[n],XtNxrtXPrecisionUseDefault,True); n++;
    break;
  case USER_SPECIFIED_RANGE:
    minF = dlCartesianPlot->axis[X_AXIS_ELEMENT].minRange;
    maxF = dlCartesianPlot->axis[X_AXIS_ELEMENT].maxRange;
    tickF = (maxF - minF)/4.0;
    XtSetArg(args[n],XtNxrtXMin,XrtFloatToArgVal(minF)); n++;
    XtSetArg(args[n],XtNxrtXMax,XrtFloatToArgVal(maxF)); n++;
    XtSetArg(args[n],XtNxrtXTick,XrtFloatToArgVal(tickF)); n++;
    XtSetArg(args[n],XtNxrtXNum,XrtFloatToArgVal(tickF)); n++;
    sprintf(string,"%f",tickF);
    k = STRLEN(string)-1;
    while (string[k] == '0') k--; /* strip off trailing zeroes */
    iPrec = k;
    while (string[k] != '.' && k >= 0) k--;
    iPrec = iPrec - k;
    XtSetArg(args[n],XtNxrtXPrecision,iPrec); n++;
    break;
  default:
    dm2kPrintf(
	       "\nexecuteDlCartesianPlot: unknown X range style");
    break;
  }

  /* Y1 Axis definition 
   */
  switch (dlCartesianPlot->axis[Y1_AXIS_ELEMENT].axisStyle)
  {
	case LINEAR_AXIS:
		break;
	case LOG10_AXIS:
		XtSetArg(args[n],XtNxrtYAxisLogarithmic,True); n++;
		break;
	default:
		dm2kPrintf(
		"\nexecuteDlCartesianPlot: unknown Y1 axis style");
		break;
  }

  switch (dlCartesianPlot->axis[Y1_AXIS_ELEMENT].rangeStyle)
  {
	case CHANNEL_RANGE: /* handle as default until connected */
	case AUTO_SCALE_RANGE:
		XtSetArg(args[n],XtNxrtYNumUseDefault,True); n++;
		XtSetArg(args[n],XtNxrtYTickUseDefault,True); n++;
		XtSetArg(args[n],XtNxrtYPrecisionUseDefault,True); n++;
		break;
	case USER_SPECIFIED_RANGE:
		minF = dlCartesianPlot->axis[Y1_AXIS_ELEMENT].minRange;
		maxF = dlCartesianPlot->axis[Y1_AXIS_ELEMENT].maxRange;
		tickF = (maxF - minF)/4.0;
		XtSetArg(args[n],XtNxrtYMin,XrtFloatToArgVal(minF)); n++;
		XtSetArg(args[n],XtNxrtYMax,XrtFloatToArgVal(maxF)); n++;
		XtSetArg(args[n],XtNxrtYTick,XrtFloatToArgVal(tickF)); n++;
		XtSetArg(args[n],XtNxrtYNum,XrtFloatToArgVal(tickF)); n++;
		sprintf(string,"%f",tickF);
		k = STRLEN(string)-1;
		while (string[k] == '0') k--; /* strip off trailing zeroes */
		iPrec = k;
		while (string[k] != '.' && k >= 0) k--;
		iPrec = iPrec - k;
		XtSetArg(args[n],XtNxrtYPrecision,iPrec); n++;
		break;
	default:
		dm2kPrintf(
		"\nexecuteDlCartesianPlot: unknown Y1 range style");
		break;
  }

  /* Y2 Axis definition 
   */
  switch (dlCartesianPlot->axis[Y2_AXIS_ELEMENT].axisStyle) 
  {
	case LINEAR_AXIS:
		break;
	case LOG10_AXIS:
		XtSetArg(args[n],XtNxrtY2AxisLogarithmic,True); n++;
		break;
	default:
		dm2kPrintf(
		"\nexecuteDlCartesianPlot: unknown Y2 axis style");
		break;
  }

  switch (dlCartesianPlot->axis[Y2_AXIS_ELEMENT].rangeStyle) 
  {
	case CHANNEL_RANGE: /* handle as default until connected */
	case AUTO_SCALE_RANGE:
		XtSetArg(args[n],XtNxrtY2NumUseDefault,True); n++;
		XtSetArg(args[n],XtNxrtY2TickUseDefault,True); n++;
		XtSetArg(args[n],XtNxrtY2PrecisionUseDefault,True); n++;
		break;
	case USER_SPECIFIED_RANGE:
		minF = dlCartesianPlot->axis[Y2_AXIS_ELEMENT].minRange;
		maxF = dlCartesianPlot->axis[Y2_AXIS_ELEMENT].maxRange;
		tickF = (maxF - minF)/4.0;
		XtSetArg(args[n],XtNxrtY2Min,XrtFloatToArgVal(minF)); n++;
		XtSetArg(args[n],XtNxrtY2Max,XrtFloatToArgVal(maxF)); n++;
		XtSetArg(args[n],XtNxrtY2Tick,XrtFloatToArgVal(tickF)); n++;
		XtSetArg(args[n],XtNxrtY2Num,XrtFloatToArgVal(tickF)); n++;
		sprintf(string,"%f",tickF);
		k = STRLEN(string)-1;
		while (string[k] == '0') k--; /* strip off trailing zeroes */
		iPrec = k;
		while (string[k] != '.' && k >= 0) k--;
		iPrec = iPrec - k;
		XtSetArg(args[n],XtNxrtY2Precision,iPrec); n++;
		break;
	default:
		dm2kPrintf(
		"\nexecuteDlCartesianPlot: unknown Y2 range style");
		break;
  }


  XtSetArg(args[n],XmNtraversalOn,False); n++;
  XtSetArg(args[n], XtNxrtDoubleBuffer, True); n++;
  XtSetArg(args[n], XtNxrtData, 0); n++;

  localWidget = XtCreateWidget("cartesianPlot",xtXrtGraphWidgetClass,
		displayInfo->drawingArea, args, n);

  dlElement->widget = localWidget;

/* add button press handlers */
  editObjectHandler (displayInfo, dlElement);

  XtManageChild(localWidget);
}

static UpdateTask * executeMethod 
  (DisplayInfo * displayInfo, 
   DlElement   * dlElement) 
{
  UpdateTask * updateTask = NULL;

  if (dlElement->widget) {
    DlObject *po = &(dlElement->structure.cartesianPlot->object);
    XtVaSetValues(dlElement->widget,
        XmNx, (Position) po->x,
        XmNy, (Position) po->y,
        XmNwidth, (Dimension) po->width,
        XmNheight, (Dimension) po->height,
        NULL);
  } 
  else if (displayInfo->traversalMode == DL_EXECUTE) {
    updateTask = cartesianPlotCreateRunTimeInstance(displayInfo, dlElement);
  } 
  else if (displayInfo->traversalMode == DL_EDIT) {
    cartesianPlotCreateEditInstance(displayInfo, dlElement);
  }

  return updateTask;
}

static void cartesianPlotUpdateGraphicalInfoCb(XtPointer cd) {
  Record *pd = (Record *) cd;
  XYTrace *pt = (XYTrace *) pd->clientData;
  CartesianPlot *pcp = pt->cartesianPlot;
  DisplayInfo *displayInfo = pcp->updateTask->displayInfo;
  DlCartesianPlot *dlCartesianPlot = pcp->dlElement->structure.cartesianPlot;
  Widget widget = pcp->dlElement->widget;
  XColor xColors[MAX_TRACES];
  char rgb[MAX_TRACES][16];
  /* xrtData1 for 1st trace, xrtData2 for 2nd -> nTraces */
  XrtData *xrtData1, *xrtData2;
  XrtDataStyle myds1[MAX_TRACES], myds2[MAX_TRACES];
  float minX, maxX, minY, maxY, minY2, maxY2;
  float minXF, maxXF, minYF, maxYF, minY2F, maxY2F, tickF;
  char string[24];
  int iPrec, kk;

  int maxElements = 0;
  int i,j;
  int n;
  Arg widgetArgs[20];

  /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
  /* !!!!! This is a temperory work around !!!!! */
  /* !!!!! for the reconnection.           !!!!! */
  /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
  dm2kRecordAddGraphicalInfoCb(pd,NULL);

  /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
  /* !!!!! End work around                 !!!!! */
  /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

  xrtData1 = xrtData2 = NULL;

  /* return until all channels get their graphical information */
  for (i = 0; i < pcp->nTraces; i++) {
    XYTrace *t = &(pcp->xyTrace[i]);
    if (t->recordX) {
      if (t->recordX->precision < 0) return;
    }

    if (t->recordY) {
      if (t->recordY->precision < 0) return;
    }
  }

  /* initialize min/max to some other-end-of-the-range values */
  minX = minY = minY2 = FLT_MAX;
  maxX = maxY = maxY2 = -0.99*FLT_MAX;

  for (i = 0; i < pcp->nTraces; i++)
    xColors[i].pixel = displayInfo->colormap[dlCartesianPlot->trace[i].data_clr];
  XQueryColors(XtDisplay(widget),cmap,xColors,pcp->nTraces);
  for (i = 0; i < pcp->nTraces; i++) {
    sprintf(rgb[i],"#%2.2x%2.2x%2.2x", xColors[i].red>>8,
                 xColors[i].green>>8, xColors[i].blue>>8);
  }
  /*
   * now create XrtData
   */
  /* allocate XrtData structures */

  /* find out the maximum number of element */
  maxElements = dlCartesianPlot->count;
  for (i = 0; i < pcp->nTraces; i++) {
    XYTrace *t = &(pcp->xyTrace[i]);
    if (t->recordX)
      maxElements = MAX(maxElements,(int)t->recordX->elementCount);
    if (t->recordY)
      maxElements = MAX(maxElements,(int)t->recordY->elementCount);
  }

  xrtData1 = XrtMakeData(XRT_GENERAL,1,maxElements,TRUE);
  /* now 2nd xrt data set if appropriate */
  if (pcp->nTraces > 1) {
    xrtData2 = XrtMakeData(XRT_GENERAL,pcp->nTraces-1,maxElements,TRUE);
  }

  /* loop over monitors, extracting information */
  for (i = 0; i < pcp->nTraces; i++) {
    XYTrace *t = &(pcp->xyTrace[i]);
    /*
     * now process data type (based on type (scalar or vector) of data)
     */
    if (t->recordX && t->recordY) {
      if ( t->recordX->elementCount > 1 && t->recordY->elementCount > 1 ) {
        t->type = CP_XYVector;
      } else
      if ( t->recordX->elementCount > 1 && t->recordY->elementCount <= 1 ) {
        t->type = CP_XVectorYScalar;
      } else
      if ( t->recordY->elementCount > 1 && t->recordX->elementCount <= 1 ) {
        t->type = CP_YVectorXScalar;
      } else
      if ( t->recordX->elementCount <= 1 && t->recordY->elementCount <= 1 ) {
        t->type = CP_XYScalar;
      }
      /* put initial data in */
      if (i <= 0) {
        xrtData1->g.data[i].npoints = 0;
        minY = MIN(minY,t->recordY->lopr);
        maxY = MAX(maxY,t->recordY->hopr);
      } else {
        xrtData2->g.data[i-1].npoints = 0;
        minY2 = MIN(minY2,t->recordY->lopr);
        maxY2 = MAX(maxY2,t->recordY->hopr);
      }
      minX = MIN(minX,t->recordX->lopr);
      maxX = MAX(maxX,t->recordX->hopr);

    } else
    if (t->recordX && t->recordY) {
      /* X channel - supporting scalar or waveform */
      if (t->recordX->elementCount > 1) {
        /* vector/waveform */
        t->type = CP_XVector;
        if (i <= 0) {
          for (j = 0; j < (int)t->recordX->elementCount; j++)
            xrtData1->g.data[i].yp[j]= (float)j;
          minY = MIN(minY,0.);
          maxY = MAX(maxY,(float)((int)t->recordX->elementCount-1));
        } else {
          for(j = 0; j < (int)t->recordX->elementCount; j++)
            xrtData2->g.data[i-1].yp[j]= (float)j;
          minY2 = MIN(minY2,0.);
          maxY2 = MAX(maxY2,(float)t->recordX->elementCount-1);
        }
        minX = MIN(minX,t->recordX->lopr);
        maxX = MAX(maxX,t->recordX->hopr);
      } else {
        /* scalar */
        t->type = CP_XScalar;
        if (i <= 0) {
          for(j = 0; j < dlCartesianPlot->count; j++)
            xrtData1->g.data[i].yp[j]= (float)j;
          xrtData1->g.data[i].xp[0]= (float)t->recordX->value;
          minY = MIN(minY,0.);
          maxY = MAX(maxY,(float)dlCartesianPlot->count);
        } else {
          for(j = 0; j < dlCartesianPlot->count; j++)
            xrtData2->g.data[i-1].yp[j]= (float)j;
          xrtData2->g.data[i-1].xp[0]= (float)t->recordX->value;
          minY2 = MIN(minY2,0.);
          maxY2 = MAX(maxY2,(float)dlCartesianPlot->count);
        }
        minX = MIN(minX,t->recordX->lopr);
        maxX = MAX(maxX,t->recordX->hopr);
      }
    } else
    if (t->recordX == NULL && t->recordY != NULL) {
      /* Y channel - supporting scalar or waveform */
      if (t->recordY->elementCount > 1) {
        /* vector/waveform */
        t->type = CP_YVector;
        if (i <= 0) {
          for(j = 0; j < t->recordY->elementCount; j++)
            xrtData1->g.data[i].xp[j]= (float)j;
          minY = MIN(minY,t->recordY->lopr);
          maxY = MAX(maxY,t->recordY->hopr);
        } else {
          for(j = 0; j < t->recordY->elementCount; j++)
            xrtData2->g.data[i-1].xp[j]= (float)j;
          minY2 = MIN(minY2,t->recordY->lopr);
          maxY2 = MAX(maxY2,t->recordY->hopr);
        }
        minX = MIN(minX,0.);
        maxX = MAX(maxX,(float)(t->recordY->elementCount-1));
      } else {
        /* scalar */
        t->type = CP_YScalar;
        if (i <= 0) {
          float *px = xrtData1->g.data[0].xp;
          for(j = 0; j < dlCartesianPlot->count; j++)
            px[j]= (float)j;
          xrtData1->g.data[0].yp[0]= (float)t->recordY->value;
          xrtData1->g.data[0].npoints = 0;
          minY = MIN(minY,t->recordY->lopr);
          maxY = MAX(maxY,t->recordY->hopr);
        } else {
          float *px = xrtData2->g.data[0].xp;
          for(j = 0; j < dlCartesianPlot->count; j++)
            px[j]= (float)j;
          xrtData2->g.data[0].yp[0]= (float)t->recordY->value;
          xrtData2->g.data[0].npoints = 0;
          minY2 = MIN(minY2,t->recordY->lopr);
          maxY2 = MAX(maxY2,t->recordY->hopr);
        }
        minX = MIN(minX,0.);
        maxX = MAX(maxX,(float)dlCartesianPlot->count);
      }
    } /* end for */
  }

  /*
   * now set XrtDataStyle
   */

  /* initialize XrtDataStyle array */
  for (i = 0; i < MAX_TRACES; i++) {
    /* there is really only going to be one myds1 in a graph */
    myds1[i].lpat = XRT_LPAT_SOLID;
    myds1[i].fpat = XRT_FPAT_SOLID;
    myds1[i].color = rgb[i];
    myds1[i].width = 1;
    if (dlCartesianPlot->style==PURELINE_PLOT)
       myds1[i].point = XRT_POINT_NONE;
    else
       myds1[i].point = XRT_POINT_DOT;
    myds1[i].pcolor = rgb[i];
    myds1[i].psize = MAX(2,dlCartesianPlot->object.height/70);
    myds1[i].res1 = 0;
    myds1[i].res1 = 0;

    myds2[i].lpat = XRT_LPAT_SOLID;
    myds2[i].fpat = (XrtFillPattern) (XRT_FPAT_SOLID + i);
    myds2[i].color = rgb[i];
    myds2[i].width = 1;
    if (dlCartesianPlot->style==PURELINE_PLOT)
       myds2[i].point = XRT_POINT_NONE;
    else
       myds2[i].point = (XrtPoint) (XRT_POINT_BOX+i);
    myds2[i].pcolor = rgb[i];
    myds2[i].psize = MAX(2,dlCartesianPlot->object.height/70);
    myds2[i].res1 = 0;
    myds2[i].res1 = 0;
  }


  /* loop over each trace and set xrtData attributes */
  for (i = 0; i < pcp->nTraces; i++) {
    switch(dlCartesianPlot->style) {
      case POINT_PLOT:
        if (i <= 0) {
          myds1[i].lpat = XRT_LPAT_NONE;
          myds1[i].fpat = XRT_FPAT_NONE;
          myds1[i].color = rgb[i];
          myds1[i].pcolor = rgb[i];
          myds1[i].psize = MAX(2,dlCartesianPlot->object.height/70);
          XrtSetNthDataStyle(widget,i,&myds1[i]);
        } else {
          myds2[i-1].lpat = XRT_LPAT_NONE;
          myds2[i-1].fpat = XRT_FPAT_NONE;
          myds2[i-1].color = rgb[i];
          myds2[i-1].pcolor = rgb[i];
          myds2[i-1].psize = MAX(2, dlCartesianPlot->object.height/70);
          XrtSetNthDataStyle2(widget, i-1,&myds2[i-1]);
        }
        break;
      case PURELINE_PLOT:
      case LINE_PLOT:
        if (i <= 0) {
          myds1[i].fpat = XRT_FPAT_NONE;
          myds1[i].color = rgb[i];
          myds1[i].pcolor = rgb[i];
          myds1[i].psize = MAX(2, dlCartesianPlot->object.height/70);
          XrtSetNthDataStyle(widget,i,&myds1[i]);
        } else {
          myds2[i-1].fpat = XRT_FPAT_NONE;
          myds2[i-1].color = rgb[i];
          myds2[i-1].pcolor = rgb[i];
          myds2[i-1].psize = MAX(2, dlCartesianPlot->object.height/70);
          XrtSetNthDataStyle2(widget, i-1,&myds2[i-1]);
        }
        break;
      case FILL_UNDER_PLOT:
        if (i <= 0) {
          myds1[i].color = rgb[i];
          myds1[i].pcolor = rgb[i];
          myds1[i].psize = MAX(2, dlCartesianPlot->object.height/70);
          XrtSetNthDataStyle(widget,i,&myds1[i]);
        } else {
          myds2[i-1].color = rgb[i];
          myds2[i-1].pcolor = rgb[i];
          myds2[i-1].psize = MAX(2, dlCartesianPlot->object.height/70);
          XrtSetNthDataStyle2(widget, i-1,&myds2[i-1]);
        }
        break;
    }
  }

  /*
   * now register event handlers for vectors of channels/monitors
   */
  for (i = 0; i < pcp->nTraces; i++) {
    XYTrace *t = &(pcp->xyTrace[i]);
    if ( i <= 0) {
      t->xrtData = xrtData1;
      t->trace = i;
    } else {
      t->xrtData = xrtData2;
      t->trace = i-1;
    }
  }


  /* fill in connect-time channel-based range specifications - NB:
   *  this is different than the min/max stored in the display element
   */
  pcp->axisRange[X_AXIS_ELEMENT].axisMin = minX;
  pcp->axisRange[X_AXIS_ELEMENT].axisMax = maxX;
  pcp->axisRange[Y1_AXIS_ELEMENT].axisMin = minY;
  pcp->axisRange[Y1_AXIS_ELEMENT].axisMax = maxY;
  pcp->axisRange[Y2_AXIS_ELEMENT].axisMin = minY2;
  pcp->axisRange[Y2_AXIS_ELEMENT].axisMax = maxY2;
  for (kk = X_AXIS_ELEMENT; kk <= Y2_AXIS_ELEMENT; kk++) {
    if (dlCartesianPlot->axis[kk].rangeStyle==CHANNEL_RANGE) {
      pcp->axisRange[kk].isCurrentlyFromChannel= True;
    } else {
      pcp->axisRange[kk].isCurrentlyFromChannel = False;
    }
  }
  minXF = minX;
  maxXF = maxX;
  minYF = minY;
  maxYF = maxY;
  minY2F = minY2;
  maxY2F = maxY2;
  n = 0;
  if (dlCartesianPlot->axis[X_AXIS_ELEMENT].rangeStyle == CHANNEL_RANGE) {

    XtSetArg(widgetArgs[n],XtNxrtXMin,XrtFloatToArgVal(minXF)); n++;
    XtSetArg(widgetArgs[n],XtNxrtXMax,XrtFloatToArgVal(maxXF)); n++;

  } else
  if (dlCartesianPlot->axis[X_AXIS_ELEMENT].rangeStyle
                                        == USER_SPECIFIED_RANGE) {
    int k;
    minXF = dlCartesianPlot->axis[X_AXIS_ELEMENT].minRange;
    maxXF = dlCartesianPlot->axis[X_AXIS_ELEMENT].maxRange;

    tickF = (maxXF - minXF)/4.0;
    XtSetArg(widgetArgs[n],XtNxrtXMin,XrtFloatToArgVal(minXF)); n++;
    XtSetArg(widgetArgs[n],XtNxrtXMax,XrtFloatToArgVal(maxXF)); n++;
    XtSetArg(widgetArgs[n],XtNxrtXTick,XrtFloatToArgVal(tickF)); n++;
    XtSetArg(widgetArgs[n],XtNxrtXNum,XrtFloatToArgVal(tickF)); n++;
    sprintf(string,"%f",tickF);
    k = STRLEN(string)-1;
    while (string[k] == '0') k--; /* strip off trailing zeroes */
    iPrec = k;
    while (string[k] != '.' && k >= 0) k--;
    iPrec = iPrec - k;
    XtSetArg(widgetArgs[n],XtNxrtXPrecision,iPrec); n++;

  }

  if (dlCartesianPlot->axis[Y1_AXIS_ELEMENT].rangeStyle
                                        == CHANNEL_RANGE) {

    XtSetArg(widgetArgs[n],XtNxrtYMin,XrtFloatToArgVal(minYF)); n++;
    XtSetArg(widgetArgs[n],XtNxrtYMax,XrtFloatToArgVal(maxYF)); n++;

  } else
  if (dlCartesianPlot->axis[Y1_AXIS_ELEMENT].rangeStyle
                                        == USER_SPECIFIED_RANGE) {

    int k;
    minYF = dlCartesianPlot->axis[Y1_AXIS_ELEMENT].minRange;
    maxYF = dlCartesianPlot->axis[Y1_AXIS_ELEMENT].maxRange;

    tickF = (maxYF - minYF)/4.0;
    XtSetArg(widgetArgs[n],XtNxrtYMin,XrtFloatToArgVal(minYF)); n++;
    XtSetArg(widgetArgs[n],XtNxrtYMax,XrtFloatToArgVal(maxYF)); n++;
    XtSetArg(widgetArgs[n],XtNxrtYTick,XrtFloatToArgVal(tickF)); n++;
    XtSetArg(widgetArgs[n],XtNxrtYNum,XrtFloatToArgVal(tickF)); n++;
    sprintf(string,"%f",tickF);
    k = STRLEN(string)-1;
    while (string[k] == '0') k--; /* strip off trailing zeroes */
    iPrec = k;
    while (string[k] != '.' && k >= 0) k--;
      iPrec = iPrec - k;
      XtSetArg(widgetArgs[n],XtNxrtYPrecision,iPrec); n++;

  }

  if (dlCartesianPlot->axis[Y2_AXIS_ELEMENT].rangeStyle
                                        == CHANNEL_RANGE) {
    XtSetArg(widgetArgs[n],XtNxrtY2Min,XrtFloatToArgVal(minY2F)); n++;
    XtSetArg(widgetArgs[n],XtNxrtY2Max,XrtFloatToArgVal(maxY2F)); n++;

  } else
  if (dlCartesianPlot->axis[Y2_AXIS_ELEMENT].rangeStyle
                                        == USER_SPECIFIED_RANGE) {

    int k;
    minY2F = dlCartesianPlot->axis[Y2_AXIS_ELEMENT].minRange;
    maxY2F = dlCartesianPlot->axis[Y2_AXIS_ELEMENT].maxRange;

    tickF = (maxY2F - minY2F)/4.0;
    XtSetArg(widgetArgs[n],XtNxrtY2Min,XrtFloatToArgVal(minY2F)); n++;
    XtSetArg(widgetArgs[n],XtNxrtY2Max,XrtFloatToArgVal(maxY2F)); n++;
    XtSetArg(widgetArgs[n],XtNxrtY2Tick,XrtFloatToArgVal(tickF)); n++;
    XtSetArg(widgetArgs[n],XtNxrtY2Num,XrtFloatToArgVal(tickF)); n++;
    sprintf(string,"%f",tickF);
    k = STRLEN(string)-1;
    while (string[k] == '0') k--; /* strip off trailing zeroes */
    iPrec = k;
    while (string[k] != '.' && k >= 0) k--;
    iPrec = iPrec - k;
    XtSetArg(widgetArgs[n],XtNxrtY2Precision,iPrec); n++;

  }

  XtSetValues(widget,widgetArgs,n);

  /* add destroy callback */
  /* not freeing monitorData here, rather using monitorData for
     it's information */
  pcp->xrtData1 = xrtData1;
  pcp->xrtData2 = xrtData2;
#if 0
  XtAddCallback(widget,XmNdestroyCallback,
          (XtCallbackProc)monitorDestroy, (XtPointer)pcp);
#endif
  cartesianPlotUpdateScreenFirstTime(cd);
}

void cartesianPlotUpdateTrace(XtPointer cd) {
  Record *pd = (Record *) cd;
  XYTrace *pt = (XYTrace *) pd->clientData;
  CartesianPlot *pcp = pt->cartesianPlot;
  DlCartesianPlot *dlCartesianPlot = pcp->dlElement->structure.cartesianPlot;
  int count, j;

  switch(pt->type) {
  case CP_XYScalar:
    /* X: x & y channels specified - scalars, up to dlCartesianPlot->count pairs */
    count = pt->xrtData->g.data[pt->trace].npoints;
    if (count <= dlCartesianPlot->count-1) {
      pt->xrtData->g.data[pt->trace].xp[count] = (float) pt->recordX->value;
      pt->xrtData->g.data[pt->trace].yp[count] = (float) pt->recordY->value;
      pt->xrtData->g.data[pt->trace].npoints++;
    } else
    if (count > dlCartesianPlot->count-1) {
      if (dlCartesianPlot->erase_oldest == ERASE_OLDEST_OFF) {
	/* don't update */
      } else
      if (dlCartesianPlot->erase_oldest == ERASE_OLDEST_ON) {
        /* shift everybody down one, add at end */
        int j;
        for (j = 1; j < dlCartesianPlot->count; j++) {
          pt->xrtData->g.data[pt->trace].xp[j-1] =
                   pt->xrtData->g.data[pt->trace].xp[j];
          pt->xrtData->g.data[pt->trace].yp[j-1] =
                   pt->xrtData->g.data[pt->trace].yp[j];
        }
        pt->xrtData->g.data[pt->trace].xp[j-1] = (float) pt->recordX->value;
        pt->xrtData->g.data[pt->trace].yp[j-1] = (float) pt->recordY->value;
        pt->xrtData->g.data[pt->trace].npoints = dlCartesianPlot->count;
      }
    }
    break;

  case CP_XScalar:
    /* x channel scalar, up to dlCartesianPlot->count pairs */
    count = pt->xrtData->g.data[pt->trace].npoints;
    if (count <= dlCartesianPlot->count-1) {
      pt->xrtData->g.data[pt->trace].xp[count] = (float) pt->recordX->value;
      pt->xrtData->g.data[pt->trace].yp[count] = (float) count;
      pt->xrtData->g.data[pt->trace].npoints++;
    } else
    if (count > dlCartesianPlot->count-1) {
      if (dlCartesianPlot->erase_oldest == ERASE_OLDEST_OFF) {
        /* don't update */
      } else
      if (dlCartesianPlot->erase_oldest == ERASE_OLDEST_ON) {
        /* shift everybody down one, add at end */
	int j;
        for (j = 1; j < dlCartesianPlot->count; j++) {
          pt->xrtData->g.data[pt->trace].xp[j-1] =
                   pt->xrtData->g.data[pt->trace].xp[j];
          pt->xrtData->g.data[pt->trace].yp[j-1] =
                   (float) (j-1);
        }
        pt->xrtData->g.data[pt->trace].xp[j-1] = (float) pt->recordX->value;
        pt->xrtData->g.data[pt->trace].yp[j-1] = (float) (j-1);
      }
    }
    break;


  case CP_XVector:
    /* x channel vector, ca_element_count(chid) elements */
    switch(pt->recordX->dataType) {
      case DBF_STRING:
      {
        for (j = 0; j < pt->recordX->elementCount; j++) {
          pt->xrtData->g.data[pt->trace].xp[j] = 0.0;
          pt->xrtData->g.data[pt->trace].yp[j] = (float) j;
        }
        break;
      }
      case DBF_INT:
      {
        short *pShort = (short *) pt->recordX->array;
        for (j = 0; j < pt->recordX->elementCount; j++) {
          pt->xrtData->g.data[pt->trace].xp[j] = (float) pShort[j];
          pt->xrtData->g.data[pt->trace].yp[j] = (float) j;
        }
        break;
      }
      case DBF_FLOAT:
      {
        float *pFloat = (float *) pt->recordX->array;
        for (j = 0; j < pt->recordX->elementCount; j++) {
          pt->xrtData->g.data[pt->trace].xp[j] = (float) pFloat[j];
          pt->xrtData->g.data[pt->trace].yp[j] = (float) j;
        }
        break;
      }
      case DBF_ENUM:
      {
        unsigned short *pUShort = (unsigned short *) pt->recordX->array;
        for (j = 0; j < pt->recordX->elementCount; j++) {
          pt->xrtData->g.data[pt->trace].xp[j] = (float) pUShort[j];
          pt->xrtData->g.data[pt->trace].yp[j] = (float) j;
        }
        break;
      }
      case DBF_CHAR:
      {
        unsigned char *ptar = (unsigned char *) pt->recordX->array;
        for (j = 0; j < pt->recordX->elementCount; j++) {
          pt->xrtData->g.data[pt->trace].xp[j] = (float) ptar[j];
          pt->xrtData->g.data[pt->trace].yp[j] = (float) j;
        }
        break;
      }
      case DBF_LONG:
      {
        dbr_long_t *pLong = (dbr_long_t *) pt->recordX->array;
        for (j = 0; j < pt->recordX->elementCount; j++) {
          pt->xrtData->g.data[pt->trace].xp[j] = (float) pLong[j];
          pt->xrtData->g.data[pt->trace].yp[j] = (float) j;
        }
        break;
      }
      case DBF_DOUBLE:
      {
        double *pDouble = (double *) pt->recordX->array;
        for (j = 0; j < pt->recordX->elementCount; j++) {
          pt->xrtData->g.data[pt->trace].xp[j] =
	     (float) pDouble[j];
          pt->xrtData->g.data[pt->trace].yp[j] = 
	     (float) j;
        }
        break;
      }
    }
    pt->xrtData->g.data[pt->trace].npoints = pt->recordX->elementCount;
    break;

  case CP_YScalar:
    /* y channel scalar, up to dlCartesianPlot->count pairs */
    count = pt->xrtData->g.data[pt->trace].npoints;
    if (count < dlCartesianPlot->count) {
      float *px = pt->xrtData->g.data[pt->trace].xp;
      float *py = pt->xrtData->g.data[pt->trace].yp;
      if (!count && pcp->timeScale) {
        time_t tval;
        tval = timeOffset + (time_t) pt->recordY->time.secPastEpoch;
        XtVaSetValues(pcp->dlElement->widget,
                      XtNxrtTimeBase,tval,
                      NULL);
        pcp->startTime = pt->recordY->time;
      }
      px[count] = (pcp->timeScale) ?
        (float) (pt->recordY->time.secPastEpoch-pcp->startTime.secPastEpoch) :
        (float) count;
      py[count] = (float) pt->recordY->value;
      pt->xrtData->g.data[pt->trace].npoints++;
    } else
    if (count >= dlCartesianPlot->count) {
      if (dlCartesianPlot->erase_oldest == ERASE_OLDEST_OFF) {
        /* Don't update */
      } else
      if (dlCartesianPlot->erase_oldest == ERASE_OLDEST_ON) {
        /* shift everybody down one, add at end */
	int j;
        float *px = pt->xrtData->g.data[pt->trace].xp;
        float *py = pt->xrtData->g.data[pt->trace].yp;
        if (pcp->timeScale) {
          for (j = 1; j < dlCartesianPlot->count; j++) {
            px[j-1] = px[j];
            py[j-1] = py[j];
          }
          px[j-1] = (float) (pt->recordY->time.secPastEpoch -
                    pcp->startTime.secPastEpoch); 
          py[j-1] = (float) pt->recordY->value;
        } else {
          for (j = 1; j < dlCartesianPlot->count; j++) {
            py[j-1] = py[j];
          }
          py[j-1] = (float) pt->recordY->value;
        } 
      }
    }
    break;
  case CP_YVector:
    /* plot first "count" elements of vector per dlCartesianPlot*/
    switch(pt->recordY->dataType) {
    case DBF_STRING:
    {
      for (j = 0; j < pt->recordY->elementCount; j++) {
        pt->xrtData->g.data[pt->trace].yp[j] = 0.0;
        pt->xrtData->g.data[pt->trace].xp[j] = (float) j;
      }
      break;
    }
    case DBF_INT:
    {
      short *pShort = (short *) pt->recordY->array;
      for (j = 0; j < pt->recordY->elementCount; j++) {
        pt->xrtData->g.data[pt->trace].yp[j] = (float) pShort[j];
        pt->xrtData->g.data[pt->trace].xp[j] = (float) j;
      }
      break;
    }
    case DBF_FLOAT:
    {
      float *pFloat = (float *) pt->recordY->array;
      for (j = 0; j < pt->recordY->elementCount; j++) {
        pt->xrtData->g.data[pt->trace].yp[j] = (float) pFloat[j];
        pt->xrtData->g.data[pt->trace].xp[j] = (float) j;
      }
      break;
    }
    case DBF_ENUM:
    {
      unsigned short *pUShort = (unsigned short *) pt->recordY->array;
      for (j = 0; j < pt->recordY->elementCount; j++) {
        pt->xrtData->g.data[pt->trace].yp[j] = (float) pUShort[j];
        pt->xrtData->g.data[pt->trace].xp[j] = (float) j;
      }
      break;
    }
    case DBF_CHAR:
    {
      unsigned char *ptar = (unsigned char *) pt->recordY->array;
      for (j = 0; j < pt->recordY->elementCount; j++) {
        pt->xrtData->g.data[pt->trace].yp[j] = (float) ptar[j];
        pt->xrtData->g.data[pt->trace].xp[j] = (float) j;
      }
      break;
    }
    case DBF_LONG:
    {
      dbr_long_t *pLong = (dbr_long_t *) pt->recordY->array;
      for (j = 0; j < pt->recordY->elementCount; j++) {
        pt->xrtData->g.data[pt->trace].yp[j] = (float) pLong[j];
        pt->xrtData->g.data[pt->trace].xp[j] = (float) j;
      }
      break;
    }
    case DBF_DOUBLE:
    {
      double *pDouble = (double *) pt->recordY->array;
      for (j = 0; j < pt->recordY->elementCount; j++) {
        pt->xrtData->g.data[pt->trace].yp[j] =
	   (float) pDouble[j];
        pt->xrtData->g.data[pt->trace].xp[j] =
	   (float) j;
      }
      break;
    }
  }
  pt->xrtData->g.data[pt->trace].npoints = pt->recordY->elementCount;
  break;


  case CP_XVectorYScalar:
    pt->xrtData->g.data[pt->trace].npoints = pt->recordX->elementCount;
    if (pd == pt->recordX) {
      /* plot first "count" elements of vector per dlCartesianPlot*/
      switch(pt->recordX->dataType) {
        case DBF_STRING:
        {
          for (j = 0; j < pt->recordX->elementCount; j++) {
            pt->xrtData->g.data[pt->trace].xp[j] = 0.0;
          }
          break;
        }
        case DBF_INT:
        {
          short *pShort = (short *) pt->recordX->array;
          for (j = 0; j < pt->recordX->elementCount; j++) {
            pt->xrtData->g.data[pt->trace].xp[j] = (float) pShort[j];
          }
          break;
        }
        case DBF_FLOAT:
        {
          float *pFloat = (float *) pt->recordX->array;
          for (j = 0; j < pt->recordX->elementCount; j++) {
            pt->xrtData->g.data[pt->trace].xp[j] = pFloat[j];
          }
          break;
        }
        case DBF_ENUM:
        {
          unsigned short *pUShort = (unsigned short *) pt->recordX->array;
          for (j = 0; j < pt->recordX->elementCount; j++) {
            pt->xrtData->g.data[pt->trace].xp[j] = (float) pUShort[j];
          }
          break;
        }
        case DBF_CHAR:
        {
          unsigned char *ptar = (unsigned char *) pt->recordX->array;
          for (j = 0; j < pt->recordX->elementCount; j++) {
            pt->xrtData->g.data[pt->trace].xp[j] = (float) ptar[j];
          }
          break;
        }
        case DBF_LONG:
        {
          dbr_long_t *pLong = (dbr_long_t *) pt->recordX->array;
          for (j = 0; j < pt->recordX->elementCount; j++) {
            pt->xrtData->g.data[pt->trace].xp[j] = (float) pLong[j];
          }
          break;
        }
        case DBF_DOUBLE:
        {
          double *pDouble = (double *) pt->recordX->array;
          for (j = 0; j < pt->recordX->elementCount; j++) {
            pt->xrtData->g.data[pt->trace].xp[j] = (double) pDouble[j];
          }
          break;
        }
      }
    } else 
    if (pd == pt->recordY) {
      for (j = 0; j < pt->recordX->elementCount; j++) {
        pt->xrtData->g.data[pt->trace].yp[j] = (float) pt->recordY->value;
      }
    }
    break;

  case CP_YVectorXScalar:
    pt->xrtData->g.data[pt->trace].npoints = pt->recordY->elementCount;
    if (pd == pt->recordY) {
      /* plot first "count" elements of vector per dlCartesianPlot*/
      switch(pt->recordY->dataType) {
        case DBF_STRING:
        {
          for (j = 0; j < pt->recordY->elementCount; j++) {
            pt->xrtData->g.data[pt->trace].yp[j] = 0.0;
          }
          break;
        }
        case DBF_INT:
        {
          short *pShort = (short *) pt->recordY->array;
          for (j = 0; j < pt->recordY->elementCount; j++) {
            pt->xrtData->g.data[pt->trace].yp[j] = (float) pShort[j];
          }
          break;
        }
        case DBF_FLOAT:
        {
          float *pFloat = (float *) pt->recordY->array;
          for (j = 0; j < pt->recordY->elementCount; j++) {
            pt->xrtData->g.data[pt->trace].yp[j] = pFloat[j];
          }
          break;
        }
        case DBF_ENUM:
        {
          unsigned short *pUShort = (unsigned short *) pt->recordY->array;
          for (j = 0; j < pt->recordY->elementCount; j++) {
            pt->xrtData->g.data[pt->trace].yp[j] = (float) pUShort[j];
          }
          break;
        }
        case DBF_CHAR:
        {
          unsigned char *ptar = (unsigned char *) pt->recordY->array;
          for (j = 0; j < pt->recordY->elementCount; j++) {
            pt->xrtData->g.data[pt->trace].yp[j] = (float) ptar[j];
          }
          break;
        }
        case DBF_LONG:
        {
          dbr_long_t *pLong = (dbr_long_t *) pt->recordY->array;
          for (j = 0; j < pt->recordY->elementCount; j++) {
            pt->xrtData->g.data[pt->trace].yp[j] = (float) pLong[j];
          }
          break;
        }
        case DBF_DOUBLE:
          {
          double *pDouble = (double *) pt->recordY->array;
          for (j = 0; j < pt->recordY->elementCount; j++) {
            pt->xrtData->g.data[pt->trace].yp[j] = (double) pDouble[j];
          }
          break;
        }
      }
    } else 
    if (pd == pt->recordX) {
      for (j = 0; j < pt->recordY->elementCount; j++) {
        pt->xrtData->g.data[pt->trace].xp[j] = (float) pt->recordX->value;
      }
    }
    break;

  case CP_XYVector: {
    float *pfa;
    int count;

    count = MIN(pt->recordX->elementCount, pt->recordY->elementCount);
    pt->xrtData->g.data[pt->trace].npoints = count;

    if (pd == pt->recordX) {
      pfa = pt->xrtData->g.data[pt->trace].xp;
    } else 
    if (pd == pt->recordY) {
      pfa = pt->xrtData->g.data[pt->trace].yp;
    } else {
      /* don't do anything */
      break;
    }
    if (pcp->timeScale && pd == pt->recordX) {
      if (pcp->startTime.nsec) {
      } else {
        pcp->startTime.secPastEpoch = (int) pd->value;
        pcp->startTime.nsec = 1;
        XtVaSetValues(pcp->dlElement->widget,
                      XtNxrtTimeBase, timeOffset + pcp->startTime.secPastEpoch,
                      NULL);
        printf("pcp->startTime = %d\n",pcp->startTime.secPastEpoch);
      }
      switch(pd->dataType) {
        case DBF_LONG:
          {
            dbr_long_t *pLong = (dbr_long_t *) pd->array;
            dbr_long_t offset = (dbr_long_t) pcp->startTime.secPastEpoch;
            for (j = 0; j < count; j++) {
              pfa[j] = (float) (pLong[j] - offset);
            }
            break;
          }
        case DBF_DOUBLE:
          {
            double *pDouble = (double *) pd->array;
            double offset = (double) pcp->startTime.secPastEpoch;
            for (j = 0; j < count; j++) {
              pfa[j] = (float) (pDouble[j] - offset);
            }
            break;
          }
        default :
          break;
      }
    } else {
      switch(pd->dataType) {
        case DBF_STRING:
          for (j = 0; j < count; j++) {
            pfa[j] = 0.0;
          }
          break;
        case DBF_INT:
          {
            short *pShort = (short *) pd->array;
            for (j = 0; j < count; j++) {
              pfa[j] = (float) pShort[j];
            }
            break;
          }
        case DBF_FLOAT:
          {
            float *pFloat = (float *) pd->array;
            for (j = 0; j < count; j++) {
              pfa[j] = pFloat[j];
            }
            break;
          }
        case DBF_ENUM:
          {
            unsigned short *pUShort = (unsigned short *) pd->array;
            for (j = 0; j < count; j++) {
              pfa[j] = (float) pUShort[j];
            }
            break;
          }
        case DBF_CHAR:
          {
            unsigned char *pChar = (unsigned char *) pd->array;
            for (j = 0; j < count; j++) {
              pfa[j] = (float) pChar[j];
            }
            break;
          }
        case DBF_LONG:
          {
            dbr_long_t *pLong = (dbr_long_t *) pd->array;
            for (j = 0; j < count; j++) {
              pfa[j] = (float) pLong[j];
            }
            break;
          }
        case DBF_DOUBLE:
          {
            double *pDouble = (double *) pd->array;
            for (j = 0; j < count; j++) {
              pfa[j] = (float) pDouble[j];
            }
            break;
          }
      }
    }
    break;
  }
  default:
      dm2kPrintf("\ncartesianPlotUpdateTrace: unhandled CP case\n");
  }
}

static void cartesianPlotUpdateScreenFirstTime(XtPointer cd) {
  Record *pd = (Record *) cd;
  XYTrace *pt = (XYTrace *) pd->clientData;
  CartesianPlot *pcp = pt->cartesianPlot;
  Widget widget = pcp->dlElement->widget;
  int i;
  Boolean clearDataSet1 = True;
  Boolean clearDataSet2 = True;

  /* return until all channels get their graphical information */
  for (i = 0; i < pcp->nTraces; i++) {
    XYTrace *t = &(pcp->xyTrace[i]);
    if ((t->recordX) || (t->recordY)) {
      if (t->xrtData == NULL) return;
      if ((t->recordX) &&
         ((!t->recordX->array) || (t->recordX->precision < 0))) return;
      if ((t->recordY) &&
         ((!t->recordY->array) || (t->recordY->precision < 0))) return;
    }
  }
  if (pcp->triggerCh.recordX) {
    if (pcp->triggerCh.recordX->precision < 0) return;
  }
  if (pcp->eraseCh.recordX) {
    if (pcp->eraseCh.recordX->precision < 0) return;
  }
  /* draw all plots once */
  for (i = 0; i < pcp->nTraces; i++) {
    XYTrace *t = &(pcp->xyTrace[i]);
    if (t->recordX) {
      cartesianPlotUpdateTrace((XtPointer)t->recordX);
      if (t->type == CP_XYVector)
        cartesianPlotUpdateTrace((XtPointer)t->recordY);
    } else
    if (t->recordY) {
      cartesianPlotUpdateTrace((XtPointer)t->recordY);
    } else {
      continue;
    }
    if (t->xrtData == pcp->xrtData1)
      XtVaSetValues(widget,XtNxrtData,t->xrtData,NULL);
    else
    if (t->xrtData == pcp->xrtData2)
      XtVaSetValues(widget,XtNxrtData2,t->xrtData,NULL);
  }
  /* erase the plot */
  if (pcp->eraseCh.recordX) {
    Record *pr = pcp->eraseCh.recordX;
    if (((pr->value == 0.0) && (pcp->eraseMode == ERASE_IF_ZERO))
      || ((pr->value != 0.0) && (pcp->eraseMode == ERASE_IF_NOT_ZERO))) {
      for (i = 0; i < pcp->nTraces; i++) {
        XYTrace *t = &(pcp->xyTrace[i]);
        if ((t->recordX) || (t->recordY)) {
          int n = t->xrtData->g.data[t->trace].npoints;
          if (n > 0) {
            if ((t->xrtData == pcp->xrtData1) && (clearDataSet1)) {
              XtVaSetValues(widget,XtNxrtData,nullData,NULL);
              clearDataSet1 = False;
            } else
            if ((t->xrtData == pcp->xrtData2) && (clearDataSet2)) {
              XtVaSetValues(widget,XtNxrtData2,nullData,NULL);
              clearDataSet2 = False;
            }
            t->xrtData->g.data[t->trace].npoints = 0;
          }
        } 
      }
    }
  }
  /* switch back to regular update routine */
  for (i = 0; i < pcp->nTraces; i++) {
    XYTrace *t = &(pcp->xyTrace[i]);
    if (t->recordX) {
      dm2kRecordAddUpdateValueCb(t->recordX,cartesianPlotUpdateValueCb);
    }
    if (t->recordY) {
      dm2kRecordAddUpdateValueCb(t->recordY,cartesianPlotUpdateValueCb);
    }
  }
  if (pcp->eraseCh.recordX) {
    dm2kRecordAddUpdateValueCb(pcp->eraseCh.recordX,cartesianPlotUpdateValueCb);
  }
  if (pcp->triggerCh.recordX) {
    dm2kRecordAddUpdateValueCb(pcp->triggerCh.recordX,cartesianPlotUpdateValueCb);
  }
  updateTaskMarkUpdate(pcp->updateTask);
}


static void cartesianPlotUpdateValueCb(XtPointer cd) {
  Record *pd = (Record *) cd;
  XYTrace *pt = (XYTrace *) pd->clientData;
  CartesianPlot *pcp = pt->cartesianPlot;
  Widget widget = pcp->dlElement->widget;
  int i;

  /* if this is in trigger mode, and this update request is not from
   * the trigger, do not update.
   */

  /* if this is an erase channel, erase screen */
  if (pd == pcp->eraseCh.recordX) {
    Boolean clearDataSet1 = True;
    Boolean clearDataSet2 = True;

    /* not the right value, return */
    if (((pd->value == 0) && (pcp->eraseMode == ERASE_IF_NOT_ZERO))
	||((pd->value != 0) && (pcp->eraseMode == ERASE_IF_ZERO))) {
      return;
    }

    /* erase */
    for (i = 0; i < pcp->nTraces; i++) {
      XYTrace *t = &(pcp->xyTrace[i]);
      if ((t->recordX) || (t->recordY)) {
	int n = t->xrtData->g.data[t->trace].npoints;
        if (n > 0) {
	  if ((t->xrtData == pcp->xrtData1) && (clearDataSet1)) {
	    XtVaSetValues(widget,XtNxrtData,nullData,NULL);
	    clearDataSet1 = False;
            pcp->dirty1 = False;
	  } else
	  if ((t->xrtData == pcp->xrtData2) && (clearDataSet2)) {
	    XtVaSetValues(widget,XtNxrtData2,nullData,NULL);
	    clearDataSet2 = False;
            pcp->dirty2 = False;
	  }
	  t->xrtData->g.data[t->trace].npoints = 0;
	}
      }
    }
    updateTaskMarkUpdate(pcp->updateTask);
    return;
  }

  /* if there is a trigger channel, but this is not the one, return */
  if (pcp->triggerCh.recordX) {
    if (pd != pcp->triggerCh.recordX)
      return;
  }

  if (pcp->triggerCh.recordX) {
    /* trigger channel monitor, update appropriate plots */
    for (i = 0; i < pcp->nTraces; i++) {
      XYTrace *t = &(pcp->xyTrace[i]);
      if ((t->recordX == NULL) && (t->recordY == NULL)) continue;
      if ((t->recordX) && (t->xrtData)) {
        cartesianPlotUpdateTrace((XtPointer)t->recordX);
        if (t->type == CP_XYVector)
          cartesianPlotUpdateTrace((XtPointer)t->recordY);
        if (t->xrtData == pcp->xrtData1)
          pcp->dirty1 = True;
	else
	if (t->xrtData == pcp->xrtData2)
          pcp->dirty2 = True;
      } else
      if ((t->recordY) && (t->xrtData)) {
        cartesianPlotUpdateTrace((XtPointer)t->recordY);
	if (t->xrtData == pcp->xrtData1)
          pcp->dirty1 = True;
	else
	if (t->xrtData == pcp->xrtData2)
          pcp->dirty2 = True;
      }
    }
  } else {
    cartesianPlotUpdateTrace((XtPointer)pd);
    /* not in CP related to trigger channel, proceed as normal */
    if (pt->xrtData == pcp->xrtData1) {
      pcp->dirty1 = True;
    } else
    if (pt->xrtData == pcp->xrtData2) {
      pcp->dirty2 = True;
    } else
      dm2kPrintf("\ncartesianPlotUpdateScreen : illegal xrtDataSet specified\n");
  }
  updateTaskMarkUpdate(pcp->updateTask);
}

static void cartesianPlotDestroyCb(XtPointer cd) {
  CartesianPlot *pcp = (CartesianPlot *) cd;
  if (executeTimeCartesianPlotWidget == pcp->dlElement->widget) {
    executeTimeCartesianPlotWidget = NULL;
    XtSetSensitive(cartesianPlotAxisS,False);
  }
  if (pcp) {
    int i;
    for (i = 0; i < pcp->nTraces; i++) {
      Record *pr;
      if ((pr = pcp->xyTrace[i].recordX)) {
        dm2kDestroyRecord(pr);
      }
      if ((pr = pcp->xyTrace[i].recordY)) {
        dm2kDestroyRecord(pr);
      }
    }
    if (pcp->triggerCh.recordX)
      dm2kDestroyRecord(pcp->triggerCh.recordX);
    if (pcp->eraseCh.recordX)
      dm2kDestroyRecord(pcp->eraseCh.recordX);
    if (pcp->xrtData1) { XrtDestroyData(pcp->xrtData1, 1); pcp->xrtData1 = NULL; }
    if (pcp->xrtData2) { XrtDestroyData(pcp->xrtData2, 1); pcp->xrtData2 = NULL; }
    free((char *)pcp);
  }
  return;
}

static void cartesianPlotDraw(XtPointer cd) {
  CartesianPlot *pcp = (CartesianPlot *) cd;
  Widget widget = pcp->dlElement->widget;
  int i;
  Boolean connected = True;
  Boolean readAccess = True;

  /* check for connection */
  for (i = 0; i < pcp->nTraces; i++) {
    Record *pr;
    if ((pr = pcp->xyTrace[i].recordX)) {
      if (!pr->connected) {
	connected = False;
	break;
      } else
      if (!pr->readAccess)
        readAccess = False;
    }
    if ((pr = pcp->xyTrace[i].recordY)) {
      if (!pr->connected) {
	connected = False;
	break;
      } else
      if (!pr->readAccess)
        readAccess = False;
    }
  }

  if (pcp->triggerCh.recordX) {
    Record *pr = pcp->triggerCh.recordX;
    if (!pr->connected) {
      connected = False;
    } else
    if (!pr->readAccess)
      readAccess = False;
  }

  if (pcp->eraseCh.recordX) {
    Record *pr = pcp->eraseCh.recordX;
    if (!pr->connected) {
      connected = False;
    } else
    if (!pr->readAccess)
      readAccess = False;
  }

  if (connected) {
    if (readAccess) {
      if (widget) {
        if (pcp->dirty1) {
          pcp->dirty1 = False;
          XtVaSetValues(widget,XtNxrtData,pcp->xrtData1,NULL);
        }
        if (pcp->dirty2) {
          pcp->dirty2 = False;
          XtVaSetValues(widget,XtNxrtData2,pcp->xrtData2,NULL);
        }
        XtManageChild(widget);
      }
    } else {
      if (widget) {
        XtUnmanageChild(widget);
      }
      draw3DPane(pcp->updateTask,
          pcp->updateTask->displayInfo->colormap[
              pcp->dlElement->structure.cartesianPlot->plotcom.bclr]);
      draw3DQuestionMark(pcp->updateTask);
    }
  } else {
    if (widget)
      XtUnmanageChild(widget);
    drawWhiteRectangle(pcp->updateTask);
  }
}

static void cartesianPlotName(XtPointer cd, char **name, short *severity, int *count) {
  CartesianPlot *pcp = (CartesianPlot *) cd;
  int i, j;

  j = 0;
  for (i = 0; i < pcp->nTraces; i++) {
    XYTrace *pt = &(pcp->xyTrace[i]);
    if (pt->recordX) {
      name[j] = pt->recordX->name;
      severity[j] = pt->recordX->severity;
    } else {
      name[j] = NULL;
      severity[j] = NO_ALARM;
    }
    j++;
    if (pt->recordY) {
      name[j] = pt->recordY->name;
      severity[j] = pt->recordY->severity;
    } else {
      name[j] = NULL;
      severity[j] = NO_ALARM;
    }
    j++;
  }
  if (pcp->triggerCh.recordX) {
    name[j] = pcp->triggerCh.recordX->name;
    severity[j] = pcp->triggerCh.recordX->severity;
  } else {
    name[j] = NULL;
    severity[j] = NO_ALARM;
  }
  j++;
  if (pcp->eraseCh.recordX) {
    name[j] = pcp->eraseCh.recordX->name;
    severity[j] = pcp->eraseCh.recordX->severity;
  } else {
    name[j] = NULL;
    severity[j] = NO_ALARM;
  }
  j++;
  /* 200 means two columns */
  *count = j + 200;
}

DlElement *createDlCartesianPlot(DlElement *p)
{
  DlCartesianPlot *dlCartesianPlot;
  DlElement *dlElement;
  int traceNumber;

  dlCartesianPlot = DM2KALLOC (DlCartesianPlot);

  if (dlCartesianPlot == NULL)  {
    dm2kPrintf("createDlCartesianPlote : memory allocation error\n");
    return NULL;
  }

  if (p != NULL) {
    objectAttributeCopy(&(dlCartesianPlot->object),
			&(p->structure.cartesianPlot->object));
    plotcomAttributeCopy(&(dlCartesianPlot->plotcom),
			 &(p->structure.cartesianPlot->plotcom));

    dlCartesianPlot->count = p->structure.cartesianPlot->count;
    dlCartesianPlot->style = p->structure.cartesianPlot->style;
    dlCartesianPlot->erase_oldest = p->structure.cartesianPlot->erase_oldest;

    for (traceNumber = 0; traceNumber < MAX_TRACES; traceNumber++)
      traceAttributeCopy
	((DlTrace *)&(dlCartesianPlot->trace[traceNumber]),
	 (DlTrace *)&(p->structure.cartesianPlot->trace[traceNumber]));

    plotAxisDefinitionCopy
      (&(dlCartesianPlot->axis[X_AXIS_ELEMENT]),
       &(p->structure.cartesianPlot->axis[X_AXIS_ELEMENT]));

    plotAxisDefinitionCopy
      (&(dlCartesianPlot->axis[Y1_AXIS_ELEMENT]),
       &(p->structure.cartesianPlot->axis[Y1_AXIS_ELEMENT]));

    plotAxisDefinitionCopy
      (&(dlCartesianPlot->axis[Y2_AXIS_ELEMENT]),
       &(p->structure.cartesianPlot->axis[Y2_AXIS_ELEMENT]));


    renewString(&dlCartesianPlot->trigger, 
		p->structure.cartesianPlot->trigger);

    renewString(&dlCartesianPlot->erase, p->structure.cartesianPlot->erase);

    dlCartesianPlot->eraseMode = p->structure.cartesianPlot->eraseMode;
  } 
  else {
    objectAttributeInit(&(dlCartesianPlot->object));
    plotcomAttributeInit(&(dlCartesianPlot->plotcom));

    plotAxisDefinitionInit(&(dlCartesianPlot->axis[X_AXIS_ELEMENT]));
    plotAxisDefinitionInit(&(dlCartesianPlot->axis[Y1_AXIS_ELEMENT]));
    plotAxisDefinitionInit(&(dlCartesianPlot->axis[Y2_AXIS_ELEMENT]));

    for (traceNumber = 0; traceNumber < MAX_TRACES; traceNumber++)
        traceAttributeInit((DlTrace *)&(dlCartesianPlot->trace[traceNumber]));

    dlCartesianPlot->count         = 1;
    dlCartesianPlot->style        = POINT_PLOT;
    dlCartesianPlot->erase_oldest = ERASE_OLDEST_OFF;
    dlCartesianPlot->trigger      = NULL;
    dlCartesianPlot->erase        = NULL;
    dlCartesianPlot->eraseMode    = ERASE_IF_NOT_ZERO;
  }

  dlElement = createDlElement(DL_CartesianPlot,
			      (XtPointer)      dlCartesianPlot,
			      &cartesianPlotDlDispatchTable);

  if (dlElement == NULL)
    destroyDlCartesianPlot(dlCartesianPlot);

  return(dlElement);
}

DlElement *parseCartesianPlot(DisplayInfo *displayInfo)
{
  char              token[MAX_TOKEN_LENGTH];
  TOKEN             tokenType;
  int               nestingLevel = 0;
  DlCartesianPlot * dlCartesianPlot;
  DlElement       * dlElement = createDlCartesianPlot(NULL);
  int               traceNumber;

  if (dlElement == NULL)
    return 0;

  dlCartesianPlot = dlElement->structure.cartesianPlot;
  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"object"))
	parseObject(displayInfo,&(dlCartesianPlot->object));
      else if (STREQL(token,"plotcom"))
	parsePlotcom(displayInfo,&(dlCartesianPlot->plotcom));
      else if (STREQL(token,"count")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	dlCartesianPlot->count= atoi(token);
      } else if (STREQL(token,"style")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	if (STREQL(token,"point plot")) 
	  dlCartesianPlot->style = POINT_PLOT;
	else if (STREQL(token,"point")) 
	  dlCartesianPlot->style = POINT_PLOT;
	else if (STREQL(token,"line plot")) 
	  dlCartesianPlot->style = LINE_PLOT;
	else if (STREQL(token,"line only")) 
	  dlCartesianPlot->style = PURELINE_PLOT;
	else if (STREQL(token,"line")) 
	  dlCartesianPlot->style = LINE_PLOT;
	else if (STREQL(token,"fill under")) 
	  dlCartesianPlot->style = FILL_UNDER_PLOT;
	else if (STREQL(token,"fill-under")) 
	  dlCartesianPlot->style = FILL_UNDER_PLOT;
      } else if (STREQL(token,"erase_oldest")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	if (STREQL(token,"on")) 
	  dlCartesianPlot->erase_oldest = ERASE_OLDEST_ON;
	else if (STREQL(token,"off")) 
	  dlCartesianPlot->erase_oldest = ERASE_OLDEST_OFF;
	else if (STREQL(token,"plot last n pts")) 
	  dlCartesianPlot->erase_oldest = ERASE_OLDEST_ON;
	else if (STREQL(token,"plot n pts & stop")) 
	  dlCartesianPlot->erase_oldest = ERASE_OLDEST_OFF;
      } else if (!strncmp(token,"trace",5)) {
	traceNumber = MIN(token[6] - '0', MAX_TRACES - 1);
	parseTrace(displayInfo,
		   &(dlCartesianPlot->trace[traceNumber]));
      } else if (STREQL(token,"x_axis")) {
	parsePlotAxisDefinition(displayInfo,
				&(dlCartesianPlot->axis[X_AXIS_ELEMENT]));
      } else if (STREQL(token,"y1_axis")) {
	parsePlotAxisDefinition(displayInfo,
				&(dlCartesianPlot->axis[Y1_AXIS_ELEMENT]));
      } else if (STREQL(token,"y2_axis")) {
	parsePlotAxisDefinition(displayInfo,
				&(dlCartesianPlot->axis[Y2_AXIS_ELEMENT]));
      } else if (STREQL(token,"trigger")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	renewString(&dlCartesianPlot->trigger,token);
      } else if (STREQL(token,"erase")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	renewString(&dlCartesianPlot->erase,token);
      } else if (STREQL(token,"eraseMode")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	if (STREQL(token,"if not zero"))
	  dlCartesianPlot->eraseMode = ERASE_IF_NOT_ZERO;
	else if (STREQL(token,"if zero"))
	  dlCartesianPlot->eraseMode = ERASE_IF_ZERO;
      }
      break;
    case T_EQUAL:
      break;
    case T_LEFT_BRACE:
		nestingLevel++; break;
    case T_RIGHT_BRACE:
      nestingLevel--; break;
    default:
      break;
    }
  } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
	    && (tokenType != T_EOF) );
  
  return dlElement;
}

void writeDlCartesianPlot(
  FILE *stream,
  DlElement *dlElement,
  int level)
{
  int i;
  char indent[256]; level=MIN(level,256-2);
  DlCartesianPlot *dlCartesianPlot = dlElement->structure.cartesianPlot;

  for (i = 0;  i < MIN(level,256-2); i++)
    indent[i] = '\t';
  indent[i] = '\0';

  fprintf(stream,"\n%s\"cartesian plot\" {",indent);
  writeDlObject(stream,&(dlCartesianPlot->object),level+1);
  writeDlPlotcom(stream,&(dlCartesianPlot->plotcom),level+1);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
  if (dlCartesianPlot->style != POINT_PLOT)
    fprintf(stream,"\n%s\tstyle=\"%s\"",indent,
        stringValueTable[dlCartesianPlot->style]);

  if (dlCartesianPlot->erase_oldest != ERASE_OLDEST_OFF)
    fprintf(stream,"\n%s\terase_oldest=\"%s\"",indent,
        stringValueTable[dlCartesianPlot->erase_oldest]);

  if (dlCartesianPlot->count != 1)
    fprintf(stream,"\n%s\tcount=\"%d\"",indent,dlCartesianPlot->count);

  for (i = 0; i < MAX_TRACES; i++) {
    writeDlTrace(stream,&(dlCartesianPlot->trace[i]),i,level+1);
  }

  writeDlPlotAxisDefinition(stream,&(dlCartesianPlot->axis[X_AXIS_ELEMENT]),
        X_AXIS_ELEMENT,level+1);

  writeDlPlotAxisDefinition(stream,&(dlCartesianPlot->axis[Y1_AXIS_ELEMENT]),
        Y1_AXIS_ELEMENT,level+1);

  writeDlPlotAxisDefinition(stream,&(dlCartesianPlot->axis[Y2_AXIS_ELEMENT]),
        Y2_AXIS_ELEMENT,level+1);

  if (dlCartesianPlot->trigger != NULL)
    fprintf(stream,"\n%s\ttrigger=\"%s\"",indent,dlCartesianPlot->trigger);

  if (dlCartesianPlot->erase != NULL)
    fprintf(stream,"\n%s\terase=\"%s\"",indent,dlCartesianPlot->erase);

  if (dlCartesianPlot->eraseMode != ERASE_IF_NOT_ZERO)
    fprintf(stream,"\n%s\teraseMode=\"%s\"",indent,
        stringValueTable[dlCartesianPlot->eraseMode]);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    fprintf(stream,"\n%s\tstyle=\"%s\"",indent,
	    stringValueTable[dlCartesianPlot->style]);
    fprintf(stream,"\n%s\terase_oldest=\"%s\"",indent,
	    stringValueTable[dlCartesianPlot->erase_oldest]);
    fprintf(stream,"\n%s\tcount=\"%d\"",indent,dlCartesianPlot->count);

    for (i = 0; i < MAX_TRACES; i++) {
      writeDlTrace(stream,&(dlCartesianPlot->trace[i]),i,level+1);
    }

    writeDlPlotAxisDefinition(stream,&(dlCartesianPlot->axis[X_AXIS_ELEMENT]),
        X_AXIS_ELEMENT,level+1);

    writeDlPlotAxisDefinition(stream,&(dlCartesianPlot->axis[Y1_AXIS_ELEMENT]),
        Y1_AXIS_ELEMENT,level+1);

    writeDlPlotAxisDefinition(stream,&(dlCartesianPlot->axis[Y2_AXIS_ELEMENT]),
        Y2_AXIS_ELEMENT,level+1);
    
    fprintf(stream,"\n%s\ttrigger=\"%s\"",indent,dlCartesianPlot->trigger);
    fprintf(stream,"\n%s\terase=\"%s\"",indent,dlCartesianPlot->erase);
    fprintf(stream,"\n%s\teraseMode=\"%s\"",indent,
	    stringValueTable[dlCartesianPlot->eraseMode]);
  }
#endif
  fprintf(stream,"\n%s}",indent);
}

static void cartesianPlotInheritValues(ResourceBundle *pRCB, DlElement *p) {
  DlCartesianPlot *dlCartesianPlot = p->structure.cartesianPlot;
  dm2kGetValues(pRCB,
    CLR_RC,        &(dlCartesianPlot->plotcom.clr),
    BCLR_RC,       &(dlCartesianPlot->plotcom.bclr),
    -1);
}

static void cartesianPlotGetValues(ResourceBundle *pRCB, DlElement *p) {
  DlCartesianPlot *dlCartesianPlot = p->structure.cartesianPlot;
  dm2kGetValues(pRCB,
    X_RC,          &(dlCartesianPlot->object.x),
    Y_RC,          &(dlCartesianPlot->object.y),
    WIDTH_RC,      &(dlCartesianPlot->object.width),
    HEIGHT_RC,     &(dlCartesianPlot->object.height),
    TITLE_RC,      &(dlCartesianPlot->plotcom.title),
    XLABEL_RC,     &(dlCartesianPlot->plotcom.xlabel),
    YLABEL_RC,     &(dlCartesianPlot->plotcom.ylabel),
    CLR_RC,        &(dlCartesianPlot->plotcom.clr),
    BCLR_RC,       &(dlCartesianPlot->plotcom.bclr),
    COUNT_RC,      &(dlCartesianPlot->count),
    CSTYLE_RC,     &(dlCartesianPlot->style),
    ERASE_OLDEST_RC,&(dlCartesianPlot->erase_oldest),
    CPDATA_RC,     &(dlCartesianPlot->trace),
    CPAXIS_RC,     &(dlCartesianPlot->axis),
    TRIGGER_RC,    &(dlCartesianPlot->trigger),
    ERASE_RC,      &(dlCartesianPlot->erase),
    ERASE_MODE_RC, &(dlCartesianPlot->eraseMode),
    -1);
}



#if 0
/* Information methode for Cartesian Plot object */
/* --------------------------------------------- */

static void  dm2kCartesianPlotInfoSimple (char *msg,
		      DlCartesianPlot *dlCartesianPlot)
{
  sprintf (&msg[STRLEN(msg)], " = %s", 
	   CARE_PRINT(dlCartesianPlot->trace[0].ydata));

  if ( dlCartesianPlot->trace[1].ydata )
    strcat (msg, " ...");
}


static void cartesianPlotInfo (char *msg, Widget w,
		   DisplayInfo *displayInfo,
		   DlElement *element,
		   XtPointer objet)
{
  CartesianPlot *pcp;
  Record *pd;
  char strg[512];
  int i;

  if (globalDisplayListTraversalMode != DL_EXECUTE) {
    dm2kCartesianPlotInfoSimple (msg, element->structure.cartesianPlot);
    return;
  }

  pcp = (CartesianPlot *) objet;
  for ( i = 0 ; i < pcp->nTraces ; i++ ) {
      pd = pcp->xyTrace[i].recordY;
      dm2kChanelInfo (strg, pd, NULL);
      sprintf (&msg[STRLEN(msg)], "\n\n  Plot y PV :\n%s", strg);
    }
}
#endif
#else

DlElement *parseCartesianPlot(DisplayInfo *displayInfo) {return NULL;}
DlElement *createDlCartesianPlot(DlElement *p) {return NULL;}

#endif
