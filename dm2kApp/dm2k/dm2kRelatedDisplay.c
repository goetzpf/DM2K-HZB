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

#include "dm2k.h"
#include <X11/IntrinsicP.h>
#include <Xm/MwmUtil.h>

#define RD_APPLY_BTN  0
#define RD_CLOSE_BTN  1
 
static Widget rdMatrix = NULL, rdForm = NULL;
static Widget table[MAX_RELATED_DISPLAYS][4];
static Pixmap stipple = 0;

void relatedDisplayCreateNewDisplay(DisplayInfo *displayInfo,
                                    DlRelatedDisplayEntry *pEntry);

static void relatedDisplayInheritValues(ResourceBundle *, DlElement *);
static void relatedDisplayGetValues(ResourceBundle *, DlElement *);
static void relatedDisplayButtonPressedCb(Widget, XtPointer, XtPointer);
static void pulldownMenuButtonPressedCb(Widget ,XtPointer ,XtPointer );
static void relatedDisplayInfo (char *, Widget, DisplayInfo *, DlElement *, XtPointer);

static void destroyDlElement (DlElement * element);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);

static DlDispatchTable relatedDisplayDlDispatchTable = {
         createDlRelatedDisplay,
	 destroyDlElement,
         executeMethod,
         writeDlRelatedDisplay,
         NULL,
         relatedDisplayGetValues,
         relatedDisplayInheritValues,
         NULL,
         NULL,
         genericMove,
         genericScale,
	 NULL,
	 relatedDisplayInfo
	 };

static const char * unlabelItem = "new display";


static void destroyDlRelatedDisplay (DlRelatedDisplay * dlRelatedDisplay)
{
  int displayNumber;

  if (dlRelatedDisplay == NULL)
    return;

  for (displayNumber = 0; 
       displayNumber < MAX_RELATED_DISPLAYS;
       displayNumber++) 
  {
    DM2KFREE(dlRelatedDisplay->display[displayNumber].label);
    DM2KFREE(dlRelatedDisplay->display[displayNumber].name);
    DM2KFREE(dlRelatedDisplay->display[displayNumber].args);
  }

  DM2KFREE(dlRelatedDisplay->label);
  
  objectAttributeDestroy(&(dlRelatedDisplay->object));

  free((char*)dlRelatedDisplay);
}

static void destroyDlElement (DlElement * element)
{
  if (element == NULL || element->type != DL_RelatedDisplay) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  destroyDlRelatedDisplay (element->structure.relatedDisplay);
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
	/* T. Straumann: The pixmap was _not_ created by motif and hence
	 *				 is not in the cache. Just destroy it.
	 *				 Furthermore: the widget might be a gadget
	 *				 --> use XtDisplayOfObject()
    XmDestroyPixmap(XtScreen(w),pixmap);
	 */
	XFreePixmap(XtDisplayOfObject(w),pixmap);
}

/*
 * local function to render the related display icon into a pixmap
 */
static void renderRelatedDisplayPixmap(Display *display, Pixmap pixmap,
        Pixel fg, Pixel bg, Dimension width, Dimension height)
{
  typedef struct { float x; float y;} XY;
/* icon is based on the 25 pixel (w & h) bitmap relatedDisplay25 */
  static float rectangleX = 4./25., rectangleY = 4./25.,
        rectangleWidth = 13./25., rectangleHeight = 14./25.;
  static XY segmentData[] = {
        {16./25.,9./25.},
        {22./25.,9./25.},
        {22./25.,22./25.},
        {10./25.,22./25.},
        {10./25.,18./25.},
  };
  GC gc;
  XSegment segments[4];

  gc = XCreateGC(display,pixmap,0,NULL);
  XSetForeground(display,gc,bg);
  XFillRectangle(display,pixmap,gc,0,0,width,height);
  XSetForeground(display,gc,fg);

  segments[0].x1 = (short)(segmentData[0].x*width);
  segments[0].y1 = (short)(segmentData[0].y*height);
  segments[0].x2 = (short)(segmentData[1].x*width);
  segments[0].y2 = (short)(segmentData[1].y*height);

  segments[1].x1 = (short)(segmentData[1].x*width);
  segments[1].y1 = (short)(segmentData[1].y*height);
  segments[1].x2 = (short)(segmentData[2].x*width);
  segments[1].y2 = (short)(segmentData[2].y*height);

  segments[2].x1 = (short)(segmentData[2].x*width);
  segments[2].y1 = (short)(segmentData[2].y*height);
  segments[2].x2 = (short)(segmentData[3].x*width);
  segments[2].y2 = (short)(segmentData[3].y*height);

  segments[3].x1 = (short)(segmentData[3].x*width);
  segments[3].y1 = (short)(segmentData[3].y*height);
  segments[3].x2 = (short)(segmentData[4].x*width);
  segments[3].y2 = (short)(segmentData[4].y*height);

  XDrawSegments(display,pixmap,gc,segments,4);

/* erase any out-of-bounds edges due to roundoff error by blanking out
 *  area of top rectangle */
  XSetForeground(display,gc,bg);
  XFillRectangle(display,pixmap,gc,
        (int)(rectangleX*width),
        (int)(rectangleY*height),
        (unsigned int)(rectangleWidth*width),
        (unsigned int)(rectangleHeight*height));

/* and draw the top rectangle */
  XSetForeground(display,gc,fg);
  XDrawRectangle(display,pixmap,gc,
        (int)(rectangleX*width),
        (int)(rectangleY*height),
        (unsigned int)(rectangleWidth*width),
        (unsigned int)(rectangleHeight*height));

  XFreeGC(display,gc);
}


#ifdef __cplusplus
int relatedDisplayFontListIndex(
  DlRelatedDisplay *dlRelatedDisplay,
  int numButtons,
  int)
#else
int relatedDisplayFontListIndex(
  DlRelatedDisplay *dlRelatedDisplay,
  int numButtons,
  int maxChars)
#endif
{
  int i;
 
#define SHADOWS_SIZE 4    /* each Toggle Button has 2 shadows...*/

/* more complicated calculation based on orientation, etc */
  for (i = MAX_FONTS-1; i >=  0; i--) {
    switch (dlRelatedDisplay->visual) {
    case RD_COL_OF_BTN:
      if ( (int)(dlRelatedDisplay->object.height/MAX(1,numButtons)
          - SHADOWS_SIZE) >=
       (fontTable[i]->ascent + fontTable[i]->descent))
      return(i);
      break;
    case RD_ROW_OF_BTN:
      if ( (int)(dlRelatedDisplay->object.height - SHADOWS_SIZE) >=
       (fontTable[i]->ascent + fontTable[i]->descent))
      return(i);
      break;
    default:
       break;
    }
  }
  return (0);
}

