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
 * .04  01-23-97        romsky  destroy callback for element's widget
 *
 *****************************************************************************
 *
 * .04  02-07-97        Fabien  Addition of Toggle button and sensitivity
 *                              A Push Button is visualised with a
 *                                larger 3D thickness,
 *                              Status of the PV (ON or OFF) is define by
 *                                comparaison of the PV value and the
 *                                release value (or release message string)
 *                                if not equal : the status is ON !
 *                              A Push_And Exit Button will close the
 *                                display on button disarm.
 *                                It is shown with a black larger border
 *
 *****************************************************************************
*/

#include "dm2k.h"

#define SHADOWTHICKNESS 3
#define BORDERWIDTH 2

typedef struct _MessageButton {
  DlElement          *dlElement;
  Record             *record;
  UpdateTask         *updateTask;
  double             pressValue;
  double             releaseValue;
  Record             *sensitiveRecord;  /* if the sensitivity is control by a PV */
  Boolean            sensitive;     /* sensitive property */
  Boolean            toggle;        /* Toggle Buttom */
  Boolean            pushAndClose;  /* Push and close display Button */
  Boolean            toggleArm;     /* buttom is armed */
  Boolean            pdOnstate;     /* saved record on/off button state */
  int                toggleSet;     /* button value when armed */
  XmString           labelXmString; /* label for OFF state */
  XmString           alabelXmString;/* label for ON state */
} MessageButton;

static UpdateTask * messageButtonCreateRunTimeInstance(DisplayInfo *, DlElement *);
static void messageButtonCreateEditInstance(DisplayInfo *, DlElement *);

static void messageButtonDraw(XtPointer);
static void messageButtonUpdateValueCb(XtPointer);
static void messageButtonUpdateGraphicalInfoCb(XtPointer);
static void messageButtonDestroyCb(XtPointer);
static void messageButtonValueChangedCb(Widget, XtPointer, XtPointer);
static void messageButtonName(XtPointer, char **, short *, int *);
static void messageButtonInheritValues(ResourceBundle *, DlElement *);
static void messageButtonGetValues(ResourceBundle *, DlElement *);
static void messageButtonInfo (char *, Widget, DisplayInfo *, DlElement *, XtPointer);
static void messageButtonDeferredAction (DlElement *, Boolean);
static Boolean dialogDisplayType (DlElement *dlElement);
static Boolean getCurrentValue (MessageButton *);
static Boolean getPvstate (MessageButton *);

static void destroyDlElement (DlElement * element);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);

static DlDispatchTable messageButtonDlDispatchTable = {
	 createDlMessageButton,
	 destroyDlElement,
         executeMethod,
         writeDlMessageButton,
         NULL,
         messageButtonGetValues,
         messageButtonInheritValues,
         NULL,
         NULL,
         genericMove,
         genericScale,
	 NULL,
	 messageButtonInfo,
	 messageButtonDeferredAction
	 };

static void destroyDlMessageButton (DlMessageButton * dlMessageButton)
{
  if (dlMessageButton == NULL)
    return;

  objectAttributeDestroy(&(dlMessageButton->object));
  controlAttributeDestroy(&(dlMessageButton->control));
  sensitveAttributeDestroy(&(dlMessageButton->sensitive));

  DM2KFREE(dlMessageButton->label);
  DM2KFREE(dlMessageButton->press_msg);
  DM2KFREE(dlMessageButton->release_msg);
  DM2KFREE(dlMessageButton->alabel);
  
  free((char*)dlMessageButton);
}

static void destroyDlElement (DlElement * element)
{
  if (element == NULL || element->type != DL_MessageButton) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  destroyDlMessageButton (element->structure.messageButton);
  free((char*)element);
}

int messageButtonFontListIndex(int height)
{
  register int i;

  /* don't allow height of font to exceed 90% - 4 pixels
   * of messageButton widget (includes nominal 2*shadowThickness=2 shadow)
   */
  for (i = MAX_FONTS-1; i >=  0; i--) {
    if ( ((int)(.90*height) - 4) >= (fontTable[i]->ascent + fontTable[i]->descent))
      return(i);
  }
  return (0);
}


static void messageButtonUpdateAttribute (
  DlElement *dlElement,
  Boolean sensitive,
  MessageButtonType type)
{
  DlMessageButton *dlMessageButton = dlElement->structure.messageButton;
  Arg   args[4];
  int   n;
  Dimension thickness;
  Dimension border;

  thickness = dlMessageButton->shadowThickness;
  border = dlMessageButton->borderWidth;
  if ( type == TOGGLE_BUTTON ) thickness += SHADOWTHICKNESS;
  if ( type == PUSH_CLOSE_BUTTON ) border += BORDERWIDTH;

  n = 0;
  XtSetArg(args[n],XmNshadowThickness, thickness); n++;
  XtSetArg(args[n],XmNborderWidth, border); n++;
  XtSetValues (dlElement->widget, args, n);
}


