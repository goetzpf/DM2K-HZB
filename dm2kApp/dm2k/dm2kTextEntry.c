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
 * .04  09-22-95        vong    accept hexidecimal input
 * .05  12-01-95        vong    fixes the precision = -1 in the valueToString
 *                              function
 *
 *****************************************************************************
 *
 * .04  02-07-97        Fabien  Addition of sensitivity
 *
 *****************************************************************************
*/

#include "dm2k.h"
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <cvtFast.h>
#ifdef __cplusplus
}
#endif

typedef struct _TextEntry {
  DlElement   * dlElement;
  Record      * record;
  UpdateTask  * updateTask;
  Boolean       updateAllowed;
  Record      * sensitiveRecord;  /* if the sensitivity is control by a PV */
  Boolean       sensitive;        /* sensitive property */
  char          textField[MAX_TOKEN_LENGTH];     /* current record value */
  char          pdTextField[MAX_TOKEN_LENGTH];   /* saved record value */
} TextEntry;

static UpdateTask * textEntryCreateRunTimeInstance(DisplayInfo *, DlElement *);
static void textEntryCreateEditInstance(DisplayInfo *,DlElement *);

static void textEntryDraw(XtPointer cd);
static void textEntryUpdateValueCb(XtPointer cd);
static void textEntryDestroyCb(XtPointer cd);
static void textEntryValueChanged(Widget, XtPointer, XtPointer);
static void textEntryModifyVerifyCallback(Widget, XtPointer, XtPointer);
static char *valueToString(TextEntry *, TextFormat format);
static void textEntryName(XtPointer, char **, short *, int *);
static void textEntryInheritValues(ResourceBundle *pRCB, DlElement *p);
static void textEntryGetValues(ResourceBundle *pRCB, DlElement *p);

static void textEntryInfo (char *, Widget, DisplayInfo *, DlElement *, XtPointer);

static void textEntryDeferredAction (DlElement *, Boolean);
static Boolean dialogDisplayType (DlElement *);
static char *getCurrentValue (TextEntry *);

static void destroyDlElement (DlElement * element);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);

static DlDispatchTable textEntryDlDispatchTable = {
	 createDlTextEntry,
	 destroyDlElement,
	 executeMethod,
	 writeDlTextEntry,
	 NULL,
	 textEntryGetValues,
	 textEntryInheritValues,
	 NULL,
	 NULL,
	 genericMove,
	 genericScale,
	 NULL,
	 textEntryInfo,
	 textEntryDeferredAction
};


static void destroyDlTextEntry (DlTextEntry * dlTextEntry)
{
  if (dlTextEntry == NULL)
    return;

  objectAttributeDestroy(&(dlTextEntry->object));
  controlAttributeDestroy(&(dlTextEntry->control));
  sensitveAttributeDestroy(&(dlTextEntry->sensitive));

  free((char*)dlTextEntry);
}

static void destroyDlElement (DlElement * element)
{
  if (element == NULL || element->type != DL_TextEntry) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  destroyDlTextEntry (element->structure.textEntry);
  free((char*)element);
}

int textFieldFontListIndex(int height)
{
  int i;
  /* don't allow height of font to exceed 90% - 4 pixels of textField widget
   *	(includes nominal 2*shadowThickness=2 shadow)
   */
  for (i = MAX_FONTS-1; i >=  0; i--) {
    if ( ((int)(.90*height) - 4) >=
	 (fontTable[i]->ascent + fontTable[i]->descent))
      return(i);
  }
  return (0);
}

