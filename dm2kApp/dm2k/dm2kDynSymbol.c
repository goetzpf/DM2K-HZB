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
a period of five years from Mtexth 30, 1993, the Government is
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
#include <ctype.h>
#include "dm2k.h"
#include "dm2kDelay.h"

#define DELAY_DYNSYMBOLDRAW_MS 100

#define INITIAL_INDEX   (-1)
#define UNDEFINED_INDEX (-2)
#define IS_UNDRAWABLE_INDEX(i) (i < 0)

typedef struct _UseDisplayList {
  struct _DlList * original;
  struct _DlList * work;
} UseDisplayList;

typedef struct _DynSymbol {
  DlElement        * dlElement;
  Record           * record;
  UpdateTask       * updateTask;
  int                active;
  UseDisplayList   * pairList;
  UpdateTask      ** updateTasks;
  int                updateTasksNum;
  Pixel              lastBackground;
} DynSymbol;

static void dynSymbolUpdateValueCb(XtPointer);
static void dynSymbolDraw(XtPointer);
static void dynSymbolDrawSched(XtPointer);
static void dynSymbolDestroyCb(XtPointer);
static void dynSymbolName(XtPointer, char **, short *, int *);
static void dynSymbolGetValues(ResourceBundle *, DlElement *);
static void dynSymbolInheritValues(ResourceBundle *, DlElement *);
static void dynSymbolGetValues(ResourceBundle *, DlElement *);

static void destroyDlElement(DlElement * );
static UpdateTask * executeMethod(DisplayInfo *, DlElement *);
static void writeDlDynSymbol(FILE *, DlElement *, int);

static DlDispatchTable dynSymbolDlDispatchTable = {
         createDlDynSymbol,
         destroyDlElement,
         executeMethod,
         writeDlDynSymbol,
         NULL,
         dynSymbolGetValues,
         dynSymbolInheritValues,
         NULL,
         NULL,
         genericMove,
         genericScale,
         NULL
};


static void drawDynSymbol(DisplayInfo * displayInfo,
			  DlDynSymbol * dlDynSymbol) 
{
  GC           gc = displayInfo->pixmapGC;
  int          x = dlDynSymbol->object.x;
  int          y = dlDynSymbol->object.y;
  unsigned int width = dlDynSymbol->object.width;
  unsigned int height = dlDynSymbol->object.height;
  int          xShift;
  int          yShift;
  int          i;
  Pixel        background;
  

  if (width == 0 || height == 0)
    return;

  background = displayInfo->colormap[dlDynSymbol->bclr];

  /* external border
   */
  XSetForeground(display, gc, background);
  XFillRectangle(display, XtWindow(displayInfo->drawingArea), gc, 
		 x, y, width+1, height+1);
  XFillRectangle(display, displayInfo->drawingAreaPixmap, gc, 
		 x, y, width+1, height+1);

  XSetForeground(display, gc, BlackPixel(display,DefaultScreen(display)));
  XDrawRectangle(display, XtWindow(displayInfo->drawingArea), gc, 
		 x, y, width, height);
  XDrawRectangle(display, displayInfo->drawingAreaPixmap, gc, 
		 x, y, width, height);


  /* chained rectangles
   */
  width  = (dlDynSymbol->object.width * 3) / 8;
  height = (dlDynSymbol->object.height * 3) / 8;

  if (width == 0 || height == 0)
    return;

  x      = dlDynSymbol->object.x + dlDynSymbol->object.width;
  x     -= (dlDynSymbol->object.width * 7) / (16);

  y      = dlDynSymbol->object.y;
  y     += (dlDynSymbol->object.height) / (16);

  xShift = (int)((dlDynSymbol->object.width * 2) / 8);
  yShift = (int)((dlDynSymbol->object.height* 2) / 8);
  

  XSetLineAttributes(display, gc, 0, LineSolid, CapButt, JoinMiter);

  for (i = 0; i < 3; i++) {
    XSetForeground(display, gc, background);
    XFillRectangle(display, XtWindow(displayInfo->drawingArea), gc, 
		   x, y, width+1, height+1);
    XFillRectangle(display, displayInfo->drawingAreaPixmap, gc, 
		   x, y, width+1, height+1);

    XSetForeground(display, gc, BlackPixel(display,DefaultScreen(display)));
    XDrawRectangle(display, XtWindow(displayInfo->drawingArea), gc, 
		   x, y, width, height);
    XDrawRectangle(display, displayInfo->drawingAreaPixmap, gc, 
		   x, y, width, height);

    x -= xShift;
    y += yShift;
  }

  /* right line
   */
  {
    int x1, y1, x2, y2;

    x1 = dlDynSymbol->object.x + dlDynSymbol->object.width * 2 / 3;
    y1 = dlDynSymbol->object.y + dlDynSymbol->object.height;
    x2 = dlDynSymbol->object.x + dlDynSymbol->object.width;
    y2 = dlDynSymbol->object.y + dlDynSymbol->object.height * 2 / 3;

    XDrawLine(display, XtWindow(displayInfo->drawingArea), gc,
	      x1, y1, x2, y2);
    XDrawLine(display, displayInfo->drawingAreaPixmap, gc,
	      x1, y1, x2, y2);
  }
  
  /* question mark
   */
  
  x      = dlDynSymbol->object.x + dlDynSymbol->object.width * 5 / 6;
  y      = dlDynSymbol->object.y + dlDynSymbol->object.height * 5 / 6;
  width  = dlDynSymbol->object.width / 6;
  height = dlDynSymbol->object.height / 6;

  if (width < 12 || height < 12)
    return;

  draw3DQuestionMark2(display, gc, 
		      displayInfo->drawingAreaPixmap,
		      displayInfo->drawingArea,
		      x, y, (int)width, (int)height,
		      background);
}


