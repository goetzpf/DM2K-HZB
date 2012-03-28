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
 *                              - using the new screen update mechanism
 * .03  09-11-95        vong    conform to c++ syntax
 * .04  09-25-95        vong    add the primitive color rule set
 *
 *****************************************************************************
*/

#include "dm2k.h"

typedef struct _Arc {
  DlElement        *dlElement;
  Record           *record;
  UpdateTask       *updateTask;
} Arc;

static void arcDraw(XtPointer cd);
static void arcUpdateValueCb(XtPointer cd);
static void arcDestroyCb(XtPointer cd);
static void arcName(XtPointer, char **, short *, int *);
static void arcInheritValues(ResourceBundle *pRCB, DlElement *p);
static void arcGetValues(ResourceBundle *pRCB, DlElement *p);

static void destroyDlElement (DlElement * element);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);

static DlDispatchTable arcDlDispatchTable = {
         createDlArc,
	 destroyDlElement,
         executeMethod,
         writeDlArc,
         NULL,
         arcGetValues,
         arcInheritValues,
         NULL,
         NULL,
         genericMove,
         genericScale,
         NULL};


static void destroyDlArc (DlArc * dlArc)
{
  if (dlArc == NULL)
    return;

  objectAttributeDestroy(&(dlArc->object));
  basicAttributeDestroy(&(dlArc->attr));
  dynamicAttributeDestroy(&(dlArc->dynAttr));

  free((char*)dlArc);
}

static void destroyDlElement (DlElement * element)
{
  if (element == NULL || element->type != DL_Arc) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  destroyDlArc (element->structure.arc);
  free((char*)element);
}

static void drawArc(Arc *pa) 
{
  unsigned int lineWidth;
  DisplayInfo *displayInfo = pa->updateTask->displayInfo;
  Widget widget = pa->updateTask->displayInfo->drawingArea;
  Display *display = XtDisplay(widget);
  DlArc *dlArc = pa->dlElement->structure.arc;

  lineWidth = (dlArc->attr.width+1)/2;

  if (dlArc->object.width == 0 || dlArc->object.height == 0)
    return;

  if (   dlArc->object.width <= 2*lineWidth 
      || dlArc->object.height <= 2*lineWidth
      || dlArc->attr.fill == F_SOLID) 
    {
      XFillArc(display,XtWindow(widget),displayInfo->gc,
	       dlArc->object.x,dlArc->object.y,
	       dlArc->object.width,dlArc->object.height,
	       dlArc->begin,dlArc->path);
    } 
  else if (dlArc->attr.fill == F_OUTLINE) 
    {
      XDrawArc(display,XtWindow(widget),displayInfo->gc,
	       dlArc->object.x + lineWidth,
	       dlArc->object.y + lineWidth,
	       dlArc->object.width - 2*lineWidth,
	       dlArc->object.height - 2*lineWidth,dlArc->begin,dlArc->path);
    }
}

static void setArcGC(DisplayInfo * displayInfo,
		     DlArc       * dlArc,
		     Record      * pd)
{
  Display       * display;
  XGCValues       gcValues;
  unsigned long   gcValueMask;

  if (displayInfo == NULL || dlArc == NULL) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  display = XtDisplay(displayInfo->drawingArea);

  gcValueMask = GCForeground|GCLineWidth|GCLineStyle;

  switch (dlArc->dynAttr.clr) {
  case DISCRETE:
    if (pd) {
      gcValues.foreground = extractColor(displayInfo,
					 pd->value,
					 dlArc->dynAttr.colorRule,
					 dlArc->attr.clr);
      break;
    }
  case STATIC :
    gcValues.foreground = displayInfo->colormap[dlArc->attr.clr];
    break;
  case ALARM :
    gcValues.foreground = alarmColorPixel[pd->severity];
    break;
  default :
    INFORM_INTERNAL_ERROR();
    gcValues.foreground = displayInfo->colormap[dlArc->attr.clr]; 
    break;
  }

  gcValues.line_width = dlArc->attr.width;
  gcValues.line_style = ((dlArc->attr.style == SOLID) ? 
			 LineSolid : LineOnOffDash);
  XChangeGC(display,displayInfo->gc,gcValueMask,&gcValues);
}

