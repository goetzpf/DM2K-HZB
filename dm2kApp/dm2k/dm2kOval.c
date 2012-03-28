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
 * .04  09-25-95        vong    add the primitive color rule set
 *
 *****************************************************************************
*/

#include "dm2k.h"

typedef struct _Oval {
  DlElement        *dlElement;
  Record           *record;
  UpdateTask       *updateTask;
} Oval;

static void ovalDraw(XtPointer cd);
static void ovalUpdateValueCb(XtPointer cd);
static void ovalDestroyCb(XtPointer cd);
static void ovalName(XtPointer, char **, short *, int *);
static void ovalGetValues(ResourceBundle *pRCB, DlElement *p);
static void ovalInheritValues(ResourceBundle *pRCB, DlElement *p);
#if 0
static void ovalSetValues(ResourceBundle *pRCB, DlElement *p);
#endif
static void ovalGetValues(ResourceBundle *pRCB, DlElement *p);

static void destroyDlElement (DlElement * element);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);

static DlDispatchTable ovalDlDispatchTable = {
         createDlOval,
         destroyDlElement,
         executeMethod,
         writeDlOval,
         NULL,
         ovalGetValues,
         ovalInheritValues,
         NULL,
         NULL,
         genericMove,
         genericScale,
         NULL
};

static void destroyDlOval (DlOval * dlOval)
{
  if (dlOval == NULL)
    return;

  objectAttributeDestroy(&(dlOval->object));
  basicAttributeDestroy(&(dlOval->attr));
  dynamicAttributeDestroy(&(dlOval->dynAttr));

  free((char*)dlOval);
}

static void destroyDlElement (DlElement * element)
{
  if (element == NULL || element->type != DL_Oval) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  destroyDlOval (element->structure.oval);
  free((char*)element);
}

static void drawOval(Oval *po) {
  unsigned int lineWidth;
  DisplayInfo *displayInfo = po->updateTask->displayInfo;
  Widget widget = po->updateTask->displayInfo->drawingArea;
  Display *display = XtDisplay(widget);
  DlOval *dlOval = po->dlElement->structure.oval;

  lineWidth = (dlOval->attr.width+1)/2;
  
  if (dlOval->object.width == 0 || dlOval->object.height == 0)
    return;

  if (   dlOval->object.width <= 2*lineWidth 
      || dlOval->object.height <= 2*lineWidth
      || dlOval->attr.fill == F_SOLID) 
    {
      XFillArc(display,XtWindow(widget),displayInfo->gc,
	       dlOval->object.x,dlOval->object.y,
	       dlOval->object.width,dlOval->object.height,0,360*64);
    } 
  else if (dlOval->attr.fill == F_OUTLINE) 
    {
      XDrawArc(display,XtWindow(widget),displayInfo->gc,
	       dlOval->object.x + lineWidth,
	       dlOval->object.y + lineWidth,
	       dlOval->object.width - 2*lineWidth,
	       dlOval->object.height - 2*lineWidth,0,360*64);
    }
}


