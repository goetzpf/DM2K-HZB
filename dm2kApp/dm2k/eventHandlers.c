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
 * .02  09-11-95        vong    conform to c++ syntax
 *
 *****************************************************************************
 *
 *      24-02-97    Fabien  call createPopupDisplayMenu from dm2k.c
 *                          and correction inthe popup menu management.
 *                          Capture display position with MB2.
 *
 *****************************************************************************
*/

#include "dm2k.h"
#include <X11/IntrinsicP.h>

#define X_MENU_SHIFT 20
#define Y_MENU_SHIFT 10

extern Widget resourceMW, resourceS;
extern Widget objectPaletteSelectToggleButton;
extern XButtonPressedEvent lastEvent;

typedef struct _Button3UtilStruct {
  DlElement      * element;
  DlElement      * compElement;
  DisplayInfo    * displayInfo;
  XtCallbackProc   callProc;
}Button3UtilStruct;

/* T. Straumann:	added a resource to give the user
 *					control over auto-raising of the display area
 *					by the event handlers.
 */
typedef struct ehResourcesRec_ {
	Boolean enableAutoRaise;
} ehResourcesRec;

static ehResourcesRec ehResources;

static XtResource ehXtResources[]={
	{ "enableEventHandlerAutoRaise", "EnableEventHandlerAutoRaise",
	  XtRBoolean, sizeof(Boolean),
	  XtOffsetOf(ehResourcesRec, enableAutoRaise),
	  XtRImmediate, (XtPointer) True },
};

void toggleSelectedElementHighlight(DlElement *element);

static DlElement * getCompositeElement(
	DlElement * element,
	int         x,
	int         y);

static DlElement *handleRectangularCreates(
	DlElementType elementType,
	int x,
	int y,
	unsigned int width,
	unsigned int height);


static DlList *tmpDlElementList = NULL;

int initEventHandlers() 
{
  if (tmpDlElementList) 
    return 0;

  /* T. Straumann:	load app resources */
  XtVaGetApplicationResources(mainShell,&ehResources,
							ehXtResources,XtNumber(ehXtResources),
							NULL);

  if ((tmpDlElementList = createDlList())) {
    return 0;
  } else {
    return -1;
  }
}


#ifdef __cplusplus
static XtPointer findObject (
  DlElement *element,
  DisplayInfo *displayInfo,
  Widget)
#else
static XtPointer findObject (
  DlElement *element,
  DisplayInfo *displayInfo,
  Widget w)
#endif
{

  switch (element->type) 
    {
    /* controllers */
    case DL_ChoiceButton  :
    case DL_Menu          :
    case DL_MessageButton :
    case DL_RelatedDisplay:
    case DL_ShellCommand  :
    case DL_TextEntry     :
    case DL_Valuator      :
      return ((XtPointer) element->structure.bar->object.runtimeDescriptor);

    /* monitors */
    case DL_Bar           :
    case DL_Byte          :
    case DL_CartesianPlot :
    case DL_Indicator     :
    case DL_Meter         :
    case DL_StripChart    :
    case DL_TextUpdate    :
      return ((XtPointer) element->structure.bar->object.runtimeDescriptor);

    /* primitives */
    case DL_Arc           :
    case DL_Image         :
    case DL_Line          :
    case DL_Oval          :
    case DL_Polygon       :
    case DL_Polyline      :
    case DL_Rectangle     :
    case DL_Text          :
      return ((XtPointer) displayInfo);
    default:
       break;
    }

  return (NULL);
}

static void ValuatorButton3Process(DisplayInfo * displayInfo,
				   DlElement   * element,
				   XEvent      * event)
{
  popupValuatorKeyboardEntry(element->widget, displayInfo, event);
  XUngrabPointer(display,CurrentTime);
}

static void ValuatorButton3Cb (Widget w, XtPointer cb, XtPointer cbs)
{
  Button3UtilStruct          * utilStruct = (Button3UtilStruct *) cb;
  XmPushButtonCallbackStruct * pushStruct = (XmPushButtonCallbackStruct *) cbs;
  DisplayInfo                * displayInfo;
  DlElement                  * element;

  if (utilStruct) {
    displayInfo = utilStruct->displayInfo;
    element     = utilStruct->element;
  }
  else {
    INFORM_INTERNAL_ERROR(); 
    return;
  }

  ValuatorButton3Process (displayInfo, element, pushStruct->event);
}


static void CartesianPlotButton3Process(DisplayInfo * displayInfo,
					DlElement   * element)
{
  /* update globalResourceBundle with this element's info 
   */
  executeTimeCartesianPlotWidget = element->widget;
  updateGlobalResourceBundleFromElement(element);
  if (cartesianPlotAxisS) {
    XtSetSensitive(cartesianPlotAxisS,True);
  } else {
    cartesianPlotAxisS = createCartesianPlotAxisDialog (mainShell);
  }
  
  /* update cartesian plot axis data from globalResourceBundle 
   */
  updateCartesianPlotAxisDialogFromWidget(element->widget);
  XtManageChild(cpAxisForm);
  XtPopup(cartesianPlotAxisS,XtGrabNone);
  
  XUngrabPointer(display,CurrentTime);
}

static void CartesianPlotButton3Cb (Widget w, XtPointer cb, XtPointer cbs)
{
  Button3UtilStruct * utilStruct = (Button3UtilStruct *) cb;
  DisplayInfo       * displayInfo;
  DlElement         * element;

  if (utilStruct) {
    displayInfo = utilStruct->displayInfo;
    element     = utilStruct->element;
  }
  else {
    INFORM_INTERNAL_ERROR(); 
    return;
  }

  CartesianPlotButton3Process(displayInfo, element);
}


/*ARGSUSED*/
static void PopdownAssociatedMenuCB(Widget    widget, 
				    XtPointer clientData, 
				    XtPointer callData)
{
  XtDestroyWidget(widget);
}


/*
 * active callback of associated menu;
 * `widget' is PushButton widget
 */