static UpdateTask * executeMethod
   (DisplayInfo * displayInfo,
    DlElement   * dlElement)
{
  DlDynSymbol * dlDynSymbol = dlElement->structure.dynSymbol;
  UpdateTask  * updateTask = NULL;

  if ((displayInfo->traversalMode == DL_EXECUTE) 
      && (dlDynSymbol->dynAttr.chan != NULL))
  {
    GraphicRuleEntry * entry;
    DynSymbol        * pt = DM2KALLOC(DynSymbol);

    if (pt == NULL) {
      dm2kPrintf("UpdateTask: memory allocation error\n");
      return updateTask;
    }

    dlDynSymbol->opaque = (char*)pt;
    pt->dlElement  = dlElement;

    updateTask = pt->updateTask = updateTaskAddTask(displayInfo,
						    &(dlDynSymbol->object),
						    /*NULL,*/
						    dynSymbolDrawSched,
						    (XtPointer)pt);

    if (pt->updateTask == NULL) {
      dm2kPrintf("dynSymbolCreateRunTimeInstance : memory allocation error\n");
    } else {
      updateTaskAddDestroyCb(pt->updateTask,dynSymbolDestroyCb);
      updateTaskAddNameCb(pt->updateTask,dynSymbolName);
      pt->updateTask->opaque = False;
    }

    pt->record = dm2kAllocateRecord(dlDynSymbol->dynAttr.chan,
				    dynSymbolUpdateValueCb,
				    NULL,
				    (XtPointer) pt);

    pt->active         = INITIAL_INDEX;
    pt->updateTasks    = NULL;
    pt->updateTasksNum = 0;
    pt->lastBackground = dlDynSymbol->bclr;

    if (dlDynSymbol->graphicRule && dlDynSymbol->graphicRule->count > 0) 
    {
      int i;

      pt->pairList = (UseDisplayList *) 
	calloc (dlDynSymbol->graphicRule->count, sizeof(UseDisplayList));

      dlDynSymbol->graphicRule->refCount++;

      for (i = 0; i < dlDynSymbol->graphicRule->count; i++)
      {
	DlElement * el;
	
	entry = &dlDynSymbol->graphicRule->entries[i];
	
	if (entry == NULL 
	    || entry->data == NULL 
	    || entry->data->dlElementList == NULL)
	  {
	    pt->pairList[i].original = NULL;
	    pt->pairList[i].work     = NULL;
	    continue;
	  }

	pt->pairList[i].original = entry->data->dlElementList;
	pt->pairList[i].work = createDlList();

	el = FirstDlElement(pt->pairList[i].original);

	while (el) {
	  if (el->type != DL_Display) {
	    DlElement *pE;

	    pE = (*el->run->create)(el);

	    if (pE) 
	      appendDlElement(pt->pairList[i].work, pE);
	  }
	  el = el->next;
	}

	if (dlDynSymbol->fit == FIT_YES)
	  resizeDlElementList
	    (pt->pairList[i].work,
	     entry->data->x,
	     entry->data->y,
	     (float)((float)dlDynSymbol->object.width / (float)entry->data->width),
	     (float)((float)dlDynSymbol->object.height / (float)entry->data->height));
	
	shiftDlElementList(pt->pairList[i].work,
			   dlDynSymbol->object.x - entry->data->x,
			   dlDynSymbol->object.y - entry->data->y);
      }
    } 
    else
      pt->pairList = NULL;

    drawWhiteRectangle(pt->updateTask);
    

    if (dlDynSymbol->dynAttr.vis == V_STATIC ) {
      pt->record->monitorZeroAndNoneZeroTransition = False;
    }
  } 
  else {
    drawDynSymbol(displayInfo, dlDynSymbol);
  }

  return updateTask;
}

