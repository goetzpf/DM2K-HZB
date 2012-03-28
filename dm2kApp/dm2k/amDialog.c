/******************************************************************************
 *  
 * Author: Vladimir T. Romanovski (romsky@x4u2.desy.de)
 *
 * Organization: KRYK/@DESY
 *
 * 10-Apr-97 
 *
 *****************************************************************************/

#include <ctype.h>
#include "dm2k.h"
#include <Xm/MwmUtil.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <cvtFast.h>
#ifdef __cplusplus
}
#endif

#define AMI_LABEL_SIZE   16
#define AMI_TYPE_SIZE    24
#define AMI_COMMAND_SIZE 24
#define AMI_ARGS_SIZE    24

#define AMI AssociatedMenuItem

/*
 * forward declarations
 */
static void CreateSubwidgetsFor_amMainWindow(Widget, AMI *);
static void CreateSubwidgetsFor_amItemForm  (Widget, AMI *);
static void CreateSubwidgetsFor_amActiveForm(Widget, AMI *);

/* static vars declarations
 */
static Widget amDialog  = NULL; /* Form widget just under the shell */
static Widget amLabel   = NULL; /* TextField widget for Label prompt */
static Widget amAction  = NULL; /* OptionMenu widget for ActioType prompt */
static Widget amCommand = NULL; /* TextField widget for Command prompt */
static Widget amArgs    = NULL; /* TextField widget for Arguments prompt */
static Widget amScrList = NULL; /* List widget shows created item */

/* it keeps number of Option menu choiced button 
 */
static int    amActionNumber = 0;

/* global array of actions type available in Associated Menu 
 */
char          *actionTypeLabels[] = { "System script", 
				      "New Display",
				      "New Display Replace" };

/* local static list of items being edited 
 */
static struct {
  AMI * head;
  AMI * tail;
  int   count;
} listAMI = { NULL, NULL, 0};




/*
 ###########################################################################
                                 HARDCODE
 ###########################################################################
*/

static void deleteItemStuff (AMI * ami)
{
  if (ami) {
    if (ami->label) free(ami->label);
    if (ami->command) free(ami->command);
    if (ami->args) free(ami->args);
    free((char*)ami);
  }
}

void deleteAMIList (AMI *head) 
{
  AMI * p;

  if (head == NULL)
	return;

  for (p = head; p != NULL; ) 
  {
    AMI * tmp = p->next;
    
    deleteItemStuff (p);
    p = tmp;
  }
}

AMI * copyAMIList (AMI *head) 
{
  AMI          * newList = NULL;  /* pointer on first item in new list */
  AMI          * lastList = NULL; /* pointer on last added item in new list */
  register AMI * n;


  if (head) { 
    for (n = head; n != NULL; n = n->next)
      {
	register AMI *ami;

	if ((ami = DM2KALLOC(AMI)))
	  {
	    ami->label      = STRDUP(n->label);
	    ami->actionType = n->actionType;
	    ami->command    = STRDUP (n->command);
	    ami->args       = STRDUP (n->args);
	    
	    ami->next = NULL;
	    ami->prev = lastList;

	    if (newList == NULL) 
	      newList = ami;

	    if (lastList) lastList->next = ami;
	    lastList = ami;
	  }
      }
  }
  return newList;
}


/*ARGSUSED*/
#ifdef __cplusplus
static void closeAmDialogCB(Widget, XtPointer, XtPointer)
#else
static void closeAmDialogCB(Widget w, XtPointer clientData, 
			    XtPointer callData)
#endif
{
  /* Set initial widgets' values
   */
  amDialog  = NULL;
  amLabel   = NULL;
  amAction  = NULL;
  amCommand = NULL;
  amArgs    = NULL;
  amScrList = NULL;

  /* delete items from the list
   */
  if (listAMI.count)
    deleteAMIList (listAMI.head);

  listAMI.head = listAMI.tail = (AMI*)(listAMI.count = 0);
}


