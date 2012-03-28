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
 * .02  09-08-95        vong    conform to c++ syntax
 *
 *****************************************************************************
 *
 *      24-02-97    Fabien  createPopupDisplayMenu is in dm2k.c
 *                          DM2K crash on Linux X server when popup menu
 *                          are direct child of shell !
 *                          Define simpleCursor
 *                          Use simpleCursor in display DrawingArea
 *
 *      20-06-97    Fabien  correction in utils.c
 *                          in dmCleanupDisplayInfo do not 
 *                          XtDestroyWidget shell if the shell does not exit.
 *
 *****************************************************************************
*/

#include <string.h>

#include "dm2k.h"
#include <Xm/MwmUtil.h>

static char *displayTypeString[] = {
  "normal", 
  "dialog_application_modal", 
  "dialog_application_modeless"
};

#if 0
static void getShellPositionPolicy (DisplayInfo *);
#endif

static void displayGetValues(ResourceBundle *, DlElement *);
static char getDialogStyle (DisplayInfo *);

static void destroyDlElement (DlElement * element);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);

static DlDispatchTable displayDlDispatchTable = {
         createDlDisplay,
         destroyDlElement,
         executeMethod,
         writeDlDisplay,
         NULL,
         displayGetValues,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
	 NULL,
	 displayObjectInfo
};

static void destroyDlDisplay (DlDisplay * dlDisplay)
{
  if (dlDisplay == NULL)
    return;

  objectAttributeDestroy(&(dlDisplay->object));
  DM2KFREE(dlDisplay->cmap);

  free((char*)dlDisplay);
}

static void destroyDlElement (DlElement * element)
{
  if (element == NULL || element->type != DL_Display) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  destroyDlDisplay (element->structure.display);
  free((char*)element);
}

/*
 * Function to set the display DrawingArea cursor
 *
 */

void drawingAreaDefineCursor (DisplayInfo *displayInfo)
{
Cursor cursorId;

  if ( ! displayInfo->drawingArea ) return;

  if (globalDisplayListTraversalMode == DL_EXECUTE) {
    cursorId = simpleCursor;
  }
  else {
    cursorId = (currentActionType == SELECT_ACTION) ?
		rubberbandCursor : crosshairCursor;
  }

  XDefineCursor (display, XtWindow(displayInfo->drawingArea),
		 cursorId);
  displayInfo->currentCursor = cursorId;
}


/* the default position must not be 0,0 (random position by Window Manager) */
#define DISPLAY_DEFAULT_X 10
#define DISPLAY_DEFAULT_Y 10
 
/*
 * create and fill in widgets for display
 */

DisplayInfo *createDisplay ()
{
  DisplayInfo *displayInfo;
  DlElement *dlElement;
  DlDisplay *dlDisplay;

/* clear currentDisplayInfo - not really one yet */
  currentDisplayInfo = NULL;
  initializeGlobalResourceBundle();

  if (!(displayInfo = allocateDisplayInfo())) return NULL;
#if 0
  /* <== <== */
  displayInfo->displayType = NORMAL_DISPLAY;
  createDisplayShell (displayInfo);
  /* <== <== */
#endif

/* general scheme: update  globalResourceBundle, then do creates */
  globalResourceBundle.x =      DISPLAY_DEFAULT_X;
  globalResourceBundle.y =      DISPLAY_DEFAULT_Y;
  globalResourceBundle.width =  DEFAULT_DISPLAY_WIDTH;
  globalResourceBundle.height = DEFAULT_DISPLAY_HEIGHT;

  renewString(&globalResourceBundle.name,DEFAULT_FILE_NAME);

  displayInfo->drawingArea = NULL;
  displayInfo->dlFile = createDlFile(displayInfo);

  dlElement = createDlDisplay(NULL);

  if (dlElement) {
    dlDisplay = dlElement->structure.display;
    dlDisplay->object.x = globalResourceBundle.x;
    dlDisplay->object.y = globalResourceBundle.y;
    dlDisplay->object.width = globalResourceBundle.width;
    dlDisplay->object.height = globalResourceBundle.height;
    dlDisplay->clr = globalResourceBundle.clr;
    dlDisplay->bclr = globalResourceBundle.bclr;
    dlDisplay->displayType = globalResourceBundle.displayType;
    appendDlElement(displayInfo->dlElementList,dlElement);
  } else {
     /* cleanup up displayInfo */
     return NULL;
  }
  
  displayInfo->dlColormap = createDlColormap(displayInfo);
  dmTraverseDisplayList(displayInfo);
  
  drawingAreaDefineCursor (displayInfo);
  
  XtManageChild (displayInfo->drawingArea);
  XtPopup (displayInfo->shell, XtGrabNone);
  positionDisplay (displayInfo, True);
  
  return(displayInfo);
}


DlElement *createDlDisplay(DlElement *p)
{
  DlDisplay *dlDisplay;
  DlElement *dlElement;
 
 
  dlDisplay = DM2KALLOC(DlDisplay);

  if (dlDisplay == NULL) 
    return NULL;

  if (p != NULL) {
    objectAttributeCopy(&(dlDisplay->object), &(p->structure.display->object));

    dlDisplay->object.x    = p->structure.display->object.x;
    dlDisplay->object.y    = p->structure.display->object.y;
    dlDisplay->clr         = p->structure.display->clr;
    dlDisplay->bclr        = p->structure.display->bclr;
    dlDisplay->displayType = p->structure.display->displayType;

    renewString(&(dlDisplay->cmap),p->structure.display->cmap);
  } 
  else {
    objectAttributeInit(&(dlDisplay->object));

    dlDisplay->object.x    = DISPLAY_DEFAULT_X;
    dlDisplay->object.y    = DISPLAY_DEFAULT_Y;
    dlDisplay->clr         = 0;
    dlDisplay->bclr        = 1;
    dlDisplay->cmap        = NULL;
    dlDisplay->displayType = NORMAL_DISPLAY;
  }

  dlElement = createDlElement(DL_Display,
			      (XtPointer)dlDisplay,
			      &displayDlDispatchTable);

  if (dlElement == NULL)
    destroyDlDisplay(dlDisplay);
 
  return(dlElement);
}


