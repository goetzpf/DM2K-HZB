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
 *                              - using new screen update dispatch mechanism
 * .03  09-12-95        vong    conform to c++ syntax
 *
 *****************************************************************************
 *
 *      18-03-97        Fabien  Add object info and enter leave window
 *
 *****************************************************************************
*/

#include "dm2k.h"

typedef struct _Indicator {
  DlElement   *dlElement;
  Record      *record;
  UpdateTask  *updateTask;
} Indicator;

static void indicatorUpdateValueCb(XtPointer cd);
static void indicatorDraw(XtPointer cd);
static void indicatorUpdateGraphicalInfoCb(XtPointer cd);
static void indicatorDestroyCb(XtPointer cd);
static void indicatorName(XtPointer, char **, short *, int *);
static void indicatorInheritValues(ResourceBundle *pRCB, DlElement *p);
static void indicatorGetValues(ResourceBundle *pRCB, DlElement *p);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);

static DlDispatchTable indicatorDlDispatchTable = {
       createDlIndicator,
       NULL,
       executeMethod,
       writeDlIndicator,
       NULL,
       indicatorGetValues,
       indicatorInheritValues,
       NULL,
       NULL,
       genericMove,
       genericScale,
       NULL,
       genericObjectInfo
};

static void destroyDlIndicator (DlIndicator * dlIndicator)
{
  if (dlIndicator == NULL)
    return;

  objectAttributeDestroy(&(dlIndicator->object));
  monitorAttributeDestroy(&(dlIndicator->monitor));

  free((char*)dlIndicator);
}

#if 0
static void destroyDlElement (DlElement * element)
{
  if (element == NULL || element->type != DL_Indicator) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  destroyDlIndicator (element->structure.indicator);
  free((char*)element);
}
#endif