void createAmDialog (Widget parent)
{
  AMI *items = globalResourceBundle.ami;

  Widget       amMainWindow;
  Widget       amItemForm;
  Widget       amSeparator;
  Widget       amActiveForm;

  Arg          args[20];
  int          n;
  
  
  /* If dialog has already been created, simply popup it 
   */
  if (amDialog == NULL) 
  { 
    /* This make take a second... give user some indication 
     */
    if (currentDisplayInfo != NULL) 
      XDefineCursor(display, XtWindow(currentDisplayInfo->drawingArea),
		    watchCursor);
    
    /* Create dialog and its main subparts
     */
    n = 0;
    XtSetArg(args[n], XmNautoUnmanage, False); n++;
    /* XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
     */
    amDialog = XmCreateFormDialog(parent, "amDialog", args, n);
    
    XtVaSetValues(XtParent(amDialog), 
		  XmNtitle, "Associated Menu Descriptor",
		  NULL);
    
    XmAddWMProtocolCallback(XtParent(amDialog), WM_DELETE_WINDOW,
			    closeAmDialogCB,
			    NULL);

    XtAddCallback (XtParent(amDialog), XmNdestroyCallback, 
		   closeAmDialogCB, NULL);
    
    amMainWindow = XmCreateMainWindow(amDialog, "amMainWindow", NULL, 0);
    amItemForm   = XmCreateForm      (amDialog, "amItemForm",   NULL, 0);
    amSeparator  = XmCreateSeparator (amDialog, "amSeparator",  NULL, 0);
    amActiveForm = XmCreateForm      (amDialog, "amActiveForm", NULL, 0);
    
    
    /* Set geometry links
     */
    n = 0;
    XtSetArg(args[n], XmNtopAttachment,    XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment,   XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment,  XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget,     amItemForm); n++;
    XtSetArg(args[n], XmNshadowThickness,  0); n++;
    XtSetValues(amMainWindow, args, n);
    
    n = 0;
    XtSetArg(args[n], XmNtopOffset,        12); n++;
    XtSetArg(args[n], XmNleftAttachment,   XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment,  XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset,      28); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget,     amSeparator); n++;
    XtSetArg(args[n], XmNshadowThickness,  0); n++;
    XtSetArg(args[n], XmNhorizontalSpacing,8); n++;
    XtSetValues(amItemForm, args, n);
    
    n = 0;
    XtSetArg(args[n], XmNtopOffset,        12); n++;
    XtSetArg(args[n], XmNleftAttachment,   XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment,  XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget,     amActiveForm); n++;
    XtSetArg(args[n], XmNshadowThickness,  2); n++;
    XtSetArg(args[n], XmNheight,           12); n++;
    XtSetValues(amSeparator, args, n);
    
    n = 0;
    XtSetArg(args[n], XmNtopOffset,        12); n++;
    XtSetArg(args[n], XmNleftAttachment,   XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment,  XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNfractionBase,     10); n++;
    XtSetArg(args[n], XmNshadowThickness,  0); n++;
    XtSetArg(args[n], XmNverticalSpacing,  12); n++;
    XtSetValues(amActiveForm, args, n);
    
    /* Make subwidgets in forms
     */
    CreateSubwidgetsFor_amMainWindow(amMainWindow , items);
    CreateSubwidgetsFor_amItemForm  (amItemForm, items);
    CreateSubwidgetsFor_amActiveForm(amActiveForm, items);
    
    /* Manage the composites 
     */
    XtManageChild(amActiveForm);
    XtManageChild(amSeparator);
    XtManageChild(amItemForm);
    XtManageChild(amMainWindow);
  }

  XtManageChild(amDialog);
    
  /* Popup the dialog
   */
  XtPopup(XtParent(amDialog), XtGrabExclusive);
  
  /* change drawingArea's cursor back to the appropriate cursor 
   */
  if (currentDisplayInfo != NULL)
    XDefineCursor(display, XtWindow(currentDisplayInfo->drawingArea),
		  (currentActionType == SELECT_ACTION ? 
		 rubberbandCursor: crosshairCursor));
}


/*ARGSUSED*/
#ifdef __cplusplus
static void amOptionCB (Widget w , XtPointer cd, XtPointer)
#else
static void amOptionCB (Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  int buttonNumber = (int) cd;
  
  amActionNumber = buttonNumber;

#if  0
  switch(buttonNumber) 
    {
    case AMI_SYSTEM_SCRIPT :
      printf("AMI_SYSTEM_SCRIPT..\n");
      break;
    case AMI_NEWDISPLAY :
      printf("AMI_NEWDISPLAY..\n");
      break;
    case AMI_NEWDISPLAY_REPLACE :
      printf("AMI_NEWDISPLAY_REPLACE..\n");
      break;
    default :
      INFORM_INTERNAL_ERROR();
      fprintf(stderr,"buttonNumber is %d\n", buttonNumber);
      break;
    }
#endif
}

