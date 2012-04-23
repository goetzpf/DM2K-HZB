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
 *
 *****************************************************************************
 *
 * .04  02-07-97        Fabien  Addition of sensitivity
 *
 *****************************************************************************
*/

#include "dm2k.h"
#include <X11/IntrinsicP.h>

typedef struct _Menu {
  DlElement   *dlElement;
  Record      *record; 
  UpdateTask  *updateTask;
  Record      *sensitiveRecord;  /* if the sensitivity is control by a PV */
  Boolean     sensitive;         /* sensitive property */
  Pixel       color;
  int         btnNumber;         /* armed menu item number */
  int         pdBtnNumber;       /* saved armed menu item number */
} Menu;

static UpdateTask * menuCreateRunTimeInstance(DisplayInfo *, DlElement *);
static void menuCreateEditInstance(DisplayInfo *, DlElement *);

static void menuDraw(XtPointer);
static void menuUpdateValueCb(XtPointer);
static void menuUpdateGraphicalInfoCb(XtPointer);
static void menuDestroyCb(XtPointer cd);
static void menuValueChangedCb(Widget, XtPointer, XtPointer);
static void menuName(XtPointer, char **, short *, int *);
static void menuInheritValues(ResourceBundle *pRCB, DlElement *p);
static void menuGetValues(ResourceBundle *pRCB, DlElement *p);
static void menuInfo (char *, Widget, DisplayInfo *, DlElement *, XtPointer);
static void menuDeferredAction (DlElement *, Boolean);
static Boolean dialogDisplayType (DlElement *);
static int getCurrentValue (Menu *);

static void destroyDlElement (DlElement * element);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);

static DlDispatchTable menuDlDispatchTable = {
         createDlMenu,
         destroyDlElement,
         executeMethod,
         writeDlMenu,
         NULL,
         menuGetValues,
         menuInheritValues,
         NULL,
         NULL,
         genericMove,
         genericScale,
	 NULL,
	 menuInfo,
	 menuDeferredAction
};


static void destroyDlMenu (DlMenu * dlMenu)
{
  if (dlMenu == NULL)
    return;

  objectAttributeDestroy(&(dlMenu->object));
  controlAttributeDestroy(&(dlMenu->control));
  sensitveAttributeDestroy(&(dlMenu->sensitive));

  free((char*)dlMenu);
}

static void destroyDlElement (DlElement * element)
{
  if (element == NULL || element->type != DL_Menu) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  destroyDlMenu (element->structure.menu);
  free((char*)element);
}


int menuFontListIndex(int height)
{
  int i;
/* don't allow height of font to exceed 90% - 4 pixels of menu widget
 *	(includes nominal 2*shadowThickness=2 shadow)
 */
  for (i = MAX_FONTS-1; i >=  0; i--) {
    if ( ((int)(.90*height) - 4) >= 
			(fontTable[i]->ascent + fontTable[i]->descent))
	return(i);
  }
  return (0);
}

static UpdateTask * menuCreateRunTimeInstance
   (DisplayInfo * displayInfo,
    DlElement   * dlElement) 
{
  Menu       * pm;
  DlMenu     * dlMenu = dlElement->structure.menu;
  UpdateTask * updateTask = NULL;

  pm = DM2KALLOC(Menu);
  if (pm == NULL) {
    dm2kPrintf("menuCreateRunTimeInstance : memory allocation error\n");
    return updateTask;
  }

  pm->dlElement = dlElement;
  dlMenu->object.runtimeDescriptor = (XtPointer) pm;

  pm->sensitive = False;    /* to be set by sensitiveCreateRecord */

  updateTask = pm->updateTask = updateTaskAddTask(displayInfo,
						  &(dlMenu->object),
						  menuDraw,
						  (XtPointer)pm);

  if (pm->updateTask == NULL) {
    dm2kPrintf("menuCreateRunTimeInstance : memory allocation error\n");
  } else {
    updateTaskAddDestroyCb(pm->updateTask,menuDestroyCb);
    updateTaskAddNameCb(pm->updateTask,menuName);
  }
  pm->record = dm2kAllocateRecord(dlMenu->control.ctrl,
                  menuUpdateValueCb,
                  menuUpdateGraphicalInfoCb,
                  (XtPointer) pm);

  if (pm->record)
    pm->btnNumber = (int)pm->record->value;
  pm->pdBtnNumber = -1;    /* not a valid value ! */

  pm->sensitiveRecord = sensitiveCreateRecord (&dlMenu->sensitive,
				menuUpdateValueCb,
				NULL,
				(XtPointer) pm,
				&pm->sensitive);

  drawWhiteRectangle(pm->updateTask);
  pm->color = displayInfo->colormap[dlMenu->control.bclr];
  return updateTask;
}