static UpdateTask * executeMethod 
   (DisplayInfo * displayInfo, 
    DlElement   * dlElement)
{
  Arg              args[30];
  int              n;
  int              usedHeight, usedCharWidth, bestSize, preferredHeight;
  Widget           localWidget;
  Indicator      * pi;
  DlIndicator    * dlIndicator = dlElement->structure.indicator;
  WidgetUserData * userData;
  UpdateTask     * updateTask = NULL;

  /**2000/12 G.Lei**/
  XcVType xc_lower, xc_upper, xc_inc;

  xc_lower.fval = 0.0;
  xc_upper.fval =100.0;
  xc_inc.fval = 1.0;


  if (!dlElement->widget) 
    {
      if (displayInfo->traversalMode == DL_EXECUTE) 
	{
	  /* alloc servise structures
	   */
	  userData = DM2KALLOC (WidgetUserData);
	  pi = DM2KALLOC(Indicator);

	  if (pi == NULL || userData == NULL) {
	    DM2KFREE (pi);
	    DM2KFREE (userData);
	    dm2kPrintf("executeDlIndicator: memory allocation error\n");
	    return updateTask;
	  }

	  pi->dlElement = dlElement;
	  dlIndicator->object.runtimeDescriptor = (XtPointer) pi;

	  updateTask = pi->updateTask = 
	    updateTaskAddTask(displayInfo,
			      &(dlIndicator->object),
			      indicatorDraw,
			      (XtPointer)pi);

	  if (pi->updateTask == NULL) {
	    dm2kPrintf("indicatorCreateRunTimeInstance : memory alloc error\n");
	  } else {
	    updateTaskAddDestroyCb(pi->updateTask,indicatorDestroyCb);
	    updateTaskAddNameCb(pi->updateTask,indicatorName);
	  }
      
	  pi->record = dm2kAllocateRecord(dlIndicator->monitor.rdbk,
					  indicatorUpdateValueCb,
					  indicatorUpdateGraphicalInfoCb,
					  (XtPointer) pi);
	  drawWhiteRectangle(pi->updateTask);
	}
  
      /* from the indicator structure, we've got Indicator's specifics 
       */
      n = 0;
      XtSetArg(args[n],XtNx,        (Position)dlIndicator->object.x); n++;
      XtSetArg(args[n],XtNy,        (Position)dlIndicator->object.y); n++;
      XtSetArg(args[n],XtNwidth,    (Dimension)dlIndicator->object.width); n++;
      XtSetArg(args[n],XtNheight,   (Dimension)dlIndicator->object.height); n++;
      XtSetArg(args[n],XcNdataType, XcFval); n++;

      switch (dlIndicator->label) 
	{
	case LABEL_NONE:
	  XtSetArg(args[n],XcNvalueVisible,FALSE); n++;
	  XtSetArg(args[n],XcNlabel," "); n++;
	  break;

	case OUTLINE:
	  XtSetArg(args[n],XcNvalueVisible,FALSE); n++;
	  XtSetArg(args[n],XcNlabel," "); n++;
	  break;

	case LIMITS:
	  XtSetArg(args[n],XcNvalueVisible,TRUE); n++;
	  XtSetArg(args[n],XcNlabel," "); n++;
	  break;

	case CHANNEL:
	  XtSetArg(args[n],XcNvalueVisible,TRUE); n++;
	  XtSetArg(args[n],XcNlabel,dlIndicator->monitor.rdbk); n++;
	  break;
	}

      switch (dlIndicator->direction) 
	{
	  /* note that this is  "direction of increase"
	   */
	case DOWN:
	  dm2kPrintf( "\nexecuteDlIndicator: DOWN direction "
		      "INDICATORS not supported");

	case UP:
	  XtSetArg(args[n],XcNscaleSegments,
		   (dlIndicator->object.width >INDICATOR_OKAY_SIZE ? 11 : 5)); n++;
		   XtSetArg(args[n],XcNorient,XcVert); n++;

		   if (dlIndicator->label == LABEL_NONE) {
		     XtSetArg(args[n],XcNscaleSegments,0); n++;
		   }

		   break;

	case LEFT:
	  dm2kPrintf( "\nexecuteDlIndicator: LEFT direction "
		      "INDICATORS not supported");

	case RIGHT:
	  XtSetArg(args[n],XcNscaleSegments,
		   (dlIndicator->object.height>INDICATOR_OKAY_SIZE ? 11 : 5)); n++;

		   XtSetArg(args[n],XcNorient,XcHoriz); n++;

		   if (dlIndicator->label == LABEL_NONE) {
		     XtSetArg(args[n],XcNscaleSegments,0); n++;
		   }

		   break;
	}

      preferredHeight = dlIndicator->object.height/INDICATOR_FONT_DIVISOR;
      bestSize = dmGetBestFontWithInfo(fontTable,MAX_FONTS,NULL,
				       preferredHeight,0,
				       &usedHeight,&usedCharWidth,FALSE);

      XtSetArg(args[n],XtNfont,fontTable[bestSize]); n++;

      XtSetArg(args[n],XcNindicatorForeground,(Pixel)
	       displayInfo->colormap[dlIndicator->monitor.clr]); n++;
      XtSetArg(args[n],XcNindicatorBackground,(Pixel)
	       displayInfo->colormap[dlIndicator->monitor.bclr]); n++;
      XtSetArg(args[n],XtNbackground,(Pixel)
	       displayInfo->colormap[dlIndicator->monitor.bclr]); n++;
      XtSetArg(args[n],XcNcontrolBackground,(Pixel)
	       displayInfo->colormap[dlIndicator->monitor.bclr]); n++;
	       
      /**2000/12 G.Lei**/
      XtSetArg(args[n],XcNincrement, xc_inc.lval); n++;
      XtSetArg(args[n],XcNupperBound, xc_upper.lval); n++;
      XtSetArg(args[n],XcNlowerBound, xc_lower.lval); n++;                        
      dlElement->widget = localWidget = 
	XtCreateWidget("indicator", xcIndicatorWidgetClass, 
		       displayInfo->drawingArea, args, n);
      

      if (displayInfo->traversalMode == DL_EXECUTE) 
	{
	  /* add the pointer to the Channel structure as userData 
	   * to widget
	   */
	  userData->privateData    = (char*) pi;
	  userData->updateTask = (pi ? pi->updateTask : NULL);
	  
	  XtVaSetValues(dlElement->widget, XcNuserData,(XtPointer)userData,NULL);

	  /* destroy callback should free allocated memory
					       */
	  XtAddCallback (dlElement->widget, XmNdestroyCallback,
			 freeUserDataCB, NULL);

	  /* add in drag/drop translations */
	  XtOverrideTranslations(localWidget,parsedTranslations);

	} 
      else if (displayInfo->traversalMode == DL_EDIT) 
	{
	  /* add button press handlers */
	  editObjectHandler (displayInfo, dlElement);

	  XtManageChild(localWidget);
	}
    } else {
      DlObject *po = &(dlElement->structure.indicator->object);
      XtVaSetValues(dlElement->widget,
		    XmNx, (Position) po->x,
		    XmNy, (Position) po->y,
		    XmNwidth, (Dimension) po->width,
		    XmNheight, (Dimension) po->height,
		    NULL);
    }

  return updateTask;
}

