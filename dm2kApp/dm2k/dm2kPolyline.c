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
a period of five years from Mpolylineh 30, 1993, the Government is
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

#define INITIAL_NUM_POINTS 16

typedef struct _Polyline {
  DlElement        *dlElement;
  Record           *record;
  UpdateTask       *updateTask;
} Polyline;

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

static void polylineUpdateValueCb(XtPointer cd);
static void polylineDraw(XtPointer cd);
static void polylineDestroyCb(XtPointer cd);
static void polylineName(XtPointer, char **, short *, int *);
static void polylineGetValues(ResourceBundle *pRCB, DlElement *p);
static void polylineInheritValues(ResourceBundle *pRCB, DlElement *p);
static void polylineMove(DlElement *dlElement, int xOffset, int yOffset);
static void polylineScale(DlElement *dlElement, int xOffset, int yOffset);
static int handlePolylineVertexManipulation(DlElement *, int, int);

static void destroyDlElement (DlElement * element);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);

static DlDispatchTable polylineDlDispatchTable = {
         createDlPolyline,
         destroyDlElement,
         executeMethod,
         writeDlPolyline,
         NULL,
         polylineGetValues,
         polylineInheritValues,
         NULL,
         NULL,
         polylineMove,
         polylineScale,
         handlePolylineVertexManipulation};

static void destroyDlPolyline(register  DlPolyline *dlPolyline)
{
  if (dlPolyline == NULL)
    return;

  objectAttributeDestroy(&(dlPolyline->object));
  basicAttributeDestroy(&(dlPolyline->attr));
  dynamicAttributeDestroy(&(dlPolyline->dynAttr));

  DM2KFREE(dlPolyline->points);
  free ((char *)dlPolyline);
}

static void destroyDlElement(register DlElement *dlElement) 
{
  destroyDlPolyline(dlElement->structure.polyline);
   free ((char *)dlElement);
}


static void drawPolyline(Polyline *pp) 
{
  DisplayInfo * displayInfo = pp->updateTask->displayInfo;
  Widget        widget = pp->updateTask->displayInfo->drawingArea;
  Display     * display = XtDisplay(widget);
  DlPolyline  * dlPolyline = pp->dlElement->structure.polyline;

  XDrawLines(display,XtWindow(widget),displayInfo->gc,
          dlPolyline->points,dlPolyline->nPoints,CoordModeOrigin);
}


static UpdateTask * executeMethod
   (DisplayInfo * displayInfo, 
    DlElement   * dlElement)
{
  DlPolyline *dlPolyline = dlElement->structure.polyline;
  UpdateTask * updateTask = NULL;


  if (dlPolyline->isFallingOrRisingLine) 
  {
    /*
     * convert the falling line and rising line into polyline format 
     */
    if (dlPolyline->attr.width > 0) 
    {
      int width = dlPolyline->attr.width;
      int halfWidth = width/2;

      if (dlPolyline->points[1].y > dlPolyline->points[0].y) {
        /* falling line */
        dlPolyline->points[0].x += halfWidth;
        dlPolyline->points[0].y += halfWidth;
        dlPolyline->points[1].x -= halfWidth;
        dlPolyline->points[1].y -= halfWidth;
      } 
      else if (dlPolyline->points[1].y < dlPolyline->points[0].y) {
        /* rising line */
        dlPolyline->points[0].x += halfWidth;
        dlPolyline->points[0].y -= width;
        dlPolyline->points[1].x -= width;
        dlPolyline->points[1].y -= halfWidth;
      }
    }
    dlPolyline->isFallingOrRisingLine = False;
  }

  if ((displayInfo->traversalMode == DL_EXECUTE) 
      && (dlPolyline->dynAttr.chan != NULL))
  {
    Polyline *pp;
    DlObject object;

    pp = DM2KALLOC(Polyline);
    if (pp == NULL) {
      dm2kPrintf("UpdateTask: memory allocation error\n");
      return updateTask;
    }

    pp->dlElement = dlElement;

#if 1
    object = dlPolyline->object;
    object.width++;
    object.height++;
#endif

     updateTask = pp->updateTask = updateTaskAddTask(displayInfo,
						     &object,
						     polylineDraw,
						     (XtPointer)pp);
    if (pp->updateTask == NULL) {
      dm2kPrintf("polylineCreateRunTimeInstance : memory allocation error\n");
    } else {
      updateTaskAddDestroyCb(pp->updateTask,polylineDestroyCb);
      updateTaskAddNameCb(pp->updateTask,polylineName);
      pp->updateTask->opaque = False;
    }
    pp->record = dm2kAllocateRecord(
                  dlPolyline->dynAttr.chan,
                  polylineUpdateValueCb,
                  NULL,
                  (XtPointer) pp);

    switch (dlPolyline->dynAttr.clr) 
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

    if (dlPolyline->dynAttr.vis == V_STATIC ) 
      pp->record->monitorZeroAndNoneZeroTransition = False;
  } 
  else {
    executeDlBasicAttribute(displayInfo,&(dlPolyline->attr));
    XDrawLines(display,XtWindow(displayInfo->drawingArea),displayInfo->gc,
          dlPolyline->points,dlPolyline->nPoints,CoordModeOrigin);
    XDrawLines(display,displayInfo->drawingAreaPixmap,displayInfo->gc,
          dlPolyline->points,dlPolyline->nPoints,CoordModeOrigin);
  }

  return updateTask;
}

