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
a period of five years from Mpolygonh 30, 1993, the Government is
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
 * .03  09-12-95        vong    - conform to c++ syntax
 * .04  09-25-95        vong    add the primitive color rule set
 *
 *****************************************************************************
*/

#include "dm2k.h"

/* from utils.c - get XOR GC */
extern GC xorGC;

/* for use in handlePoly*Create() functions */
static const double okRadiansTable[24]  =    { 0.,
                                        1.*M_PI/4., 1.*M_PI/4.,
                                        2.*M_PI/4., 2.*M_PI/4.,
                                        3.*M_PI/4., 3.*M_PI/4.,
                                        4.*M_PI/4., 4.*M_PI/4.,
                                        5.*M_PI/4., 5.*M_PI/4.,
                                        6.*M_PI/4., 6.*M_PI/4.,
                                        7.*M_PI/4., 7.*M_PI/4.,
                                        0.};

typedef struct _Polygon {
  DlElement       *dlElement;
  Record           *record;
  UpdateTask       *updateTask;
} Polygon;

static void polygonDraw(XtPointer cd);
static void polygonUpdateValueCb(XtPointer cd);
static void polygonDestroyCb(XtPointer cd);
static void polygonName(XtPointer, char **, short *, int *);
static void polygonGetValues(ResourceBundle *pRCB, DlElement *p);
static void polygonInheritValues(ResourceBundle *pRCB, DlElement *p);
static void polygonMove(DlElement *, int, int);
static void polygonScale(DlElement *, int, int);
static int handlePolygonVertexManipulation(DlElement *, int, int);

static void destroyDlElement (DlElement * element);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);

static DlDispatchTable polygonDlDispatchTable = {
         createDlPolygon,
         destroyDlElement,
         executeMethod,
         writeDlPolygon,
         NULL,
         polygonGetValues,
         polygonInheritValues,
         NULL,
         NULL,
         polygonMove,
         polygonScale,
         handlePolygonVertexManipulation};

static void destroyDlPolygon(DlPolygon *dlPolygon)
{
  if (dlPolygon == NULL)
    return;

  objectAttributeDestroy(&(dlPolygon->object));
  basicAttributeDestroy(&(dlPolygon->attr));
  dynamicAttributeDestroy(&(dlPolygon->dynAttr));

  DM2KFREE(dlPolygon->points);
  free ((char *)dlPolygon);
}

static void destroyDlElement(register DlElement *dlElement) 
{
  destroyDlPolygon(dlElement->structure.polygon);
   free ((char *)dlElement);
}

static void drawPolygon(Polygon *pp) 
{
  DisplayInfo *displayInfo = pp->updateTask->displayInfo;
  Widget widget = pp->updateTask->displayInfo->drawingArea;
  Display *display = XtDisplay(widget);
  DlPolygon *dlPolygon = pp->dlElement->structure.polygon;

  if (dlPolygon->attr.fill == F_SOLID) {
    XFillPolygon(display,XtWindow(widget),displayInfo->gc,
        dlPolygon->points,dlPolygon->nPoints,Complex,CoordModeOrigin);
  } else
  if (dlPolygon->attr.fill == F_OUTLINE) {
    XDrawLines(display,XtWindow(widget),displayInfo->gc,
        dlPolygon->points,dlPolygon->nPoints,CoordModeOrigin);
  }
}