static void 
_associatedMenuCB(Widget widget, XtPointer clientData, XtPointer callData)
{
  int                   buttonNumber = (int) clientData;
  DlComposite         * dlComposite;
  AssociatedMenuItem  * ami;
  int                   i, len;
  Button3UtilStruct   * utilStruct;

  XtVaGetValues (XtParent(widget), XmNuserData, &utilStruct, NULL);

  if (buttonNumber == 0 && utilStruct->callProc) 
    {
      /* some additional item in a top of menu
       */
      (*utilStruct->callProc)(widget, utilStruct, callData);
      free((char*)utilStruct);
      return;
    }

  if (utilStruct->callProc)
    buttonNumber --;

  dlComposite = utilStruct->compElement->structure.composite;
  
  for (i = 0, ami = dlComposite->ami; 
       i < buttonNumber && ami; 
       i++, ami = ami->next)
    /*EMPTY*/;
  
  if (ami) {
    switch (ami->actionType)
      {
      case AMI_NEWDISPLAY:
      case AMI_NEWDISPLAY_REPLACE :
	createNewDisplay (currentDisplayInfo, 
			  ami->command,
			  ami->args,
			  (ami->actionType == AMI_NEWDISPLAY_REPLACE));
	break;

      case AMI_SYSTEM_SCRIPT :
	if (ami->command && (len = STRLEN(ami->command)) > 0) 
	{
	  char *command;

	  /*
	   * build system command with parameters and `&' in the end
	   */
	  command = (char*) malloc (len + 1 + (ami->args?STRLEN(ami->args):0) +2);

	  /* T. Straumann: protect against null pointer */
	  strcpy (command, ami->command ? ami->command : "");
	  strcat (command, " ");

	  if (ami->args)
	    strcat (command, ami->args);

	  strcat (command, "&");

	  /*
	   * start system script and free memory
	   */
	  system(command);
	  free (command);
	}

	break;
	
      default :
	INFORM_INTERNAL_ERROR();
	break;
      }
  }
  else 
    INFORM_INTERNAL_ERROR();
}


static void AssociatedMenuButton3Process
    (DisplayInfo       * displayInfo,
     DlElement         * element,
     Button3UtilStruct * utilStruct,
     char              * utilLabel,
     XEvent            * event)
{
  DlComposite        * dlComposite = element->structure.composite;
  AssociatedMenuItem * ami;
  XmString           * buttons;
  XmButtonType       * buttonType;
  int                  buttonCount;
  Arg                  args[20];
  int                  i, n;
  Widget               menu;

  if (utilStruct->callProc)
    buttonCount = 2;
  else
    buttonCount = 0;

  /* how much items will be in menu?
   */
  for (ami = dlComposite->ami;
       ami != NULL; 
       ami = ami->next) 
    {
      buttonCount++;
    }
  
  /* make buttons and buttonType arrays ready
   */
  buttons    = (XmString *)     calloc (buttonCount, sizeof(XmString));
  buttonType = (XmButtonType *) calloc (buttonCount, sizeof(XmButtonType));
  
  if (buttons == NULL || buttonType == NULL) {
    if (buttons)    free ((char*)buttons);
    if (buttonType) free ((char*)buttonType);
    return;
  }

  /* let's construct resources for ``Simple'' menu creating routine
   */
  i = 0;

  if (utilStruct->callProc) {
    buttons[0]    = XmStringCreateSimple(utilLabel);
    buttonType[0] = XmPUSHBUTTON;
    buttons[1]    = XmStringCreateSimple("");
    buttonType[1] = XmSEPARATOR;
    i = 2;
  }
  
  for (ami = dlComposite->ami; 
       ami != NULL; 
       i++, ami = ami->next)
    {
      buttons[i]    = XmStringCreateSimple(ami->label);
      buttonType[i] = XmPUSHBUTTON;
    }
  
  /* bring menu on screen
   */
  n = 0;
  XtSetArg(args[n], XmNbuttonCount,    buttonCount); n++;
  XtSetArg(args[n], XmNbuttonType,     buttonType); n++;
  XtSetArg(args[n], XmNbuttons,        buttons); n++;
  XtSetArg(args[n], XmNsimpleCallback, _associatedMenuCB); n++;
  XtSetArg(args[n], XmNuserData,       utilStruct); n++;
  XtSetArg(args[n], XmNtearOffModel,   XmTEAR_OFF_DISABLED); n++;
  menu = XmCreateSimplePopupMenu(displayInfo->drawingArea, 
				 "associatedMenu",
				 args,
				 n);

  XtAddCallback(XtParent(menu), XtNpopdownCallback, 
		PopdownAssociatedMenuCB, (XtPointer)utilStruct);

  XtAddCallback(menu, XtNdestroyCallback, freeUserDataCB, NULL);

  /* let's shift menu such way, so cursor would be just on first item of
   * the menu, this allows such think as a press&realease mouse action will
   * invoke the first button in the menu
   */
  if (((XButtonPressedEvent *)event)->x_root >= X_MENU_SHIFT)
    ((XButtonPressedEvent *)event)->x_root -= X_MENU_SHIFT;
  if (((XButtonPressedEvent *)event)->y_root >= Y_MENU_SHIFT)
    ((XButtonPressedEvent *)event)->y_root -= Y_MENU_SHIFT;


  popup (menu, event);
  
  /* free all alocated resources
   */
  for (i = 0; i < buttonCount; i++) 
    XmStringFree (buttons[i]);
  
  free ((char*)buttons);
  free ((char*)buttonType);
}

/*
 * event handlers
 */

/*
 * T. Straumann: There's a subtle problem with this kind of 'homemade'
 *				 popup(). The menu is a subclass of OverrideShell and
 *				 hence ignored by the WM.
 *				 The drawing area however is a child of a 'normal' shell
 *				 (i.e. a shell managed by the WM).
 *				 Before calling 'popup()' there were several XRaiseWindow()
 *				 calls done in order to raise the drawing area and now
 *				 we wish to raise the menu on top of everything.
 *				 Note that there are WMs that do their own stuff when they
 *				 get aware that the drawing area was raised (maybe they
 *				 raise their decoration window). Since this may
 *				 take some time, the menu might become obscured again!!
 *
 *				 A resource was added that allows the user to switch
 *				 off auto-raising of windows managed by the WM (i.e. WM shells)
 *				 by the event handlers. Raising windows / passing focus should
 *				 be the task of the WM anyway and the application should use
 *				 some ICCM protocol to talk to the WM.
 */

void popup (Widget popupMenu, XEvent *event)
{
/* T. Straumann: we may raise the menu because it is NOT managed by the WM */
  XmMenuPosition (popupMenu, (XButtonPressedEvent *)event);
  XtManageChild(popupMenu);
  XRaiseWindow (XtDisplay(popupMenu), XtWindow(XtParent(popupMenu)));
}


/*ARGSUSED*/
#ifdef __cplusplus
void popupMenu(
  Widget,
  XtPointer cd,
  XEvent *event,
  Boolean *)
#else
void popupMenu(
  Widget w,
  XtPointer cd,
  XEvent *event,
  Boolean *ctd)
