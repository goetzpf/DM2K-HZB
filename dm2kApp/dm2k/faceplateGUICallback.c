/*
** Generated by WorkShop Visual
*/

#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>

#include <Xm/Xm.h>
#include <Xm/CascadeB.h>
#include <Xm/DialogS.h>
#include <Xm/PushB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleBG.h>

#include "dm2k.h"
#include "faceplateGUI.h"


extern void XDmanage_link ( Widget, XtPointer, XtPointer);
extern void XDunmanage_link ( Widget, XtPointer, XtPointer);
extern void XDpopup_link ( Widget, XtPointer, XtPointer);
extern void XDpopdown_link ( Widget, XtPointer, XtPointer);
extern void XDmap_link ( Widget, XtPointer, XtPointer);
extern void XDunmap_link ( Widget, XtPointer, XtPointer);
extern void XDenable_link ( Widget, XtPointer, XtPointer);
extern void XDdisable_link ( Widget, XtPointer, XtPointer);
extern FaceplateGUI * getFaceplateGUI(Widget);
extern void setFaceplateSensitivity(FaceplateGUI *);

void cbFaceplateMacro(Widget , XtPointer , XtPointer);
static void cbFreeUserData(Widget, XtPointer, XtPointer);
void cbWholeGroup(Widget ,XtPointer ,XtPointer );

/* End of WorkShop Visual generated prelude */

static void showMacroInWidgets(
   FaceplateGUI * fGUI, 
   const char   * adl, 
   const char   * macro)
{
  Widget      form50;
  Widget      text60;
  Widget      text61;
  Widget      children[5];
  char     ** macroList;
  int         macros;
  int         i;
  NameValueTable * nameTable;
  int         numPairs;
  WidgetList  wList;
  int         numWidgets;
  Arg         al[64];                    /* Arg List */
  int         ac = 0;                    /* Arg Count */
  XmString    xmstring;

  if (fGUI == NULL || adl == NULL)
    return;

  XtVaGetValues(fGUI->wFaceplateMacroRowColomn,
		XtNchildren, &wList,
		XtNnumChildren, &numWidgets,
		NULL);
    
  for (i = 0; i < numWidgets; i++)
    XtDestroyWidget(wList[i]);

  nameTable = generateNameValueTable(macro, &numPairs);

  macros = getAllMacros(adl, &macroList);
  for (i = 0; i < macros; i++)
  {
    int j;
    char * value;
    
    value = NULL;
    for (j = 0; j < numPairs; j++)
    {
      if (STREQL(macroList[i], nameTable[j].name))
	value = nameTable[j].value;
    }

    ac = 0;
    form50 = XmCreateForm ( fGUI->wFaceplateMacroRowColomn, "form50", al,ac);
      
    xmstring = XmStringCreateLtoR (macroList[i],
				   (XmStringCharSet)XmFONTLIST_DEFAULT_TAG );
    XtSetArg(al[ac], XmNlabelString, xmstring); ac++;
    XtSetArg(al[ac], XmNalignment, XmALIGNMENT_END); ac++;
    text60 = XmCreateLabel ( form50, "text60", al, ac );
    XmStringFree(xmstring);
      
    ac = 0;
    XtSetArg(al[ac], XmNcolumns, 29); ac++;
    XtSetArg(al[ac], XmNuserData, STRDUP(macroList[i])); ac++;
    text61 = XmCreateTextField ( form50, "text61", al, ac );
    XmTextFieldSetString (text61, value);

    XtAddCallback(text61, XmNdestroyCallback, cbFreeUserData, NULL);

    ac = 0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 35); ac++;
    XtSetValues ( text60,al, ac );
      
    ac = 0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNleftWidget, text60); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetValues ( text61,al, ac );

    ac = 0;
    children[ac++] = text60;
    children[ac++] = text61;
    XtManageChildren(children, ac);

    XtManageChild(form50);

    DM2KFREE(macroList[i]);
  }

  DM2KFREE(macroList);
    
  freeNameValueTable(nameTable, numPairs);
}

static void highligthButton(Widget w)
{
  Pixel back;
  Pixel fore;

  XtVaGetValues(w, 
		XtNbackground, &back,
		XtNforeground, &fore,
		NULL);

  XtVaSetValues(w, 
		XtNbackground, fore,
		XtNforeground, back,
		NULL);
}

