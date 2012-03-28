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
 *                              - add version number into the FILE object
 * .03  09-08-95        vong    conform to c++ syntax
 *
 *****************************************************************************
 *
 *      24-02-97    Fabien  object information with MB3
 *
 *****************************************************************************
*/

#include <X11/IntrinsicP.h>
#include "dm2k.h"

static void shellCommandInheritValues(ResourceBundle *pRCB, DlElement *p);
static void shellCommandGetValues(ResourceBundle *pRCB, DlElement *p);
static void shellCommandInfo (char *, Widget, DisplayInfo *, DlElement *, XtPointer);

static void destroyDlElement (DlElement * element);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);

static DlDispatchTable shellCommandDlDispatchTable = {
         createDlShellCommand,
         destroyDlElement,
         executeMethod,
         writeDlShellCommand,
         NULL,
         shellCommandGetValues,
         shellCommandInheritValues,
         NULL,
         NULL,
         genericMove,
         genericScale,
	 NULL,
	 shellCommandInfo
};

static void destroyDlShellCommand (DlShellCommand * dlShellCommand)
{
  int cmdNumber;

  if (dlShellCommand == NULL)
    return;

  objectAttributeDestroy(&(dlShellCommand->object));

  for (cmdNumber = 0; cmdNumber < MAX_SHELL_COMMANDS; cmdNumber++) 
  {
   DM2KFREE(dlShellCommand->command[cmdNumber].label);
   DM2KFREE(dlShellCommand->command[cmdNumber].command);
   DM2KFREE(dlShellCommand->command[cmdNumber].args);
  }

  free((char*)dlShellCommand);
}

static void destroyDlElement (DlElement * element)
{
  if (element == NULL || element->type != DL_ShellCommand) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  destroyDlShellCommand (element->structure.shellCommand);
  free((char*)element);
}

#ifdef __cplusplus
static void freePixmapCallback(Widget w, XtPointer cd, XtPointer)
#else
static void freePixmapCallback(Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  Pixmap pixmap = (Pixmap) cd;
  if (pixmap != (Pixmap)NULL) 
/* T. Straumann: The pixmap is not in motif's cache
 *				 since it was created by XCreatePixmap()
 *				 directly.
 *				 Furthermore: the 'widget' might be a gadget
 *				 --> use XtDisplayOfObject()
    XmDestroyPixmap(XtScreen(w),pixmap);
 */
	XFreePixmap(XtDisplayOfObject(w),pixmap);
}

/*
 * local function to render the related display icon into a pixmap
 */
static void renderShellCommandPixmap(Display *display, Pixmap pixmap,
        Pixel fg, Pixel bg, Dimension width, Dimension height)
{
  typedef struct { float x; float y;} XY;
/* icon is based on the 25 pixel (w & h) bitmap shellCommand25 */
  static float rectangleX = 12./25., rectangleY = 4./25.,
        rectangleWidth = 3./25., rectangleHeight = 14./25.;
  static float dotX = 12./25., dotY = 20./25.,
        dotWidth = 3./25., dotHeight = 3./25.;
  GC gc;

  gc = XCreateGC(display,pixmap,0,NULL);
  XSetForeground(display,gc,bg);
  XFillRectangle(display,pixmap,gc,0,0,width,height);
  XSetForeground(display,gc,fg);

  XFillRectangle(display,pixmap,gc,
        (int)(rectangleX*width),
        (int)(rectangleY*height),
        (unsigned int)(MAX(1,(unsigned int)(rectangleWidth*width))),
        (unsigned int)(MAX(1,(unsigned int)(rectangleHeight*height))) );

  XFillRectangle(display,pixmap,gc,
        (int)(dotX*width),
        (int)(dotY*height),
        (unsigned int)(MAX(1,(unsigned int)(dotWidth*width))),
        (unsigned int)(MAX(1,(unsigned int)(dotHeight*height))) );

  XFreeGC(display,gc);
}