static void menuCreateEditInstance
   (DisplayInfo *displayInfo, 
    DlElement *dlElement) 
{
  Arg args[20];
  XmString buttons[1];
  XmButtonType buttonType[1];
  int nargs;
  Widget menu, mb;
  XmFontList fontList;
  XmString xmStr;
  Dimension useableWidth, useableHeight;
  DlMenu *dlMenu = dlElement->structure.menu;

  buttons[0] = XmStringCreateSimple("menu");
  buttonType[0] = XmPUSHBUTTON;
  /* from the menu structure, we've got Menu's specifics */
  /*
   * take a guess here  - keep this constant the same is in dm2kCA.c
   *	this takes out the extra space needed for the cascade pixmap, etc
   */
  useableWidth = dlMenu->object.width;
  useableHeight = dlMenu->object.height;

  nargs = 0;
  XtSetArg(args[nargs],XmNforeground, 
               (Pixel) displayInfo->colormap[dlMenu->control.clr]);  nargs++;

  XtSetArg(args[nargs],XmNbackground,
	   (Pixel) displayInfo->colormap[dlMenu->control.bclr]);  nargs++;
  XtSetArg(args[nargs],XmNtearOffModel,    XmTEAR_OFF_DISABLED);  nargs++;

  menu = XmCreatePulldownMenu(displayInfo->drawingArea,
			      "menu", args, nargs);

  nargs = 2;
  XtSetArg(args[nargs],XmNwidth,dlMenu->object.width-27); nargs++;
  XtSetArg(args[nargs],XmNheight,dlMenu->object.height-6); nargs++;
  XtSetArg(args[nargs],XmNrecomputeSize,   FALSE);  nargs++;
  XtSetArg(args[nargs],XmNmarginWidth,     0);  nargs++;
  XtSetArg(args[nargs],XmNmarginHeight,    0);  nargs++;
  fontList = fontListTable[menuFontListIndex(dlMenu->object.height)];
  XtSetArg(args[nargs],XmNfontList,        fontList);  nargs++;

  xmStr  = XmStringCreateSimple("menu");
  XtSetArg(args[nargs], XmNlabelString, xmStr);  nargs++;
  mb = XmCreatePushButtonGadget(menu, "menuButtons", args, nargs);
  XmStringFree(xmStr);
  XtManageChild(mb);

  nargs = 2;
  XtSetArg(args[nargs],XmNrecomputeSize,   False);  nargs++;
  XtSetArg(args[nargs],XmNx,            dlMenu->object.x); nargs++;
  XtSetArg(args[nargs],XmNy,            dlMenu->object.y); nargs++;
  XtSetArg(args[nargs],XmNmarginWidth,  0); nargs++;
  XtSetArg(args[nargs],XmNmarginHeight, 0); nargs++;
  XtSetArg(args[nargs],XmNentryBorder,  0); nargs++;
  XtSetArg(args[nargs],XmNspacing,  0); nargs++;
  XtSetArg(args[nargs],XmNshadowThickness,  0); nargs++;
  XtSetArg(args[nargs],XmNborderWidth,  0); nargs++;
  XtSetArg(args[nargs],XmNsubMenuId,    menu); nargs++;

  dlElement->widget =
    XmCreateOptionMenu(displayInfo->drawingArea,
		       "optionMenu",args,nargs);

  /* unmanage the option label gadget, manage the option menu 
   */
  XtUnmanageChild(XmOptionLabelGadget(dlElement->widget));
  XtManageChild(dlElement->widget);

  /* remove all translations if in edit mode 
   * add button press handler
   */
  controlHandler (displayInfo, dlElement);
  return;
}

static UpdateTask * executeMethod
  (DisplayInfo * displayInfo,
   DlElement   * dlElement)
{
  UpdateTask * updateTask = NULL;

  switch (displayInfo->traversalMode) {
  case DL_EXECUTE :
    updateTask =  menuCreateRunTimeInstance(displayInfo,dlElement);
    break;
  case DL_EDIT :
    if (dlElement->widget) {
      XtDestroyWidget(dlElement->widget);
      dlElement->widget = NULL;
    }
    menuCreateEditInstance(displayInfo,dlElement);
    break;
  default :
    break;
  }
  return updateTask;
}