static void messageButtonCreate (
  DisplayInfo *displayInfo,
  DlElement   *dlElement,
  Boolean     sensitive,
  XtPointer   userData)
{
  XmString        xmString;
  DlMessageButton *dlMessageButton;
  Arg             args[25];
  int             n;
  Boolean         toggle;

  dlMessageButton = dlElement->structure.messageButton;
  toggle = dlMessageButton->buttonType == TOGGLE_BUTTON;
  xmString = XmStringCreateSimple(dlMessageButton->label);

  n = 0;
  XtSetArg(args[n],XmNx,(Position)dlMessageButton->object.x); n++;
  XtSetArg(args[n],XmNy,(Position)dlMessageButton->object.y); n++;
  XtSetArg(args[n],XmNwidth,(Dimension)dlMessageButton->object.width); n++;
  XtSetArg(args[n],XmNheight,(Dimension)dlMessageButton->object.height); n++;
  XtSetArg(args[n],XmNforeground,(Pixel)
	displayInfo->colormap[dlMessageButton->control.clr]); n++;
  XtSetArg(args[n],XmNbackground,(Pixel)
	displayInfo->colormap[dlMessageButton->control.bclr]); n++;
  XtSetArg(args[n],XmNhighlightThickness,1); n++;
  XtSetArg(args[n],XmNhighlightOnEnter,TRUE); n++;
  XtSetArg(args[n],XmNindicatorOn,(Boolean)FALSE); n++;
  XtSetArg(args[n],XmNrecomputeSize,(Boolean)FALSE); n++;
  XtSetArg(args[n],XmNlabelString, xmString); n++;
  XtSetArg(args[n],XmNlabelType, XmSTRING); n++;
  XtSetArg(args[n],XmNfontList,
	   fontListTable[messageButtonFontListIndex
			 (dlMessageButton->object.height)]);
  n++;

  dlElement->widget = XtCreateWidget("messageButton",
			     (toggle) ? xmToggleButtonWidgetClass
				      : xmPushButtonWidgetClass,
			     displayInfo->drawingArea, args, n);
  XmStringFree(xmString);

  if (userData) {
    XtVaSetValues(dlElement->widget, XmNuserData, userData, NULL);
    /* destroy callback should free allocated memory
     */
    XtAddCallback (dlElement->widget, XmNdestroyCallback,
		   freeUserDataCB, NULL);
  }

  /* get and update the basic attributes */
  n = 0;
  XtSetArg(args[n],XmNborderWidth, &(dlMessageButton->borderWidth)); n++;
  XtSetArg(args[n],XmNshadowThickness, &(dlMessageButton->shadowThickness)); n++;
  XtGetValues (dlElement->widget, args, n);
  messageButtonUpdateAttribute (dlElement, sensitive, dlMessageButton->buttonType);
}


static void messageButtonCreateEditInstance(
      DisplayInfo *displayInfo,
      DlElement   *dlElement) 
{
  DlMessageButton *dlMessageButton;
  Boolean sensitive;

  dlMessageButton = dlElement->structure.messageButton;
  sensitive = (dlMessageButton->sensitive.chan != NULL) && dlMessageButton->sensitive.chan[0];

  messageButtonCreate (displayInfo, dlElement, sensitive, NULL);

  controlHandler (displayInfo, dlElement);

  XtManageChild(dlElement->widget);
}