static void executeDlRelatedDisplay_PushButton
  (DisplayInfo      * displayInfo,
   DlElement        * dlElement,
   DlRelatedDisplay * dlRelDisp)
{
  Arg                args[15];
  int                n;
  XmString           xmString = NULL;
  WidgetUserData   * userData;
  int                fontIndex;
  char             * label;
  Pixel              background = displayInfo->colormap[dlRelDisp->bclr];
  Pixel              foreground = displayInfo->colormap[dlRelDisp->clr];
  Pixmap             relatedDisplayPixmap = 0;


  fontIndex = messageButtonFontListIndex (dlRelDisp->object.height);

  if (dlRelDisp->label) 
    label = dlRelDisp->label;
  else 
    label = dlRelDisp->display[0].label;

  n = 0;
  /* geometric resources
   */
  XtSetArg(args[n],XmNx,      (Position)dlRelDisp->object.x); n++;
  XtSetArg(args[n],XmNy,      (Position)dlRelDisp->object.y); n++;
  XtSetArg(args[n],XmNwidth,  (Dimension)dlRelDisp->object.width); n++;
  XtSetArg(args[n],XmNheight, (Dimension)dlRelDisp->object.height); n++;
  
  /* specific resources
   */
  if (XTextWidth(fontTable[fontIndex], label, STRLEN(label)) <
      dlRelDisp->object.width - 4) 
    {
      xmString = XmStringCreateSimple(label);
      XtSetArg(args[n],XmNlabelString,        xmString); n++;
      XtSetArg(args[n],XmNlabelType,          XmSTRING); n++;
    }
  else
    {
      unsigned int pixmapSize;

      pixmapSize = MIN(dlRelDisp->object.width, dlRelDisp->object.height);
      
      /* allowing for shadows etc 
       */
      pixmapSize = (unsigned int) MAX(1,(int)pixmapSize - 8);
      
      /* create relatedDisplay icon (render to appropriate size) 
       */
      relatedDisplayPixmap = 
	XCreatePixmap(display,
		      RootWindow(display,screenNum),
		      pixmapSize, pixmapSize,
		      XDefaultDepth(display,screenNum));
      
      renderRelatedDisplayPixmap(display,relatedDisplayPixmap,
				 foreground, background,
				 pixmapSize, pixmapSize);
      
      XtSetArg(args[n],XmNlabelPixmap, relatedDisplayPixmap); n++;
      XtSetArg(args[n],XmNlabelType,   XmPIXMAP); n++;
    }
  
  XtSetArg(args[n],XmNhighlightThickness, 0); n++;
  XtSetArg(args[n],XmNhighlightOnEnter,   True); n++;
  XtSetArg(args[n],XmNindicatorOn,        False); n++;
  XtSetArg(args[n],XmNrecomputeSize,      False); n++;
  
  /* fonts
   */
  XtSetArg(args[n],XmNfontList, fontListTable[fontIndex]); n++;
  
  /* colors
   */
  XtSetArg(args[n],XmNforeground, foreground); n++;
  XtSetArg(args[n],XmNbackground, background); n++;

  /* create push button widget
   */
  dlElement->widget = XtCreateWidget("relatedDisplayPushButton",
				     xmPushButtonWidgetClass, 
				     displayInfo->drawingArea, 
				     args,
				     n);
  
  /* alloc servise structure
   */
  userData = DM2KALLOC (WidgetUserData);

  if (userData == NULL) {
    dm2kPrintf("executeDlRelatedDisplay: memory allocation error\n");
  } else {
    userData->privateData    = (char*) displayInfo;
    userData->updateTask = NULL;

    XtVaSetValues(dlElement->widget, XmNuserData, userData, NULL);

    /* destroy callback should free allocated memory
     */
    XtAddCallback (dlElement->widget, XmNdestroyCallback, 
		   freeUserDataCB, NULL);
  }

  /* mode dependent
   */
  if (displayInfo->traversalMode == DL_EDIT) 
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
		    relatedDisplayButtonPressedCb,
		    (XtPointer) &(dlRelDisp->display[0]));
    }
  
  if (relatedDisplayPixmap == 0)
    XmStringFree(xmString);
  else
    XtAddCallback(dlElement->widget, XmNdestroyCallback,freePixmapCallback,
		  (XtPointer)relatedDisplayPixmap);

  XtManageChild(dlElement->widget);
}


static void executeDlRelatedDisplay_Menu
  (DisplayInfo      * displayInfo,
   DlElement        * dlElement,
   DlRelatedDisplay * dlRelDisp,
   int                iNumberOfDisplays)
{
  Widget             widget;
  Widget             tearOff;
  Widget             relatedDisplayPulldownMenu;
  XmString           xmString;
  Arg                args[25];
  int                n;
  int                i;
  unsigned int       pixmapSize;
  Pixmap             relatedDisplayPixmap = 0;
  WidgetUserData   * userData;
  XmButtonType     * buttonTypes;
  XmString         * buttons;
  WidgetList         children;
  int                buttonIter;
  Pixel              background = displayInfo->colormap[dlRelDisp->bclr];
  Pixel              foreground = displayInfo->colormap[dlRelDisp->clr];
  Position           x = (Position)dlRelDisp->object.x;
  Position           y = (Position)dlRelDisp->object.y;
  Dimension          width = (Dimension)dlRelDisp->object.width;
  Dimension          height = (Dimension)dlRelDisp->object.height;
  XmFontList         fontList;

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
				      "relatedDisplayMenuBar",
				      args,
				      n);
  
  colorMenuBar(dlElement->widget, foreground, background);


  /* Pulldown menu construction
   */
  buttonTypes = (XmButtonType *) 
    calloc (iNumberOfDisplays, sizeof(XmButtonType));
  
  buttons = (XmString *) malloc (iNumberOfDisplays * sizeof(XmString));
  
  if (buttonTypes == NULL || buttons == NULL) {
    if (buttonTypes) free((char*)buttonTypes);
    if (buttons)     free((char*)buttons);
    dm2kPrintf("Cannot allocate memory!!\n");
    return;
  }
  
  for (i = 0, buttonIter = 0; i < MAX_RELATED_DISPLAYS; i++)
  {
    if (dlRelDisp->display[i].name != NULL &&
	STRLEN(dlRelDisp->display[i].name) > 0) 
    {
      const char * label;

      if (dlRelDisp->display[i].label != NULL)
	label = dlRelDisp->display[i].label;
      else
	label = unlabelItem;

      buttons[buttonIter]     = XmStringCreateSimple((char*)label);
      buttonTypes[buttonIter] = XmPUSHBUTTON;
      buttonIter++;
    }
  }

  n = 0;
  XtSetArg(args[n],XmNbackground,      background); n++;
  XtSetArg(args[n],XmNforeground,      foreground); n++;
  XtSetArg(args[n],XmNbuttonCount,     iNumberOfDisplays); n++;
  XtSetArg(args[n],XmNbuttons,         buttons); n++;
  XtSetArg(args[n],XmNbuttonType,      buttonTypes); n++;
  XtSetArg(args[n],XmNpostFromButton,  0); n++;
  XtSetArg(args[n],XmNsimpleCallback,  pulldownMenuButtonPressedCb); n++;