static char * GetShortFileName (char * fullName)
{
  char * copyFullName;
  char * shortName = NULL;
  
  if (fullName) {
    copyFullName = strdup(fullName);
 
    if (copyFullName != NULL) 
      {
        if (strtok(copyFullName, "/") != NULL) {
          char * tmp;
 
          while ((tmp = strtok(NULL, "/")) != NULL)
            shortName = tmp;
          
          shortName = strdup((shortName != NULL) ? shortName : fullName);
        }
 
        free(copyFullName);
      }
  }
 
  return shortName;
}


static Boolean createPopupDisplayMenuWorkProc (XtPointer client_data)
{
  DisplayInfo *displayInfo,*found;
  displayInfo = (DisplayInfo *) client_data;

  /* T. Straumann: at the time this work procedure gets a chance to
   *			   run the displayInfo ptr may be stale, i.e. the
   *			   display has been closed (destroyed) already.
   *			   We better check if it's still in the list.
   */
  for (found=displayInfoListHead; found && found!=displayInfo ; found=found->next)
	/* do nothing more */;

  if (found)
     createPopupDisplayMenu (displayInfo->traversalMode, 
			     displayInfo, displayInfo->drawingArea);

  /* remove the Work Proc automaticaly 
   */
  return (found!=NULL); 
}



static void displayDeferedAction (DisplayInfo *displayInfo, Boolean applyFlag)
{
  DlElement *dlElement;

  /* traverse the display list */
  dlElement = FirstDlElement(displayInfo->dlElementList);
  while (dlElement) {
    if ( dlElement->type != DL_Display ) {
      if ( dlElement->run->deferredAction ) {
	if ( (dlElement->widget != NULL) || dlElement->actif ) {
	  (dlElement->run->deferredAction) (dlElement, applyFlag);
	}
      }
    }
    dlElement = dlElement->next;
  }
}


static void displayOkActionCb(Widget w,
                XtPointer clientData,
		XtPointer callbackData)
{
  DisplayInfo *displayInfo;

  displayInfo = (DisplayInfo *) clientData;
  displayDeferedAction (displayInfo, True);
  closeDisplay (displayInfo->drawingArea);
}


static void displayApplyActionCb(Widget w,
                XtPointer clientData,
		XtPointer callbackData)
{
  DisplayInfo *displayInfo;

  displayInfo = (DisplayInfo *) clientData;
  displayDeferedAction (displayInfo, True);
}


static void displayClearActionCb(Widget w,
                XtPointer clientData,
		XtPointer callbackData)
{
  DisplayInfo *displayInfo;

  displayInfo = (DisplayInfo *) clientData;
  displayDeferedAction (displayInfo, False);
}


static void displayCancelActionCb(Widget w,
                XtPointer clientData,
		XtPointer callbackData)
{
  DisplayInfo *displayInfo;

  displayInfo = (DisplayInfo *) clientData;
  closeDisplay (displayInfo->drawingArea);
}


typedef struct {
    char *label;
    void (*callback)(Widget, void*, void*);
} ActionAreaItem;

static ActionAreaItem action_items[] = {
  { "OK",     displayOkActionCb     },
  { "Apply",  displayApplyActionCb  },
  { "Clear",  displayClearActionCb  },
  { "Cancel", displayCancelActionCb },
};


static void createActionArea (
  DisplayInfo *displayInfo,
  ActionAreaItem *actions,
  int num_actions,
  int defaultButton)
{
  Widget widget;
  int i;

  if ( ! displayInfo->dialog ) return;

  for (i = 0; i < num_actions; i++) {
    widget = XtVaCreateManagedWidget (actions[i].label,
			xmPushButtonWidgetClass, displayInfo->dialog,
			XmNshowAsDefault, i == defaultButton,
			XmNdefaultButtonShadowThickness, 1,
			NULL);
    if (displayInfo->traversalMode == DL_EDIT) {
      XtSetSensitive (widget, False);
    }
    else {
      if (actions[i].callback)
	XtAddCallback (widget, XmNactivateCallback,
	    actions[i].callback, (XtPointer) displayInfo);
    }
    if (i == 0)
      XtVaSetValues (displayInfo->dialog, XmNdefaultButton, widget, NULL);
  }
}


void createDialogActionArea (DisplayInfo *displayInfo)
{
  Arg args[20];
  int n;

  createActionArea (displayInfo, action_items, XtNumber (action_items), -1);
  n = 0;
  /* No default action button can be used,
     they are in conflict with the Text Entry controler */
  XtSetArg (args[n], XmNdefaultButtonType, XmDIALOG_NONE); n++;
  XtSetValues (displayInfo->dialog, args, n);
}


static Boolean testRebuildDisplayShell (DisplayInfo *displayInfo,
					DisplayType newDisplayType)
{
  Boolean dialogDisplay, dialogShell;

  if ( displayInfo->shell == NULL ) return (True);
  if ( displayInfo->displayType == newDisplayType ) return (False);

  dialogDisplay = (newDisplayType != NORMAL_DISPLAY);
  dialogShell = XtIsTransientShell (displayInfo->shell);
  if (   (dialogDisplay && dialogShell)
      || (!dialogDisplay && !dialogShell) )
    return (False);
  return (True);
}