#endif
{
  DisplayInfo       * displayInfo = (DisplayInfo *) cd;
  XButtonEvent      * xEvent = (XButtonEvent *)event;
  Widget              widget;
  DlElement         * element;
  DlElement         * compElement;
  Button3UtilStruct * utilStruct = NULL;
  XtCallbackProc      callProc = NULL;
  char              * label = NULL;
  Boolean             sensitive;

  displayInfo->dlPopupElement = NULL;

  if (displayInfo != currentDisplayInfo) {
    currentDisplayInfo = displayInfo;
    enableControllerRC (currentDisplayInfo);
    currentColormap = currentDisplayInfo->colormap;
    currentColormapSize = currentDisplayInfo->dlColormapSize;
  }

  /* and always make sure the window is on top and has input focus */
  /* T. Straumann:	Don't raise shells managed by the WM
   *				(consult comment for 'popup()'
   */
  if ( ehResources.enableAutoRaise || ! XtIsWMShell(displayInfo->shell) ) {
  	XRaiseWindow(display,XtWindow(displayInfo->shell));
  	XSetInputFocus(display,XtWindow(displayInfo->shell),
		 RevertToParent,CurrentTime);
  }

  /* MB3 = Menu */
  if (xEvent->button == Button3) {
    if (globalDisplayListTraversalMode == DL_EDIT) 
      {
	if ( ! currentDisplayInfo->editPopupMenu ) {
	   createPopupDisplayMenu (globalDisplayListTraversalMode,
				   displayInfo,
				   displayInfo->drawingArea);
	}
	
	/* 
	 * edit menu doesn't have valid/unique displayInfo ptr, 
	 * hence use current 
	 */
	lastEvent = *((XButtonPressedEvent *)event);
	enableControllerMenu (displayInfo);
	popup (currentDisplayInfo->editPopupMenu, event);
      } 
    else if (globalDisplayListTraversalMode == DL_EXECUTE)
      {
	if ( ! currentDisplayInfo->executePopupMenu )
	      createPopupDisplayMenu (globalDisplayListTraversalMode,
				      displayInfo, displayInfo->drawingArea);
	/*
	 * in EXECUTE mode, MB3 can also mean things based on where it occurs,
	 *   hence, lookup to see if MB3 occured in an object that cares
	 */

	element = lookupElement(displayInfo->dlElementList,
				(Position)xEvent->x, (Position)xEvent->y,
				False);

	if (xEvent->state != ControlMask) 
	  {
	    compElement = 
	      getCompositeElement (displayInfo->dlElementList->head,
				   (int)xEvent->x, (int)xEvent->y);
	
	    if (element == compElement)
	      element = NULL;

	    if (!(compElement 
		  && compElement->type == DL_Composite
		  && compElement->structure.composite->ami != NULL))
	      compElement = NULL;
	  
	    if (element) 
	      {
		displayInfo->dlPopupElement = element;
		displayInfo->pointedObject = NULL;
		sensitive = False;
		switch(element->type) 
		  {
		  case DL_Valuator:
		    if (compElement) {
		      callProc = ValuatorButton3Cb;
		      label = "Valuator dialog";
		    } else {
		      ValuatorButton3Process (displayInfo, element, event);
		      return;
		    }

		    break;
	      
		  case DL_CartesianPlot:
		    if (compElement) {
		      callProc = CartesianPlotButton3Cb;
		      label = "CartesianPlot dialog";
		    } else {
		      CartesianPlotButton3Process(displayInfo, element);
		      return;
		    }

		    break;
	      
		  default:
		    /* if element is monitor let's show default display with
		     * macro substitution
		     */
		    if (ELEMENT_IS_MONITOR(element->type)) {
		      char * defName = 
			getenv(EPICS_DM2K_DEFAULT_MB3_DISPLAY_ENV);
		
		      if (defName) {
			char args[256];
		  
			sprintf(args,"record=%s", 
				element->structure.textUpdate->monitor.rdbk);
			createNewDisplay(displayInfo, defName, args, 
					 (Boolean)False);

			XUngrabPointer(display,CurrentTime);
			return;
		      }
		    }
		    break;
		  }
	      }
	
	    if (compElement) {
	      utilStruct = DM2KALLOC (Button3UtilStruct);
	  
	      if (utilStruct == NULL) {
		fprintf(stderr,"memory allocation error, %s:%d\n", 
			__FILE__,__LINE__);
		return;
	      } else {
		utilStruct->displayInfo = displayInfo;
		utilStruct->element     = element;
		utilStruct->compElement = compElement;
		utilStruct->callProc    = callProc;
	      }

	      AssociatedMenuButton3Process(displayInfo, compElement,
					   utilStruct, label, event);
	      return;
	    }

	    /* execute menu does have valid/unique displayInfo ptr, 
	     * hence use it 
	     */
	    XtSetSensitive (displayInfo->executePropertyButton, sensitive);
	    popup (displayInfo->executePopupMenu, event);
	  }
	else 
	  {
	    sensitive = False;

	    if (element != NULL)
	      {
		displayInfo->dlPopupElement = element;
		displayInfo->pointedObject = NULL;

		if ( objectInfoImplemented (element->type) ) {
		  switch (element->type) {
		  case DL_Display :
		  case DL_Image :
		  case DL_RelatedDisplay :
		  case DL_ShellCommand :
		    sensitive = True;
		    displayInfo->pointedObject = (XtPointer) displayInfo;
		    break;
		  default :
		    widget = element->widget;
		    displayInfo->pointedObject = 
		      findObject (element, displayInfo, widget);
		  }
		}
		sensitive=((displayInfo->pointedObject != NULL) || sensitive );
	      }
	    
	    XtSetSensitive (displayInfo->executePropertyButton, sensitive);
	    popup (displayInfo->executePopupMenu, event);
	  }
      }
  }
  else if (xEvent->button == Button1 &&
	   globalDisplayListTraversalMode == DL_EXECUTE) 
    {
      element = lookupElement(displayInfo->dlElementList,
			      (Position)xEvent->x, 
			      (Position)xEvent->y,
			      False);
      if (element != NULL)
	{
	  if (element->type == DL_RelatedDisplay &&
	      element->structure.relatedDisplay->visual == RD_HIDDEN_BTN) {
	    if ( element->actif ) {
	      relatedDisplayCreateNewDisplay(displayInfo,
			  &(element->structure.relatedDisplay->display[0]));
	    }
	  }
	}
    }
}

#ifdef __cplusplus
void popdownMenu(
  Widget,
  XtPointer cd,
  XEvent *event,
  Boolean *)
#else
void popdownMenu(
  Widget w,
  XtPointer cd,
  XEvent *event,
  Boolean *ctd)
#endif
{
  DisplayInfo *displayInfo = (DisplayInfo *) cd;
  XButtonEvent *xEvent = (XButtonEvent *)event;

  /* MB3 = Menu */
  if (xEvent->button == Button3) {
     if (globalDisplayListTraversalMode == DL_EDIT) {
	/* edit menu doesn't have valid/unique displayInfo ptr, hence use current */
	XtUnmanageChild(currentDisplayInfo->editPopupMenu);
     } else {
	/* execute menu does have valid/unique displayInfo ptr, hence use it */
	XtUnmanageChild(displayInfo->executePopupMenu);
     }
  }
}



#ifdef __cplusplus
void handleEnterWindow(
  Widget,
  XtPointer cd,
  XEvent *,
  Boolean *)
#else
void handleEnterWindow(
  Widget w,
  XtPointer cd,
  XEvent *event,
  Boolean *ctd)
#endif
{
  pointerInDisplayInfo = (DisplayInfo *) cd;
}