static void menuUpdateGraphicalInfoCb(XtPointer cd)
 {
  Record    * pd = (Record *) cd;
  Menu      * pm = (Menu *) pd->clientData;
  DlMenu    * dlMenu = pm->dlElement->structure.menu;
  XmFontList  fontList;
  int         i, nargs;
  Arg         args[50];
  Widget      buttons[db_state_dim], menu;
  WidgetUserData * userData;

  fontList = fontListTable[menuFontListIndex(dlMenu->object.height)];

  /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
  /* !!!!! This is a temperory work around !!!!! */
  /* !!!!! for the reconnection.           !!!!! */
  /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
  dm2kRecordAddGraphicalInfoCb(pm->record,NULL);

  /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
  /* !!!!! End work around                 !!!!! */
  /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */


  /* alloc servise structures
   */
  userData = DM2KALLOC (WidgetUserData);

  if (userData == NULL) {
    dm2kPrintf("menuUpdateGraphicalInfoCb: memory allocation error\n");
    return;
  }

  /* buildMenu function needs an extra entry to work 
   */
  if (pd->dataType != DBF_ENUM) {
    dm2kPrintf("menuUpdateGraphicalInfoCb :\n    %s\n    \"%s\" %s\n\n",
	       "Cannot create Choice Button,",
	       dlMenu->control.ctrl,"is not an ENUM type!");
    dm2kPostTime();
    return;
  }

  nargs = 0;
  XtSetArg(args[nargs],XmNforeground, ((dlMenu->clrmod == ALARM)?
				   alarmColorPixel[pd->severity] :
				   pm->updateTask->displayInfo->colormap[dlMenu->control.clr]));  nargs++;
  XtSetArg(args[nargs],XmNbackground,
	   pm->updateTask->displayInfo->colormap[dlMenu->control.bclr]);  nargs++;

  /* add userData 
   */
  userData->privateData    = (char*) pm;
  userData->updateTask = (pm ? pm->updateTask : NULL);
  XtSetArg(args[nargs],XmNuserData,        userData); nargs++;
  XtSetArg(args[nargs],XmNtearOffModel,    XmTEAR_OFF_DISABLED);  nargs++;

  menu = XmCreatePulldownMenu(pm->updateTask->displayInfo->drawingArea,
			      "menu", args, nargs);

  /* destroy callback should free allocated memory
   */
  XtAddCallback (menu, XmNdestroyCallback, freeUserDataCB, NULL);
  
  
  nargs = 2;
  XtSetArg(args[nargs],XmNwidth,dlMenu->object.width-31); nargs++;
  XtSetArg(args[nargs],XmNheight,dlMenu->object.height-6); nargs++;
  XtSetArg(args[nargs],XmNrecomputeSize,   FALSE);  nargs++;
  XtSetArg(args[nargs],XmNmarginWidth,     0);  nargs++;
  XtSetArg(args[nargs],XmNmarginHeight,    0);  nargs++;
  XtSetArg(args[nargs],XmNfontList,        fontList);  nargs++;
  XtSetArg(args[nargs],XmNuserData,        userData); nargs++;

  for (i = 0; i <= pd->hopr; i++) 
  {
    XmString xmStr  = XmStringCreateSimple(pd->stateStrings[i]);
    XtSetArg(args[nargs], XmNlabelString, xmStr);
    buttons[i] = XmCreatePushButtonGadget(menu, "menuButtons", args, nargs+1);

    XtAddCallback(buttons[i], XmNactivateCallback, menuValueChangedCb, (XtPointer) i);
    XmStringFree(xmStr);
  }

  XtManageChildren(buttons,i);

  nargs = 0;

  XtSetArg(args[nargs],XmNforeground,
	   ((dlMenu->clrmod == ALARM)?
	    alarmColorPixel[pd->severity] :
	    pm->updateTask->displayInfo->colormap[dlMenu->control.clr]));  nargs++;

  XtSetArg(args[nargs],XmNbackground,
	   pm->updateTask->displayInfo->colormap[dlMenu->control.bclr]); nargs++;

  XtSetArg(args[nargs],XmNrecomputeSize,   False);  nargs++;
  XtSetArg(args[nargs],XmNx,            dlMenu->object.x); nargs++;
  XtSetArg(args[nargs],XmNy,            dlMenu->object.y); nargs++;

  XtSetArg(args[nargs],XmNwidth,dlMenu->object.width); nargs++;
  XtSetArg(args[nargs],XmNheight,dlMenu->object.height); nargs++;

  XtSetArg(args[nargs],XmNmarginWidth,  0); nargs++;
  XtSetArg(args[nargs],XmNmarginHeight, 0); nargs++;
  XtSetArg(args[nargs],XmNsubMenuId,    menu); nargs++;

  /* add userData
   */
  userData = DM2KALLOC (WidgetUserData);

  if (userData == NULL) {
    dm2kPrintf("menuUpdateGraphicalInfoCb: memory allocation error\n");
  }
  else {
    userData->privateData    = (char*) pm;
    userData->updateTask = (pm ? pm->updateTask : NULL);
    XtSetArg(args[nargs],XmNuserData, userData);  nargs++;
  }

  pm->dlElement->widget =
    XmCreateOptionMenu(pm->updateTask->displayInfo->drawingArea,
		       "optionMenu",args,nargs);


  if (userData)
    XtAddCallback (pm->dlElement->widget, XmNdestroyCallback,
		   freeUserDataCB, NULL);

  /* unmanage the option label gadget, manage the option menu 
   */
  XtUnmanageChild(XmOptionLabelGadget(pm->dlElement->widget));
  XtManageChild(pm->dlElement->widget);

  /* add in drag/drop translations 
   */
  XtOverrideTranslations(pm->dlElement->widget,parsedTranslations);
  updateTaskMarkUpdate(pm->updateTask);
}