void destroyRebuildDisplayShell (DisplayInfo *displayInfo, 
				 DisplayType newDisplayType)
{
  if ( displayInfo->shell ) {
    /* destroy the shell and associated ressources */
    XtDestroyWidget (displayInfo->shell);
    displayInfo->shell = displayInfo->dialog = NULL;
    freeDisplayXResources (displayInfo);
  }
  /* rebuild the shell */
  displayInfo->displayType = newDisplayType;
  createDisplayShell (displayInfo);
}


void rebuildDisplayShell (DisplayInfo *displayInfo, DisplayType newDisplayType)
{
#ifdef __hpux
  /* set correct position of window  
  *** HACK ***  13 Aug 97 16:01:53 Thomas Birke 
  */
  if (displayInfo->shell)
    XtVaSetValues (displayInfo->shell,
		   XmNx, displayInfo->xPosition,
		   XmNy, displayInfo->yPosition,
		   NULL);  
#endif

  if ( displayInfo->displayType == newDisplayType ) return;

  if ( ! testRebuildDisplayShell (displayInfo, newDisplayType) ) {
    /* no need for a full rebuild of the shell */
    displayInfo->displayType = newDisplayType;
    if ( displayInfo->dialog )
      XtVaSetValues (displayInfo->dialog,
		     XmNdialogStyle, getDialogStyle (displayInfo),
		     NULL);
    return;
  }

  /* destroy and rebuild the shell */
  destroyRebuildDisplayShell (displayInfo, newDisplayType);
}


static char getDialogStyle (DisplayInfo *displayInfo)
{
  if (displayInfo->traversalMode == DL_EDIT) return (MODELESS_DIALOG);

  return ((displayInfo->displayType == MODELESS_DIALOG) ? XmDIALOG_MODELESS : XmDIALOG_APPLICATION_MODAL);
}


/* WARNING : creation and position and size management of the shells,
 *  drawing areas, dialogs widgets is very sensitive to the behaviour
 *  of the window manager.
 *  This version was tested with window managers
 *  of the mwm (Motif Wiwdow Manager) and fvwm  (F(?) Virtual Window Manager)
 *  families. There is probably still a number of strange phenomenas
 *  in the interraction between wm and the application windows.
 *  Any modification in the piece of code which deal with window size,
 *  and with the window screen position must be carrefully checked with
 *  the various Window Manager, and in a local (X server, and the dm2k
 *  application in the same computer) and remote case (wm and dm2k
 *  on different computers).
*/

static void createDisplayDrawingArea (DisplayInfo *displayInfo, DlElement *dlElement)
{
  Arg args[20];
  int n, n1;
  DlDisplay *dlDisplay;
  Widget parent;
  Boolean displayDialog, resize;
 
  if (displayInfo->drawingArea != NULL) return;

  dlDisplay = dlElement->structure.display;

  displayDialog = (displayInfo->displayType != NORMAL_DISPLAY);
  resize = (displayInfo->traversalMode == DL_EDIT);

  n = 0;
  XtSetArg(args[n],XmNborderWidth,(Dimension)0); n++;
  XtSetArg(args[n],XmNmarginWidth,(Dimension)0); n++;
  XtSetArg(args[n],XmNmarginHeight,(Dimension)0); n++;
  XtSetArg(args[n],XmNshadowThickness,(Dimension)0); n++;
  n1 = n;

  if ( displayDialog && (displayInfo->dialog == NULL)) {
    /* create the message box */
    XtSetArg (args[n], XmNdialogType, XmDIALOG_TEMPLATE); n++;
    XtSetArg (args[n], XmNdialogStyle, getDialogStyle (displayInfo)); n++;
    XtSetArg (args[n], XmNdefaultPosition, False); n++;
    XtSetArg (args[n], XmNautoUnmanage, False); n++;
    XtSetArg (args[n], XmNnoResize, True); n++;
    displayInfo->dialog = XmCreateMessageBox (displayInfo->shell, "displayMB", args, n);
    createDialogActionArea (displayInfo);
  }

  /* from the DlDisplay structure, we've got drawingArea's dimensions */
  n = n1;
  XtSetArg(args[n],XmNwidth,(Dimension)dlDisplay->object.width); n++;
  XtSetArg(args[n],XmNheight,(Dimension)dlDisplay->object.height); n++;
  XtSetArg(args[n],XmNresizePolicy,XmRESIZE_NONE); n++;

  /* N.B.: don't use userData resource since it is used later on for aspect
   *   ratio-preserving resizes 
   */
    
  displayInfo->executePopupMenu = NULL;
  displayInfo->editPopupMenu    = NULL;

  parent = ( displayInfo->dialog ) ? displayInfo->dialog : displayInfo->shell;
  displayInfo->drawingArea = XmCreateDrawingArea(parent, "displayDA",args,n);

  if ( displayInfo->dialog ) {
    XtVaSetValues (displayInfo->dialog, XmNnoResize, !resize, NULL);
  }
  XtVaSetValues (displayInfo->shell,
                 XmNmwmFunctions,   MWM_FUNC_ALL|MWM_FUNC_RESIZE,
		 XmNmwmDecorations, MWM_DECOR_ALL|MWM_DECOR_RESIZEH,
		 NULL);

  XtRealizeWidget (displayInfo->shell);
  XtManageChild (displayInfo->drawingArea);

  /* add expose & resize  & input callbacks for drawingArea */
  XtAddCallback(displayInfo->drawingArea,XmNexposeCallback,
		 (XtCallbackProc)drawingAreaCallback,
		 (XtPointer)displayInfo);
  XtAddCallback(displayInfo->drawingArea,XmNresizeCallback,
		(XtCallbackProc)drawingAreaCallback,(XtPointer)displayInfo);

  if ( displayInfo->dialog ) XtManageChild(parent);

  (void) XtAppAddWorkProc (appContext, createPopupDisplayMenuWorkProc,
			   (XtPointer) displayInfo);

  /*
   * and if in EDIT mode...
   */
  if (displayInfo->traversalMode == DL_EDIT) {
    /* handle input (arrow keys) */
    XtAddCallback(displayInfo->drawingArea,XmNinputCallback,
		  (XtCallbackProc)drawingAreaCallback,
		  (XtPointer)displayInfo);

    /* and handle button presses and enter windows */
    XtAddEventHandler(displayInfo->drawingArea,ButtonPressMask,False,
		      handleButtonPress,(XtPointer)displayInfo);

    XtAddEventHandler(displayInfo->drawingArea,EnterWindowMask,False,
		      (XtEventHandler)handleEnterWindow,
		      (XtPointer)displayInfo);

  }
  else if (displayInfo->traversalMode == DL_EXECUTE)
  {
    /*
     *  MDA --- HACK to fix DND visuals problem with SUN server
     *   Note: This call is in here strictly to satisfy some defect in
     *    the MIT and other X servers for SUNOS machines
     *     This is completely unnecessary for HP, DEC, NCD, ...
     */
    XtSetArg(args[0],XmNdropSiteType,XmDROP_SITE_COMPOSITE);
    XmDropSiteRegister(displayInfo->drawingArea,args,1);

    /* and handle button presses and enter windows */
    XtAddEventHandler(displayInfo->drawingArea,ButtonPressMask,False,
		      popupMenu,(XtPointer)displayInfo);

    /* cursor motion event handler */
    XtAddEventHandler(displayInfo->drawingArea,PointerMotionMask,False,
		      (XtEventHandler)motionHandler,
		      (XtPointer)displayInfo);

    /* add in drag/drop translations */
    XtOverrideTranslations(displayInfo->drawingArea,parsedTranslations);
  }

  /* add handler for the popup menu */
  XtAddEventHandler (displayInfo->drawingArea, ButtonReleaseMask, False,
		     popdownMenu, displayInfo);
}