static void polylineUpdateValueCb(XtPointer cd) {
  Polyline *pp = (Polyline *) ((Record *) cd)->clientData;
  updateTaskMarkUpdate(pp->updateTask);
}

static void polylineDraw(XtPointer cd) {
  Polyline *pp = (Polyline *) cd;
  Record *pd = pp->record;
  DisplayInfo *displayInfo = pp->updateTask->displayInfo;
  XGCValues gcValues;
  unsigned long gcValueMask;
  Widget widget = pp->updateTask->displayInfo->drawingArea;
  Display *display = XtDisplay(widget);
  DlPolyline *dlPolyline = pp->dlElement->structure.polyline;

  if (pd->connected) {
    gcValueMask = GCForeground|GCLineWidth|GCLineStyle;
    switch (dlPolyline->dynAttr.clr) {
      case STATIC :
        gcValues.foreground = displayInfo->colormap[dlPolyline->attr.clr];
        break;
      case DISCRETE:
        gcValues.foreground = extractColor(displayInfo,
                                  pd->value,
                                  dlPolyline->dynAttr.colorRule,
                                  dlPolyline->attr.clr);
        break;
      case ALARM :
        gcValues.foreground = alarmColorPixel[pd->severity];
        break;
      default :
	gcValues.foreground = displayInfo->colormap[dlPolyline->attr.clr];
	break;
    }
    gcValues.line_width = dlPolyline->attr.width;
    gcValues.line_style = ((dlPolyline->attr.style == SOLID) ? LineSolid : LineOnOffDash);
    XChangeGC(display,displayInfo->gc,gcValueMask,&gcValues);

    switch (dlPolyline->dynAttr.vis) {
      case V_STATIC:
        drawPolyline(pp);
        break;
      case IF_NOT_ZERO:
        if (pd->value != 0.0)
          drawPolyline(pp);
        break;
      case IF_ZERO:
        if (pd->value == 0.0)
          drawPolyline(pp);
        break;
      default :
	INFORM_INTERNAL_ERROR();
        break;
    }
    if (pd->readAccess) {
      if (!pp->updateTask->overlapped && dlPolyline->dynAttr.vis == V_STATIC) {
        pp->updateTask->opaque = True;
      }
    } else {
      pp->updateTask->opaque = False;
      draw3DQuestionMark(pp->updateTask);
    }
  } else {
    gcValueMask = GCForeground|GCLineWidth|GCLineStyle;
    gcValues.foreground = WhitePixel(display,DefaultScreen(display));
    gcValues.line_width = dlPolyline->attr.width;
    gcValues.line_style = ((dlPolyline->attr.style == SOLID) ? LineSolid : LineOnOffDash);
    XChangeGC(display,displayInfo->gc,gcValueMask,&gcValues);
    drawPolyline(pp);
  }
}