/*ARGSUSED*/
#ifdef __cplusplus
static void amApplyCB (Widget, XtPointer, XtPointer)
#else
static void amApplyCB (Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  /* delete ald items list in globalResourceBundle and 
   * commit local items list to global resource
   */ 
  deleteAMIList (globalResourceBundle.ami);
  globalResourceBundle.ami = listAMI.head;

  /* unhang item list from local static list
   */
  listAMI.head = listAMI.tail = (AMI*)(listAMI.count = 0);

  /* delete current dialog
   */
  if (amDialog) XtDestroyWidget (XtParent(amDialog)); /*Olx changed*/

  /* copy changes into all selected element of the same type
   */
  if (currentDisplayInfo) {
    DlElement *dlElement = 
      FirstDlElement(currentDisplayInfo->selectedDlElementList);

    while (dlElement) {
      if (dlElement->structure.element->type == DL_Composite) {
	updateElementFromGlobalResourceBundle (dlElement->structure.element);
      }
      dlElement = dlElement->next;
    }
  }

  /* current display was edited
   */
  if (currentDisplayInfo->hasBeenEditedButNotSaved == False)
    dm2kMarkDisplayBeingEdited(currentDisplayInfo);
}


/*ARGSUSED*/
#ifdef __cplusplus
static void amCancelCB (Widget, XtPointer, XtPointer)
#else
static void amCancelCB (Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  if (amDialog) XtDestroyWidget (XtParent(amDialog)); /*Olx changed*/
}

/*ARGSUSED*/
#ifdef __cplusplus
static void amResetCB(Widget , XtPointer clientData, XtPointer)
#else
static void amResetCB(Widget    pushB, 
		      XtPointer clientData, 
		      XtPointer callData)
#endif
{
  Widget      menu;
  int         numButtons;
  WidgetList  buttons;  
  Widget      simpleOptionMenu = (Widget)clientData;

  XmTextFieldSetString (amLabel,   NULL);
  XmTextFieldSetString (amCommand, NULL);
  XmTextFieldSetString (amArgs,    NULL);

  /* reset Option Menu;this code episode is from Motif FAQ(Q:174)
   */
  XtVaGetValues(simpleOptionMenu, XmNsubMenuId, &menu, NULL);
  
  XtVaGetValues(menu, 
		XmNnumChildren, &numButtons,
                XmNchildren,    &buttons, 
		NULL);

  XtVaSetValues(simpleOptionMenu, XmNmenuHistory, buttons[0], NULL);
}


static void CreateSubwidgetsFor_amActiveForm(
     Widget amActiveForm,
     AMI    *items)
{
  Widget        buttons[3];
  int           i, left, right;
  static char * name[] = {"Apply", "Cancel", "Help"};

  for (i = 0; i < 3; i++) 
  {
    right = (left = 3*i + 1) + 2;

    buttons[i] = XtVaCreateManagedWidget (name[i], 
			   xmPushButtonWidgetClass, amActiveForm,
			   XmNtopAttachment,    XmATTACH_FORM,
			   XmNleftAttachment,   XmATTACH_POSITION,
			   XmNleftPosition,     left,
			   XmNrightAttachment,  XmATTACH_POSITION,
			   XmNrightPosition,    right,
			   XmNbottomAttachment, XmATTACH_FORM,
			   NULL);
  }

  XtAddCallback(buttons[0], XmNactivateCallback, amApplyCB, NULL);
  XtAddCallback(buttons[1], XmNactivateCallback, amCancelCB, NULL);

  XtSetSensitive (buttons[2], False);

  /*
  XtAddCallback(buttons[2], XmNactivateCallback, amHelpCB, (XtPointer)items);
  */
}