static void highligthCell(FaceplateGUI * fGUI,Widget w)
{
  if (fGUI->highligthedCell != NULL) 
    highligthButton(fGUI->highligthedCell);

  fGUI->highligthedCell = w;
  
  if (w != NULL)
    highligthButton(w);
}


static void highligthCellOfFaceplate(
   FaceplateGUI * fGUI,
   const char   * position)
{
  char buffer[8];
  char * tmp;
  int row;
  int column;
  int indx;
  Widget w = NULL;


  if (fGUI == NULL || position == NULL || *position == '\0') {
    highligthCell(fGUI, NULL);
    return;
  }

  strcpy(buffer, position);

  tmp = strchr(buffer, ',');
  if (tmp)
    *tmp = ' ';

  sscanf(buffer, "%d%d\n", &column, &row);

  indx = column * 10 + row;

  switch (indx)
    {
    case 00: w = fGUI->wCell00; break;
    case 10: w = fGUI->wCell10; break;
    case 20: w = fGUI->wCell20; break;
    case 30: w = fGUI->wCell30; break;
    case 40: w = fGUI->wCell40; break;
    case 50: w = fGUI->wCell50; break;
    case 60: w = fGUI->wCell60; break;
    case 70: w = fGUI->wCell70; break;
    case 01: w = fGUI->wCell01; break;
    case 11: w = fGUI->wCell11; break;
    case 21: w = fGUI->wCell21; break;
    case 31: w = fGUI->wCell31; break;
    case 41: w = fGUI->wCell41; break;
    case 51: w = fGUI->wCell51; break;
    case 61: w = fGUI->wCell61; break;
    case 71: w = fGUI->wCell71; break;
    default :
      fprintf(stderr, "internal error (%s:%d)\n", __FILE__, __LINE__);
      return;
      break;
    }

  highligthCell(fGUI, w);
}

static void showResourcesOfFaceplate(FaceplateGUI * fGUI, Faceplate * entry)
{
  char        buffer[256];

  if (fGUI == NULL || entry == NULL)
    return;

  highligthCellOfFaceplate(fGUI, entry->position);

  if (entry->position != NULL) {
    strcpy(fGUI->position, entry->position);

    XmTextFieldSetString (fGUI->wFaceplateX, NULL);
    XmTextFieldSetString (fGUI->wFaceplateY, NULL);
    XmTextFieldSetString (fGUI->wFaceplateWidth, NULL);
    XmTextFieldSetString (fGUI->wFaceplateHeigth, NULL);
    XtSetSensitive(fGUI->wFaceplateX, False);
    XtSetSensitive(fGUI->wFaceplateY, False);
    XtSetSensitive(fGUI->wFaceplateHeigth, False);
    XtSetSensitive(fGUI->wFaceplateWidth, False);
  }
  else {
    fGUI->position[0] = '\0';

    sprintf(buffer, "%d", entry->x);
    XmTextFieldSetString (fGUI->wFaceplateX, buffer);
    sprintf(buffer, "%d", entry->y);
    XmTextFieldSetString (fGUI->wFaceplateY,  buffer);
    sprintf(buffer, "%d", entry->w);
    XmTextFieldSetString (fGUI->wFaceplateWidth, buffer);
    sprintf(buffer, "%d", entry->h);
    XmTextFieldSetString (fGUI->wFaceplateHeigth, buffer);
    XtSetSensitive(fGUI->wFaceplateX, True);
    XtSetSensitive(fGUI->wFaceplateY, True);
    XtSetSensitive(fGUI->wFaceplateHeigth, True);
    XtSetSensitive(fGUI->wFaceplateWidth, True);
  }

  /* adl file
   */
  XmTextFieldSetString (fGUI->wSelectedAdlFile, entry->adl);

  /* macro substitution
   */
  showMacroInWidgets(fGUI, entry->adl, entry->macro);
}


