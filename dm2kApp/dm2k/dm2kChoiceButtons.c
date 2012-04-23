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
 * .03  09-11-95        vong    conform to c++ syntax
 * 
 *****************************************************************************
 *
 * .04  02-07-97        Fabien  Addition of sensitivity
 *
 *****************************************************************************
*/

#include "dm2k.h"

typedef struct _ChoiceButtons {
  DlElement  * dlElement;
  Record     * record;
  UpdateTask * updateTask;
  Record     * sensitiveRecord; /* if the sensitivity is control by a PV */
  Boolean      sensitive;       /* sensitive property */
  int          btnNumber;       /* armed button number */
  int          pdBtnNumber;     /* saved armed menu item number */
} ChoiceButtons;

static UpdateTask * choiceButtonCreateRunTimeInstance(DisplayInfo *,DlElement *);
static void choiceButtonCreateEditInstance(DisplayInfo *, DlElement *);

static void choiceButtonDraw(XtPointer);
static void choiceButtonUpdateValueCb(XtPointer);
static void choiceButtonUpdateGraphicalInfoCb(XtPointer);
static void choiceButtonDestroyCb(XtPointer cd);
static void choiceButtonName(XtPointer, char **, short *, int *);
static void choiceButtonInheritValues(ResourceBundle *pRCB, DlElement *p);
static void choiceButtonGetValues(ResourceBundle *pRCB, DlElement *p);

static void choiceButtonInfo (char *, Widget, DisplayInfo *, DlElement *, XtPointer);
static void choiceButtonDeferredAction (DlElement *, Boolean);
static Boolean dialogDisplayType (DlElement *);
static int  getCurrentValue (ChoiceButtons *);

static void destroyDlElement (DlElement * element);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);

static DlDispatchTable choiceButtonDlDispatchTable = {
	 createDlChoiceButton,
	 destroyDlElement,
	 executeMethod,
	 writeDlChoiceButton,
	 NULL,
	 choiceButtonGetValues,
	 choiceButtonInheritValues,
	 NULL,
	 NULL,
	 genericMove,
	 genericScale,
	 NULL,
	 choiceButtonInfo,
	 choiceButtonDeferredAction
};

static void destroyDlChoiceButton (DlChoiceButton * dlChoiceButton)
{
  if (dlChoiceButton == NULL)
    return;

  objectAttributeDestroy(&(dlChoiceButton->object));
  controlAttributeDestroy(&(dlChoiceButton->control));
  sensitveAttributeDestroy(&(dlChoiceButton->sensitive));

  free((char*)dlChoiceButton);
}

static void destroyDlElement (DlElement * element)
{
  if (element == NULL || element->type != DL_ChoiceButton) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  destroyDlChoiceButton (element->structure.choiceButton);
  free((char*)element);
}

#ifdef __cplusplus
int choiceButtonFontListIndex
  (DlChoiceButton *dlChoiceButton,
   int numButtons,
   int)
#else
int choiceButtonFontListIndex
  (DlChoiceButton *dlChoiceButton,
   int numButtons,
   int maxChars)
#endif
{
  int i, useNumButtons;
  short sqrtNumButtons;

#define SHADOWS_SIZE 4		/* each Toggle Button has 2 shadows...*/

  /* pay cost of sqrt() and ceil() once */
  sqrtNumButtons = (short) ceil(sqrt((double)numButtons));
  sqrtNumButtons = MAX(1,sqrtNumButtons);
  useNumButtons = MAX(1,numButtons);


  /* more complicated calculation based on orientation, etc */
  for (i = MAX_FONTS-1; i >=  0; i--) {
    switch (dlChoiceButton->stacking) {
    case ROW:
      if ( (int)(dlChoiceButton->object.height/useNumButtons 
		 - SHADOWS_SIZE) >=
	   (fontTable[i]->ascent + fontTable[i]->descent))
	return(i);
      break;
    case ROW_COLUMN:
      if ( (int)(dlChoiceButton->object.height/sqrtNumButtons
		 - SHADOWS_SIZE) >=
	   (fontTable[i]->ascent + fontTable[i]->descent))
	return(i);
      break;
    case COLUMN:
      if ( (int)(dlChoiceButton->object.height - SHADOWS_SIZE) >=
	   (fontTable[i]->ascent + fontTable[i]->descent))
	return(i);
      break;
    }
  }
  return (0);
}