/*
 * this execute... function is a bit unique in that it can properly handle
 *  being called on extant widgets (drawingArea's/shells) - all other
 *  execute... functions want to always create new widgets (since there
 *  is no direct record of the widget attached to the object/element)
 *  {this can be fixed to save on widget create/destroy cycles later}
 */
static UpdateTask * executeMethod
   (DisplayInfo * displayInfo, 
    DlElement   * dlElement)
{
  DlColormap *dlColormap;
  Arg args[12];
  int n, n1;
  DlDisplay *dlDisplay = dlElement->structure.display;
  Widget parent;
  Boolean savedFlag;
  if ( displayInfo->dlDisplayElement != dlElement ) {
    /* only when the display is created */
    displayInfo->dlDisplayElement = dlElement;
    displayInfo->xPosition = dlDisplay->object.x;
    displayInfo->yPosition = dlDisplay->object.y;
  }

  /* check if we have new type of display's shell, 
   * if yes, rebuild it
   */
  rebuildDisplayShell (displayInfo, dlDisplay->displayType);

  parent = ( displayInfo->dialog ) ? displayInfo->dialog : displayInfo->shell;

  /* set the display's foreground and background colors */
  displayInfo->drawingAreaBackgroundColor = dlDisplay->bclr;
  displayInfo->drawingAreaForegroundColor = dlDisplay->clr;
 
  /* N.B.: don't use userData resource since it is used later on for aspect
   *   ratio-preserving resizes 
   */
    
  savedFlag = displayInfo->hasBeenEditedButNotSaved;
  if (displayInfo->drawingArea == NULL) {
    createDisplayDrawingArea (displayInfo, dlElement);
  } else {
    /* from the DlDisplay structure, we've got drawingArea's dimensions */
    n = 0;
    XtSetArg(args[n],XmNwidth,(Dimension)dlDisplay->object.width); n++;
    XtSetArg(args[n],XmNheight,(Dimension)dlDisplay->object.height); n++;
    XtSetArg(args[n],XmNresizePolicy,XmRESIZE_NONE); n++;
    XtSetValues(displayInfo->drawingArea,args,n);
  }
 
  /* wait to realize the shell... */
  n = 0;
  XtSetArg(args[n],XmNiconName, GetShortFileName(displayInfo->dlFile->name)); n++;
  XtSetArg(args[n],XmNmwmFunctions,     MWM_FUNC_ALL|MWM_FUNC_RESIZE); n++;
  XtSetArg(args[n],XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_RESIZEH); n++;
  XtSetArg(args[n],XmNallowShellResize,True); n++;
  XtSetArg(args[n],XmNallowShellResize,True); n++;
  if ( displayInfo->dialog == NULL ) {
    n1 = n;
    XtSetArg(args[n],XmNwidth,(Dimension)dlDisplay->object.width); n++;
    XtSetArg(args[n],XmNheight,(Dimension)dlDisplay->object.height); n++;
    XtSetValues (displayInfo->drawingArea, &args[n1], n-n1);
  }

  /* This call can generated resize callback on DrawingArea */
  XtSetValues(displayInfo->shell,args,n);

  dm2kSetDisplayTitle(displayInfo);

  XtRealizeWidget(displayInfo->shell);
  if ( displayInfo->dialog != NULL ) {
    XtRealizeWidget (displayInfo->dialog);
  }
  XtManageChild(parent);
  XtPopup (displayInfo->shell, XtGrabNone);
  positionDisplay (displayInfo, True);

  displayInfo->hasBeenEditedButNotSaved = savedFlag;

  /* if there is an external colormap file specification, 
   * parse/execute it now
   */
  if (dlDisplay->cmap != NULL)  {
    if ((dlColormap = 
	 parseAndExtractExternalColormap(displayInfo, dlDisplay->cmap))) {
      executeDlColormap(displayInfo,dlColormap);
    } else {
      fprintf(stderr,
        "\nexecuteMethod: can't parse and execute external colormap %s",
        dlDisplay->cmap);
      if (displayInfo->dlColormap != NULL)  {
	 executeDlColormap(displayInfo,displayInfo->dlColormap);
      } else {
	 fprintf(stderr, "\nexecuteMethod: no own colormap found, terminating\n");
	 dm2kCATerminate();
	 dmTerminateX();
	 exit(-1);
      }
    }
  } else {
    executeDlColormap(displayInfo,displayInfo->dlColormap);
  }
  positionDisplayRead (displayInfo);
  return NULL;
}