static UpdateTask * executeMethod
   (DisplayInfo * displayInfo, 
    DlElement   * dlElement)
{
  DlPolygon *dlPolygon = dlElement->structure.polygon;
  UpdateTask * updateTask = NULL;


  if ((displayInfo->traversalMode == DL_EXECUTE) 
      && (dlPolygon->dynAttr.chan != NULL)) {
    Polygon *pp;
    DlObject object;

    pp = DM2KALLOC(Polygon);
    if (pp == NULL) {
      dm2kPrintf("UpdateTask: memory allocation error\n");
      return updateTask;
    }

    pp->dlElement = dlElement;

    object = dlPolygon->object;
    object.width++;
    object.height++;

    updateTask = pp->updateTask = updateTaskAddTask(displayInfo,
						    &object,
						    polygonDraw,
						    (XtPointer)pp);

    if (pp->updateTask == NULL) {
      dm2kPrintf("polygonCreateRunTimeInstance : memory allocation error\n");
    } else {
      updateTaskAddDestroyCb(pp->updateTask,polygonDestroyCb);
      updateTaskAddNameCb(pp->updateTask,polygonName);
      pp->updateTask->opaque = False;
    }

    pp->record = dm2kAllocateRecord(dlPolygon->dynAttr.chan,
				    polygonUpdateValueCb,
				    NULL,
				    (XtPointer) pp);

    switch (dlPolygon->dynAttr.clr) 
      {
      case STATIC :
        pp->record->monitorValueChanged = False;
        pp->record->monitorSeverityChanged = False;
        break;

      case ALARM :
        pp->record->monitorValueChanged = False;
        break;

      case DISCRETE :
        pp->record->monitorSeverityChanged = False;
        break;
      }

    if (dlPolygon->dynAttr.vis == V_STATIC ) {
      pp->record->monitorZeroAndNoneZeroTransition = False;
    }
  } 
  else {
    executeDlBasicAttribute(displayInfo,&(dlPolygon->attr));
    if (dlPolygon->attr.fill == F_SOLID) {
      XFillPolygon(display,XtWindow(displayInfo->drawingArea),displayInfo->gc,
        dlPolygon->points,dlPolygon->nPoints,Complex,CoordModeOrigin);
      XFillPolygon(display,displayInfo->drawingAreaPixmap,displayInfo->gc,
        dlPolygon->points,dlPolygon->nPoints,Complex,CoordModeOrigin);
    } 
    else if (dlPolygon->attr.fill == F_OUTLINE) {
      XDrawLines(display,XtWindow(displayInfo->drawingArea),displayInfo->gc,
        dlPolygon->points,dlPolygon->nPoints,CoordModeOrigin);
      XDrawLines(display,displayInfo->drawingAreaPixmap,displayInfo->gc,
        dlPolygon->points,dlPolygon->nPoints,CoordModeOrigin);
    }
  }

  return updateTask;
}

static void polygonUpdateValueCb(XtPointer cd) {
  Polygon *pp = (Polygon *) ((Record *) cd)->clientData;
  updateTaskMarkUpdate(pp->updateTask);
}

static void polygonDraw(XtPointer cd) 
{
  Polygon       * pp = (Polygon *) cd;
  Record        * pd = pp->record;
  DisplayInfo   * displayInfo = pp->updateTask->displayInfo;
  XGCValues       gcValues;
  unsigned long   gcValueMask;
  DlPolygon     * dlPolygon = pp->dlElement->structure.polygon;

  if (pd->connected) 
    {
    gcValueMask = GCForeground|GCLineWidth|GCLineStyle;

    switch (dlPolygon->dynAttr.clr) 
      {
      case STATIC :
        gcValues.foreground = displayInfo->colormap[dlPolygon->attr.clr];
        break;
      case DISCRETE:
        gcValues.foreground = extractColor(displayInfo,
					   pd->value,
					   dlPolygon->dynAttr.colorRule,
					   dlPolygon->attr.clr);
        break;
      case ALARM :
        gcValues.foreground = alarmColorPixel[pd->severity];
        break;
	default :
	gcValues.foreground = displayInfo->colormap[dlPolygon->attr.clr];
	break;
    }
    
    gcValues.line_width = dlPolygon->attr.width;
    gcValues.line_style = 
      ((dlPolygon->attr.style == SOLID) ? LineSolid : LineOnOffDash);

    XChangeGC(display,displayInfo->gc,gcValueMask,&gcValues);

    switch (dlPolygon->dynAttr.vis) {
      case V_STATIC:
        drawPolygon(pp);
        break;
      case IF_NOT_ZERO:
        if (pd->value != 0.0)
          drawPolygon(pp);
        break;
      case IF_ZERO:
        if (pd->value == 0.0)
          drawPolygon(pp);
        break;
      default :
	INFORM_INTERNAL_ERROR();
        break;
    }

    if (pd->readAccess) {
      if (!pp->updateTask->overlapped && dlPolygon->dynAttr.vis == V_STATIC) {
        pp->updateTask->opaque = True;
      }
    } else {
      pp->updateTask->opaque = False;
      draw3DQuestionMark(pp->updateTask);
    }
  } else {
    gcValueMask = GCForeground|GCLineWidth|GCLineStyle;
    gcValues.foreground = WhitePixel(display,DefaultScreen(display));
    gcValues.line_width = dlPolygon->attr.width;
    gcValues.line_style = 
      ((dlPolygon->attr.style == SOLID) ? LineSolid : LineOnOffDash);

    XChangeGC(display,displayInfo->gc,gcValueMask,&gcValues);
    drawPolygon(pp);
  }
}

static void polygonDestroyCb(XtPointer cd) {
  Polygon *pp = (Polygon *) cd;
  if (pp) {
    dm2kDestroyRecord(pp->record);
    free((char *)pp);
  }
  return;
}

static void polygonName(XtPointer cd, char **name, short *severity, int *count) {
  Polygon *pp = (Polygon *) cd;
  *count = 1;
  name[0] = pp->record->name;
  severity[0] = pp->record->severity;
}