static UpdateTask * executeMethod
   (DisplayInfo * displayInfo, 
    DlElement   * dlElement)
{
  DlOval *dlOval = dlElement->structure.oval;
  UpdateTask * updateTask = NULL;

  if ((displayInfo->traversalMode == DL_EXECUTE) 
      && (dlOval->dynAttr.chan != NULL)){

    Oval *po;
    po = DM2KALLOC(Oval);

    if (po == NULL) {
      dm2kPrintf("executeDlOval : memory allocation error\n");
      return updateTask;
    }

    po->dlElement = dlElement;

    updateTask = po->updateTask = updateTaskAddTask(displayInfo,
				       &(dlOval->object),
				       ovalDraw,
				       (XtPointer)po);

    if (po->updateTask == NULL) {
      dm2kPrintf("ovalCreateRunTimeInstance : memory allocation error\n");
    } else {
      updateTaskAddDestroyCb(po->updateTask,ovalDestroyCb);
      updateTaskAddNameCb(po->updateTask,ovalName);
      po->updateTask->opaque = False;
    }
    po->record = dm2kAllocateRecord(
                  dlOval->dynAttr.chan,
                  ovalUpdateValueCb,
                  NULL,
                  (XtPointer) po);

    switch (dlOval->dynAttr.clr) 
      {
      case STATIC :
        po->record->monitorValueChanged = False;
        po->record->monitorSeverityChanged = False;
        break;

      case ALARM :
        po->record->monitorValueChanged = False;
        break;

      case DISCRETE :
        po->record->monitorSeverityChanged = False;
        break;
      }

  } 
  else 
  {
    unsigned int lineWidth = (dlOval->attr.width+1)/2;

    executeDlBasicAttribute(displayInfo,&(dlOval->attr));

    if (dlOval->object.width == 0 || dlOval->object.height == 0)
      return updateTask;

    if (   dlOval->object.width <= 2*lineWidth 
	|| dlOval->object.height <= 2*lineWidth
	|| dlOval->attr.fill == F_SOLID) 
      {
	XFillArc(display,XtWindow(displayInfo->drawingArea),displayInfo->gc,
		 dlOval->object.x,dlOval->object.y,
		 dlOval->object.width,dlOval->object.height,0,360*64);
	XFillArc(display,displayInfo->drawingAreaPixmap,displayInfo->gc,
		 dlOval->object.x,dlOval->object.y,
		 dlOval->object.width,dlOval->object.height,0,360*64);
	
    } 
    else if (dlOval->attr.fill == F_OUTLINE) 
      {
	XDrawArc(display,XtWindow(displayInfo->drawingArea),displayInfo->gc,
		 dlOval->object.x + lineWidth,
		 dlOval->object.y + lineWidth,
		 dlOval->object.width - 2*lineWidth,
		 dlOval->object.height - 2*lineWidth,0,360*64);
	XDrawArc(display,displayInfo->drawingAreaPixmap,displayInfo->gc,
		 dlOval->object.x + lineWidth,
		 dlOval->object.y + lineWidth,
		 dlOval->object.width - 2*lineWidth,
		 dlOval->object.height - 2*lineWidth,0,360*64);
      }
  }

  return updateTask;
}


static void ovalUpdateValueCb(XtPointer cd) {
  Oval *po = (Oval *) ((Record *) cd)->clientData;
  updateTaskMarkUpdate(po->updateTask);
}

static void ovalDraw(XtPointer cd) {
  Oval *po = (Oval *) cd;
  Record *pd = po->record;
  DisplayInfo *displayInfo = po->updateTask->displayInfo;
  XGCValues gcValues;
  unsigned long gcValueMask;
  Display *display = XtDisplay(po->updateTask->displayInfo->drawingArea);
  DlOval *dlOval = po->dlElement->structure.oval;

  if (pd->connected) {
    gcValueMask = GCForeground|GCLineWidth|GCLineStyle;
    switch (dlOval->dynAttr.clr) 
      {
      case STATIC :
        gcValues.foreground = displayInfo->colormap[dlOval->attr.clr];
        break;

      case DISCRETE:
        gcValues.foreground = extractColor(displayInfo,
					   pd->value,
					   dlOval->dynAttr.colorRule,
					   dlOval->attr.clr);
        break;
	
      case ALARM :
        gcValues.foreground = alarmColorPixel[pd->severity];
        break;
      }
    
    gcValues.line_width = dlOval->attr.width;
    gcValues.line_style = ((dlOval->attr.style == SOLID) ? LineSolid : LineOnOffDash);
    XChangeGC(display,displayInfo->gc,gcValueMask,&gcValues);

    switch (dlOval->dynAttr.vis) 
      {
      case V_STATIC:
        drawOval(po);
        break;

      case IF_NOT_ZERO:
        if (pd->value != 0.0)
          drawOval(po);
        break;

      case IF_ZERO:
        if (pd->value == 0.0)
          drawOval(po);
        break;

      default :
	INFORM_INTERNAL_ERROR();
        break;
      }
    
    if (pd->readAccess) {
      if (!po->updateTask->overlapped && dlOval->dynAttr.vis == V_STATIC) {
        po->updateTask->opaque = True;
      }
    } else {
      po->updateTask->opaque = False;
      draw3DQuestionMark(po->updateTask);
    }
  } 
  else {
    gcValueMask = GCForeground|GCLineWidth|GCLineStyle;
    gcValues.foreground = WhitePixel(display,DefaultScreen(display));
    gcValues.line_width = dlOval->attr.width;
    gcValues.line_style = ((dlOval->attr.style == SOLID) ? LineSolid : LineOnOffDash);
    XChangeGC(display,displayInfo->gc, gcValueMask, &gcValues);
    drawOval(po);
  }
}

static void ovalDestroyCb(XtPointer cd) {
  Oval *po = (Oval *) cd;
  if (po) {
    dm2kDestroyRecord(po->record);
    free((char *)po);
  }
  return;
}