static UpdateTask * messageButtonCreateRunTimeInstance
   ( DisplayInfo *displayInfo,
     DlElement   *dlElement) 
{
  MessageButton   * pmb;
  DlMessageButton * dlMessageButton = dlElement->structure.messageButton;
  WidgetUserData  * userData;
  UpdateTask      * updateTask = NULL;

  /* alloc servise structures
   */
  userData = DM2KALLOC (WidgetUserData);
  pmb = DM2KALLOC (MessageButton);

  if (pmb == NULL || userData == NULL) {
    DM2KFREE (pmb);
    DM2KFREE (userData);
    dm2kPrintf("messageButtonCreateRunTimeInstance: memory alloc error\n");
    return updateTask;
  }

  pmb->dlElement = dlElement;
  dlMessageButton->object.runtimeDescriptor = (XtPointer) pmb;

  pmb->toggle = dlMessageButton->buttonType == TOGGLE_BUTTON;
  pmb->pushAndClose = dlMessageButton->buttonType == PUSH_CLOSE_BUTTON;
  pmb->toggleArm = False;
  pmb->sensitive = (dlMessageButton->sensitive.chan != NULL) && dlMessageButton->sensitive.chan[0];
  pmb->labelXmString  = XmStringCreateSimple(dlMessageButton->label);
  pmb->alabelXmString = XmStringCreateSimple
    ( (dlMessageButton->alabel != NULL)
      ? dlMessageButton->alabel
      : dlMessageButton->label);

  updateTask = pmb->updateTask = updateTaskAddTask(displayInfo,
						   &(dlMessageButton->object),
						   messageButtonDraw,
						   (XtPointer)pmb);
  if (pmb->updateTask == NULL) {
    dm2kPrintf
      ("messageButtonCreateRunTimeInstance : memory allocation error\n");
  }
  else {
    updateTaskAddDestroyCb(pmb->updateTask,messageButtonDestroyCb);
    updateTaskAddNameCb(pmb->updateTask,messageButtonName);
  }
/* Olx changed dlMessageButton->control.ctrl to
 * &dlMessageButton->control.ctrl*/
  pmb->record = dm2kAllocateRecord(dlMessageButton->control.ctrl,
				   messageButtonUpdateValueCb,
				   messageButtonUpdateGraphicalInfoCb,
				   (XtPointer) pmb);
  
  pmb->toggleSet = 0;
  pmb->pdOnstate = -3;    /* not possible value ! */

  pmb->sensitiveRecord =  sensitiveCreateRecord (&dlMessageButton->sensitive,
				messageButtonUpdateValueCb,
				NULL,
				(XtPointer) pmb,
				&pmb->sensitive);

  drawWhiteRectangle(pmb->updateTask);

  userData->privateData = (char*) pmb;
  userData->updateTask = (pmb ? pmb->updateTask : NULL);

  messageButtonCreate (displayInfo, dlElement, 
		       pmb->sensitive, (XtPointer) userData);

  /* no controlled channel case : just a "Close Push Button" */
  if ( globalDm2kReadOnly && pmb->pushAndClose
       && dlMessageButton->control.ctrl == NULL
       && ( ( dlMessageButton->sensitive.chan == NULL ) ||
	    ( dlMessageButton->sensitive.chan[0] == 0 ) )
       && dlElement->widget ) 
    XtSetSensitive (dlElement->widget, True);
  else
    sensitiveSetWidget (dlElement->widget, &dlMessageButton->sensitive,
			pmb->sensitiveRecord, pmb->updateTask);

  /* add in drag/drop translations */
  XtOverrideTranslations(dlElement->widget,parsedTranslations);

  /* add the callbacks for update and destroy */
  XtAddCallback(dlElement->widget,XmNarmCallback,messageButtonValueChangedCb,
		(XtPointer)pmb);
  XtAddCallback(dlElement->widget,XmNdisarmCallback,
		messageButtonValueChangedCb,
		(XtPointer)pmb);

  XtManageChild(dlElement->widget);

  return updateTask;
}

static UpdateTask * executeMethod
  (DisplayInfo *displayInfo,
   DlElement *dlElement)
{
  UpdateTask * updateTask = NULL;

  if (displayInfo->traversalMode == DL_EXECUTE) 
    {
      dlElement->structure.messageButton->object.runtimeDescriptor = NULL;
      updateTask = messageButtonCreateRunTimeInstance(displayInfo,dlElement);
    } 
  else if (displayInfo->traversalMode == DL_EDIT) 
    {
      if (dlElement->widget) {
	DlMessageButton *dlMessageButton;
	DlObject *po;
	XmString xmString;
	Boolean sensitive;

	dlMessageButton = dlElement->structure.messageButton;
	po = &dlMessageButton->object;
	sensitive = (dlMessageButton->sensitive.chan != NULL) && dlMessageButton->sensitive.chan[0];
	xmString = XmStringCreateSimple(dlMessageButton->label);
	XtVaSetValues(dlElement->widget,
		      XmNx,           (Position) po->x,
		      XmNy,           (Position) po->y,
		      XmNwidth,       (Dimension) po->width,
		      XmNheight,      (Dimension) po->height,
		      XmNlabelString, xmString,
		      NULL);
	XmStringFree(xmString);
	messageButtonUpdateAttribute (dlElement, sensitive, dlMessageButton->buttonType);
      } else {
	dlElement->structure.messageButton->object.runtimeDescriptor = NULL;
	messageButtonCreateEditInstance(displayInfo,dlElement);
      }
    }
  
  return updateTask;
}