static void menuUpdateValueCb(XtPointer cd)
{
  Menu *pm = (Menu *) ((Record *) cd)->clientData;

  updateTaskMarkUpdate(pm->updateTask);
}


static void menuDraw(XtPointer cd) 
{
  Menu   * pm = (Menu *) cd;
  Record * pd = pm->record;
  Widget   widget = pm->dlElement->widget;
  DlMenu * dlMenu = pm->dlElement->structure.menu;


  if (pd->connected) {
    if (pd->readAccess) {
      if ((widget) && !XtIsManaged(widget))
        XtManageChild(widget);
 
      if (pd->precision < 0) return;

      if (pd->dataType == DBF_ENUM) {
        Widget menuWidget;
        WidgetList children;
        Cardinal numChildren;
        int i;

        XtVaGetValues(widget,XmNsubMenuId,&menuWidget,NULL);
        XtVaGetValues(menuWidget,
		XmNchildren,&children,
		XmNnumChildren,&numChildren,
		NULL);
        i = getCurrentValue (pm);
        if ((i >=0) && (i < (int) numChildren)) {
          XtVaSetValues(widget,XmNmenuHistory,children[i],NULL);
	  pm->btnNumber = i;      /* current active item */
	  pm->pdBtnNumber = (int)pd->value;
        } else {
	  char theMesg[260];

	  sprintf(theMesg,"menuDraw: invalid menuHistory child(%d:max=%d:value=%f)\n",
		  i, numChildren-1, pd->value);
          dm2kPrintf(theMesg);
          dm2kPostTime();
          return;
        }
        switch (dlMenu->clrmod) {
          case STATIC :
          case DISCRETE :
            break;
          case ALARM :
            XtVaSetValues(widget,XmNforeground,alarmColorPixel[pd->severity],NULL);
            XtVaSetValues(menuWidget,XmNforeground,alarmColorPixel[pd->severity],NULL);
            break;
          default :
            dm2kPrintf("Message: Unknown color modifier!\n");
            dm2kPrintf("Channel Name : %s\n",dlMenu->control.ctrl);
            dm2kPostMsg("Error: menuUpdateValueCb\n");
            return;
        }
      } else {
        dm2kPrintf("Message: Data type must be enum!\n");
        dm2kPrintf("Channel Name : %s\n",dlMenu->control.ctrl);
        dm2kPostMsg("Error: menuUpdateValueCb\n");
        return;
      }
      if (pd->writeAccess)
        XDefineCursor(XtDisplay(widget),XtWindow(widget),rubberbandCursor);
      else
        XDefineCursor(XtDisplay(widget),XtWindow(widget),noWriteAccessCursor);

      /* sensitive Controller */
      sensitiveSetWidget (widget, &dlMenu->sensitive,
			  pm->sensitiveRecord, pm->updateTask);

    } else {
      if (widget) XtUnmanageChild(widget);
      draw3DPane(pm->updateTask,pm->color);
      draw3DQuestionMark(pm->updateTask);
    }
  } else {
    if ((widget) && XtIsManaged(widget))
      XtUnmanageChild(widget);
    drawWhiteRectangle(pm->updateTask);
  }
}