static Widget createFaceplateToggleButton(FaceplateGUI * fGUI, int num)
{
  XrmValue from_value, to_value; /* For resource conversion */
  XmString xmstring;             /* temporary storage for XmStrings */
  Widget toggle;
  char   buffer[256];
  Arg al[64];                    /* Arg List */
  int ac = 0;                    /* Arg Count */

  sprintf(buffer, "faceplate%d", num);

  ac = 0;
  xmstring = XmStringCreateLtoR ( buffer,
				  (XmStringCharSet)XmFONTLIST_DEFAULT_TAG );
  XtSetArg(al[ac], XmNlabelString, xmstring); ac++;
  from_value.addr = "-*-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*";
  from_value.size = strlen( from_value.addr ) + 1;
  to_value.addr = NULL;
  XtConvertAndStore ( fGUI->wGroupRowColumn, XmRString, &from_value, XmRFontList, &to_value);
  if ( to_value.addr ) {
    XtSetArg(al[ac], XmNfontList, *(XmFontList *)to_value.addr); ac++;
  }
    
  toggle = XmCreateToggleButtonGadget ( fGUI->wGroupRowColumn, buffer,
					al,ac ); 

  XtAddCallback( toggle, XmNvalueChangedCallback, cbWholeGroup, 0 );
  XmStringFree (xmstring);
    
  XtManageChild(toggle);
  
  return toggle;
}

static void transferFromDataIntoWidgets(FaceplateGUI * fGUI)
{
  char       buffer[256];
  int        i;
  WidgetList wList;
  int        numWidgets;

  if (fGUI == NULL || fGUI->fpg == NULL)
    return;

  /* group attributes 
   */
  XmTextFieldSetString (fGUI->wTitle, fGUI->fpg->title);
  XmTextFieldSetString (fGUI->wNotes, fGUI->fpg->notes);
  XmTextSetString (fGUI->wComments, fGUI->fpg->comments);

  /* group coordinates 
   */
  sprintf(buffer, "%d", fGUI->fpg->x);
  XmTextFieldSetString (fGUI->wX, buffer);
  sprintf(buffer, "%d", fGUI->fpg->y);
  XmTextFieldSetString (fGUI->wY,  buffer);
  sprintf(buffer, "%d", fGUI->fpg->w);
  XmTextFieldSetString (fGUI->wWidth, buffer);
  sprintf(buffer, "%d", fGUI->fpg->h);
  XmTextFieldSetString (fGUI->wHeight, buffer);


  if (fGUI->fpg->fractionBase > 0) {
    XtSetSensitive(fGUI->wFractionBaseTXT, True);
    sprintf(buffer, "%d", fGUI->fpg->fractionBase);
    XmTextFieldSetString (fGUI->wFractionBaseTXT, buffer);
    XmToggleButtonSetState(fGUI->wInPixelTGL, False, False);
    XmToggleButtonSetState(fGUI->wFractionBaseTGL, True, False);
  }
  else {
    XmToggleButtonSetState(fGUI->wInPixelTGL, True, False);
    XmToggleButtonSetState(fGUI->wFractionBaseTGL, False, False);
    XtSetSensitive(fGUI->wFractionBaseTXT, False);
  }

  /* destroy old faceplate toggle buttons
   */
  XtVaGetValues(fGUI->wGroupRowColumn,
		XtNchildren, &wList,
		XtNnumChildren, &numWidgets,
		NULL);

  for (i = 0; i < numWidgets; i++)
  {
    if (XmIsToggleButtonGadget(wList[i]))
      XtDestroyWidget(wList[i]);
  }

  /* faceplates 
   */
  for (i = 0; i < fGUI->fpg->entriesNum; i++) {
    Widget toggle = createFaceplateToggleButton(fGUI, i);

    if (i == 0)
      XmToggleButtonSetState(toggle, True, False);
    else
      XmToggleButtonSetState(toggle, False, False);
  }

  if (fGUI->fpg->entriesNum > 0)
    showResourcesOfFaceplate(fGUI, fGUI->fpg->entries[0]);
}

/* T. Straumann: this macro is nonsense. XmTextFieldGetString()
 *				 _always_ returns a string (although maybe empty).
 *				 On the other hand it's our duty to call XtFree(s)
 *				 in order to avoid memory leaks.
#define ATOI(s) ((tmp = s) != NULL ? atoi(tmp) :0)
*/
#define ATOI(s) ((tmpval=strtol((s),&tmp,10)),XtFree(s),tmp==s ? 0 : tmpval)
#define UINT unsigned int