static char *valueToString(TextEntry *pte, TextFormat format) {
  Record *pd = pte->record;
  static char textField[MAX_TOKEN_LENGTH];
  double value;
  short precision = 0;
  textField[0] = '\0';
  switch(pd->dataType) {
  case DBF_STRING :
    if (pd->array) {
      strncpy(textField,(const char *)pd->array, MAX_TEXT_UPDATE_WIDTH-1);
      textField[MAX_TEXT_UPDATE_WIDTH-1] = '\0';
    }
    return textField;
  case DBF_ENUM :
    if ((pd->precision >= 0) && (pd->hopr+1 > 0)) {
      int i = (int) pd->value;
      /* getting values of -1 for data->value for invalid connections */
	  /* T. Straumann: check for null ptr */
      if ( i >= 0 && i < (int) pd->hopr+1 && pd->stateStrings[i]) {
	strncpy(textField,(const char*)pd->stateStrings[i], 
		MAX_TEXT_UPDATE_WIDTH-1);
	textField[MAX_TEXT_UPDATE_WIDTH-1] = '\0';
	return textField;
      } else {
	return " ";
      }
    } else {
      value = pd->value;
    }
    break;
  case DBF_CHAR :
    if (format == STRING) {
      if (pd->array) {
	strncpy(textField, (const char*)pd->array, 
		MIN(pd->elementCount,(MAX_TOKEN_LENGTH-1)));
	textField[MAX_TOKEN_LENGTH-1] = '\0';
      }
      return textField;
    }
  case DBF_INT :
  case DBF_LONG :
  case DBF_FLOAT :
  case DBF_DOUBLE :
    precision = pd->precision;
    value = pd->value;
    break;
  default :
    dm2kPrintf("Name  : %s\n",pte->dlElement->structure.textEntry->control.ctrl);
    dm2kPrintf("Error : valueToString\n");
    dm2kPostMsg("Msg   : Unknown Data Type!\n");
    return "Error!";
  }
  if (precision < 0) {
    precision = 0;
  }
  switch (format) {
  case DECIMAL:
  case STRING:
    cvtDoubleToString(value,textField,precision);
    break;
  case EXPONENTIAL:
    cvtDoubleToExpString(value,textField,precision);
    break;
  case ENGR_NOTATION:
    localCvtDoubleToExpNotationString(value,textField,precision);
    break;
  case COMPACT:
    cvtDoubleToCompactString(value,textField,precision);
    break;
  case TRUNCATED:
    cvtLongToString((long)value,textField);
    break;
  case HEXADECIMAL:
    localCvtLongToHexString((long)value, textField);
    break;
  case OCTAL:
    cvtLongToOctalString((long)value, textField);
    break;
  default :
    dm2kPrintf("Name  : %s\n",pte->dlElement->structure.textEntry->control.ctrl);
    dm2kPrintf("Error : valueToString\n");
    dm2kPostMsg("Msg   : Unknown Format Type!\n");
    return "Error!";
  }
  return textField;
}
 
/***
 *** Text Entry
 ***/