static void messageButtonUpdateGraphicalInfoCb(XtPointer cd) 
{
  Record *pd = (Record *) cd;
  MessageButton *pmb = (MessageButton *) pd->clientData;
  DlMessageButton *dlMessageButton = pmb->dlElement->structure.messageButton;
  int i;
  Boolean match;
  Boolean dynLabel;

  /* Default value for toggle button */
  pmb->pressValue = (double) 1.0;
  pmb->releaseValue = (double) 0.0;

  dynLabel = (pmb->toggle && (dlMessageButton->label == NULL)
			&& (dlMessageButton->alabel == NULL));
  switch (pd->dataType) {
    case DBF_STRING:
      break;
    case DBF_ENUM :
      if (dlMessageButton->press_msg && dlMessageButton->press_msg[0]) {
        match = False;
        for (i = 0; i < pd->hopr+1; i++) {
          if (pd->stateStrings[i]) {
            if (STREQL(dlMessageButton->press_msg,pd->stateStrings[i])) {
              pmb->pressValue = (double) i;
              match = True;
              break;
            }
	  }
        }
        if (match == False) {
          pmb->pressValue = (double) atof(dlMessageButton->press_msg);
        }
	if ( dynLabel ) {
	  i = (int) pmb->pressValue;
	  if ( (i >= 0) && (i < pd->hopr+1) ) {
	    if ( pmb->alabelXmString )  XmStringFree (pmb->alabelXmString);
	    pmb->alabelXmString = XmStringCreateSimple(pd->stateStrings[i]);
	  }
	}
      }
      if (dlMessageButton->release_msg && dlMessageButton->release_msg[0] ) {
        match = False;
        for (i = 0; i < pd->hopr+1; i++) {
          if (pd->stateStrings[i]) {
            if (STREQL(dlMessageButton->release_msg,pd->stateStrings[i])) {
              pmb->releaseValue = (double) i;
              match = True;
              break;
            }
          }
	}
        if (match == False) {
           pmb->releaseValue = (double) atof(dlMessageButton->release_msg);
        }
      }
      break;
    default:
      
      if (dlMessageButton->press_msg && dlMessageButton->press_msg[0])
        pmb->pressValue = (double) atof(dlMessageButton->press_msg);
      if (dlMessageButton->release_msg && dlMessageButton->release_msg[0])
        pmb->releaseValue = (double) atof(dlMessageButton->release_msg);
      break;
  }
}


static void messageButtonUpdateValueCb(XtPointer cd) {
  MessageButton *pmb = (MessageButton *) ((Record *) cd)->clientData;
  updateTaskMarkUpdate(pmb->updateTask);
}

static void messageButtonDraw(XtPointer cd) 
{
  MessageButton    * pmb = (MessageButton *) cd;
  Record           * pd = pmb->record;
  Widget             widget = pmb->dlElement->widget;
  DlElement        * dlElement;
  DlMessageButton  * dlMessageButton;
  Boolean            onstate;
  Boolean            pvstate;
  Pixel              bpixel;
  int                clrix;
  int                n;
  Arg                args[4];

  dlElement = pmb->dlElement;
  dlMessageButton = dlElement->structure.messageButton;

  if (pd && pd->connected) {
    if (pd->readAccess) {
      if (widget)
        XtManageChild(widget);
      else 
        return;
      switch (dlMessageButton->clrmod) {
        case STATIC :
        case DISCRETE :
          break;
        case ALARM :
          XtVaSetValues(widget,
			XmNforeground,alarmColorPixel[pd->severity],
			NULL);
          break;
        default :
          break;
      }

      if (pd->writeAccess)
        XDefineCursor(XtDisplay(widget),XtWindow(widget),rubberbandCursor);
      else
        XDefineCursor(XtDisplay(widget),XtWindow(widget),noWriteAccessCursor);

      /* Toggle Button */
      if ( pmb->toggle ) {
	if ( ! pmb->toggleArm ) {
	  pvstate = getPvstate (pmb);
	  onstate = getCurrentValue (pmb);
	  clrix = (pvstate) ? 
	    dlMessageButton->abclr : dlMessageButton->control.bclr;
	  bpixel = pmb->updateTask->displayInfo->colormap[clrix];

	  n = 0;
	  XtSetArg (args[n], XmNset, onstate); n++;
	  XtSetArg (args[n], XmNbackground, bpixel); n++;
	  XtSetArg (args[n], XmNlabelString, 
		    (pvstate) ? pmb->alabelXmString : pmb->labelXmString); n++;
	  XtSetValues (dlElement->widget, args, n);
	  pmb->toggleSet = onstate;
	  pmb->pdOnstate = pvstate;
	}
      }

      /* sensitive Controller */
      sensitiveSetWidget (dlElement->widget, &dlMessageButton->sensitive,
			  pmb->sensitiveRecord, pmb->updateTask);

    } else {
      draw3DPane(pmb->updateTask,
         pmb->updateTask->displayInfo->colormap[dlMessageButton->control.bclr]);
      draw3DQuestionMark(pmb->updateTask);
      if (widget) XtUnmanageChild(widget);
    }
  } else {
    if ( widget && pmb->pushAndClose ) {
/* Olx changed dlMessageButton->control.ctrl[0] to &dlMessageButton->control.ctrl[0]*/    
      if ( dlMessageButton->control.ctrl && dlMessageButton->control.ctrl[0] ) {
	XtUnmanageChild(widget);
	drawWhiteRectangle(pmb->updateTask);
      }
      else {    /* no controlled channel : just a "Close Push Button" */
	if ( globalDm2kReadOnly && 
	     ( ( dlMessageButton->sensitive.chan == NULL ) ||
	       ( dlMessageButton->sensitive.chan[0] == 0 ) ) )
	  XtSetSensitive (widget, True);
	else
	  sensitiveSetWidget (dlElement->widget, &dlMessageButton->sensitive,
			      pmb->sensitiveRecord, pmb->updateTask);
	XtManageChild(widget);
      }
    }
    else {
      if (widget) XtUnmanageChild(widget);
      drawWhiteRectangle(pmb->updateTask);
    }
  }
}