static void executeDlShellCommand_PushButton
  (DisplayInfo      * displayInfo,
   DlElement        * dlElement,
   DlShellCommand * dlRelDisp)
{
  Arg                args[15];
  int                n;
  XmString           xmString = NULL;
  WidgetUserData   * userData;
  Pixel              background = displayInfo->colormap[dlRelDisp->bclr];
  Pixel              foreground = displayInfo->colormap[dlRelDisp->clr];
  Pixmap             shellCommandPixmap = 0;
  unsigned int       pixmapSize;


  n = 0;
  /* geometric resources
   */
  XtSetArg(args[n],XmNx,      (Position)dlRelDisp->object.x); n++;
  XtSetArg(args[n],XmNy,      (Position)dlRelDisp->object.y); n++;
  XtSetArg(args[n],XmNwidth,  (Dimension)dlRelDisp->object.width); n++;
  XtSetArg(args[n],XmNheight, (Dimension)dlRelDisp->object.height); n++;
  
  /* specific resources
   */
  pixmapSize = MIN(dlRelDisp->object.width, dlRelDisp->object.height);
      
  /* allowing for shadows etc 
   */
  pixmapSize = (unsigned int) MAX(1,(int)pixmapSize - 8);
  
  /* create shellCommand icon (render to appropriate size) 
       */
  shellCommandPixmap = 
    XCreatePixmap(display,
		  RootWindow(display,screenNum),
		  pixmapSize, pixmapSize,
		  XDefaultDepth(display,screenNum));
  
  renderShellCommandPixmap(display,shellCommandPixmap,
			   foreground, background,
			   pixmapSize, pixmapSize);
  
  XtSetArg(args[n],XmNlabelPixmap, shellCommandPixmap); n++;
  XtSetArg(args[n],XmNlabelType,   XmPIXMAP); n++;

  
  XtSetArg(args[n],XmNhighlightThickness, 0); n++;
  XtSetArg(args[n],XmNhighlightOnEnter,   True); n++;
  XtSetArg(args[n],XmNindicatorOn,        False); n++;
  XtSetArg(args[n],XmNrecomputeSize,      False); n++;

  userData = DM2KALLOC (WidgetUserData);
  if (userData == NULL) {
     dm2kPrintf("executeDlShellCommand: memory allocation error\n");
  } else  {
     userData->privateData    = (char*) displayInfo;
     userData->updateTask = NULL;
     XtSetArg(args[n],XmNuserData,	userData ); n++;
  }
  
  /* colors
   */
  XtSetArg(args[n],XmNforeground, foreground); n++;
  XtSetArg(args[n],XmNbackground, background); n++;

  /* create push button widget
   */
  dlElement->widget = XtCreateWidget("shellCommandPushButton",
				     xmPushButtonWidgetClass, 
				     displayInfo->drawingArea, 
				     args,
				     n);

  /* mode dependent
   */
  if (globalDisplayListTraversalMode == DL_EDIT) 
    { 
      /* remove all translations if in edit mode */
      XtUninstallTranslations(dlElement->widget);
      
      /* add button press handlers too */
      XtAddEventHandler(dlElement->widget,ButtonPressMask, False,
			handleButtonPress,(XtPointer)displayInfo);
    }
  else 
    {
      /* add the callbacks for bring up menu */
      XtAddCallback(dlElement->widget, XmNarmCallback,
		    (XtCallbackProc) dmExecuteShellCommand,
		    (XtPointer) &(dlRelDisp->command[0]));
    }
  
  if (shellCommandPixmap == 0)
    XmStringFree(xmString);
  else
    XtAddCallback(dlElement->widget, XmNdestroyCallback,freePixmapCallback,
		  (XtPointer)shellCommandPixmap);
  
  XtManageChild(dlElement->widget);
}