#ifdef __cplusplus
void handleEnterObject (Widget w, XtPointer cd, XEvent *, Boolean *)
#else
void handleEnterObject (Widget w, XtPointer cd, XEvent *event, Boolean *ctd)
#endif
{
  DlElement *dlElement;
  char msg[256];

  memset (msg, '\0', sizeof(msg));
  dlElement = (DlElement *) cd;
  if ( dlElement->run->info )
    (*dlElement->run->info) (msg, w, NULL, dlElement, NULL);
  else
    genericObjectInfo (msg, w, NULL, dlElement, NULL);
  updateLabelObjectINF (dlElement, msg);
}


#ifdef __cplusplus
void handleLeaveObject (Widget w, XtPointer , XEvent *, Boolean *)
#else
void handleLeaveObject (Widget w, XtPointer cd, XEvent *event, Boolean *ctd)
#endif
{
  displayLeaveWindow (w);
}


static void highlightElements () 
{
  /* highlight currently selected elements */
  highlightSelectedElements();

  /* if only one selected, use first one (only) in list */
  if (currentDisplayInfo->selectedDlElementList->count ==1 ) {
    updateGlobalResourceBundleAndResourcePalette (True);
    setResourcePaletteEntriesIfVisible
      (FirstDlElement(currentDisplayInfo->selectedDlElementList)->structure.element);
  }
}


#ifdef __cplusplus
void handleButtonPress(
  Widget w,
  XtPointer clientData,
  XEvent *event,
  Boolean *continueToDispatch)
#else
void handleButtonPress(
  Widget w,
  XtPointer clientData,
  XEvent *event,
  Boolean *continueToDispatch)