static int getCurrentValue (ChoiceButtons *pcb)
{
  Record *pd;
  int btnNumber;

  pd = pcb->record;
  if ( ! pd ) return (-1);

  if ( dialogDisplayType (pcb->dlElement) ) {
    btnNumber = ( (int)pd->value == pcb->pdBtnNumber ) ? pcb->btnNumber : (int)pd->value;
  } else btnNumber = (int)pd->value;
  return (btnNumber);
}


static Boolean dialogDisplayType (DlElement *dlElement)
{
  DisplayInfo *displayInfo;

  if ( ! dlElement ) return (False);

  displayInfo = dlElement->displayInfo;
  return ( displayInfo && (displayInfo->displayType != NORMAL_DISPLAY) );
}


static void choiceButtonSendData (
  ChoiceButtons *pcb,
  int btnNumber,
  Boolean executeFlag)      /* if True, execute in any case */
{
  Record *pd;

  if ( ! executeFlag ) {
    DisplayInfo *displayInfo;
    displayInfo = pcb->dlElement->displayInfo;
    if ( displayInfo == NULL ) return;
    if ( displayInfo->displayType != NORMAL_DISPLAY ) return;   /* deferred action */
  }

  pd = pcb->record;
  if ( pd == NULL ) return;     /* no PV attached */

  if ( btnNumber >= 0 ) {
    dm2kSendDouble(pcb->record,(double)btnNumber);
  }
}


static void choiceButtonAction (
  ChoiceButtons *pcb,
  Boolean DeferredFlag) /* if True, deferred action case */
{
  Record *pd;

  pd = pcb->record;
  if ( pd == NULL ) return;     /* no PV attached */

  if (pd->connected) {
    if (pd->writeAccess) {
      choiceButtonSendData (pcb, pcb->btnNumber, DeferredFlag);
    } else {
      fputc('\a',stderr);
      if ( ! DeferredFlag ) choiceButtonUpdateValueCb((XtPointer)pcb->record);
    }
  }
}


static void choiceButtonDeferredAction (DlElement *dlElement, Boolean applyFlag)
{
  DlChoiceButton *dlChoiceButton;
  ChoiceButtons *pcb;
  Record *pd;

  dlChoiceButton = dlElement->structure.choiceButton;
  pcb = (ChoiceButtons *) dlChoiceButton->object.runtimeDescriptor;
  if ( pcb == NULL ) return;    /* nothing instancied */

  if ( pcb->record == NULL ) return;     /* no PV attached */

  if ( applyFlag ) {
    choiceButtonAction (pcb, True);
  } else {      /* reset to current state */
    pd = pcb->record;
    if ( pd ) {
      pcb->btnNumber = (int)pd->value;
      choiceButtonDraw ((XtPointer) pcb);
    }
  }
}

void choiceButtonValueChangedCb(Widget    w,
				XtPointer clientData,
				XtPointer callbackStruct)
{
  XmToggleButtonCallbackStruct * call_data = 
    (XmToggleButtonCallbackStruct *) callbackStruct;
  int                            btnNumber = (int) clientData;
  ChoiceButtons                * pcb;
  WidgetUserData               * userData;

  /* only do ca_put if this widget actually initiated the channel change
   */
  if (call_data->event != NULL && call_data->set == True) 
    {
      /* button's parent (menuPane) has the displayInfo pointer 
       */
      XtVaGetValues(XtParent(w),XmNuserData, &userData,NULL);

      pcb = (userData ? (ChoiceButtons *) userData->privateData : NULL);
      if (pcb == NULL) {
	INFORM_INTERNAL_ERROR();
	return;
      }
    
      pcb->btnNumber = btnNumber;
      choiceButtonAction (pcb, False);
    }
}