static void messageButtonDestroyCb(XtPointer cd) {
  MessageButton *pmb = (MessageButton *) cd;
  if (pmb) {
    if ( pmb->labelXmString )  XmStringFree (pmb->labelXmString);
    if ( pmb->alabelXmString ) XmStringFree (pmb->alabelXmString);
    if ( pmb->record ) dm2kDestroyRecord(pmb->record);
    sensitiveDestroyRecord (&pmb->sensitiveRecord, &pmb->sensitive);
    free((char *)pmb);
  }
}

static Boolean getPvstate (MessageButton *pmb)
{
  Record *pd;
  DlMessageButton *dlMessageButton;
  Boolean pvstate;

  pd = pmb->record;
  if ( ! pd ) return (False);

  /* evaluated the record on / off state */
  dlMessageButton = pmb->dlElement->structure.messageButton;
  pvstate = (pd->dataType == DBF_STRING)
		/* T. Straumann: protect against null pointer */
	    ? ( strcmp ((char *)pd->array, dlMessageButton->release_msg ? dlMessageButton->release_msg : "") )
	    : ( (int)pd->value != (int)pmb->releaseValue );
  return (pvstate);
}


static Boolean getCurrentValue (MessageButton *pmb)
{
  Record *pd;
  Boolean onstate;

  pd = pmb->record;
  if ( ! pd ) return (False);

  /* significant only for toggle button if not armed */

  if ( !pmb->toggle || pmb->toggleArm ) return (False);

  /* evaluated the record on / off state */
  onstate = getPvstate (pmb);
  if ( dialogDisplayType (pmb->dlElement) ) {
    /* special case for window expose */
    if ( onstate == pmb->pdOnstate ) onstate = pmb->toggleSet;
  }
  return (onstate);
}


static Boolean dialogDisplayType (DlElement *dlElement)
{
  DisplayInfo *displayInfo;

  if ( ! dlElement ) return (False);

  displayInfo = dlElement->displayInfo;
  return ( displayInfo && (displayInfo->displayType != NORMAL_DISPLAY) );
}


static void messageButtonSendData (
  MessageButton *pmb,
  Boolean pressFlag,        /* press (True) or release (False) case */
  Boolean executeFlag)      /* if True, execute in any case */
{
  Record *pd;
  DlMessageButton *dlMessageButton;
  double dval;
  char *msg;

  if ( ! executeFlag ) {
    if ( dialogDisplayType (pmb->dlElement) ) return;
  }

  pd = pmb->record;
  if ( pd == NULL ) return;     /* no PV attached */

  dlMessageButton = pmb->dlElement->structure.messageButton;
  msg =  (pressFlag) ? dlMessageButton->press_msg : dlMessageButton->release_msg;
  if (msg != NULL) {
    switch (pd->dataType) {
      case DBF_STRING:
	dm2kSendString(pmb->record, msg);
	break;
      default:
	dval = (pressFlag) ? pmb->pressValue : pmb->releaseValue;
	dm2kSendDouble(pmb->record, dval);
	break;
    }
  }
}


static void messageButtonDeferredAction (DlElement *dlElement, Boolean applyFlag)
{
  DlMessageButton *dlMessageButton;
  MessageButton *pmb;
  Record *pd;

  dlMessageButton = dlElement->structure.messageButton;
  pmb = (MessageButton *) dlMessageButton->object.runtimeDescriptor;
  if ( pmb == NULL ) return;    /* nothing instancied */
  if ( ! pmb->toggle ) return;  /* only for TOGGLE button */

  pd = pmb->record;
  if ( pd == NULL ) return;     /* no PV attached */

  if ( applyFlag ) {
    if (pd && pd->connected) {
      if (pd->writeAccess) {
	messageButtonSendData (pmb, pmb->toggleSet, True);
      } else {
	fputc((int)'\a',stderr);
      }
    }
  } else {      /* reset to current state */
    pmb->toggleSet = (int) getPvstate (pmb);
    messageButtonDraw ((XtPointer) pmb);
  }
}


