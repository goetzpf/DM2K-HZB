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
 * .03  09-11-95        vong    conform to c++ syntax
 *
 *****************************************************************************
 *
 *      18-03-97        Fabien  Add object info and enter leave window
 *
 *****************************************************************************
*/

#include "dm2k.h"

typedef struct _Bar {
  DlElement   *dlElement;
  Record      *record;
  UpdateTask  *updateTask;
} Bar;

static void barDraw(XtPointer cd);
static void barUpdateValueCb(XtPointer cd);
static void barUpdateGraphicalInfoCb(XtPointer cd);
static void barDestroyCb(XtPointer cd);
static void barName(XtPointer, char **, short *, int *);
static void barInheritValues(ResourceBundle *pRCB, DlElement *p);
static void barGetValues(ResourceBundle *pRCB, DlElement *p);

static void destroyDlElement (DlElement * element);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);

static DlDispatchTable barDlDispatchTable = {
         createDlBar,
	 destroyDlElement,
	 executeMethod,
	 writeDlBar,
	 NULL,
	 barGetValues,
	 barInheritValues,
	 NULL,
	 NULL,
	 genericMove,
	 genericScale,
	 NULL,
	 genericObjectInfo
};


static void destroyDlBar (DlBar * dlBar)
{
  if (dlBar == NULL)
    return;

  objectAttributeDestroy(&(dlBar->object));
  monitorAttributeDestroy(&(dlBar->monitor));
  dynamicAttributeDestroy(&(dlBar->dynAttr));

  free((char*)dlBar);
}

static void destroyDlElement (DlElement * element)
{
  if (element == NULL || element->type != DL_Bar) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  destroyDlBar (element->structure.bar);
  free((char*)element);
}