static void dynSymbolUpdateValueCb(XtPointer cd) 
{
  DynSymbol * pt = (DynSymbol *) ((Record *) cd)->clientData;
  updateTaskMarkUpdate(pt->updateTask);
}

static void destroyWidgetsInElementChain (DlElement * element)
{
  if (element == NULL || element->type == DL_DynSymbol)
    return;

  while (element) {
    switch (element->type)
      {
      case DL_Composite: {
	DlComposite * dlComposite = element->structure.composite;
	DlElement   * it = FirstDlElement(dlComposite->dlElementList);

	while (it) {
	  destroyWidgetsInElementChain(it);
	  it = it->next;
	}
      }
      break;

      case DL_DynSymbol : {
	DlDynSymbol * dlDynSymbol = element->structure.dynSymbol;
	DynSymbol   * pt;
	DlElement   * el;
	
	pt = (DynSymbol *)dlDynSymbol->opaque;
	el = FirstDlElement(pt->pairList[pt->active].work);
	
	destroyWidgetsInElementChain(el);
      }
      break;

      
      case DL_Bar:
      case DL_RelatedDisplay:
	if (element->widget != NULL) {
	  XtDestroyWidget (element->widget);
	  element->widget = NULL;
	}

	break;

      default:
	if (element->widget != NULL) {
	  XtDestroyWidget (element->widget);
	  element->widget = NULL;
	}

	break;
      }
    
    element = element->next;
  }
}

