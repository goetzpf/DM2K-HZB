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
 *                              - new update screen dispatch mechanism
 * .03  09-13-95        vong    conform to c++ syntax
 *
 *****************************************************************************
*/

#include "dm2k.h"
#include <sys/time.h>

#define IF_NOT_CROSS(x0,y0,x1,y1,xx0,yy0,xx1,yy1) \
 ((x0) > (xx1) || (xx0) > (x1) || (y0) > (yy1) || (yy0) > (y1) \
 || (x0)>=(x1) || (y0)>=(y1) || (xx0)>=(xx1) || (yy0)>=(yy1))

typedef struct _UpdateTaskStatus {
  XtWorkProcId workProcId;
  UpdateTask  *nextToServe;
  int          taskCount;
  int          periodicTaskCount;
  int          updateRequestCount;
  int          updateDiscardCount;
  int          periodicUpdateRequestCount;
  int          periodicUpdateDiscardCount;
  int          updateRequestQueued;          /* this one won't reset */
  int          updateExecuted;
  double       since;
} UpdateTaskStatus;

typedef struct {
  XtIntervalId id;
  double      systemTime;
  double      tenthSecond;
} PeriodicTask;

static UpdateTaskStatus updateTaskStatus;
static PeriodicTask periodicTask;
static Boolean moduleInitialized = False;
static UpdateTask * nextUpdateTaskToDestroy = NULL;

static void dm2kScheduler(XtPointer, XtIntervalId *);
static Boolean updateTaskWorkProc(XtPointer);
static int updateTaskMarkTimeout(UpdateTask*, double);

Boolean dm2kInitSharedDotC() {
  if (moduleInitialized) return True;
  /* initialize the update task */
  updateTaskStatus.workProcId          = 0;
  updateTaskStatus.nextToServe         = NULL;
  updateTaskStatus.taskCount           = 0;
  updateTaskStatus.periodicTaskCount   = 0;
  updateTaskStatus.updateRequestCount  = 0;
  updateTaskStatus.updateDiscardCount  = 0;
  updateTaskStatus.periodicUpdateRequestCount = 0;
  updateTaskStatus.periodicUpdateDiscardCount = 0;
  updateTaskStatus.updateRequestQueued = 0;
  updateTaskStatus.updateExecuted      = 0;
  updateTaskStatus.since = dm2kTime();

  /* initialize the periodic task */
  periodicTask.systemTime = dm2kTime();
  periodicTask.tenthSecond = 0.0; 
  dm2kScheduler((XtPointer) &periodicTask, NULL);
  moduleInitialized = True;
  return True;
}

void updateTaskStatusGetInfo(int *taskCount,
                             int *periodicTaskCount,
                             int *updateRequestCount,
                             int *updateDiscardCount,
                             int *periodicUpdateRequestCount,
                             int *periodicUpdateDiscardCount,
                             int *updateRequestQueued,
                             int *updateExecuted,
                             double *timeInterval) {
  double time = dm2kTime();
  *taskCount = updateTaskStatus.taskCount;
  *periodicTaskCount = updateTaskStatus.periodicTaskCount;
  *updateRequestCount = updateTaskStatus.updateRequestCount;
  *updateDiscardCount = updateTaskStatus.updateDiscardCount;
  *periodicUpdateRequestCount = updateTaskStatus.periodicUpdateRequestCount;
  *periodicUpdateDiscardCount = updateTaskStatus.periodicUpdateDiscardCount;
  *updateRequestQueued = updateTaskStatus.updateRequestQueued;
  *updateExecuted = updateTaskStatus.updateExecuted;
  *timeInterval = time - updateTaskStatus.since;

  /* reset the periodic data */
  updateTaskStatus.updateRequestCount = 0;
  updateTaskStatus.updateDiscardCount = 0;
  updateTaskStatus.periodicUpdateRequestCount = 0;
  updateTaskStatus.periodicUpdateDiscardCount = 0;
  updateTaskStatus.updateExecuted = 0;
  updateTaskStatus.since = time;
}