static UpdateTask * executeMethod
   (DisplayInfo * displayInfo, 
    DlElement   * dlElement)
{
  DlArc      * dlArc = dlElement->structure.arc;
  UpdateTask * updateTask = NULL;

  if (displayInfo->traversalMode == DL_EXECUTE && dlArc->dynAttr.chan != NULL)
  {
    Arc * pa = DM2KALLOC(Arc);
    if (pa == NULL) {
      dm2kPrintf("UpdateTask: memory allocation error\n");
      return updateTask;
    }
    
    pa->dlElement = dlElement;
    updateTask = pa->updateTask = updateTaskAddTask(displayInfo,
						    &(dlArc->object),
						    arcDraw,
						    (XtPointer)pa);
    
    if (pa->updateTask == NULL) {
      dm2kPrintf("arcCreateRunTimeInstance : memory allocation error\n");
    } else {
      updateTaskAddDestroyCb(pa->updateTask,arcDestroyCb);
      updateTaskAddNameCb(pa->updateTask,arcName);
      pa->updateTask->opaque = False;
    }
    
    pa->record = dm2kAllocateRecord(dlArc->dynAttr.chan,
				    arcUpdateValueCb,
				    NULL,
				    (XtPointer) pa);
      
    switch (dlArc->dynAttr.clr) 
      {
      case STATIC :
	pa->record->monitorValueChanged = False;
	pa->record->monitorSeverityChanged = False;
	break;
	
      case ALARM :
	pa->record->monitorValueChanged = False;
	break;
	
      case DISCRETE :
	pa->record->monitorSeverityChanged = False;
	break;
      }
    
      if (dlArc->dynAttr.vis == V_STATIC ) 
	pa->record->monitorZeroAndNoneZeroTransition = False;
  }
  else if (dlArc->attr.fill == F_SOLID) 
  {
    if (dlArc->object.width == 0 || dlArc->object.height == 0)
      return updateTask;
    
    setArcGC(displayInfo, dlArc, NULL);
    
    XFillArc(display,XtWindow(displayInfo->drawingArea),displayInfo->gc,
	     dlArc->object.x,dlArc->object.y,
	     dlArc->object.width,dlArc->object.height,
	     dlArc->begin,dlArc->path);
    
    XFillArc(display,displayInfo->drawingAreaPixmap,displayInfo->gc,
	     dlArc->object.x,dlArc->object.y,
	     dlArc->object.width,dlArc->object.height,
	     dlArc->begin,dlArc->path);
  } 
  else 
  {
    unsigned int lineWidth = (dlArc->attr.width+1)/2;
    
    executeDlBasicAttribute(displayInfo,&(dlArc->attr));
    
    setArcGC(displayInfo, dlArc, NULL);
    
    if (   dlArc->object.width <= 2*lineWidth 
	   || dlArc->object.height <= 2*lineWidth) 
    {
      XDrawPoint(display,XtWindow(displayInfo->drawingArea),
		 displayInfo->gc,
		 dlArc->object.x, dlArc->object.y);
      
      XDrawPoint(display,displayInfo->drawingAreaPixmap,
		 displayInfo->gc,
		 dlArc->object.x, dlArc->object.y);
    } 
    else if(dlArc->attr.fill == F_OUTLINE) 
    {
      XDrawArc(display,XtWindow(displayInfo->drawingArea),displayInfo->gc,
	       dlArc->object.x + lineWidth,
	       dlArc->object.y + lineWidth,
	       dlArc->object.width - 2*lineWidth,
	       dlArc->object.height - 2*lineWidth,
	       dlArc->begin,dlArc->path);
      
      XDrawArc(display,displayInfo->drawingAreaPixmap,displayInfo->gc,
	       dlArc->object.x + lineWidth,
	       dlArc->object.y + lineWidth,
	       dlArc->object.width - 2*lineWidth,
	       dlArc->object.height - 2*lineWidth,
	       dlArc->begin,dlArc->path);
    }
  }

  return updateTask;
}