DlElement *createDlPolygon(DlElement *p)
{
  DlPolygon *dlPolygon;
  DlElement *dlElement;
 
 
  dlPolygon = DM2KALLOC(DlPolygon);

  if (dlPolygon == NULL)
    return 0;

  if (p != NULL) {
    objectAttributeCopy(&(dlPolygon->object), &(p->structure.polygon->object));
    basicAttributeCopy(&(dlPolygon->attr), &(p->structure.polygon->attr));
    dynamicAttributeCopy(&(dlPolygon->dynAttr), &(p->structure.polygon->dynAttr));


    dlPolygon->points = NULL;
    dlPolygon->nPoints = p->structure.polygon->nPoints;

    if (dlPolygon->nPoints > 0) {
      int i;

      dlPolygon->points = (XPoint *)calloc(dlPolygon->nPoints,sizeof(XPoint));
      for (i = 0; i < dlPolygon->nPoints; i++) {
	dlPolygon->points[i] = p->structure.polygon->points[i];
      }
    }
  } 
  else {
    objectAttributeInit(&(dlPolygon->object));
    basicAttributeInit(&(dlPolygon->attr));
    dynamicAttributeInit(&(dlPolygon->dynAttr));

    dlPolygon->points = NULL;
    dlPolygon->nPoints = 0;
  }
 
  dlElement = createDlElement(DL_Polygon,
			      (XtPointer) dlPolygon,
			      &polygonDlDispatchTable);

  if (dlElement == NULL)
    destroyDlPolygon(dlPolygon);

  return(dlElement);
}

void parsePolygonPoints(
  DisplayInfo *displayInfo,
  DlPolygon *dlPolygon)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel;
#define INITIAL_NUM_POINTS 16
  int pointsArraySize = INITIAL_NUM_POINTS;

/* initialize some data in structure */
  dlPolygon->nPoints = 0;
  dlPolygon->points = (XPoint *)calloc(pointsArraySize,sizeof(XPoint));

  nestingLevel = 0;
  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"(")) {
	if (dlPolygon->nPoints >= pointsArraySize) {
	  /* reallocate the points array: enlarge by 4X, etc */
	  pointsArraySize *= 4;
	  dlPolygon->points = (XPoint *)
	    realloc((char*)dlPolygon->points,
		    (pointsArraySize+1)*sizeof(XPoint));
	}

	getToken(displayInfo,token);
	dlPolygon->points[dlPolygon->nPoints].x = atoi(token);
	getToken(displayInfo,token);	/* separator	*/
	getToken(displayInfo,token);
	dlPolygon->points[dlPolygon->nPoints].y = atoi(token);
	getToken(displayInfo,token);	/*   ")"	*/
	dlPolygon->nPoints++;
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

  /* ensure closure of the polygon... */
  if (dlPolygon->points[0].x != dlPolygon->points[dlPolygon->nPoints-1].x &&
      dlPolygon->points[0].y != dlPolygon->points[dlPolygon->nPoints-1].y) {
    if (dlPolygon->nPoints >= pointsArraySize) {
      dlPolygon->points = (XPoint *)
	realloc( (char*)dlPolygon->points,
		 (dlPolygon->nPoints+2)*sizeof(XPoint));
    }
    dlPolygon->points[dlPolygon->nPoints].x = dlPolygon->points[0].x;
    dlPolygon->points[dlPolygon->nPoints].y = dlPolygon->points[0].y;
    dlPolygon->nPoints++;
  }

}

DlElement *parsePolygon(DisplayInfo *displayInfo)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;
  DlPolygon *dlPolygon;
  DlElement *dlElement = createDlPolygon(NULL);
  if (!dlElement) return 0;
  dlPolygon = dlElement->structure.polygon;
  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
      case T_WORD:
	if (STREQL(token,"object"))
  	  parseObject(displayInfo,&(dlPolygon->object));
        else
        if (STREQL(token,"basic attribute"))
          parseBasicAttribute(displayInfo,&(dlPolygon->attr));
        else
        if (STREQL(token,"dynamic attribute"))
          parseDynamicAttribute(displayInfo,&(dlPolygon->dynAttr));
	else
        if (STREQL(token,"points"))
	  parsePolygonPoints(displayInfo,dlPolygon);
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

/*
 * function to write all points of polygon out
 */
