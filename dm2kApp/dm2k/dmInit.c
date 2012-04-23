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
 *                              - strip chart list is removed
 *                                from the displayInfo structure.
 *                              - a updateTaskList is added to the 
 *                                displayInfo structure.
 * .03  09-08-95        vong    conform to c++ syntax
 * .04  10-03-95        vong    call dmResizeDisplayList() before
 *                              dmTraverseDisplayList() instead of
 *                              using XResizeWindows()
 *                              
 *
 *****************************************************************************
 *
 *      24-02-97    Fabien  call createDisplayDrawingArea from display.c
 *                          DM2K crash on Linux X server when popup menu
 *                          are attached direct child of shell !
 *                          Create the Information dialog box to
 *                          display object information
 *
 *****************************************************************************
*/

#include "dm2k.h"

#include <X11/keysym.h>
#ifdef USE_XPM
# include <X11/xpm.h>
# include "adl_icon.xpm"
#endif

extern Widget mainShell;

#define DISPLAY_DEFAULT_X 10
#define DISPLAY_DEFAULT_Y 10

typedef DlElement *(*dm2kParseFunc)(DisplayInfo *);
typedef struct {
   char *name;
   dm2kParseFunc func;
} ParseFuncEntry;

typedef struct _parseFuncEntryNode {
   ParseFuncEntry *entry;
   struct _parseFuncEntryNode *next;
   struct _parseFuncEntryNode *prev;
} ParseFuncEntryNode;

ParseFuncEntry parseFuncTable[] = {
         {"rectangle",            parseRectangle},
         {"oval",                 parseOval},
         {"arc",                  parseArc},
         {"text",                 parseText},
         {"falling line",         parseFallingLine},
         {"rising line",          parseRisingLine},
         {"related display",      parseRelatedDisplay},
         {"shell command",        parseShellCommand},
         {"bar",                  parseBar},
         {"indicator",            parseIndicator},
         {"meter",                parseMeter},
         {"byte",                 parseByte},
         {"strip chart",          parseStripChart},
         {"cartesian plot",       parseCartesianPlot},
         {"text update",          parseTextUpdate},
         {"choice button",        parseChoiceButton},
         {"button",               parseChoiceButton},
         {"message button",       parseMessageButton},
         {"menu",                 parseMenu},
         {"text entry",           parseTextEntry},
         {"valuator",             parseValuator},
         {"image",                parseImage},
         {"composite",            parseComposite},
         {"polyline",             parsePolyline},
         {"polygon",              parsePolygon},
	 {"dynamic symbol",       parseDynSymbol},
};

static const int parseFuncTableSize = XtNumber(parseFuncTable);

static DlElement *getNextElement(DisplayInfo *pDI, char *token) 
{
  int i;
  for (i=0; i<parseFuncTableSize; i++) {
    if (STREQL(token,parseFuncTable[i].name)) {
      return parseFuncTable[i].func(pDI);
    }
  }
  return 0;
}

/*ARGSUSED*/
static void 
#ifdef __cplusplus
displayShellPopdownCallback(Widget , XtPointer cd, XtPointer)
#else
displayShellPopdownCallback(Widget shell, XtPointer cd, XtPointer cbs)
#endif
{
  positionDisplayRead ((DisplayInfo *) cd);
}


void keepAspectRatio (DisplayInfo *displayInfo)
{
  Dimension w, h;

  w = h = -1;
  if ( displayInfo->drawingArea )
     XtVaGetValues (displayInfo->drawingArea, XmNwidth, &w, XmNheight, &h, NULL);
  if ( w > 0 && h > 0 )
     XtVaSetValues(displayInfo->shell,
		   XtNminAspectX, w, XtNminAspectY, h, XtNmaxAspectX, w, XtNmaxAspectY, h,
		   NULL );
}


/*ARGSUSED*/
static void 
#ifdef __cplusplus
displayShellPopupCallback(Widget shell, XtPointer cd, XtPointer)
#else
displayShellPopupCallback(Widget shell, XtPointer cd, XtPointer cbs)
#endif
{
  positionDisplayRead ((DisplayInfo *) cd);
/*  keepAspectRatio((DisplayInfo *) cd);*/
}

/***
 ***  displayInfo creation
 ***/