static void polylineDestroyCb(XtPointer cd) {
  Polyline *pp = (Polyline *) cd;
  if (pp) {
    dm2kDestroyRecord(pp->record);
    free((char *)pp);
  }
  return;
}

static void polylineName(XtPointer cd, char **name, short *severity, int *count) {
  Polyline *pp = (Polyline *) cd;
  *count = 1;
  name[0] = pp->record->name;
  severity[0] = pp->record->severity;
}

DlElement *createDlPolyline(DlElement *p)
{
  DlPolyline * dlPolyline;
  DlElement  * dlElement;
 
 
  dlPolyline = DM2KALLOC(DlPolyline);

  if (dlPolyline == NULL)
    return 0;

  if (p != NULL) {
    int i;

    objectAttributeCopy(&(dlPolyline->object), &(p->structure.polyline->object));
    basicAttributeCopy(&(dlPolyline->attr), &(p->structure.polyline->attr));
    dynamicAttributeCopy(&(dlPolyline->dynAttr), &(p->structure.polyline->dynAttr));

    dlPolyline->points = NULL;
    dlPolyline->nPoints = p->structure.polyline->nPoints;
    dlPolyline->isFallingOrRisingLine = 
      p->structure.polyline->isFallingOrRisingLine;

    if (dlPolyline->nPoints > 0) {
      dlPolyline->points = (XPoint *)calloc(dlPolyline->nPoints,sizeof(XPoint));
      for (i = 0; i < dlPolyline->nPoints; i++) {
	dlPolyline->points[i] = p->structure.polyline->points[i];
      }
    }
  } 
  else {
    objectAttributeInit(&(dlPolyline->object));
    basicAttributeInit(&(dlPolyline->attr));
    dynamicAttributeInit(&(dlPolyline->dynAttr));

    dlPolyline->points = NULL;
    dlPolyline->nPoints = 0;
    dlPolyline->isFallingOrRisingLine = False;
  }
 
  dlElement = createDlElement(DL_Polyline, (XtPointer) dlPolyline,
			      &polylineDlDispatchTable);
  if (dlElement == NULL)
    destroyDlPolyline(dlPolyline);

  return(dlElement);
}

void parsePolylinePoints(
  DisplayInfo *displayInfo,
  DlPolyline *dlPolyline)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel;
  int pointsArraySize = INITIAL_NUM_POINTS;

/* initialize some data in structure */
  dlPolyline->nPoints = 0;
  dlPolyline->points = (XPoint *)calloc(pointsArraySize,sizeof(XPoint));

  nestingLevel = 0;
  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
      case T_WORD:
	if (STREQL(token,"(")) {
	  if (dlPolyline->nPoints >= pointsArraySize) {
	    /* reallocate the points array: enlarge by 4X, etc */
	    pointsArraySize *= 4;
	    dlPolyline->points = (XPoint *) 
	      realloc ((char*)dlPolyline->points,
		       (pointsArraySize+1)*sizeof(XPoint));
	  }
	  getToken(displayInfo,token);
	  dlPolyline->points[dlPolyline->nPoints].x = atoi(token);
	  getToken(displayInfo,token);	/* separator	*/
	  getToken(displayInfo,token);
	  dlPolyline->points[dlPolyline->nPoints].y = atoi(token);
	  getToken(displayInfo,token);	/*   ")"	*/
	  dlPolyline->nPoints++;
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

}


