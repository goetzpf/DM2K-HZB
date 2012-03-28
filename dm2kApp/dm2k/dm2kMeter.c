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

typedef struct _Meter {
  DlElement   *dlElement;
  Record      *record;
  UpdateTask  *updateTask;
} Meter;

static void meterUpdateValueCb(XtPointer cd);
static void meterDraw(XtPointer cd);
static void meterUpdateGraphicalInfoCb(XtPointer cd);
static void meterDestroyCb(XtPointer cd);
static void meterName(XtPointer, char **, short *, int *);
static void meterInheritValues(ResourceBundle *pRCB, DlElement *p);
static void meterGetValues(ResourceBundle *pRCB, DlElement *p);

static void destroyDlElement (DlElement * element);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);

static DlDispatchTable meterDlDispatchTable = {
       createDlMeter,
       destroyDlElement,
       executeMethod,
       writeDlMeter,
       NULL,
       meterGetValues,
       meterInheritValues,
       NULL,
       NULL,
       genericMove,
       genericScale,
       NULL,
       genericObjectInfo
};

static void destroyDlMeter (DlMeter * dlMeter)
{
  if (dlMeter == NULL)
    return;

  objectAttributeDestroy(&(dlMeter->object));
  monitorAttributeDestroy(&(dlMeter->monitor));

  free((char*)dlMeter);
}

static void destroyDlElement (DlElement * element)
{
  if (element == NULL || element->type != DL_Meter) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  destroyDlMeter (element->structure.meter);
  free((char*)element);
}