void writeDlPolygonPoints(
  FILE *stream,
  DlPolygon *dlPolygon,
  int level)
{
  int i;
  char indent[256]; level=MIN(level,256-2);

  level = MIN(level,256-2);
  memset(indent,'\t',level);
  indent[level] = '\0';

  fprintf(stream,"\n%spoints {",indent);
  for (i = 0; i < dlPolygon->nPoints; i++) {
    fprintf(stream,"\n%s\t(%d,%d)",indent,
        dlPolygon->points[i].x,dlPolygon->points[i].y);
  }

  fprintf(stream,"\n%s}",indent);
}


void writeDlPolygon(
  FILE *stream,
  DlElement *dlElement,
  int level)
{
  char indent[256]; level=MIN(level,256-2);
  DlPolygon *dlPolygon = dlElement->structure.polygon;

  memset(indent,'\t',level);
  indent[level] = '\0';

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
    fprintf(stream,"\n%spolygon {",indent);
    writeDlObject(stream,&(dlPolygon->object),level+1);
    writeDlBasicAttribute(stream,&(dlPolygon->attr),level+1);
    writeDlDynamicAttribute(stream,&(dlPolygon->dynAttr),DYNATTR_ALL,level+1);
    writeDlPolygonPoints(stream,dlPolygon,level+1);
    fprintf(stream,"\n%s}",indent);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    writeDlBasicAttribute(stream,&(dlPolygon->attr),level);
    writeDlDynamicAttribute(stream,&(dlPolygon->dynAttr),DYNATTR_ALL,level);
    fprintf(stream,"\n%spolygon {",indent);
    writeDlObject(stream,&(dlPolygon->object),level+1);
    writeDlPolygonPoints(stream,dlPolygon,level+1);
    fprintf(stream,"\n%s}",indent);
  }
#endif
}

/*
 * manipulate a polygon vertex
 */