#ifndef CONFIG_NO_TEAR_OFF
  XtSetArg(args[n],XmNtearOffModel,    XmTEAR_OFF_ENABLED); n++;
#else
  XtSetArg(args[n],XmNtearOffModel,    XmTEAR_OFF_DISABLED); n++;
#endif

  relatedDisplayPulldownMenu = 
    XmCreateSimplePulldownMenu (dlElement->widget,
				"relatedDisplayPulldownMenu",
				args, 
				n);

  for (i = 0; i < iNumberOfDisplays; i++)
    if (buttons[i]) XmStringFree(buttons[i]);

  free((char*)buttons);
  free((char*)buttonTypes);
  
  /* alloc servise structure
   */
  userData = DM2KALLOC (WidgetUserData);
  if (userData == NULL) {
    dm2kPrintf("executeDlRelatedDisplay: memory allocation error\n");
  } 
  else  {
    userData->privateData    = (char*) displayInfo;
    userData->updateTask = NULL;
    
    XtVaSetValues(relatedDisplayPulldownMenu, XmNuserData, userData,NULL);
    
    /* destroy callback should free allocated memory
     */
    XtAddCallback (relatedDisplayPulldownMenu, XmNdestroyCallback, 
		   freeUserDataCB, NULL);
  }

  /* set resource into Pushbutton of Pulldowm menu
   */
  XtVaGetValues(relatedDisplayPulldownMenu, 
		XmNchildren,     &children,
		NULL);

  for (i = 0, buttonIter = 0; i < MAX_RELATED_DISPLAYS; i++)
  {
    if (dlRelDisp->display[i].name != NULL &&
	STRLEN(dlRelDisp->display[i].name) > 0) 
    {
      XtVaSetValues(children[buttonIter], 
		    XmNforeground, foreground,
		    XmNbackground, background,
		    XmNuserData,  &(dlRelDisp->display[i]),
		    NULL);
      buttonIter++;
    }
  }
  
  tearOff = XmGetTearOffControl(relatedDisplayPulldownMenu);
  
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

  /* label or pixmap
   */
  if (dlRelDisp->label) 
  {
    fontList = fontListTable[messageButtonFontListIndex
			    (dlRelDisp->object.height)];
    xmString = XmStringCreateSimple(dlRelDisp->label);

    XtSetArg(args[n],XmNlabelType,   XmSTRING); n++;
    XtSetArg(args[n],XmNlabelString, xmString); n++;
    XtSetArg(args[n],XmNfontList,    fontList); n++;
    XtSetArg(args[n],XmNalignment,   XmALIGNMENT_BEGINNING); n++;
  } 
  else 
  {
    pixmapSize = MIN(width, height);
    
    /* allowing for shadows etc 
     */
    pixmapSize = (unsigned int) MAX(1,(int)pixmapSize - 8);
    
    /* create relatedDisplay icon (render to appropriate size) 
     */
    relatedDisplayPixmap = XCreatePixmap(display,
					 RootWindow(display,screenNum),
					 pixmapSize, pixmapSize,
					 XDefaultDepth(display,screenNum));
    renderRelatedDisplayPixmap(display,relatedDisplayPixmap,
			       foreground, background,
			       pixmapSize, pixmapSize);
    
    XtSetArg(args[n],XmNlabelPixmap, relatedDisplayPixmap); n++;
    XtSetArg(args[n],XmNlabelType,   XmPIXMAP); n++;
  }
  
  XtSetArg(args[n],XmNsubMenuId,   relatedDisplayPulldownMenu); n++;
  
  widget = XtCreateManagedWidget("relatedDisplayMenuLabel",
				 xmCascadeButtonGadgetClass,
				 dlElement->widget, 
				 args, 
				 n);

  if (relatedDisplayPixmap == 0)
    XmStringFree(xmString);
  else
    XtAddCallback(widget, XmNdestroyCallback,freePixmapCallback,
		  (XtPointer)relatedDisplayPixmap);

  XtManageChild(dlElement->widget);
}