static void executeDlShellCommand_Menu
  (DisplayInfo      * displayInfo,
   DlElement        * dlElement,
   DlShellCommand   * DlShell,
   int                iNumberOfCommands)
{
  Widget             widget;
  Widget             tearOff;
  Widget             shellCommandPulldownMenu;
  XmString           xmString = NULL;
  Arg                args[25];
  int                n;
  int                i;
  unsigned int       pixmapSize;
  Pixmap             shellCommandPixmap = 0;
  WidgetUserData   * userData;
  XmButtonType     * buttonTypes;
  XmString         * buttons;
  WidgetList         children;
  int                buttonIter;
  Pixel              background = displayInfo->colormap[DlShell->bclr];
  Pixel              foreground = displayInfo->colormap[DlShell->clr];
  Position           x = (Position)DlShell->object.x;
  Position           y = (Position)DlShell->object.y;
  Dimension          width = (Dimension)DlShell->object.width;
  Dimension          height = (Dimension)DlShell->object.height;

  /* MenuBar construction
   */
  n = 0;
  XtSetArg(args[n],XmNx,                  x); n++;
  XtSetArg(args[n],XmNy,                  y); n++;
  XtSetArg(args[n],XmNwidth,              width); n++;
  XtSetArg(args[n],XmNheight,             height); n++;
  XtSetArg(args[n],XmNbackground,         background); n++;
  XtSetArg(args[n],XmNforeground,         foreground); n++;
  XtSetArg(args[n],XmNhighlightThickness, 1); n++;
  XtSetArg(args[n],XmNmarginHeight,       0); n++;
  XtSetArg(args[n],XmNmarginWidth,        0); n++;
  XtSetArg(args[n],XmNspacing,            0); n++;
  XtSetArg(args[n],XmNhighlightOnEnter,   True); n++;
  XtSetArg(args[n],XmNresizeHeight,       False); n++;
  XtSetArg(args[n],XmNresizeWidth,        False); n++;

  dlElement->widget = XmCreateMenuBar(displayInfo->drawingArea,
				      "shellCommandMenuBar",
				      args,
				      n);
  
  colorMenuBar(dlElement->widget, foreground, background);


  /* Pulldown menu construction
   */
  buttonTypes = (XmButtonType *) 
    calloc (iNumberOfCommands, sizeof(XmButtonType));
  
  buttons = (XmString *) malloc (iNumberOfCommands * sizeof(XmString));
  
  if (buttonTypes == NULL || buttons == NULL) {
    if (buttonTypes) free((char*)buttonTypes);
    if (buttons)     free((char*)buttons);
    dm2kPrintf("Cannot allocate memory!!\n");
    return;
  }
  
  for (i = 0, buttonIter = 0; i < MAX_RELATED_DISPLAYS; i++)
  {
    if (DlShell->command[i].label != NULL &&
	STRLEN(DlShell->command[i].label) > 0) 
    {
      buttons[buttonIter]    = XmStringCreateSimple(DlShell->command[i].label);
      buttonTypes[buttonIter] = XmPUSHBUTTON;
      buttonIter++;
    }
  }

  n = 0;
  XtSetArg(args[n],XmNbackground,      background); n++;
  XtSetArg(args[n],XmNforeground,      foreground); n++;
  XtSetArg(args[n],XmNbuttonCount,     iNumberOfCommands); n++;
  XtSetArg(args[n],XmNbuttons,         buttons); n++;
  XtSetArg(args[n],XmNbuttonType,      buttonTypes); n++;
  XtSetArg(args[n],XmNpostFromButton,  0); n++;
  XtSetArg(args[n],XmNsimpleCallback,  dmExecuteShellCommand); n++;
#ifndef CONFIG_NO_TEAR_OFF
  XtSetArg(args[n],XmNtearOffModel,    XmTEAR_OFF_ENABLED); n++;
#else
  XtSetArg(args[n],XmNtearOffModel,    XmTEAR_OFF_DISABLED); n++;
#endif

  shellCommandPulldownMenu = 
    XmCreateSimplePulldownMenu (dlElement->widget,
				"shellCommandPulldownMenu",
				args, 
				n);

  for (i = 0; i < iNumberOfCommands; i++)
    if (buttons[i]) XmStringFree(buttons[i]);

  free((char*)buttons);
  free((char*)buttonTypes);
  
  /* alloc servise structure
   */
  userData = DM2KALLOC (WidgetUserData);
  if (userData == NULL) {
    dm2kPrintf("executeDlShellCommand: memory allocation error\n");
  } 
  else  {
    userData->privateData    = (char*) displayInfo;
    userData->updateTask = NULL;
    
    XtVaSetValues(shellCommandPulldownMenu, XmNuserData, userData,NULL);
    
    /* destroy callback should free allocated memory
     */
    XtAddCallback (shellCommandPulldownMenu, XmNdestroyCallback, 
		   freeUserDataCB, NULL);
  }

  /* set resource into Pushbutton of Pulldowm menu
   */
  XtVaGetValues(shellCommandPulldownMenu, 
		XmNchildren,     &children,
		NULL);

  for (i = 0, buttonIter = 0; i < MAX_RELATED_DISPLAYS; i++)
  {
    if (DlShell->command[i].label != NULL &&
	STRLEN(DlShell->command[i].label) > 0) 
    {
      XtVaSetValues(children[buttonIter], 
		    XmNforeground, foreground,
		    XmNbackground, background,
		    XmNuserData,  &(DlShell->command[buttonIter]),
		    NULL);
      buttonIter++;
    }
  }
  
  tearOff = XmGetTearOffControl(shellCommandPulldownMenu);
  
  if (tearOff) 
    XtVaSetValues(tearOff, 
		  XmNforeground, foreground,
		  XmNbackground, background,
		  NULL);


  /* construction of very one Cascade button on Menubar
   */
  n = 0;
  XtSetArg(args[n],XmNwidth,              width); n++;
  XtSetArg(args[n],XmNheight,             height); n++;
  XtSetArg(args[n],XmNbackground,         background); n++;
  XtSetArg(args[n],XmNforeground,         foreground); n++;
  XtSetArg(args[n],XmNmarginTop,          0); n++;
  XtSetArg(args[n],XmNmarginBottom,       0); n++;
  XtSetArg(args[n],XmNmarginLeft,         0); n++;
  XtSetArg(args[n],XmNmarginRight,        0); n++;
  XtSetArg(args[n],XmNmarginWidth,        0); n++;
  XtSetArg(args[n],XmNmarginHeight,       0); n++;
  XtSetArg(args[n],XmNhighlightOnEnter,   0); n++;
  XtSetArg(args[n],XmNhighlightThickness, 0); n++;
  XtSetArg(args[n],XmNrecomputeSize,      False); n++;

  /* Pixmap
   */
  pixmapSize = MIN(width, height);
    
  /* allowing for shadows etc 
   */
  pixmapSize = (unsigned int) MAX(1,(int)pixmapSize - 8);
  
  /* create shellCommand icon (render to appropriate size) 
   */
  shellCommandPixmap = XCreatePixmap(display,
				       RootWindow(display,screenNum),
				       pixmapSize, pixmapSize,
				       XDefaultDepth(display,screenNum));
  renderShellCommandPixmap(display,shellCommandPixmap,
			     foreground, background,
			     pixmapSize, pixmapSize);
  
  XtSetArg(args[n],XmNlabelPixmap, shellCommandPixmap); n++;
  XtSetArg(args[n],XmNlabelType,   XmPIXMAP); n++;
  XtSetArg(args[n],XmNsubMenuId,   shellCommandPulldownMenu); n++;
  
  widget = XtCreateManagedWidget("shellCommandMenuLabel",
				 xmCascadeButtonGadgetClass,
				 dlElement->widget, 
				 args, 
				 n);

  if (shellCommandPixmap == 0)
    XmStringFree(xmString);
  else
    XtAddCallback(widget, XmNdestroyCallback,freePixmapCallback,
		  (XtPointer)shellCommandPixmap);

  XtManageChild(dlElement->widget);
}