static void choiceButtonUpdateGraphicalInfoCb(XtPointer cd) 
{
  Record         * pd = (Record *) cd;
  ChoiceButtons  * cb = (ChoiceButtons *) pd->clientData;
  DlElement      * dlElement = cb->dlElement;
  DlChoiceButton * pCB = dlElement->structure.choiceButton;
  Arg              wargs[20];
  int              i, n, maxChars, usedWidth, usedHeight;
  short            sqrtEntries;
  double           dSqrt;
  XmFontList       fontList;
  Pixel            fg, bg;
  WidgetUserData * userData;

  /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
  /* !!!!! This is a temperory work around !!!!! */
  /* !!!!! for the reconnection.           !!!!! */
  /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
  dm2kRecordAddGraphicalInfoCb(cb->record,NULL);

  /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
  /* !!!!! End work around                 !!!!! */
  /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

  if (pd->dataType != DBF_ENUM) {
    dm2kPrintf("choiceButtonUpdateGraphicalInfoCb :\n"
	       "%s\n    \"%s (%s)\" %s\n\n",
	       "Cannot create Choice Button,",
	       pCB->control.ctrl,"DBF_ENUM",
	       "is not an ENUM type!");
    dm2kPostTime();
    return;
  }

  maxChars = 0;
  for (i = 0; i <= pd->hopr; i++) {
    maxChars = MAX((size_t) maxChars,STRLEN(pd->stateStrings[i]));
  }

  fg = (pCB->clrmod == ALARM ? alarmColorPixel[pd->severity] :
	cb->updateTask->displayInfo->colormap[pCB->control.clr]);
  bg = cb->updateTask->displayInfo->colormap[pCB->control.bclr];

  n = 0;
  XtSetArg(wargs[n],XmNx,             (Position)pCB->object.x); n++;
  XtSetArg(wargs[n],XmNy,             (Position)pCB->object.y); n++;
  XtSetArg(wargs[n],XmNwidth,         (Dimension)pCB->object.width); n++;
  XtSetArg(wargs[n],XmNheight,        (Dimension)pCB->object.height); n++;
  XtSetArg(wargs[n],XmNforeground,    fg); n++;
  XtSetArg(wargs[n],XmNbackground,    bg); n++;
  XtSetArg(wargs[n],XmNindicatorOn,   (Boolean)FALSE); n++;
  XtSetArg(wargs[n],XmNmarginWidth,   0); n++;
  XtSetArg(wargs[n],XmNmarginHeight,  0); n++;
  XtSetArg(wargs[n],XmNresizeWidth,   (Boolean)FALSE); n++;
  XtSetArg(wargs[n],XmNresizeHeight,  (Boolean)FALSE); n++;
  XtSetArg(wargs[n],XmNspacing,       0); n++;
  XtSetArg(wargs[n],XmNrecomputeSize, (Boolean)FALSE); n++;


  /* alloc servise structures
   */
  userData = DM2KALLOC (WidgetUserData);
  if (userData == NULL) {
    dm2kPrintf("choiceButtonUpdateGraphicalInfoCb: memory alloc error\n");
  }
  else {
    userData->privateData    = (char*) cb;
    userData->updateTask = cb->updateTask;
    
    XtSetArg(wargs[n], XmNuserData, userData); n++;
  }

  switch (pCB->stacking) 
    {
    case ROW:
      XtSetArg(wargs[n],XmNorientation,XmVERTICAL); n++;
      usedWidth = pCB->object.width;
      usedHeight = (int) (pCB->object.height/MAX(1,pd->hopr+1));
      break;
      
    case COLUMN:
      XtSetArg(wargs[n],XmNorientation,XmHORIZONTAL); n++;
      usedWidth = (int) (pCB->object.width/MAX(1,pd->hopr+1));
      usedHeight = pCB->object.height;
      break;

    case ROW_COLUMN:
      XtSetArg(wargs[n],XmNorientation,XmVERTICAL); n++;
      dSqrt = ceil(sqrt((double)pd->hopr+1));
      sqrtEntries = MAX(2,(short)dSqrt);
      XtSetArg(wargs[n],XmNnumColumns,sqrtEntries); n++;
      usedWidth = pCB->object.width/sqrtEntries;
      usedHeight = pCB->object.height/sqrtEntries;
      break;

    default:
      dm2kPrintf( "choiceButtonUpdateGraphicalInfoCb:\n    Unknown stacking mode  = %d",pCB->stacking);
      dm2kPostTime();
      break;
    }

  dlElement->widget = 
    XmCreateRadioBox(cb->updateTask->displayInfo->drawingArea, 
		     "radioBox",wargs,n);

  /* destroy callback should free allocated memory
   */
  XtAddCallback (dlElement->widget, XmNdestroyCallback,
		 freeUserDataCB, NULL);
  
  
  /* now make push-in type radio buttons of the correct size */
  fontList = fontListTable[choiceButtonFontListIndex(
		    			     pCB,(int)pd->hopr+1,maxChars)];

  n = 0;
  XtSetArg(wargs[n],XmNindicatorOn,        False); n++;
  XtSetArg(wargs[n],XmNshadowThickness,    2); n++;
  XtSetArg(wargs[n],XmNhighlightThickness, 1); n++;
  XtSetArg(wargs[n],XmNrecomputeSize,      (Boolean)FALSE); n++;
  XtSetArg(wargs[n],XmNwidth,              (Dimension)usedWidth); n++;
  XtSetArg(wargs[n],XmNheight,             (Dimension)usedHeight); n++;
  XtSetArg(wargs[n],XmNfontList,           fontList); n++;
  XtSetArg(wargs[n],XmNalignment,          XmALIGNMENT_CENTER); n++;
  XtSetArg(wargs[n],XmNindicatorOn,        False); n++;
  XtSetArg(wargs[n],XmNindicatorSize,      0); n++;
  XtSetArg(wargs[n],XmNspacing,            0); n++;
  XtSetArg(wargs[n],XmNvisibleWhenOff,     False); n++;
  XtSetArg(wargs[n],XmNforeground,         fg); n++;
  XtSetArg(wargs[n],XmNbackground,         bg); n++;
  XtSetArg(wargs[n],XmNalignment,          XmALIGNMENT_CENTER); n++;

  for (i = 0; i <= pd->hopr; i++) {
    XmString xmStr;
    Widget   toggleButton;
    xmStr = XmStringCreateSimple(pd->stateStrings[i]);
    XtSetArg(wargs[n],XmNlabelString,xmStr);

    /* use gadgets here so that changing foreground of radioBox 
     * changes buttons 
     */
    toggleButton = XmCreateToggleButtonGadget(dlElement->widget,"toggleButton",
					      wargs,n+1);
    if (i==(int)pd->value)
      XmToggleButtonGadgetSetState(toggleButton,True,True);
    XtAddCallback(toggleButton,XmNvalueChangedCallback,
		  (XtCallbackProc)choiceButtonValueChangedCb,(XtPointer)i);

    /* MDA - for some reason, need to do this after the fact for gadgets... */
    XtVaSetValues(toggleButton,XmNalignment,XmALIGNMENT_CENTER,NULL);

    XtManageChild(toggleButton);
  }

  cb->btnNumber = (int) pd->value;      /* current active button */

  /* add in drag/drop translations */
  XtOverrideTranslations(dlElement->widget,parsedTranslations);
  choiceButtonUpdateValueCb(cd);
}