void parseDisplayType (DisplayInfo * displayInfo,
		       DlDisplay   * dlDisplay,
		       char        * token)
{
  int i;

  dlDisplay->displayType = displayInfo->displayType;

  /* T. Straumann: protect against null pointer arg */
  if (!token) return;

  for ( i = 0 ; i < XtNumber(displayTypeString); i++ ) 
    {
      if ( !strcmp (token, displayTypeString[i]) ) {
	dlDisplay->displayType = (DisplayType)(FIRST_DISPLAY_TYPE + i);
	break;
      }
    }
}

char * displayTypeMsg (DisplayType displayType)
{
  if ( displayType < FIRST_DISPLAY_TYPE ) return ("Undefined type");
  return (displayTypeString[displayType-FIRST_DISPLAY_TYPE]);
}


void writeDlDisplay(
  FILE *stream,
  DlElement *dlElement,
  int level)
{
  int i;
  char indent[256]; level=MIN(level,256-2);
  DlDisplay *dlDisplay = dlElement->structure.display;
 
  for (i = 0; i < MIN(level,256-2) && i<256-1; i++) 
    indent[i] = '\t'; 
  indent[i] = '\0';
 
  fprintf(stream,"\n%sdisplay {",indent);
  writeDlObject(stream,&(dlDisplay->object),level+1);
  fprintf(stream,"\n%s\tclr=%d",indent,dlDisplay->clr);
  fprintf(stream,"\n%s\tbclr=%d",indent,dlDisplay->bclr);

  fprintf(stream,"\n%s\tcmap=\"%s\"",indent,CARE_PRINT(dlDisplay->cmap));

  if ( dlDisplay->displayType != NORMAL_DISPLAY )
    fprintf(stream,"\n%s\ttype=\"%s\"",indent, displayTypeMsg (dlDisplay->displayType));
  fprintf(stream,"\n%s}",indent);
}

static void displayGetValues(ResourceBundle *pRCB, DlElement *dlElement)
{
  DlDisplay *dlDisplay;
  DisplayInfo *displayInfo;
  Boolean forcePosition;

  dlDisplay = dlElement->structure.display;
  displayInfo = currentDisplayInfo;

  updateElementObjectAttribute (pRCB, &dlDisplay->object);
  dm2kGetValues(pRCB,
    CLR_RC,             &(dlDisplay->clr),
    BCLR_RC,            &(dlDisplay->bclr),
    CMAP_RC,            &(dlDisplay->cmap),
    DISPLAY_TYPE_RC,    &(dlDisplay->displayType),
    -1);

  displayInfo->drawingAreaBackgroundColor = globalResourceBundle.bclr;
  displayInfo->drawingAreaForegroundColor = globalResourceBundle.clr;
  displayInfo->xPosition = dlDisplay->object.x;
  displayInfo->yPosition = dlDisplay->object.y;

  /* rebuild the shell ? */
  forcePosition = False;
  if ( (displayInfo->displayType != dlDisplay->displayType) ) {
    if ( testRebuildDisplayShell (displayInfo, dlDisplay->displayType) ) {
      /* re-create the shell if needed */
/*
      XtUnmanageChild (displayInfo->shell);
*/
      cleanDisplayModeEdit ();
      dmTraverseDisplayList (displayInfo);
      XFlush(display);
      caPendEvent ("displayGetValues");
      XtRealizeWidget(displayInfo->shell);
      if ( displayInfo->dialog ) XtManageChild (displayInfo->dialog);
      else XtPopup (displayInfo->shell, XtGrabNone);
      forcePosition = True;
    }
    else if ( displayInfo->dialog ) {
      /* just change the modal / modeless type */
      displayInfo->displayType = dlDisplay->displayType;
      XtVaSetValues (displayInfo->dialog,
		     XmNdialogStyle, getDialogStyle (displayInfo),
		     NULL);
    }
  }

  positionDisplay (displayInfo, forcePosition);

  /* and resize the shell (drawing area) */
  XtVaSetValues(displayInfo->drawingArea,
		XmNwidth,dlDisplay->object.width,
		XmNheight,dlDisplay->object.height,
		NULL);
}