static int handlePolygonVertexManipulation(
  DlElement *dlElement,
  int x0, int y0)
{
  XEvent event;
  Window window;
  int i, minX, maxX, minY, maxY;
  int x01, y01;
  DlPolygon *dlPolygon = dlElement->structure.polygon;
  int pointIndex = 0;

  int deltaX, deltaY, okIndex;
  double radians, okRadians, length;
  int foundVertex = False;

  window = XtWindow(currentDisplayInfo->drawingArea);
  minX = INT_MAX; maxX = INT_MIN; minY = INT_MAX; maxY = INT_MIN;
 
  for (i = 0; i < dlPolygon->nPoints; i++) {
    x01 = dlPolygon->points[i].x;
    y01 = dlPolygon->points[i].y;
#define TOR 6
    if ((x01 + TOR > x0) && (x01 - TOR < x0) &&
        (y01 + TOR > y0) && (y01 - TOR < y0)) {
      pointIndex = i;
      foundVertex = True;
      break;
    }
#undef TOR 
  }

  if (!foundVertex) return 0;

  XGrabPointer(display,window,FALSE,
	(unsigned int) (PointerMotionMask|ButtonMotionMask|ButtonReleaseMask),
	GrabModeAsync,GrabModeAsync,None,None,CurrentTime);
  XGrabServer(display);


/* now loop until button is released */
  while (TRUE) {

      XtAppNextEvent(appContext,&event);

      switch(event.type) {

	case ButtonRelease:

  /* modify point and leave here */
	    if (event.xbutton.state & ShiftMask) {
/* constrain to 45 degree increments */
	      deltaX = event.xbutton.x - dlPolygon->points[
		     (pointIndex > 0 ? pointIndex-1 : dlPolygon->nPoints-1)].x;
	      deltaY = event.xbutton.y - dlPolygon->points[
		     (pointIndex > 0 ? pointIndex-1 : dlPolygon->nPoints-1)].y;
	      length = sqrt(pow((double)deltaX,2.) + pow((double)deltaY,2.0));
	      radians = atan2((double)(deltaY),(double)(deltaX));
	/* use positive radians */
	      if (radians < 0.) radians = 2*M_PI + radians;
	      okIndex = (int)((radians*8.0)/M_PI);
	      okRadians = okRadiansTable[okIndex];
	      x01 = (int) (cos(okRadians)*length) + dlPolygon->points[
		    (pointIndex > 0 ? pointIndex-1 : dlPolygon->nPoints-1)].x;
	      y01 = (int) (sin(okRadians)*length) + dlPolygon->points[
		    (pointIndex > 0 ? pointIndex-1 : dlPolygon->nPoints-1)].y;
	      dlPolygon->points[pointIndex].x = x01;
	      dlPolygon->points[pointIndex].y = y01;
	    } else {
/* unconstrained */
	      dlPolygon->points[pointIndex].x = event.xbutton.x;
	      dlPolygon->points[pointIndex].y = event.xbutton.y;
	    }

	/* also update 0 or nPoints-1 point to keep order of polygon same */
	    if (pointIndex == 0) {
	      dlPolygon->points[dlPolygon->nPoints-1] = dlPolygon->points[0];
	    } else if (pointIndex == dlPolygon->nPoints-1) {
	      dlPolygon->points[0] = dlPolygon->points[dlPolygon->nPoints-1];
	    }

	    XUngrabPointer(display,CurrentTime);
	    XUngrabServer(display);
	    for (i = 0; i < dlPolygon->nPoints; i++) {
	      minX = MIN(minX,dlPolygon->points[i].x);
	      maxX = MAX(maxX,dlPolygon->points[i].x);
	      minY = MIN(minY,dlPolygon->points[i].y);
	      maxY = MAX(maxY,dlPolygon->points[i].y);
	    }
	    dlPolygon->object.x = minX - globalResourceBundle.lineWidth/2;
	    dlPolygon->object.y = minY - globalResourceBundle.lineWidth/2;
	    dlPolygon->object.width = maxX - minX
					+ globalResourceBundle.lineWidth;
	    dlPolygon->object.height = maxY - minY
					+ globalResourceBundle.lineWidth;
	  /* update global resource bundle  - then do create out of it */
	    globalResourceBundle.x = dlPolygon->object.x;
	    globalResourceBundle.y = dlPolygon->object.y;
	    globalResourceBundle.width = dlPolygon->object.width;
	    globalResourceBundle.height = dlPolygon->object.height;

	    dmTraverseNonWidgetsInDisplayList(currentDisplayInfo);
/* since dmTraverseNonWidgets... clears the window, redraw highlights */
	    highlightSelectedElements();
	    return 1;

	case MotionNotify:
	/* undraw old line segments */
	  if (pointIndex > 0)
	      XDrawLine(display,window,xorGC,
		dlPolygon->points[pointIndex-1].x,
		dlPolygon->points[pointIndex-1].y, x01,y01);
	  else 
	      XDrawLine(display,window,xorGC,
		dlPolygon->points[MAX(dlPolygon->nPoints-2,0)].x,
		dlPolygon->points[MAX(dlPolygon->nPoints-2,0)].y, x01,y01);

	  if (pointIndex < dlPolygon->nPoints-1)
	      XDrawLine(display,window,xorGC,
		dlPolygon->points[pointIndex+1].x,
		dlPolygon->points[pointIndex+1].y, x01,y01);
	  else
	      XDrawLine(display,window,xorGC,
		dlPolygon->points[0].x,dlPolygon->points[0].y, x01,y01);

	  if (event.xmotion.state & ShiftMask) {
/* constrain redraw to 45 degree increments */
	    deltaX =  event.xmotion.x - dlPolygon->points[
		     (pointIndex > 0 ? pointIndex-1 : dlPolygon->nPoints-1)].x;
	    deltaY = event.xmotion.y - dlPolygon->points[
		     (pointIndex > 0 ? pointIndex-1 : dlPolygon->nPoints-1)].y;
	    length = sqrt(pow((double)deltaX,2.) + pow((double)deltaY,2.0));
	    radians = atan2((double)(deltaY),(double)(deltaX));
	/* use positive radians */
	    if (radians < 0.) radians = 2*M_PI + radians;
	    okIndex = (int)((radians*8.0)/M_PI);
	    okRadians = okRadiansTable[okIndex];
	    x01 = (int) (cos(okRadians)*length) + dlPolygon->points[
		     (pointIndex > 0 ? pointIndex-1 : dlPolygon->nPoints-1)].x;
	    y01 = (int) (sin(okRadians)*length) + dlPolygon->points[
		     (pointIndex > 0 ? pointIndex-1 : dlPolygon->nPoints-1)].y;
	  } else {
/* unconstrained */
	    x01 = event.xmotion.x;
	    y01 = event.xmotion.y;
	  }
	/* draw new line segments */
	  if (pointIndex > 0)
	      XDrawLine(display,window,xorGC,
		dlPolygon->points[pointIndex-1].x,
		dlPolygon->points[pointIndex-1].y, x01,y01);
	  else 
	      XDrawLine(display,window,xorGC,
		dlPolygon->points[MAX(dlPolygon->nPoints-2,0)].x,
		dlPolygon->points[MAX(dlPolygon->nPoints-2,0)].y, x01,y01);
	  if (pointIndex < dlPolygon->nPoints-1)
	      XDrawLine(display,window,xorGC,
		dlPolygon->points[pointIndex+1].x,
		dlPolygon->points[pointIndex+1].y, x01,y01);
	  else
	      XDrawLine(display,window,xorGC,
		dlPolygon->points[0].x,dlPolygon->points[0].y, x01,y01);

	  break;

	default:
	  XtDispatchEvent(&event);
	  break;
     }
  }
}