static void getFaceplateFromWidgets(FaceplateGUI * fGUI, Faceplate * entry)
{ 
  WidgetList wList;
  int        numWidgets;
  char     * tmp;
  char       macroString[10000];
  int        i,tmpval;

  if (fGUI == NULL || entry == NULL)
    return;

  /* position and size
   */
  if (fGUI->position[0] == '\0') {
    entry->position = NULL;
    entry->x = ATOI(XmTextFieldGetString(fGUI->wFaceplateX));
    entry->y = ATOI(XmTextFieldGetString(fGUI->wFaceplateY));
    entry->w = (UINT)ATOI(XmTextFieldGetString(fGUI->wFaceplateWidth));
    entry->h = (UINT)ATOI(XmTextFieldGetString(fGUI->wFaceplateHeigth));
  }
  else {
    renewString(&entry->position, fGUI->position);
    entry->x = 0;
    entry->y = 0;
    entry->w = 0;
    entry->h = 0;
  }

  /* adl file
   */

  /* T. Straumann: dont make no copy -- read the docs !!!
   *			   XmTextFieldGetString() already returns a copy.
   */

  if (entry->adl) XtFree(entry->adl);
  entry->adl = XmTextFieldGetString(fGUI->wSelectedAdlFile);

  /* macros
   */
  macroString[0] = '\0';
  XtVaGetValues(fGUI->wFaceplateMacroRowColomn,
		XtNchildren, &wList,
		XtNnumChildren, &numWidgets,
		NULL);
    
  for (i = 0; i < numWidgets; i++)
    {
      WidgetList wList1;
      int        num;
      int        j;
      char     * macroName;
      char     * macroValue;

      XtVaGetValues(wList[i],
		    XtNchildren, &wList1,
		    XtNnumChildren, &num,
		    NULL);

      macroName = NULL;
      macroValue = NULL;
      for (j = 0; j < num; j++) {
	if (XmIsTextField(wList1[j])) {
	  macroValue = XmTextFieldGetString(wList1[j]);
	  XtVaGetValues(wList1[j], XmNuserData, &macroName, NULL);
	}
      }

      if (macroName != NULL && macroValue != NULL && *macroValue != '\0') {
	if (macroString[0] != '\0') {
	  char copy[10000];

	  sprintf(copy, "%s,%s=%s", macroString, macroName, macroValue);
	  strcpy(macroString, copy);
	}
	else
	  sprintf(macroString, "%s=%s", macroName, macroValue);

/* T. Straumann: what if macroValue=="" ? 
	free(macroValue);
 */
      }
	  if (macroValue) XtFree(macroValue);
    }

  renewString(&entry->macro, macroString[0] != '\0' ? macroString : NULL);
}

static void transferFromWidgetsIntoData(FaceplateGUI * fGUI)
{
  int        tmpval;
  char     * tmp;

  if (fGUI == NULL || fGUI->fpg == NULL)
    return;

  /* group attributes 
   */
  /* T. Straumann: dont make no copy -- read the docs !!!
   *			   XmTextFieldGetString() already returns a copy.
   */

  if (fGUI->fpg->title) XtFree(fGUI->fpg->title);
  fGUI->fpg->title = XmTextFieldGetString(fGUI->wTitle);
  if (fGUI->fpg->notes) XtFree(fGUI->fpg->notes);
  fGUI->fpg->notes = XmTextFieldGetString(fGUI->wNotes);
  if (fGUI->fpg->comments) XtFree(fGUI->fpg->comments);
  fGUI->fpg->notes = XmTextFieldGetString(fGUI->wComments);

  /* group coordinates 
   */
  fGUI->fpg->x = ATOI(XmTextFieldGetString(fGUI->wX));
  fGUI->fpg->y = ATOI(XmTextFieldGetString(fGUI->wY));
  fGUI->fpg->w = (UINT)ATOI(XmTextFieldGetString(fGUI->wWidth));
  fGUI->fpg->h = (UINT)ATOI(XmTextFieldGetString(fGUI->wHeight));

  if (XmToggleButtonGetState(fGUI->wInPixelTGL)) 
    fGUI->fpg->fractionBase = 0;
  else 
    fGUI->fpg->fractionBase = 
      ATOI(XmTextFieldGetString(fGUI->wFractionBaseTXT));

#if 0
  /* faceplates 
   */
   if (fGUI->current >= 0) 
     getFaceplateFromWidgets(fGUI, fGUI->fpg->entries[fGUI->current]);
#endif
}