static UpdateTask * executeMethod
   (DisplayInfo * displayInfo, 
    DlElement   * dlElement)
{
  Meter          * pm = NULL;
  Arg              args[24];
  int              n;
  int              usedHeight, usedCharWidth, bestSize, preferredHeight;
  Widget           localWidget;
  DlMeter        * dlMeter = dlElement->structure.meter;
  WidgetUserData * userData = NULL;
  UpdateTask * updateTask = NULL;
  XcVType xc_lower, xc_upper, xc_inc; /*2000/12 G.Lei*/

  xc_lower.fval = 0.0;
  xc_upper.fval =100.0;
  xc_inc.fval = 1.0;

  if (dlElement->widget == NULL) 
    {
      if (displayInfo->traversalMode == DL_EXECUTE) 
	{
	  /* alloc servise structures
	   */
	  userData = DM2KALLOC (WidgetUserData);
	  pm = DM2KALLOC (Meter);
      
	  if (pm == NULL || userData == NULL) {
	    DM2KFREE (pm);
	    DM2KFREE (userData);
	    dm2kPrintf("executeDlMeter: memory allocation error\n");
	    return updateTask;
	  }

	  pm->dlElement = dlElement;
	  dlMeter->object.runtimeDescriptor = (XtPointer) pm;

	  updateTask = pm->updateTask = updateTaskAddTask(displayInfo,
							  &(dlMeter->object),
							  meterDraw,
							  (XtPointer)pm);

	  if (pm->updateTask == NULL) {
	    dm2kPrintf("meterCreateRunTimeInstance : memory allocation error\n");
	  } else {
	    updateTaskAddDestroyCb(pm->updateTask,meterDestroyCb);
	    updateTaskAddNameCb(pm->updateTask,meterName);
	  }

	  pm->record = dm2kAllocateRecord(dlMeter->monitor.rdbk,
					  meterUpdateValueCb,
					  meterUpdateGraphicalInfoCb,
					  (XtPointer) pm);
	  drawWhiteRectangle(pm->updateTask);
	}

      /* from the meter structure, we've got Meter's specifics */
      n = 0;
      XtSetArg(args[n],XtNx,      (Position)dlMeter->object.x); n++;
      XtSetArg(args[n],XtNy,      (Position)dlMeter->object.y); n++;
      XtSetArg(args[n],XtNwidth,  (Dimension)dlMeter->object.width); n++;
      XtSetArg(args[n],XtNheight, (Dimension)dlMeter->object.height); n++;

      XtSetArg(args[n],XcNdataType,XcFval); n++;
      XtSetArg(args[n],XcNscaleSegments,
	       (dlMeter->object.width > METER_OKAY_SIZE ? 11 : 5) ); n++;

	       switch (dlMeter->label) 
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
		   XtSetArg(args[n],XcNlabel,dlMeter->monitor.rdbk); n++;
		   break;
		 }

	       preferredHeight = dlMeter->object.height/METER_FONT_DIVISOR;
	       bestSize = dmGetBestFontWithInfo(fontTable,MAX_FONTS,NULL,
						preferredHeight,0,
						&usedHeight,&usedCharWidth,FALSE);

	       XtSetArg(args[n],XtNfont,fontTable[bestSize]); n++;
	       XtSetArg(args[n],XcNmeterForeground,(Pixel)
			displayInfo->colormap[dlMeter->monitor.clr]); n++;
			XtSetArg(args[n],XcNmeterBackground,(Pixel)
				 displayInfo->colormap[dlMeter->monitor.bclr]); n++;
				 XtSetArg(args[n],XtNbackground,(Pixel)
					  displayInfo->colormap[dlMeter->monitor.bclr]); n++;
					  XtSetArg(args[n],XcNcontrolBackground,(Pixel)
						   displayInfo->colormap[dlMeter->monitor.bclr]); n++;

	       /*2000/12 G.Lei*/
	       XtSetArg(args[n],XcNincrement, xc_inc.lval); n++;
	       XtSetArg(args[n],XcNupperBound, xc_upper.lval); n++;
	       XtSetArg(args[n],XcNlowerBound, xc_lower.lval); n++;
               
						   dlElement->widget = localWidget = 
						     XtCreateWidget("meter", xcMeterWidgetClass, 
								    displayInfo->drawingArea, args, n);

						   if (userData) 
						     {
						       /* add the pointer to the Channel structure as userData 
							*  to widget
							*/
						       userData->privateData    = (char*) pm;
						       userData->updateTask = (pm ? pm->updateTask: NULL);
      
						       XtVaSetValues(localWidget, XcNuserData, (XtPointer)userData, NULL);

						       /* destroy callback should free allocated memory
							*/
						       XtAddCallback (localWidget, XmNdestroyCallback,
								      freeUserDataCB, NULL);
						     }

						   if (displayInfo->traversalMode == DL_EXECUTE) 
						     {
						       /* add in drag/drop translations */
						       XtOverrideTranslations(localWidget,parsedTranslations);
						     } 
						   else if (displayInfo->traversalMode == DL_EDIT) 
						     {
						       /* add button press handlers */
						       editObjectHandler (displayInfo, dlElement);
						     }

						   XtManageChild(localWidget);

    } 
  else {
    DlObject *po = &(dlElement->structure.meter->object);

    XtVaSetValues(dlElement->widget,
		  XmNx,      (Position) po->x,
		  XmNy,      (Position) po->y,
		  XmNwidth,  (Dimension) po->width,
		  XmNheight, (Dimension) po->height,
		  NULL);
  }
  return NULL;
}

static void meterUpdateValueCb(XtPointer cd) 
{
  Meter *pm = (Meter *) ((Record *) cd)->clientData;
  updateTaskMarkUpdate(pm->updateTask);
}

static void meterDraw(XtPointer cd) 
{
  Meter *pm = (Meter *) cd;
  Record *pd = pm->record;
  Widget widget = pm->dlElement->widget;
  DlMeter *dlMeter = pm->dlElement->structure.meter;
  XcVType val;

  if (pd && pd->connected) 
    {
      if (pd->readAccess) {
	if (widget)
	  XtManageChild(widget);
	else
	  return;

	val.fval = (float) pd->value;
	XcMeterUpdateValue(widget,&val);

	switch (dlMeter->clrmod) 
	  {
	  case STATIC :
	  case DISCRETE :
	    break;
	  case ALARM :
	    XcMeterUpdateMeterForeground(widget,alarmColorPixel[pd->severity]);
	    break;
	  }
      } 
      else {
	if (widget) 
	  XtUnmanageChild(widget);
      
	draw3DPane(pm->updateTask,
		   pm->updateTask->displayInfo->colormap[dlMeter->monitor.bclr]);
      
	draw3DQuestionMark(pm->updateTask);
      }
    } 
  else {
    if (widget) 
      XtUnmanageChild(widget);

    drawWhiteRectangle(pm->updateTask);
  }
}