static void CreateSubwidgetsFor_amItemForm(
     Widget amItemForm, 
     AMI    *items) 
{
  Widget tmp;
  

  tmp = XtVaCreateManagedWidget ("Label",
				 xmLabelWidgetClass, amItemForm,
				 XmNtopAttachment,    XmATTACH_FORM,
				 XmNleftAttachment,   XmATTACH_FORM,
				 NULL);
				 
  amLabel = XtVaCreateManagedWidget ("amLabel", 
				 xmTextFieldWidgetClass, amItemForm,
				 XmNtopAttachment,    XmATTACH_WIDGET,
				 XmNtopWidget,        tmp,
				 XmNleftAttachment,   XmATTACH_FORM,
				 XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
				 XmNrightWidget,      tmp,
				 XmNrightOffset,      -64,
				 XmNbottomAttachment, XmATTACH_FORM,
				 NULL);

  tmp = XtVaCreateManagedWidget ("Action",
				 xmLabelWidgetClass, amItemForm,
				 XmNtopAttachment,    XmATTACH_FORM,
				 XmNleftAttachment,   XmATTACH_WIDGET,
				 XmNleftWidget,       amLabel,
				 NULL);

  /*
   * Option menu construct's playground
   */
  {
    XmString     buttons[20];
    XmButtonType buttonType[20];
    int          i, n;
    Arg          args[20];
				 
#define MENU_BUTTON_SET(n,label,type) \
 buttons[n] = XmStringCreateSimple(label); buttonType[n] = type

  MENU_BUTTON_SET(0, "System Script",       XmPUSHBUTTON);
  MENU_BUTTON_SET(1, "New Display",         XmPUSHBUTTON);
  MENU_BUTTON_SET(2, "New Display Replace", XmPUSHBUTTON);
#undef MENU_BUTTON_SET

    n = 0;
    XtSetArg(args[n], XmNbuttonCount,      3); n++;
    XtSetArg(args[n], XmNbuttons,          buttons); n++;
    XtSetArg(args[n], XmNbuttonType,       buttonType); n++;
    XtSetArg(args[n], XmNpostFromButton,   0); n++;
    XtSetArg(args[n], XmNsimpleCallback,   amOptionCB); n++;
    /* layout */
    XtSetArg(args[n], XmNtopAttachment,    XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget,        tmp); n++;
    XtSetArg(args[n], XmNleftAttachment,   XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget,       amLabel); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    amAction = XmCreateSimpleOptionMenu (amItemForm, "amAction", args,n);

    /* delete option menu label at left from menu */
    if (amAction) XtDestroyWidget(XmOptionLabelGadget (amAction)); /*Olx changed*/

    XtManageChild (amAction);

    for (i = 0; i < 3; i++) 
      XmStringFree(buttons[i]);
  }

  tmp = XtVaCreateManagedWidget ("Command",
				 xmLabelWidgetClass, amItemForm,
				 XmNtopAttachment,    XmATTACH_FORM,
				 XmNleftAttachment,   XmATTACH_WIDGET,
				 XmNleftWidget,       amAction,
				 NULL);
				 
  amCommand = XtVaCreateManagedWidget ("amCommand",
				 xmTextFieldWidgetClass, amItemForm,
				 XmNtopAttachment,    XmATTACH_WIDGET,
				 XmNtopWidget,        tmp,
				 XmNleftAttachment,   XmATTACH_WIDGET,
				 XmNleftWidget,       amAction,
				 XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
				 XmNrightWidget,      tmp,
				 XmNrightOffset,      -128,
				 XmNbottomAttachment, XmATTACH_FORM,
				 NULL);

  tmp = XtVaCreateManagedWidget ("Arguments",
				 xmLabelWidgetClass, amItemForm,
				 XmNtopAttachment,    XmATTACH_FORM,
				 XmNleftAttachment,   XmATTACH_WIDGET,
				 XmNleftWidget,       amCommand,
				 NULL);
				 
  amArgs = XtVaCreateManagedWidget ("amArgs",
			         xmTextFieldWidgetClass, amItemForm,
			         XmNtopAttachment,    XmATTACH_WIDGET,
			         XmNtopWidget,        tmp,
			         XmNleftAttachment,   XmATTACH_WIDGET,
			         XmNleftWidget,       amCommand,
			         XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
			         XmNrightWidget,      tmp,
			         XmNrightOffset,      -128,
			         XmNbottomAttachment, XmATTACH_FORM,
				 NULL);

  tmp = XtVaCreateManagedWidget ("Reset",
				 xmPushButtonWidgetClass, amItemForm,
				 XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
				 XmNtopWidget,     amArgs,
				 XmNleftAttachment,   XmATTACH_WIDGET,
				 XmNleftWidget,    amArgs,
				 XmNbottomAttachment, XmATTACH_FORM,
				 NULL);
  
  XtAddCallback (tmp, XmNactivateCallback, amResetCB, (XtPointer)amAction);
}


#define AM_ITEM_BTN_Add     0
#define AM_ITEM_BTN_Replace 1
#define AM_ITEM_BTN_Up      2
#define AM_ITEM_BTN_Down    3
#define AM_ITEM_BTN_Delete  4

/*
 * format a string: if it's longer then `bound', put '..' in the end.
 */
static void formatString(char *row, char *stable, int bound, int chars)
{
  int len = STRLEN (row);

  /* T. Straumann: make sure row!=0 */
  if (!row) row="";

  memset (stable, ' ', chars);
  strncpy (stable, row, MIN(len, bound));

  if (len > bound) 
    stable[bound-1] = stable[bound-2] = '.';

  stable[chars] = '\0';
}