static UpdateTask * textEntryCreateRunTimeInstance
   (DisplayInfo *displayInfo,
    DlElement   *dlElement) 
{
  TextEntry      * pte;
  Arg              args[20];
  int              n;
  DlTextEntry    * dlTextEntry = dlElement->structure.textEntry;
  WidgetUserData * userData;
  UpdateTask * updateTask = NULL;

  /* alloc servise structures
   */
  userData = DM2KALLOC (WidgetUserData);
  pte = DM2KALLOC(TextEntry);
  if (pte == NULL || userData == NULL) {
    DM2KFREE(userData);
    DM2KFREE(pte);
    dm2kPrintf("UpdateTask: memory allocation error\n");
    return updateTask;
  }

  pte->dlElement = dlElement;
  dlTextEntry->object.runtimeDescriptor = (XtPointer) pte;
  pte->sensitive = False;   /* to be set by sensitiveCreateRecord */

  updateTask = pte->updateTask = updateTaskAddTask(displayInfo,
						   &(dlTextEntry->object),
						   textEntryDraw,
						   (XtPointer)pte);

  if (pte->updateTask == NULL) {
    dm2kPrintf("menuCreateRunTimeInstance : memory allocation error\n");
  } else {
    updateTaskAddDestroyCb(pte->updateTask,textEntryDestroyCb);
    updateTaskAddNameCb(pte->updateTask,textEntryName);
  }
  pte->record = dm2kAllocateRecord(dlTextEntry->control.ctrl,
				   textEntryUpdateValueCb,
				   NULL,
				   (XtPointer) pte);

  pte->sensitiveRecord =  sensitiveCreateRecord (&dlTextEntry->sensitive,
						 textEntryUpdateValueCb,
						 NULL,
						 (XtPointer) pte,
						 &pte->sensitive);

  pte->updateAllowed = True;
  drawWhiteRectangle(pte->updateTask);

  /* from the text entry structure, we've got TextEntry's specifics 
   */
  n = 0;
  XtSetArg(args[n],XmNx,(Position)dlTextEntry->object.x); n++;
  XtSetArg(args[n],XmNy,(Position)dlTextEntry->object.y); n++;
  XtSetArg(args[n],XmNwidth,(Dimension)dlTextEntry->object.width); n++;
  XtSetArg(args[n],XmNheight,(Dimension)dlTextEntry->object.height); n++;
  XtSetArg(args[n],XmNforeground,(Pixel)
	   displayInfo->colormap[dlTextEntry->control.clr]); n++;
  XtSetArg(args[n],XmNbackground,(Pixel)
	   displayInfo->colormap[dlTextEntry->control.bclr]); n++;
  XtSetArg(args[n],XmNhighlightThickness,1); n++;
  XtSetArg(args[n],XmNhighlightOnEnter,TRUE); n++;
  XtSetArg(args[n],XmNindicatorOn,(Boolean)FALSE); n++;
  XtSetArg(args[n],XmNresizeWidth,(Boolean)FALSE); n++;
  XtSetArg(args[n],XmNmarginWidth,
	   ( (dlTextEntry->object.height <= 2*GOOD_MARGIN_DIVISOR)
	     ?  0 : (dlTextEntry->object.height/GOOD_MARGIN_DIVISOR)) ); n++;
  XtSetArg(args[n],XmNmarginHeight,
	   ( (dlTextEntry->object.height <= 2*GOOD_MARGIN_DIVISOR) 
	     ?  0 : (dlTextEntry->object.height/GOOD_MARGIN_DIVISOR)) ); n++;
  XtSetArg(args[n],XmNfontList,fontListTable[textFieldFontListIndex(dlTextEntry->object.height)]); n++;

  userData->privateData    = (char*) pte;
  userData->updateTask = (pte ? pte->updateTask: NULL);
  XtSetArg(args[n],XmNuserData,(XtPointer) userData); n++;
										     
  dlElement->widget = XtCreateWidget("textField", xmTextFieldWidgetClass,
				     displayInfo->drawingArea, args, n);

  /* destroy callback should free allocated memory
   */
  XtAddCallback (dlElement->widget, XmNdestroyCallback,
		 freeUserDataCB, NULL);
  
  /* add in drag/drop translations 
   */
  XtOverrideTranslations(dlElement->widget,parsedTranslations);

  /* add the callbacks for update */
  XtAddCallback(dlElement->widget,XmNactivateCallback,
		(XtCallbackProc)textEntryValueChanged, (XtPointer)pte);

  /* special stuff: if user started entering new data into text field, but
   *  doesn't do the actual Activate <CR>, then restore old value on
   *  losing focus...
   */
  XtAddCallback(dlElement->widget,XmNmodifyVerifyCallback,
		(XtCallbackProc)textEntryModifyVerifyCallback,(XtPointer)pte);
  return updateTask;
}

static void textEntryCreateEditInstance(DisplayInfo *displayInfo,
				 DlElement *dlElement) {
  Arg args[20];
  int n;
  Widget localWidget;
  DlTextEntry *dlTextEntry = dlElement->structure.textEntry;

  /* from the text entry structure, we've got TextEntry's specifics */
  n = 0;
  XtSetArg(args[n],XmNx,(Position)dlTextEntry->object.x); n++;
  XtSetArg(args[n],XmNy,(Position)dlTextEntry->object.y); n++;
  XtSetArg(args[n],XmNwidth,(Dimension)dlTextEntry->object.width); n++;
  XtSetArg(args[n],XmNheight,(Dimension)dlTextEntry->object.height); n++;
  XtSetArg(args[n],XmNforeground,(Pixel)
	   displayInfo->colormap[dlTextEntry->control.clr]); n++;
  XtSetArg(args[n],XmNbackground,(Pixel)
	   displayInfo->colormap[dlTextEntry->control.bclr]); n++;
	   XtSetArg(args[n],XmNhighlightThickness,1); n++;
	   XtSetArg(args[n],XmNhighlightOnEnter,TRUE); n++;
	   XtSetArg(args[n],XmNindicatorOn,(Boolean)FALSE); n++;
	   XtSetArg(args[n],XmNresizeWidth,(Boolean)FALSE); n++;
	   XtSetArg(args[n],XmNmarginWidth,
		    ( (dlTextEntry->object.height <= 2*GOOD_MARGIN_DIVISOR)
		      ?  0 : (dlTextEntry->object.height/GOOD_MARGIN_DIVISOR)) ); n++;
  XtSetArg(args[n],XmNmarginHeight,
	   ( (dlTextEntry->object.height <= 2*GOOD_MARGIN_DIVISOR)
	     ?  0 : (dlTextEntry->object.height/GOOD_MARGIN_DIVISOR)) ); n++;
  XtSetArg(args[n],XmNfontList,
	   fontListTable[textFieldFontListIndex(dlTextEntry->object.height)]); n++;
					     
  dlElement->widget = localWidget = 
    XtCreateWidget("textField",
		   xmTextFieldWidgetClass, displayInfo->drawingArea, args, n);
  
  controlHandler (displayInfo, dlElement);
  
  XtManageChild(localWidget);
}