static void indicatorDraw(XtPointer cd) {
  Indicator *pi = (Indicator *) cd;
  Record *pd = pi->record;
  Widget widget= pi->dlElement->widget;
  DlIndicator *dlIndicator = pi->dlElement->structure.indicator;
  XcVType val;

  if (pd && pd->connected) {
    if (pd->readAccess) {
      if (widget)
        XtManageChild(widget);
      else
	return;
      val.fval = (float) pd->value;
      XcIndUpdateValue(widget,&val);
      switch (dlIndicator->clrmod) {
      case STATIC :
      case DISCRETE :
	break;
      case ALARM :
	XcIndUpdateIndicatorForeground(widget,alarmColorPixel[pd->severity]);
	break;
      }
    } else {
      if (widget) XtUnmanageChild(widget);
      draw3DPane(pi->updateTask,
		 pi->updateTask->displayInfo->colormap[dlIndicator->monitor.bclr]);
      draw3DQuestionMark(pi->updateTask);
    }
  } else {
    if (widget) XtUnmanageChild(widget);
    drawWhiteRectangle(pi->updateTask);
  }
}

static void indicatorUpdateValueCb(XtPointer cd) {
  Indicator *pi = (Indicator *) ((Record *) cd)->clientData;
  updateTaskMarkUpdate(pi->updateTask);
}

static void indicatorUpdateGraphicalInfoCb(XtPointer cd) {
  Record *pd = (Record *) cd;
  Indicator *pi = (Indicator *) pd->clientData;
  XcVType hopr, lopr, val;
  int precision;
  Widget widget = pi->dlElement->widget;
  DlIndicator *dlIndicator = pi->dlElement->structure.indicator;

  SET_IF_NOT_MAXFLOAT(pd->lopr,dlIndicator->override.displayLowLimit);
  SET_IF_NOT_MAXFLOAT(pd->hopr,dlIndicator->override.displayHighLimit);
  SET_IF_NOT_MINUSONE(pd->precision,dlIndicator->override.displayPrecision);

  switch (pd->dataType) {
  case DBF_STRING :
  case DBF_ENUM :
    dm2kPrintf("indicatorUpdateGraphicalInfoCb : %s %s %s\n",
	       "illegal channel type for",dlIndicator->monitor.rdbk, ": cannot attach Indicator");
    dm2kPostTime();
    return;
  case DBF_CHAR :
  case DBF_INT :
  case DBF_LONG :
  case DBF_FLOAT :
  case DBF_DOUBLE :
    hopr.fval = (float) pd->hopr;
    lopr.fval = (float) pd->lopr;
    val.fval = (float) pd->value;
    precision = pd->precision;
    break;
  default :
    dm2kPrintf("indicatorUpdateGraphicalInfoCb: %s %s %s\n",
	       "unknown channel type for",dlIndicator->monitor.rdbk, ": cannot attach Indicator");
    dm2kPostTime();
    break;
  }
  if ((hopr.fval == 0.0) && (lopr.fval == 0.0)) {
    hopr.fval += 1.0;
  }

  if (widget != NULL) {
    Pixel pixel;
    pixel = (dlIndicator->clrmod == ALARM) ?
      alarmColorPixel[pd->severity] :
      pi->updateTask->displayInfo->colormap[dlIndicator->monitor.clr];
    XtVaSetValues(widget,
		  XcNlowerBound,lopr.lval,
		  XcNupperBound,hopr.lval,
		  XcNindicatorForeground,pixel,
		  XcNdecimals, precision,
		  NULL);
    XcIndUpdateValue(widget,&val);
  }
}