#ifdef __cplusplus
static void messageButtonValueChangedCb(Widget,
                XtPointer clientData,
                XtPointer callbackData) {
#else
static void messageButtonValueChangedCb(Widget w,
                XtPointer clientData,
                XtPointer callbackData) {
#endif
  MessageButton *pmb = (MessageButton *) clientData;
  Record *pd = pmb->record;
  XmPushButtonCallbackStruct *pushCallData;
  XmToggleButtonCallbackStruct *toggleCallData;
  DlElement *dlElement;
  DlMessageButton *dlMessageButton;
  Boolean onstate;

  dlElement = pmb->dlElement;
  dlMessageButton = dlElement->structure.messageButton;

  if ( pmb->toggle ) {      /* Toggle Button case */
    toggleCallData = (XmToggleButtonCallbackStruct *) callbackData;
    if (toggleCallData->reason == XmCR_DISARM) pmb->toggleArm = False;
    if (pd && pd->connected) {
      if (pd->writeAccess) {
	XtVaGetValues(dlElement->widget, XmNset, &onstate, NULL);
	if (toggleCallData->reason == XmCR_ARM) {
	  pmb->toggleArm = True;
	  pmb->toggleSet = toggleCallData->set;
	}
	else if (toggleCallData->reason == XmCR_DISARM) {
	  if ( pmb->toggleSet != toggleCallData->set ) {
	    /* Send data to PV only if the button state was changed */
	    messageButtonSendData (pmb, toggleCallData->set, False);
	  }
	  pmb->toggleSet = toggleCallData->set;
	}
      } else {
	fputc((int)'\a',stderr);
      }
    }
  }

  else {        /* Push (and PUSH and EXIT) Button case */
    pushCallData = (XmPushButtonCallbackStruct *) callbackData;
    if (pd && pd->connected) {
      if (pd->writeAccess) {
	if (pushCallData->reason == XmCR_ARM) {
	  /* message button can only put strings */
	  messageButtonSendData (pmb, True, False);
	} else
	if (pushCallData->reason == XmCR_DISARM) {
	  messageButtonSendData (pmb, False, False);
	}
      } else {
	fputc((int)'\a',stderr);
      }
    }
    if ( pmb->pushAndClose && (pushCallData->reason == XmCR_DISARM))
      closeDisplay(dlElement->widget);
  }
}

static void messageButtonName(XtPointer cd, char **name, short *severity, int *count) {
  MessageButton *pmb = (MessageButton *) cd;
  *count = 1;
  name[0] = pmb->record->name;
  severity[0] = pmb->record->severity;
}

 
 
/***
 *** Message Button
 ***/
 
DlElement *createDlMessageButton(DlElement *p)
{
  DlMessageButton * dlMessageButton;
  DlElement       * dlElement;
 
  dlMessageButton = DM2KALLOC(DlMessageButton);

  if (p != NULL) {
    objectAttributeCopy(&(dlMessageButton->object), 
			&(p->structure.messageButton->object));
    controlAttributeCopy(&(dlMessageButton->control),
			 &(p->structure.messageButton->control));
    sensitveAttributeCopy(&(dlMessageButton->sensitive),
			  &(p->structure.messageButton->sensitive));

    renewString(&dlMessageButton->label, p->structure.messageButton->label);
    renewString(&dlMessageButton->press_msg, p->structure.messageButton->press_msg);
    renewString(&dlMessageButton->release_msg, p->structure.messageButton->release_msg);
    renewString(&dlMessageButton->alabel, p->structure.messageButton->alabel);

    dlMessageButton->clrmod      = p->structure.messageButton->clrmod;
    dlMessageButton->buttonType  = p->structure.messageButton->buttonType;
    dlMessageButton->abclr       = p->structure.messageButton->abclr;
  } 
  else {
    objectAttributeInit(&(dlMessageButton->object));
    controlAttributeInit(&(dlMessageButton->control));
    sensitveAttributeInit(&(dlMessageButton->sensitive));

    dlMessageButton->label       = NULL;
    dlMessageButton->press_msg   = NULL;
    dlMessageButton->release_msg = NULL;
    dlMessageButton->alabel      = NULL;
    dlMessageButton->clrmod      = STATIC;
    dlMessageButton->buttonType  = PUSH_BUTTON;
    dlMessageButton->abclr       = -1;
  }

  dlElement = createDlElement(DL_MessageButton,
			      (XtPointer) dlMessageButton,
			      &messageButtonDlDispatchTable);

  if (dlElement == NULL)
    destroyDlMessageButton(dlMessageButton);

  return(dlElement);
}

DlElement *parseMessageButton(DisplayInfo *displayInfo)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;
  DlMessageButton *dlMessageButton;
  DlElement *dlElement = createDlMessageButton(NULL);

  if (!dlElement) return 0;
  dlMessageButton = dlElement->structure.messageButton;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
      case T_WORD:
        if (STREQL(token,"object"))
          parseObject(displayInfo,&(dlMessageButton->object));
        else if (STREQL(token,"control"))
	   parseControl(displayInfo,&(dlMessageButton->control));
	else if (STREQL(token,"sensitive")) 
	{
	  /* add on for sensitivity property */
	  parseSensitive (displayInfo, &(dlMessageButton->sensitive));
	}
        else if (STREQL(token,"press_msg")) {
          getToken(displayInfo,token);
          getToken(displayInfo,token);
          renewString(&dlMessageButton->press_msg,token);
        } else if (STREQL(token,"release_msg")) {
          getToken(displayInfo,token);
          getToken(displayInfo,token);
          renewString(&dlMessageButton->release_msg,token);
        } 
	else if (STREQL(token,"label")) 
	{
          getToken(displayInfo,token);
          getToken(displayInfo,token);
          renewString(&dlMessageButton->label,token);
        }
	else if (STREQL(token,"clrmod")) 
	{
          getToken(displayInfo,token);
          getToken(displayInfo,token);
          if (STREQL(token,"static"))
            dlMessageButton->clrmod = STATIC;
          else if (STREQL(token,"alarm"))
            dlMessageButton->clrmod = ALARM;
          else if (STREQL(token,"discrete"))
            dlMessageButton->clrmod = DISCRETE;
	 } else

	 /* add on for toggle button */
	 if (STREQL(token,"type")) {
	    dlMessageButton->buttonType = PUSH_BUTTON;
	    getToken(displayInfo,token);
	    getToken(displayInfo,token);
	    if (STREQL(token,"toggle"))
		dlMessageButton->buttonType = TOGGLE_BUTTON;
	    else if (STREQL(token,"push_and_close"))
		dlMessageButton->buttonType = PUSH_CLOSE_BUTTON;
	 } else
	 if (STREQL(token,"pressed_bclr")) {
	    getToken(displayInfo,token);
	    getToken(displayInfo,token);
	    dlMessageButton->abclr = atoi(token) % DL_MAX_COLORS;
	 } else
	 if (STREQL(token,"pressed_label")) {
	    getToken(displayInfo,token);
	    getToken(displayInfo,token);
	    renewString(&dlMessageButton->alabel,token);
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

  if ( dlMessageButton->abclr == -1 )
    dlMessageButton->abclr = dlMessageButton->control.bclr;

  return dlElement;
}

void writeDlMessageButton(
  FILE *stream,
  DlElement *dlElement,
  int level)
{
  char *type;
  char indent[256]; level=MIN(level,256-2);
  DlMessageButton *dlMessageButton = dlElement->structure.messageButton;

  memset(indent,'\t',level);
  indent[level] = '\0';
 
  switch (dlMessageButton->buttonType) {
    case TOGGLE_BUTTON :
      type = "toggle";
      break;
    case PUSH_BUTTON :
      type = "push";
      break;
    case PUSH_CLOSE_BUTTON :
      type = "push_and_close";
      break;
    }

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
    fprintf(stream,"\n%s\"message button\" {",indent);
    fprintf(stream,"\n%s\ttype=\"%s\"",indent, type);
    writeDlObject(stream,&(dlMessageButton->object),level+1);
    writeDlControl(stream,&(dlMessageButton->control),level+1);
    writeDlSensitive(stream,&(dlMessageButton->sensitive),level+1);

    if (dlMessageButton->label != NULL)
      fprintf(stream,"\n%s\tlabel=\"%s\"",indent,dlMessageButton->label);

    if (dlMessageButton->press_msg && dlMessageButton->press_msg[0] )
      fprintf(stream,"\n%s\tpress_msg=\"%s\"",indent,dlMessageButton->press_msg);

    if (dlMessageButton->release_msg && dlMessageButton->release_msg[0])
      fprintf(stream,"\n%s\trelease_msg=\"%s\"",
	      indent,dlMessageButton->release_msg);

    if (dlMessageButton->clrmod != STATIC) 
      fprintf(stream,"\n%s\tclrmod=\"%s\"",indent,
	      stringValueTable[dlMessageButton->clrmod]);

    if ( dlMessageButton->buttonType == TOGGLE_BUTTON ) {
      if (   (dlMessageButton->abclr != -1)
	     && (dlMessageButton->abclr != dlMessageButton->control.bclr ) )
	fprintf(stream,"\n%s\tpressed_bclr=%d",indent,dlMessageButton->abclr);
      if (dlMessageButton->alabel != NULL)
	fprintf(stream,"\n%s\tpressed_label=\"%s\"",indent,dlMessageButton->alabel);
    }
    fprintf(stream,"\n%s}",indent);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    fprintf(stream,"\n%s\"message button\" {",indent);
    fprintf(stream,"\n%s\ttype=\"%s\"",indent, type);
    writeDlObject(stream,&(dlMessageButton->object),level+1);
    writeDlControl(stream,&(dlMessageButton->control),level+1);
    writeDlSensitive(stream,&(dlMessageButton->sensitive),level+1);

    fprintf(stream,"\n%s\tlabel=\"%s\"",indent,CARE_PRINT(dlMessageButton->label));
    fprintf(stream,"\n%s\tpress_msg=\"%s\"",indent,CARE_PRINT(dlMessageButton->press_msg));
    fprintf(stream,"\n%s\trelease_msg=\"%s\"",
	    indent,CARE_PRINT(dlMessageButton->release_msg));

    fprintf(stream,"\n%s\tclrmod=\"%s\"",indent,
	    stringValueTable[dlMessageButton->clrmod]);

    fprintf(stream,"\n%s}",indent);

    if ( dlMessageButton->buttonType == TOGGLE_BUTTON ) {
      fprintf(stream,"\n%s\tabclr=%d",indent,dlMessageButton->abclr);
      fprintf(stream,"\n%s\talabel=\"%s\"",indent,dlMessageButton->alabel);
    }
  }
#endif
}