static void choiceButtonUpdateValueCb(XtPointer cd) 
{
  ChoiceButtons *pcb = (ChoiceButtons *) ((Record *) cd)->clientData;
  updateTaskMarkUpdate(pcb->updateTask);
}


static void choiceButtonDraw(XtPointer cd) 
{
  ChoiceButtons *pcb = (ChoiceButtons *) cd;
  Record *pd = pcb->record;
  Widget widget = pcb->dlElement->widget;
  DlChoiceButton *dlChoiceButton = pcb->dlElement->structure.choiceButton;

  if (pd->connected) {
    if (pd->readAccess) {
      if (widget && !XtIsManaged(widget))
        XtManageChild(widget);
      if (pd->precision < 0) return;
      if (pd->dataType == DBF_ENUM) {
        WidgetList children;
        Cardinal numChildren;
        int i;
        XtVaGetValues(widget,
		      XmNchildren,&children, XmNnumChildren,&numChildren,
		      NULL);
        /* Change the color */
        switch (dlChoiceButton->clrmod) {
	case STATIC :
	case DISCRETE :
	  break;
	case ALARM :
	  /* set alarm color */
	  XtVaSetValues(widget,XmNforeground,alarmColorPixel[pd->severity],
			NULL);
	  break;
	default :
	  dm2kPrintf("Message: Unknown color modifier!\n");
	  dm2kPrintf("Channel Name : %s\n",dlChoiceButton->control.ctrl);
	  dm2kPostMsg("Error: choiceButtonUpdateValueCb\n");
	  return;
        }
        i = getCurrentValue (pcb);
        if ((i >= 0) && (i < (int) numChildren)) {
          XmToggleButtonGadgetSetState(children[i],True,True);
	  pcb->btnNumber = i;      /* current active item */
	  pcb->pdBtnNumber = (int)pd->value;
        } else {
          dm2kPrintf("Message: Value out of range!\n");
          dm2kPrintf("Channel Name : %s\n",dlChoiceButton->control.ctrl);
          dm2kPostMsg("Error: choiceButtonUpdateValueCb\n");
          return;
        }
      } else {
        dm2kPrintf("Message: Data type must be enum!\n");
        dm2kPrintf("Channel Name : %s\n",dlChoiceButton->control.ctrl);
        dm2kPostMsg("Error: choiceButtonUpdateValueCb\n");
        return;
      }
      if (pd->writeAccess) 
        XDefineCursor(XtDisplay(widget),XtWindow(widget),rubberbandCursor);
      else
        XDefineCursor(XtDisplay(widget),XtWindow(widget),noWriteAccessCursor);

      /* sensitive Controller */
      sensitiveSetWidget (widget, &dlChoiceButton->sensitive,
			  pcb->sensitiveRecord, pcb->updateTask);

    } else {
      if (widget && XtIsManaged(widget)) XtUnmanageChild(widget);
      draw3DPane(pcb->updateTask,
		 pcb->updateTask->displayInfo->colormap[dlChoiceButton->control.bclr]);
      draw3DQuestionMark(pcb->updateTask);
    }
  } else {
    if (widget) XtUnmanageChild(widget);
    drawWhiteRectangle(pcb->updateTask);
  }
}