/*
** WorkShop Visual Stub cbFaceplateDlg
*/

static void cbFreeUserData(Widget w, XtPointer client_data, XtPointer xt_call_data)
{
  XtPointer data;

  XtVaGetValues(w, XmNuserData, &data, NULL);

  DM2KFREE(data);
}

void cbFaceplateDlg(Widget w, XtPointer client_data, XtPointer xt_call_data)
{
  XtPointer userData;
    
  XtVaGetValues(w, XmNuserData, &userData, NULL);
  
  if (userData != NULL)
    free((char*)userData);
  else
    INFORM_INTERNAL_ERROR();
}

/*
** WorkShop Visual Stub cbFile
*/

void cbFile(Widget w, XtPointer client_data, XtPointer xt_call_data)
{
  int cd = (int)client_data;
  FaceplateGUI * fGUI = getFaceplateGUI(w);

  XUngrabPointer(XtDisplay(w),CurrentTime);

  switch(cd) 
    {
    case FP_FILE_NEW: {
      Widget     toggle;
      WidgetList wList;
      int        numWidgets;
      int        i;

      /* if we have some faceplates, flush it out and create new FaceplateGr. 
       */
      if (fGUI->fpg != NULL) 
	destroyFaceplateGroup(fGUI->fpg);

      fGUI->fpg = createFaceplateGroup(NULL);

      /* add one faceplate in the new group 
       */
      appendNewFaceplate(fGUI->fpg, 
			 NULL,
			 0,
			 0,
			 0,
			 0,
			 NULL,
			 NULL);

      /* destroy all toggle widgets represent faceplate
       */
      XtVaGetValues(fGUI->wGroupRowColumn,
		    XtNchildren, &wList,
		    XtNnumChildren, &numWidgets,
		    NULL);
      
      for (i = 0; i < numWidgets; i++)
      {
	if (XmIsToggleButtonGadget(wList[i]))
	  XtDestroyWidget(wList[i]);
      }

      /* make a toggle for first faceplate
       */
      toggle = createFaceplateToggleButton(fGUI, fGUI->fpg->entriesNum-1);
      XmToggleButtonSetState(toggle, True, False);

      fGUI->current = 0;
      setFaceplateSensitivity(fGUI);
    }
    break;

    case FP_FILE_OPEN:
      fGUI->saveIt = False;
      create_shell2(fGUI->wFaceplateDlgShell);
      break;

    case FP_FILE_SAVE:
      if (fGUI->fpg) {
	transferFromWidgetsIntoData(fGUI);
	writeFaceplateGroupToFile(fGUI->wFaceplateDlg, fGUI->fpg, fGUI->fpg->name, True);
      }
      break;
    case FP_FILE_SAVEAS:
      if (fGUI->fpg) {
	fGUI->saveIt = True;
	create_shell2(fGUI->wFaceplateDlgShell);
      }
      break;
    case FP_FILE_CLOSE:
      XtDestroyWidget(fGUI->wFaceplateDlgShell);
      break;
    default :
      INFORM_INTERNAL_ERROR();
      break;
    }
}

/*
** WorkShop Visual Stub cbGroupAttributes
*/

void cbGroupAttributes(Widget w, XtPointer client_data, XtPointer xt_call_data)
{
}

/*
** WorkShop Visual Stub cbFaceplateCoordinates
*/

void cbFaceplateCoordinates(Widget w, XtPointer client_data, XtPointer xt_call_data)
{
  FaceplateGUI * fGUI = getFaceplateGUI(w);
  int button = (int)client_data;

  switch (button)
    {
    case FP_INPIXEL :
      XmToggleButtonSetState(fGUI->wInPixelTGL, True, False);
      XmToggleButtonSetState(fGUI->wFractionBaseTGL, False, False);
      XtSetSensitive(fGUI->wFractionBaseTXT, False);
      break;
      
    case FP_FRACTIONBASE :
      XmToggleButtonSetState(fGUI->wInPixelTGL, False, False);
      XmToggleButtonSetState(fGUI->wFractionBaseTGL, True, False);
      XtSetSensitive(fGUI->wFractionBaseTXT, True);
      break;
      
    default :
      break;
    }
}

/*
** WorkShop Visual Stub cbWholeGroup
*/