static void messageButtonInheritValues(ResourceBundle *pRCB, DlElement *p)
{
  DlMessageButton *dlMessageButton = p->structure.messageButton;
  updateElementControlAttribute   (pRCB, &dlMessageButton->control);
  updateElementSensitiveAttribute (pRCB, &dlMessageButton->sensitive);

  dm2kGetValues(pRCB,
		CLRMOD_RC,     &(dlMessageButton->clrmod),
		BUTTON_TYPE_RC,    &(dlMessageButton->buttonType),
		-1);
}

static void messageButtonGetValues(ResourceBundle *pRCB, DlElement *p)
{
  DlMessageButton *dlMessageButton = p->structure.messageButton;
  /*
  dm2kGetValues(pRCB,
    X_RC,          &(dlMessageButton->object.x),
    Y_RC,          &(dlMessageButton->object.y),
    WIDTH_RC,      &(dlMessageButton->object.width),
    HEIGHT_RC,     &(dlMessageButton->object.height),
    CTRL_RC,       &(dlMessageButton->control.ctrl),
    CLR_RC,        &(dlMessageButton->control.clr),
    BCLR_RC,       &(dlMessageButton->control.bclr),
    MSG_LABEL_RC,  &(dlMessageButton->label),
    PRESS_MSG_RC,  &(dlMessageButton->press_msg),
    RELEASE_MSG_RC,&(dlMessageButton->release_msg),
    CLRMOD_RC,     &(dlMessageButton->clrmod),
    BUTTON_TYPE_RC,    &(dlMessageButton->buttonType),
    ACTIVE_LABEL_RC,   &(dlMessageButton->alabel),
    ACTIVE_COLOR_RC,   &(dlMessageButton->abclr),
    SENSITIVE_MODE_RC, &(dlMessageButton->sensitive.mode),
    SENSITIVE_CHAN_RC, &(dlMessageButton->sensitive.chan),
    -1);
  */
  updateElementObjectAttribute    (pRCB, &dlMessageButton->object);
  updateElementControlAttribute   (pRCB, &dlMessageButton->control);
  updateElementSensitiveAttribute (pRCB, &dlMessageButton->sensitive);

  dm2kGetValues(pRCB,
    MSG_LABEL_RC,      &(dlMessageButton->label),
    PRESS_MSG_RC,      &(dlMessageButton->press_msg),
    RELEASE_MSG_RC,    &(dlMessageButton->release_msg),
    CLRMOD_RC,         &(dlMessageButton->clrmod),
    BUTTON_TYPE_RC,    &(dlMessageButton->buttonType),
    ACTIVE_LABEL_RC,   &(dlMessageButton->alabel),
    ACTIVE_COLOR_RC,   &(dlMessageButton->abclr),
    -1);
}