DlElement *handlePolygonCreate(
  int x0, int y0)
{
  XEvent event, newEvent;
  int newEventType;
  Window window;
  DlPolygon *dlPolygon;
  DlElement *dlElement;
#define INITIAL_NUM_POINTS 16
  int pointsArraySize = INITIAL_NUM_POINTS;
  int i, minX, maxX, minY, maxY;
  int x01, y01;

  int deltaX, deltaY, okIndex;
  double radians, okRadians, length;

  window = XtWindow(currentDisplayInfo->drawingArea);
  if (!(dlElement = createDlPolygon(NULL))) return 0;
  dlPolygon = dlElement->structure.polygon;
  polygonInheritValues(&globalResourceBundle,dlElement);
  objectAttributeSet(&(dlPolygon->object),x0,y0,0,0);

  minX = INT_MAX; maxX = INT_MIN; minY = INT_MAX; maxY = INT_MIN;
  
/* first click is first point... */
  dlPolygon->nPoints = 1;
  dlPolygon->points = (XPoint *)calloc((pointsArraySize+3),sizeof(XPoint));
  dlPolygon->points[0].x = (short)x0;
  dlPolygon->points[0].y = (short)y0;
  x01 = x0; y01 = y0;

  XGrabPointer(display,window,FALSE,
	(unsigned int) (PointerMotionMask|ButtonMotionMask|ButtonPressMask|ButtonReleaseMask),
	GrabModeAsync,GrabModeAsync,None,None,CurrentTime);
  XGrabServer(display);


  /* now loop until button is double-clicked */
  while (TRUE) {

      XtAppNextEvent(appContext,&event);

      switch(event.type) {

	case ButtonPress:
	  XWindowEvent(display,XtWindow(currentDisplayInfo->drawingArea),
		ButtonPressMask|Button1MotionMask|PointerMotionMask,&newEvent);
	  newEventType = newEvent.type;
	  if (newEventType == ButtonPress) {
  		/* -> double click... add last point and leave here */
	    if (event.xbutton.state & ShiftMask) {
				/* constrain to 45 degree increments */
	      deltaX = event.xbutton.x - dlPolygon->points[dlPolygon->nPoints-1].x;
	      deltaY = event.xbutton.y - dlPolygon->points[dlPolygon->nPoints-1].y;
	      length = sqrt(pow((double)deltaX,2.) + pow((double)deltaY,2.0));
	      radians = atan2((double)(deltaY),(double)(deltaX));
				/* use positive radians */
	      if (radians < 0.) radians = 2*M_PI + radians;
	      okIndex = (int)((radians*8.0)/M_PI);
	      okRadians = okRadiansTable[okIndex];
	      x01 = (int) (cos(okRadians)*length)
					+ dlPolygon->points[dlPolygon->nPoints-1].x;
	      y01 = (int) (sin(okRadians)*length)
					+ dlPolygon->points[dlPolygon->nPoints-1].y;
	      dlPolygon->nPoints++;
	      dlPolygon->points[dlPolygon->nPoints-1].x = x01;
	      dlPolygon->points[dlPolygon->nPoints-1].y = y01;
	    } else {
				/* unconstrained */
	      dlPolygon->nPoints++;
	      dlPolygon->points[dlPolygon->nPoints-1].x = event.xbutton.x;
	      dlPolygon->points[dlPolygon->nPoints-1].y = event.xbutton.y;
	    }
	    XUngrabPointer(display,CurrentTime);
	    XUngrabServer(display);
	    for (i = 0; i < dlPolygon->nPoints; i++) {
	      minX = MIN(minX,dlPolygon->points[i].x);
	      maxX = MAX(maxX,dlPolygon->points[i].x);
	      minY = MIN(minY,dlPolygon->points[i].y);
	      maxY = MAX(maxY,dlPolygon->points[i].y);
	    }
	/* to ensure closure, make sure last point = first point */
	    if(!(dlPolygon->points[0].x ==
				dlPolygon->points[dlPolygon->nPoints-1].x &&
				dlPolygon->points[0].y ==
				dlPolygon->points[dlPolygon->nPoints-1].y)) {
	      dlPolygon->points[dlPolygon->nPoints] = dlPolygon->points[0];
	      dlPolygon->nPoints++;
      }

	    if (dlPolygon->attr.fill == F_SOLID) {
				dlPolygon->object.x = minX;
 				dlPolygon->object.y = minY;
				dlPolygon->object.width = maxX - minX;
				dlPolygon->object.height = maxY - minY;
	    } else {            /* F_OUTLINE, therfore lineWidth is a factor */
				dlPolygon->object.x = minX - dlPolygon->object.width/2;
				dlPolygon->object.y = minY - dlPolygon->object.width/2;
				dlPolygon->object.width = maxX - minX
					+ dlPolygon->object.width;
				dlPolygon->object.height = maxY - minY
					+ dlPolygon->object.width;
	    }

	    XCopyArea(display,currentDisplayInfo->drawingAreaPixmap,
				XtWindow(currentDisplayInfo->drawingArea),
				currentDisplayInfo->pixmapGC,
				dlPolygon->object.x, dlPolygon->object.y,
				dlPolygon->object.width, dlPolygon->object.height,
				dlPolygon->object.x, dlPolygon->object.y);
	    XBell(display,50); XBell(display,50);
	    return (dlElement);

	  } else {
			/* not last point: more points to add.. */
				/* undraw old line segments */
	    XDrawLine(display,window,xorGC,
				dlPolygon->points[dlPolygon->nPoints-1].x,
				dlPolygon->points[dlPolygon->nPoints-1].y,
				x01, y01);
	    if (dlPolygon->nPoints > 1) XDrawLine(display,window,xorGC,
				dlPolygon->points[0].x, dlPolygon->points[0].y, x01, y01);

	/* new line segment added: update coordinates */
	    if (dlPolygon->nPoints >= pointsArraySize) {
	    /* reallocate the points array: enlarge by 4X, etc */
				pointsArraySize *= 4;
        dlPolygon->points = (XPoint *)realloc(
						 (char*)dlPolygon->points,
                           (pointsArraySize+3)*sizeof(XPoint));
	    }

	    if (event.xbutton.state & ShiftMask) {
/* constrain to 45 degree increments */
	      deltaX = event.xbutton.x - dlPolygon->points[dlPolygon->nPoints-1].x;
	      deltaY = event.xbutton.y - dlPolygon->points[dlPolygon->nPoints-1].y;
	      length = sqrt(pow((double)deltaX,2.) + pow((double)deltaY,2.0));
	      radians = atan2((double)(deltaY),(double)(deltaX));
	/* use positive radians */
	      if (radians < 0.) radians = 2*M_PI + radians;
	      okIndex = (int)((radians*8.0)/M_PI);
	      okRadians = okRadiansTable[okIndex];
	      x01 = (int) (cos(okRadians)*length)
					+ dlPolygon->points[dlPolygon->nPoints-1].x;
	      y01 = (int) (sin(okRadians)*length)
					+ dlPolygon->points[dlPolygon->nPoints-1].y;
	      dlPolygon->nPoints++;
	      dlPolygon->points[dlPolygon->nPoints-1].x = x01;
	      dlPolygon->points[dlPolygon->nPoints-1].y = y01;
	    } else {
/* unconstrained */
	      dlPolygon->nPoints++;
	      dlPolygon->points[dlPolygon->nPoints-1].x = event.xbutton.x;
	      dlPolygon->points[dlPolygon->nPoints-1].y = event.xbutton.y;
	      x01 = event.xbutton.x; y01 = event.xbutton.y;
	    }
	/* draw new line segments */
	    XDrawLine(display,window,xorGC,
				dlPolygon->points[dlPolygon->nPoints-2].x,
				dlPolygon->points[dlPolygon->nPoints-2].y,
				dlPolygon->points[dlPolygon->nPoints-1].x,
				dlPolygon->points[dlPolygon->nPoints-1].y);
	    if (dlPolygon->nPoints > 1) XDrawLine(display,window,xorGC,
			  dlPolygon->points[0].x,dlPolygon->points[0].y,x01,y01);
	  }
	  break;

	case MotionNotify:
	/* undraw old line segments */
	  XDrawLine(display,window,xorGC,
		dlPolygon->points[dlPolygon->nPoints-1].x,
		dlPolygon->points[dlPolygon->nPoints-1].y,
		x01, y01);
	  if (dlPolygon->nPoints > 1) XDrawLine(display,window,xorGC,
			dlPolygon->points[0].x,dlPolygon->points[0].y, x01, y01);


	  if (event.xmotion.state & ShiftMask) {
/* constrain redraw to 45 degree increments */
	    deltaX = event.xmotion.x -
			dlPolygon->points[dlPolygon->nPoints-1].x;
	    deltaY = event.xmotion.y -
			dlPolygon->points[dlPolygon->nPoints-1].y;
	    length = sqrt(pow((double)deltaX,2.) + pow((double)deltaY,2.0));
	    radians = atan2((double)(deltaY),(double)(deltaX));
	/* use positive radians */
	    if (radians < 0.) radians = 2*M_PI + radians;
	    okIndex = (int)((radians*8.0)/M_PI);
	    okRadians = okRadiansTable[okIndex];
	    x01 = (int) (cos(okRadians)*length)
			 + dlPolygon->points[dlPolygon->nPoints-1].x;
	    y01 = (int) (sin(okRadians)*length)
			 + dlPolygon->points[dlPolygon->nPoints-1].y;
	  } else {
/* unconstrained */
	    x01 = event.xmotion.x;
	    y01 = event.xmotion.y;
	  }
	/* draw new last line segment */
	  XDrawLine(display,window,xorGC,
			dlPolygon->points[dlPolygon->nPoints-1].x,
			dlPolygon->points[dlPolygon->nPoints-1].y,
			x01,y01);
	  if (dlPolygon->nPoints > 1) XDrawLine(display,window,xorGC,
			dlPolygon->points[0].x,dlPolygon->points[0].y, x01,y01);

	  break;

	default:
	  XtDispatchEvent(&event);
	  break;
     }
  }
}