static void ovalName(XtPointer cd, char **name, short *severity, int *count) {
  Oval *po = (Oval *) cd;
  *count = 1;
  name[0] = po->record->name;
  severity[0] = po->record->severity;
}

DlElement *createDlOval(DlElement *p)
{
  DlOval *dlOval;
  DlElement *dlElement;
 
  dlOval = DM2KALLOC(DlOval);

  if (dlOval == NULL) 
    return 0;

  if (p != NULL) {
    objectAttributeCopy(&(dlOval->object), &(p->structure.oval->object));
    basicAttributeCopy(&(dlOval->attr), &(p->structure.oval->attr));
    dynamicAttributeCopy(&(dlOval->dynAttr), &(p->structure.oval->dynAttr));
  } else {
    objectAttributeInit(&(dlOval->object));
    basicAttributeInit(&(dlOval->attr));
    dynamicAttributeInit(&(dlOval->dynAttr));
  }
 
  dlElement = createDlElement(DL_Oval, (XtPointer) dlOval, 
			      &ovalDlDispatchTable);

  if (dlElement == NULL)
    destroyDlOval(dlOval);
  
  return(dlElement);
}

DlElement *parseOval(DisplayInfo *displayInfo)
{
  char        token[MAX_TOKEN_LENGTH];
  TOKEN       tokenType;
  int         nestingLevel = 0;
  DlOval    * dlOval;
  DlElement * dlElement = createDlOval(NULL);

  if (dlElement == NULL)
    return 0;

  dlOval = dlElement->structure.oval;
 
  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"object"))
	parseObject(displayInfo,&(dlOval->object));
      else if (STREQL(token,"basic attribute"))
	parseBasicAttribute(displayInfo,&(dlOval->attr));
      else if (STREQL(token,"dynamic attribute"))
	parseDynamicAttribute(displayInfo,&(dlOval->dynAttr));
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

void writeDlOval(
  FILE *stream,
  DlElement *dlElement,
  int level)
{
  char indent[256]; level=MIN(level,256-2);
  DlOval *dlOval = dlElement->structure.oval;

  level = MIN(level, 256-2);
  memset(indent,'\t',level);
  indent[level] = '\0'; 

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
    fprintf(stream,"\n%soval {",indent);
    writeDlObject(stream,&(dlOval->object),level+1);
    writeDlBasicAttribute(stream,&(dlOval->attr),level+1);
    writeDlDynamicAttribute(stream,&(dlOval->dynAttr),DYNATTR_ALL,level+1);
    fprintf(stream,"\n%s}",indent);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    writeDlBasicAttribute(stream,&(dlOval->attr),level);
    writeDlDynamicAttribute(stream,&(dlOval->dynAttr),DYNATTR_ALL,level);
    fprintf(stream,"\n%soval {",indent);
    writeDlObject(stream,&(dlOval->object),level+1);
    fprintf(stream,"\n%s}",indent);
  }
#endif
}

static void ovalGetValues(ResourceBundle *pRCB, DlElement *p) 
{
  register DlOval *dlOval = p->structure.oval;

  dm2kGetValues(pRCB,
		X_RC,          &(dlOval->object.x),
		Y_RC,          &(dlOval->object.y),
		WIDTH_RC,      &(dlOval->object.width),
		HEIGHT_RC,     &(dlOval->object.height),
		CLR_RC,        &(dlOval->attr.clr),
		STYLE_RC,      &(dlOval->attr.style),
		FILL_RC,       &(dlOval->attr.fill),
		LINEWIDTH_RC,  &(dlOval->attr.width),
		CLRMOD_RC,     &(dlOval->dynAttr.clr),
		VIS_RC,        &(dlOval->dynAttr.vis),
		COLOR_RULE_RC, &(dlOval->dynAttr.colorRule),
		CHAN_RC,       &(dlOval->dynAttr.chan),
		-1);
}

static void ovalInheritValues(ResourceBundle *pRCB, DlElement *p) 
{
  register DlOval *dlOval = p->structure.oval;

  dm2kGetValues(pRCB,
		CLR_RC,        &(dlOval->attr.clr),
		STYLE_RC,      &(dlOval->attr.style),
		FILL_RC,       &(dlOval->attr.fill),
		LINEWIDTH_RC,  &(dlOval->attr.width),
		CLRMOD_RC,     &(dlOval->dynAttr.clr),
		VIS_RC,        &(dlOval->dynAttr.vis),
		COLOR_RULE_RC, &(dlOval->dynAttr.colorRule),
		CHAN_RC,       &(dlOval->dynAttr.chan),
		-1);
}