static UpdateTask * executeMethod
   (DisplayInfo * displayInfo, 
    DlElement   * dlElement)
{
  UpdateTask * updateTask = NULL;

  if (displayInfo->traversalMode == DL_EXECUTE) {
     updateTask = textEntryCreateRunTimeInstance(displayInfo,dlElement);
  } else
    if (displayInfo->traversalMode == DL_EDIT) {
      if (dlElement->widget) {
	DlObject *po = &(dlElement->structure.textEntry->object);
	XtVaSetValues(dlElement->widget,
		      XmNx, (Position) po->x,
		      XmNy, (Position) po->y,
		      XmNwidth, (Dimension) po->width,
		      XmNheight, (Dimension) po->height,
		      NULL);
      } else {
	textEntryCreateEditInstance(displayInfo,dlElement);
      }
    }

  return updateTask;
}

static void textEntryUpdateValueCb(XtPointer cd) {
  TextEntry *pte = (TextEntry *) ((Record *) cd)->clientData;
  updateTaskMarkUpdate(pte->updateTask);
}

static void textEntryDraw(XtPointer cd) {
  TextEntry *pte = (TextEntry *) cd;
  Record *pd = pte->record;
  Widget widget = pte->dlElement->widget;
  DlTextEntry *dlTextEntry = pte->dlElement->structure.textEntry;
  XmTextPosition rightsel, leftsel, cursorpos;
  Boolean isSel;
  
  if (pd && pd->connected) {
    if (pd->readAccess) {
      if (widget) 
        XtManageChild(widget);
      else
        return;
      if (pd->writeAccess) {
	XtVaSetValues(widget,XmNeditable,True,NULL);
	XDefineCursor(XtDisplay(widget),XtWindow(widget),rubberbandCursor);
      } else {
	XtVaSetValues(widget,XmNeditable,False,NULL);
	XDefineCursor(XtDisplay(widget),XtWindow(widget),noWriteAccessCursor);
        pte->updateAllowed = True;
      }
      if (pte->updateAllowed) {
	char * strg = getCurrentValue (pte);
	/* T. Straumann: protect against null rval */
	if (!strg) strg="";

	 isSel = XmTextFieldGetSelectionPosition(widget, &leftsel, &rightsel);
	 cursorpos = XmTextFieldGetInsertionPosition(widget);
	 
	XmTextFieldSetString (widget, strg);
	strcpy (pte->textField, strg);
	strcpy (pte->pdTextField, valueToString (pte, dlTextEntry->format));

        switch (dlTextEntry->clrmod) {
	case STATIC :
	case DISCRETE:
	  break;
	case ALARM:
	  XtVaSetValues(widget,
			XmNforeground,alarmColorPixel[pd->severity],
			NULL);
	  break;
        }
	if (isSel) 
	   XmTextFieldSetSelection( widget, leftsel, rightsel, 1 );
	XmTextFieldSetInsertionPosition( widget, cursorpos );
      }

      /* sensitive Controller */
      sensitiveSetWidget (widget, &dlTextEntry->sensitive,
			  pte->sensitiveRecord, pte->updateTask);

    } else {
      draw3DPane(pte->updateTask,
		 pte->updateTask->displayInfo->colormap[dlTextEntry->control.bclr]);
      draw3DQuestionMark(pte->updateTask);
      if (widget) XtUnmanageChild(widget);
    }
  } else {
    if (widget) XtUnmanageChild(widget);
    drawWhiteRectangle(pte->updateTask);
  }
}

static void textEntryDestroyCb(XtPointer cd) {
  TextEntry *pte = (TextEntry *) cd;
  if (pte) {
    dm2kDestroyRecord(pte->record);
    sensitiveDestroyRecord (&pte->sensitiveRecord, &pte->sensitive);
    free((char *)pte);
  }
  return;
}