DlElement *parsePolyline(DisplayInfo *displayInfo)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;
  DlPolyline *dlPolyline;
  DlElement *dlElement = createDlPolyline(NULL);
  if (!dlElement) return 0;
  dlPolyline = dlElement->structure.polyline;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) 
      {
      case T_WORD:
	if (STREQL(token,"object"))
	  parseObject(displayInfo,&(dlPolyline->object));
	else if (STREQL(token,"basic attribute"))
	    parseBasicAttribute(displayInfo,&(dlPolyline->attr));
	else if (STREQL(token,"dynamic attribute"))
	  parseDynamicAttribute(displayInfo,&(dlPolyline->dynAttr));
        else if (STREQL(token,"points"))
          parsePolylinePoints(displayInfo,dlPolyline);
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
 * function to write all points of polyline out
 */
void writeDlPolylinePoints(
  FILE *stream,
  DlPolyline *dlPolyline, 
  int level)
{
  int i;
  char indent[256]; level=MIN(level,256-2);

  level = MIN(level, 256-2);
  memset(indent,'\t',level);
  indent[level] = '\0';

  fprintf(stream,"\n%spoints {",indent);

  for (i = 0; i < dlPolyline->nPoints; i++) {
    fprintf(stream,"\n%s\t(%d,%d)",indent,
        dlPolyline->points[i].x,dlPolyline->points[i].y);
  }

  fprintf(stream,"\n%s}",indent);
}


void writeDlPolyline(
  FILE *stream,
  DlElement *dlElement,
  int level)
{
  char indent[256]; level=MIN(level,256-2);
  DlPolyline *dlPolyline = dlElement->structure.polyline;

  memset(indent,'\t',level);
  indent[level] = '\0';

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
    fprintf(stream,"\n%spolyline {",indent);
    writeDlObject(stream,&(dlPolyline->object),level+1);
    writeDlBasicAttribute(stream,&(dlPolyline->attr),level+1);
    writeDlDynamicAttribute(stream,&(dlPolyline->dynAttr),DYNATTR_ALL,level+1);
    writeDlPolylinePoints(stream,dlPolyline,level+1);
    fprintf(stream,"\n%s}",indent);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    writeDlBasicAttribute(stream,&(dlPolyline->attr),level);
    writeDlDynamicAttribute(stream,&(dlPolyline->dynAttr),DYNATTR_ALL,level);

    fprintf(stream,"\n%spolyline {",indent);
    writeDlObject(stream,&(dlPolyline->object),level+1);
    writeDlPolylinePoints(stream,dlPolyline,level+1);
    fprintf(stream,"\n%s}",indent);
  }
#endif
}

/*
 * manipulate a polyline vertex
 */