static void  dm2kMessageButtonInfoSimple (char *msg,
		      DlMessageButton *dlMessageButton)
{
  char *strg;

  strcat (msg, " (");

  if  (dlMessageButton->sensitive.chan != NULL)
    strcat (msg, "sensitive ");

  switch ( dlMessageButton->buttonType ) {
    case PUSH_BUTTON :
      strg = "Push) ";
      break;
    case PUSH_CLOSE_BUTTON :
      strg = "Push and Close) ";
      break;
    case TOGGLE_BUTTON :
      strg = "Toggle) ";
	/* T. Straumann: I'm paranoid */
	  break;
	default: strg = "";
	 break;
    }

  strcat (msg, strg);
  /* T. Straumann: see above :-) */
  strcat (msg, dlMessageButton->control.ctrl ? dlMessageButton->control.ctrl : "");
}

static void messageButtonInfo (char *msg, Widget w,
		      DisplayInfo *displayInfo,
		      DlElement *element,
		      XtPointer objet)
{
  MessageButton *pmb;
  Record *pd;
  char strg[512];

  Boolean onstate;
  DlMessageButton *dlMessageButton;

  if (globalDisplayListTraversalMode != DL_EXECUTE) {
    dm2kMessageButtonInfoSimple (msg, element->structure.messageButton);
    return;
  }

  pmb = (MessageButton *) objet;
  pd = pmb->record;
  dlMessageButton = pmb->dlElement->structure.messageButton;

  if ( pmb->toggle ) {
    XtVaGetValues (pmb->dlElement->widget, XmNset, &onstate, NULL);
    strcat (msg, "\n    Toggle button : state = ");
    strcat (msg, onstate ? "ON" : "OFF");
  }
  else if (pmb->pushAndClose)
    strcat (msg, "\n    Push And Close display button");
  else strcat (msg, "\n    Push button");

  if ( pmb->sensitive ) {
    strcat (msg, "\n    Sensitivity controlled button");
  }

  if ( !pmb->pushAndClose || dlMessageButton->control.ctrl[0] ) {
    dm2kChanelInfo (strg, pmb->record, dlMessageButton->control.ctrl);
    sprintf (&msg[STRLEN(msg)], "\n\n  Control PV :\n%s", strg);
  }
  sensitiveInfo (msg, &dlMessageButton->sensitive, pmb->sensitiveRecord);
}