static UpdateTask * executeMethod
  (DisplayInfo * displayInfo,
   DlElement   * dlElement)
{
  Bar            * pb;
  Arg              args[30];
  int              n;
  int              usedHeight, usedCharWidth, bestSize, preferredHeight;
  Widget           localWidget;
  DlBar          * dlBar = dlElement->structure.bar;
  WidgetUserData * userData = NULL;
  UpdateTask     * updateTask = NULL;
  XcVType lval, hval;

  /**2000/12 G.Lei**/
  XcVType xc_lower, xc_upper, xc_inc;

  xc_lower.fval = 0.0;
  xc_upper.fval =100.0;
  xc_inc.fval = 1.0;

  if (!dlElement->widget) {
    if (displayInfo->traversalMode == DL_EXECUTE) 
    {
      /* alloc servise structures
	   */
      userData = DM2KALLOC (WidgetUserData);
      pb = DM2KALLOC (Bar);

      if (pb == NULL || userData == NULL) {
	DM2KFREE (pb);
	DM2KFREE (userData);
	dm2kPrintf("executeDlBar: memory allocation error\n");
	return NULL;
      }

      dlBar->object.runtimeDescriptor = (XtPointer) pb;
      pb->dlElement = dlElement;
      updateTask = pb->updateTask = updateTaskAddTask(displayInfo,
						      &(dlBar->object),
						      barDraw,
						      (XtPointer)pb);
      
      if (pb->updateTask == NULL) {
	dm2kPrintf("barCreateRunTimeInstance : memory allocation error\n");
      } else {
	updateTaskAddDestroyCb(pb->updateTask,barDestroyCb);
	updateTaskAddNameCb(pb->updateTask,barName);
      }

      pb->record = dm2kAllocateRecord(dlBar->monitor.rdbk,
				      barUpdateValueCb,
				      barUpdateGraphicalInfoCb,
				      (XtPointer) pb);
      drawWhiteRectangle(pb->updateTask);
    }

    /* from the bar structure, we've got Bar's specifics 
     */
    n = 0;
    XtSetArg(args[n],XtNx,(Position)dlBar->object.x); n++;
    XtSetArg(args[n],XtNy,(Position)dlBar->object.y); n++;
    XtSetArg(args[n],XtNwidth,(Dimension)dlBar->object.width); n++;
    XtSetArg(args[n],XtNheight,(Dimension)dlBar->object.height); n++;
    XtSetArg(args[n],XcNdataType,XcFval); n++;

    switch (dlBar->label) {
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
      XtSetArg(args[n],XcNlabel,dlBar->monitor.rdbk); n++;
      break;
    }

    /* note that this is  "direction of increase" for Bar
     */
    switch (dlBar->direction) {
    case LEFT:
      dm2kPrintf("\nexecuteDlBar: LEFT direction BARS not supported");
    case RIGHT:
      XtSetArg(args[n],XcNorient,XcHoriz); n++;
      XtSetArg(args[n],XcNscaleSegments,
	       (dlBar->object.width>INDICATOR_OKAY_SIZE ? 11 : 5)); n++;
	       if (dlBar->label == LABEL_NONE) {
		 XtSetArg(args[n],XcNscaleSegments, 0); n++;
	       }
	       break;

    case DOWN:
      dm2kPrintf("\nexecuteDlBar: DOWN direction BARS not supported");
    case UP:
      XtSetArg(args[n],XcNorient,XcVert); n++;
      XtSetArg(args[n],XcNscaleSegments,
	       (dlBar->object.height>INDICATOR_OKAY_SIZE ? 11 : 5)); n++;
	       if (dlBar->label == LABEL_NONE) {
		 XtSetArg(args[n],XcNscaleSegments, 0); n++;
	       }
	       break;
    }

    if (dlBar->fillmod == FROM_CENTER) {
      XtSetArg(args[n], XcNfillmod, XcCenter); n++;
    } else {
      XtSetArg(args[n], XcNfillmod, XcEdge); n++;
    }

    preferredHeight = MIN(dlBar->object.height,dlBar->object.width)/INDICATOR_FONT_DIVISOR;
    bestSize = dmGetBestFontWithInfo(fontTable,MAX_FONTS,NULL,
				     preferredHeight,
				     0,&usedHeight,&usedCharWidth,FALSE);

    XtSetArg(args[n],XtNfont,fontTable[bestSize]); n++;

    XtSetArg(args[n],XcNbarForeground,(Pixel)
	     displayInfo->colormap[dlBar->monitor.clr]); n++;
    XtSetArg(args[n],XcNbarBackground,(Pixel)
	     displayInfo->colormap[dlBar->monitor.bclr]); n++;
    XtSetArg(args[n],XtNbackground,(Pixel)
	     displayInfo->colormap[dlBar->monitor.bclr]); n++;
    XtSetArg(args[n],XcNcontrolBackground,(Pixel)
	     displayInfo->colormap[dlBar->monitor.bclr]); n++;

    XtSetArg(args[n], XcNbarOnly, dlBar->barOnly == BAR_ONLY_YES);  n++;
    XtSetArg(args[n], XcNlogScale, dlBar->scaleType == LOG_SCALE);  n++;

    /*SET_IF_NOT_MAXFLOAT(lval.fval,(float)dlBar->override.displayLowLimit);
    SET_IF_NOT_MAXFLOAT(hval.fval,(float)dlBar->override.displayHighLimit);
    XtSetArg(args[n], XcNlowerBound, lval.lval);  n++;
    XtSetArg(args[n], XcNupperBound, hval.lval);  n++;
    */

    /**2000/12 G.Lei**/
    XtSetArg(args[n],XcNincrement, xc_inc.lval); n++;
    XtSetArg(args[n],XcNupperBound, xc_upper.lval); n++;
    XtSetArg(args[n],XcNlowerBound, xc_lower.lval); n++;
    XtSetArg(args[n], XcNautoSegment, 
	     dlBar->showScale == SHOW_SCALE_ON) ; n++;

    dlElement->widget =
      localWidget = XtCreateWidget("bar", xcBarGraphWidgetClass, 
				   displayInfo->drawingArea, args, n);
      
    if (userData) 
    {
      /* add the pointer to the Channel structure as privateData 
					     * in userData to widget
					     */
      userData->privateData    = (char*) pb;
      userData->updateTask = pb->updateTask;
	  
      XtVaSetValues(localWidget, XcNuserData, (XtPointer) userData, NULL);
	  
      /* destroy callback should free allocated memory
       */
      XtAddCallback (localWidget, XmNdestroyCallback, freeUserDataCB, 
		     NULL);
    }
      
      
    if (displayInfo->traversalMode == DL_EXECUTE) 
      {
	/* add in drag/drop translations 
	 */
	XtOverrideTranslations(localWidget,parsedTranslations);
      } 
    else if (displayInfo->traversalMode == DL_EDIT) 
      {
	/* add button press handlers */
	editObjectHandler (displayInfo, dlElement);
      }
      
    XtManageChild(localWidget);
  } 
  else 
  {
    register DlObject *po = &(dlElement->structure.bar->object);
    XtVaSetValues(dlElement->widget,
		  XmNx,      (Position) po->x,
		  XmNy,      (Position) po->y,
		  XmNwidth,  (Dimension) po->width,
		  XmNheight, (Dimension) po->height,
		  NULL);
  }

  return updateTask;
}