static void drawGraphicEntry(DisplayInfo      * displayInfo, 
			     struct _DlList   * list,  
			     DynSymbol        * pt,
			     DlTraversalMode    mode)
{
  DlElement       * el;
  DlTraversalMode   copyGlobal = globalDisplayListTraversalMode;
  DlTraversalMode   copyLocal = displayInfo->traversalMode;

  if (list == NULL)
    return;

  globalDisplayListTraversalMode = mode;
  displayInfo->traversalMode = mode;

  
  if (mode == DL_EXECUTE) {
    DM2KFREE(pt->updateTasks);
    pt->updateTasks = NULL;
    pt->updateTasksNum = 0;
  }
  else if (mode == DL_EDIT) {
    int i;
    
    for (i = 0; i < pt->updateTasksNum; i++)
      updateTaskDeleteTask(pt->updateTasks[i]);
  }


  el = FirstDlElement(list);
  
  while (el) {
    if (el->type != DL_Display) {
      if (el->run->execute) {
	UpdateTask * ut;
	extern UpdateTask ** globalUpdateTasks;
	extern int           globalUpdateTasksNum;

	DM2KFREE(globalUpdateTasks);
	globalUpdateTasks = NULL;
	globalUpdateTasksNum = 0;

	ut = (*el->run->execute)(displayInfo, el);

	if (mode == DL_EXECUTE && 
	    (ut != NULL || globalUpdateTasks != NULL)) {
	  
	  if (ut != NULL)
	    pt->updateTasksNum++;
	  else
	    pt->updateTasksNum += globalUpdateTasksNum;

	  REALLOC(UpdateTask *, pt->updateTasks, pt->updateTasksNum);
	  
	  if (pt->updateTasks != NULL) {
	    if (ut != NULL)
	      pt->updateTasks[pt->updateTasksNum-1] = ut;
	    else {
	      memcpy(&pt->updateTasks[pt->updateTasksNum-globalUpdateTasksNum],
		     globalUpdateTasks, 
		     globalUpdateTasksNum * sizeof(UpdateTask *));

	      DM2KFREE(globalUpdateTasks);
	      globalUpdateTasks = NULL;
	      globalUpdateTasksNum = 0;
	    }
	    
	  }
	}
      }
    }
    el = el->next;
  }

  if (mode == DL_EDIT) 
    destroyWidgetsInElementChain (FirstDlElement(list));

  globalDisplayListTraversalMode = copyGlobal;
  displayInfo->traversalMode = copyLocal;
}


static int extractGraphicRuleEntry(
  DisplayInfo          * displayInfo, 
  double                 value, 
  register GraphicRule * graphicRule)
{
  int i;

  if (graphicRule == NULL)
    return UNDEFINED_INDEX;

  for (i = 0; i < graphicRule->count; i++) {
    if (value <= graphicRule->entries[i].upperBoundary && 
	value >= graphicRule->entries[i].lowerBoundary) 
      {
	return i;
      }
  }

  return UNDEFINED_INDEX;
}

#if 0
static GraphicRuleEntry * extractGraphicRuleEntry2(
  DisplayInfo          * displayInfo, 
  double                 value, 
  register GraphicRule * graphicRule)
{
  int i;

  if (graphicRule == NULL)
    return NULL;

  for (i = 0; i < graphicRule->count; i++) {
    if (value <= graphicRule->entries[i].upperBoundary && 
	value >= graphicRule->entries[i].lowerBoundary) 
      {
	return &(graphicRule->entries[i]);
      }
  }

  return NULL;
}
#endif