static UpdateTask * executeMethod
   (DisplayInfo * displayInfo, 
    DlElement   * dlElement)
{
  DlShellCommand * DlShell = dlElement->structure.shellCommand;
  int                iNumberOfCommands = 0;
  int                i;

  if (dlElement->widget && displayInfo->traversalMode == DL_EDIT)
  {
    if (!dlElement->widget->core.being_destroyed)
      XtDestroyWidget(dlElement->widget);
    dlElement->widget = NULL;
  }

  /* count number of displays 
   */
  for (i = 0, iNumberOfCommands = 0; i < MAX_RELATED_DISPLAYS; i++) {
    if (DlShell->command[i].label != NULL &&
	STRLEN(DlShell->command[i].label) > 0) 
      iNumberOfCommands++;
  } 

  if (displayInfo->traversalMode == DL_EDIT || iNumberOfCommands == 1)
  {
    executeDlShellCommand_PushButton(displayInfo, dlElement, DlShell);
  } 
  else
    executeDlShellCommand_Menu (displayInfo, dlElement, DlShell,
				iNumberOfCommands);
  return NULL;
}

#if 0
void executeDlShellCommand(DisplayInfo *displayInfo, DlElement *dlElement)
{
  Widget localMenuBar;
  Arg args[16];
  int i, shellNumber=0;
  XmString xmString;
  Pixmap shellCommandPixmap;
  unsigned int pixmapSize;
  DlShellCommand *DlShellCommand = dlElement->structure.shellCommand;
/*
 * these are widget ids, but they are recorded in the otherChild widget list
 *   as well, for destruction when new shells are selected at the top level
 */
  Widget shellCommandPulldownMenu, shellCommandMenuButton;
  Widget widget;
  Widget tearOff;


/***
 *** from the DlShellCommand structure, we've got specifics
 *** (MDA)  create a pulldown menu with the following related shell menu
 ***   entries in it...  --  careful with the XtSetArgs here (special)
 ***/
  if (dlElement->widget && displayInfo->traversalMode == DL_EDIT) {
    DlObject *po = &(dlElement->structure.shellCommand->object);
    XtVaSetValues(dlElement->widget,
		  XmNx, (Position) po->x,
		  XmNy, (Position) po->y,
		  XmNwidth, (Dimension) po->width,
		  XmNheight, (Dimension) po->height,
		  NULL);
    return;
  }

  XtSetArg(args[0],XmNforeground,(Pixel)
        displayInfo->colormap[DlShellCommand->clr]);
  XtSetArg(args[1],XmNbackground,(Pixel)
        displayInfo->colormap[DlShellCommand->bclr]);
  XtSetArg(args[2],XmNhighlightThickness,1);
  XtSetArg(args[3],XmNwidth,DlShellCommand->object.width);
  XtSetArg(args[4],XmNheight,DlShellCommand->object.height);
  XtSetArg(args[5],XmNmarginHeight,0);
  XtSetArg(args[6],XmNmarginWidth,0);
  XtSetArg(args[7],XmNresizeHeight,(Boolean)FALSE);
  XtSetArg(args[8],XmNresizeWidth,(Boolean)FALSE);
  XtSetArg(args[9],XmNspacing,0);
  XtSetArg(args[10],XmNx,(Position)DlShellCommand->object.x);
  XtSetArg(args[11],XmNy,(Position)DlShellCommand->object.y);
  XtSetArg(args[12],XmNhighlightOnEnter,TRUE);
  
  dlElement->widget = localMenuBar =
     XmCreateMenuBar(displayInfo->drawingArea,"shellCommandMenuBar",args,13);

  XtManageChild(localMenuBar);

  colorMenuBar(localMenuBar,
	       (Pixel)displayInfo->colormap[DlShellCommand->clr],
	       (Pixel)displayInfo->colormap[DlShellCommand->bclr]);

  shellCommandPulldownMenu = 
    XmCreatePulldownMenu(localMenuBar, "shellCommandPulldownMenu",args,2);
  
  tearOff = XmGetTearOffControl(shellCommandPulldownMenu);
  
  if (tearOff) 
    XtVaSetValues(tearOff, 
		  XmNforeground, displayInfo->colormap[DlShellCommand->clr],
		  XmNbackground, displayInfo->colormap[DlShellCommand->bclr],
		  NULL);

  pixmapSize = MIN(DlShellCommand->object.width,DlShellCommand->object.height);

  /* allowing for shadows etc 
   */
  pixmapSize = (unsigned int) MAX(1,(int)pixmapSize - 8);

  /* create shellCommand icon (render to appropriate size) 
   */
  shellCommandPixmap = XCreatePixmap(display,RootWindow(display,screenNum),
				     pixmapSize,pixmapSize,
				     XDefaultDepth(display,screenNum));

  renderShellCommandPixmap(display,shellCommandPixmap,
			   displayInfo->colormap[DlShellCommand->clr],
			   displayInfo->colormap[DlShellCommand->bclr],
			   pixmapSize,pixmapSize);

  XtSetArg(args[7],XmNrecomputeSize,      False);
  XtSetArg(args[8],XmNlabelPixmap,        shellCommandPixmap);
  XtSetArg(args[9],XmNlabelType,          XmPIXMAP);
  XtSetArg(args[10],XmNsubMenuId,         shellCommandPulldownMenu);
  XtSetArg(args[11],XmNhighlightOnEnter,  True);

  widget = XtCreateManagedWidget("shellCommandMenuLabel",
				 xmCascadeButtonGadgetClass,
				 localMenuBar, args, 12);

  /* add destroy callback to free pixmap from pixmap cache 
   */
  XtAddCallback(widget, XmNdestroyCallback,freePixmapCallback,
		(XtPointer)shellCommandPixmap);

  for (i = 0; i < MAX_SHELL_COMMANDS; i++) {
    if (DlShellCommand->command[i].command != NULL &&
	STRLEN(DlShell->command[i].command) > 0 )
    {
      xmString = XmStringCreateSimple(CARE_PRINT(DlShellCommand->command[i].label));
      XtSetArg(args[3], XmNlabelString,xmString);
 
     /* set the displayInfo as the button's userData */
      XtSetArg(args[4], XmNuserData,(XtPointer)displayInfo);

      shellCommandMenuButton = 
	XtCreateManagedWidget("relatedButton", xmPushButtonWidgetClass,
			      shellCommandPulldownMenu, args, 5);

      if (globalDm2kReadOnly) 
	XtSetSensitive (shellCommandMenuButton, False);

      XtAddCallback(shellCommandMenuButton,XmNactivateCallback,
		    (XtCallbackProc)dmExecuteShellCommand,
		    (XtPointer)&(DlShellCommand->command[i]));
      XmStringFree(xmString);
    }
  }
  

  /* add event handlers to shellCommand... 
   */
  if (displayInfo->traversalMode == DL_EDIT) {
    /* remove all translations if in edit mode */
    controlHandler (displayInfo, dlElement);
  }

}