static void polygonMove(DlElement *dlElement, int xOffset, int yOffset) {
  int i;
  XPoint *pts;
  if (dlElement->type != DL_Polygon) return;
  dlElement->structure.polygon->object.x += xOffset;
  dlElement->structure.polygon->object.y += yOffset;
  pts = dlElement->structure.polygon->points;
  for (i = 0; i < dlElement->structure.polygon->nPoints; i++) {
    pts[i].x += xOffset;
    pts[i].y += yOffset;
  }
}

static void polygonScale(DlElement *dlElement, int xOffset, int yOffset) {
  float sX, sY;
  int i;
  DlPolygon *dlPolygon;
  int width, height;

  if (dlElement->type != DL_Polygon) return;
  dlPolygon = dlElement->structure.polygon;

  width = MAX(1,((int)dlPolygon->object.width + xOffset));
  height = MAX(1,((int)dlPolygon->object.height + yOffset));
  sX = (float)((float)width/(float)(dlPolygon->object.width));
  sY = (float)((float)(height)/(float)(dlPolygon->object.height));
  for (i = 0; i < dlPolygon->nPoints; i++) {
    dlPolygon->points[i].x = (short) (dlPolygon->object.x +
      sX*(dlPolygon->points[i].x - dlPolygon->object.x));
    dlPolygon->points[i].y = (short) (dlPolygon->object.y +
      sY*(dlPolygon->points[i].y - dlPolygon->object.y));
  }
  dlPolygon->object.width = width;
  dlPolygon->object.height = height;
}