static void barUpdateValueCb(XtPointer cd) {
  Bar *pb = (Bar *) ((Record *) cd)->clientData;
  updateTaskMarkUpdate(pb->updateTask);
}

static void barDraw(XtPointer cd) 
{
  Bar     * pb = (Bar *) cd;
  Record  * pd = pb->record;
  Widget    widget = pb->dlElement->widget;
  DlBar   * dlBar = pb->dlElement->structure.bar;
  XcVType   val;
  Pixel     pixel;

  if (pd && pd->connected) {
    if (pd->readAccess) {
      if (widget)
	XtManageChild(widget);
      else
	return;
      val.fval = (float) pd->value;
      XcBGUpdateValue(widget,&val);

      switch (dlBar->dynAttr.clr) 
      {
      case STATIC :
	break;

      case DISCRETE :
	if (dlBar->dynAttr.clr == ALARM) {
	  pixel = alarmColorPixel[pd->severity];
	} else if (dlBar->dynAttr.clr == DISCRETE) {
	  pixel = extractColor(pb->updateTask->displayInfo,
			       pd->value,
			       dlBar->dynAttr.colorRule,
			       pb->updateTask->displayInfo->colormap[dlBar->monitor.clr]);
	} else
	  pixel = pb->updateTask->displayInfo->colormap[dlBar->monitor.clr];
	
	XcBGUpdateBarForeground(widget,pixel);
	break;
      case ALARM :
	XcBGUpdateBarForeground(widget,alarmColorPixel[pd->severity]);
	break;
      }
    } else {
      if (widget) XtUnmanageChild(widget);
      draw3DPane(pb->updateTask,
		 pb->updateTask->displayInfo->colormap[dlBar->monitor.bclr]);
      draw3DQuestionMark(pb->updateTask);
    }
  } else {
    if (widget) XtUnmanageChild(widget);
    drawWhiteRectangle(pb->updateTask);
  }
}