#endif

static void copyDlShellCommandEntry(
  DlShellCommandEntry * from,
  DlShellCommandEntry * to)
{
  renewString(&(to->label), from->label);
  renewString(&(to->command), from->command);
  renewString(&(to->args), from->args);
}


DlElement *createDlShellCommand(DlElement *p)
{
  DlShellCommand *dlShellCommand;
  DlElement *dlElement;
  int cmdNumber;

  dlShellCommand = DM2KALLOC(DlShellCommand);

  if (dlShellCommand == NULL) 
    return 0;

  if (p != NULL) {
    objectAttributeCopy(&(dlShellCommand->object), 
		    &(p->structure.shellCommand->object));

    for (cmdNumber = 0; cmdNumber < MAX_SHELL_COMMANDS; cmdNumber++) 
      {
	copyDlShellCommandEntry
	  (&(p->structure.shellCommand->command[cmdNumber]),
	   &(dlShellCommand->command[cmdNumber]));
      }

    dlShellCommand->clr  = p->structure.shellCommand->clr;
    dlShellCommand->bclr = p->structure.shellCommand->bclr;
  } 
  else {
    objectAttributeInit(&(dlShellCommand->object));

    for (cmdNumber = 0; cmdNumber < MAX_SHELL_COMMANDS; cmdNumber++) 
      {
	dlShellCommand->command[cmdNumber].label   =
	dlShellCommand->command[cmdNumber].command =
	dlShellCommand->command[cmdNumber].args    = NULL;
      }

    dlShellCommand->clr  = 14;
    dlShellCommand->bclr = 3;
  }


  dlElement = createDlElement(DL_ShellCommand,
			      (XtPointer) dlShellCommand,
			      &shellCommandDlDispatchTable);
  if (dlElement == NULL)
    destroyDlShellCommand(dlShellCommand);

  return(dlElement);
}