static UpdateTask * choiceButtonCreateRunTimeInstance
   (DisplayInfo *displayInfo,
    DlElement   *dlElement) 
{
  ChoiceButtons *pcb;
  DlChoiceButton *dlChoiceButton = dlElement->structure.choiceButton;
  UpdateTask * updateTask = NULL;

  pcb = (ChoiceButtons *) malloc(sizeof(ChoiceButtons));
  pcb->dlElement = dlElement;
  dlChoiceButton->object.runtimeDescriptor = (XtPointer) pcb;

  pcb->sensitive = False;   /* to be set by sensitiveCreateRecord */

  updateTask = pcb->updateTask = updateTaskAddTask(displayInfo,
						   &(dlChoiceButton->object),
						   choiceButtonDraw,
						   (XtPointer) pcb);
  if (pcb->updateTask == NULL) {
    dm2kPrintf("choiceButtonCreateRunTimeInstance : memory allocation error\n");
  } else {
    updateTaskAddDestroyCb(pcb->updateTask,choiceButtonDestroyCb);
    updateTaskAddNameCb(pcb->updateTask,choiceButtonName);
  }

  pcb->record = dm2kAllocateRecord(dlChoiceButton->control.ctrl,
				   choiceButtonUpdateValueCb,
				   choiceButtonUpdateGraphicalInfoCb,
				   (XtPointer) pcb);

  if (pcb->record)
    pcb->btnNumber = (int) pcb->record->value;
  pcb->pdBtnNumber = -1;    /* not a valid value ! */

  pcb->sensitiveRecord =  sensitiveCreateRecord (&dlChoiceButton->sensitive,
						 choiceButtonUpdateValueCb,
						 NULL,
						 (XtPointer) pcb,
						 &pcb->sensitive);


  /* put up white rectangle so that unconnected channels are obvious */
  drawWhiteRectangle(pcb->updateTask);

  return updateTask;
}