void displayObjectInfo (char *msg,
			Widget w,               /* ignored */
			DisplayInfo *displayInfo,
			DlElement *dlElement,
			XtPointer objet)        /* ignored */
{
  extern Boolean verboseFlag, debugFlag;
  DlImage *image;

  if ( displayInfo->displayType != NORMAL_DISPLAY ) {
    strcat (msg, (displayInfo->displayType == MODAL_DIALOG)
		 ? " : Application MODAL Dialog" : " : MODELESS Dialog");
  }
  if ( globalDm2kReadOnly ) strcat (msg, " (READ ONLY display)");
  if ( verboseFlag || debugFlag) {
    strcat (msg, "\n  Window Manager positioning policy IS ");
    strcat (msg, (displayInfo->wmPositionIsFrame) ? "FRAME" : "USER");
  }
  sprintf (&msg[STRLEN(msg)], "\n\nDisplay definition file :\n%s", displayInfo->dlFile->name);

  if ( dlElement->type != DL_Image ) return;

  image = dlElement->structure.image;
  if ( image->imageType != NO_IMAGE ) {
    sprintf (&msg[STRLEN(msg)], "\n\n    Background image file :\n%s",
	     image->imageName);
  }
}


/* Display shell window positionning routines */
/* ------------------------------------------ */

  /*  The position defined in the displayInfo is the position on the
   *  screen of the upper left corner of the decoration frame
   *  added by the Window Manager around the shell window.
   *
   *  This routine returns the position parameter values to be used
   *  to move the shell window.
   *
   *  NB : to work these routine assume that the XmNwaitForWm resource
   *       of the display Shell must be "true".
   */

static char * positionPolicyMsg (DisplayInfo *displayInfo)
{
  return ( displayInfo->wmPositionIsFrame ) ? "\"Position Is Frame\"" : "\"Position Is User Window\"";
}


static Boolean getWmDecorationOffset (DisplayInfo *displayInfo)
{
  Window       root, parent, window, *children, child;
  int          i;
  unsigned int nb;
  int          nx, ny;

  if (   (displayInfo->xWmDecoration > 0)
      && (displayInfo->yWmDecoration > 0) ) return (True);

  window = parent = XtWindow (displayInfo->shell);
  root = 0;
  for ( i = 0 ; ; i++, window = parent) {
    (void) XQueryTree (display, window, &root, &parent, &children, &nb);
    if ( children ) XFree (children);
    if ( parent == root ) break;
  }
  if ( i == 0 ) return (False);

  XTranslateCoordinates (display, XtWindow (displayInfo->shell), window,
			 0, 0, &nx, &ny, &child);
  displayInfo->xWmDecoration = nx;
  displayInfo->yWmDecoration = ny;
  return ( ((nx != 0) || (ny != 0)) );
}


void positionDisplayRead (DisplayInfo *displayInfo)
{
  /* only for debuging */
  extern Boolean debugFlag;
  Position x, y;
  Dimension w, h;

  if ( ! debugFlag ) return;

  w = h = -1;
  XtVaGetValues (displayInfo->shell, XmNx, &x, XmNy, &y, NULL);
  if ( displayInfo->drawingArea )
    XtVaGetValues (displayInfo->drawingArea, XmNwidth, &w, XmNheight, &h, NULL);
  printf("\n<== positionDisplayRead %x : x %d y %d (w %d h %d)\n",
	 (unsigned int)displayInfo, x, y, w, h);
}


static void checkXYDisplay (Position *x, Position *y)
{
  /* a x=0, y=0 give a random position by Window Manager */
  if ( (*x == 0) && (*y == 0) ) *x = 1;
}

void getXYDisplay (DisplayInfo *displayInfo, Position *x, Position *y)
{
  /*  This routine returns the position parameter values to be used
   *  to position the shell window.
   */

  /* assume position is frame */
  *x = displayInfo->xPosition;
  *y = displayInfo->yPosition;
  if ( !displayInfo->wmPositionIsFrame ) {
    /* position is user : user position = frame position + decoration */
    *x += displayInfo->xWmDecoration;
    *y += displayInfo->yWmDecoration;
  }
  checkXYDisplay (x, y);
}

void positionDisplay (DisplayInfo *displayInfo, Boolean forcePosition)
{
  /* position the display at the position defined in displayInfo */
#if 0 /* by VTR */

  extern Boolean motifWMRunning;
  extern Boolean debugFlag;
  Position x, y;
  Position oldx, oldy;
  char *strg;

  if ( debugFlag ) {
    strg =  positionPolicyMsg (displayInfo);
    printf("\n<== positionDisplay %x : upper left corner of decoration frame at\n     x %d y %d (force %d) in %s policy\n",
	   displayInfo, displayInfo->xPosition, displayInfo->yPosition, forcePosition, strg);
  }

  /* translate into WM position parameter values */
  getXYDisplay (displayInfo, &x, &y);

  /* Try to do the best positionning with any Window Manager */
  if ( forcePosition && !displayInfo->dialog && !motifWMRunning ) {
    /* seem to work with all window managers */
    XtVaSetValues(displayInfo->shell, XmNx, x+1, XmNy, y+1, NULL);
  }
  XtVaSetValues(displayInfo->shell, XmNx, x, XmNy, y, NULL);

  if ( ! motifWMRunning ) {
    /* with fv WM familly : no ConfigureNotify event is generated */
    /* try to re-evaluate the WM positionning policy */
    oldx = x;
    oldy = y;
    getShellPositionPolicy (displayInfo);
    getXYDisplay (displayInfo, &x, &y);
    if ( (x != oldx) || (y != oldy) ) {
      XtVaSetValues(displayInfo->shell, XmNx, x, XmNy, y, NULL);
      if ( debugFlag ) {
	printf("    Apply a new position x %d y %d (old %d %d)\n", x, y, oldx, oldy);
      }
    }
  }
#endif
}