static void arcUpdateValueCb(XtPointer cd) {
  Arc *pa = (Arc *) ((Record *) cd)->clientData;
  updateTaskMarkUpdate(pa->updateTask);
}


static void arcDraw(XtPointer cd) 
{
  Arc           * pa = (Arc *) cd;
  Record        * pd = pa->record;
  DisplayInfo   * displayInfo = pa->updateTask->displayInfo;
  XGCValues       gcValues;
  unsigned long   gcValueMask;
  Display       * display = XtDisplay(pa->updateTask->displayInfo->drawingArea);
  DlArc         * dlArc = pa->dlElement->structure.arc;


  if (pd->connected) {
    setArcGC(displayInfo, dlArc, pd);
  
    switch (dlArc->dynAttr.vis) 
      {
      case V_STATIC:    drawArc(pa);
        break;
      case IF_NOT_ZERO: if (pd->value != 0.0) drawArc(pa);
        break;
      case IF_ZERO:     if (pd->value == 0.0) drawArc(pa);
        break;
      default :
	INFORM_INTERNAL_ERROR();
        break;
      }

    if (pd->readAccess) {
      if (!pa->updateTask->overlapped && dlArc->dynAttr.vis == V_STATIC) {
        pa->updateTask->opaque = True;
      }
    } 
    else {
      pa->updateTask->opaque = False;
      draw3DQuestionMark(pa->updateTask);
    }
  } 
  else {
    gcValueMask = GCForeground|GCLineWidth|GCLineStyle;
    gcValues.foreground = WhitePixel(display,DefaultScreen(display));
    gcValues.line_width = dlArc->attr.width;
    gcValues.line_style = ((dlArc->attr.style == SOLID) ?
			   LineSolid : LineOnOffDash);
    XChangeGC(display,displayInfo->gc,gcValueMask,&gcValues);
    drawArc(pa);
  }
}

static void arcDestroyCb(XtPointer cd) 
{
  Arc *pa = (Arc *) cd;

  if (pa) {
    dm2kDestroyRecord(pa->record);
    free((char *)pa);
  }
  return;
}

static void arcName(XtPointer cd, char **name, short *severity, int *count) {
  Arc *pa = (Arc *) cd;
  *count = 1;
  name[0] = pa->record->name;
  severity[0] = pa->record->severity;
}

DlElement *createDlArc(DlElement *p)
{
  DlArc     * dlArc;
  DlElement * dlElement;
 
  dlArc = DM2KALLOC(DlArc);

  if (dlArc == NULL) 
    return NULL;

  if (p != NULL) {
    objectAttributeCopy(&(dlArc->object), &(p->structure.arc->object));
    basicAttributeCopy(&(dlArc->attr), &(p->structure.arc->attr));
    dynamicAttributeCopy(&(dlArc->dynAttr), &(p->structure.arc->dynAttr));

    dlArc->begin = p->structure.arc->begin;
    dlArc->path = p->structure.arc->path;
  } 
  else { 
    objectAttributeInit(&(dlArc->object));
    basicAttributeInit(&(dlArc->attr));
    dynamicAttributeInit(&(dlArc->dynAttr));
    dlArc->begin = 0;
    dlArc->path = 90*64;
  }
 
  dlElement = createDlElement(DL_Arc, (XtPointer) dlArc, &arcDlDispatchTable);

  if (dlElement == NULL)
    destroyDlArc(dlArc);

  return(dlElement);
}