static void menuDestroyCb(XtPointer cd) {
  Menu *pm = (Menu *) cd;
  if (pm) {
    dm2kDestroyRecord(pm->record); 
    sensitiveDestroyRecord (&pm->sensitiveRecord, &pm->sensitive);
    free((char *)pm);
  }
}

static int getCurrentValue (Menu *pm)
{
  Record *pd;
  int btnNumber;

  pd = pm->record;
  if ( ! pd ) return (-1);

  if ( dialogDisplayType (pm->dlElement) ) {
    btnNumber = ( (int)pd->value == pm->pdBtnNumber ) ? pm->btnNumber : (int)pd->value;
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


static void menuSendData (
  Menu *pm,
  int btnNumber,
  Boolean executeFlag)      /* if True, execute in any case */
{
  Record *pd;

  if ( ! executeFlag ) {
    DisplayInfo *displayInfo;
    displayInfo = pm->dlElement->displayInfo;
    if ( displayInfo == NULL ) return;
    if ( displayInfo->displayType != NORMAL_DISPLAY ) return;   /* deferred action */
  }

  pd = pm->record;
  if ( pd == NULL ) return;     /* no PV attached */

  if ( btnNumber >= 0 ) {
    dm2kSendDouble(pm->record,(double)btnNumber);
  }
}


static void menuAction (
  Menu *pm,
  Boolean DeferredFlag) /* if True, deferred action case */
{
  Record *pd;

  pd = pm->record;
  if ( pd == NULL ) return;     /* no PV attached */

  if (pd->connected) {
    if (pd->writeAccess) {
      menuSendData (pm, pm->btnNumber, DeferredFlag);
    } else {
      fputc('\a',stderr);
      if ( ! DeferredFlag ) {
	pm->btnNumber = (int)pd->value;
	menuUpdateValueCb((XtPointer)pm->record);
      }
    }
  } else if ( ! DeferredFlag ) {
    dm2kPrintf("menuValueChangedCb : %s not connected",
	      pm->dlElement->structure.menu->control.ctrl);
  }
}


static void menuDeferredAction (DlElement *dlElement, Boolean applyFlag)
{
  DlMenu *dlMenu;
  Menu *pm;
  Record *pd;

  dlMenu = dlElement->structure.menu;
  pm = (Menu *) dlMenu->object.runtimeDescriptor;
  if ( pm == NULL ) return;    /* nothing instancied */

  if ( pm->record == NULL ) return;     /* no PV attached */

  if ( applyFlag ) {
    menuAction (pm, True);
  } else {      /* reset to current state */
    pd = pm->record;
    if ( pd ) {
      pm->btnNumber = (int)pd->value;
      menuDraw ((XtPointer) pm);
    }
  }
}

static void menuValueChangedCb(
  Widget  w,
  XtPointer clientData,
  XtPointer callbackStruct)
{
  Menu                       * pm;
  int                          btnNumber = (int) clientData;
  WidgetUserData             * userData = NULL;
  XmPushButtonCallbackStruct * call_data = 
                                (XmPushButtonCallbackStruct *) callbackStruct;

  
  /* only do ca_put if this widget actually initiated the channel change
   */
  if (call_data->event != NULL && call_data->reason == XmCR_ACTIVATE) 
  {
    /* button's parent (menuPane) has the displayInfo pointer 
     */
    XtVaGetValues(XtParent(w), XmNuserData, &userData, NULL);
    if (userData == NULL || userData->privateData == NULL)
      return;
    
    pm = (Menu *) userData->privateData;
    pm->btnNumber = btnNumber;
    menuAction (pm, False);
  }
}

static void menuName(XtPointer cd, char **name, short *severity, int *count) {
  Menu *pm = (Menu *) cd;
  *count = 1;
  name[0] = (char*)pm->record->name;
  severity[0] = pm->record->severity;
}

DlElement *createDlMenu(DlElement *p)
{
  DlMenu *dlMenu;
  DlElement *dlElement;
 
  dlMenu = DM2KALLOC(DlMenu); 

  if (dlMenu == NULL)
    return 0;

  if (p != NULL) {
    objectAttributeCopy(&(dlMenu->object), &(p->structure.menu->object));
    controlAttributeCopy(&(dlMenu->control), &(p->structure.menu->control));
    sensitveAttributeCopy(&(dlMenu->sensitive), &(p->structure.menu->sensitive));

    dlMenu->clrmod = p->structure.menu->clrmod;
  } else {
    objectAttributeInit(&(dlMenu->object));
    controlAttributeInit(&(dlMenu->control));
    sensitveAttributeInit(&(dlMenu->sensitive));

    dlMenu->clrmod = STATIC;
  }

  dlElement = createDlElement(DL_Menu, (XtPointer) dlMenu,
			      &menuDlDispatchTable);
  if (dlElement == NULL)
    destroyDlMenu(dlMenu);
 
  return(dlElement);
}

DlElement *parseMenu(DisplayInfo *displayInfo)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;
  DlMenu *dlMenu;
  DlElement *dlElement = createDlMenu(NULL);
 
  if (!dlElement) return 0;

  dlMenu = dlElement->structure.menu;
  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
      case T_WORD:
        if (STREQL(token,"object"))
          parseObject(displayInfo,&(dlMenu->object));
        else
        if (STREQL(token,"control"))
          parseControl(displayInfo,&(dlMenu->control));
	else if (STREQL(token,"sensitive")) {
	  /* add on for sensitivity property */
	  parseSensitive (displayInfo, &(dlMenu->sensitive));
	}
        else
        if (STREQL(token,"clrmod")) {
          getToken(displayInfo,token);
          getToken(displayInfo,token);
          if (STREQL(token,"static"))
            dlMenu->clrmod = STATIC;
          else
          if (STREQL(token,"alarm"))
            dlMenu->clrmod = ALARM;
          else
          if (STREQL(token,"discrete"))
            dlMenu->clrmod = DISCRETE;
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

void writeDlMenu(
  FILE *stream,
  DlElement *dlElement,
  int level)
{
  int i;
  char indent[256]; level=MIN(level,256-2);
  DlMenu *dlMenu = dlElement->structure.menu;
 
  for (i = 0; i < MIN(level,256-2); i++) indent[i] = '\t';
  indent[i] = '\0';
 
  fprintf(stream,"\n%smenu {",indent);
  writeDlObject(stream,&(dlMenu->object),level+1);
  writeDlControl(stream,&(dlMenu->control),level+1);
  writeDlSensitive(stream,&(dlMenu->sensitive),level+1);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
  if (dlMenu->clrmod != STATIC) 
    fprintf(stream,"\n%s\tclrmod=\"%s\"",indent,
                stringValueTable[dlMenu->clrmod]);
#ifdef SUPPORT_0201XX_FILE_FORMAT
	} else {
    fprintf(stream,"\n%s\tclrmod=\"%s\"",indent,
                stringValueTable[dlMenu->clrmod]);
	}
#endif
  fprintf(stream,"\n%s}",indent);
 
}

static void menuInheritValues(ResourceBundle *pRCB, DlElement *p) {
  DlMenu *dlMenu = p->structure.menu;

  updateElementControlAttribute   (pRCB, &dlMenu->control);
  updateElementSensitiveAttribute (pRCB, &dlMenu->sensitive);

  dm2kGetValues(pRCB,
    CLRMOD_RC,     &(dlMenu->clrmod),
    -1);
}

static void menuGetValues(ResourceBundle *pRCB, DlElement *p) {
  DlMenu *dlMenu = p->structure.menu;

  updateElementObjectAttribute    (pRCB, &dlMenu->object);
  updateElementControlAttribute   (pRCB, &dlMenu->control);
  updateElementSensitiveAttribute (pRCB, &dlMenu->sensitive);

  dm2kGetValues(pRCB,
    CLRMOD_RC,     &(dlMenu->clrmod),
    -1);
}


static void menuInfo (char *msg, Widget w,
		      DisplayInfo *displayInfo,
		      DlElement *element,
		      XtPointer objet)
{
  Menu *pm;
  DlMenu *dlMenu;

  if (globalDisplayListTraversalMode != DL_EXECUTE) {
    dlMenu = element->structure.menu;
    sensitiveControllerInfoSimple (msg, &dlMenu->control,
				   &dlMenu->sensitive);
    return;
  }

  pm = (Menu *) objet;
  dlMenu = pm->dlElement->structure.menu;
  sensitiveControllerInfo (msg,
		      &dlMenu->control, pm->record,
		      &dlMenu->sensitive, pm->sensitiveRecord);
}
