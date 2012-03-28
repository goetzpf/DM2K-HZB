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
a period of five years from Mrectangleh 30, 1993, the Government is
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
 * .04  09-25-95        vong    add the primitive color rule set
 *
 *****************************************************************************
*/

#include "dm2k.h"

typedef struct _Rectangle {
  DlElement        *dlElement;
  Record           *record;
  UpdateTask       *updateTask;
} Rectangle;

static void rectangleDraw(XtPointer cd);
static void rectangleUpdateValueCb(XtPointer cd);
static void rectangleDestroyCb(XtPointer cd);
static void rectangleName(XtPointer, char **, short *, int *);
static void rectangleInheritValues(ResourceBundle *pRCB, DlElement *p);
static void rectangleGetValues(ResourceBundle *pRCB, DlElement *p);

static void destroyDlElement (DlElement * element);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);

static DlDispatchTable rectangleDlDispatchTable = {
         createDlRectangle,
         destroyDlElement,
         executeMethod,
         writeDlRectangle,
         NULL,
         rectangleGetValues,
         rectangleInheritValues,
         NULL,
         NULL,
         genericMove,
         genericScale,
         NULL
};

static void destroyDlRectangle (DlRectangle * dlRectangle)
{
  if (dlRectangle == NULL)
    return;

  objectAttributeDestroy(&(dlRectangle->object));
  basicAttributeDestroy(&(dlRectangle->attr));
  dynamicAttributeDestroy(&(dlRectangle->dynAttr));

  free((char*)dlRectangle);
}

static void destroyDlElement (DlElement * element)
{
  if (element == NULL || element->type != DL_Rectangle) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  destroyDlRectangle (element->structure.rectangle);
  free((char*)element);
}

static void _drawRectangle(DisplayInfo * displayInfo,
			   DlRectangle * dlRectangle,
			   Drawable      drawable)
{
  unsigned int  lineWidth;

  if (dlRectangle->attr.width == 0)
    lineWidth = 1;
  else
    lineWidth = (dlRectangle->attr.width+1)/2;

  if (dlRectangle->attr.fill == F_SOLID) {
    XFillRectangle(display, drawable, displayInfo->gc,
		   dlRectangle->object.x,
		   dlRectangle->object.y,
		   dlRectangle->object.width,
		   dlRectangle->object.height);
  } 
  else if (dlRectangle->attr.fill == F_OUTLINE) {
    XDrawRectangle(display, drawable, displayInfo->gc,
		   dlRectangle->object.x + lineWidth,
		   dlRectangle->object.y + lineWidth,
		   dlRectangle->object.width - 2*lineWidth,
		   dlRectangle->object.height - 2*lineWidth);
  }
}

static void drawRectangle(Rectangle * pr) 
{
  if (pr)
    _drawRectangle(pr->updateTask->displayInfo,
		   pr->dlElement->structure.rectangle,
		   XtWindow(pr->updateTask->displayInfo->drawingArea));
  else {
    INFORM_INTERNAL_ERROR();
  }
}

static void setRectangleGC(DisplayInfo * displayInfo,
			   DlRectangle * dlRectangle,
			   Record      * pd)
{
  Display       * display;
  XGCValues       gcValues;
  unsigned long   gcValueMask;

  if (displayInfo == NULL || dlRectangle == NULL) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  display = XtDisplay(displayInfo->drawingArea);

  gcValueMask = GCForeground|GCLineWidth|GCLineStyle;

  switch (dlRectangle->dynAttr.clr) {
  case DISCRETE:
    if (pd) {
      gcValues.foreground = extractColor(displayInfo,
					 pd->value,
					 dlRectangle->dynAttr.colorRule,
					 dlRectangle->attr.clr);
      break;
    }
  case STATIC :
    gcValues.foreground = displayInfo->colormap[dlRectangle->attr.clr];
    break;
  case ALARM :
    if (pd)
      gcValues.foreground = alarmColorPixel[pd->severity];
    else
      gcValues.foreground = displayInfo->colormap[dlRectangle->attr.clr];
    break;
  default :
    INFORM_INTERNAL_ERROR();
    gcValues.foreground = displayInfo->colormap[dlRectangle->attr.clr];
    break;
  }

  gcValues.line_width = dlRectangle->attr.width;
  gcValues.line_style = ((dlRectangle->attr.style == SOLID) ? 
			 LineSolid : LineOnOffDash);
  XChangeGC(display,displayInfo->gc, gcValueMask, &gcValues);
}