static void executeDlRelatedDisplay_RowOrColumn
  (DisplayInfo      * displayInfo,
   DlElement        * dlElement,
   DlRelatedDisplay * dlRelDisp,
   int                iNumberOfDisplays)
{
  Arg              wargs[20];
  int              i, n, saved_n, maxChars, usedWidth, usedHeight;
  XmFontList       fontList;
  Pixel            fg, bg;
  Widget           widget;
  WidgetUserData * userData;
  
  maxChars = 0;
  for (i = 0; i < MAX_RELATED_DISPLAYS; i++) {
    maxChars = MAX((size_t) maxChars, STRLEN(dlRelDisp->display[i].label));
  }
  
  fg = displayInfo->colormap[dlRelDisp->clr];
  bg = displayInfo->colormap[dlRelDisp->bclr];
  
  n = 0;
  XtSetArg(wargs[n],XmNx,      (Position)dlRelDisp->object.x); n++;
  XtSetArg(wargs[n],XmNy,      (Position)dlRelDisp->object.y); n++;
  XtSetArg(wargs[n],XmNwidth,  (Dimension)dlRelDisp->object.width); n++;
  XtSetArg(wargs[n],XmNheight, (Dimension)dlRelDisp->object.height); n++;

  XtSetArg(wargs[n],XmNforeground,    fg); n++;
  XtSetArg(wargs[n],XmNbackground,    bg); n++;
  XtSetArg(wargs[n],XmNindicatorOn,   (Boolean)FALSE); n++;
  XtSetArg(wargs[n],XmNmarginWidth,   0); n++;
  XtSetArg(wargs[n],XmNmarginHeight,  0); n++;
  XtSetArg(wargs[n],XmNresizeWidth,   (Boolean)FALSE); n++;
  XtSetArg(wargs[n],XmNresizeHeight,  (Boolean)FALSE); n++;
  XtSetArg(wargs[n],XmNspacing,       0); n++;
  XtSetArg(wargs[n],XmNrecomputeSize, (Boolean)FALSE); n++;
  XtSetArg(wargs[n],XmNshadowThickness,   0); n++;
/*
  XtSetArg(wargs[n],XmNthickness,   0); n++;
  XtSetArg(wargs[n],XmNhighlightThickness,   0); n++;
  XtSetArg(wargs[n],XmNborderWidth,   0); n++;
*/
  switch (dlRelDisp->visual) 
    {
    case RD_COL_OF_BTN:
      XtSetArg(wargs[n],XmNorientation,XmVERTICAL); n++;
      usedWidth = dlRelDisp->object.width;
      usedHeight = (int) (dlRelDisp->object.height/
			  MAX(1,iNumberOfDisplays));
      break;
      
    case RD_ROW_OF_BTN:
      XtSetArg(wargs[n],XmNorientation,XmHORIZONTAL); n++;
      usedWidth = (int) (dlRelDisp->object.width/
			 MAX(1,iNumberOfDisplays));
      usedHeight = dlRelDisp->object.height;
      break;
      
    default:
      break;
    }
  
  dlElement->widget = widget = 
    XmCreateRowColumn(displayInfo->drawingArea,"radioBox",wargs,n);

  /* alloc servise structure
   */
  userData = DM2KALLOC (WidgetUserData);
  if (userData == NULL) {
    dm2kPrintf("executeDlRelatedDisplay: memory allocation error\n");
  } 
  else  {
    userData->privateData = (char*) displayInfo;
    userData->updateTask  = NULL;
    
    XtVaSetValues(dlElement->widget, XmNuserData, userData,NULL);
    
    /* destroy callback should free allocated memory
     */
    XtAddCallback (dlElement->widget, XmNdestroyCallback, 
		   freeUserDataCB, NULL);
  }
 
  /* now make push-in type radio buttons of the correct size 
   */
  fontList = fontListTable[relatedDisplayFontListIndex
			  (dlRelDisp,iNumberOfDisplays,maxChars)];
  
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

  saved_n = n;
  for (i = 0; i < iNumberOfDisplays; i++) 
  {
    XmString xmStr;
    Widget   toggleButton;
    const char   * label;
    
    n = saved_n;
    if (dlRelDisp->display[i].label != NULL)
      label = dlRelDisp->display[i].label;
    else
      label = unlabelItem;

    xmStr = XmStringCreateSimple((char*)label);
    XtSetArg(wargs[n],XmNlabelString,xmStr); n++;

    /* use gadgets here so that changing foreground of
     * radioBox changes buttons */
    toggleButton = XmCreatePushButtonGadget(widget,"toggleButton",
					    wargs, n);

    XmStringFree(xmStr);

    /* alloc servise structure
     */
    userData = DM2KALLOC (WidgetUserData);
    if (userData == NULL) {
      dm2kPrintf("executeDlRelatedDisplay: memory allocation error\n");
    } 
    else  {
      userData->privateData    = (char*) displayInfo;
      userData->updateTask = NULL;
      
      XtVaSetValues(toggleButton, XmNuserData, userData,NULL);
      
      /* destroy callback should free allocated memory
       */
      XtAddCallback (toggleButton, XmNdestroyCallback, 
		     freeUserDataCB, NULL);
    }
 
    XtAddCallback(toggleButton,XmNarmCallback,
		  relatedDisplayButtonPressedCb,
		  (XtPointer) &(dlRelDisp->display[i]));
    
    /* MDA - for some reason, need to do this after the 
     * fact for gadgets..
     */
    XtVaSetValues(toggleButton,XmNalignment,XmALIGNMENT_CENTER,NULL);
    XtManageChild(toggleButton);
  }
  
  /* add in drag/drop translations 
   */
  XtOverrideTranslations(widget,parsedTranslations);

  XtManageChild(widget);
} 
  
static void executeDlRelatedDisplay_HiddenButton
  (DisplayInfo      * displayInfo,
   DlElement        * dlElement,
   DlRelatedDisplay * dlRelDisp)
{
  unsigned long   gcValueMask;
  XGCValues       gcValues;
  Display       * display = XtDisplay(displayInfo->drawingArea);
  
  
  gcValueMask = GCForeground | GCBackground | GCFillStyle | GCStipple;
  
  gcValues.foreground = displayInfo->colormap[dlRelDisp->clr];
  gcValues.background = displayInfo->colormap[dlRelDisp->bclr];
  gcValues.fill_style = FillStippled;

  if (!stipple) {
    static char stipple_bitmap[] = {0x03, 0x03, 0x0c, 0x0c};
    
    stipple = XCreateBitmapFromData(display,
				    RootWindow(display, 
					       DefaultScreen(display)),
				    stipple_bitmap, 4, 4);
  }
  
  gcValues.stipple = stipple;
  XChangeGC(XtDisplay(displayInfo->drawingArea),
	    displayInfo->gc,
	    gcValueMask, &gcValues);
  
  XFillRectangle(XtDisplay(displayInfo->drawingArea),
		 XtWindow(displayInfo->drawingArea),displayInfo->gc,
		 dlRelDisp->object.x,dlRelDisp->object.y,
		 dlRelDisp->object.width,dlRelDisp->object.height);
  
  XFillRectangle(XtDisplay(displayInfo->drawingArea),
		 displayInfo->drawingAreaPixmap,displayInfo->gc,
		 dlRelDisp->object.x,dlRelDisp->object.y,
		 dlRelDisp->object.width,dlRelDisp->object.height);
}


static UpdateTask * executeMethod
   (DisplayInfo * displayInfo, 
    DlElement   * dlElement)
{
  DlRelatedDisplay * dlRelDisp = dlElement->structure.relatedDisplay;
  int                iNumberOfDisplays = 0;
  int                i;

  if (dlElement->widget && displayInfo->traversalMode == DL_EDIT)
  {
    /* this is necessary for moving 
     */
    if (!dlElement->widget->core.being_destroyed)
      XtDestroyWidget(dlElement->widget);
    dlElement->widget = NULL;
  }

  /* count number of displays 
   */
  for (i = 0, iNumberOfDisplays = 0; i < MAX_RELATED_DISPLAYS; i++) {
    if (dlRelDisp->display[i].name != NULL &&
	STRLEN(dlRelDisp->display[i].name) > 0) 
      iNumberOfDisplays++;
  } 

  /* if mode is EDIT, use PushButton's face
   */
  if (displayInfo->traversalMode == DL_EDIT ||
      (iNumberOfDisplays == 1 && dlRelDisp->visual != RD_HIDDEN_BTN) )
  {
    executeDlRelatedDisplay_PushButton(displayInfo, dlElement, dlRelDisp);
  } 
  else if (dlRelDisp->visual == RD_MENU) 
  {
    executeDlRelatedDisplay_Menu (displayInfo, dlElement, dlRelDisp,
				  iNumberOfDisplays);
  } 
  else if (dlRelDisp->visual == RD_ROW_OF_BTN || 
	   dlRelDisp->visual == RD_COL_OF_BTN) 
  {
    executeDlRelatedDisplay_RowOrColumn(displayInfo, dlElement, dlRelDisp,
					iNumberOfDisplays);
  }
  else if (dlRelDisp->visual == RD_HIDDEN_BTN) 
  {
    executeDlRelatedDisplay_HiddenButton(displayInfo, dlElement, dlRelDisp);
  }

  return NULL;
}