static void polygonGetValues(ResourceBundle *pRCB, DlElement *p) 
{
  DlPolygon *dlPolygon = p->structure.polygon;
  int x, y;
  unsigned int width, height;
  int xOffset, yOffset;

  dm2kGetValues(pRCB,
		X_RC,          &x,
		Y_RC,          &y,
		WIDTH_RC,      &width,
		HEIGHT_RC,     &height,
		CLR_RC,        &(dlPolygon->attr.clr),
		STYLE_RC,      &(dlPolygon->attr.style),
		FILL_RC,       &(dlPolygon->attr.fill),
		LINEWIDTH_RC,  &(dlPolygon->attr.width),
		CLRMOD_RC,     &(dlPolygon->dynAttr.clr),
		VIS_RC,        &(dlPolygon->dynAttr.vis),
		COLOR_RULE_RC, &(dlPolygon->dynAttr.colorRule),
		CHAN_RC,       &dlPolygon->dynAttr.chan,
		-1);

  xOffset = (int) width - (int) dlPolygon->object.width;
  yOffset = (int) height - (int) dlPolygon->object.height;

  if (xOffset || yOffset)
    polygonScale(p,xOffset,yOffset);

  xOffset = x - dlPolygon->object.x;
  yOffset = y - dlPolygon->object.y;

  if (xOffset || yOffset) 
    polygonMove(p,xOffset,yOffset);
}

static void polygonInheritValues(ResourceBundle *pRCB, DlElement *p) {
  DlPolygon *dlPolygon = p->structure.polygon;
  dm2kGetValues(pRCB,
    CLR_RC,        &(dlPolygon->attr.clr),
    STYLE_RC,      &(dlPolygon->attr.style),
    FILL_RC,       &(dlPolygon->attr.fill),
    LINEWIDTH_RC,  &(dlPolygon->attr.width),
    CLRMOD_RC,     &(dlPolygon->dynAttr.clr),
    VIS_RC,        &(dlPolygon->dynAttr.vis),
    COLOR_RULE_RC, &(dlPolygon->dynAttr.colorRule),
    CHAN_RC,       &(dlPolygon->dynAttr.chan),
    -1);
}