#endif
{
  DisplayInfo *displayInfo = (DisplayInfo *) clientData;
  XButtonEvent *xEvent = (XButtonEvent *)event;
  Position x0, y0, x1, y1;
  Dimension daWidth, daHeight;
  Boolean validDrag, validResize;
  int minSize;
  XEvent newEvent;
  Boolean foundVertex = False;
  int newEventType;
  DisplayInfo *di;


  /* if in execute mode - update currentDisplayInfo and simply return */
  if (globalDisplayListTraversalMode == DL_EXECUTE) {
     currentDisplayInfo = displayInfo;
     return;
  }

  /* loop over all displays, unhighlight and unselect previously
   * selected elements
   */

  di = displayInfoListHead;
  while (di) {
    if (di != displayInfo) {
      currentDisplayInfo = di;
      unhighlightSelectedElements();
      destroyDlDisplayList(di->selectedDlElementList);
    }
    di = di->next;
  }

  currentDisplayInfo = displayInfo;
  enableControllerRC (currentDisplayInfo);
  currentColormap = currentDisplayInfo->colormap;
  currentColormapSize = currentDisplayInfo->dlColormapSize;

  /* and always make sure the window is on top and has input focus */
  /* T. Straumann:	Don't raise shells managed by the WM
   *				(consult comment for 'popup()'
   */
  if ( ehResources.enableAutoRaise || ! XtIsWMShell(displayInfo->shell) ) {
  	XRaiseWindow(display,XtWindow(displayInfo->shell));
  	XSetInputFocus(display,XtWindow(displayInfo->shell),
                 RevertToParent,CurrentTime);
  }

  x0 = event->xbutton.x;
  y0 = event->xbutton.y;
  if (w != displayInfo->drawingArea) {
    Dimension dx0, dy0;
    /* other widget trapped the button press, therefore "normalize" */
    XtVaGetValues(w,XmNx,&dx0,XmNy,&dy0,NULL);
    x0 += dx0;
    y0 += dy0;
  }

  /*
   * handle differently, based on currentActionType
   */

  /* change drawingArea's cursor back to the appropriate cursor */
  drawingAreaDefineCursor (currentDisplayInfo);


  /**********************************
   ***       SELECT_ACTION        ***
   **********************************/

  if (currentActionType == SELECT_ACTION) 
  {
    /****************************************
     * MB1       =  select (rubberband)     *
     * shift-MB1 =  multi-select            *
     * MB2       =  adjust (drag)           *
     * ctrl-MB2  =  adjust( resize)         *
     * MB3       =  menu                    *
     ****************************************/

    switch (xEvent->button) {
      case Button1:
	if (xEvent->state & ShiftMask) 
	{
          /* Shift-MB1 = toggle and append selections 
	   */
          doRubberbanding(XtWindow(displayInfo->drawingArea),&x0,&y0,&x1,&y1);

          selectedElementsLookup(displayInfo->dlElementList,
				 x0,y0,x1,y1,tmpDlElementList,False);
	  
          if (!IsEmpty(tmpDlElementList))
	  {
            DlElement *pE = FirstDlElement(tmpDlElementList);

            unhighlightSelectedElements();

            while (pE) 
	    {
              DlElement * pT;
              int         found = False;

	      pT = FirstDlElement(displayInfo->selectedDlElementList);

              /* if found, remove it from the selected list 
	       */
              while (pT) {
                if (pT->structure.element == pE->structure.element) {
                  removeDlElement(displayInfo->selectedDlElementList,pT);
                  freeDlElement(pT);
                  found = True;
                  break;
                }
                pT = pT->next;
              }
	      
              /* if not found, add it to the selected list 
	       */
              if (!found) 
	      {
                DlElement *pF = pE;
                pE = pE->next;
                removeDlElement(tmpDlElementList,pF);
                appendDlElement(displayInfo->selectedDlElementList,pF);
              } else {
                pE = pE->next;
              }
            }
            highlightSelectedElements();
            destroyDlDisplayList(tmpDlElementList);
          }
        } 
	else {
	  Boolean searchForChildren;

	  if (xEvent->state & ControlMask) 
	    searchForChildren = True;
	  else
	    searchForChildren = False; 

	  /* handle the MB1 */
          /* see whether this is a vertex edit */
          if (NumberOfDlElement(displayInfo->selectedDlElementList) == 1) {
            DlElement *dlElement =
              FirstDlElement(displayInfo->selectedDlElementList)
                                            ->structure.element;
	    selectedElementsLookup(displayInfo->dlElementList,
				   x0,y0,x0,y0,tmpDlElementList,
				   searchForChildren);
            if ((tmpDlElementList->count > 1) &&
                (dlElement ==
                FirstDlElement(tmpDlElementList)->structure.element) &&
                (dlElement->run->editVertex)) {
              unhighlightSelectedElements();
              if (dlElement->run->editVertex(dlElement,x0,y0)) {
                foundVertex = True;
              }
              highlightSelectedElements();
            }
            destroyDlDisplayList(tmpDlElementList);
          }

	  /* if this is not a vertex edit */
          if (!foundVertex) {
            doRubberbanding(XtWindow(displayInfo->drawingArea),
			    &x0,&y0,&x1,&y1);
            selectedElementsLookup(displayInfo->dlElementList,
				   x0,y0,x1,y1,tmpDlElementList,
				   searchForChildren);
            if (!IsEmpty(tmpDlElementList)) {
              unhighlightSelectedElements();
              destroyDlDisplayList(displayInfo->selectedDlElementList);
              appendDlList(displayInfo->selectedDlElementList,
			   tmpDlElementList);
              highlightSelectedElements();
              destroyDlDisplayList(tmpDlElementList);
            }
          }
        }

	if (!foundVertex) {
          clearResourcePaletteEntries();
          if (NumberOfDlElement(currentDisplayInfo->selectedDlElementList)==1){
              currentElementType =
                  FirstDlElement(currentDisplayInfo->selectedDlElementList)
                      ->structure.element->type;
            setResourcePaletteEntries();
          }
        }
        break;


      /**************************************************************/
      /**************************************************************/
      case Button2:
        if (xEvent->state & ShiftMask) {
          /***********************/
          /* Shift-MB2 = NOTHING */
          /***********************/
          break;
        }
	      selectedElementsLookup(displayInfo->dlElementList,
                    x0,y0,x0,y0,tmpDlElementList,False);
        if (IsEmpty(tmpDlElementList)) break;

        if (xEvent->state & ControlMask) {
          /*********************/
          /* Ctrl-MB2 = RESIZE */
          /*********************/
          if (alreadySelected(FirstDlElement(tmpDlElementList))) {
            /* element already selected - resize it and any others */
            /* (MDA) ?? separate resize of display here? */
            /* unhighlight currently selected elements */
            unhighlightSelectedElements();

            /* this element already selected: resize all selected elements */
            validResize = doResizing(XtWindow(displayInfo->drawingArea),
                              x0,y0,&x1,&y1);
            if (validResize) {
              updateResizedElements(x0,y0,x1,y1);
              if (currentDisplayInfo->hasBeenEditedButNotSaved == False) {
                dm2kMarkDisplayBeingEdited(currentDisplayInfo);
              }
            }

	    highlightElements ();
          } else {
            /* this element not already selected,
               deselect others and resize it */
            unhighlightSelectedElements();
            destroyDlDisplayList(currentDisplayInfo->selectedDlElementList);
            appendDlList(currentDisplayInfo->selectedDlElementList,
                         tmpDlElementList);
            validResize = doResizing(XtWindow(displayInfo->drawingArea),
                                     x0,y0,&x1,&y1);
            if (validResize) {
              updateResizedElements(x0,y0,x1,y1);
              if (currentDisplayInfo->hasBeenEditedButNotSaved == False) {
                dm2kMarkDisplayBeingEdited(currentDisplayInfo);
              }
	    }
            clearResourcePaletteEntries();
	    highlightElements ();
          }
        } else {
	  /************************************/
	  /* MB2 = ADJUST (MOVE)              */
	  /*     for a display object :       */
	  /*     capture the current position */
	  /************************************/
          XtVaGetValues(displayInfo->drawingArea,
              XmNwidth,&daWidth,
              XmNheight,&daHeight,
              NULL);
#if 0
          printf("\nTemp. element list :\n");
          dumpDlElementList(tmpDlElementList);
#endif
          if (alreadySelected(FirstDlElement(tmpDlElementList))) {
            /* element already selected - move it and any others */
            /* unhighlight currently selected elements */
	    /*   display are never seen at this point */
            unhighlightSelectedElements();
            /* this element already selected: move all selected elements */
            validDrag = doDragging(XtWindow(displayInfo->drawingArea),
                                daWidth,daHeight,x0,y0,&x1,&y1);
            if (validDrag) {
              updateDraggedElements(x0,y0,x1,y1);
              if (currentDisplayInfo->hasBeenEditedButNotSaved == False) {
                dm2kMarkDisplayBeingEdited(currentDisplayInfo);
              }
            }
	    highlightElements ();
          } else {
	    Boolean updateDisplayFlg;

	    /* is the display already selected ? */
	    updateDisplayFlg = 
	      displayAlreadySelected (FirstDlElement (tmpDlElementList));

            /* this element not already selected,
               deselect others and move it */
            unhighlightSelectedElements();
            destroyDlDisplayList(currentDisplayInfo->selectedDlElementList);
            appendDlList(currentDisplayInfo->selectedDlElementList,
                         tmpDlElementList);

	    validDrag = doDragging(XtWindow(displayInfo->drawingArea),
				   daWidth,daHeight,x0,y0,&x1,&y1);
	    if (validDrag) {
	      updateDraggedElements(x0,y0,x1,y1);
	      if (currentDisplayInfo->hasBeenEditedButNotSaved == False) {
		dm2kMarkDisplayBeingEdited(currentDisplayInfo);
	      }
	    }

	    if ( updateDisplayFlg && (currentElementType == DL_Display) ) {
	      /* display already selected and palette resource set */
	      updateGlobalResourceBundleDisplayPosition ();
	      if (currentDisplayInfo->hasBeenEditedButNotSaved == False) {
		dm2kMarkDisplayBeingEdited(currentDisplayInfo);
	      }
	    }
	    else clearResourcePaletteEntries();

	    highlightElements ();
          }
        }
        destroyDlDisplayList(tmpDlElementList);
        break;

      case Button3:
        /************************************************************/
        /* MB3 = MENU  --- this is really handled in                */
        /*                 popupMenu/popdownMenu handler            */
        /************************************************************/
	popupMenu(w, clientData, event, continueToDispatch);
        break;
    }
  } 
  else if (currentActionType == CREATE_ACTION) 
  {
    /**********************************
     ***	CREATE_ACTION ***
     **********************************/
    DlElement *dlElement = 0;

    /************************************************
     * MB1       =  create (rubberband)	   	    *
     * MB2       =  adjust (drag) {as above}	    *
     * ctrl-MB2  =  adjust( resize)		    *
     * MB3       =  menu			    *
     ************************************************/
    switch (xEvent->button) {
    case Button1:
      /*****************************/
      /* MB1 = CREATE (Rubberband) */
      /*****************************/
      /* see how to handle text -
         either rectangular-create or type-create */
      XWindowEvent(display,XtWindow(displayInfo->drawingArea),
		   ButtonReleaseMask|Button1MotionMask,&newEvent);
      newEventType = newEvent.type;
      XPutBackEvent(display,&newEvent);

      if (currentElementType == DL_Text &&
          newEventType == ButtonRelease) {
        dlElement = handleTextCreate(x0,y0);
      } 
      else if (currentElementType == DL_Polyline) {
	dlElement = handlePolylineCreate(x0,y0,(Boolean)False);
      } 
      else if (currentElementType == DL_Line) {
        dlElement = handlePolylineCreate(x0,y0,(Boolean)True);
      } 
      else if (currentElementType == DL_Polygon) {
        dlElement = handlePolygonCreate(x0,y0);
      } 
      else {
        /* everybody else has "rectangular" creates */
        doRubberbanding(XtWindow(displayInfo->drawingArea),
                        &x0,&y0,&x1,&y1);
        /* pick some semi-sensible size for widget type elements */
        if (ELEMENT_HAS_WIDGET(currentElementType))
          minSize = 12;
        else
          minSize = 2;
        /* actually create elements */
	if ( checkControllerObjectType (displayInfo, currentElementType, globalResourceBundle.messageButtonType) )
	  dlElement = handleRectangularCreates(currentElementType, x0, y0,
				MAX(minSize,x1 - x0),MAX(minSize,y1 - y0));
	else {
	  invalidObjectWarning (displayInfo, currentElementType);
	  dlElement = NULL;
	}
      }
      
      if (dlElement) 
      {
        DlElement *pSE = 0;

        if (currentDisplayInfo->hasBeenEditedButNotSaved == False) {
          dm2kMarkDisplayBeingEdited(currentDisplayInfo);
        }

        appendDlElement(currentDisplayInfo->dlElementList,dlElement);
        (*dlElement->run->execute)(currentDisplayInfo,dlElement);

        unhighlightSelectedElements();
        destroyDlDisplayList(displayInfo->selectedDlElementList);
        pSE = createDlElement(DL_Element,(XtPointer)dlElement,NULL);

        if (pSE) {
          appendDlElement(displayInfo->selectedDlElementList,pSE);
        }

        currentElementType =
          FirstDlElement(currentDisplayInfo->selectedDlElementList)
            ->structure.element->type;
        highlightSelectedElements();
      }

      break;
    case Button2:
      /****************/
      /* MB2 = ADJUST */
      /****************/
	    break;

    case Button3:
      /********************************************/
      /* MB3 = MENU  --- this is really           */
      /* handled in popupMenu/popdownMenu handler */
      /********************************************/
	    break;
    }
#if 0
    printf("\nselected element list :\n");
    dumpDlElementList(displayInfo->selectedDlElementList);
#endif
    /* now toggle back to SELECT_ACTION from CREATE_ACTION */
    resetPaletteButton ();
    setActionToSelect();
    clearResourcePaletteEntries();
    setResourcePaletteEntries();
  }
}