DlElement *createDlRelatedDisplay(DlElement *p)
{
  DlRelatedDisplay *dlRelatedDisplay;
  DlElement *dlElement;
  int displayNumber;

  dlRelatedDisplay = DM2KALLOC(DlRelatedDisplay);

  if (dlRelatedDisplay == NULL) 
    return 0;

  if (p != NULL) {
    objectAttributeCopy(&(dlRelatedDisplay->object), 
			&(p->structure.relatedDisplay->object));

    for (displayNumber = 0; 
	 displayNumber < MAX_RELATED_DISPLAYS;
         displayNumber++) 
    {
      renewString(& dlRelatedDisplay->display[displayNumber].label,
		  p->structure.relatedDisplay->display[displayNumber].label);
      
      renewString(& dlRelatedDisplay->display[displayNumber].name,
		  p->structure.relatedDisplay->display[displayNumber].name);
      
      renewString(& dlRelatedDisplay->display[displayNumber].args,
		  p->structure.relatedDisplay->display[displayNumber].args);
      
      dlRelatedDisplay->display[displayNumber].mode  =
	p->structure.relatedDisplay->display[displayNumber].mode;
    }
    
    dlRelatedDisplay->clr    = p->structure.relatedDisplay->clr;
    dlRelatedDisplay->bclr   = p->structure.relatedDisplay->bclr;
    dlRelatedDisplay->visual = p->structure.relatedDisplay->visual;
    renewString(&dlRelatedDisplay->label,p->structure.relatedDisplay->label);
  } 
  else {
    objectAttributeInit(&(dlRelatedDisplay->object));
    
    for (displayNumber = 0; 
	 displayNumber < MAX_RELATED_DISPLAYS;
         displayNumber++) 
    {
      dlRelatedDisplay->display[displayNumber].label = 
      dlRelatedDisplay->display[displayNumber].name  = 
      dlRelatedDisplay->display[displayNumber].args  = NULL;
      dlRelatedDisplay->display[displayNumber].mode  = FIRST_RD_MODE;
    }

    dlRelatedDisplay->clr    = globalResourceBundle.clr;
    dlRelatedDisplay->bclr   = globalResourceBundle.bclr;
    dlRelatedDisplay->label  = NULL;
    dlRelatedDisplay->visual = RD_MENU;
  }
  
  dlElement = createDlElement(DL_RelatedDisplay,
			      (XtPointer)      dlRelatedDisplay,
			      &relatedDisplayDlDispatchTable);
  if (dlElement == NULL)
    free((char*)dlRelatedDisplay);
  
  return(dlElement);
}

void parseRelatedDisplayEntry(DisplayInfo           * displayInfo, 
			      DlRelatedDisplayEntry * relatedDisplay)
{
  char  token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int   nestingLevel = 0;

  do {
    switch( (tokenType = getToken(displayInfo,token)) ) 
    {
    case T_WORD:
      if (STREQL(token,"label")) {
        getToken(displayInfo,token);
        getToken(displayInfo,token);
	if (STRLEN(token) > (size_t)0)
	  renewString(&relatedDisplay->label,token);
      } 
      else if (STREQL(token,"name")) 
      {
        getToken(displayInfo,token);
        getToken(displayInfo,token);
	if (STRLEN(token) > (size_t)0)
	  renewString(&relatedDisplay->name,token);
      } 
      else if (STREQL(token,"args"))
      {
        getToken(displayInfo,token);
        getToken(displayInfo,token);
	if (STRLEN(token) > (size_t)0)
	  renewString(&relatedDisplay->args,token);
      }
      else if (STREQL(token,"policy")) 
      {
        getToken(displayInfo,token);
        getToken(displayInfo,token);
        if (STREQL(token,stringValueTable[REPLACE_DISPLAY]))
          relatedDisplay->mode = REPLACE_DISPLAY;
	else if (STREQL(token,stringValueTable[ADD_NEW_DISPLAY]))
	  relatedDisplay->mode = ADD_NEW_DISPLAY;
	else {
	  dm2kPrintf("Unknown related display replace policy..\n");
	  relatedDisplay->mode = ADD_NEW_DISPLAY;
	}
      }
      break;

    case T_LEFT_BRACE:
      nestingLevel++; 
      break;

    case T_RIGHT_BRACE:
      nestingLevel--; 
      break;
    default:
       break;
    }
  } while (   (tokenType != T_RIGHT_BRACE) 
	   && (nestingLevel > 0)
	   && (tokenType != T_EOF) );
}

DlElement *parseRelatedDisplay(DisplayInfo *displayInfo)
{
  char               token[MAX_TOKEN_LENGTH];
  TOKEN              tokenType;
  int                nestingLevel = 0;
  DlRelatedDisplay * dlRelatedDisplay = 0;
  DlElement        * dlElement = createDlRelatedDisplay(NULL);
  int                displayNumber, idx;

  if (!dlElement)
    return 0;

  dlRelatedDisplay = dlElement->structure.relatedDisplay;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) 
    {
    case T_WORD:
      if (STREQL(token,"object")) {
        parseObject(displayInfo,&(dlRelatedDisplay->object));
      } 
      else if (!strncmp(token,"display",7)) 
      {
	/*
	 * compare the first 7 characters to see if a display entry.
	 *   if more than one digit is allowed for the display index, 
	 *   then change the following code to pick up all the digits 
	 *   (can't use atoi() unless we get a null-terminated string
	 */
	 displayNumber = 0;
	 idx = 8;
	 while (isdigit(token[idx])) displayNumber = 10*displayNumber + token[idx++] - '0';
	 displayNumber = MIN(displayNumber, MAX_RELATED_DISPLAYS-1);
	 parseRelatedDisplayEntry(displayInfo,
				  &(dlRelatedDisplay->display[displayNumber]) );
      } 
      else if (STREQL(token,"clr")) 
      {
        getToken(displayInfo,token);
        getToken(displayInfo,token);
        dlRelatedDisplay->clr = atoi(token) % DL_MAX_COLORS;
      }
      else if (STREQL(token,"bclr")) 
      {
        getToken(displayInfo,token);
        getToken(displayInfo,token);
        dlRelatedDisplay->bclr = atoi(token) % DL_MAX_COLORS;
      }
      else if (STREQL(token,"label")) 
      {
        getToken(displayInfo,token);
        getToken(displayInfo,token);
        renewString(&dlRelatedDisplay->label,token);
      }
      else if (STREQL(token,"visual")) 
      {
        getToken(displayInfo,token);
        getToken(displayInfo,token);

        if (STREQL(token,stringValueTable[FIRST_RD_VISUAL+1])) {
          dlRelatedDisplay->visual = RD_ROW_OF_BTN;
        } 
	else if (STREQL(token,stringValueTable[FIRST_RD_VISUAL+2])) {
          dlRelatedDisplay->visual = RD_COL_OF_BTN;
        } 
	else if (STREQL(token,stringValueTable[FIRST_RD_VISUAL+3])) {
          dlRelatedDisplay->visual = RD_HIDDEN_BTN;
        }
      }
      break;

    case T_EQUAL:
      break;

    case T_LEFT_BRACE:
      nestingLevel++; 
      break;

    case T_RIGHT_BRACE:
      nestingLevel--; 
      break;
    default:
       break;
    }
  } while (   (tokenType != T_RIGHT_BRACE) 
	   && (nestingLevel > 0)
	   && (tokenType != T_EOF) );

  return dlElement;

}