void createDisplayShell (DisplayInfo *displayInfo)
{
  extern Boolean motifWMRunning;
  extern Boolean debugFlag;
  Boolean displayDialog;
  Position x, y;
  int n;
  Arg args[20];

  if ( displayInfo->shell ) return;

  displayDialog = (displayInfo->displayType != NORMAL_DISPLAY);
  displayInfo->dialog = NULL;
  displayInfo->xWmDecoration = displayInfo->yWmDecoration = 0;
  /* this way seems to be efficient with both mwm and fwm Window managers */
  displayInfo->wmPositionIsFrame = motifWMRunning;
  displayInfo->selectedElementsAreHighlighted = False;

  x = displayInfo->xPosition;
  y = displayInfo->yPosition;
  getXYDisplay (displayInfo, &x, &y);

  /* create the shell and add callbacks */
  n = 0;
  XtSetArg(args[n],XmNiconName,"display"); n++;
  XtSetArg(args[n],XmNtitle,"display"); n++;
  XtSetArg(args[n],XmNallowShellResize,True); n++;

  /* for highlightOnEnter on pointer motion, this must be set for shells */
  XtSetArg(args[n],XmNkeyboardFocusPolicy,XmPOINTER); n++;

  /* map window manager menu Close function to application close... */
  XtSetArg(args[n],XmNdeleteResponse,XmDO_NOTHING); n++;
  if (privateDataCmap) {
    XtSetArg(args[n],XmNcolormap,cmap); n++;
  }

  XtSetArg(args[n],XmNx,x); n++;
  XtSetArg(args[n],XmNy,y); n++;
  XtSetArg(args[n],XmNwaitForWm,True); n++;

  if ( displayDialog ) {
        displayInfo->shell = 
      XmCreateDialogShell (mainShell, "displayShell", args, n);
  } else {
    displayInfo->shell = 
      XtCreatePopupShell ("display", topLevelShellWidgetClass,
			  mainShell, args, n);

  }

  XtAddCallback(displayInfo->shell,XmNpopupCallback,
			displayShellPopupCallback, (XtPointer) displayInfo);
  XtAddCallback(displayInfo->shell,XmNpopdownCallback,
			displayShellPopdownCallback, (XtPointer) displayInfo);

  /* register interest in these protocols */
  { Atom atoms[2];
    atoms[0] = WM_DELETE_WINDOW;
    atoms[1] = WM_TAKE_FOCUS;
    XmAddWMProtocols(displayInfo->shell,atoms,2);
  }

  /* and register the callbacks for these protocols */
  XmAddWMProtocolCallback(displayInfo->shell,WM_DELETE_WINDOW,
			(XtCallbackProc)wmCloseCallback,
			(XtPointer)DISPLAY_SHELL);
  XtAddEventHandler (displayInfo->shell, StructureNotifyMask, False,
		     handleShellStructureNotify, (XtPointer)displayInfo);


  if ( debugFlag ) {
    printf ("\n<== Created new shell %x for %x type %s\n    at x %d y %d\n",
	   (int)displayInfo->shell, (int)displayInfo, displayTypeMsg (displayInfo->displayType),
	   x, y);
   }

  /*
   * creation of the shell's EXECUTE and EDIT popup menus is done by the
   *     creation of the Drawing Area widget in display.c
   *     see : createPopupDisplayMenu in dm2k.c
   */

  /*
   * create the display objects information dialog box
   */
   n = 0;
  XtSetArg(args[n],XmNdialogStyle,XmDIALOG_APPLICATION_MODAL); n++;
  displayInfo->objectID = XmCreateInformationDialog (displayInfo->shell,
					    "informationDialog", args, n);
  XtUnmanageChild (XmMessageBoxGetChild (displayInfo->objectID, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (displayInfo->objectID, XmDIALOG_HELP_BUTTON));
  XtVaSetValues (displayInfo->objectID, XmNdialogType, XmDIALOG_MESSAGE, NULL);
}

DisplayInfo * createInitializedDisplayInfo()
{
  DisplayInfo * displayInfo;
  extern Boolean motifWMRunning;

  /* allocate a DisplayInfo structure 
   */
  displayInfo = DM2KALLOC(DisplayInfo);
  if (displayInfo == NULL) 
    return NULL;

  /* cleate display lists */
  displayInfo->dlElementList = createDlList();
  displayInfo->selectedDlElementList = createDlList();

  if (displayInfo->dlElementList == NULL 
      || displayInfo->selectedDlElementList == NULL) 
  {
    DM2KFREE(displayInfo->dlElementList);
    DM2KFREE(displayInfo->selectedDlElementList);
    DM2KFREE(displayInfo);
    return NULL;
  }

  /* initialise fields */
  displayInfo->selectedElementsAreHighlighted = False;
  displayInfo->traverseDisplayListFlag        = False;

  displayInfo->filePtr                = NULL;
  displayInfo->newDisplay             = True;
  displayInfo->versionNumber          = 0;

  displayInfo->drawingArea            = 0;
  displayInfo->drawingAreaPixmap      = 0;
  displayInfo->cartesianPlotPopupMenu = 0;
  displayInfo->selectedCartesianPlot  = 0;
  displayInfo->warningDialog          = NULL;
  displayInfo->warningDialogAnswer    = 0;
  displayInfo->questionDialog         = NULL;
  displayInfo->questionDialogAnswer   = 0;
  displayInfo->shellCommandPromptD    = NULL;
  displayInfo->dlDisplayElement       = NULL;

  displayInfo->colormap                   = 0;
  displayInfo->dlColormapCounter          = 0;
  displayInfo->dlColormapSize             = 0;
  displayInfo->drawingAreaBackgroundColor = globalResourceBundle.bclr;
  displayInfo->drawingAreaForegroundColor = globalResourceBundle.clr;
  displayInfo->gc                         = 0;
  displayInfo->pixmapGC                   = 0;

  displayInfo->traversalMode               = globalDisplayListTraversalMode;
  displayInfo->hasBeenEditedButNotSaved    = False;
  displayInfo->fromRelatedDisplayExecution = FALSE;

  displayInfo->nameValueTable = NULL;
  displayInfo->numNameValues  = 0;

  displayInfo->dlFile     = NULL;
  displayInfo->dlColormap = NULL;

  displayInfo->currentCursor = None;

  displayInfo->shell = NULL;
  displayInfo->dialog = NULL;

  displayInfo->displayType   = (DisplayType)-1;
  displayInfo->xPosition     = displayInfo->yPosition = 1;
  displayInfo->xWmDecoration = displayInfo->yWmDecoration = 0;

  /* this way seems to be efficient with both mwm and fwm Window managers 
   */
  displayInfo->wmPositionIsFrame = motifWMRunning;
  return displayInfo;
}

/*
 * create and return a DisplayInfo structure pointer on tail 
 * of displayInfoList  this includes a shell 
 * (with it's dialogs and event handlers)
 */
DisplayInfo *allocateDisplayInfo()
{
  DisplayInfo *displayInfo;

  displayInfo = createInitializedDisplayInfo();

  if (displayInfo == NULL) 
    return NULL;

  updateTaskInit(displayInfo);

  /* append to end of the list */
  displayInfo->next = NULL;
  displayInfo->prev = displayInfoListTail;
  
  if (displayInfoListHead == NULL) displayInfoListHead = displayInfo;
  if (displayInfoListTail) displayInfoListTail->next = displayInfo;

  displayInfoListTail = displayInfo;

  return(displayInfo);
}

void dumpDisplayInfoList() {
  DisplayInfo *dlPtr = displayInfoListHead;
  int i;
  FILE *f = fopen("/var/tmp/dm2k-calls","w");
  while (dlPtr) {
    fprintf(f,"dm2k");
    fprintf(f," -dg +%d+%d", dlPtr->xPosition, dlPtr->yPosition );
    if ( dlPtr->numNameValues > 0 ) {
      fprintf(f," -macro ");
      for ( i=0; i < dlPtr->numNameValues; i++) {
        if (i>0) fprintf(f,",");
        fprintf(f,"%s=%s", dlPtr->nameValueTable[i].name, dlPtr->nameValueTable[i].value );
      }
    }
    fprintf(f," -x %s\n", dlPtr->dlFile->name);
    dlPtr=dlPtr->next;
  }
  fclose(f);
}

TOKEN parseAndAppendDisplayList(DisplayInfo *displayInfo, DlList *dlList) 
{
  TOKEN tokenType;
  char token[MAX_TOKEN_LENGTH];
  int nestingLevel = 0;
  static DlBasicAttribute attr = {0,0,0,0};
  static DlDynamicAttribute dynAttr = {0,0,0,0};
  static Boolean init = True;
 
  if (init && displayInfo->versionNumber < DM2K_ADL_FILE_JUMP_VERSION) {
    basicAttributeInit(&attr);
    dynamicAttributeInit(&dynAttr);
    init = False;
  }
 
  do {
    switch (tokenType=getToken(displayInfo,token)) {
      case T_WORD : {
        DlElement *pe = 0;
        if ((pe = getNextElement(displayInfo,token))) {
          if (displayInfo->versionNumber < DM2K_ADL_FILE_JUMP_VERSION) {
            switch (pe->type) {
            case DL_Rectangle :
            case DL_Oval      :
            case DL_Arc       :
            case DL_Text      :
            case DL_Polyline  :
            case DL_Polygon   :
              basicAttributeCopy(&(pe->structure.rectangle->attr), &attr);
	      basicAttributeDestroy(&attr);
	      basicAttributeInit(&attr);

              if (dynAttr.chan != NULL) {
                dynamicAttributeCopy(&(pe->structure.rectangle->dynAttr),
				     &dynAttr);
		dynamicAttributeDestroy(&dynAttr);
		dynamicAttributeInit(&dynAttr);
              }
              break;
	    default:
	       break;
            }
          }
        } else
        if (displayInfo->versionNumber < DM2K_ADL_FILE_JUMP_VERSION) {
          if (STREQL(token,"<<basic atribute>>") ||
              STREQL(token,"basic attribute") ||
              STREQL(token,"<<basic attribute>>")) {
            parseOldBasicAttribute(displayInfo,&attr);
          } else
          if (STREQL(token,"dynamic attribute") ||
              STREQL(token,"<<dynamic attribute>>")) {
            parseOldDynamicAttribute(displayInfo,&dynAttr);
          }
        }
        if (pe) {
          appendDlElement(dlList,pe);
        }
      }
      case T_EQUAL:
        break;
      case T_LEFT_BRACE:
        nestingLevel++; break;
      case T_RIGHT_BRACE:
        nestingLevel--; break;
      default :
        break;
    }
  } while ((tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
           && (tokenType != T_EOF));

  /* reset the init flag */
  if (tokenType == T_EOF) 
    init = True;
  return tokenType;
}

/***
 ***  parsing and drawing/widget creation routines
 ***/

/*
 * routine which actually parses the display list in the opened file
 *  N.B.  this function must be kept in sync with parseCompositeChildren
 *  which follows...
 */
void dmDisplayListParse(
  DisplayInfo *displayInfo,
  FILE *filePtr,
  char *argsString,
  char *filename,
  char *geometryString,
  Boolean fromRelatedDisplayExecution)
{
  char            token[MAX_TOKEN_LENGTH];
  TOKEN           tokenType;
  int             numPairs;
  DisplayInfo    *identicalDisplayInfo;
  Pixmap          iconPixmap = (Pixmap)0;

  if (displayInfo == NULL) {
    /* allocate a DisplayInfo structure and shell for this display file/list
     */
    displayInfo = allocateDisplayInfo();
  } else {
     if (displayInfo->shell)
	XtVaSetValues(displayInfo->shell, 
		      XtNminAspectX, 0, XtNminAspectY, 0,
		      XtNmaxAspectX, 0, XtNmaxAspectY, 0,
		      NULL );
    dmCleanupDisplayInfo(displayInfo,False);
    destroyDlDisplayList(displayInfo->dlElementList);
    destroyDlDisplayList(displayInfo->selectedDlElementList);
  }
  
  displayInfo->filePtr = filePtr;
  currentDisplayInfo = displayInfo;
  currentDisplayInfo->newDisplay = False;

  enableControllerRC (currentDisplayInfo);

  
  displayInfo->fromRelatedDisplayExecution = fromRelatedDisplayExecution;

  /* generate the name-value table for 
   * macro substitutions (related display)
   */
  if (argsString) {
    displayInfo->nameValueTable = 
      generateNameValueTable(argsString,&numPairs);
    displayInfo->numNameValues = numPairs;
  } else {
    displayInfo->nameValueTable = NULL;
    displayInfo->numNameValues = 0;
  }


  /* if first token isn't "file" then bail out! */
  tokenType=getToken(displayInfo,token);
  if (tokenType == T_WORD && STREQL(token,"file")) {
    displayInfo->dlFile = parseFile(displayInfo);
    if (displayInfo->dlFile) {
      displayInfo->versionNumber = displayInfo->dlFile->versionNumber;
      renewString(&displayInfo->dlFile->name,filename);
    } else {
      fprintf(stderr,"\ndmDisplayListParse: out of memory!");
      displayInfo->filePtr = NULL;
      dmRemoveDisplayInfo(displayInfo);
      currentDisplayInfo = NULL;
      return;
    }
  } else {
    fprintf(stderr,"\ndmDisplayListParse: invalid .adl file %s (bad first token)", 
	    filename);
    displayInfo->filePtr = NULL;
    dmRemoveDisplayInfo(displayInfo);
    currentDisplayInfo = NULL;
    return;
  }

  /* let's check if such display already exists
   */
  identicalDisplayInfo = lookupIdenticalDisplayInfo(displayInfo);
    
  if (identicalDisplayInfo != NULL) 
    {
      /* pup up window
       */
      if (identicalDisplayInfo->shell) 
      {
	/* deiconify identic display's shell window and raise on top of stack
	 */
	XMapWindow (XtDisplay (identicalDisplayInfo->shell),
		    XtWindow (identicalDisplayInfo->shell));

	XRaiseWindow(XtDisplay (identicalDisplayInfo->shell),
		     XtWindow (identicalDisplayInfo->shell));
      }

      /* if EDIT mode to propose a question to user 
       */
      if (identicalDisplayInfo->traversalMode == DL_EDIT)
      {
	int button;

	button = getUserChoiseViaPopupQuestionDialog 
	  (identicalDisplayInfo->shell,
	   DM2K_DIALOG_MESSAGE_LABEL, 
	   "That display was already created.\n"
	   "Do You want to create new copy of that?",
	   DM2K_DIALOG_OK_BUTTON,     "Don't create",
	   DM2K_DIALOG_CANCEL_BUTTON, "Create new copy",
	   DM2K_DIALOG_TITLE,         "Identical Display",
	   NULL);

	if (button == 0) { /* don't create */
	  currentDisplayInfo = identicalDisplayInfo;
	  dmRemoveDisplayInfo(displayInfo);
	  return;
	}
      }
      else if (identicalDisplayInfo->traversalMode == DL_EXECUTE) {
	/* in EXECUTE mode, return w/o questions
	 */
	dmRemoveDisplayInfo(displayInfo);
	return; 
      }
    }

  tokenType=getToken(displayInfo,token);
  if (tokenType == T_WORD && STREQL(token,"display")) {
	  parseDisplay(displayInfo);
  }

  tokenType=getToken(displayInfo,token);
  if (tokenType == T_WORD &&
      (STREQL(token,"color map") || STREQL(token,"<<color map>>"))) {
     displayInfo->dlColormap=parseColormap(displayInfo,displayInfo->filePtr);
     if (!displayInfo->dlColormap) {
	/* error - do total bail out */
	fclose(displayInfo->filePtr);
	dmRemoveDisplayInfo(displayInfo);
	return;
     }
  }

  /*
   * proceed with parsing
   */
  while (parseAndAppendDisplayList(displayInfo,displayInfo->dlElementList)
         != T_EOF );

  displayInfo->filePtr = NULL;

/*
 * traverse (execute) this displayInfo and associated display list
 */
  {
    int x, y;
    unsigned int w, h;
    int mask;

    mask = XParseGeometry(geometryString,&x,&y,&w,&h);

    if ((mask & WidthValue) && (mask & HeightValue)) {
      dmResizeDisplayList(displayInfo,w,h);
    }

    dmTraverseDisplayList(displayInfo);

#ifdef USE_XPM
    if (iconPixmap==0)
      XpmCreatePixmapFromData (display,
			       XDefaultRootWindow(display),
			       adl_icon_xpm,
			       &iconPixmap, 
			       NULL,
			       NULL);
#endif

    if (! ( iconPixmap == (Pixmap) NULL 
	    || iconPixmap == XtUnspecifiedPixmap
	    || iconPixmap == None) )
      {
	XtVaSetValues(displayInfo->shell, XtNiconPixmap, iconPixmap, NULL);
      }
    
    XtPopup(displayInfo->shell,XtGrabNone);

    if ((mask & XValue) && (mask & YValue)) {
      XMoveWindow(XtDisplay(displayInfo->shell),XtWindow(displayInfo->shell),x,y);
    }
/*    keepAspectRatio(displayInfo);*/
  }
}

DisplayInfo * dmDisplayListParse2
  (DisplayInfo * displayInfoHead,
   DisplayInfo * displayInfo,
   FILE        * filePtr,
   char        * argsString,
   char        * filename)
{
  char            token[MAX_TOKEN_LENGTH];
  TOKEN           tokenType;
  int             numPairs;
  DisplayInfo    *identicalDisplayInfo;

  currentDisplayInfo = displayInfo;

  /* generate the name-value table for 
   * macro substitutions (related display)
   */
  if (argsString) {
    displayInfo->nameValueTable = generateNameValueTable(argsString,&numPairs);
    displayInfo->numNameValues = numPairs;
  } else {
    displayInfo->nameValueTable = NULL;
    displayInfo->numNameValues = 0;
  }


  /* if first token isn't "file" then bail out! 
   */
  tokenType = getToken(displayInfo,token);

  if (tokenType == T_WORD && STREQL(token,"file")) {
    displayInfo->dlFile = parseFile(displayInfo);

    if (displayInfo->dlFile) {
      displayInfo->versionNumber = displayInfo->dlFile->versionNumber;
      renewString(&displayInfo->dlFile->name, filename);
    } else {
      fprintf(stderr,"\ndmDisplayListParse: out of memory!");
      return NULL;
    }
  } 
  else {
    fprintf(stderr,"\ndmDisplayListParse: invalid .adl file(bad first token)");
    return NULL;
  }

  /* let's check if such display already exists,
   * if yes, return it as result;
   */
  identicalDisplayInfo = NULL;
  if (displayInfoHead != NULL)
    identicalDisplayInfo = 
      lookupIdenticalDisplayInfo2(displayInfoHead,displayInfo);
    
  if (identicalDisplayInfo != NULL) 
    return identicalDisplayInfo;


  tokenType = getToken(displayInfo,token);
  if (tokenType == T_WORD && STREQL(token,"display"))
    parseDisplay(displayInfo);


  tokenType = getToken(displayInfo,token);
  if (tokenType == T_WORD &&
      (STREQL(token, "color map") || STREQL(token, "<<color map>>"))) {
    displayInfo->dlColormap = parseColormap(displayInfo,displayInfo->filePtr);

    if (displayInfo->dlColormap == NULL) 
      return NULL;
  }
  /* proceed with parsing
   */
  while (parseAndAppendDisplayList(displayInfo, displayInfo->dlElementList)
         != T_EOF )
    /*EMPTY*/;
  
  displayInfo->filePtr = NULL;

  return displayInfo;
}

DlElement *parseDisplay(DisplayInfo *displayInfo)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;
  DlDisplay *dlDisplay;
  DlElement *dlElement = createDlDisplay(NULL);
 
  if (!dlElement) return 0;
  dlDisplay = dlElement->structure.display;
 
  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"object")) {
	parseObject(displayInfo,&(dlDisplay->object));
      } 
      else if (STREQL(token,"cmap")) {
	/* parse separate display list to get and use that colormap */
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	if (STRLEN(token) > (size_t) 0) {
	  renewString(&dlDisplay->cmap,token);
/*	  printf("cmap should be taken from (%p) '%s'\n", dlDisplay, dlDisplay->cmap);*/
	}
      }
      else if (STREQL(token,"bclr")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	dlDisplay->bclr = atoi(token) % DL_MAX_COLORS;
	displayInfo->drawingAreaBackgroundColor =
	  dlDisplay->bclr;
      }
      else if (STREQL(token,"clr")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	dlDisplay->clr = atoi(token) % DL_MAX_COLORS;
	displayInfo->drawingAreaForegroundColor =
	  dlDisplay->clr;
      } 
      else if (STREQL(token,"type")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	parseDisplayType (displayInfo, dlDisplay, token);
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
  } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
	    && (tokenType != T_EOF) );

  appendDlElement(displayInfo->dlElementList,dlElement); 

  /* fix up x,y so that 0,0 (old defaults) are replaced 
   */
  if (dlDisplay->object.x <= 0) dlDisplay->object.x = DISPLAY_DEFAULT_X;
  if (dlDisplay->object.y <= 0) dlDisplay->object.y = DISPLAY_DEFAULT_Y;
 
  return dlElement;
}