/* Help on mouse button usage */
/* -------------------------- */

static char editSelectMB1Msg[] =
"MB1\n"
"      Select an object (or objects) on the screen and\n"
"      highlight. The resource palette is updated to\n"
"      reflect the selected item's internal data.\n"
"      A drag operation under MB1 selects a group of\n"
"      objects on the screen (including those objects\n"
"      wholly bounded by the selection rectangle).\n"
"Ctrl-MB1\n"
"      The same as above, but it allows to select a children\n"
"      object within a group instead of Composite element.\n" 
"Shift-MB1\n"
"      Multiple-select.  A set of objects are selected \n"
"      for operations (such as grouping).\n"
"\n"
;

static char editCreateMB1Msg[] =
"MB1 (any modifier)\n"
"      An object of current type is created, starting at\n"
"      the origin of button press, of size determined by\n"
"      button release (a bounding rectangle is rubberbanded).\n"
"      The resource palette is updated to reflect the object's\n"
"      internal data.\n"
"\n"
;

static char editMB2Msg[] =
"MB2 (any modifier exepted Ctrl)\n"
"      Selected object(s) are moved while MB2 is depressed\n"
"      and deposited on button release. If no objects are\n"
"      currently selected, the object under the cursor when\n"
"      MB2 is depressed in made the current object for\n"
"      moving. To cancel the effect of the current move,\n"
"      the cursor may be dragged off the current display\n"
"      window and the button released. This cancels the\n"
"      effect of the move.\n"
"      If a DISPLAY is currently selected, the geometry\n"
"      (position on the screen and size) is sampled and\n"
"      registered for this display.\n"
;

static char editCtrlMB2Msg[] =
"Ctrl-MB2\n"
"      Selected object(s) are resized while MB2 is depressed.\n"
"      If no objects are currently selected, the object under\n"
"      the cursor when MB2 is depressed in made the current\n"
"      object for resizing. To cancel the effect of the current\n"
"      resize, the cursor may be dragged off the current display\n"
"      window and the button released. This cancels the\n"
"      effect of the resize.\n"
"      N.B. this mechanism performs ABSOLUTE resizing, in which\n"
"      all objects are extended in width and height by the\n"
"      magnitude of the x and y mouse motion. For PROPORTIONAL\n"
"      resizing in which selected objects resize consistently,\n"
"      the object must first be grouped.\n"
"\n"
;

static char editCtrlDefaultMsg[] =
"MB2 & MB3 (any modifier)\n"
"      Exit from Create mode and fall back into Select mode\n"
"\n"
;


static char editMB3Msg[] =
"MB3\n"
"      Popup applicable menus.\n"
"\n"
;

static char executeMB1Msg[] = "\n"
"MB1\n"
"      Object actions.\n"
"      Special case for VALUATOR / SCALE:\n"
"        - clicking MB1 increments or decrements of specified\n"
"          precision the valuator / scale object;\n"
"        - clicking Ctrl-MB1 increments or decrements 10x of\n"
"          specified precision the valuator / scale object.\n"
;

static char executeMB2Msg[] = "\n"
"MB2\n"
"      Drag-and-drop mechanism.\n"
"      Deposition of object process variable names onto other\n"
"      objects or programs.  For example, depressing MB2 on a\n"
"      controller (e.g., valuator) will retrieve the underlying\n"
"      channel's name, and allow it to be dropped onto other\n"
"      objects (e.g., text fields in the same or other Motif\n"
"      applications) or other programs (e.g., KM - the knob\n"
"      manager program, which for instance, can then assign\n"
"      that channel to the specified knob and initiate physical\n"
"      control).\n"
;

static char executeMB3Msg[] = "\n"
"MB3"
"      Popup the main display menu (with Print and Close\n"
"      options for the current display).\n"
"      Special case for elements of Valuator and CartesianPlot types.\n"
"      Changing of cursor's face indicates that an addition menu\n"
"      can be poped up in that display's place by MB3.\n"
"Ctrl-MB3\n"
"      Popup the main display menu (with Print, Close and\n"
"      Property options for the current display).\n"
;


static char *editSelectMsgs [] = {
  editSelectMB1Msg,
  editMB2Msg,
  editCtrlMB2Msg,
  editMB3Msg,
  NULL};

static char *editCreateMsgs [] = {
  editCreateMB1Msg,
  editCtrlDefaultMsg,
  NULL};

static char *executeMsgs [] = {
  executeMB1Msg,
  executeMB2Msg,
  executeMB3Msg,
  NULL};

char ** mouseButtonUsageMsg (DlTraversalMode traversalMode,
			     ActionType actionType,
			     char *vmsg, int vmsgsz)
{
  char **msgs;

  strcpy (vmsg, "Mouse Buttons usage ");
  if ( globalDisplayListTraversalMode == DL_EDIT ) {
    strcat (vmsg, "EDITING");
  }
  else {
    strcat (vmsg, "RUNNING");
  }
  strcat (vmsg, " displays");
  if ( traversalMode == DL_EDIT ) {
    if (actionType == CREATE_ACTION) {
      strcat (vmsg, " in CREATE mode");
      msgs = &editCreateMsgs[0];
    }
    else {
      strcat (vmsg, " in SELECT mode");
      msgs = &editSelectMsgs[0];
    }
  }
  else {
    msgs = &executeMsgs[0];
  }
  strcat (vmsg, "\n\n");
  vmsg[vmsgsz-1] = '\0';
  return (msgs);
}