void writeDlRelatedDisplayEntry(
  FILE *stream,
  DlRelatedDisplayEntry *entry,
  int index,
  int level)
{
  char indent[256]; level=MIN(level,256-2);

  if ((!entry->label || !entry->label[0]) &&
      (!entry->name || !entry->name[0]) &&
      (!entry->args || !entry->args[0]))
     return;
  
  level = MIN(level,256-2);
  memset(indent,'\t',level);
  indent[level] = '\0';

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
    fprintf(stream,"\n%sdisplay[%d] {",indent,index);

    if (entry->label && entry->label[0])
      fprintf(stream,"\n%s\tlabel=\"%s\"",indent,entry->label);

    if (entry->name && entry->name[0])
      fprintf(stream,"\n%s\tname=\"%s\"",indent,entry->name);

    if (entry->args && entry->args[0])
      fprintf(stream,"\n%s\targs=\"%s\"",indent,entry->args);

    if (entry->mode != ADD_NEW_DISPLAY)
      fprintf(stream,"\n%s\tpolicy=\"%s\"",
              indent,stringValueTable[entry->mode]);

    fprintf(stream,"\n%s}",indent);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    fprintf(stream,"\n%sdisplay[%d] {",indent,index);

    fprintf(stream,"\n%s\tlabel=\"%s\"",indent,
	    entry->label ? entry->label : "");

    fprintf(stream,"\n%s\tname=\"%s\"",indent,
	    entry->name ? entry->name : "");

    fprintf(stream,"\n%s\targs=\"%s\"",
	    indent,entry->args ? entry->args : "");

    fprintf(stream,"\n%s}",indent);
  }
#endif
}

void writeDlRelatedDisplay(
  FILE *stream,
  DlElement *dlElement,
  int level)
{
  int i;
  char indent[356];
  DlRelatedDisplay *dlRelatedDisplay = dlElement->structure.relatedDisplay;

  level = MIN(level,256-2);
  memset(indent,'\t',level);
  indent[level] = '\0';

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif

    fprintf(stream,"\n%s\"related display\" {",indent);
    writeDlObject(stream,&(dlRelatedDisplay->object),level+1);

    for (i = 0; i < MAX_RELATED_DISPLAYS; i++) {
       writeDlRelatedDisplayEntry(stream,&(dlRelatedDisplay->display[i]),i,level+1);
    }

    fprintf(stream,"\n%s\tclr=%d",indent,dlRelatedDisplay->clr);
    fprintf(stream,"\n%s\tbclr=%d",indent,dlRelatedDisplay->bclr);

    if (dlRelatedDisplay->label != NULL) 
      fprintf(stream,"\n%s\tlabel=\"%s\"",indent,dlRelatedDisplay->label);

    if (dlRelatedDisplay->visual != RD_MENU)
      fprintf(stream,"\n%s\tvisual=\"%s\"", 
	      indent,stringValueTable[dlRelatedDisplay->visual]);

    fprintf(stream,"\n%s}",indent);

#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    fprintf(stream,"\n%s\"related display\" {",indent);
    writeDlObject(stream,&(dlRelatedDisplay->object),level+1);
    for (i = 0; i < MAX_RELATED_DISPLAYS; i++)
      writeDlRelatedDisplayEntry(stream,&(dlRelatedDisplay->display[i]),i,level+1);
    fprintf(stream,"\n%s\tclr=%d",indent,dlRelatedDisplay->clr);
    fprintf(stream,"\n%s\tbclr=%d",indent,dlRelatedDisplay->bclr);
    fprintf(stream,"\n%s}",indent);
  }
#endif
}

static void relatedDisplayInheritValues(ResourceBundle *pRCB, DlElement *p) 
{
  DlRelatedDisplay *dlRelatedDisplay = p->structure.relatedDisplay;
  dm2kGetValues(pRCB,
		CLR_RC,        &(dlRelatedDisplay->clr),
		BCLR_RC,       &(dlRelatedDisplay->bclr),
		-1);
}

static void relatedDisplayGetValues(ResourceBundle *pRCB, DlElement *p) 
{
  DlRelatedDisplay *dlRelatedDisplay = p->structure.relatedDisplay;
  dm2kGetValues(pRCB,
		X_RC,          &(dlRelatedDisplay->object.x),
		Y_RC,          &(dlRelatedDisplay->object.y),
		WIDTH_RC,      &(dlRelatedDisplay->object.width),
		HEIGHT_RC,     &(dlRelatedDisplay->object.height),
		CLR_RC,        &(dlRelatedDisplay->clr),
		BCLR_RC,       &(dlRelatedDisplay->bclr),
		RD_LABEL_RC,   &(dlRelatedDisplay->label),
		RD_VISUAL_RC,  &(dlRelatedDisplay->visual),
		RDDATA_RC,     &(dlRelatedDisplay->display),
		-1);
}

static void relatedDisplayButtonPressedCb
    (Widget    w,
     XtPointer clientData,
     XtPointer callbackData) 
{
  DlRelatedDisplayEntry * pEntry = (DlRelatedDisplayEntry *) clientData;
  DisplayInfo           * displayInfo;
  WidgetUserData        * userData;

  XtVaGetValues(w, XmNuserData, &userData, NULL);

  if (userData && userData->privateData) {
    displayInfo = (DisplayInfo *) userData->privateData;
    relatedDisplayCreateNewDisplay(displayInfo, pEntry);
  } 
}

static void pulldownMenuButtonPressedCb(Widget w,
					XtPointer clientData,
					XtPointer callbackData) 
{
  DlRelatedDisplayEntry * pEntry;
  DisplayInfo           * displayInfo;
  WidgetUserData        * userData;

  XtVaGetValues(XtParent(w), XmNuserData, &userData, NULL);
  XtVaGetValues(w, XmNuserData, (XtPointer)&pEntry, NULL);

  if (userData && userData->privateData && pEntry) {
    displayInfo = (DisplayInfo *) userData->privateData;
    relatedDisplayCreateNewDisplay(displayInfo, pEntry);
  } 
}