static void dynSymbolDraw(XtPointer cd) 
{
  DynSymbol   * pt = (DynSymbol *) cd;
  Record      * pd = pt->record;
  DisplayInfo * displayInfo = pt->updateTask->displayInfo;
  DlDynSymbol * dlDynSymbol = pt->dlElement->structure.dynSymbol;
  int           graphicEntry;
  Pixel         background;

  /* if channel is not connected, just draw white rectangle;
   */
  if (!pd->connected) {
    drawWhiteRectangle(pt->updateTask);
    return;
  }

  /* channel is connected;
   */
  graphicEntry = extractGraphicRuleEntry(displayInfo,
					 pd->value,
					 dlDynSymbol->graphicRule);
  
  switch (dlDynSymbol->dynAttr.clr) 
    {
    case DISCRETE:
      if (pd) {
	background = extractColor(displayInfo,
				  pd->value,
				  dlDynSymbol->dynAttr.colorRule,
				  dlDynSymbol->bclr);
	break;
      }
    case STATIC :
      background = displayInfo->colormap[dlDynSymbol->bclr];
      break;
    case ALARM :
      background = alarmColorPixel[pd->severity];
      break;
    default :
      break;
    }

  /* check whether we have to change graphic face or not;
   */
  if (background == pt->lastBackground && pt->active == graphicEntry) {
    return;
  }

  /* if there is not graphic rule entry for such value
   * let's draw logo of DynElement;
   */
  if (IS_UNDRAWABLE_INDEX(graphicEntry)) {
    drawWhiteRectangle(pt->updateTask);
    pt->active = graphicEntry;
    return;
  }

  /* clear old face and draw new graphic face;
   */
  if ( !IS_UNDRAWABLE_INDEX(pt->active) )
     drawGraphicEntry(displayInfo, pt->pairList[pt->active].work, pt,
		      DL_EDIT);

  XSetForeground(display, displayInfo->pixmapGC, background);

  XFillRectangle(display, XtWindow(displayInfo->drawingArea),
		 displayInfo->pixmapGC,
		 dlDynSymbol->object.x,dlDynSymbol->object.y,
		 dlDynSymbol->object.width,dlDynSymbol->object.height); 
  
  XFillRectangle(display,displayInfo->drawingAreaPixmap,
		 displayInfo->pixmapGC,
		 dlDynSymbol->object.x,dlDynSymbol->object.y,
		 dlDynSymbol->object.width,dlDynSymbol->object.height); 

  drawGraphicEntry(displayInfo, pt->pairList[graphicEntry].work, pt,
		   DL_EXECUTE);

  /* keep active graphic entry updated */
  pt->lastBackground = background;
  pt->active = graphicEntry;
}

static void dynSymbolDrawSched ( XtPointer cb )
{
   delayExec( DELAY_DYNSYMBOLDRAW_MS, cb, dynSymbolDraw, cb );
}

static void dynSymbolName(XtPointer    cd, 
			  char      ** name, 
			  short      * severity, 
			  int        * count) 
{
  DynSymbol *pt = (DynSymbol *) cd;

  *count = 1;
  name[0]     = pt->record->name;
  severity[0] = pt->record->severity;
}

void destroyGraphicRule(GraphicRule * rule)
{
  /*
    destroyDlDisplayList(dlElement->structure.composite->dlElementList);
    TODO VTR */
}

static void destroyDlDynSymbol(DlDynSymbol * dlDynSymbol)
{
  if (dlDynSymbol == NULL)
    return;

  objectAttributeDestroy(&(dlDynSymbol->object));
  dynamicAttributeDestroy(&(dlDynSymbol->dynAttr));

  free((char *)dlDynSymbol);
}


static void destroyDlElement(DlElement * p)
{
  if (p->type != DL_DynSymbol)
    return;

  destroyDlDynSymbol(p->structure.dynSymbol);

  free((char *)p);
}

static void dynSymbolDestroyCb(XtPointer cd) 
{
  DynSymbol * pt = (DynSymbol *) cd;

  if (pt != NULL) {
    int i;
    DlDynSymbol * dlDynSymbol = pt->dlElement->structure.dynSymbol;

    cancelDelay( cd );

    if (dlDynSymbol->graphicRule) {
     dlDynSymbol->graphicRule->refCount--;

     for (i = 0; i < dlDynSymbol->graphicRule->count; i++) {
	destroyDlDisplayList(pt->pairList[i].work);
	DM2KFREE(pt->pairList[i].work);
     }
     DM2KFREE(pt->pairList);
    }

    if ( pt->record ) dm2kDestroyRecord(pt->record); /*Olx changed*/

    for (i = 0; i < pt->updateTasksNum; i++) {
      updateTaskDeleteTask(pt->updateTasks[i]);
    }

    DM2KFREE(pt->updateTasks);
    free((char *)pt);
  }
}