void highlightSelectedElements()
{
  extern Boolean debugFlag;
  DlElement *dlElement;

  if (currentDisplayInfo == NULL
      || IsEmpty(currentDisplayInfo->selectedDlElementList)
      || currentDisplayInfo->selectedElementsAreHighlighted == True)
    return;

  currentDisplayInfo->selectedElementsAreHighlighted = True;
  dlElement = FirstDlElement(currentDisplayInfo->selectedDlElementList);
  while (dlElement) {
    toggleSelectedElementHighlight(dlElement->structure.element);
    dlElement = dlElement->next;
  }

  if ( debugFlag ) dumpDisplayObjects (currentDisplayInfo, "highlightSelectedElements");
}

/*
 *  function to unhighlight the currently highlighted (and  therfore
 *	selected) elements
 */
void unhighlightSelectedElements()
{
  extern Boolean debugFlag;
  DlElement *dlElement;

  if ( debugFlag )
    dumpDisplayObjects (currentDisplayInfo, "unhighlightSelectedElements");

  if (currentDisplayInfo == NULL
      || IsEmpty(currentDisplayInfo->selectedDlElementList)
      || currentDisplayInfo->selectedElementsAreHighlighted == False)
    return;

  currentDisplayInfo->selectedElementsAreHighlighted = False;

  dlElement = FirstDlElement(currentDisplayInfo->selectedDlElementList);
  while (dlElement) {
    toggleSelectedElementHighlight(dlElement->structure.element);
    dlElement = dlElement->next;
  }
}

/*
 *  function to determine if specified element is already in selected list
 *	and unhighlight and unselect if it is...
 *	RETURNS: Boolean - True if an element was found and unselected,
 *		False if none found...
 *	also - the TOTAL number of elements selected is returned in
 *		numSelected  argument
 */
Boolean unhighlightAndUnselectElement(
  DlElement *element,
  int *numSelected)
{
  DisplayInfo *cdi;
  DlElement *dlElement = 0;

  /*
   *	N.B.: need to highlight in both window and pixmap in case
   *	there are expose events...
   */

  /* simply return if no current display */
  if (!currentDisplayInfo) return False;
  cdi = currentDisplayInfo;
  if (!IsEmpty(cdi->selectedDlElementList)) {
    DlElement *pE = FirstDlElement(cdi->selectedDlElementList);
    while (pE && !dlElement) {
      if (pE->structure.element == element) dlElement = pE;
      pE = pE->next;
    }
  }

  /* element not found - no changes */
  if (!dlElement) {
    return (False);
  }

  /* undraw the highlight */
  toggleSelectedElementHighlight(dlElement->structure.element);

  removeDlElement(cdi->selectedDlElementList,dlElement);
  if (dlElement->run->destroy) 
    dlElement->run->destroy(dlElement);
  return(True);
}


/*
 * update (move) all currently selected elements and rerender
 */
void updateDraggedElements(Position x0, Position y0, Position x1, Position y1)
{
  int xOffset, yOffset;
  DisplayInfo *cdi;
  DlElement *pE;
  Display *display;
  GC gc;

  /* if no current display or selected elements array, simply return */
  if (!currentDisplayInfo) return;
  if (IsEmpty(currentDisplayInfo->selectedDlElementList)) return;

  cdi = currentDisplayInfo;
  display = XtDisplay(cdi->drawingArea);
  gc = cdi->gc;

  xOffset = x1 - x0;
  yOffset = y1 - y0;

  unhighlightSelectedElements();
  /* as usual, type in union unimportant as long as object is 1st thing...*/
  pE = FirstDlElement(currentDisplayInfo->selectedDlElementList);
  while (pE) {
    DlElement *dlElement = pE->structure.element;
    if (dlElement->run->move) 
      dlElement->run->move(dlElement,xOffset,yOffset);
    if (dlElement->widget) 
      dlElement->run->execute(currentDisplayInfo,dlElement);
    pE = pE->next;
  }
  dmTraverseNonWidgetsInDisplayList(currentDisplayInfo);
  highlightSelectedElements();
}

/*
 * update all currently selected elements and rerender
 *	note: elements are only resized if their size remains physical 
 *		
 */
void updateResizedElements(Position x0, Position y0, Position x1, Position y1)
{
  int xOffset, yOffset;
  DisplayInfo *cdi;
  DlElement *dlElement;

/* if no current display or selected elements array, simply return */
  if (!currentDisplayInfo) return;
  if (IsEmpty(currentDisplayInfo->selectedDlElementList)) return;

  cdi = currentDisplayInfo;

  xOffset = x1 - x0;
  yOffset = y1 - y0;

/*
 * Use expensive but reliable destroy-update-recreate sequence to get
 *	resizing right.  (XtResizeWidget mostly worked here, except for
 *	aggregate types like menu which had children which really defined
 *	it's parent's size.)
 * One additional advantage - this method guarantees WYSIWYG wrt fonts, etc
 */

  /* as usual, type in union unimportant as long as object is 1st thing...*/
  dlElement = FirstDlElement(currentDisplayInfo->selectedDlElementList);
  while (dlElement) {
    DlElement *pE = dlElement->structure.element;
    if (pE->run->scale) {
      pE->run->scale(pE,xOffset,yOffset);
    }
    if (pE->widget) {
      pE->run->execute(currentDisplayInfo,pE);
    }
    dlElement = dlElement->next;
  }
  dmTraverseNonWidgetsInDisplayList(currentDisplayInfo);
}





/* 
 * handle creates (based on filled in globalResourceBundle values and
 *	currentElementType);
 *
 */