static void meterUpdateGraphicalInfoCb(XtPointer cd) 
{
  Record  * pd = (Record *) cd;
  Meter   * pm = (Meter *) pd->clientData;
  DlMeter * dlMeter = pm->dlElement->structure.meter;
  Widget    widget = pm->dlElement->widget;
  XcVType   hopr, lopr, val;
  int       precision;

  switch (pd->dataType) 
    {
    case DBF_STRING :
    case DBF_ENUM :
      dm2kPrintf("meterUpdateGraphicalInfoCb : %s %s %s\n",
		 "illegal channel type for",dlMeter->monitor.rdbk, 
		 ": cannot attach Meter");
      dm2kPostTime();
      return;

    case DBF_CHAR :
    case DBF_INT :
    case DBF_LONG :
    case DBF_FLOAT :
    case DBF_DOUBLE :
       SET_IF_NOT_MAXFLOAT(pd->hopr,(float)dlMeter->override.displayHighLimit);
       SET_IF_NOT_MAXFLOAT(pd->lopr,(float)dlMeter->override.displayLowLimit);
       SET_IF_NOT_MINUSONE(pd->precision,dlMeter->override.displayPrecision);

       hopr.fval = (float) pd->hopr;
       lopr.fval = (float) pd->lopr;
       precision = pd->precision;
       if (precision < 0) precision = 0;
       val.fval = (float) pd->value;
      break;

    default :
      dm2kPrintf("meterUpdateGraphicalInfoCb: %s %s %s\n",
		 "unknown channel type for",
		 dlMeter->monitor.rdbk, ": cannot attach Meter");
      dm2kPostTime();
      break;
    }
  
  if ((hopr.fval == 0.0) && (lopr.fval == 0.0)) {
    hopr.fval += 1.0;
  }

  if (widget != NULL) 
    {
      Pixel pixel;

      pixel = (dlMeter->clrmod == ALARM) ?
	alarmColorPixel[pd->severity] :
	pm->updateTask->displayInfo->colormap[dlMeter->monitor.clr];

      XtVaSetValues(widget,
		    XcNlowerBound,lopr.lval,
		    XcNupperBound,hopr.lval,
		    XcNmeterForeground,pixel,
		    XcNdecimals, precision,
		    NULL);
      XcMeterUpdateValue(widget,&val);
    }
}


static void meterDestroyCb(XtPointer cd) 
{
  Meter *pm = (Meter *) cd;

  if (pm) {
    dm2kDestroyRecord(pm->record);
    free((char *)pm);
  }
  return;
}


static void meterName(XtPointer cd, char **name, short *severity, int *count) {
  Meter *pm = (Meter *) cd;
  *count = 1;
  name[0] = pm->record->name;
  severity[0] = pm->record->severity;
}


DlElement *createDlMeter(DlElement *p)
{
  DlMeter   * dlMeter;
  DlElement * dlElement;

  dlMeter = DM2KALLOC(DlMeter);

  if (dlMeter == NULL)
    return NULL;

  if (p != NULL) {
    objectAttributeCopy(&(dlMeter->object), &(p->structure.meter->object));
    monitorAttributeCopy(&(dlMeter->monitor), &(p->structure.meter->monitor));
    overrideAttributeCopy(&(dlMeter->override), &(p->structure.meter->override));
    dlMeter->label = p->structure.meter->label;
    dlMeter->clrmod = p->structure.meter->clrmod;
  } else {
    objectAttributeInit(&(dlMeter->object));
    monitorAttributeInit(&(dlMeter->monitor));
    overrideAttributeInit(&(dlMeter->override));
    dlMeter->label = LABEL_NONE;
    dlMeter->clrmod = STATIC;
  }

  dlElement = createDlElement(DL_Meter, (XtPointer) dlMeter, 
			      &meterDlDispatchTable);
  
  if (dlElement == NULL) 
     destroyDlMeter(dlMeter);


  return(dlElement);
}