DlElement *createDlDynSymbol(DlElement * p)
{
  DlDynSymbol * dlDynSymbol;
  DlElement   * dlElement;
 
  dlDynSymbol = DM2KALLOC(DlDynSymbol);
  if (dlDynSymbol == NULL) 
    return NULL;

  if (p != NULL) { 
    objectAttributeCopy(&(dlDynSymbol->object), 
			&(p->structure.dynSymbol->object));
    dynamicAttributeCopy(&(dlDynSymbol->dynAttr), 
			 &(p->structure.dynSymbol->dynAttr));
    
    dlDynSymbol->fit         = p->structure.dynSymbol->fit;
    dlDynSymbol->graphicRule = p->structure.dynSymbol->graphicRule;
    dlDynSymbol->bclr        = p->structure.dynSymbol->bclr;
  } 
  else {
    objectAttributeInit(&(dlDynSymbol->object));
    dynamicAttributeInit(&(dlDynSymbol->dynAttr));

    dlDynSymbol->fit         = FIT_NO;
    dlDynSymbol->graphicRule = NULL;
    dlDynSymbol->bclr        = globalResourceBundle.bclr;
  }
 
  dlElement = createDlElement(DL_DynSymbol, (XtPointer) dlDynSymbol,
			      &dynSymbolDlDispatchTable);

  if (dlElement == NULL)
    destroyDlDynSymbol(dlDynSymbol);

  return(dlElement);
}