#ifdef __cplusplus 
void wmCloseCallback(Widget w, XtPointer cd, XtPointer) {
#else
void wmCloseCallback(Widget w, XtPointer cd, XtPointer cbs) {
#endif
  ShellType shellType = (ShellType) cd;
/*
 * handle WM Close functions like all the separate dialog close functions,
 *   dispatch based upon widget value that the callback is called with
 */
  switch (shellType) {
    case DISPLAY_SHELL:
      closeDisplay(w);
      break;

    case OTHER_SHELL:
      /* it's one of the permanent shells */
      if (w == mainShell) {
	dm2kExit();
      } else
#if 1
	 XtPopdown(w);
#else
	 if (w == variableS) {
	    XtPopdown(variableS);
	 } else if (w == objectS) {
	    XtPopdown(objectS);
	 } else if (w == resourceS) {
	    XtPopdown(resourceS);
	 } else if (w == colorS) {
	    XtPopdown(colorS);
	 } else if (w == channelS) {
	    XtPopdown(channelS);
	 } else if (w == helpS) {
	    XtPopdown(helpS);
	 }
#endif
      break;
  }
}



/*
 * optionMenuService :  routine to set option menu to specified button index (sensitivity = False)
 *    or to set/reset the button sensitivity (sensitivity = True)
 */

static void optionMenuService (Widget menu, int buttonId, Boolean sensitive,
			       Boolean sensitivity)
{
  WidgetList buttons;
  Cardinal numButtons;
  Widget subMenu;

/* (MDA) - if option menus are ever created using non pushbutton or
 *	pushbutton widgets in them (e.g., separators) then this routine must
 *	loop over all children and make sure to only reference the push
 *	button derived children
 *
 *	Note: for invalid buttons, don't do anything (this can occur
 *	for example, when setting dynamic attributes when they don't
 *	really apply (and this is usually okay because they are not
 *	managed in invalid cases anyway))
 */
  XtVaGetValues(menu,XmNsubMenuId,&subMenu,NULL);
  if (subMenu != NULL) {
    XtVaGetValues(subMenu,XmNchildren,&buttons,XmNnumChildren,&numButtons,NULL);
    if (buttonId < numButtons && buttonId >= 0) {
      if ( sensitivity )
	XtSetSensitive (buttons[buttonId], sensitive);
      else
	XtVaSetValues(menu,XmNmenuHistory,buttons[buttonId],NULL);
    }
  } else {
    fprintf(stderr,"\noptionMenuService : no subMenu found for option menu");
  }
}

/*
 * optionMenuSet:  routine to set option menu to specified button index
 *		(0 - (# buttons - 1))
 */
void optionMenuSet(Widget menu, int buttonId)
{
  optionMenuService (menu, buttonId, False, False);
}

/*
 * optionMenuSensitive :  routine to set option menu senstivity for the specified button
 *		(0 - (# buttons - 1))
 */
void optionMenuSensitive (Widget menu, int buttonId, Boolean sensitive)
{
  optionMenuService (menu, buttonId, sensitive, True);
}


#ifdef __cplusplus
static void dm2kScheduler(XtPointer cd, XtIntervalId *)
#else
static void dm2kScheduler(XtPointer cd, XtIntervalId *id)
#endif
{
  extern void caCheckEvent (char *);
  PeriodicTask *t = (PeriodicTask *) cd;
  double currentTime = dm2kTime();

#ifdef DM2K_SOFT_CLOCK
  t->tenthSecond += 0.1;
  if (currentTime != t->systemTime) {
    t->systemTime = currentTime;
    t->tenthSecond = 0.0;
  }
#endif
  /* poll channel access connection event every one tenth of a second ??? */
  caCheckEvent ("dm2kScheduler");

  /* wake up any periodic task which is time out */
  if (updateTaskStatus.periodicTaskCount > 0) { 
    DisplayInfo *d = displayInfoListHead;
    while (d) {
      if (d->periodicTaskCount > 0) {
        UpdateTask *pt = d->updateTaskListHead.next;
        if (pt && pt->nextExecuteTime < currentTime) {
          updateTaskMarkTimeout(pt,currentTime);
        }
      }
      d = d->next;
    }
  }  
  
  if ((updateTaskStatus.updateRequestQueued > 0) && 
     (!updateTaskStatus.workProcId)) {
    updateTaskStatus.workProcId =
        XtAppAddWorkProc(appContext,updateTaskWorkProc,&updateTaskStatus);
  }
  t->id = XtAppAddTimeOut(appContext,100,dm2kScheduler,cd);
}

#ifdef DM2K_SOFT_CLOCK
double dm2kTime() {
  return task.systemTime + task.tenthSecond;
}
#else
double dm2kTime() {
  struct timeval tp;
  if (gettimeofday(&tp,NULL)) fprintf(stderr,"Failed!\n");
  return (double) tp.tv_sec + (double) tp.tv_usec*1e-6;
}
#endif

/* 
 *  ---------------------------
 *  routines for update tasks
 *  ---------------------------
 */


void
checkUpdateTask(UpdateTask *ut) 
{
   static int corrCnt = 0;
   fprintf(stderr, "UpdateTask @ %p\n", ut );
   if (ut && ((ut->pre_guard != PRE_GUARD) || (ut->post_guard != POST_GUARD))) {
      fprintf(stderr, "DATA CORRUPTED!!!\n");
      corrCnt += 1; exit(1);
   }
   fprintf(stderr, "  pre_guard       = 0x%08lx\n", ut->pre_guard );
   fprintf(stderr, "  executeTask     = %p\n", ut->executeTask );
   fprintf(stderr, "  destroyTask     = %p\n", ut->destroyTask );
   fprintf(stderr, "  widget          = %p\n", ut->widget );
   fprintf(stderr, "  name            = %p\n", ut->name );
   fprintf(stderr, "  clientData      = %p\n", ut->clientData );
   fprintf(stderr, "  timeInterval    = %f\n", ut->timeInterval );
   fprintf(stderr, "  nextExecuteTime = %f\n", ut->nextExecuteTime );
   fprintf(stderr, "  displayInfo     = %p\n", ut->displayInfo );
   fprintf(stderr, "  next            = %p\n", ut->next );
   fprintf(stderr, "  exeReqPendCount = %d\n", ut->executeRequestsPendingCount );
/* fprintf("  rectangle     = %\n", ut-> );*/
   fprintf(stderr, "  overlapped      = %d\n", ut->overlapped );
   fprintf(stderr, "  opaque          = %d\n", ut->opaque );
   fprintf(stderr, "  post_guard      = 0x%08lx\n", ut->post_guard );

   if (corrCnt == 4) exit(1);
}
 

void updateTaskInit(DisplayInfo *displayInfo) {
  UpdateTask *pt = &(displayInfo->updateTaskListHead);
 pt->pre_guard = PRE_GUARD;
 pt->post_guard = POST_GUARD;
  pt->executeTask = NULL;
  pt->destroyTask = NULL;
  pt->clientData = NULL;
  pt->timeInterval = 3600.0;            /* pull every hours */
  pt->nextExecuteTime = dm2kTime() + pt->timeInterval;
  pt->displayInfo = displayInfo;
  pt->next = NULL;
  pt->executeRequestsPendingCount = 0;
  displayInfo->updateTaskListTail = pt;

  if (!moduleInitialized) dm2kInitSharedDotC();
}

UpdateTask *updateTaskAddTask(
  DisplayInfo * displayInfo, 
  DlObject    * rectangle,
  void       (* executeTask)(XtPointer),
  XtPointer     clientData) 
{

  /* Notice :
   * ====
   * The value of clientData must be a pointer to the
   * dynamic structure which describe the element.
   */

  UpdateTask *pt;
  if (displayInfo == NULL) 
    return NULL;

  pt = DM2KALLOC(UpdateTask);
  if (pt == NULL) 
    return NULL;
/*printf("updateTaskAddTask %x\n", pt);*/

 pt->pre_guard = PRE_GUARD;
 pt->post_guard = POST_GUARD;

  pt->executeTask                 = executeTask;
  pt->destroyTask                 = NULL;
  pt->name                        = NULL;
  pt->clientData                  = clientData;
  pt->timeInterval                = 0.0;
  pt->nextExecuteTime             = dm2kTime() + pt->timeInterval;
  pt->displayInfo                 = displayInfo;
  pt->next                        = NULL;
  pt->executeRequestsPendingCount = 0;

  if (rectangle) {
    pt->rectangle.x      = rectangle->x;
    pt->rectangle.y      = rectangle->y;
    pt->rectangle.width  = rectangle->width;
    pt->rectangle.height = rectangle->height;
    rectangle->runtimeDescriptor = clientData;
  } else {
    pt->rectangle.x      = 0;
    pt->rectangle.y      = 0;
    pt->rectangle.width  = 0;
    pt->rectangle.height = 0;
  }

  pt->overlapped = True;  /* make the default is True */
  pt->opaque = True;      /* don't draw the background */

  displayInfo->updateTaskListTail->next = pt;
  displayInfo->updateTaskListTail = pt;

/*printf("  displayInfo = %x\n", displayInfo);*/

  if (pt->timeInterval > 0.0) 
  {
    displayInfo->periodicTaskCount++;
    updateTaskStatus.periodicTaskCount++;
    
    if (pt->nextExecuteTime < 
	displayInfo->updateTaskListHead.nextExecuteTime) 
      displayInfo->updateTaskListHead.nextExecuteTime = pt->nextExecuteTime;
  }

  updateTaskStatus.taskCount++;

  return pt;
}  

void updateTaskDeleteTask(UpdateTask *pt) 
{
  UpdateTask  * tmp;
  DisplayInfo * displayInfo;

/*printf("updateTaskDeleteTask (pt=%x)\n", pt);*/
  /*if (pt == NULL) return; */

/*printf("updateTaskStatus.taskCount = %d\n", updateTaskStatus.taskCount);*/
  displayInfo = pt->displayInfo;
  tmp = &(pt->displayInfo->updateTaskListHead);

/*  checkUpdateTask(tmp);*/
  while (tmp->next) 
  {
    if (tmp->next == pt) {
/*printf( "  UpdateTask found\n" );*/
      tmp->next = pt->next;
      
      if (pt == displayInfo->updateTaskListTail)
        displayInfo->updateTaskListTail = tmp;
  
      if (pt->destroyTask) 
	(*pt->destroyTask)(pt->clientData);

      if (pt->timeInterval > 0.0) {
	displayInfo->periodicTaskCount--;
	updateTaskStatus.periodicTaskCount--;
      }

      updateTaskStatus.taskCount--;

      if (pt->executeRequestsPendingCount > 0) 
	updateTaskStatus.updateRequestQueued--;

      if (updateTaskStatus.nextToServe == pt) 
	updateTaskStatus.nextToServe = pt->next;

      if (nextUpdateTaskToDestroy == pt)
	nextUpdateTaskToDestroy  = pt->next;

/*printf("Sfreeing 0x%x\n", (char*)pt );*/
      free((char *)pt);

      break;
    }

    tmp = tmp->next;
/*    checkUpdateTask(tmp);*/
  }

#if 0

  {
    int i;
    time_t t = time(NULL);
    struct tm * s = localtime(&t);
    char buffer[40];

    strftime(buffer, 40, "%a %d/%m/%Y-%H:%M:%S", s);
    
    tmp = &(displayInfo->updateTaskListHead);
    
    for ( i = 0; tmp->next != NULL; i++ , tmp = tmp->next)
      ;
    printf("%-40s i = %-5d c=%-5d\n",
	   buffer, i, displayInfo->periodicTaskCount,
	   updateTaskStatus.periodicTaskCount );
  }
#endif
/*printf(" updateTaskDeleteTask done.\n");*/

}


void updateTaskDeleteAllTask(UpdateTask *pt) 
{
  UpdateTask *tmp;
  DisplayInfo *displayInfo;

  if (pt == NULL) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  if (pt->displayInfo == NULL)
    return;

  displayInfo = pt->displayInfo;
  displayInfo->periodicTaskCount = 0;
/*printf("updateTaskDeleteAllTask( 0x%x, displayInfo = 0x%x )\n", pt, displayInfo );*/
  tmp = displayInfo->updateTaskListHead.next;

#if 1
  displayInfo->updateTaskListHead.next = NULL;
  displayInfo->updateTaskListTail = &(displayInfo->updateTaskListHead);
#endif

  while (tmp) 
  {
    UpdateTask *tmp1 = tmp;
/*printf("updateTaskDelete(All)Task( 0x%x )\n", tmp1 );*/
    /* ordering in list can be changed after 
     * an invokation of tmp1->destroyTask method;
     * nextUpdateTaskToDestroy helps us to keep next leaf
     */
    nextUpdateTaskToDestroy = tmp1->next;

    if (tmp1->destroyTask) {
      tmp1->destroyTask(tmp1->clientData);
    }

    if (tmp1->timeInterval > 0.0) {
      updateTaskStatus.periodicTaskCount--;
    }

    updateTaskStatus.taskCount--;
    if (tmp1->executeRequestsPendingCount > 0) {
      updateTaskStatus.updateRequestQueued--;
    }

    if (updateTaskStatus.nextToServe == tmp1) {
      updateTaskStatus.nextToServe = NULL;
    }

    tmp1->executeTask = NULL;
/*printf("Sfreeing 0x%x\n", tmp1 );*/
    free((char *)tmp1);

    tmp = nextUpdateTaskToDestroy;
/*printf(".\n" );*/
  }

  displayInfo->updateTaskListHead.next = NULL;
  displayInfo->updateTaskListTail = &(displayInfo->updateTaskListHead);

  if ((updateTaskStatus.taskCount <=0) && (updateTaskStatus.workProcId)) {
    XtRemoveWorkProc(updateTaskStatus.workProcId);
    updateTaskStatus.workProcId = 0;
  }

  caPendEvent ("updateTaskDeleteAllTask");
}
  
static int updateTaskMarkTimeout(UpdateTask *pt, double currentTime) 
{
  UpdateTask *head = &(pt->displayInfo->updateTaskListHead);
  UpdateTask *tmp = head->next;
  int count = 0;

  /* reset the nextExecuteTime for the display an hour later 
   */
  head->nextExecuteTime = currentTime + head->timeInterval;

  while (tmp) 
  {
    /* if periodic task 
     */
    if (tmp->timeInterval > 0.0) {
      /* mark if the task is time out already */
      if (currentTime > tmp->nextExecuteTime) {
        count++;
        if (tmp->executeRequestsPendingCount > 0) {
          updateTaskStatus.periodicUpdateDiscardCount++;
        } else {
          updateTaskStatus.periodicUpdateRequestCount++;
          updateTaskStatus.updateRequestQueued++;
        }
        tmp->executeRequestsPendingCount++;
        tmp->nextExecuteTime += tmp->timeInterval;
        /* retrieve the closest next execute time */
        if (tmp->nextExecuteTime < head->nextExecuteTime) {
          head->nextExecuteTime = tmp->nextExecuteTime;
        }
      }
    }
    tmp = tmp->next;
  }
  return count;
}   

void updateTaskMarkUpdate(UpdateTask *pt) 
{
  if (pt->executeRequestsPendingCount > 0) {
    updateTaskStatus.updateDiscardCount++;
  } else {
    updateTaskStatus.updateRequestCount++;
    updateTaskStatus.updateRequestQueued++;
  }
  pt->executeRequestsPendingCount++;
}

void updateTaskSetScanRate(UpdateTask *pt, double timeInterval) 
{
  UpdateTask *head = &(pt->displayInfo->updateTaskListHead);
  double currentTime = dm2kTime();

  /*
   * increase or decrease the periodic task count depends
   * on the condition
   */
  if ((pt->timeInterval == 0.0) && (timeInterval != 0.0)) {
    pt->displayInfo->periodicTaskCount++;
    updateTaskStatus.periodicTaskCount++;
  } else
  if ((pt->timeInterval != 0.0) && (timeInterval == 0.0)) {
    pt->displayInfo->periodicTaskCount--;
    updateTaskStatus.periodicTaskCount--;
  }

  /*
   * set up the next scan time for this task, if it
   * this is the sooner one, set it to the display
   * scan time too.
   */
  pt->timeInterval = timeInterval;
  pt->nextExecuteTime = currentTime + pt->timeInterval;
  if (pt->nextExecuteTime < head->nextExecuteTime) {
    head->nextExecuteTime = pt->nextExecuteTime;
  }
}

void updateTaskAddExecuteCb(UpdateTask *pt, void (*executeTaskCb)(XtPointer)) 
{
  pt->executeTask = executeTaskCb;
}

void updateTaskAddDestroyCb(UpdateTask *pt, void (*destroyTaskCb)(XtPointer)) 
{
  pt->destroyTask = destroyTaskCb;
}


static Boolean updateTaskWorkProc(XtPointer cd) 
{
  UpdateTaskStatus * ts = (UpdateTaskStatus *) cd;
  UpdateTask       * t = ts->nextToServe;
  double             endTime;
  UpdateTask       * copyNext;

#define FINISH_RETURN ts->workProcId = 0; return True
  endTime = dm2kTime() + 0.05; 
 
  do { 
    if (ts->updateRequestQueued <=0) { FINISH_RETURN; } 
    /* if no valid update task, find one 
     */
    if (t == NULL) {
      DisplayInfo *d = displayInfoListHead;

      if (d == NULL) { FINISH_RETURN; } /* no displays */
      
      /* find out the next update task 
       */
      while (d) {
	if ((t = d->updateTaskListHead.next))
	  break;
	d = d->next;
      }
      
      if (t == NULL) { FINISH_RETURN; }	/* no update task */ 

      ts->nextToServe = t;
    }
    
    /* Now, at least one update task found 
     * find one of which executeRequestsPendingCount > 0 
     */ 
    while (t->executeRequestsPendingCount <=0 ) {
      DisplayInfo *d = t->displayInfo;

      t = t->next;
      while (t == NULL) {
	/* end of the update task for this display */
	/* check the next display.                 */

	if ((d = d->next) == NULL) 
	  d = displayInfoListHead;

	t = d->updateTaskListHead.next;
      } 

      /* all display are checked, no update is needed */
      if (t == ts->nextToServe) { FINISH_RETURN; } 
    }

    copyNext = ts->nextToServe = t;
    t->executeRequestsPendingCount = 0;

    if (t->overlapped)   /* repaint the selected rectangle */
      {
	DisplayInfo *pDI     = t->displayInfo;
	Display     *display = XtDisplay(pDI->drawingArea);
	GC          gc       = pDI->gc;
	XPoint      active_rectangle[2];
	
	active_rectangle[0].x = t->rectangle.x;
	active_rectangle[0].y = t->rectangle.y;
	active_rectangle[1].x = t->rectangle.x + t->rectangle.width;
	active_rectangle[1].y = t->rectangle.y + t->rectangle.height;
	
	XSetClipRectangles(display, gc, 0, 0, &t->rectangle, 1, YXBanded);

	if (t->opaque == 0)
	  XCopyArea(display,pDI->drawingAreaPixmap, 
		    XtWindow(pDI->drawingArea),gc,
		    t->rectangle.x,     t->rectangle.y,
		    t->rectangle.width, t->rectangle.height,
		    t->rectangle.x,     t->rectangle.y);
	
	t->overlapped = False;     
	
	t = t->displayInfo->updateTaskListHead.next;
	
	while (t) 
	  {
	    /* check whether rectangle of active task crosses 
	     * current task's rect.
	     */
	    if ( ! IF_NOT_CROSS(active_rectangle[0].x,
				active_rectangle[0].y,
				active_rectangle[1].x,
				active_rectangle[1].y,
				t->rectangle.x,
				t->rectangle.y,
				t->rectangle.x + t->rectangle.width,
				t->rectangle.y + t->rectangle.height))
	      {
		t->overlapped = True;
		if (t->executeTask)
		  (*t->executeTask)(t->clientData);
	      }
	    t = t->next;
	  }
	
	/* reset the clipping region */
	XSetClipOrigin(display,gc,0,0);
	XSetClipMask(display,gc,None);
      } 
    else 
      {
	if (t->opaque == 0) 
	  XCopyArea(XtDisplay(t->displayInfo->drawingArea),
		    t->displayInfo->drawingAreaPixmap,
		    XtWindow(t->displayInfo->drawingArea),
		    t->displayInfo->gc,
		    t->rectangle.x, t->rectangle.y,
		    t->rectangle.width, t->rectangle.height,
		    t->rectangle.x, t->rectangle.y);
	if (t->executeTask) 
	  (*t->executeTask)(t->clientData);
      }     
    
    ts->updateExecuted++;
    ts->updateRequestQueued--;
    
    /* find out next to serve 
     */
    t = ts->nextToServe;

    if (copyNext == ts->nextToServe && t != NULL)
    {
      while (t->executeRequestsPendingCount <=0 ) {
	DisplayInfo *d = t->displayInfo;
	
	t = t->next;
	while (t == NULL) 
	{
	  /* end of the update task for this display 
	   * check the next display.                 
	   */

	  if ((d = d->next) == NULL) 
	    d = displayInfoListHead;

	  t = d->updateTaskListHead.next;
	}

	/* check all displays, no update is needed */
	if (t == ts->nextToServe) { FINISH_RETURN; }
      }

      ts->nextToServe = t;
    }

  } while (endTime > dm2kTime());
  
  return False;
#undef FINISH_RETURN
}

void updateTaskRepaintRectangle (
     DisplayInfo *displayInfo, 
     int         x0, 
     int         y0, 
     int         x1, 
     int         y1)
{
  register UpdateTask *t = displayInfo->updateTaskListHead.next;

  while (t) {
    if ( /* (t->rectangle.width == 0 && t->rectangle.height == 0) ||  */
	 ! IF_NOT_CROSS(x0, y0, x1, y1,
			t->rectangle.x, 
			t->rectangle.y,
			t->rectangle.x + t->rectangle.width,
			t->rectangle.y + t->rectangle.height))
      {
	if (t->executeTask) 
	  (*t->executeTask)(t->clientData);
      }
    t = t->next;
  }
}

void updateTaskAddNameCb(UpdateTask *pt, 
			 void (*nameCb)(XtPointer, char **, short *, int *)) 
{
  pt->name = nameCb;
}