void relatedDisplayCreateNewDisplay(DisplayInfo *displayInfo,
                                    DlRelatedDisplayEntry *pEntry) 
{ 
  char *filename, *argsString, *newFilename, token[MAX_TOKEN_LENGTH];
  FILE *filePtr;
  char *adlPtr;
  char processedArgs[2*MAX_TOKEN_LENGTH];
  int suffixLength, prefixLength;
  filename = pEntry->name;
  argsString = pEntry->args;

/*
 * if we want to be able to have RD's inherit their parent's
 *   macro-substitutions, then we must perform any macro substitution on
 *   this argument string in this displayInfo's context before passing
 *   it to the created child display
 */
  if (globalDisplayListTraversalMode == DL_EXECUTE) 
    {
      performMacroSubstitutions(displayInfo,argsString,processedArgs,
				2*MAX_TOKEN_LENGTH);
      if (strstr(filename,DISPLAY_FILE_FACEPLATE_SUFFIX)) {
	 openFaceplate( filename );
      } else {
	 filePtr = dmOpenUseableFile(filename);
	 
	 if (filePtr == NULL) 
	 {
	    newFilename = STRDUP(filename);
	    adlPtr = strstr(filename,DISPLAY_FILE_ASCII_SUFFIX);
	    
	    if (adlPtr != NULL) /* ascii name */
	       suffixLength = STRLEN(DISPLAY_FILE_ASCII_SUFFIX);
	    else               /* binary name */
	       suffixLength = STRLEN(DISPLAY_FILE_BINARY_SUFFIX);
	    
	    prefixLength = STRLEN(newFilename) - suffixLength;
	    newFilename[prefixLength] = '\0';
	    sprintf(token,
		    "Can't open related display:\n\n        %s%s\n\n%s",
		    newFilename, DISPLAY_FILE_ASCII_SUFFIX,
		    "--> check EPICS_DISPLAY_PATH ");
	    dmSetAndPopupWarningDialog(displayInfo,token,"Ok",NULL,NULL);
	    fprintf(stderr,"\n%s",token);
	    free(newFilename);
	 } 
	 else 
	 {
	    if (pEntry->mode == REPLACE_DISPLAY) {
               char fncopy[strlen(filename)+1];
               strcpy(fncopy,filename);
	       dmDisplayListParse(displayInfo,filePtr,processedArgs,
				  fncopy,NULL,(Boolean)True);
	       fclose(filePtr);
	    } else {
	       dmDisplayListParse(NULL,filePtr,processedArgs,filename,NULL,
				  (Boolean)True);
	       fclose(filePtr);
	    }
	 }
      }
    }
}