/*
 * TextEntry special handling:  if user starts editing text field,
 *  then be sure to update value on losingFocus (since until activate,
 *  the value isn't ca_put()-ed, and the text field can be inconsistent
 *  with the underlying channel
 */
#ifdef __cplusplus
static void textEntryLosingFocusCallback(
					 Widget w,
					 XtPointer cd,
					 XtPointer)
#else
     static void textEntryLosingFocusCallback(
					      Widget w,
					      XtPointer cd,
					      XtPointer cbs)
#endif
{
  TextEntry *pte = (TextEntry *) cd;
  XtRemoveCallback(w,XmNlosingFocusCallback,
		   (XtCallbackProc)textEntryLosingFocusCallback,pte);
  pte->updateAllowed = True;
  XmTextFieldClearSelection(pte->dlElement->widget, 0);
  textEntryUpdateValueCb((XtPointer)pte->record);
}


static void textEntryModifyVerifyCallback(
   Widget w,
   XtPointer clientData,
   XtPointer pCallbackData)
{
  TextEntry *pte = (TextEntry *) clientData;
  XmTextVerifyCallbackStruct *pcbs = (XmTextVerifyCallbackStruct *) pCallbackData;

  /* NULL event means value changed programmatically; hence don't process */
  if (pcbs->event != NULL) {
    switch (XtHasCallbacks(w,XmNlosingFocusCallback)) {
    case XtCallbackNoList:
    case XtCallbackHasNone:
      XtAddCallback(w,XmNlosingFocusCallback,
		    (XtCallbackProc)textEntryLosingFocusCallback,pte);
      pte->updateAllowed = False; 
      break;
    case XtCallbackHasSome:
      break;
    }
    pcbs->doit = True;
  }

}


static char *getCurrentValue (TextEntry *pte)
{
  Record *pd;
  char *strg;

  pd = pte->record;
  if ( ! pd ) return ("Not Connected !");

  /* string from record */
  strg = valueToString (pte, pte->dlElement->structure.textEntry->format);
  if ( dialogDisplayType (pte->dlElement) ) {
	/* T. Straumann: better be paranoid about pointer values */
    if (pte->pdTextField && strcmp (strg, pte->pdTextField) == 0 ) strg = pte->textField;
  }
  return (strg);
}



static Boolean dialogDisplayType (DlElement *dlElement)
{
  DisplayInfo *displayInfo;

  if ( ! dlElement ) return (False);

  displayInfo = dlElement->displayInfo;
  return ( displayInfo && (displayInfo->displayType != NORMAL_DISPLAY) );
}


static void textEntrySendData (
  TextEntry *pte,
  char *textValue,
  Boolean executeFlag)      /* if True, execute in any case */
{
  Record *pd;
  double value;
  Widget w;

  if ( ! textValue ) return;

  if ( ! executeFlag ) {
    DisplayInfo *displayInfo;
    displayInfo = pte->dlElement->displayInfo;
    if ( displayInfo != NULL ) {
       if ( displayInfo->displayType != NORMAL_DISPLAY ) {
	  w = pte->dlElement->widget;
	  XtRemoveCallback (w ,XmNlosingFocusCallback,
			    (XtCallbackProc)textEntryLosingFocusCallback, pte);
	  pte->updateAllowed = True;
	  return;   /* dialog display : request deferred action */
       }
    }
  }

  pd = pte->record;
  if ( pd == NULL ) return;     /* no PV attached */

  switch (pd->dataType) {
    case DBF_STRING:
      if (STRLEN(textValue) >= (size_t) MAX_STRING_SIZE)
	textValue[MAX_STRING_SIZE-1] = '\0';
      dm2kSendString(pte->record,textValue);
      break;
    case DBF_CHAR:
      if (pte->dlElement->structure.textEntry->format == STRING) {
	unsigned long len =
	  MIN((unsigned long)pd->elementCount,
	      (unsigned long)(STRLEN(textValue)+1));
	textValue[len-1] = '\0';
	dm2kSendCharacterArray(pte->record,textValue,len);
	break;
      }
    default:
      if ((STRLEN(textValue) > (size_t) 2) && (textValue[0] == '0')
	&& (textValue[1] == 'x' || textValue[1] == 'X')) {
	unsigned long longValue;
	longValue = strtoul(textValue,NULL,16);
	value = (double) longValue;
      } else {
	value = (double) atof(textValue);
      }
      dm2kSendDouble(pte->record,value);
      break;
  }
/* T. Straumann: Don't free textValue twice !!!
 *				 it will be freed by the caller
  XtFree(textValue);
 */
}