static void indicatorDestroyCb(XtPointer cd) {
  Indicator *pi = (Indicator *) cd;
  if (pi) {
    dm2kDestroyRecord(pi->record);
    free((char *)pi);
  }
  return;
}

static void indicatorName(XtPointer cd, char **name, short *severity, int *count) {
  Indicator *pi = (Indicator *) cd;
  *count = 1;
  name[0] = pi->record->name;
  severity[0] = pi->record->severity;
}

DlElement *createDlIndicator(DlElement *p) 
{
  DlIndicator *dlIndicator;
  DlElement *dlElement;

  dlIndicator = DM2KALLOC(DlIndicator);

  if (dlIndicator == NULL)
    return 0;

  if (p != NULL) {
    objectAttributeCopy(&(dlIndicator->object), &(p->structure.indicator->object));
    monitorAttributeCopy(&(dlIndicator->monitor), &(p->structure.indicator->monitor));
    overrideAttributeCopy(&(dlIndicator->override), &(p->structure.indicator->override));

    dlIndicator->label = p->structure.indicator->label;
    dlIndicator->clrmod = p->structure.indicator->clrmod;
    dlIndicator->direction = p->structure.indicator->direction;
  } else {
    objectAttributeInit(&(dlIndicator->object));
    monitorAttributeInit(&(dlIndicator->monitor));
    overrideAttributeInit(&(dlIndicator->override));

    dlIndicator->label = LABEL_NONE;
    dlIndicator->clrmod = STATIC;
    dlIndicator->direction = RIGHT;
  }

  dlElement = createDlElement(DL_Indicator, (XtPointer) dlIndicator,
			      &indicatorDlDispatchTable);
  if (dlElement == NULL)
    destroyDlIndicator(dlIndicator);

  return(dlElement);
}

DlElement *parseIndicator(DisplayInfo *displayInfo)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;
  DlIndicator *dlIndicator;
  DlElement *dlElement = createDlIndicator(NULL);

  if (!dlElement) return 0;
  dlIndicator = dlElement->structure.indicator;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"object")) {
	parseObject(displayInfo,&(dlIndicator->object));
      }
      else if (STREQL(token,"monitor")) {
	parseMonitor(displayInfo,&(dlIndicator->monitor));
      } 
      else if (STREQL(token,"override")) {
	parseOverride(displayInfo,&(dlIndicator->override));
      } 
      else if (STREQL(token,"label")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);

	if (STREQL(token,"none")) 
	  dlIndicator->label = LABEL_NONE;
	else if (STREQL(token,"outline"))
	  dlIndicator->label = OUTLINE;
	else if (STREQL(token,"limits"))
	  dlIndicator->label = LIMITS;
	else if (STREQL(token,"channel"))
	  dlIndicator->label = CHANNEL;
      }
      else if (STREQL(token,"clrmod")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);

	if (STREQL(token,"static")) 
	  dlIndicator->clrmod = STATIC;
	else if (STREQL(token,"alarm"))
	  dlIndicator->clrmod = ALARM;
	else if (STREQL(token,"discrete"))
	  dlIndicator->clrmod = DISCRETE;
      }
      else if (STREQL(token,"direction")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);

	if (STREQL(token,"up")) 
	  dlIndicator->direction = UP;
	else if (STREQL(token,"down"))
	  dlIndicator->direction = DOWN;
	else if (STREQL(token,"right"))
	  dlIndicator->direction = RIGHT;
	else if (STREQL(token,"left"))
	  dlIndicator->direction = LEFT;
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

