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
 * .03  09-07-95        vong    conform to c++ syntax
 * .04  02-23-96        vong    fixe the wrong aspect calculation.
 *
 *****************************************************************************
 *
 *      24-02-97    Fabien      Use simpleCursor in display DrawingArea
 *                              Add function for display element information
 *
 *****************************************************************************
*/

#include "dm2k.h"

#include <limits.h>


/* for modal dialogs - defined in utils.c and referenced here in callbacks.c */
extern Boolean modalGrab;
extern Widget mainMW;

extern char *stripChartWidgetName;


/* Information methode for simple object with a single PV */
/* ------------------------------------------------------ */

Boolean objectInfoImplemented (DlElementType type)
{
  if ( (type < FIRST_RENDERABLE) || (type > MAX_DL_ELEMENT_TYPE) )
    return (False);

  switch (type) {
    case DL_Composite :
    case DL_CartesianPlot :
       return (False);
    default:
       return (True);
    }
}


static void genericObjectInfoSimple (char *msg, Widget w,
				     DisplayInfo *displayInfo,
				     DlElement *element,
				     XtPointer object)
{
  char *defname;

  switch (element->type) {
    case DL_Rectangle :
    case DL_Oval :
    case DL_Arc :
    case DL_Text :
    case DL_Line :
    case DL_Polyline :
    case DL_Polygon :
      break;

    case DL_Meter :
    case DL_TextUpdate :
    case DL_Bar :
    case DL_Byte :
    case DL_Indicator :
      switch (element->type) {
      case DL_Meter :
	defname = element->structure.meter->monitor.rdbk;
	break;
      case DL_TextUpdate :
	defname = element->structure.textUpdate->monitor.rdbk;
	break;
      case DL_Bar :
	defname = element->structure.bar->monitor.rdbk;
	break;
      case DL_Byte :
	defname = element->structure.byte->monitor.rdbk;
	break;
      case DL_Indicator :
	defname = element->structure.indicator->monitor.rdbk;
	break;
      default:
	 break;
      }
      sprintf (&msg[STRLEN(msg)], " : %s", CARE_PRINT(defname));
      break;

    case DL_Valuator :
    case DL_ChoiceButton :
    case DL_MessageButton :
    case DL_TextEntry :
    case DL_Menu :
      switch (element->type) {
      case DL_Valuator :
	defname = element->structure.valuator->control.ctrl;
	break;
      case DL_ChoiceButton :
	defname = element->structure.choiceButton->control.ctrl;
	break;
      case DL_TextEntry :
	defname = element->structure.textEntry->control.ctrl;
	break;
      case DL_Menu :
	defname = element->structure.menu->control.ctrl;
	break;
      default:
	 break;
      }
      sprintf (&msg[STRLEN(msg)], " = %s", CARE_PRINT(defname));
      break;

    case DL_ShellCommand :
    case DL_RelatedDisplay :
    case DL_Display :
    case DL_Image :
      break;
    default:
	 break;
    }
}


void genericObjectInfo (char        * msg, 
			Widget        w,
			DisplayInfo * displayInfo,
			DlElement   * element,
			XtPointer     object)
{
  /*
   * _SimplePmb structure must be compatible with the object pmb structure
   *      ref in the dm2kXxxx.c specific object method files.
   */
  typedef struct _SimplePmb {
    DlStructurePtr structurePtr;
    Record      *record;
  } SimplePmb;

  SimplePmb *pmb;
  char strg[512];
  char *defname;
  DlStructurePtr dlptr;

  if (globalDisplayListTraversalMode != DL_EXECUTE) {
    genericObjectInfoSimple (msg, w, displayInfo, element, object);
    return;
  }

  pmb = (SimplePmb *) object;

  if ( displayInfo == (DisplayInfo *) pmb ) {
    displayObjectInfo (msg, w, displayInfo, element, object);
    return;
  }

  dlptr = pmb->structurePtr;
  switch (element->type) {
    case DL_Rectangle :
    case DL_Oval :
    case DL_Arc :
    case DL_Text :
    case DL_Line :
    case DL_Polyline :
    case DL_Polygon :
      if ( pmb->record->monitorSeverityChanged ) {
      strcat (msg, "\n  Severity change");
      }
      else if (   pmb->record->monitorValueChanged
	     || pmb->record->monitorZeroAndNoneZeroTransition ) {
      strcat (msg, "\n  Value change");
      }
      else {
      strcat (msg, "\n  Static");
      }
      dm2kChanelInfo (strg, pmb->record, NULL);
      sprintf (&msg[STRLEN(msg)], "\n\n  Monitor PV :\n%s", strg);
      break;

    case DL_Meter :
    case DL_TextUpdate :
    case DL_Bar :
    case DL_Byte :
    case DL_Indicator :
      switch (element->type) {
      case DL_Meter :
	defname = dlptr.meter->monitor.rdbk;
	break;
      case DL_TextUpdate :
	defname = dlptr.textUpdate->monitor.rdbk;
	break;
      case DL_Bar :
	defname = dlptr.bar->monitor.rdbk;
	break;
      case DL_Byte :
	defname = dlptr.byte->monitor.rdbk;
	break;
      case DL_Indicator :
	defname = dlptr.indicator->monitor.rdbk;
	break;
    default:
	 break;
      }
      dm2kChanelInfo (strg, pmb->record, defname);
      sprintf (&msg[STRLEN(msg)], "\n\n  Monitor PV :\n%s", strg);
      break;

    case DL_Valuator :
    case DL_ChoiceButton :
    case DL_MessageButton :
    case DL_TextEntry :
    case DL_Menu :
      switch (element->type) {
      case DL_Valuator :
	defname = dlptr.valuator->control.ctrl;
	break;
      case DL_ChoiceButton :
	defname = dlptr.choiceButton->control.ctrl;
	break;
      case DL_TextEntry :
	defname = dlptr.textEntry->control.ctrl;
	break;
      case DL_Menu :
	defname = dlptr.menu->control.ctrl;
	break;
    default:
	 break;
      }
      dm2kChanelInfo (strg, pmb->record, defname);
      sprintf (&msg[STRLEN(msg)], "\n\n  Control PV :\n%s", strg);
      break;

    case DL_ShellCommand :
    case DL_RelatedDisplay :
    case DL_Display :
    case DL_Image :
      displayObjectInfo (msg, w, displayInfo, element, object);
      break;
    default:
	 break;
    }
}