static XmString makeXmStringForListWidget (AMI *ami)
{
  char     labelN[AMI_LABEL_SIZE+1];
  char     actN[AMI_TYPE_SIZE+1];
  char     commandN[AMI_COMMAND_SIZE+1];
  char     argsN[AMI_ARGS_SIZE+1];
  char     *act;
  XmString labelXmString;
  XmString actTypeXmString;
  XmString commandXmString;
  XmString argsXmString;
  XmString itemXmString;
  XmString tmp1, tmp2;


  if (ami == NULL) return NULL;

  /* get ascii description of action type
   */ 
  act = actionTypeLabels[ami->actionType];

  /* formating subpart of item
   */
  formatString (ami->label,   labelN,   AMI_LABEL_SIZE - 4,   AMI_LABEL_SIZE);
  formatString (act,          actN,     AMI_TYPE_SIZE - 4,    AMI_TYPE_SIZE);
  formatString (ami->command, commandN, AMI_COMMAND_SIZE - 4, AMI_COMMAND_SIZE);
  formatString (ami->args,    argsN,    AMI_ARGS_SIZE - 4,    AMI_ARGS_SIZE);

  /* let's put XmString of subparts together
   */
  labelXmString   = XmStringCreate (labelN,   "TAG1");
  actTypeXmString = XmStringCreate (actN,     "TAG2");
  commandXmString = XmStringCreate (commandN, "TAG1");
  argsXmString    = XmStringCreate (argsN,  "TAG1");
  
  tmp1 = XmStringConcat (labelXmString, actTypeXmString);
  tmp2 = XmStringConcat (commandXmString, argsXmString);
  
  itemXmString = XmStringConcat (tmp1, tmp2);
  
  /* free allocated memory
   */
  XmStringFree (labelXmString);
  XmStringFree (actTypeXmString);
  XmStringFree (commandXmString);
  XmStringFree (argsXmString);
  XmStringFree (tmp1);
  XmStringFree (tmp2);

  return itemXmString;
}