#ifdef __cplusplus
static void relatedDisplayActivate(Widget w, XtPointer cd, XtPointer) {
#else
static void relatedDisplayActivate(Widget w, XtPointer cd, XtPointer cbs) {
#endif
  int buttonType = (int) cd;
  int i;

  switch (buttonType) {
 
    case RD_APPLY_BTN:
      /* commit changes in matrix to global matrix array data */

      for (i = 0; i < MAX_RELATED_DISPLAYS; i++) {
        char *tmp = NULL;
        if ((tmp = XmTextFieldGetString(table[i][0]))) {
          renewString(&globalResourceBundle.rdData[i].label, tmp);
          XtFree(tmp);
        }

        if ((tmp = XmTextFieldGetString(table[i][1]))) {
          renewString(&globalResourceBundle.rdData[i].name, tmp);
          XtFree(tmp);
        }

        if ((tmp = XmTextFieldGetString(table[i][2]))) {
          renewString(&globalResourceBundle.rdData[i].args, tmp);
          XtFree(tmp);
        }

        if (XmToggleButtonGetState(table[i][3])) {
          globalResourceBundle.rdData[i].mode = REPLACE_DISPLAY;
        } else {
          globalResourceBundle.rdData[i].mode = ADD_NEW_DISPLAY;
        }
      }
      if (currentDisplayInfo) {
        DlElement *dlElement =
                   FirstDlElement(currentDisplayInfo->selectedDlElementList);
        while (dlElement) {
          if (dlElement->structure.element->type == DL_RelatedDisplay) {
            updateElementFromGlobalResourceBundle(
                dlElement->structure.element);
          }
          dlElement = dlElement->next;
        }
      }

      if (currentDisplayInfo->hasBeenEditedButNotSaved == False)
        dm2kMarkDisplayBeingEdited(currentDisplayInfo);

      /* break; */
 
    case RD_CLOSE_BTN:
      if (XtClass(w) == xmPushButtonWidgetClass) {
	XtPopdown(relatedDisplayS);
      }
      break;
  }
}
 
/*
 * create related display data dialog
 */
Widget createRelatedDisplayDataDialog (Widget parent) 
{
  Widget    shell, applyButton, closeButton, sw;
  Dimension cWidth, cHeight, aWidth, aHeight;
  Arg       args[12];
  int       i, n;
 
  /*
   * now create the interface
   *
   *         label | name | args | mode
   *         --------------------------
   *      1 |  A      B      C      D
   *      2 |
   *      3 |
   *         ...
   *     OK     CANCEL
   */
 
  n = 0;
  XtSetArg(args[n],XmNautoUnmanage, False); n++;
  XtSetArg(args[n],XmNmarginHeight, 8); n++;
  XtSetArg(args[n],XmNmarginWidth,  8); n++;
  rdForm = XmCreateFormDialog(parent,"relatedDisplayDataF",args,n);

  shell = XtParent(rdForm);

  XtVaSetValues(shell,
		XmNtitle,"Related Display Data",
		XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_RESIZEH,
		NULL);
 
  XmAddWMProtocolCallback(shell,WM_DELETE_WINDOW,
			  relatedDisplayActivate,(XtPointer)RD_CLOSE_BTN);

  n = 0;
  XtSetArg(args[n],XmNscrollingPolicy,          XmAUTOMATIC); n++;
  XtSetArg(args[n],XmNscrollBarDisplayPolicy,   XmAS_NEEDED); n++;
  XtSetArg(args[n],XmNwidth,   750); n++;
  XtSetArg(args[n],XmNheight,   600); n++;
  sw = XmCreateScrolledWindow(rdForm, "scrollRdMatrix", args, n);

  rdMatrix = XtVaCreateManagedWidget("rdMatrix", 
				     xmRowColumnWidgetClass,sw,
				     XmNpacking,     XmPACK_COLUMN,
				     XmNorientation, XmHORIZONTAL,
				     XmNnumColumns,  MAX_RELATED_DISPLAYS + 1,
				     NULL);

  /* create column label 
   */
  XtVaCreateManagedWidget("Display Label",
			  xmLabelWidgetClass, rdMatrix,
			  XmNalignment, XmALIGNMENT_CENTER,
			  NULL);

  XtVaCreateManagedWidget("Display File",
			  xmLabelWidgetClass, rdMatrix,
			  XmNalignment, XmALIGNMENT_CENTER,
			  NULL);

  XtVaCreateManagedWidget("Arguments",
			  xmLabelWidgetClass, rdMatrix,
			  XmNalignment, XmALIGNMENT_CENTER,
			  NULL);

  XtVaCreateManagedWidget("Policy",
			  xmLabelWidgetClass, rdMatrix,
			  XmNalignment, XmALIGNMENT_CENTER,
			  NULL);

  for (i=0; i<MAX_RELATED_DISPLAYS; i++) 
  {
    table[i][0] = XtVaCreateManagedWidget("label", xmTextFieldWidgetClass,
					  rdMatrix, NULL);

    table[i][1] = XtVaCreateManagedWidget("display", xmTextFieldWidgetClass,
					  rdMatrix, NULL);

    table[i][2] = XtVaCreateManagedWidget("arguments", xmTextFieldWidgetClass,
					  rdMatrix, NULL);

    table[i][3] = XtVaCreateManagedWidget("Remove Parent Display",
					  xmToggleButtonWidgetClass, rdMatrix,
					  XmNshadowThickness, 0,
					  NULL);
  }

  closeButton = XtVaCreateWidget("Close", xmPushButtonWidgetClass, 
				 rdForm, NULL);

  XtAddCallback(closeButton,XmNactivateCallback,
		relatedDisplayActivate,(XtPointer)RD_CLOSE_BTN);

  XtManageChild(closeButton);

  applyButton = XtVaCreateWidget("Apply", xmPushButtonWidgetClass, 
				 rdForm, NULL); 

  XtAddCallback(applyButton,XmNactivateCallback,
		relatedDisplayActivate,(XtPointer)RD_APPLY_BTN);
  XtManageChild(applyButton);
 

  /* make APPLY and CLOSE buttons same size 
   */
  XtVaGetValues(closeButton,
		XmNwidth,  &cWidth,
		XmNheight, &cHeight,
		NULL);

  XtVaGetValues(applyButton,
		XmNwidth,  &aWidth,
		XmNheight, &aHeight,
		NULL);

  XtVaSetValues(closeButton,
		XmNwidth , MAX(cWidth,aWidth),
		XmNheight, MAX(cHeight,aHeight),
		NULL);
  
  /* now do form layout
   */
  
  /* rdMatrix */
  n = 0;
  XtSetArg(args[n],XmNtopAttachment,   XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNleftAttachment,  XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetValues(sw,args,n);

  /* apply */
  n = 0;
/*
  XtSetArg(args[n],XmNtopAttachment,   XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNtopWidget,       sw); n++;
  XtSetArg(args[n],XmNtopOffset,       12); n++;
*/
  XtSetArg(args[n],XmNleftAttachment,  XmATTACH_POSITION); n++;
  XtSetArg(args[n],XmNleftPosition,    30); n++;
  XtSetArg(args[n],XmNbottomAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNbottomOffset,    12); n++;
  XtSetValues(applyButton,args,n);

  /* close */
  n = 0;
/*
  XtSetArg(args[n],XmNtopAttachment,   XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNtopWidget,       sw); n++;
  XtSetArg(args[n],XmNtopOffset,       12); n++;
*/
  XtSetArg(args[n],XmNrightAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n],XmNrightPosition,   70); n++;
  XtSetArg(args[n],XmNbottomAttachment,XmATTACH_FORM); n++;
  XtSetArg(args[n],XmNbottomOffset,    12); n++;
  XtSetValues(closeButton,args,n);

  n = 0;
  XtSetArg(args[n],XmNbottomAttachment,   XmATTACH_WIDGET); n++;
  XtSetArg(args[n],XmNbottomWidget,       applyButton); n++;
  XtSetArg(args[n],XmNbottomOffset,       12); n++;
  XtSetValues(sw,args,n);
 
  XtManageChild(sw);
  XtManageChild(rdForm);

  return shell;
}

/*
 * access function (for file-scoped globals, etc) to udpate the
 *  related display data dialog with the values currently in
 *  globalResourceBundle
 */
void updateRelatedDisplayDataDialog()
{
  int i;

  if (rdMatrix) { 
    for (i = 0; i < MAX_RELATED_DISPLAYS; i++) 
    {
#define SET_TEXT(widget,string) \
     XmTextFieldSetString(widget,CARE_PRINT(string))      

      SET_TEXT(table[i][0],globalResourceBundle.rdData[i].label);
      SET_TEXT(table[i][1],globalResourceBundle.rdData[i].name);
      SET_TEXT(table[i][2],globalResourceBundle.rdData[i].args);

#undef SET_TEXT

      if (globalResourceBundle.rdData[i].mode == REPLACE_DISPLAY) {
        XmToggleButtonSetState(table[i][3],True,False);
      } else {
        XmToggleButtonSetState(table[i][3],False,False);
      }
    }
  }
}

void relatedDisplayDataDialogPopup(Widget w) 
{
  if (relatedDisplayS == NULL) {
    relatedDisplayS = createRelatedDisplayDataDialog(w);
  }

  /* update related display data from globalResourceBundle 
   */
  updateRelatedDisplayDataDialog();
  XtManageChild(rdForm);
  XtPopup(relatedDisplayS,XtGrabNone);
}


static void  dm2kRelatedDisplayInfoSimple (char *msg,
		      DlRelatedDisplay *dlRelatedDisplay)
{
  int i, j;
  DlRelatedDisplayEntry *display;

  strcat (msg, " ");
  for ( i = j = 0 ; i < MAX_RELATED_DISPLAYS ; i++ ) {
    display = &(dlRelatedDisplay->display[i]);

    if ( display->name == NULL ) 
      continue;

    if ( j == 0 )
      sprintf (&msg[STRLEN(msg)], "'%s : %s'", display->label, display->name);
    j++;
  }
  if ( j > 1 ) strcat (msg, " ...");
}


static void relatedDisplayInfo (char *msg, Widget w,
		      DisplayInfo *displayInfo,
		      DlElement *dlElement,
		      XtPointer objet)
{
  int i;
  DlRelatedDisplay *dlRelatedDisplay;
  DlRelatedDisplayEntry *display;

  if (globalDisplayListTraversalMode != DL_EXECUTE) {
    dm2kRelatedDisplayInfoSimple (msg, dlElement->structure.relatedDisplay);
    return;
  }

  dlRelatedDisplay = dlElement->structure.relatedDisplay;
  strcat (msg, "\n\n  List of Related Display");

  for ( i = 0 ; i < MAX_RELATED_DISPLAYS ; i++ ) {
    display = &(dlRelatedDisplay->display[i]);

    if ( display->name == NULL ) 
      continue;
    sprintf (&msg[STRLEN(msg)], "\n    %s : %s", 
	     CARE_PRINT(display->label), CARE_PRINT(display->name));
  }
}