void cbWholeGroup(Widget w, XtPointer client_data, XtPointer xt_call_data)
{
  FaceplateGUI * fGUI = getFaceplateGUI(w);
  WidgetList wList;
  int numWidgets;
  int i;

  if (!XmToggleButtonGetState(w))
    return;

  XtVaGetValues(XtParent(w),
		XtNchildren, &wList,
		XtNnumChildren, &numWidgets,
		NULL);

  for (i = 0; i < numWidgets; i++)
  {
    if (w == wList[i]) {
      fGUI->position[0] = '\0';
      fGUI->current = i;
      showResourcesOfFaceplate(fGUI, fGUI->fpg->entries[i]);
      break;
    }
  }  
}

/*
** WorkShop Visual Stub cbFaceplateCell
*/

void cbFaceplateCell(Widget w, XtPointer client_data, XtPointer xt_call_data)
{
  int      button = (int)client_data;
  const char * str = NULL;
  static const char * loc[] = {"0,0", "1,0", "2,0", "3,0", "4,0", "5,0", "6,0", "7,0",
			"0,1", "1,1", "2,1", "3,1", "4,1", "5,1", "6,1", "7,1"};
  FaceplateGUI * fGUI = getFaceplateGUI(w);

  if (fGUI->current < 0 || fGUI->fpg == NULL)
    return;

  if (button == FP_FACEPLATE_CELL_RESET) {
    str = NULL;
    highligthCell(fGUI, NULL); 
    XtSetSensitive(fGUI->wFaceplateX, True);
    XtSetSensitive(fGUI->wFaceplateY, True);
    XtSetSensitive(fGUI->wFaceplateHeigth, True);
    XtSetSensitive(fGUI->wFaceplateWidth, True);
  }
  else {
    str = loc[button - FP_FACEPLATE_CELL_00];
    highligthCell(fGUI, w);
    XtSetSensitive(fGUI->wFaceplateX, False);
    XtSetSensitive(fGUI->wFaceplateY, False);
    XtSetSensitive(fGUI->wFaceplateHeigth, False);
    XtSetSensitive(fGUI->wFaceplateWidth, False);
  }

  strcpy(fGUI->position, str ? str : "");
}


/*
** WorkShop Visual Stub cbFaceplateAdlFile
*/

void cbFaceplateAdlFile(Widget w, XtPointer client_data, XtPointer xt_call_data)
{
  int cd = (int)client_data;
  FaceplateGUI * fGUI = getFaceplateGUI(w);

  switch(cd) 
    {
    case FP_FACEPLATE_ADL_FILE_BROWSE:
      create_adlSelectDlg(fGUI->wFaceplateDlgShell);
      break;

    case FP_FACEPLATE_ADL_FILE:
	  /* T. Straumann: fixed memory leak */
	  {char *tmp=XmTextFieldGetString(w);
      	showMacroInWidgets(fGUI, tmp, NULL);
		if (tmp) XtFree(tmp);
	  }
      break;

    default :
      INFORM_INTERNAL_ERROR();
      break;
    }
}

/*
** WorkShop Visual Stub cbFaceplateSubmit
*/

void cbFaceplateSubmit(Widget w, XtPointer client_data, XtPointer xt_call_data)
{
  int button = (int)client_data;
  FaceplateGUI * fGUI = getFaceplateGUI(w);

  if (fGUI == NULL){ 
    INFORM_INTERNAL_ERROR();
    return;
  }

  XtSetSensitive(w, False);

  switch(button)
    {
    case FP_FACEPLATE_ADD_NEW: {
      Faceplate entry;
      Widget toggle;

      entry.position = entry.adl = entry.macro = NULL;

      getFaceplateFromWidgets(fGUI, &entry);

      appendNewFaceplate(fGUI->fpg, 
			 entry.position,
			 entry.x,
			 entry.y,
			 entry.w,
			 entry.h,
			 entry.adl,
			 entry.macro);

      freeFaceplate(&entry);

      toggle = createFaceplateToggleButton(fGUI, fGUI->fpg->entriesNum-1);
      XmToggleButtonSetState(toggle, False, False);
    }      
    break;

    case FP_FACEPLATE_APPLY:
      if(fGUI != NULL && fGUI->current >= 0)
        getFaceplateFromWidgets(fGUI, fGUI->fpg->entries[fGUI->current]);
      break;
    case FP_FACEPLATE_DELETE: {
      WidgetList wList;
      int        numWidgets;
      int        i;

      destroyFaceplate(fGUI->fpg, fGUI->current);

      if (fGUI->fpg->entriesNum == 0)
	fGUI->current = -1;
      else
	fGUI->current = 0;


      /* destroy old faceplate toggle buttons
       */
      XtVaGetValues(fGUI->wGroupRowColumn,
		    XtNchildren, &wList,
		    XtNnumChildren, &numWidgets,
		    NULL);
      
      for (i = 0; i < numWidgets; i++)
	{
	  if (XmIsToggleButtonGadget(wList[i]))
	    XtDestroyWidget(wList[i]);
	}
      
      /* faceplates 
       */
      for (i = 0; i < fGUI->fpg->entriesNum; i++) {
	Widget toggle = createFaceplateToggleButton(fGUI, i);
	
	if (i == 0)
	  XmToggleButtonSetState(toggle, True, False);
	else
	  XmToggleButtonSetState(toggle, False, False);
      }

      if (fGUI->fpg->entriesNum > 0)
	showResourcesOfFaceplate(fGUI, fGUI->fpg->entries[0]);
    }
    break;

    default :
      break;
    }

  XtSetSensitive(w, True);
}