/*ARGSUSED*/
static void amItemCB(Widget w, XtPointer cd, XtPointer cbs)
{
  int      buttonNumber = (int) cd;
  int      i;
  AMI      *ami;

  /* Please remember items in List window starts from 1 rather then 0 !!!
   */
  switch(buttonNumber) 
    {
    case AM_ITEM_BTN_Add :
      if ((ami = (AMI*) malloc (sizeof(AMI))) != NULL)
      {
	XmString itemXmString;

	/* make XmStrings of new item
	 */
	ami->label    = XmTextGetString(amLabel);
	ami->actionType = amActionNumber;
	ami->command  = XmTextGetString(amCommand);
	ami->args     = XmTextGetString(amArgs);

	itemXmString = makeXmStringForListWidget (ami);

	if (itemXmString == NULL) {
	  /* T. Straumann: free resources */
	  XtFree(ami->label);
	  XtFree(ami->command);
	  XtFree(ami->args);
	  free ((char*)ami);
	  break;
	}

	/* add item to local static list
	 */
	ami->next = NULL;
	ami->prev = listAMI.tail;

	if (listAMI.head == NULL) listAMI.head = ami;
	if (listAMI.tail != NULL) listAMI.tail->next = ami;

	listAMI.tail = ami;
	listAMI.count++;

	/* put item on List
	 */
	XmListAddItem (amScrList, itemXmString, 0);

	/* itemXmString was allocated in makeXmStringForListWidget()
	 */
	XmStringFree (itemXmString);

      }
      break;
    
    case AM_ITEM_BTN_Replace : /* replace current item in List widget*/
      {
	int      * posList;
	int        posCount;
	AMI      * n;
	int        i;
	XmString   newItem[1];

	if (XmListGetSelectedPos (amScrList, &posList, &posCount)) 
	{
	  for (i = 1, n = listAMI.head ; i < posList[0]; i++, n = n->next)
	    /*EMPTY*/;

	  /* free old contents and make new 
	   */
#define CLEAN_MAKE(field,widget)                \
	  if (n->field) free ((char*)n->field); \
	  n->field = XmTextGetString (widget)

	  CLEAN_MAKE(label,   amLabel);
	  CLEAN_MAKE(command, amCommand);
	  CLEAN_MAKE(args,    amArgs);
#undef CLEAN_MAKE

	  n->actionType = amActionNumber;

	  newItem[0] = makeXmStringForListWidget (n);

	  XmListReplaceItemsPos (amScrList, newItem, 1, posList[0]);
	  XmStringFree (newItem[0]);
	}
      }
      break;

    case AM_ITEM_BTN_Up : /* put selected items up */
      {
	int  * posList;
	int    posCount;
	AMI  * n;
	int    i,j;
	int    changed = 0;

	if (XmListGetSelectedPos (amScrList, &posList, &posCount)) 
	{
	  XmStringTable table, newStringTable;
	  XmString      tmpXmString;
	  int           count;

	  XtVaGetValues (amScrList,
			 XmNitems,     &table, 
			 XmNitemCount, &count, 
			 NULL);

	  for (i = 0; i < posCount; i++) 
	  {
	    AMI tmpAMI;

	    if (posList[i] == 1)
	      continue;

	    changed++;

	    for (j = 1,n = listAMI.head ; j < posList[i]; j++, n = n->next)
	      /*EMPTY*/;

	    /* copy item's data
	     */
#define MYSWAP(a,b,field)        \
     tmpAMI.field = (a)->field;  \
     (a)->field   = (b)->field;  \
     (b)->field   = tmpAMI.field

	    MYSWAP(n, n->prev, label);
	    MYSWAP(n, n->prev, actionType);
	    MYSWAP(n, n->prev, command);
	    MYSWAP(n, n->prev, args);
#undef MYSWAP    

	    tmpXmString         = table[posList[i]-1];
	    table[posList[i]-1] = table[posList[i]-2];
	    table[posList[i]-2] = tmpXmString;
	  }

	  if (changed) 
	  {
	    /* let's make a new XmStringTable as modified copy of existed one
	     */
	    newStringTable = (XmStringTable) 
	      XtMalloc (count * sizeof(XmStringTable));
	    
	    for ( i = 0; i < count; i++) 
	      newStringTable[i] = XmStringCopy (table[i]);
	    
	    XmListDeleteAllItems (amScrList);
	    XtVaSetValues (amScrList, 
			   XmNitems,     newStringTable, 
			   XmNitemCount, count, 
			   NULL);

	    /* Select positions
	     */
	    if (posCount == count) {
	      for (i = 0; i < count; i++)
		XmListSelectPos (amScrList, i+1, False);
	    } else {
	      for (i = 0; i < posCount; i++) 
		if (posList[i] != 1)
		  XmListSelectPos (amScrList, posList[i]-1, False);
	    }

	    /* free memory
	     */
	    for (i = 0; i < count; i++) 
	      XmStringFree (newStringTable[i]);
	    
	    XtFree ((char*)newStringTable);
	    XtFree ((char*)posList);
	  }
	}
      }
      break;
      
    case AM_ITEM_BTN_Down : /* put selected items down */
      {
	int  * posList;
	int    posCount;
	AMI  * n;
	int    i,j;
	int    changed = 0;

	if (XmListGetSelectedPos (amScrList, &posList, &posCount)) 
	{
	  XmStringTable table, newStringTable;
	  XmString      tmpXmString;
	  int           count;

	  XtVaGetValues (amScrList, 
			 XmNitems,     &table, 
			 XmNitemCount, &count, 
			 NULL);

	  for (i = posCount-1; i >= 0; i--) 
	  {
	    AMI tmpAMI;

	    if (posList[i] == count)
	      continue;

	    changed++;

	    for (j = 1, n = listAMI.head; j < posList[i]; j++, n = n->next)
	      /*EMPTY*/;

	    /* copy item's data
	     */
#define MYSWAP(a,b,field)        \
     tmpAMI.field = (a)->field;  \
     (a)->field   = (b)->field;  \
     (b)->field   = tmpAMI.field
	    MYSWAP(n, n->next, label);
	    MYSWAP(n, n->next, actionType);
	    MYSWAP(n, n->next, command);
	    MYSWAP(n, n->next, args);
#undef MYSWAP    

	    tmpXmString         = table[posList[i]-1];
	    table[posList[i]-1] = table[posList[i]];
	    table[posList[i]]   = tmpXmString;
	  }

	  if (changed) 
	  {
	    /* let's make a new XmStringTable as modified copy of existed one
	     */
	    newStringTable = (XmStringTable) 
	      XtMalloc (count * sizeof(XmStringTable));
	    
	    for ( i = 0; i < count; i++) 
	      newStringTable[i] = XmStringCopy (table[i]);
	    
	    XmListDeleteAllItems (amScrList);
	    XtVaSetValues (amScrList, XmNitems, newStringTable, 
			   XmNitemCount, count, NULL);
	    
	    /* Select positions
	     */
	    if (posCount == count) {
	      for (i = 0; i < count; i++)
		XmListSelectPos (amScrList, i+1, False);
	    } else {
	      for (i = 0; i < posCount; i++) 
		if (posList[i] != count)
		XmListSelectPos (amScrList, posList[i]+1, False);
	    }

	    /* free memory
	     */
	    for (i = 0; i < count; i++) 
	      XmStringFree (newStringTable[i]);
	    
	    XtFree ((char*)newStringTable);
	    XtFree ((char*)posList);
	  }
	}
      }
      break;

    case AM_ITEM_BTN_Delete :
      {
	int  * posList;
	int    posCount;
	AMI  * n;
	int    j;

	if (XmListGetSelectedPos (amScrList, &posList, &posCount)) 
	{
	  XmStringTable table, newStringTable;
	  int           count;
	  int           *maskNoneDeleting;

	  XtVaGetValues (amScrList, 
			 XmNitems,     &table, 
			 XmNitemCount, &count, 
			 NULL);

	  /* create and initialize mask of item which will not be deleted
	   */
	  maskNoneDeleting = (int*) calloc (count, sizeof (int));
	  for (i = 0; i < count; i++)
	    maskNoneDeleting[i] = 1;

	  /* delete items from bottom to up
	   */
	  for (i = posCount-1; i >= 0; i--) 
          {
	    /* mark item as to be deleted
	     */
	    maskNoneDeleting[posList[i]-1] = 0;

	    /* items in List widget are counted from 1 */
	    for (j = 1, n = listAMI.head ; j < posList[i]; j++, n = n->next)
	      /*EMPTY*/;

	    /* deleting of item's stuff meens just freing allocated memory
	     */
	    free(n->label);
	    free(n->command);
	    free(n->args);

	    /* get item out from list
	     */
	    if (n->next) n->next->prev = n->prev;
	    if (n->prev) n->prev->next = n->next;

	    if (listAMI.head == n) listAMI.head = n->next;
	    if (listAMI.tail == n) listAMI.tail = n->prev;
	    listAMI.count--;
	  }

	  
	  /* delete all items from List widget and add new set of items
	   */
	  if (posCount < count) 
	  {
	    newStringTable = (XmStringTable) 
	      XtMalloc ((count-posCount) * sizeof(XmStringTable));
	    
	    /* make copies only of not deleting items
	     */
	    for (i = 0, j = 0; i < count; i++)  {
	      if (maskNoneDeleting[i]) 
		newStringTable[j++] = XmStringCopy (table[i]);
	    }

	    XmListDeleteAllItems (amScrList);
	    XtVaSetValues (amScrList, 
			   XmNitems,     newStringTable, 
			   XmNitemCount, (count-posCount), 
			   NULL);
	  }
	  else 
	  {
	    XmListDeleteAllItems (amScrList);
	  }

	  free ((char*)maskNoneDeleting);
	}
      }
      break;
      
    default :
      INFORM_INTERNAL_ERROR();
      fprintf(stderr,"unrecognize menu button %d..\n", buttonNumber);
      break;
    }

#ifdef DEBUG
  printf("\n\n");
  for (ami = listAMI.head; ami != NULL; ami = ami->next) {
    printf ("%20s %d %20s %20s\n",
	    ami->label,	ami->actionType, ami->command, ami->args);
  }
#endif
}