static void choiceButtonCreateEditInstance (
					    DisplayInfo *displayInfo, 
					    DlElement *dlElement) 
{
  Arg            args[24];
  int            i, n;
  XmString       buttons[2];
  XmButtonType   buttonType[2];
  WidgetList     children;
  Cardinal       numChildren;
  int            usedWidth, usedHeight;
  XmFontList     fontList;
  DlChoiceButton *dlChoiceButton = dlElement->structure.choiceButton;

  buttons[0] = XmStringCreateSimple("0...");
  buttons[1] = XmStringCreateSimple("1...");
  buttonType[0] = XmRADIOBUTTON;
  buttonType[1] = XmRADIOBUTTON;

  n = 0;
  XtSetArg(args[n],XmNx,(Position)dlChoiceButton->object.x); n++;
  XtSetArg(args[n],XmNy,(Position)dlChoiceButton->object.y); n++;
  XtSetArg(args[n],XmNwidth,(Dimension)dlChoiceButton->object.width); n++;
  XtSetArg(args[n],XmNheight,(Dimension)dlChoiceButton->object.height); n++;
  XtSetArg(args[n],XmNforeground,(Pixel)
	   displayInfo->colormap[dlChoiceButton->control.clr]); n++;
  XtSetArg(args[n],XmNbackground,(Pixel)
	   displayInfo->colormap[dlChoiceButton->control.bclr]); n++;
  XtSetArg(args[n],XmNindicatorOn,(Boolean)FALSE); n++;
  XtSetArg(args[n],XmNmarginWidth,0); n++;
  XtSetArg(args[n],XmNmarginHeight,0); n++;
  XtSetArg(args[n],XmNrecomputeSize,(Boolean)FALSE); n++;
  XtSetArg(args[n],XmNresizeWidth,FALSE); n++;
  XtSetArg(args[n],XmNresizeHeight,FALSE); n++;
  XtSetArg(args[n],XmNspacing,0); n++;
  XtSetArg(args[n],XmNbuttonCount,2); n++;
  XtSetArg(args[n],XmNbuttonType,buttonType); n++;
  XtSetArg(args[n],XmNbuttons,buttons); n++;
  XtSetArg(args[n],XmNbuttonSet,0); n++;
  
  switch (dlChoiceButton->stacking) 
    {
    case COLUMN:
      usedWidth = dlChoiceButton->object.width/2;
      usedHeight = dlChoiceButton->object.height;
      XtSetArg(args[n],XmNorientation,XmHORIZONTAL); n++;
      break;
      
    case ROW:
      usedWidth = dlChoiceButton->object.width;
      usedHeight = dlChoiceButton->object.height/2;
      XtSetArg(args[n],XmNorientation,XmVERTICAL); n++;
      break;
      
    case ROW_COLUMN:
      usedWidth = dlChoiceButton->object.width/2;
      usedHeight = dlChoiceButton->object.height/2;
      XtSetArg(args[n],XmNorientation,XmVERTICAL); n++;
      XtSetArg(args[n],XmNnumColumns,2); n++;
      break;
    default:
      dm2kPrintf( "\nexecuteDlChoiceButton: unknown stacking = %d",
		  dlChoiceButton->stacking);
      break;
    }
  
  /*** (MDA)  ChoiceButton is really a radio box and push button children ***/
  dlElement->widget = XmCreateSimpleRadioBox(displayInfo->drawingArea,
					     "radioBox", args, n);
  
  /* remove all translations if in edit mode */
  XtUninstallTranslations(dlElement->widget);
  
  /* now make push-in type radio box...*/
  XtVaGetValues(dlElement->widget,
		XmNchildren, &children,
		XmNnumChildren,&numChildren,
		NULL);
  
  fontList = fontListTable[choiceButtonFontListIndex(dlChoiceButton,2,4)];
  
  for (i = 0; i < numChildren; i++) {
    XtUninstallTranslations(children[i]);
    XtVaSetValues(children[i],
		  XmNindicatorOn,        False,
		  XmNshadowThickness,    2,
		  XmNhighlightThickness, 1,
		  XmNfontList,           fontList,
		  XmNwidth,              (Dimension)usedWidth,
		  XmNheight,             (Dimension)usedHeight,
		  XmNrecomputeSize,      (Boolean)FALSE,
		  NULL);
  }
  
  XtManageChild(dlElement->widget);
  
  XmStringFree(buttons[0]);
  XmStringFree(buttons[1]);
  
  controlHandler (displayInfo, dlElement); /*checkhere*/
}