void parseShellCommandEntry(DisplayInfo *displayInfo,
                            DlShellCommandEntry *shellCommand)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"label")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	if (STRLEN(token) > (size_t)0)
	  renewString(&shellCommand->label,token);
      } 
      else if (STREQL(token,"name")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	if (STRLEN(token) > (size_t)0)
	  renewString(&shellCommand->command,token);
      }
      else if (STREQL(token,"args")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	if (STRLEN(token) > (size_t)0)
	  renewString(&shellCommand->args,token);
      }
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
}

DlElement *parseShellCommand(DisplayInfo *displayInfo)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;
  DlShellCommand *DlShellCommand = 0;
  DlElement *dlElement = createDlShellCommand(NULL);
  int cmdNumber;

  if (!dlElement) return 0;
  DlShellCommand = dlElement->structure.shellCommand;
  do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	  if (STREQL(token,"object")) {
	    parseObject(displayInfo,&(DlShellCommand->object));
	  } else if (!strncmp(token,"command",7)) {
/*
 * compare the first 7 characters to see if a command entry.
 *   if more than one digit is allowed for the command index, then change
 *   the following code to pick up all the digits (can't use atoi() unless
 *   we get a null-terminated string
 */
	    cmdNumber = MIN(
			    token[8] - '0', MAX_SHELL_COMMANDS - 1);
	    parseShellCommandEntry(displayInfo,
				   &(DlShellCommand->command[cmdNumber]) );
	  } else if (STREQL(token,"clr")) {
	    getToken(displayInfo,token);
	    getToken(displayInfo,token);
	    DlShellCommand->clr = atoi(token) % DL_MAX_COLORS;
	  } else if (STREQL(token,"bclr")) {
	    getToken(displayInfo,token);
	    getToken(displayInfo,token);
	    DlShellCommand->bclr = atoi(token) % DL_MAX_COLORS;
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

void writeDlShellCommandEntry(
  FILE *stream,
  DlShellCommandEntry *entry,
  int index,
  int level)
{
  char indent[256]; level=MIN(level,256-2);
 
  level = MIN(level,256-2);
  memset(indent,'\t',level);
  indent[level] = '\0';
 
  fprintf(stream,"\n%scommand[%d] {",indent,index);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
  if (entry->label != NULL)
    fprintf(stream,"\n%s\tlabel=\"%s\"",indent,entry->label);
  if (entry->command != NULL)
    fprintf(stream,"\n%s\tname=\"%s\"",indent,entry->command);
  if (entry->args != NULL)
    fprintf(stream,"\n%s\targs=\"%s\"",indent,entry->args);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    fprintf(stream,"\n%s\tlabel=\"%s\"",indent,CARE_PRINT(entry->label));
    fprintf(stream,"\n%s\tname=\"%s\"",indent,CARE_PRINT(entry->command));
    fprintf(stream,"\n%s\targs=\"%s\"",indent,CARE_PRINT(entry->args));
  }
#endif
  fprintf(stream,"\n%s}",indent);
}

void writeDlShellCommand(
  FILE *stream,
  DlElement *dlElement,
  int level)
{
  int i;
  char indent[256]; level=MIN(level,256-2);
  DlShellCommand *DlShellCommand = dlElement->structure.shellCommand;

  level=MIN(level,256-2);
  memset(indent,'\t',level);
  indent[level] = '\0';
 
  fprintf(stream,"\n%s\"shell command\" {",indent);
  writeDlObject(stream,&(DlShellCommand->object),level+1);

  for (i = 0; i < MAX_RELATED_DISPLAYS; i++) {
#ifdef SUPPORT_0201XX_FILE_FORMAT
    if (Dm2kUseNewFileFormat) {
#endif
      if ((DlShellCommand->command[i].label != NULL) ||
	  (DlShellCommand->command[i].command != NULL) ||
	  (DlShellCommand->command[i].args != NULL))
	writeDlShellCommandEntry(stream,&(DlShellCommand->command[i]),i,level+1);
#ifdef SUPPORT_0201XX_FILE_FORMAT
    } else {
      writeDlShellCommandEntry(stream,&(DlShellCommand->command[i]),i,level+1);
    }
#endif
  }

  fprintf(stream,"\n%s\tclr=%d",indent,DlShellCommand->clr);
  fprintf(stream,"\n%s\tbclr=%d",indent,DlShellCommand->bclr);
  fprintf(stream,"\n%s}",indent);
}