static void textEntryAction (
  TextEntry *pte,
  Widget w,
  Boolean DeferredFlag) /* if True, deferred action case */
{
  Record *pd;
  char *textValue;
  if ( ! w ) return;
  pd = pte->record;
  if ( pd == NULL ) return;     /* no PV attached */

  if ( ! (textValue = XmTextFieldGetString(w)) ) return;

  if ( pd->connected && pd->writeAccess ) {
    textEntrySendData (pte, textValue, DeferredFlag);
  }
  XtFree(textValue);
}


static void textEntryDeferredAction (DlElement *dlElement, Boolean applyFlag)
{
  DlTextEntry *dlTextEntry;
  TextEntry *pte;
  Widget w;
  char *strg;

  w = dlElement->widget;
  dlTextEntry = dlElement->structure.textEntry;
  pte = (TextEntry *) dlTextEntry->object.runtimeDescriptor;
  if ( pte == NULL ) return;    /* nothing instancied */

  if ( pte->record == NULL ) return;     /* no PV attached */

  if ( applyFlag ) {
    textEntryAction (pte, w, True);
  } else {      /* reset to current state */
    strg = valueToString (pte, dlTextEntry->format);  /* string from record */
    strcpy (pte->textField, strg);
    textEntryDraw ((XtPointer) pte);
  }
}

#ifdef __cplusplus
static void textEntryValueChanged(Widget w, XtPointer clientData, XtPointer)
#else
static void textEntryValueChanged(Widget w, XtPointer clientData, XtPointer dummy)
#endif
{
  TextEntry * pte = (TextEntry *) clientData;
  char      * strg = XmTextFieldGetString(w);

  strcpy (pte->textField, strg);
  /* T. Straumann: strg must be freed (read the docs) */
  XtFree(strg);
  textEntryAction (pte, w, False);
}

static void textEntryName(XtPointer cd, 
			  char **name, 
			  short *severity,
			  int *count)
{
  TextEntry *pte = (TextEntry *) cd;
  *count = 1;
  name[0] = pte->record->name;
  severity[0] = pte->record->severity;
}

DlElement *createDlTextEntry(DlElement *p)
{
  DlTextEntry *dlTextEntry;
  DlElement *dlElement;

  dlTextEntry = DM2KALLOC(DlTextEntry);

  if (dlTextEntry == NULL)
    return 0;

  if (p != NULL) {
    objectAttributeCopy(&dlTextEntry->object, &p->structure.textEntry->object);
    controlAttributeCopy(&dlTextEntry->control, &p->structure.textEntry->control);
    sensitveAttributeCopy(&(dlTextEntry->sensitive), &p->structure.textEntry->sensitive);
    dlTextEntry->clrmod = p->structure.textEntry->clrmod;
    dlTextEntry->format = p->structure.textEntry->format;
  }
  else {
    objectAttributeInit(&(dlTextEntry->object));
    controlAttributeInit(&(dlTextEntry->control));
    sensitveAttributeInit(&(dlTextEntry->sensitive));
    dlTextEntry->clrmod = STATIC;
    dlTextEntry->format = DECIMAL;
  }

  dlElement = createDlElement(DL_TextEntry,
				    (XtPointer)      dlTextEntry,
				    &textEntryDlDispatchTable);
  if (dlElement == NULL)
    destroyDlTextEntry(dlTextEntry);

  return dlElement;
}