static DlElement *handleRectangularCreates(
	DlElementType elementType,
	int x,
	int y,
	unsigned int width,
	unsigned int height)
{
  DlElement *element = (DlElement *) NULL;

  /* now create the actual element 
   */
  switch(elementType) 
  {
    case DL_Image:
      element = handleImageCreate();
      break;
      /* others are more straight-forward */

    case DL_Valuator:
      element = createDlValuator(NULL);
      break;
    case DL_ChoiceButton:
      element = createDlChoiceButton(NULL);
      break;
    case DL_MessageButton:
      element = createDlMessageButton(NULL);
      break;
    case DL_TextEntry:
      element = createDlTextEntry(NULL);
      break;
    case DL_Menu:
      element = createDlMenu(NULL);
      break;
    case DL_Meter:
      element = createDlMeter(NULL);
      break;
    case DL_TextUpdate:
      element = createDlTextUpdate(NULL);
      break;
    case DL_Bar:
      element = createDlBar(NULL);
      break;
    case DL_Indicator:
      element = createDlIndicator(NULL);
      break;
    case DL_Byte:
      element = createDlByte(NULL);
      break;
    case DL_StripChart:
      element = createDlStripChart(NULL);
      break;
    case DL_CartesianPlot:
      element = createDlCartesianPlot(NULL);
      break;
    case DL_Rectangle:
      element = createDlRectangle(NULL);
      break;
    case DL_Oval:
      element = createDlOval(NULL);
      break;
    case DL_Arc:
      element = createDlArc(NULL);
      break;
    case DL_RelatedDisplay:
      element = createDlRelatedDisplay(NULL);
      break;
    case DL_ShellCommand:
      element = createDlShellCommand(NULL);
      break;
    case DL_Text:
      element = createDlText(NULL);
      break;
    case DL_DynSymbol:
      element = createDlDynSymbol(NULL);
      break;
    default:
      fprintf(stderr,"handleRectangularCreates: CREATE - invalid type %d",
	elementType);
      break;
  }

  if (element) {
    if (element->run->inheritValues) {
      element->run->inheritValues(&globalResourceBundle,element);
    }
    objectAttributeSet(&(element->structure.rectangle->object),
		       x,y,width,height);
  }
  return element;
}

void toggleSelectedElementHighlight(DlElement *dlElement) 
{
  extern Boolean debugFlag;
  DisplayInfo *cdi = currentDisplayInfo;
  int x, y, width, height; 
  DlObject *po = &(dlElement->structure.display->object);

  if ( debugFlag ) {
    printf ("  toggle Higlight for %x in %x (da %x pixmap %x)\n",
	    (int)dlElement, (int)cdi, (int)cdi->drawingArea, (int)cdi->drawingAreaPixmap);
  }

  if (dlElement->type == DL_Display) {
    x = HIGHLIGHT_LINE_THICKNESS;
    y = HIGHLIGHT_LINE_THICKNESS;
    width = po->width - 2*HIGHLIGHT_LINE_THICKNESS;
    height = po->height - 2*HIGHLIGHT_LINE_THICKNESS;
  } else {
    x = po->x-HIGHLIGHT_LINE_THICKNESS;
    y = po->y-HIGHLIGHT_LINE_THICKNESS;
    width = po->width + 2*HIGHLIGHT_LINE_THICKNESS;
    height = po->height + 2*HIGHLIGHT_LINE_THICKNESS;
  }
#if 0
  XSetForeground(display,highlightGC,cdi->drawingAreaBackgroundColor);
  XSetBackground(display,highlightGC,cdi->drawingAreaForegroundColor);
#endif

  XDrawRectangle(display,XtWindow(cdi->drawingArea),highlightGC,
    x, y, width, height);
  XDrawRectangle(display,cdi->drawingAreaPixmap,highlightGC,
    x, y, width, height);
}

void unselectSelectedElements() 
{
  DisplayInfo *cdi; /* abbreviation for currentDisplayInfo for clarity */

  if (!currentDisplayInfo) return;
  cdi = currentDisplayInfo;

  if (IsEmpty(cdi->selectedDlElementList)) return;

  deallocateAllDlElements(cdi->selectedDlElementList);

  return;
}

#define IF_NOT_INSIDE(x,y,x0,y0,x1,y1) \
     ((x) < (x0) || (x) > (x1) || (y) < (y0) || (y) > (y1))

#if 0
/*
 * starting at given element, look for one which has the point inside
 */
static DlElement *lookupElementForPosition(DlElement * element, 
					   Position    x0, 
					   Position    y0)
{
  /* single element lookup
   */
  while (element != NULL) 
  {
    register DlObject *po = &(element->structure.rectangle->object);

    if ( ! IF_NOT_INSIDE (x0, y0, 
			  po->x, po->y,
			  po->x + po->width, po->y + po->height))
      {
	return (element);
      }
    
    element = element->next;
  }

  return NULL;
}

/* this routine was used before for getting most deep composite.
 * as an hierarchy of element meens not so much in DM2K the routine
 * is not so usefull yet..
 */
static DlElement * getDeeperCompositeElement (DlElement * element,
					      int         x,
					      int         y)
{
  DlElement *localElement;

  if (element == NULL)
    return (DlElement *)NULL;

  localElement = element->structure.composite->dlElementList->head;

  while (localElement) {
    register DlObject *po = &(localElement->structure.rectangle->object);
    
    if ( ! IF_NOT_INSIDE (x, y, 
			  po->x, po->y,
			  po->x + po->width, po->y + po->height)) 
      {
	if (localElement->type == DL_Composite)
	  return getDeeperCompositeElement (localElement, x, y);
	else
	  return element;
      }
    
    localElement = localElement->next;
  }

  return element;
}
#endif /* 0 */

/* it looks for smallest size element without dependecies of layout
 * or position in the element chain
 */
static DlElement * getCompositeElement (DlElement * element,
					int         x,
					int         y)
{
  int minElementSize = 4194304; /* size of screen with 2048x2048 pixel */
  DlElement * look = NULL;

  while (element) {
    register DlObject *po = &(element->structure.rectangle->object);
    
    if ( ! IF_NOT_INSIDE (x, y, 
			  po->x, po->y,
			  po->x + po->width, po->y + po->height)) 
      {
	if (element->type == DL_Composite ) {
	  DlComposite * composite = element->structure.composite;

	  if (composite->ami) 
	    {
	      register int tmp = po->width * po->height;
	      
	      if (tmp < minElementSize)
		{
		  minElementSize = tmp;
		  look = element;
		}
	    }
	}
      }
    element = element->next;
  }
  return look;
}


/*ARGSUSED*/
#ifdef __cplusplus
void motionHandler (Widget,
		    XtPointer callData,
		    XEvent    *event,
		    Boolean   *)
#else
void motionHandler (Widget    w,
		    XtPointer callData,
		    XEvent    *event,
		    Boolean   *ctd)
#endif
{
  DisplayInfo  * displayInfo = (DisplayInfo *) callData;
  XMotionEvent * mEvent = (XMotionEvent *)event;
  DlElement    * element;

  if (displayInfo != currentDisplayInfo) {
    currentDisplayInfo  = displayInfo;
    currentColormap     = currentDisplayInfo->colormap;
    currentColormapSize = currentDisplayInfo->dlColormapSize;
  }

  element = getCompositeElement (displayInfo->dlElementList->head, 
				 (int)mEvent->x, (int)mEvent->y);

  if (element && element->type == DL_Composite) 
  {
    DlComposite * composite = element->structure.composite;

    if (composite->ami) {
      if (displayInfo->currentCursor != associatedMenuCursor) {
	XDefineCursor(display, XtWindow(displayInfo->drawingArea),
		      associatedMenuCursor);
	displayInfo->currentCursor = associatedMenuCursor;
      }
      return;
    } 
  }

  if (displayInfo->currentCursor == associatedMenuCursor)
    drawingAreaDefineCursor (displayInfo);
}