static int handlePolylineVertexManipulation(
  DlElement *dlElement,
  int x0, int y0)
{
  XEvent event;
  Window window;
  int i, minX, maxX, minY, maxY;
  int x01, y01;
  DlPolyline *dlPolyline = dlElement->structure.polyline;
  int pointIndex = 0;

  int deltaX, deltaY, okIndex;
  double radians, okRadians, length;
  int foundVertex = False;

  window = XtWindow(currentDisplayInfo->drawingArea);
  minX = INT_MAX; maxX = INT_MIN; minY = INT_MAX; maxY = INT_MIN;

  for (i = 0; i < dlPolyline->nPoints; i++) {
    x01 = dlPolyline->points[i].x;
    y01 = dlPolyline->points[i].y;
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

	/* modify point and leave here 
	 */
	if (event.xbutton.state & ShiftMask) {
	  /* constrain to 45 degree increments */
	      deltaX = event.xmotion.x - dlPolyline->points[
		    (pointIndex > 0 ? pointIndex-1 : dlPolyline->nPoints-1)].x;
	      deltaY = event.xmotion.y - dlPolyline->points[
		    (pointIndex > 0 ? pointIndex-1 : dlPolyline->nPoints-1)].y;
	      length = sqrt(pow((double)deltaX,2.) + pow((double)deltaY,2.0));
	      radians = atan2((double)(deltaY),(double)(deltaX));
	/* use positive radians */
	      if (radians < 0.) radians = 2*M_PI + radians;
	      okIndex = (int)((radians*8.0)/M_PI);
	      okRadians = okRadiansTable[okIndex];
	      x01 = (int) (cos(okRadians)*length) + dlPolyline->points[
		    (pointIndex > 0 ? pointIndex-1 : dlPolyline->nPoints-1)].x;
	      y01 = (int) (sin(okRadians)*length) + dlPolyline->points[
		    (pointIndex > 0 ? pointIndex-1 : dlPolyline->nPoints-1)].y;
	      dlPolyline->points[pointIndex].x = x01;
	      dlPolyline->points[pointIndex].y = y01;
	    } else {
/* unconstrained */
	      dlPolyline->points[pointIndex].x = event.xbutton.x;
	      dlPolyline->points[pointIndex].y = event.xbutton.y;
	    }
	    XUngrabPointer(display,CurrentTime);
	    XUngrabServer(display);
	    for (i = 0; i < dlPolyline->nPoints; i++) {
	      minX = MIN(minX,dlPolyline->points[i].x);
	      maxX = MAX(maxX,dlPolyline->points[i].x);
	      minY = MIN(minY,dlPolyline->points[i].y);
	      maxY = MAX(maxY,dlPolyline->points[i].y);
	    }
	    dlPolyline->object.x = minX - globalResourceBundle.lineWidth/2;
	    dlPolyline->object.y = minY - globalResourceBundle.lineWidth/2;
	    dlPolyline->object.width = maxX - minX
					+ globalResourceBundle.lineWidth;
	    dlPolyline->object.height = maxY - minY
					+ globalResourceBundle.lineWidth;
	  /* update global resource bundle  - then do create out of it */
	    globalResourceBundle.x = dlPolyline->object.x;
	    globalResourceBundle.y = dlPolyline->object.y;
	    globalResourceBundle.width = dlPolyline->object.width;
	    globalResourceBundle.height = dlPolyline->object.height;

	    dmTraverseNonWidgetsInDisplayList(currentDisplayInfo);
/* since dmTraverseNonWidgets... clears the window, redraw highlights */
	    highlightSelectedElements();
	    return 1;

	case MotionNotify:
	/* undraw old line segments */
	  if (pointIndex > 0)
	      XDrawLine(display,window,xorGC,
		dlPolyline->points[pointIndex-1].x,
		dlPolyline->points[pointIndex-1].y, x01,y01);
	  if (pointIndex < dlPolyline->nPoints-1)
	      XDrawLine(display,window,xorGC,
		dlPolyline->points[pointIndex+1].x,
		dlPolyline->points[pointIndex+1].y, x01,y01);

	  if (event.xmotion.state & ShiftMask) {
/* constrain redraw to 45 degree increments */
	    deltaX = event.xmotion.x - dlPolyline->points[
		    (pointIndex > 0 ? pointIndex-1 : dlPolyline->nPoints-1)].x;
	    deltaY = event.xmotion.y - dlPolyline->points[
		    (pointIndex > 0 ? pointIndex-1 : dlPolyline->nPoints-1)].y;
	    length = sqrt(pow((double)deltaX,2.) + pow((double)deltaY,2.0));
	    radians = atan2((double)(deltaY),(double)(deltaX));
	/* use positive radians */
	    if (radians < 0.) radians = 2*M_PI + radians;
	    okIndex = (int)((radians*8.0)/M_PI);
	    okRadians = okRadiansTable[okIndex];
	    x01 = (int) (cos(okRadians)*length) + dlPolyline->points[
		    (pointIndex > 0 ? pointIndex-1 : dlPolyline->nPoints-1)].x;
	    y01 = (int) (sin(okRadians)*length) + dlPolyline->points[
		    (pointIndex > 0 ? pointIndex-1 : dlPolyline->nPoints-1)].y;
	  } else {
/* unconstrained */
	    x01 = event.xmotion.x;
	    y01 = event.xmotion.y;
	  }
	/* draw new line segments */
	  if (pointIndex > 0)
	      XDrawLine(display,window,xorGC,
		dlPolyline->points[pointIndex-1].x,
		dlPolyline->points[pointIndex-1].y, x01,y01);
	  if (pointIndex < dlPolyline->nPoints-1)
	      XDrawLine(display,window,xorGC,
		dlPolyline->points[pointIndex+1].x,
		dlPolyline->points[pointIndex+1].y, x01,y01);

	  break;

	default:
	  XtDispatchEvent(&event);
	  break;
     }
  }

}

/*
 * create a polyline - if Boolean simpleLine is True then want a simple
 *  (2 point) line, else create and add points to the polyline until
 *  the user enters a double click
 */