#ifdef __cplusplus
static void shellCommandCallback(
  Widget,
  XtPointer client_data,
  XtPointer cbs)
#else
static void shellCommandCallback(
  Widget w,
  XtPointer client_data,
  XtPointer cbs)
#endif

{
  char                         *command;
  DisplayInfo                  *displayInfo;
  char                          processedCommand[2*MAX_TOKEN_LENGTH];
  XmSelectionBoxCallbackStruct *call_data = (XmSelectionBoxCallbackStruct *) cbs;

  Widget realParent = (Widget)client_data;

  displayInfo = dmGetDisplayInfoFromWidget(realParent);

/* CANCEL */
  if (call_data->reason == XmCR_CANCEL) {
    XtUnmanageChild(displayInfo->shellCommandPromptD);
    return;
  }

/* OK */
  XmStringGetLtoR(call_data->value,XmSTRING_DEFAULT_CHARSET,&command);

  /* (MDA) NB: system() blocks! need to background (&) to not block */

  if (command != NULL) {
    performMacroSubstitutions(displayInfo,command,processedCommand,
          2*MAX_TOKEN_LENGTH);
    if (STRLEN(processedCommand) > (size_t) 0)
      system(processedCommand);

    XtFree(command);
  }

  XtUnmanageChild(displayInfo->shellCommandPromptD);
}

static Widget createShellCommandPromptD(
  Widget parent)
{
  Arg args[6];
  int n;
  XmString title;
  Widget prompt;

  title = XmStringCreateSimple("Command");
  n = 0;
  XtSetArg(args[n],XmNdialogTitle,title); n++;
  XtSetArg(args[n],XmNdialogStyle,XmDIALOG_FULL_APPLICATION_MODAL); n++;
  XtSetArg(args[n],XmNselectionLabelString,title); n++;
  XtSetArg(args[n],XmNautoUnmanage,False); n++;
 /* update global for selection box widget access */
  prompt = XmCreatePromptDialog(parent,
    "shellCommandPromptD",args,n);
  XmStringFree(title);

  XtAddCallback(prompt, XmNcancelCallback,shellCommandCallback,parent);
  XtAddCallback(prompt,XmNokCallback,shellCommandCallback,parent);
  return (prompt);
}

#ifdef __cplusplus
void dmExecuteShellCommand(
  Widget  w,
  DlShellCommandEntry *commandEntry,
  XmPushButtonCallbackStruct *)
#else
void dmExecuteShellCommand(
  Widget  w,
  DlShellCommandEntry *commandEntry,
  XmPushButtonCallbackStruct *call_data)