static UpdateTask * executeMethod 
   (DisplayInfo * displayInfo, 
    DlElement   * dlElement)
{
   UpdateTask * updateTask;
  
  if (displayInfo->traversalMode == DL_EXECUTE) 
    {
      updateTask = choiceButtonCreateRunTimeInstance(displayInfo, dlElement);
    } 
  else if (displayInfo->traversalMode == DL_EDIT) 
    {
      if (dlElement->widget) 
	XtDestroyWidget(dlElement->widget);

      choiceButtonCreateEditInstance(displayInfo, dlElement);
    }
  return NULL;
}


static void choiceButtonDestroyCb(XtPointer cd) 
{
  ChoiceButtons *pcb = (ChoiceButtons *) cd;

  if (pcb) {
    dm2kDestroyRecord(pcb->record);
    sensitiveDestroyRecord (&pcb->sensitiveRecord, &pcb->sensitive);
    free((char *)pcb);
  }
}

static void choiceButtonName
  (XtPointer cd, 
   char      **name, 
   short     *severity, 
   int       *count)
{
  ChoiceButtons *pcb = (ChoiceButtons *) cd;

  *count = 1;
  name[0] = pcb->record->name;
  severity[0] = pcb->record->severity;
}


DlElement *createDlChoiceButton(DlElement *p)
{
  DlChoiceButton * dlChoiceButton;
  DlElement      * dlElement;
 
  dlChoiceButton = DM2KALLOC(DlChoiceButton);

  if (dlChoiceButton == NULL) 
    return 0;

  if (p != NULL) {
    objectAttributeCopy(&dlChoiceButton->object, 
			&p->structure.choiceButton->object);
    controlAttributeCopy(&dlChoiceButton->control, 
			 &p->structure.choiceButton->control);
    sensitveAttributeCopy(&(dlChoiceButton->sensitive), 
			 &p->structure.choiceButton->sensitive);

    dlChoiceButton->clrmod   = p->structure.choiceButton->clrmod;
    dlChoiceButton->stacking = p->structure.choiceButton->stacking;
  } 
  else {
    objectAttributeInit(&(dlChoiceButton->object));
    controlAttributeInit(&(dlChoiceButton->control));
    sensitveAttributeInit(&(dlChoiceButton->sensitive));

    dlChoiceButton->clrmod   = STATIC;
    dlChoiceButton->stacking = ROW;
  }
 
  dlElement = createDlElement(DL_ChoiceButton,
			      (XtPointer)      dlChoiceButton,
			      &choiceButtonDlDispatchTable);
  if (dlElement == NULL)
    destroyDlChoiceButton(dlChoiceButton);
 
  return(dlElement);
}