DlElement *handlePolylineCreate(
  int x0, int y0, Boolean simpleLine)
{
  XEvent event, newEvent;
  int newEventType;
  Window window;
  DlPolyline *dlPolyline;
  DlElement *element;
  int pointsArraySize = INITIAL_NUM_POINTS;
  int i, minX, maxX, minY, maxY;
  int x01, y01;

  int deltaX, deltaY, okIndex;
  double radians, okRadians, length;

  window = XtWindow(currentDisplayInfo->drawingArea);
  element = createDlPolyline(NULL);
  if (!element) return 0;
  dlPolyline = element->structure.polyline;
  polylineInheritValues(&globalResourceBundle,element);
  objectAttributeSet(&(dlPolyline->object),x0,y0,0,0);

  minX = INT_MAX; maxX = INT_MIN; minY = INT_MAX; maxY = INT_MIN;
  
/* first click is first point... */
  dlPolyline->nPoints = 1;
  if (simpleLine) {
    dlPolyline->points = (XPoint *)calloc(2,sizeof(XPoint));
  } else {
    dlPolyline->points = (XPoint *)calloc(pointsArraySize+1,sizeof(XPoint));
  }
  dlPolyline->points[0].x = (short)x0;
  dlPolyline->points[0].y = (short)y0;
  x01 = x0; y01 = y0;

  XGrabPointer(display,window,FALSE,
	(unsigned int) (PointerMotionMask|ButtonMotionMask|ButtonPressMask|ButtonReleaseMask),
	GrabModeAsync,GrabModeAsync,None,None,CurrentTime);
  XGrabServer(display);


/* now loop until button is double-clicked (or until 2 points if simpleLine) */
  while (TRUE) {

      XtAppNextEvent(appContext,&event);

      switch(event.type) {

	case ButtonPress:
	  if (!simpleLine) {
			/* need double click to end */
	    XWindowEvent(display,XtWindow(currentDisplayInfo->drawingArea),
			ButtonPressMask|Button1MotionMask|PointerMotionMask,&newEvent);
	    newEventType = newEvent.type;
	  } else {
			/* just need second point, set type so we can terminate wi/ 2 points */
	    newEventType = ButtonPress;
	  }
		if (newEventType == ButtonPress) {
  		/* -> double click... add last point and leave here */
	    if (event.xbutton.state & ShiftMask) {
				/* constrain to 45 degree increments */
	      deltaX = event.xbutton.x - dlPolyline->points[dlPolyline->nPoints-1].x;
	      deltaY = event.xbutton.y - dlPolyline->points[dlPolyline->nPoints-1].y;
	      length = sqrt(pow((double)deltaX,2.) + pow((double)deltaY,2.0));
	      radians = atan2((double)(deltaY),(double)(deltaX));
	/* use positive radians */
	      if (radians < 0.) radians = 2*M_PI + radians;
	      okIndex = (int)((radians*8.0)/M_PI);
	      okRadians = okRadiansTable[okIndex];
	      x01 = (int) (cos(okRadians)*length)
			 + dlPolyline->points[dlPolyline->nPoints-1].x;
	      y01 = (int) (sin(okRadians)*length)
			 + dlPolyline->points[dlPolyline->nPoints-1].y;
	      dlPolyline->nPoints++;
	      dlPolyline->points[dlPolyline->nPoints-1].x = x01;
	      dlPolyline->points[dlPolyline->nPoints-1].y = y01;
	    } else {
/* unconstrained */
	      dlPolyline->nPoints++;
	      dlPolyline->points[dlPolyline->nPoints-1].x = event.xbutton.x;
	      dlPolyline->points[dlPolyline->nPoints-1].y = event.xbutton.y;
	    }
	    XUngrabPointer(display,CurrentTime);
	    XUngrabServer(display);
	    for (i = 0; i < dlPolyline->nPoints; i++) {
	      minX = MIN(minX,dlPolyline->points[i].x);
	      maxX = MAX(maxX,dlPolyline->points[i].x);
	      minY = MIN(minY,dlPolyline->points[i].y);
	      maxY = MAX(maxY,dlPolyline->points[i].y);
	    }
	    dlPolyline->object.x = minX - dlPolyline->attr.width/2;
	    dlPolyline->object.y = minY - dlPolyline->attr.width/2;
	    dlPolyline->object.width = maxX - minX
					+ dlPolyline->attr.width;
	    dlPolyline->object.height = maxY - minY
					+ dlPolyline->attr.width;
	    XBell(display,50); XBell(display,50);
	    return (element);

	  } else {
			/* not last point: more points to add.. */
			/* undraw old last line segment */
	    XDrawLine(display,window,xorGC,
				dlPolyline->points[dlPolyline->nPoints-1].x,
				dlPolyline->points[dlPolyline->nPoints-1].y,
				x01, y01);
			/* new line segment added: update coordinates */
	    if (dlPolyline->nPoints >= pointsArraySize) {
	    	/* reallocate the points array: enlarge by 4X, etc */
				pointsArraySize *= 4;
				dlPolyline->points = (XPoint *)
				  realloc((char*)dlPolyline->points,
					  (pointsArraySize+1)*sizeof(XPoint));
	    }

	    if (event.xbutton.state & ShiftMask) {
				/* constrain to 45 degree increments */
	      deltaX = event.xbutton.x - 
		dlPolyline->points[dlPolyline->nPoints-1].x;

	      deltaY = event.xbutton.y - 
		dlPolyline->points[dlPolyline->nPoints-1].y;

	      length = sqrt(pow((double)deltaX,2.) + pow((double)deltaY,2.0));
	      radians = atan2((double)(deltaY),(double)(deltaX));
				/* use positive radians */
	      if (radians < 0.) radians = 2*M_PI + radians;
	      okIndex = (int)((radians*8.0)/M_PI);
	      okRadians = okRadiansTable[okIndex];
	      x01 = (int) (cos(okRadians)*length)
					+ dlPolyline->points[dlPolyline->nPoints-1].x;
	      y01 = (int) (sin(okRadians)*length)
					+ dlPolyline->points[dlPolyline->nPoints-1].y;
	      dlPolyline->nPoints++;
	      dlPolyline->points[dlPolyline->nPoints-1].x = x01;
	      dlPolyline->points[dlPolyline->nPoints-1].y = y01;
	    } else {
				/* unconstrained */
	      dlPolyline->nPoints++;
	      dlPolyline->points[dlPolyline->nPoints-1].x = event.xbutton.x;
	      dlPolyline->points[dlPolyline->nPoints-1].y = event.xbutton.y;
	      x01 = event.xbutton.x; y01 = event.xbutton.y;
	    }
			/* draw new line segment */
	    XDrawLine(display,window,xorGC,
				dlPolyline->points[dlPolyline->nPoints-2].x,
				dlPolyline->points[dlPolyline->nPoints-2].y,
				dlPolyline->points[dlPolyline->nPoints-1].x,
				dlPolyline->points[dlPolyline->nPoints-1].y);
	  }
	  break;

	case MotionNotify:
		/* undraw old last line segment */
	  XDrawLine(display,window,xorGC,
			dlPolyline->points[dlPolyline->nPoints-1].x,
			dlPolyline->points[dlPolyline->nPoints-1].y,
			x01, y01);

	  if (event.xmotion.state & ShiftMask) {
			/* constrain redraw to 45 degree increments */
	    deltaX = event.xmotion.x -
			dlPolyline->points[dlPolyline->nPoints-1].x;
	    deltaY = event.xmotion.y -
			dlPolyline->points[dlPolyline->nPoints-1].y;
	    length = sqrt(pow((double)deltaX,2.) + pow((double)deltaY,2.0));
	    radians = atan2((double)(deltaY),(double)(deltaX));
			/* use positive radians */
	    if (radians < 0.) radians = 2*M_PI + radians;
	    okIndex = (int)((radians*8.0)/M_PI);
	    okRadians = okRadiansTable[okIndex];
	    x01 = (int) (cos(okRadians)*length)
			 + dlPolyline->points[dlPolyline->nPoints-1].x;
	    y01 = (int) (sin(okRadians)*length)
			 + dlPolyline->points[dlPolyline->nPoints-1].y;
	  } else {
			/* unconstrained */
	    x01 = event.xmotion.x;
	    y01 = event.xmotion.y;
	  }
		/* draw new last line segment */
	  XDrawLine(display,window,xorGC,
			dlPolyline->points[dlPolyline->nPoints-1].x,
			dlPolyline->points[dlPolyline->nPoints-1].y,
			x01,y01);
	  break;

	default:
	  XtDispatchEvent(&event);
	  break;
     }
  }

}