static void barUpdateGraphicalInfoCb(XtPointer cd) 
{
  Record  * pd = (Record *) cd;
  Bar     * pb = (Bar *) pd->clientData;
  XcVType   hopr, lopr, val;
  Pixel     pixel;
  int       precision;
  Widget    widget = pb->dlElement->widget;
  DlBar   * dlBar = pb->dlElement->structure.bar;
  double    alarms[4]; 
  Pixel     alarmsColor[4];
  Arg       args[30];
  int       n;

  if (widget == NULL) 
    return;

  switch (pd->dataType) {
  case DBF_STRING :
  case DBF_ENUM :
    dm2kPrintf("barUpdateGraphicalInfoCb : %s %s %s\n",
	       "illegal channel type for",dlBar->monitor.rdbk, 
	       ": cannot attach Bar");
    dm2kPostTime();
    return;
  case DBF_CHAR :
  case DBF_INT :
  case DBF_LONG :
  case DBF_FLOAT :
  case DBF_DOUBLE :
     SET_IF_NOT_MAXFLOAT(pd->hopr,dlBar->override.displayHighLimit);
     SET_IF_NOT_MAXFLOAT(pd->lopr,dlBar->override.displayLowLimit);
     SET_IF_NOT_MINUSONE(pd->precision,dlBar->override.displayPrecision);
     hopr.fval = (float) pd->hopr;
     lopr.fval = (float) pd->lopr;
     precision = pd->precision;
     if (precision < 0) precision = 0;
     val.fval = (float) pd->value;
     alarms[0] = (double) pd->lowerWarningLimit;
     alarms[1] = (double) pd->upperWarningLimit;
     alarms[2] = (double) pd->lowerAlarmLimit;
     alarms[3] = (double) pd->upperAlarmLimit;
     
     alarmsColor[0] = alarmColorPixel[MINOR_ALARM];
     alarmsColor[1] = alarmColorPixel[MINOR_ALARM];
     alarmsColor[2] = alarmColorPixel[MAJOR_ALARM];
     alarmsColor[3] = alarmColorPixel[MAJOR_ALARM];

     break;
  default :
    dm2kPrintf("barUpdateGraphicalInfoCb: %s %s %s\n",
	       "unknown channel type for",dlBar->monitor.rdbk, ": cannot attach Bar");
    dm2kPostTime();
    break;
  }
  if ((hopr.fval == 0.0) && (lopr.fval == 0.0)) {
    hopr.fval += 1.0;
  }

  if (dlBar->dynAttr.clr == ALARM) {
    pixel = alarmColorPixel[pd->severity];
  } else if (dlBar->dynAttr.clr == DISCRETE) {
    pixel = extractColor(pb->updateTask->displayInfo,
			 pd->value,
			 dlBar->dynAttr.colorRule,
			 pb->updateTask->displayInfo->colormap[dlBar->monitor.clr]);
  } else
    pixel = pb->updateTask->displayInfo->colormap[dlBar->monitor.clr];

  n = 0;
  XtSetArg(args[n], XcNlowerBound,     lopr.lval); n++;
  XtSetArg(args[n], XcNupperBound,     hopr.lval); n++;
  XtSetArg(args[n], XcNbarForeground,  pixel); n++;
  XtSetArg(args[n], XcNdecimals,       precision); n++;

  if (dlBar->showAlarmLimits == SHOW_ALARM_LIMITS_ON) {
    XtSetArg(args[n], XcNmarkers,        alarms); n++;
    XtSetArg(args[n], XcNmarkersColors,  alarmsColor); n++; 
    XtSetArg(args[n], XcNmarkersNum,     4); n++;
  }

  XtSetArg(args[n], XcNautoSegment, 
	   dlBar->showScale == SHOW_SCALE_ON) ; n++;

  XtSetValues(widget, args, n);

  XcBGUpdateValue(widget,&val);
}

static void barDestroyCb(XtPointer cd) {
  Bar *pb = (Bar *) cd;
  if (pb) {
    dm2kDestroyRecord(pb->record);
    free((char *)pb);
  }
  return;
}

static void barName(XtPointer cd, char **name, short *severity, int *count) {
  Bar *pb = (Bar *) cd;
  *count = 1;
  name[0] = pb->record->name;
  severity[0] = pb->record->severity;
}

DlElement *createDlBar(DlElement *p)
{
  DlBar     * dlBar;
  DlElement * dlElement;

  dlBar = DM2KALLOC(DlBar);

  if (dlBar == NULL) 
    return 0;

  if (p != NULL) {
    objectAttributeCopy(&(dlBar->object),&p->structure.bar->object);
    monitorAttributeCopy(&(dlBar->monitor),&p->structure.bar->monitor);
    dynamicAttributeCopy(&(dlBar->dynAttr),&p->structure.bar->dynAttr);
    overrideAttributeCopy(&(dlBar->override),&p->structure.bar->override);

    dlBar->label            = p->structure.bar->label;
    dlBar->direction        = p->structure.bar->direction;
    dlBar->fillmod          = p->structure.bar->fillmod;
    dlBar->scaleType        = p->structure.bar->scaleType;
    dlBar->barOnly          = p->structure.bar->barOnly;
    dlBar->showAlarmLimits  = p->structure.bar->showAlarmLimits;
    dlBar->showScale        = p->structure.bar->showScale;
  } 
  else {
    objectAttributeInit(&(dlBar->object));
    monitorAttributeInit(&(dlBar->monitor));
    dynamicAttributeInit(&(dlBar->dynAttr));
    overrideAttributeInit(&(dlBar->override));

    dlBar->label            = LABEL_NONE;
    dlBar->direction        = RIGHT;
    dlBar->fillmod          = FROM_EDGE;
    dlBar->scaleType        = FIRST_SCALE_TYPE;
    dlBar->barOnly          = FIRST_SHOW_BAR;
    dlBar->showAlarmLimits  = FIRST_SHOW_ALARM_LIMITS_TYPE;
    dlBar->showScale        = FIRST_SHOW_SCALE_TYPE;
  }

  dlElement = createDlElement(DL_Bar, (XtPointer) dlBar, &barDlDispatchTable);
  if (dlElement == NULL)
    destroyDlBar(dlBar);

  return(dlElement);
}