DlElement *parseArc(DisplayInfo *displayInfo)
{
  char        token[MAX_TOKEN_LENGTH];
  TOKEN       tokenType;
  int         nestingLevel = 0;
  DlArc     * dlArc;
  DlElement * dlElement = createDlArc(NULL);

  if (dlElement == NULL)
    return 0;

  dlArc = dlElement->structure.arc;
 
  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
      case T_WORD:
        if (STREQL(token,"object"))
          parseObject(displayInfo,&(dlArc->object));
        else if (STREQL(token,"basic attribute"))
          parseBasicAttribute(displayInfo,&(dlArc->attr));
        else if (STREQL(token,"dynamic attribute"))
          parseDynamicAttribute(displayInfo,&(dlArc->dynAttr));
        else if (STREQL(token,"begin")) 
	{
          getToken(displayInfo,token);
          getToken(displayInfo,token);
          dlArc->begin = atoi(token);
        } 
	else if (STREQL(token,"path")) 
	{
          getToken(displayInfo,token);
          getToken(displayInfo,token);
          dlArc->path = atoi(token);
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

void writeDlArc(
  FILE *stream,
  DlElement *dlElement,
  int level)
{
  char indent[265];
  DlArc *dlArc = dlElement->structure.arc;

  level = MIN(level, 256-2);
  memset(indent,'\t',level); 
  indent[level] = '\0';

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
    fprintf(stream,"\n%sarc {",indent);
    writeDlObject(stream,&(dlArc->object),level+1);
    writeDlBasicAttribute(stream,&(dlArc->attr),level+1);
    writeDlDynamicAttribute(stream,&(dlArc->dynAttr),DYNATTR_ALL,level+1);
    fprintf(stream,"\n%s\tbegin=%d",indent,dlArc->begin);
    fprintf(stream,"\n%s\tpath=%d",indent,dlArc->path);
    fprintf(stream,"\n%s}",indent);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    writeDlObject(stream,&(dlArc->object),level);
    writeDlBasicAttribute(stream,&(dlArc->attr),level);
    fprintf(stream,"\n%sarc {",indent);
    writeDlDynamicAttribute(stream,&(dlArc->dynAttr),DYNATTR_ALL,level+1);
    fprintf(stream,"\n%s\tbegin=%d",indent,dlArc->begin);
    fprintf(stream,"\n%s\tpath=%d",indent,dlArc->path);
    fprintf(stream,"\n%s}",indent);
  }
#endif
}

static void arcGetValues(ResourceBundle *pRCB, DlElement *p) 
{
  register DlArc *dlArc = p->structure.arc;

  dm2kGetValues(pRCB,
		X_RC,          &(dlArc->object.x),
		Y_RC,          &(dlArc->object.y),
		WIDTH_RC,      &(dlArc->object.width),
		HEIGHT_RC,     &(dlArc->object.height),
		CLR_RC,        &(dlArc->attr.clr),
		STYLE_RC,      &(dlArc->attr.style),
		FILL_RC,       &(dlArc->attr.fill),
		LINEWIDTH_RC,  &(dlArc->attr.width),
		CLRMOD_RC,     &(dlArc->dynAttr.clr),
		VIS_RC,        &(dlArc->dynAttr.vis),
		COLOR_RULE_RC, &(dlArc->dynAttr.colorRule),
		CHAN_RC,       &(dlArc->dynAttr.chan),
		BEGIN_RC,      &(dlArc->begin),
		PATH_RC,       &(dlArc->path),
		-1);
}

static void arcInheritValues(ResourceBundle *pRCB, DlElement *p) 
{
  register DlArc *dlArc = p->structure.arc;

  dm2kGetValues(pRCB,
		CLR_RC,        &(dlArc->attr.clr),
		STYLE_RC,      &(dlArc->attr.style),
		FILL_RC,       &(dlArc->attr.fill),
		LINEWIDTH_RC,  &(dlArc->attr.width),
		CLRMOD_RC,     &(dlArc->dynAttr.clr),
		VIS_RC,        &(dlArc->dynAttr.vis),
		COLOR_RULE_RC, &(dlArc->dynAttr.colorRule),
		CHAN_RC,       &(dlArc->dynAttr.chan),
		BEGIN_RC,      &(dlArc->begin),
		PATH_RC,       &(dlArc->path),
		-1);
}