void cleanObjectID ()
{
  DisplayInfo *displayInfo;
  Widget widget;

  for ( displayInfo = displayInfoListHead; displayInfo; displayInfo = displayInfo->next ) {
    if ( ! (widget = displayInfo->objectID) ) continue;
    if ( XtIsManaged (widget) ) XtUnmanageChild (widget);
  }
}



void objectPropertyFunc (Widget w, DisplayInfo *displayInfo)
{
  DlElement *element;
  XtPointer object;
  void (* elementInfoMethode)(char*,Widget,DisplayInfo*,DlElement*,XtPointer);
  char msg[2048];
  XmString msgXmString;
  DlElementType type;
  Widget widget;

  cleanObjectID ();

  element = displayInfo->dlPopupElement;
  object  = displayInfo->pointedObject;
  type    = element->type;

  if ( ! objectInfoImplemented (type) )
    return;

  elementInfoMethode = 
    (element->run->info) ? element->run->info : genericObjectInfo;
  /*
  elementInfoMethode = elementInfoTable[element->type - MIN_DL_ELEMENT_TYPE];
  */
  if ( elementInfoMethode && object ) {
      initializeXmStringValueTables ();
      memset (msg, '\0', sizeof(msg));
      sprintf (msg, "%s PROPERTIES",
	    elementStringTable[element->type - MIN_DL_ELEMENT_TYPE]);
      (*elementInfoMethode) (&msg[STRLEN(msg)], w, displayInfo, 
			     element, object);
      msg[sizeof(msg)-1] = '\0';
      msgXmString = XmStringCreateLtoR (msg, XmFONTLIST_DEFAULT_TAG);
      if (( widget = displayInfo->objectID )) {
	XtVaSetValues (widget,
		     XmNmessageString, msgXmString,
		     XmNdialogTitle, elementXmStringTable[element->type - MIN_DL_ELEMENT_TYPE],
		     NULL);
	XmStringFree (msgXmString);
	XtManageChild (widget);
      }
  }
}


void executePopupMenuCallback(Widget  w, XtPointer cd, XtPointer cbs)
{
  int                   buttonNumber = (int) cd;
  Arg                   args[3];
  XtPointer             data;
  DisplayInfo         * displayInfo;

 /* button's parent (menuPane) has the displayInfo pointer 
  */
  XtSetArg(args[0],XmNuserData,&data);
  XtGetValues(XtParent(w),args,1);
  displayInfo = (DisplayInfo *) data;

  switch(buttonNumber)
    {
    case EXECUTE_POPUP_MENU_PRINT_ID: 
      {
	/* This make take a second... give user some indication 
	 */
	if (currentDisplayInfo != NULL) 
	  XDefineCursor(display, XtWindow(currentDisplayInfo->drawingArea),
			watchCursor);

	XFlush(display);

	utilPrint(XtDisplay(displayInfo->drawingArea),
		  XtWindow(displayInfo->drawingArea),DISPLAY_XWD_FILE);
	
	/* change drawingArea's cursor back to the appropriate cursor 
	 */
	drawingAreaDefineCursor (currentDisplayInfo);
      } 
    break;
    
    case EXECUTE_POPUP_MENU_P_SETUP_ID :
      printerSetup(XtParent(currentDisplayInfo->drawingArea));
      break;

    case EXECUTE_POPUP_MENU_PROPERTY_ID:
      objectPropertyFunc (w, displayInfo);
      break;

    case EXECUTE_POPUP_MENU_HELP_ID :
      mouseButtonHelpCB (w, 
			 (XtPointer)XtParent(currentDisplayInfo->drawingArea),
			 NULL);
      break;

    case EXECUTE_POPUP_MENU_CLOSE_ID : 
      closeDisplay(w);
      break;

    case EXECUTE_POPUP_MENU_QUIT_ID :
      dm2kExit();
      break;

    default :
      break;
    }
}