/*
** WorkShop Visual Stub cbAdlFileSelect
*/

void cbAdlFileSelect(Widget w, XtPointer client_data, XtPointer xt_call_data)
{
  XmPushButtonCallbackStruct *call_data = (XmPushButtonCallbackStruct *) xt_call_data ;

  FaceplateGUI * fGUI = getFaceplateGUI(w);

  if (fGUI == NULL)
    INFORM_INTERNAL_ERROR();

  XtUnmanageChild(w);

  switch(call_data->reason)
    {
    case XmCR_CANCEL:
      break;

    case XmCR_OK: {
      char *filename;
      
      XmSelectionBoxCallbackStruct *call_data =
	(XmSelectionBoxCallbackStruct *) xt_call_data;
	    
      /* if no list element selected, simply return */
      if (call_data->value == NULL) return;
      
      /* get the filename string from the selection box */
      XmStringGetLtoR(call_data->value, XmSTRING_DEFAULT_CHARSET, &filename);
      
      if (filename) {
	XmTextFieldSetString (fGUI->wSelectedAdlFile, filename);
	showMacroInWidgets(fGUI, filename, NULL);
	XtFree(filename);
      }
    }
    break;

    default :
      INFORM_INTERNAL_ERROR();
      break;
    }
}

/*
** WorkShop Visual Stub cbFaceplateFileSelect
*/

void cbFaceplateFileSelect(Widget w, XtPointer client_data, XtPointer xt_call_data)
{
  int cd = (int)client_data;
  FaceplateGUI * fGUI = getFaceplateGUI(w);

  if (fGUI == NULL)
    INFORM_INTERNAL_ERROR();

  XtUnmanageChild(w);
  XUngrabPointer(XtDisplay(w),CurrentTime);

  switch(cd) 
    {
    case FP_FILE_SELECT_OK: {
      char *filename;
      
      XmSelectionBoxCallbackStruct *call_data =
	(XmSelectionBoxCallbackStruct *) xt_call_data;
	    
      /* if no list element selected, simply return */
      if (call_data->value == NULL) return;
      
      /* get the filename string from the selection box */
      XmStringGetLtoR(call_data->value, XmSTRING_DEFAULT_CHARSET, &filename);
      
      if (filename) {
	if (fGUI->saveIt) {
	  renewString(&fGUI->fpg->name, filename);
	  transferFromWidgetsIntoData(fGUI);
	  writeFaceplateGroupToFile(fGUI->wFaceplateDlg,
				    fGUI->fpg, fGUI->fpg->name, False);
	} 
	else {
	  fGUI->fpg = createFaceplateGroup(filename);
	  setFaceplateSensitivity(fGUI);
	  transferFromDataIntoWidgets(fGUI);
	  XtFree(filename);
	}
	
	XtVaSetValues(fGUI->wFaceplateDlgShell, 
		      XtNtitle, fGUI->fpg->name, 
		      NULL);
      }
    }
    break;

    case FP_FILE_SELECT_CANCEL:
      break;
      
    default :
      INFORM_INTERNAL_ERROR();
      break;
    }
}