DlElement *parseMeter(DisplayInfo *displayInfo)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;
  DlMeter *dlMeter;
  DlElement *dlElement = createDlMeter(NULL);
  int i = 0;

  if (!dlElement) return 0;
  dlMeter = dlElement->structure.meter;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"object"))
	parseObject(displayInfo,&(dlMeter->object));
      else if (STREQL(token,"monitor"))
	parseMonitor(displayInfo,&(dlMeter->monitor));
      else if (STREQL(token,"override"))
	parseOverride(displayInfo,&(dlMeter->override));
      else if (STREQL(token,"label")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	for (i=FIRST_LABEL_TYPE;i<FIRST_LABEL_TYPE+NUM_LABEL_TYPES;i++) {
	  if (STREQL(token,stringValueTable[i])) {
	    dlMeter->label = (LabelType)i;
	    break;
	  }
	}
      } else if (STREQL(token,"clrmod")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	for (i=FIRST_COLOR_MODE;i<FIRST_COLOR_MODE+NUM_COLOR_MODES;i++) {
	  if (STREQL(token,stringValueTable[i])) {
	    dlMeter->clrmod = (ColorMode)i;
	    break;
	  }
	}
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

void writeDlMeter
   (FILE *stream,
    DlElement *dlElement,
    int level)
{
  int i;
  char indent[256]; level=MIN(level,256-2);
  DlMeter *dlMeter = dlElement->structure.meter;

  for (i = 0;  i < MIN(level,256-2); i++) indent[i] = '\t';
  indent[i] = '\0';

  fprintf(stream,"\n%smeter {",indent);
  writeDlObject(stream,&(dlMeter->object),level+1);
  writeDlMonitor(stream,&(dlMeter->monitor),level+1);
  writeDlOverride(stream,&(dlMeter->override),level+1);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
    if (dlMeter->label != LABEL_NONE)
      fprintf(stream,"\n%s\tlabel=\"%s\"",indent,stringValueTable[dlMeter->label]);
    if (dlMeter->clrmod != STATIC) 
      fprintf(stream,"\n%s\tclrmod=\"%s\"",indent,stringValueTable[dlMeter->clrmod]);
#ifdef SUPPORT_0201XX_FILE_FORMAT	
  } else {
    fprintf(stream,"\n%s\tlabel=\"%s\"",indent,stringValueTable[dlMeter->label]);
    fprintf(stream,"\n%s\tclrmod=\"%s\"",indent,stringValueTable[dlMeter->clrmod]);
  }
#endif
  fprintf(stream,"\n%s}",indent);
}

static void meterInheritValues(ResourceBundle *pRCB, DlElement *p) {
  DlMeter *dlMeter = p->structure.meter;
  dm2kGetValues(pRCB,
		RDBK_RC,       &(dlMeter->monitor.rdbk),
		CLR_RC,        &(dlMeter->monitor.clr),
		BCLR_RC,       &(dlMeter->monitor.bclr),
		LABEL_RC,      &(dlMeter->label),
		CLRMOD_RC,     &(dlMeter->clrmod),
		LOW_LIMIT_RC,  &(dlMeter->override.displayLowLimit),
		HIGH_LIMIT_RC, &(dlMeter->override.displayHighLimit),
		VAL_PRECISION_RC, &(dlMeter->override.displayPrecision),
		-1);
}

static void meterGetValues(ResourceBundle *pRCB, DlElement *p) {
  DlMeter *dlMeter = p->structure.meter;
  dm2kGetValues(pRCB,
		X_RC,          &(dlMeter->object.x),
		Y_RC,          &(dlMeter->object.y),
		WIDTH_RC,      &(dlMeter->object.width),
		HEIGHT_RC,     &(dlMeter->object.height),
		RDBK_RC,       &(dlMeter->monitor.rdbk),
		CLR_RC,        &(dlMeter->monitor.clr),
		BCLR_RC,       &(dlMeter->monitor.bclr),
		LABEL_RC,      &(dlMeter->label),
		CLRMOD_RC,     &(dlMeter->clrmod),
		LOW_LIMIT_RC,  &(dlMeter->override.displayLowLimit),
		HIGH_LIMIT_RC, &(dlMeter->override.displayHighLimit),
		VAL_PRECISION_RC, &(dlMeter->override.displayPrecision),
		-1);
}