#endif
{
  char           * promptPosition;
  int              cmdLength, argsLength;
  int              shellCommandStringPosition;
  char             shellCommand[2*MAX_TOKEN_LENGTH];
  char             processedShellCommand[2*MAX_TOKEN_LENGTH];
  XmString         xmString;
  WidgetUserData * userData;


  XtVaGetValues(XtParent(w), XmNuserData, &userData, NULL);
  if (userData)
     XtVaGetValues(w, XmNuserData, (XtPointer)&commandEntry, NULL);
  else
     XtVaGetValues(w, XmNuserData, &userData, NULL);

  if (!(userData && userData->privateData && commandEntry))
    return;
 
  currentDisplayInfo = (DisplayInfo *) userData->privateData;
  
/*
 * this is really an ugly bit of code which will have to be cleaned
 *  up when the requirements are better defined!!
 */
  cmdLength = STRLEN(commandEntry->command);
  argsLength = STRLEN(commandEntry->args);
  promptPosition = NULL;
  shellCommandStringPosition = 0;
  shellCommand[0] = '\0';


  /* create shell command prompt dialog 
   */
  if (currentDisplayInfo->shellCommandPromptD == (Widget)NULL) {
    currentDisplayInfo->shellCommandPromptD = createShellCommandPromptD(
    currentDisplayInfo->shell);
  }


  /* command */
  if (cmdLength > 0) {
	/* T. Straumann: should always check for NULL ptrs! */
    strcpy(shellCommand,commandEntry->command ? commandEntry->command : "");
    shellCommandStringPosition = cmdLength;
    shellCommand[shellCommandStringPosition++] = ' ';
    shellCommand[shellCommandStringPosition] = '\0';

    /* also support ? as first char. in command field for arbitrary cmd. */
    if (commandEntry->command[0] == '?') {
      xmString = XmStringCreateSimple("");
      XtVaSetValues(currentDisplayInfo->shellCommandPromptD,
		    XmNtextString,xmString,
		    NULL);
      XmStringFree(xmString);
      XtManageChild(currentDisplayInfo->shellCommandPromptD);
      return;
    }
  }


  /* also have some command arguments, see if ? for prompted input */
  if (cmdLength > 0 && argsLength > 0) {

    promptPosition = strchr(commandEntry->args,SHELL_CMD_PROMPT_CHAR);

    if (promptPosition == NULL) {
      /* no  prompt character found */
	  /* T. Straumann: should always check for NULL ptrs! */
      strcpy(&(shellCommand[shellCommandStringPosition]),commandEntry->args ? commandEntry->args : "");
      /* (MDA) NB: system() blocks! need to background (&) to not block */
      performMacroSubstitutions(currentDisplayInfo,
				shellCommand,processedShellCommand,
				2*MAX_TOKEN_LENGTH);
      if (STRLEN(processedShellCommand) > (size_t) 0) 
	system(processedShellCommand);
      shellCommand[0] = '\0';
    } 
    else {
      /* a prompt character found - handle it by replacing with NULL 
       * and copying 
       */
	  /* T. Straumann: should always check for NULL ptrs! */
      strcpy(&(shellCommand[shellCommandStringPosition]),commandEntry->args ? commandEntry->args : "");
      promptPosition = strchr(shellCommand,SHELL_CMD_PROMPT_CHAR);
      if (promptPosition != NULL)
	*promptPosition = '\0';
      
      /* now popup the prompt dialog and wait for input */
      xmString = XmStringCreateSimple(shellCommand);
      XtVaSetValues(currentDisplayInfo->shellCommandPromptD,
		    XmNtextString,xmString,
		    NULL);
      XmStringFree(xmString);
      XtManageChild(currentDisplayInfo->shellCommandPromptD);
    }
  } 
  else if (cmdLength > 0 && argsLength == 0) {
    /* (MDA) NB: system() blocks! need to background (&) to not block */
    performMacroSubstitutions(currentDisplayInfo,
			      shellCommand,processedShellCommand,
			      2*MAX_TOKEN_LENGTH);
    if (STRLEN(processedShellCommand) > (size_t) 0)
      system(processedShellCommand);
    shellCommand[0] = '\0';
  }
}

static void shellCommandInheritValues(ResourceBundle *pRCB, DlElement *p) {
  DlShellCommand *DlShellCommand = p->structure.shellCommand;
  dm2kGetValues(pRCB,
    CLR_RC,        &(DlShellCommand->clr),
    BCLR_RC,       &(DlShellCommand->bclr),
    -1);
}

static void shellCommandGetValues(ResourceBundle *pRCB, DlElement *p) {
  DlShellCommand *DlShellCommand = p->structure.shellCommand;
  dm2kGetValues(pRCB,
    X_RC,          &(DlShellCommand->object.x),
    Y_RC,          &(DlShellCommand->object.y),
    WIDTH_RC,      &(DlShellCommand->object.width),
    HEIGHT_RC,     &(DlShellCommand->object.height),
    CLR_RC,        &(DlShellCommand->clr),
    BCLR_RC,       &(DlShellCommand->bclr),
    SHELLDATA_RC,  &(DlShellCommand->command),
    -1);
}


static void dm2kShellCommandInfoSimple (
   char           * msg,
   DlShellCommand * DlShellCommand)
{
  int i, j;
  DlShellCommandEntry *command;

  strcat (msg, " ");
  for ( i = j = 0 ; i < MAX_SHELL_COMMANDS ; i++ ) {
    command = DlShellCommand->command;

    if ( command->command == NULL ) 
      continue;

    if ( j == 0 )
      sprintf (&msg[STRLEN(msg)], "'%s : %s'",
	       CARE_PRINT(command->label), command->command);

    j++;
  }

  if ( j > 1 )
    strcat (msg, " ...");
}


static void shellCommandInfo (char *msg, Widget w,
		      DisplayInfo *displayInfo,
		      DlElement *dlElement,
		      XtPointer objet)
{
  int i;
  DlShellCommand *DlShellCommand;
  DlShellCommandEntry *command;

  if (globalDisplayListTraversalMode != DL_EXECUTE) {
    dm2kShellCommandInfoSimple (msg, dlElement->structure.shellCommand);
    return;
  }

  DlShellCommand = dlElement->structure.shellCommand;

  strcat (msg, "\n\n  List of Shell Command");

  for ( i = 0 ; i < MAX_SHELL_COMMANDS ; i++ ) {
    command = DlShellCommand->command;

    if ( command->command == NULL )
      continue;

    sprintf (&msg[STRLEN(msg)], "\n    %s : %s", 
	     CARE_PRINT(command->label), command->command);
  }
}