DlElement *parseChoiceButton(DisplayInfo *displayInfo)
{
  char             token[MAX_TOKEN_LENGTH];
  TOKEN            tokenType;
  int              nestingLevel = 0;
  DlChoiceButton * dlChoiceButton;
  DlElement      * dlElement = createDlChoiceButton(NULL);
 
  if (dlElement == NULL)
    return 0;

  dlChoiceButton = dlElement->structure.choiceButton;
 
  do {
    switch((tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"object")) 
	{
	  parseObject(displayInfo,&(dlChoiceButton->object));
	} 
      else if (STREQL(token,"control")) 
	{
	  parseControl(displayInfo,&(dlChoiceButton->control));
	} 
      else if (STREQL(token,"sensitive"))
	{
	  /* add on for sensitivity property */
	  parseSensitive (displayInfo, &(dlChoiceButton->sensitive));
	}
      else if (STREQL(token,"clrmod")) 
	{
	  getToken(displayInfo,token);
	  getToken(displayInfo,token);

	  if (STREQL(token,"static"))
	    dlChoiceButton->clrmod = STATIC;
	  else if (STREQL(token,"alarm"))
	    dlChoiceButton->clrmod = ALARM;
	  else if (STREQL(token,"discrete"))
	    dlChoiceButton->clrmod = DISCRETE;
	}
      else if (STREQL(token,"stacking")) 
	{
	  getToken(displayInfo,token);
	  getToken(displayInfo,token);

	  if (STREQL(token,"row"))
	    dlChoiceButton->stacking = ROW;
	  else if (STREQL(token,"column"))
	    dlChoiceButton->stacking = COLUMN;
	  else if (STREQL(token,"row column"))
	    dlChoiceButton->stacking = ROW_COLUMN;
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

void writeDlChoiceButton
  (FILE *stream,
   DlElement *dlElement,
   int level)
{
  char indent[256]; level=MIN(level,256-2);
  DlChoiceButton *dlChoiceButton = dlElement->structure.choiceButton;
 
  level = MIN(level, 256-2);
  memset(indent,'\t',level);
  indent[level] = '\0';

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif 
    fprintf(stream,"\n%s\"choice button\" {",indent);
    writeDlObject(stream,&(dlChoiceButton->object),level+1);
    writeDlControl(stream,&(dlChoiceButton->control),level+1);
    writeDlSensitive(stream,&(dlChoiceButton->sensitive),level+1);

    if (dlChoiceButton->clrmod != STATIC) 
      fprintf(stream,"\n%s\tclrmod=\"%s\"",indent,
	      stringValueTable[dlChoiceButton->clrmod]);

    if (dlChoiceButton->stacking != ROW)
      fprintf(stream,"\n%s\tstacking=\"%s\"",indent,
	      stringValueTable[dlChoiceButton->stacking]);
    fprintf(stream,"\n%s}",indent);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    fprintf(stream,"\n%s\"choice button\" {",indent);
    writeDlObject(stream,&(dlChoiceButton->object),level+1);
    writeDlControl(stream,&(dlChoiceButton->control),level+1);
    writeDlSensitive(stream,&(dlChoiceButton->sensitive),level+1);
    fprintf(stream,"\n%s\tclrmod=\"%s\"",indent,
	    stringValueTable[dlChoiceButton->clrmod]);
    fprintf(stream,"\n%s\tstacking=\"%s\"",indent,
	    stringValueTable[dlChoiceButton->stacking]);
    fprintf(stream,"\n%s}",indent);
  }
#endif
}

static void choiceButtonInheritValues(ResourceBundle *pRCB, DlElement *p) {
  DlChoiceButton *dlChoiceButton = p->structure.choiceButton;

  updateElementControlAttribute   (pRCB, &dlChoiceButton->control);
  updateElementSensitiveAttribute (pRCB, &dlChoiceButton->sensitive);

  dm2kGetValues(pRCB,
		CLRMOD_RC,     &(dlChoiceButton->clrmod),
		STACKING_RC,   &(dlChoiceButton->stacking),
		-1);
}

static void choiceButtonGetValues(ResourceBundle *pRCB, DlElement *p) {
  DlChoiceButton *dlChoiceButton = p->structure.choiceButton;

  updateElementObjectAttribute    (pRCB, &dlChoiceButton->object);
  updateElementControlAttribute   (pRCB, &dlChoiceButton->control);
  updateElementSensitiveAttribute (pRCB, &dlChoiceButton->sensitive);

  dm2kGetValues(pRCB,
		CLRMOD_RC,     &(dlChoiceButton->clrmod),
		STACKING_RC,   &(dlChoiceButton->stacking),
		-1);
}


static void choiceButtonInfo (char *msg, Widget w,
			      DisplayInfo *displayInfo,
			      DlElement *element,
			      XtPointer objet)
{
  ChoiceButtons *pcb;
  DlChoiceButton *dlChoiceButton;

  if (globalDisplayListTraversalMode != DL_EXECUTE) {
    dlChoiceButton = element->structure.choiceButton;
    sensitiveControllerInfoSimple (msg, &dlChoiceButton->control,
				   &dlChoiceButton->sensitive);
    return;
  }

  pcb = (ChoiceButtons *) objet;
  dlChoiceButton = pcb->dlElement->structure.choiceButton;

  sensitiveControllerInfo (msg,
			   &dlChoiceButton->control, pcb->record,
			   &dlChoiceButton->sensitive, pcb->sensitiveRecord);
}