static void polylineMove(DlElement *dlElement, int xOffset, int yOffset) {
  int i;
  XPoint *pts;
  if (dlElement->type != DL_Polyline) return;
  dlElement->structure.polyline->object.x += xOffset;
  dlElement->structure.polyline->object.y += yOffset;
  pts = dlElement->structure.polyline->points;
  for (i = 0; i < dlElement->structure.polyline->nPoints; i++) {
    pts[i].x += xOffset;
    pts[i].y += yOffset;
  }
}

static void polylineScale(DlElement *dlElement, int xOffset, int yOffset) 
{
  float sX, sY;
  int i;
  register DlPolyline *dlPolyline;
  int width, height;

  if (dlElement->type != DL_Polyline) return;
  dlPolyline = dlElement->structure.polyline;

  width = MAX(1,((int)dlPolyline->object.width + xOffset));
  height = MAX(1,((int)dlPolyline->object.height + yOffset));
  sX = (float)width/(float)(MAX(1,dlPolyline->object.width));
  sY = (float)height/(float)(MAX(1,dlPolyline->object.height));
  
  for (i = 0; i < dlPolyline->nPoints; i++) 
    {
      dlPolyline->points[i].x = (dlPolyline->object.x +
		    sX * (dlPolyline->points[i].x - dlPolyline->object.x));
      dlPolyline->points[i].y = (dlPolyline->object.y +
	            sY * (dlPolyline->points[i].y - dlPolyline->object.y));
    }

  dlPolyline->object.width = width;
  dlPolyline->object.height = height;
}