static UpdateTask * executeMethod
   (DisplayInfo * displayInfo, 
    DlElement   * dlElement)
{
  DlRectangle *dlRectangle = dlElement->structure.rectangle;
  UpdateTask * updateTask = NULL;

  if ((displayInfo->traversalMode == DL_EXECUTE) 
      && (dlRectangle->dynAttr.chan != NULL))
  {
    Rectangle *pr = DM2KALLOC(Rectangle);

    if (pr == NULL) {
      dm2kPrintf("UpdateTask: memory allocation error\n");
      return updateTask;
    }

    pr->dlElement = dlElement;
    updateTask = pr->updateTask = updateTaskAddTask(displayInfo,
				       &(dlRectangle->object),
				       rectangleDraw,
				       (XtPointer)pr);

    if (pr->updateTask == NULL) {
      dm2kPrintf("rectangleCreateRunTimeInstance : memory allocation error\n");
    } else {
      updateTaskAddDestroyCb(pr->updateTask,rectangleDestroyCb);
      updateTaskAddNameCb(pr->updateTask,rectangleName);
      pr->updateTask->opaque = False;
    }

    pr->record = dm2kAllocateRecord(dlRectangle->dynAttr.chan,
				    rectangleUpdateValueCb,
				    NULL,
				    (XtPointer) pr);

    switch (dlRectangle->dynAttr.clr) 
      {
      case STATIC :
        pr->record->monitorValueChanged = False;
        pr->record->monitorSeverityChanged = False;
        break;
      case ALARM :
        pr->record->monitorValueChanged = False;
        break;
      case DISCRETE :
        pr->record->monitorSeverityChanged = False;
        break;
      }

    if (dlRectangle->dynAttr.vis == V_STATIC ) {
      pr->record->monitorZeroAndNoneZeroTransition = False;
    }
  } 
  else {
    executeDlBasicAttribute(displayInfo,&(dlRectangle->attr));

    setRectangleGC(displayInfo, dlRectangle, NULL);

    _drawRectangle(displayInfo, dlRectangle,
		   XtWindow(displayInfo->drawingArea));
    
    _drawRectangle(displayInfo, dlRectangle,
		   displayInfo->drawingAreaPixmap);
  }

  return updateTask;
}

static void rectangleUpdateValueCb(XtPointer cd) 
{
  Rectangle *pr = (Rectangle *) ((Record *) cd)->clientData;
  updateTaskMarkUpdate(pr->updateTask);
}


static void rectangleDraw(XtPointer cd) 
{
  Rectangle     * pr = (Rectangle *) cd;
  Record        * pd = pr->record;
  DisplayInfo   * displayInfo = pr->updateTask->displayInfo;
  XGCValues       gcValues;
  unsigned long   gcValueMask;
  DlRectangle   * dlRectangle = pr->dlElement->structure.rectangle;

  if (pd->connected) 
  {
    setRectangleGC(displayInfo, dlRectangle, pd);

    switch (dlRectangle->dynAttr.vis) 
    {
      case V_STATIC:
        drawRectangle(pr);
        break;
      case IF_NOT_ZERO:
        if (pd->value != 0.0)
          drawRectangle(pr);
        break;
      case IF_ZERO:
        if (pd->value == 0.0)
          drawRectangle(pr);
        break;
      default :
	INFORM_INTERNAL_ERROR();
        break;
    }
    
    if (pd->readAccess) {
      if (!pr->updateTask->overlapped
	  && dlRectangle->dynAttr.vis == V_STATIC) 
	{
	  pr->updateTask->opaque = True;
	}
    } 
    else {
      pr->updateTask->opaque = False;
      draw3DQuestionMark(pr->updateTask);
    }
  } 
  else {
    gcValueMask = GCForeground|GCLineWidth|GCLineStyle;

    gcValues.foreground = WhitePixel(display,DefaultScreen(display));
    gcValues.line_width = dlRectangle->attr.width;
    gcValues.line_style = ((dlRectangle->attr.style == SOLID) ? 
			   LineSolid : LineOnOffDash);

    XChangeGC(display,displayInfo->gc,gcValueMask,&gcValues);
    drawRectangle(pr);
  }
}

static void rectangleDestroyCb(XtPointer cd) 
{
  Rectangle *pr = (Rectangle *) cd;

  if (pr) {
    dm2kDestroyRecord(pr->record);
    free((char *)pr);
  }
  return;
}

static void rectangleName(XtPointer   cd, 
			  char      ** name, 
			  short      * severity, 
			  int        * count)
{
  Rectangle *pr = (Rectangle *) cd;

  *count = 1;
  name[0] = pr->record->name;
  severity[0] = pr->record->severity;
}