/*ARGSUSED*/
#ifdef __cplusplus
static void amScrListMultSelectCB(Widget , XtPointer, XtPointer callData)
#else
static void amScrListMultSelectCB(Widget    list_w, 
				  XtPointer clientData, 
				  XtPointer callData)
#endif
{
  XmListCallbackStruct *cbs = (XmListCallbackStruct*) callData;
  int                  itemNumber;
  int                  i;
  AMI                  *ami;
  Widget      menu;
  int         numButtons;
  WidgetList  buttons;  
  Widget      simpleOptionMenu = amAction;

  /* always choice first selected item
   */
  if (cbs->reason == XmCR_MULTIPLE_SELECT && cbs->selected_item_count) 
    {
      itemNumber = cbs->selected_item_positions[0];
    }
  else
    {
      itemNumber = cbs->item_position;
    }

  /* let's go for requested item (items are counted from `1')
   */
  for (i = 1, ami = listAMI.head; i < itemNumber && ami; i++, ami = ami->next)
    /*EMPTY*/;

  if (ami == NULL) {
    INFORM_INTERNAL_ERROR();
    return;
  }
  
  XmTextFieldSetString (amLabel,   ami->label);
  XmTextFieldSetString (amCommand, ami->command);
  XmTextFieldSetString (amArgs,    ami->args);

  /* reset Option Menu;this code episode is from Motif FAQ(Q:174)
   */
  XtVaGetValues(simpleOptionMenu, XmNsubMenuId, &menu, NULL);
  
  XtVaGetValues(menu, 
		XmNnumChildren, &numButtons,
                XmNchildren,    &buttons, 
		NULL);

  XtVaSetValues(simpleOptionMenu, 
		XmNmenuHistory, buttons[ami->actionType], 
		NULL);
}