static Boolean evaluateWMStrategy (DisplayInfo *displayInfo,
	 Position x, Position y,    /* upper left corner position of the shell in root coord */
	 char **strg1, char **strg2)
{
  /*
   * The WM positionning strategy is either :
   *  - Position is Frame : the parameter values of the position request
   *                        specify the position of the frame decoration
   *                        upper left corner
   *  - Position is User  : the parameter values of the position request
   *                        specify the position of the user window
   *                        upper left corner (decoration is above and
   *                        on right
   * For Motif window manager (mwm) familly the default policy is
   *   "Position is Frame", but the other one can be defined with
   *   the mwm resources : positionIsFrame  (ref : mwm man page)
   */
  extern Boolean debugFlag;
  extern Boolean positionPolicyFlag;     /* True if the policy is defined by option or environment variable */
  extern Boolean positionPolicyFrame;    /* policy is FRAME / USER if positionPolicyFlag is True */
  Boolean oldPolicy;
  Position rx, ry;
  int dx, dy;
  Boolean retval;

  oldPolicy = displayInfo->wmPositionIsFrame;

  /* get the used positionning paramter values */
  (void) getWmDecorationOffset (displayInfo);
  dx = displayInfo->xWmDecoration;
  dy = displayInfo->yWmDecoration;
  getXYDisplay (displayInfo, &rx, &ry);

  if ( positionPolicyFlag ) {
    /* user defined (by option para or environment variable) */
    displayInfo->wmPositionIsFrame = positionPolicyFrame;
    *strg1 = "User defined ";
    retval = True;
    if ( debugFlag ) {
      printf("\n<== evaluateWMStrategy : %s dx %d dy %d\n",
	     *strg1, dx, dy);
    }
  }

  else {
    /* evaluate the Window Manager positionning strategy */
    *strg1 = "";
    if ( (x == (rx + dx)) && (y == (ry + dy)) ) {
      /* Window manager position the user window */
      displayInfo->wmPositionIsFrame = False;
      if ( displayInfo->wmPositionIsFrame != oldPolicy ) *strg1 = "New ";
      retval = True;
    }
    else if ( (x == rx) && (y == ry) ) {
      /* Window manager position the frame (wm decoration window) */
      displayInfo->wmPositionIsFrame = True;
      if ( displayInfo->wmPositionIsFrame != oldPolicy ) *strg1 = "New ";
      retval = True;
    }
    else retval = False;  /* WM positionning policy not re-evaluated */

    if ( debugFlag ) {
      printf("\n<== evaluateWMStrategy :x %d rx %d dx %d  y %d ry %d dy %d\n",
	     x, rx, dx, y, ry, dy);
    }
  }

  *strg2 =  positionPolicyMsg (displayInfo);
  return (retval);
}


#if 0
static void getShellPositionPolicy (DisplayInfo *displayInfo)
{
  extern Boolean debugFlag;
  Position x, y;
  char *strg1, *strg2;

  if ( ! XtIsManaged (displayInfo->shell) ) return;
  if ( ! getWmDecorationOffset (displayInfo) ) return;   /* no WM decoration yet */

  /* get position of the upper left corner of the shell */
  XtTranslateCoords (displayInfo->shell, 0, 0, &x, &y);

  /* evaluate the positionning policy */
  if ( ! evaluateWMStrategy (displayInfo, x, y, &strg1, &strg2) ) {
    /* a new position was given by Widow Manager */
    strg1 = "Not re-evaluated ";
  }

  if ( debugFlag ) {
    printf("\n<== getShellPositionPolicy %x : x %d (%d) y %d (%d)\n",
	   (int)displayInfo,
	   x, displayInfo->xWmDecoration,
	   y, displayInfo->yWmDecoration);
    printf("   requested : x %d  y %d\n", displayInfo->xPosition, displayInfo->yPosition);
    printf("   %sPolicy : %s\n", strg1, strg2);
  }
}
#endif

/*
 * ConfigureNotify event handler to be used for capture of new position
 * of the display (the shell) on the screen
 * and with mwm to evaluated the current positionning policy
 */

void handleShellStructureNotify (
  Widget widget,
  XtPointer callData,
  XEvent *event,
  Boolean *ctd)
{
  extern Boolean debugFlag;
  extern Boolean motifWMRunning;
  DisplayInfo *displayInfo;
  XConfigureEvent *cfgEvent;
  int dx, dy;
  Position x, y;
  char *strg1, *strg2;

  if ( event->type != ConfigureNotify ) return;
  displayInfo = (DisplayInfo *) callData;
  cfgEvent = (XConfigureEvent *)event;

  if ( widget != displayInfo->shell ) return;

  if ( ! getWmDecorationOffset (displayInfo) ) {
    /* no decoration, or not yet applied */
    displayInfo->xPosition = cfgEvent->x;
    displayInfo->yPosition = cfgEvent->y;
    return;
  }

  dx = displayInfo->xWmDecoration;
  dy = displayInfo->yWmDecoration;

  /* x, y position in root coordinate of the shell window */
  x = (Position) cfgEvent->x;
  y = (Position) cfgEvent->y;

  strg1 = "New position by WM with ";

  if ( motifWMRunning ) {
    /* evaluate the WM positionning policy */
    if ( ! evaluateWMStrategy (displayInfo, x, y, &strg1, &strg2) ) {
      /* A new position was provided by user interaction
       *   capture the new position defined by the Widow Manager
       */
      /* frame position = user window position - decoration */
      displayInfo->xPosition = cfgEvent->x - dx;
      displayInfo->yPosition = cfgEvent->y - dy;
    }
  }

  strg2 =  positionPolicyMsg (displayInfo);
  /* frame position = user window position - decoration */
  displayInfo->xPosition = cfgEvent->x - dx;
  displayInfo->yPosition = cfgEvent->y - dy;


  if ( debugFlag ) {
    printf("\n<== handleShellStructureNotify %x : x %d (%d) y %d (%d) w %d h %d\n",
	   (int)displayInfo, cfgEvent->x, dx, cfgEvent->y, dy,
	   cfgEvent->width, cfgEvent->height);
    printf("   display position : x %d  y %d\n", displayInfo->xPosition, displayInfo->yPosition);
    printf("   %sPolicy : %s\n", strg1, strg2);
  }
}