DlElement *createDlRectangle(DlElement *p)
{
  DlRectangle * dlRectangle;
  DlElement   * dlElement;

  dlRectangle = DM2KALLOC(DlRectangle);

  if (dlRectangle == NULL) 
    return NULL;

  if (p != NULL) {
    objectAttributeCopy(&(dlRectangle->object), &(p->structure.rectangle->object)); 
    basicAttributeCopy(&(dlRectangle->attr), &(p->structure.rectangle->attr));
    dynamicAttributeCopy(&(dlRectangle->dynAttr), &(p->structure.rectangle->dynAttr));
  } 
  else {
    objectAttributeInit(&(dlRectangle->object)); 
    basicAttributeInit(&(dlRectangle->attr));
    dynamicAttributeInit(&(dlRectangle->dynAttr));
  }
 
  dlElement = createDlElement(DL_Rectangle, (XtPointer) dlRectangle,
			      &rectangleDlDispatchTable);
  if (dlElement == NULL)
    destroyDlRectangle(dlRectangle);

  return(dlElement);
}

DlElement *parseRectangle(DisplayInfo *displayInfo)
{
  char          token[MAX_TOKEN_LENGTH];
  TOKEN         tokenType;
  int           nestingLevel = 0;
  DlRectangle * dlRectangle;
  DlElement   * dlElement = createDlRectangle(NULL);

  if (dlElement == NULL)
    return 0;

  dlRectangle = dlElement->structure.rectangle;
  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"object"))
	parseObject(displayInfo,&(dlRectangle->object));
      else if (STREQL(token,"basic attribute"))
          parseBasicAttribute(displayInfo,&(dlRectangle->attr));
      else if (STREQL(token,"dynamic attribute"))
	parseDynamicAttribute(displayInfo,&(dlRectangle->dynAttr));
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

void writeDlRectangle(
  FILE *stream,
  DlElement *dlElement,
  int level)
{
  char indent[256]; level=MIN(level,256-2);
  DlRectangle *dlRectangle = dlElement->structure.rectangle;

  level = MIN(level,256-2);
  memset(indent,'\t',level);
  indent[level] = '\0'; 
 
#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
    fprintf(stream,"\n%srectangle {",indent);
    writeDlObject(stream,&(dlRectangle->object),level+1);
    writeDlBasicAttribute(stream,&(dlRectangle->attr),level+1);
    writeDlDynamicAttribute(stream,&(dlRectangle->dynAttr),DYNATTR_ALL,level+1);
    fprintf(stream,"\n%s}",indent);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    writeDlBasicAttribute(stream,&(dlRectangle->attr),level);
    writeDlDynamicAttribute(stream,&(dlRectangle->dynAttr),DYNATTR_ALL,level);
    fprintf(stream,"\n%srectangle {",indent);
    writeDlObject(stream,&(dlRectangle->object),level+1);
    fprintf(stream,"\n%s}",indent);
  }
#endif
}


static void rectangleGetValues(ResourceBundle *pRCB, DlElement *p) {
  DlRectangle *dlRectangle = p->structure.rectangle;
  dm2kGetValues(pRCB,
    X_RC,          &(dlRectangle->object.x),
    Y_RC,          &(dlRectangle->object.y),
    WIDTH_RC,      &(dlRectangle->object.width),
    HEIGHT_RC,     &(dlRectangle->object.height),
    CLR_RC,        &(dlRectangle->attr.clr),
    STYLE_RC,      &(dlRectangle->attr.style),
    FILL_RC,       &(dlRectangle->attr.fill),
    LINEWIDTH_RC,  &(dlRectangle->attr.width),
    CLRMOD_RC,     &(dlRectangle->dynAttr.clr),
    VIS_RC,        &(dlRectangle->dynAttr.vis),
    COLOR_RULE_RC, &(dlRectangle->dynAttr.colorRule),
    CHAN_RC,       &(dlRectangle->dynAttr.chan),
    -1);
}
 
static void rectangleInheritValues(ResourceBundle *pRCB, DlElement *p) {
  DlRectangle *dlRectangle = p->structure.rectangle;
  dm2kGetValues(pRCB,
    CLR_RC,        &(dlRectangle->attr.clr),
    STYLE_RC,      &(dlRectangle->attr.style),
    FILL_RC,       &(dlRectangle->attr.fill),
    LINEWIDTH_RC,  &(dlRectangle->attr.width),
    CLRMOD_RC,     &(dlRectangle->dynAttr.clr),
    VIS_RC,        &(dlRectangle->dynAttr.vis),
    COLOR_RULE_RC, &(dlRectangle->dynAttr.colorRule),
    CHAN_RC,       &(dlRectangle->dynAttr.chan),
    -1);
}