static void CreateSubwidgetsFor_amMainWindow(
					     Widget   amMainWindow, 
					     AMI    * items) 
{
  Widget       tmpMB, tmpPDM;
  XmString     buttons[20];
  KeySym       keySyms[20];
  XmButtonType buttonType[20];
  Arg          args[20];
  int          i, n;
#if 0
  XmFontList        fontList;
  XmFontListEntry   entry;
#endif

  /* Create menu bar 
   */
  buttons[0] = XmStringCreateSimple("Item");
  tmpMB = XmVaCreateSimpleMenuBar(amMainWindow, "amMainWindowMB",
				  XmVaCASCADEBUTTON, buttons[0], 'I',
				  NULL);
  XmStringFree(buttons[0]);
  
  
  /* Color resourceMB properly (force so VUE doesn't interfere) 
   */
  colorMenuBar(tmpMB, defaultForeground, defaultBackground);


  /* create the file pulldown menu pane 
   */
#define MENU_BUTTON_SET(n,label,sym,type)  \
  buttons[n] = XmStringCreateSimple(label); \
    keySyms[n] = sym; buttonType[n] = type

      MENU_BUTTON_SET(0, "Add new", 'A', XmPUSHBUTTON);
  MENU_BUTTON_SET(1, "Replace", 'R', XmPUSHBUTTON);
  MENU_BUTTON_SET(2, " ",       ' ', XmSEPARATOR);
  MENU_BUTTON_SET(3, "Up",      'U', XmPUSHBUTTON);
  MENU_BUTTON_SET(4, "Down",    'D', XmPUSHBUTTON);
  MENU_BUTTON_SET(5, " ",       ' ', XmSEPARATOR);
  MENU_BUTTON_SET(6, "Delete",  'e', XmPUSHBUTTON);
#undef MENU_BUTTON_SET

  n = 0;
  XtSetArg(args[n], XmNbuttonCount,    7); n++;
  XtSetArg(args[n], XmNbuttons,        buttons); n++;
  XtSetArg(args[n], XmNbuttonType,     buttonType); n++;
  XtSetArg(args[n], XmNbuttonMnemonics,keySyms); n++;
  XtSetArg(args[n], XmNpostFromButton, 0); n++;
  XtSetArg(args[n],XmNsimpleCallback,  amItemCB); n++;
  tmpPDM = XmCreateSimplePulldownMenu(tmpMB, "tmpPDM", args, n);

  for (i = 0; i < 7; i++) 
    XmStringFree(buttons[i]);


  /* Add scrolled window and contents 
   */
#if 0				/* VTR, I was testing here fontlist usage.. */
  entry = XmFontListEntryLoad (XtDisplay(amMainWindow), 
			       "-*-courier-medium-r-normal-*-14-*-*-*-*-*-*-*",
			       XmFONT_IS_FONT,
			       "TAG1");
  fontList = XmFontListAppendEntry (NULL, entry);
  XmFontListEntryFree (&entry);
#endif

  n = 0;
  XtSetArg(args[n], XmNscrollingPolicy,        XmAUTOMATIC); n++;
  XtSetArg(args[n], XmNscrollBarDisplayPolicy, XmSTATIC); n++;
  XtSetArg(args[n], XmNvisibleItemCount,       5); n++;
  /*XtSetArg(args[n], XmNfontList,               fontList); n++;*/
  XtSetArg(args[n], XmNselectionPolicy,        XmMULTIPLE_SELECT); n++;
  amScrList = XmCreateScrolledList(amMainWindow, "amScrList", args, n);

  XtAddCallback (amScrList, XmNmultipleSelectionCallback, 
		 amScrListMultSelectCB, (XtPointer)NULL);
#if 0
  XmFontListFree (fontList);
#endif

  /* Set areas
   */
  XmMainWindowSetAreas(amMainWindow, tmpMB, 
		       NULL, NULL, NULL, XtParent(amScrList));


  /* if we have some items, let's process them
   */
  if (items) 
    {
      AMI *n;
      
      /* copy `items' into the local static list and
       * show preexisted items in List widget
       */
      listAMI.head = copyAMIList(items);

      for (n = listAMI.head; n; n = n->next) 
	{
	  XmString itemXmString = makeXmStringForListWidget (n);
	
	  if (itemXmString) {
	    XmListAddItemUnselected (amScrList, itemXmString, 0);
	    XmStringFree (itemXmString); 
	    /*
	     * may be that item is the last one
	     */
	    listAMI.tail = n;
	    listAMI.count++;
	  }
	}
    }


  /* Manage the composites 
   */
  XtManageChild(tmpMB);
  XtManageChild(amScrList);
}


void destroyAMPalette(void) 
{
  if (amDialog) XtDestroyWidget (XtParent(amDialog));
}