DlElement *parseBar(DisplayInfo *displayInfo)
{
  char        token[MAX_TOKEN_LENGTH];
  TOKEN       tokenType;
  int         nestingLevel = 0;
  DlBar     * dlBar;
  DlElement * dlElement = createDlBar(NULL);
 
  if (dlElement == NULL) 
    return 0;

  dlBar = dlElement->structure.bar;
 
  do {
    switch( tokenType = getToken(displayInfo,token) ) 
    {
    case T_WORD:
      if (STREQL(token,"object")) {
        parseObject(displayInfo,&(dlBar->object));
      } 
      else if (STREQL(token,"monitor")) {
	parseMonitor(displayInfo,&(dlBar->monitor));
      } 
      else if (STREQL(token,"override")) {
	parseOverride(displayInfo,&(dlBar->override));
      } 
      else if (STREQL(token,"label")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);

	if (STREQL(token,"none"))
	  dlBar->label = LABEL_NONE;
	else if (STREQL(token,"outline"))
	  dlBar->label = OUTLINE;
	else if (STREQL(token,"limits"))
	  dlBar->label = LIMITS;
	else if (STREQL(token,"channel"))
	  dlBar->label = CHANNEL;
	else printf("unknown token in file``%s''\n",token);
      } 
      else if (STREQL(token,"dynamic attribute")) {
	parseDynamicAttribute(displayInfo,&(dlBar->dynAttr));
      }
      else if (STREQL(token,"direction")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	if (STREQL(token,"up"))	  dlBar->direction = UP;
	else if (STREQL(token,"down"))	  dlBar->direction = DOWN;
	else if (STREQL(token,"right"))  dlBar->direction = RIGHT;
	else if (STREQL(token,"left"))	  dlBar->direction = LEFT;
	else printf("unknown token in file``%s''\n",token);
      } 
      else if (STREQL(token,"fillmod")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	if (STREQL(token,"from edge"))	      dlBar->fillmod = FROM_EDGE;
	else if(STREQL(token,"from center")) dlBar->fillmod = FROM_CENTER;
	else printf("unknown token in file``%s''\n",token);
      }
      else if (STREQL(token,"scaleType")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	if (STREQL(token,"linear"))         dlBar->scaleType = LINEAR_SCALE;
	else if (STREQL(token,"logarithm")) dlBar->scaleType = LOG_SCALE;
	else printf("unknown token in file``%s''\n",token);
      }
      else if (STREQL(token,"showBar")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	if (STREQL(token,"normal"))        dlBar->barOnly = BAR_ONLY_NO;
	else if (STREQL(token,"bar only")) dlBar->barOnly = BAR_ONLY_YES;
	else printf("unknown token in file``%s''\n",token);
      }
      else if (STREQL(token,"showAlarmLimits")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	if (STREQL(token,"on"))       
	  dlBar->showAlarmLimits = SHOW_ALARM_LIMITS_ON;
	else if (STREQL(token,"off"))
	  dlBar->showAlarmLimits = SHOW_ALARM_LIMITS_OFF;
	else
	  printf("unknown token in file``%s''\n",token);
      }
      else if (STREQL(token,"showScale")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	if (STREQL(token,"on"))       
	  dlBar->showScale = SHOW_SCALE_ON;
	else if (STREQL(token,"off"))
	  dlBar->showScale = SHOW_SCALE_OFF;
	else
	  printf("unknown token in file``%s''\n",token);
      }
      else if (STREQL(token,"limitType")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	
	if (STREQL(token,"from channel")) {
	   dlBar->override.displayLowLimit = MAXFLOAT;
	   dlBar->override.displayHighLimit = MAXFLOAT;
	   dlBar->override.displayPrecision = -1;
	} else
	   printf("unknown token in file``%s'' (please save with new file-format)\n",token);
      }
      else if (STREQL(token,"lowLimit")) {
	 getToken(displayInfo,token);
	 getToken(displayInfo,token);
	 dlBar->override.displayLowLimit = atof(token);
      }
      else if (STREQL(token,"highLimit")) {
	 getToken(displayInfo,token);
	 getToken(displayInfo,token);
	 dlBar->override.displayHighLimit = atof(token);
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

void writeDlBar( FILE *stream, DlElement *dlElement, int level) {
  int i;
  char indent[256]; level=MIN(level,256-2);
  DlBar *dlBar = dlElement->structure.bar;

  for (i = 0;  i < MIN(level,256-2); i++)
    indent[i] = '\t';
  indent[i] = '\0';

  fprintf(stream,"\n%sbar {",indent);
  writeDlObject(stream,&(dlBar->object),level+1);
  writeDlDynamicAttribute(stream,&(dlBar->dynAttr),
			  DYNATTR_COLORRULE | DYNATTR_COLORMODE,level+1);
  writeDlMonitor(stream,&(dlBar->monitor),level+1);
  writeDlOverride(stream,&(dlBar->override),level+1);

  if (dlBar->label != LABEL_NONE)
    fprintf(stream,"\n%s\tlabel=\"%s\"",indent,
	    stringValueTable[dlBar->label]);
  if (dlBar->direction != RIGHT)
    fprintf(stream,"\n%s\tdirection=\"%s\"",indent,
	    stringValueTable[dlBar->direction]);
  if (dlBar->fillmod != FROM_EDGE) 
    fprintf(stream,"\n%s\tfillmod=\"%s\"",indent,
	    stringValueTable[dlBar->fillmod]);

  fprintf(stream,"\n%s\tscaleType=\"%s\"",indent,
	  stringValueTable[dlBar->scaleType]);

  fprintf(stream,"\n%s\tshowAlarmLimits=\"%s\"",indent,
	  stringValueTable[dlBar->showAlarmLimits]);

  fprintf(stream,"\n%s\tshowScale=\"%s\"",indent,
	  stringValueTable[dlBar->showScale]);

  fprintf(stream,"\n%s\tshowBar=\"%s\"",indent,
	  stringValueTable[dlBar->barOnly]);

  fprintf(stream,"\n%s}",indent);
}

static void barInheritValues(ResourceBundle *pRCB, DlElement *p) {
  DlBar *dlBar = p->structure.bar;
  dm2kGetValues(pRCB,
		RDBK_RC,       &(dlBar->monitor.rdbk),
		CLR_RC,        &(dlBar->monitor.clr),
		BCLR_RC,       &(dlBar->monitor.bclr),
		LABEL_RC,      &(dlBar->label),
		DIRECTION_RC,  &(dlBar->direction),
		CLRMOD_RC,     &(dlBar->dynAttr.clr),
		FILLMOD_RC,    &(dlBar->fillmod),
		COLOR_RULE_RC, &(dlBar->dynAttr.colorRule),
		LOW_LIMIT_RC,  &(dlBar->override.displayLowLimit),
		HIGH_LIMIT_RC, &(dlBar->override.displayHighLimit),
		VAL_PRECISION_RC, &(dlBar->override.displayPrecision),
		-1);
}

static void barGetValues(ResourceBundle *pRCB, DlElement *p) {
  DlBar *dlBar = p->structure.bar;
  dm2kGetValues(pRCB,
		X_RC,          &(dlBar->object.x),
		Y_RC,          &(dlBar->object.y),
		WIDTH_RC,      &(dlBar->object.width),
		HEIGHT_RC,     &(dlBar->object.height),
		RDBK_RC,       &(dlBar->monitor.rdbk),
		CLR_RC,        &(dlBar->monitor.clr),
		BCLR_RC,       &(dlBar->monitor.bclr),
		LABEL_RC,      &(dlBar->label),
		DIRECTION_RC,  &(dlBar->direction),
		FILLMOD_RC,    &(dlBar->fillmod),
		CLRMOD_RC,     &(dlBar->dynAttr.clr),
		COLOR_RULE_RC, &(dlBar->dynAttr.colorRule),
		SCALE_TYPE_RC, &(dlBar->scaleType),
		BAR_ONLY_RC,   &(dlBar->barOnly),
		SHOW_ALARM_LIMIT_RC, &(dlBar->showAlarmLimits),
		SHOW_SCALE_RC, &(dlBar->showScale),
		LOW_LIMIT_RC,  &(dlBar->override.displayLowLimit),
		HIGH_LIMIT_RC, &(dlBar->override.displayHighLimit),
		VAL_PRECISION_RC, &(dlBar->override.displayPrecision),
		-1);
}