void drawingAreaCallback(
  Widget  w,
  DisplayInfo *displayInfo,
  XmDrawingAreaCallbackStruct *call_data)
{
  int x, y;
  unsigned int uiw, uih;
  Dimension width, height, goodWidth, goodHeight, oldWidth, oldHeight;
  Boolean resized;
  XKeyEvent *key;
  Modifiers modifiers;
  KeySym keysym;
  Boolean objectDataOnly;
  Arg args[4];
  XtPointer userData;
  DlElement *elementPtr;
  float aspectRatio, newAspectRatio;

  Window root, child;
  int rootX,rootY,winX,winY;
  unsigned int mask;


  if (call_data->reason == XmCR_EXPOSE) 
    {
      x   = call_data->event->xexpose.x;
      y   = call_data->event->xexpose.y;
      uiw = call_data->event->xexpose.width;
      uih = call_data->event->xexpose.height;

    if (displayInfo->drawingAreaPixmap != (Pixmap)NULL 
	&& displayInfo->pixmapGC != (GC)NULL 
	&& displayInfo->drawingArea != (Widget)NULL) 
      {
	XCopyArea(display,displayInfo->drawingAreaPixmap,XtWindow(w),
		  displayInfo->pixmapGC,x,y,uiw,uih,x,y);

	if (globalDisplayListTraversalMode == DL_EXECUTE) 
	{
	  Display *display = XtDisplay(displayInfo->drawingArea);
	  GC gc = displayInfo->gc;
	  XRectangle clipRect[1];
	  
	  /* clip the rectangle */
	  clipRect[0].x = x;
	  clipRect[0].y = y;
	  clipRect[0].width = uiw;
	  clipRect[0].height = uih;

	  XSetClipRectangles(display, gc, 0, 0, clipRect, 1, YXBanded);

	  updateTaskRepaintRectangle(displayInfo, 
				     x, y, (int)(x + uiw), (int)(y + uih));

	  XSetClipOrigin(display,gc,0,0);
	  XSetClipMask(display,gc,None);
	}
      }
      return;
  } 
  else if (call_data->reason == XmCR_RESIZE) 
  {
    /* RESIZE */
    if (displayInfo->traverseDisplayListFlag) return;

    XtSetArg(args[0],XmNwidth,&width);
    XtSetArg(args[1],XmNheight,&height);
    XtSetArg(args[2],XmNuserData,&userData);
    XtGetValues(w,args,3);


    if (globalDisplayListTraversalMode == DL_EDIT) {
      unhighlightSelectedElements();
      resized = dmResizeSelectedElements(displayInfo,width,height);
      if (displayInfo->hasBeenEditedButNotSaved == False)
        dm2kMarkDisplayBeingEdited(displayInfo);

    } else { /* in EXECUTE mode - resize all elements */

      /* since calling for resize in resize handler - use this flag to ignore
       *   derived resize 
       */
      XQueryPointer(display,RootWindow(display,screenNum),&root,&child,
		&rootX,&rootY,&winX,&winY,&mask);

      if (userData != NULL || !(mask & ShiftMask) ) {

        XtSetArg(args[0],XmNuserData,(XtPointer)NULL);
        XtSetValues(w,args,1);
	goodWidth = width;
	goodHeight = height;

      } 
      else {

	/* constrain resizes to original aspect ratio, 
	 * call for resize, then return 
	 */
	elementPtr = FirstDlElement(displayInfo->dlElementList);

	/* 
	 * get to DL_Display type which has old x,y,width,height 
	 */
	while (elementPtr->type != DL_Display) {
	  elementPtr = elementPtr->next;
	}

	oldWidth  = elementPtr->structure.display->object.width;
	oldHeight = elementPtr->structure.display->object.height;

	aspectRatio    = (float)oldWidth/(float)oldHeight;
	newAspectRatio = (float)width/(float)height;

	if (newAspectRatio > aspectRatio) {
	  /* w too big; derive w=f(h) */
	  goodWidth = (unsigned short) (aspectRatio*(float)height);
	  goodHeight = height;
	} 
	else {
	  /* h too big; derive h=f(w) */
	  goodWidth = width;
	  goodHeight = (Dimension)((float)width/aspectRatio);
	}

	/* change width/height of DA */
	/* use DA's userData to signify a "forced" resize which 
	 * can be ignored */
	XtSetArg(args[0],XmNwidth,goodWidth);
	XtSetArg(args[1],XmNheight,goodHeight);
	XtSetArg(args[2],XmNuserData,(XtPointer)1);
	XtSetValues(w,args,3);

	return;
      }


      resized = dmResizeDisplayList(displayInfo,goodWidth,goodHeight);
    }

    /* (MDA) should always cleanup before traversing!! */
   if (resized) {
     clearResourcePaletteEntries();	/* clear any selected entries */
     dmCleanupDisplayInfo(displayInfo,FALSE);
#if 0
     XtAppAddTimeOut(appContext,1000,traverseDisplayLater,displayInfo); 
#else
     dmTraverseDisplayList(displayInfo);
#endif
   }


  } else
  if (call_data->reason == XmCR_INPUT) {
    /* INPUT */
    /* left/right/up/down for movement of selected elements */
    if (currentActionType == SELECT_ACTION &&
      !IsEmpty(currentDisplayInfo->selectedDlElementList)) {

      key = &(call_data->event->xkey);

      if (key->type == KeyPress ) {
        XtTranslateKeycode(display,key->keycode,(Modifiers)NULL,
                         &modifiers,&keysym);
        if (keysym == osfXK_Left || keysym == osfXK_Right  ||
            keysym == osfXK_Up   || keysym == osfXK_Down) {
          switch (keysym) {
            case osfXK_Left:
              updateDraggedElements(1,0,0,0);
              break;
            case osfXK_Right:
              updateDraggedElements(0,0,1,0);
              break;
            case osfXK_Up:
              updateDraggedElements(0,1,0,0);
              break;
            case osfXK_Down:
              updateDraggedElements(0,0,0,1);
              break;
            default:
              break;
          }
          if (currentDisplayInfo->selectedDlElementList->count == 1) {
            objectDataOnly = True;
            updateGlobalResourceBundleAndResourcePalette(objectDataOnly);
          }
          if (currentDisplayInfo->hasBeenEditedButNotSaved == False) 
             dm2kMarkDisplayBeingEdited(currentDisplayInfo);
        }
      }
    }
  }
}