void writeDlIndicator( FILE *stream, DlElement *dlElement, int level) {
  /****************************************************************************
   * Write DL Indicator                                                       *
 ****************************************************************************/
  int i;
  char indent[256]; level=MIN(level,256-2);
  DlIndicator *dlIndicator = dlElement->structure.indicator;


  for (i = 0;  i < MIN(level,256-2); i++)
    indent[i] = '\t';
  indent[i] = '\0';

  fprintf(stream,"\n%sindicator {",indent);
  writeDlObject(stream,&(dlIndicator->object),level+1);
  writeDlMonitor(stream,&(dlIndicator->monitor),level+1);
  writeDlOverride(stream,&(dlIndicator->override),level+1);

  if (dlIndicator->label != LABEL_NONE)
    fprintf(stream,"\n%s\tlabel=\"%s\"",indent,
	    stringValueTable[dlIndicator->label]);

  if (dlIndicator->clrmod != STATIC)
    fprintf(stream,"\n%s\tclrmod=\"%s\"",indent,
	    stringValueTable[dlIndicator->clrmod]);

  if (dlIndicator->direction != RIGHT) 
    fprintf(stream,"\n%s\tdirection=\"%s\"",indent,
	    stringValueTable[dlIndicator->direction]);

  fprintf(stream,"\n%s}",indent);
}

static void indicatorInheritValues(ResourceBundle *pRCB, DlElement *p) {
  DlIndicator *dlIndicator = p->structure.indicator;
  dm2kGetValues(pRCB,
		RDBK_RC,       &(dlIndicator->monitor.rdbk),
		CLR_RC,        &(dlIndicator->monitor.clr),
		BCLR_RC,       &(dlIndicator->monitor.bclr),
		LABEL_RC,      &(dlIndicator->label),
		DIRECTION_RC,  &(dlIndicator->direction),
		CLRMOD_RC,     &(dlIndicator->clrmod),
		LOW_LIMIT_RC,  &(dlIndicator->override.displayLowLimit),
		HIGH_LIMIT_RC, &(dlIndicator->override.displayHighLimit),
		VAL_PRECISION_RC, &(dlIndicator->override.displayPrecision),
		-1);
}

static void indicatorGetValues(ResourceBundle *pRCB, DlElement *p) {
  DlIndicator *dlIndicator = p->structure.indicator;
  dm2kGetValues(pRCB,
		X_RC,          &(dlIndicator->object.x),
		Y_RC,          &(dlIndicator->object.y),
		WIDTH_RC,      &(dlIndicator->object.width),
		HEIGHT_RC,     &(dlIndicator->object.height),
		RDBK_RC,       &(dlIndicator->monitor.rdbk),
		CLR_RC,        &(dlIndicator->monitor.clr),
		BCLR_RC,       &(dlIndicator->monitor.bclr),
		LABEL_RC,      &(dlIndicator->label),
		DIRECTION_RC,  &(dlIndicator->direction),
		CLRMOD_RC,     &(dlIndicator->clrmod),
		LOW_LIMIT_RC,  &(dlIndicator->override.displayLowLimit),
		HIGH_LIMIT_RC, &(dlIndicator->override.displayHighLimit),
		VAL_PRECISION_RC, &(dlIndicator->override.displayPrecision),
		-1);
}