DlElement * parseDynSymbol(DisplayInfo *displayInfo)
{
  char          token[MAX_TOKEN_LENGTH];
  TOKEN         tokenType;
  int           nestingLevel = 0;
  DlDynSymbol * dlDynSymbol;
  DlElement   * dlElement = createDlDynSymbol(NULL);

  if (dlElement == NULL) 
    return NULL;

  dlDynSymbol = dlElement->structure.dynSymbol;

  do {
    switch( (tokenType = getToken(displayInfo,token)) ) 
    {
    case T_WORD:
      if (STREQL(token,"object")) 
      {
	parseObject(displayInfo,&(dlDynSymbol->object));
      } 
      else if (STREQL(token,"dynamic attribute")) 
      {
	parseDynamicAttribute(displayInfo,&(dlDynSymbol->dynAttr));  
      }
      else if (STREQL(token,"fit")) 
      {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	dlDynSymbol->fit = getBooleanByName(token) ? FIT_YES : FIT_NO;
      }         
      else if (STREQL(token,"colorRule")) 
      {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	dlDynSymbol->dynAttr.colorRule = getColorRuleByName(token);
      }
      else if (STREQL(token,"graphicRule")) 
      {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	dlDynSymbol->graphicRule = getGraphicRuleByName(token);
      }
      else if (STREQL(token,"bclr")) 
      {
        getToken(displayInfo,token);
        getToken(displayInfo,token);
        dlDynSymbol->bclr = atoi(token) % DL_MAX_COLORS;
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

static void writeDlDynSymbol (
  FILE      * stream,
  DlElement * dlElement,
  int         level)
{
  char          indent[265];
  DlDynSymbol * dlDynSymbol = dlElement->structure.dynSymbol;

  level = MIN(level, 265-2);
  memset(indent,'\t',level);
  indent[level] = '\0'; 

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
    fprintf(stream,"\n%s\"dynamic symbol\" {",indent);
    writeDlObject(stream,&(dlDynSymbol->object),level+1);
    writeDlDynamicAttribute(stream,&(dlDynSymbol->dynAttr),
			    DYNATTR_CHANNEL|DYNATTR_COLORRULE|DYNATTR_COLORMODE,level+1); 

    fprintf(stream,"\n%s\tfit=\"%s\"",indent,
	    dlDynSymbol->fit == FIT_YES ? "yes" : "no" );

    if (dlDynSymbol->graphicRule != NULL) {
      fprintf(stream,"\n%s\tgraphicRule=\"%s\"",
	      indent,
	      dlDynSymbol->graphicRule->name);
    }

    fprintf(stream,"\n%s\tbclr=%d",indent,dlDynSymbol->bclr);

    fprintf(stream,"\n%s}",indent);
    
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } 
  else {
    fprintf(stream,"\n%sdynSymbol {",indent);
    writeDlObject(stream,&(dlDynSymbol->object),level+1);
    writeDlDynamicAttribute(stream,&(dlDynSymbol->dynAttr),
			    DYNATTR_CHANNEL|DYNATTR_COLORRULE,level+1); 

    fprintf(stream,"\n%s\tfit=\"%s\"",indent,
	    dlDynSymbol->fit? "yes" : "no" );

    if (dlDynSymbol->graphicRule != NULL) {
      fprintf(stream,"\n%s\tgraphicRule=\"%s\"",
	      indent,
	      dlDynSymbol->graphicRule->name);
    }

    fprintf(stream,"\n%s\tbclr=%d",indent,dlDynSymbol->bclr);

    fprintf(stream,"\n%s}",indent);
  }
#endif
}

#if 0

/* some timer (cursor blinking) related functions and globals */
#define BLINK_INTERVAL 700
#define CURSOR_WIDTH 10
 
XtIntervalId intervalId;
int cursorX, cursorY;

#ifdef __cplusplus
static void blinkCursor(
  XtPointer,
  XtIntervalId *)
#else
static void blinkCursor(
  XtPointer client_data,
  XtIntervalId *id)
#endif
{
  static Boolean state = FALSE;
 
  if (state == TRUE) {
    XDrawLine(display,XtWindow(currentDisplayInfo->drawingArea),
	      currentDisplayInfo->gc,
	      cursorX, cursorY, cursorX + CURSOR_WIDTH, cursorY);
    state = FALSE;
  } else {
    XCopyArea(display,currentDisplayInfo->drawingAreaPixmap,
	      XtWindow(currentDisplayInfo->drawingArea),
	      currentDisplayInfo->pixmapGC,
	      (int)cursorX, (int)cursorY,
	      (unsigned int) CURSOR_WIDTH + 1,
	      (unsigned int) 1,
	      (int)cursorX, (int)cursorY);
    state = TRUE;
  }
  intervalId =
    XtAppAddTimeOut(appCondynSymbol,BLINK_INTERVAL,blinkCursor,NULL);
}
 
#endif
 
static void dynSymbolGetValues(ResourceBundle *pRCB, DlElement *p) 
{
  DlDynSymbol * dlDynSymbol = p->structure.dynSymbol;

  dm2kGetValues(pRCB,
		X_RC,                  &(dlDynSymbol->object.x),
		Y_RC,                  &(dlDynSymbol->object.y),
		WIDTH_RC,              &(dlDynSymbol->object.width),
		HEIGHT_RC,             &(dlDynSymbol->object.height),
		VIS_RC,                &(dlDynSymbol->dynAttr.vis),
		COLOR_RULE_RC,         &(dlDynSymbol->dynAttr.colorRule),
		RDBK_RC,               &(dlDynSymbol->dynAttr.chan),
		FIT_RC,                &(dlDynSymbol->fit),
		GRAPHIC_RULE_RC,       &(dlDynSymbol->graphicRule),
		BCLR_RC,               &(dlDynSymbol->bclr),
		-1);
}

static void dynSymbolInheritValues(ResourceBundle *pRCB, DlElement *p) 
{
  DlDynSymbol * dlDynSymbol = p->structure.dynSymbol;

  dm2kGetValues(pRCB,
		X_RC,                  &(dlDynSymbol->object.x),
		Y_RC,                  &(dlDynSymbol->object.y),
		WIDTH_RC,              &(dlDynSymbol->object.width),
		HEIGHT_RC,             &(dlDynSymbol->object.height),
		COLOR_RULE_RC,         &(dlDynSymbol->dynAttr.colorRule),
		RDBK_RC,               &(dlDynSymbol->dynAttr.chan),
		FIT_RC,                &(dlDynSymbol->fit),
		GRAPHIC_RULE_RC,       &(dlDynSymbol->graphicRule),
		BCLR_RC,               &(dlDynSymbol->bclr),
		-1);
}