void createNewDisplay(DisplayInfo * displayInfo,
		      char        * adlFilename,
		      char        * argsString,
		      int           replaceCurrentDisplay)
{
  char token[MAX_TOKEN_LENGTH];
  FILE *filePtr;
  char processedArgs[2*MAX_TOKEN_LENGTH];


  if (globalDisplayListTraversalMode == DL_EXECUTE) 
  {
    /* perform any macro substitution
     */
    performMacroSubstitutions(displayInfo, 
			      argsString,
			      processedArgs,
			      sizeof(processedArgs));
    /* find file and open it
     */
    filePtr = dmOpenUseableFile(adlFilename);

    if (filePtr == NULL) 
    {
      /* show warrning if file is not found
       */
      sprintf(token, "Can't open new display:\n\n  ``%s''", adlFilename);
      strcat (token, "\n\nPlease, check system env. EPICS_DISPLAY_PATH .");

      dmSetAndPopupWarningDialog(displayInfo, token, "Ok", NULL, NULL);

      fprintf(stderr,"%s\n",token);
    } 
    else 
    {
      /*
       * if file is found, parse it and display
       */
      dmDisplayListParse(replaceCurrentDisplay == True? displayInfo : NULL,
			 filePtr,
			 processedArgs,
			 adlFilename,
			 NULL,
			 (Boolean)True);
      fclose(filePtr);
    }
  }
}

/*
 * active callback of associated menu;
 * `widget' is PushButton widget
 */
void associatedMenuCB(Widget widget, XtPointer clientData, XtPointer callData)
{
  int                 buttonNumber = (int) clientData;
  DlElement           *element;
  DlComposite         *dlComposite;
  AssociatedMenuItem  *ami;
  int                 i, len;

  XtVaGetValues (XtParent(widget), XmNuserData, &element, NULL);
  dlComposite = element->structure.composite;
  
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
	break;
      }
  }
  else 
    fprintf (stderr, "problem %s:%d\n", __FILE__, __LINE__);

  /*
   * destroy popup menu shell
   */
  XtDestroyWidget (XtParent(XtParent(widget)));
}

/*ARGSUSED*/
#ifdef __cplusplus
void freeUserDataCB(Widget w, XtPointer , XtPointer)
#else
void freeUserDataCB(Widget w, XtPointer clientData, XtPointer callData)
#endif
{
  WidgetUserData * userData = NULL;

  XtVaGetValues (w, XmNuserData, &userData, NULL);

  DM2KFREE (userData);
}