/* Help facility about Mouse Button usage in display */
/* ------------------------------------------------- */

static Widget mouseButtonHelpWidget        = NULL;
static DlTraversalMode saved_traversalMode = (DlTraversalMode)-1;
static ActionType saved_actionType         = (ActionType)-1;

static void mouseButtonHelpMsg (Boolean manageFlag)
{
  char msg[128], **tmp;
  XmString msgXmString, xtmp1, xtmp2;
  Position x, y;

  if ( !mouseButtonHelpWidget ) {
    saved_traversalMode = (DlTraversalMode)-1;
    saved_actionType = (ActionType)-1;
    return;
  }

  if ( !manageFlag && !XtIsManaged (mouseButtonHelpWidget) ) {
    saved_traversalMode = (DlTraversalMode)-1;
    saved_actionType = (ActionType)-1;
    return;
  }

  if (   (globalDisplayListTraversalMode != saved_traversalMode)
      || (currentActionType != saved_actionType) ) {
    memset (msg, '\0', sizeof(msg));
    tmp = mouseButtonUsageMsg (globalDisplayListTraversalMode,
			       currentActionType,
			       msg, sizeof(msg));
    saved_traversalMode = globalDisplayListTraversalMode;
    saved_actionType = currentActionType;
    xtmp1 = XmStringCreateLtoR (msg, XmFONTLIST_DEFAULT_TAG);
    for ( ; *tmp ; tmp++ ) {
      xtmp2 = XmStringCreateLtoR (*tmp, XmFONTLIST_DEFAULT_TAG);
      msgXmString = XmStringConcat (xtmp1, xtmp2);
      XmStringFree (xtmp2);
      XmStringFree (xtmp1);
      xtmp1 = msgXmString;
    }
    XtVaSetValues (mouseButtonHelpWidget, XmNmessageString, msgXmString, NULL);
    XmStringFree (msgXmString);
  }

  if ( ! XtIsManaged (mouseButtonHelpWidget) ) {
    XtManageChild (mouseButtonHelpWidget);
    if ( currentDisplayInfo ) {
      x = currentDisplayInfo->xPosition +20;
      y = currentDisplayInfo->yPosition +20;
      XtVaSetValues(mouseButtonHelpWidget, XmNx, x, XmNy, y, NULL);
    }
  }
}


static void destroyHelpMBDialog (Widget w, XtPointer cd, XtPointer cbs)
{
  XtDestroyWidget(mouseButtonHelpWidget);
  mouseButtonHelpWidget = NULL;
  saved_traversalMode = (DlTraversalMode)-1;
  saved_actionType = (ActionType)-1;
}


void mouseButtonHelpCB (Widget w, XtPointer cd, XtPointer cbs)
{
  int    n;
  Arg    args[5];
  Widget parent = cd ? (Widget)cd : mainShell;

  /* let's reparent mouseButtonHelpWidget to other shell
   */
  if (mouseButtonHelpWidget)
    destroyHelpMBDialog(NULL,NULL,NULL);

  /* create the display objects information dialog box 
   */
  n = 0;
  XtSetArg (args[n], XmNdialogStyle, XmDIALOG_MODELESS); n++;
  
  mouseButtonHelpWidget = 
    XmCreateInformationDialog (parent, "helpMBDialog", args, n);
  
  XtUnmanageChild 
    (XmMessageBoxGetChild (mouseButtonHelpWidget, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild 
    (XmMessageBoxGetChild (mouseButtonHelpWidget, XmDIALOG_HELP_BUTTON));

  XtAddCallback(mouseButtonHelpWidget,XmNokCallback, destroyHelpMBDialog,
		&mouseButtonHelpWidget);

  XtVaSetValues (mouseButtonHelpWidget, 
		 XmNdialogType, XmDIALOG_MESSAGE,
		 NULL);

  mouseButtonHelpMsg (True);
}


void mouseButtonHelp ()
{
  mouseButtonHelpMsg (False);
}


void dumpElementList (DlList *dlList, char *msg)
{
  DlElement *dlElement, *dlpt;

  printf ("   dump of %s (%x) count %ld\n", msg, (int)dlList, dlList->count);
  dlElement = FirstDlElement(dlList);

  while (dlElement) {
    printf ("    element ");
    for ( dlpt = dlElement ; 
	  dlpt->type == DL_Element ; 
	  dlpt = dlpt->structure.element) 
      /*EMPTY*/;
    printf ("%x -> ", (int)dlpt);
    printf ("%x type %d %s\n", (int)dlpt->structure.element,
	    dlpt->type, elementStringTable[dlpt->type -MIN_DL_ELEMENT_TYPE]);
    dlElement = dlElement->next;
  }
}


void dumpDisplayObjects (DisplayInfo *displayInfo, char *msg)
{
  if ( !displayInfo ) return;

  printf ("\nDump Display structure for %x\n", (int)displayInfo);
  if ( msg ) printf ("    called from : %s\n", msg);
  dumpElementList (displayInfo->dlElementList, "dlElementList");
  dumpElementList (displayInfo->selectedDlElementList, "selectedDlElementList");
  printf ("  selectedDlElementList is Highlighted : %s\n",
	  (displayInfo->selectedElementsAreHighlighted) ? "True" : "False");
}