static void polylineGetValues(ResourceBundle *pRCB, DlElement *p) 
{
  DlPolyline * dlPolyline = p->structure.polyline;
  int          x, y;
  unsigned int width, height;
  int          xOffset, yOffset;
#ifdef _MUST_BE_IMPROVED_
  int          lineWidth;
#endif

  dm2kGetValues(pRCB,
    X_RC,          &x,
    Y_RC,          &y,
    WIDTH_RC,      &width,
    HEIGHT_RC,     &height,
    CLR_RC,        &(dlPolyline->attr.clr),
    STYLE_RC,      &(dlPolyline->attr.style),
    FILL_RC,       &(dlPolyline->attr.fill),
#ifdef _MUST_BE_IMPROVED_
    LINEWIDTH_RC,  &lineWidth,
#else
    LINEWIDTH_RC,  &(dlPolyline->attr.width),
#endif
    CLRMOD_RC,     &(dlPolyline->dynAttr.clr),
    VIS_RC,        &(dlPolyline->dynAttr.vis),
    COLOR_RULE_RC, &(dlPolyline->dynAttr.colorRule),
    CHAN_RC,       &(dlPolyline->dynAttr.chan),
    -1);

#ifdef _MUST_BE_IMPROVED_
  if (lineWidth != dlPolyline->attr.width) {
    if (dlPolyline->nPoints == 2) {
    }
  }
#endif

  xOffset = (int) width - (int) dlPolyline->object.width;
  yOffset = (int) height - (int) dlPolyline->object.height;

  if (xOffset || yOffset) 
    polylineScale(p,xOffset,yOffset);

  xOffset = x - dlPolyline->object.x;
  yOffset = y - dlPolyline->object.y;
  
  if (xOffset || yOffset)
    polylineMove(p,xOffset,yOffset);
}

static void polylineInheritValues(ResourceBundle *pRCB, DlElement *p) 
{
  register DlPolyline *dlPolyline = p->structure.polyline;
  dm2kGetValues(pRCB,
    CLR_RC,        &(dlPolyline->attr.clr),
    STYLE_RC,      &(dlPolyline->attr.style),
    FILL_RC,       &(dlPolyline->attr.fill),
    LINEWIDTH_RC,  &(dlPolyline->attr.width),
    CLRMOD_RC,     &(dlPolyline->dynAttr.clr),
    VIS_RC,        &(dlPolyline->dynAttr.vis),
    COLOR_RULE_RC, &(dlPolyline->dynAttr.colorRule),
    CHAN_RC,       &(dlPolyline->dynAttr.chan),
    -1);
}