DlElement *parseTextEntry(DisplayInfo *displayInfo)
{
  char          token[MAX_TOKEN_LENGTH];
  TOKEN         tokenType;
  int           nestingLevel = 0;
  DlTextEntry * dlTextEntry;
  DlElement   * dlElement = createDlTextEntry(NULL);
  int           i = 0;

  if (dlElement == NULL) 
    return 0;

  dlTextEntry = dlElement->structure.textEntry; 
  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"object"))
	parseObject(displayInfo,&(dlTextEntry->object));
      else if (STREQL(token,"control"))
      {
	parseControl(displayInfo,&(dlTextEntry->control));
      }
      else if (STREQL(token,"sensitive"))
      {
	/* add on for sensitivity property */
	parseSensitive (displayInfo, &(dlTextEntry->sensitive));
      }
      else if (STREQL(token,"clrmod")) 
      {
	getToken(displayInfo,token);
	getToken(displayInfo,token);

	for (i = FIRST_COLOR_MODE; i < FIRST_COLOR_MODE+NUM_COLOR_MODES; i++){
	  if (STREQL(token,stringValueTable[i])) {
	    dlTextEntry->clrmod = (ColorMode)i;
	    break;
	  }
	}
      } 
      else if (STREQL(token,"format")) 
      {
	getToken(displayInfo,token);
	getToken(displayInfo,token);

	for (i=FIRST_TEXT_FORMAT;i<FIRST_TEXT_FORMAT+NUM_TEXT_FORMATS; i++) {
	  if (STREQL(token,stringValueTable[i])) {
	    dlTextEntry->format = (TextFormat)i;
	    break;
	  }
	}
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

void writeDlTextEntry(
  FILE *stream,
  DlElement *dlElement,
  int level)
{
  char indent[256]; level=MIN(level,256-2);
  DlTextEntry *dlTextEntry = dlElement->structure.textEntry;

  level = MIN(level, 256-2);
  memset(indent,'\t',level);
  indent[level] = '\0';

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
    fprintf(stream,"\n%s\"text entry\" {",indent);
    writeDlObject(stream,&(dlTextEntry->object),level+1);
    writeDlControl(stream,&(dlTextEntry->control),level+1);
    writeDlSensitive(stream,&(dlTextEntry->sensitive),level+1);

    if (dlTextEntry->clrmod != STATIC)
      fprintf(stream,"\n%s\tclrmod=\"%s\"",indent,
	      stringValueTable[dlTextEntry->clrmod]);

    if (dlTextEntry->format != DECIMAL)
      fprintf(stream,"\n%s\tformat=\"%s\"",indent,
	      stringValueTable[dlTextEntry->format]);
    fprintf(stream,"\n%s}",indent);

#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    fprintf(stream,"\n%s\"text entry\" {",indent);
    writeDlObject(stream,&(dlTextEntry->object),level+1);
    writeDlControl(stream,&(dlTextEntry->control),level+1);
    writeDlSensitive(stream,&(dlTextEntry->sensitive),level+1);
    fprintf(stream,"\n%s\tclrmod=\"%s\"",indent,
	    stringValueTable[dlTextEntry->clrmod]);
    fprintf(stream,"\n%s\tformat=\"%s\"",indent,
	    stringValueTable[dlTextEntry->format]);
    fprintf(stream,"\n%s}",indent);
  }
#endif
}

static void textEntryInheritValues(ResourceBundle *pRCB, DlElement *p) {
  DlTextEntry *dlTextEntry = p->structure.textEntry;

  updateElementControlAttribute   (pRCB, &dlTextEntry->control);
  updateElementSensitiveAttribute (pRCB, &dlTextEntry->sensitive);

  dm2kGetValues(pRCB,
		CLRMOD_RC,     &(dlTextEntry->clrmod),
		FORMAT_RC,     &(dlTextEntry->format),
		-1);
}

static void textEntryGetValues(ResourceBundle *pRCB, DlElement *p) {
  DlTextEntry *dlTextEntry = p->structure.textEntry;

  updateElementObjectAttribute    (pRCB, &dlTextEntry->object);
  updateElementControlAttribute   (pRCB, &dlTextEntry->control);
  updateElementSensitiveAttribute (pRCB, &dlTextEntry->sensitive);

  dm2kGetValues(pRCB,
		CLRMOD_RC,     &(dlTextEntry->clrmod),
		FORMAT_RC,     &(dlTextEntry->format),
		-1);
}


static void textEntryInfo (char *msg, Widget w,
			   DisplayInfo *displayInfo,
			   DlElement *element,
			   XtPointer objet)
{
  TextEntry *pte;
  DlTextEntry *dlTextEntry;

  if (globalDisplayListTraversalMode != DL_EXECUTE) {
    dlTextEntry = element->structure.textEntry;
    sensitiveControllerInfoSimple (msg, &dlTextEntry->control,
				   &dlTextEntry->sensitive);
    return;
  }

  pte = (TextEntry *) objet;
  dlTextEntry = pte->dlElement->structure.textEntry;

  sensitiveControllerInfo (msg,
			   &dlTextEntry->control, pte->record,
			   &dlTextEntry->sensitive, pte->sensitiveRecord);
}
