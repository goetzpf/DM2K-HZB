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
 * .03  09-07-95        vong    remove all the falling line and rising line stuff
 * .04  09-13-95        vong    conform to c++ syntax
 * .05  03-06-96        vong    in closeDislay(), disable the edit
 *                              functions by calling disableEditFunctions()
 *                              if there is no display left.
 *
 * .06  09-02-96        romsky  fixe bug in toggleHighLightRectangles
 *
 *****************************************************************************
 *
 *      24-02-97    Fabien      Use simpleCursor in display DrawingArea
 *
 *****************************************************************************
*/


/************************************************************************
 ****                  Utility Functions                             ****
 ************************************************************************/


#include <string.h>
#include <ctype.h>
#include <X11/IntrinsicP.h>

#if defined(__hpux) || defined(__linux__)
# include <stdlib.h>
# define INIT          register char *sp = instring;
# define GETC()        (*sp++)
# define PEEKC()       (*sp)
# define UNGETC(c)     (--sp)
# define RETURN(c)     return ((char*)c);
# define ERROR(c)      exit(c)
# include <regexp.h>
#else
# include <regexpr.h>
#endif

#include "dm2k.h"

#ifdef  __TED__
#include <Dt/Wsm.h>
#endif

#define MAX_DIR_LENGTH    512		/* max. length of directory name */

Boolean modalGrab = FALSE;

/*
 * function to open a specified file (as .adl if specified as .dl),
 *	looking in EPICS_DISPLAY_PATH directory if unavailable in the
 *	working directory
 */

FILE *dmOpenUseableFile(const char * filename)
{
  FILE * filePtr;
  char   name[MAX_FILE_CHARS];
  char   fullPathName[MAX_DIR_LENGTH];
  char   dirName[MAX_DIR_LENGTH];
  char * dir;
  char * adlPtr;
  int    suffixLength, usedLength, startPos;

/*  printf("openUseableFile(%s)\n", filename);*/

  /* try to open the file as a ".adl"  rather than ".dl" which the
   * editor generates
   */
  /* T. Straumann: avoid 'filename==0' */
  if (!filename) filename="";

  /* look in current directory first 
   */

/* this is for "backward-compatibility" */
#define DISPLAY_FILE_BLANK_SUFFIX "   "

  if ( (adlPtr = strstr(filename,DISPLAY_FILE_ASCII_SUFFIX)) != NULL) {
     /* ascii name */
     suffixLength = STRLEN(DISPLAY_FILE_ASCII_SUFFIX);
  } else if ( (adlPtr = strstr(filename,DISPLAY_FILE_BINARY_SUFFIX)) != NULL) {
     /* binary name */
     suffixLength = STRLEN(DISPLAY_FILE_BINARY_SUFFIX);
  } else if ( (adlPtr = strstr(filename,DISPLAY_FILE_BLANK_SUFFIX)) != NULL) {
     /* suffix consists of 3 blanks */
     suffixLength = STRLEN(DISPLAY_FILE_BLANK_SUFFIX);
  } else {
     /* no suffix, we'll use ASCII_SUFFIX */
/*     printf("no suffix, we'll use ASCII_SUFFIX\n");*/
     suffixLength = 0;
  }

  usedLength = STRLEN(filename);
  if (usedLength < suffixLength) usedLength = suffixLength;
/*
  if ( adlPtr != filename + usedLength - suffixLength ) {
     printf("suffix not at end: adlPtr = %d, usedLength - suffixLength = %d\n",
	    (int)(adlPtr-filename), usedLength - suffixLength);
  }
*/
  strncpy(name,filename,usedLength-suffixLength);
  name[usedLength-suffixLength] = '\0';
  strcat(name,DISPLAY_FILE_ASCII_SUFFIX);
  filePtr = fopen(name,"r");

  /* if not in current directory, look in EPICS_DISPLAY_PATH directory 
   */
  if (filePtr == NULL) {
     dir = getenv(DISPLAY_LIST_ENV);
     if (dir != NULL) {
        startPos = 0;
        while (filePtr == NULL &&
		extractStringBetweenColons(dir,dirName,startPos,&startPos)) {
	  strcpy(fullPathName,dirName);
	  strcat(fullPathName,"/");
	  strcat(fullPathName,name);
	  filePtr = fopen(fullPathName,"r");
        }
     }
  }

  return (filePtr);
}


/*
 *  extract strings between colons from input to output
 *    this function works as an iterator...
 */
Boolean extractStringBetweenColons(
  char *input,
  char *output,
  int  startPos,
  int  *endPos)
{
  int i, j;

  i = startPos; j = 0;
  while (input[i] != '\0') {
    if (input[i] != ':') {
         output[j++] = input[i];
    } else break;
    i++;
  }
  output[j] = '\0';
  if (input[i] != '\0') {
    i++;
  }
  *endPos = i;
  if (j == 0)
     return(False);
  else
     return(True);
}

/* recursive routine set widget field in element to NULL
 */
static void nullWidget(DlList *list)
{
    register DlElement *dlElement;
    
    if (list == NULL)
      return;

    dlElement = FirstDlElement(list);
    while (dlElement) 
    {
      dlElement->widget = NULL;

      if (dlElement->type == DL_Composite) 
	 nullWidget(dlElement->structure.composite->dlElementList);

      dlElement = dlElement->next;
    }
}

#ifdef __cplusplus
void destroyAnyWidgetCB (Widget , XtPointer cd, XtPointer)
#else
void destroyAnyWidgetCB (Widget w, XtPointer cd, XtPointer cbs)
#endif
{
  Widget *widgetPT;

  widgetPT = (Widget *) cd;
  *widgetPT = NULL;
}


/*
 * clean up the memory-resident display list (if there is one)
 */
void destroyDlDisplayList(DlList *l) 
{
  DlElement *dlElement, *freeElement;
  if (!l) return;
  if (l->count > 0) {

     dlElement = FirstDlElement(l);
     while (dlElement) {
	freeElement = dlElement;
	dlElement = dlElement->next;
	
	if (freeElement->run->destroy) {
	   (*freeElement->run->destroy)(freeElement);
	} 
	else {
	   free((char *)freeElement->structure.composite);
	   freeDlElement(freeElement);
	}
     }
     emptyDlList(l);
  }
}


void freeDisplayXResources (DisplayInfo *displayInfo)
{
  int i;
  Boolean alreadyFreedUnphysical;

  if (displayInfo->drawingAreaPixmap != (Pixmap)NULL) {
    XFreePixmap(display,displayInfo->drawingAreaPixmap);
    displayInfo->drawingAreaPixmap = (Pixmap)NULL;
  }
  if (displayInfo->colormap != NULL && displayInfo->dlColormapCounter > 0) {
    alreadyFreedUnphysical = False;
    for (i = 0; i < displayInfo->dlColormapCounter; i++) {
      if (displayInfo->colormap[i] != unphysicalPixel) {
        XFreeColors(display,cmap,&(displayInfo->colormap[i]),1,0);
	    } else if (!alreadyFreedUnphysical) {
      /* only free "unphysical" pixel once */
      XFreeColors(display,cmap,&(displayInfo->colormap[i]),1,0);
      alreadyFreedUnphysical = True;
      }
    }
    free( (char *) displayInfo->colormap);
    displayInfo->colormap = NULL;
    displayInfo->dlColormapCounter = 0;
    displayInfo->dlColormapSize = 0;
  }
  if (displayInfo->gc) {
    XFreeGC(display,displayInfo->gc);
    displayInfo->gc = NULL;
  }
  if (displayInfo->pixmapGC != NULL) {
    XFreeGC(display,displayInfo->pixmapGC);
    displayInfo->pixmapGC = NULL;
  }
  displayInfo->drawingAreaBackgroundColor = 0;
  displayInfo->drawingAreaForegroundColor = 0;
}


/*
 * function which cleans up a given displayInfo in the displayInfoList
 * (including the displayInfo's display list if specified)
 */

void dmCleanupDisplayInfo(
  DisplayInfo *displayInfo,
  Boolean cleanupDisplayList)
{
  Widget       DA;
  UpdateTask * ut = &(displayInfo->updateTaskListHead);

/* save off current DA */
  positionDisplayRead (displayInfo);
  DA = displayInfo->drawingArea;

  /* now set to NULL in displayInfo to signify "in cleanup" 
   */
  displayInfo->drawingArea = NULL;

  /* remove all update tasks in this display 
   */
  updateTaskDeleteAllTask(ut);
 
  /* Nothing is any more Highlighted  (DA will be destroyed) */
  displayInfo->selectedElementsAreHighlighted = False;

  /* as a composite widget, drawingArea is responsible for destroying
   * it's children
   */
  if (DA != NULL) {
    if (displayInfo->dialog) {
      /* the list of shell chlidren is not updated 
       */
      XtDestroyWidget(displayInfo->dialog);

      /* also we need to destroy and rebuild the shell 
       */
      destroyRebuildDisplayShell (displayInfo, displayInfo->displayType);

      displayInfo->dialog = NULL;
    }
    else 
      XtDestroyWidget(DA);
  }

  /* force a wait for all outstanding CA event completion 
   * (wanted to do   while (ca_pend_event() != ECA_NORMAL); 
   * but that sits there     forever)
   */
  caPendEvent ("dmCleanupDisplayInfo");


  /* if cleanupDisplayList == TRUE
   *  then global cleanup ==> delete shell, free memory/structures, etc
   */

  if (cleanupDisplayList) {
    if ( displayInfo->shell ) {
      XtDestroyWidget(displayInfo->shell);
      displayInfo->shell = NULL;
    }

    /* remove display lists here 
     */
    destroyDlDisplayList(displayInfo->dlElementList);
    destroyDlDisplayList(displayInfo->selectedDlElementList);
  } else {
    nullWidget(displayInfo->dlElementList);
  }

  /* free other X resources */
  freeDisplayXResources (displayInfo);
}


void dmRemoveDisplayInfo(DisplayInfo *displayInfo)
{
  if (displayInfoListHead == displayInfo)
	displayInfoListHead = displayInfoListHead->next;

  if (displayInfoListTail == displayInfo)
	displayInfoListTail = displayInfoListTail->prev;

  if (displayInfo->prev) displayInfo->prev->next = displayInfo->next;
  if (displayInfo->next) displayInfo->next->prev = displayInfo->prev;

  /* cleaup resources and free display list 
   */
  dmCleanupDisplayInfo(displayInfo,True);
  freeNameValueTable(displayInfo->nameValueTable,displayInfo->numNameValues);

  destroyDlDisplayList(displayInfo->selectedDlElementList);
  destroyDlDisplayList(displayInfo->dlElementList);
  DM2KFREE(displayInfo->selectedDlElementList);
  DM2KFREE(displayInfo->dlElementList);
  DM2KFREE(displayInfo->dlFile->name);
  DM2KFREE(displayInfo->dlFile);
  DM2KFREE(displayInfo->dlColormap);
  DM2KFREE(displayInfo);

  if (displayInfoListHead == NULL) {
    currentColormap     = defaultColormap;
    currentColormapSize = DL_MAX_COLORS;
    currentDisplayInfo  = NULL;
  }
}


/*
 * function to remove ALL displayInfo's
 *   this includes a full cleanup of associated resources and displayList
 */
void dmRemoveAllDisplayInfo()
{
  DisplayInfo *nextDisplay, *displayInfo;

  displayInfo = displayInfoListHead;

  while (displayInfo != NULL) {
    nextDisplay = displayInfo->next;
    dmRemoveDisplayInfo(displayInfo);
    displayInfo = nextDisplay;
  }

  displayInfoListHead = displayInfoListTail = NULL;

  currentColormap = defaultColormap;
  currentColormapSize = DL_MAX_COLORS;
  currentDisplayInfo = NULL;
}


void invalidObjectMessage (char *msg, DisplayInfo *displayInfo, DlElementType type)
{
  sprintf(msg, "Invalid Element : %s (%d)\n    for a %s display",
	  elementStringTable[type -MIN_DL_ELEMENT_TYPE],
	  type, stringValueTable[displayInfo->displayType]);
}


static void executeElement (DisplayInfo *displayInfo, DlElement *dlElement, Boolean warning)
{
  extern Boolean silentFlag, debugFlag;
  char msg[128];

  if ( checkControllerObject (displayInfo, dlElement) ) {
    dlElement->displayInfo = displayInfo;
    if ( dlElement->run->execute )
      (*dlElement->run->execute) (displayInfo, dlElement);
  }
  else {    /* this controller object is not allowed in this context */
    if ( dlElement->widget ) {
      XtDestroyWidget (dlElement->widget);
    }
    if ( (!silentFlag && warning) || debugFlag ) {
      invalidObjectMessage (msg, displayInfo, dlElement->type);
      fprintf(stderr, "\n!===! dm2k WARNING : %s !===!\n", msg);
      fprintf(stderr, "    Display file : %s\n    -- Object ignored --\n", displayInfo->dlFile->name);
    }
    dlElement->displayInfo = NULL;
    dlElement->widget = NULL;
    dlElement->actif = False;
  }
}



/*
 * traverse (execute) specified displayInfo's display list
 */

static void traverseDisplayList (DisplayInfo *displayInfo)
{
  DlElement *element;
  DlDisplay *dlDisplay;

  displayInfo->traverseDisplayListFlag = True;

  /* if needed change the shell type 
   */
  element = FirstDlElement(displayInfo->dlElementList);
  for  ( ; element != NULL; element = element->next) {
    if ( element->type != DL_Display ) 
      continue;

    dlDisplay = element->structure.display;

    rebuildDisplayShell (displayInfo, dlDisplay->displayType);


    /* and build the display Working Area 
     */
    if ( element->run->execute ) 
      (*element->run->execute) (displayInfo, element);
    
    XtRealizeWidget(displayInfo->shell);
    break;
  }

  /* traverse the display list */
  element = FirstDlElement(displayInfo->dlElementList);
  while (element) {
    if ( element->type != DL_Display ) {
      executeElement (displayInfo, element, True);
    }
    element = element->next;
  }
  displayInfo->traverseDisplayListFlag = False;
  
  drawingAreaDefineCursor (displayInfo);
  XFlush(display);
}


/*
 * traverse (execute) specified displayInfo's display list
 */
void dmTraverseDisplayList(DisplayInfo *displayInfo)
{
  traverseDisplayList (displayInfo);
  caPendEvent ("dmTraverseDisplayList");
}


/*
 * traverse (execute) all displayInfos and display lists
 */
void dmTraverseAllDisplayLists()
{
  DisplayInfo *displayInfo;

  displayInfo = displayInfoListHead;
  while (displayInfo != NULL) {
    traverseDisplayList (displayInfo);
    displayInfo = displayInfo->next;
  }
  caPendEvent ("dmTraverseAllDisplayLists");
}

/*
 * traverse (execute) specified displayInfo's display list non-widget elements
 */
void dmTraverseNonWidgetsInDisplayList(DisplayInfo *displayInfo)
{
  DlElement *element;
  Dimension width,height;

  if (displayInfo == NULL) return;

  /* fill the background with the background color */
  XSetForeground(display,displayInfo->pixmapGC,
  displayInfo->colormap[displayInfo->drawingAreaBackgroundColor]);
  XtVaGetValues(displayInfo->drawingArea,
                XmNwidth,&width,XmNheight,&height,NULL);
  XFillRectangle(display,displayInfo->drawingAreaPixmap,displayInfo->pixmapGC,
		 0, 0, (unsigned int)width,(unsigned int)height);

  /* traverse the display list */
  /* point element after the display */
  element = FirstDlElement(displayInfo->dlElementList)->next;
  while (element) {
    if (!element->widget) {
      executeElement (displayInfo, element, False);
    }
    element = element->next;
  }

  /* since the execute traversal copies to the pixmap,
   * now udpate the window 
   */
  XCopyArea(display,displayInfo->drawingAreaPixmap,
	    XtWindow(displayInfo->drawingArea),
	    displayInfo->pixmapGC, 0, 0, (unsigned int)width,
	    (unsigned int)height, 0, 0);

  /* change drawingArea's cursor to the appropriate cursor */
  drawingAreaDefineCursor (displayInfo);
}


/*
 * function to return the best fitting font for the field and string
 *   if textWidthFlag = TRUE:  use the text string and find width also
 *   if textWidthFlag = FALSE: ignore text,w fields and
 *	make judgment based on height info only & return
 *	width of largest character as *usedW
 */
int dmGetBestFontWithInfo(
  XFontStruct **fontTable,
  int nFonts,
  char *text,
  int h,
  int w, 
  int *usedH, 
  int *usedW,
  Boolean textWidthFlag)
{
  int i, temp, count, upper, lower;

  i = nFonts/2;
  upper = nFonts-1;
  lower = 0;
  count = 0;

  i = nFonts-1;
  while ( ( h < fontTable[i]->ascent + fontTable[i]->descent ) && ( i > 0 ) )
     i--;

  *usedH = fontTable[i]->ascent + fontTable[i]->descent;
  *usedW = fontTable[i]->max_bounds.width;

  if (textWidthFlag) {
    /* now select on width of bounding box */
    while ( ((temp = XTextWidth(fontTable[i],text,STRLEN(text))) > w)
	   && i > 0 ) 
      i--;

    *usedW = temp;

#ifdef DEBUG
    if ( *usedH > h || *usedW > w)
      if (errorOnI != i) {
         errorOnI = i;
         fprintf(stderr,
	"\ndmGetBestFontWithInfo: need another font near pixel height = %d",
		 h);
      }
#endif
  }
  return (i);
}


XtErrorHandler trapExtraneousWarningsHandler(String message)
{
  if (message && *message) 
  {
    /* "Attempt to remove non-existant passive grab" 
     */
    if (!strcmp(message,"Attempt to remove non-existant passive grab"))
	return(0);

    /* "The specified scale value is less than the minimum scale value." 
     * "The specified scale value is greater than the maximum scale value." 
     */
    if (!strcmp(message,"The specified scale value is"))
	return(0);
    } else {
	fprintf(stderr,"Warning: %s\n", message);
    }
  
  return(0);
}


/*
 * function to march up widget hierarchy to retrieve top shell, and
 *  then run over displayInfoList and return the corresponding DisplayInfo *
    F.P. : test on shellWidgetClass (no more topLevelShellWidgetClass)

 */
DisplayInfo *dmGetDisplayInfoFromWidget(Widget w)
{
  DisplayInfo *displayInfo = NULL;

  if (w == NULL) {
    INFORM_INTERNAL_ERROR();
    return displayInfo;
  }

  while (w && !XtIsSubclass(w, wmShellWidgetClass)) {
    w = XtParent(w);
  }

  if (w) {
    displayInfo = displayInfoListHead;
    while (displayInfo && (displayInfo->shell != w)) {
      displayInfo = displayInfo->next;
    }
  }
  return displayInfo;
}





/*
 * write specified displayInfo's display list
 */
void dmWriteDisplayList(DisplayInfo *displayInfo, FILE *stream)
{
  DlElement *element;

  writeDlFile(stream,displayInfo->dlFile,0);
  if ((element = FirstDlElement(displayInfo->dlElementList))) {
    /* This must be DL_DISPLAY */
    (element->run->write)(stream,element,0);
  }

/*  if ( element && ! element->structure.display->cmap )*/
  writeDlColormap(stream,displayInfo->dlColormap,0);
  element = element->next;

  /* traverse the display list 
   */
  while (element) {
    (*element->run->write)(stream,element,0);
    element = element->next;
  }

  fprintf(stream,"\n");
}

void dm2kSetDisplayTitle(DisplayInfo *displayInfo)
{
  char str[MAX_FILE_CHARS+10];

  if (displayInfo->dlFile) {
    char *tmp, *tmp1;
	/* T. Straumann: better be paranoid */
    tmp = tmp1 = displayInfo->dlFile->name ? displayInfo->dlFile->name : "";
    while (*tmp != '\0')
      if (*tmp++ == '/') tmp1 = tmp;
    if (displayInfo->hasBeenEditedButNotSaved) {
      strcpy(str,tmp1);
      strcat(str," (edited)");
      XtVaSetValues(displayInfo->shell,XmNtitle,str,NULL);
    } else {
      XtVaSetValues(displayInfo->shell,XmNtitle,tmp1,NULL);
    }
  }
}

void dm2kMarkDisplayBeingEdited(DisplayInfo *displayInfo)
{
  if (globalDisplayListTraversalMode == DL_EXECUTE) return;
  if (displayInfo->hasBeenEditedButNotSaved) return;
  displayInfo->hasBeenEditedButNotSaved = True;
  dm2kSetDisplayTitle(displayInfo);
}

static DlObject * GetObjectFromElement(DlElement * dlElement)
{
  DlObject * po = NULL;

  if (dlElement != NULL)
    {
      if (dlElement->type != DL_Element &&
	  dlElement->type >= MIN_DL_ELEMENT_TYPE &&
	  dlElement->type <= MAX_DL_ELEMENT_TYPE)
	po = &(dlElement->structure.rectangle->object);
      else
	po = &(dlElement->structure.element->structure.display->object);
    }

  return po;
}

/*
 * starting at tail of display list, look for smallest object which bounds
 *   the specified position 
 */
DlElement * lookupElement
   (DlList   * l, 
    Position   x0, 
    Position   y0,
    Boolean    searchForChildren)
{
  DlElement *element, *saveElement;
  int minWidth, minHeight;
  int pox, poy;

  /* traverse the display list */
  minWidth = INT_MAX;		/* according to XPG2's values.h */
  minHeight = INT_MAX;
  saveElement = NULL;

  /* single element lookup
   */
  element = l->tail;

  while (element != NULL) {
    register DlObject * po = GetObjectFromElement(element);

    if ( checkActiveObject (element) ) 
      {
	if (po == NULL) {
	  INFORM_INTERNAL_ERROR();
	  element = element->next;
	  continue;
	}

	if ( element->type == DL_Display ) {
	  /* x0 and y0 are the relative positions in the drawing area */
	  pox = poy = 0;
	} else {
	  pox = po->x;
	  poy = po->y;
	}

	if (((x0 >= pox) && (x0 <= pox + po->width))    &&
	    ((y0 >= poy) && (y0 <= poy + po->height)))
	  {
	    /* eligible element, now see if smallest element so far 
	     */
	    if (po->width < minWidth && po->height < minHeight) 
	      {
		/* if only DL_Display is smaller than object, return object 
		 */
		if (element->type != DL_Display || saveElement == NULL) {
		  minWidth = po->width;
		  minHeight = po->height;
		  saveElement = element;
		}
	      }
	  }
      }
    element = element->prev;
  }

  /*
   * if searchForChildren is true or if we are in EXECUTE mode, 
   * then we need to decompose a group hit into
   * its component, if in EDIT mode, then the group is what we want
   */
  if ( searchForChildren || 
       (globalDisplayListTraversalMode == DL_EXECUTE
       && saveElement && saveElement->type == DL_Composite) )
    {
      DlElement * nextChild;

      /* deepen find child of composite which was picked 
       */
      nextChild = lookupCompositeChild(saveElement,x0,y0);
      while (nextChild != saveElement && nextChild->type != DL_Composite) {
	saveElement = nextChild;
	nextChild = lookupCompositeChild(saveElement,x0,y0);
      }

      return nextChild;
    }
  return (saveElement);
}



/*
 * starting at head of composite (specified element), lookup picked object
 *    (a display cannot be in a composite element)
 */
DlElement *lookupCompositeChild(DlElement * composite,
				Position    x0,
				Position    y0)
{
  DlElement *element, *saveElement;
  int minWidth, minHeight;

  if (!composite || (composite->type != DL_Composite))
    return composite;

  minWidth = INT_MAX;		/* according to XPG2's values.h */
  minHeight = INT_MAX;
  saveElement = NULL;

  /* single element lookup
   */
  element = FirstDlElement(composite->structure.composite->dlElementList);

  while (element) {
    register DlObject * po = GetObjectFromElement(element);

    if ( checkActiveObject (element) ) {
      if (po == NULL) {
	INFORM_INTERNAL_ERROR();
	element = element->next;
	continue;
      }

      if (x0 >= po->x     && x0 <= po->x + po->width &&
	  y0 >= po->y && y0 <= po->y + po->height) {
	/* eligible element, now see if smallest element so far */
	if (po->width < minWidth && po->height < minHeight) {
	  minWidth = (element->structure.rectangle)->object.width;
	  minHeight = (element->structure.rectangle)->object.height;
	  saveElement = element;
	}
      }
      element = element->next;
    }
  }


  if (saveElement) {
    /* found a new element - if it is composite,
     * recurse, otherwise return it 
     */
    if (saveElement->type == DL_Composite) {
      return(lookupCompositeChild(saveElement,x0,y0));
    } else {
      return(saveElement);
    }
  } 
  else {
    /* didn't find anything, return old composite 
     */
    return(composite);
  }
}



/*
 * starting at tail of display list, look for smallest object which bounds
 *   the specified position if single point select, else look for list
 *   of objects bounded by the region defined by (x0,y0) and (x1,y1)
 *
 * if the smallest bounding object has a composite "parent" (is a member of
 *   a group) and searchForChildren is false then actually select 
 *   the composite/group.
 *
 *   - also update currentDisplayInfo's attribute / dynamicAttribute structures
 *     from previous data in display list for single element select
 */
void selectedElementsLookup(
  DlList   * l1,
  Position   x0, 
  Position   y0,
  Position   x1,
  Position   y1,
  DlList   * l2,
  Boolean    searchForChildren)
{
  DlElement * element;
  DlElement * saveElement;
  DlElement * displayElement;
  Position    x, y;
  int         minWidth;
  int         minHeight;
  char        string[48];

  /* traverse the display list */
  minWidth       = INT_MAX;		/* according to XPG2's values.h */
  minHeight      = INT_MAX;
  saveElement    = NULL;
  displayElement = NULL;

  /* number of pixels to consider  same as no motion... */
#define RUBBERBAND_EPSILON 4

  if ((x1 - x0) <= RUBBERBAND_EPSILON && (y1 - y0) <= RUBBERBAND_EPSILON) {
    /*
     * single element lookup
     *   N.B. - this is really just lookupElement()!
     */
    DlElement *dlElement;
    x = (x0 + x1)/2;
    y = (y0 + y1)/2;
    dlElement = lookupElement(l1,x,y,searchForChildren);
    if (dlElement) {
      DlElement *pE = createDlElement(DL_Element,(XtPointer)dlElement,NULL);
      if (pE) {
        appendDlElement(l2, pE);
      }
    }
    return;
  } else {
    /*
     * multi-element lookup
     *	don't allow DL_Display to be part of multi-element lookup
     *	(only single element lookups for display - this avoids problems
     *	further down the pike and makes sense since logically the display
     *	can in fact extend beyond visible borders {since objects can
     *	extend beyond visible borders})
     */
    element = LastDlElement(l1);

    while (element && element->prev) {
      register DlObject * po = GetObjectFromElement(element);

      if ( checkActiveObject (element) ) {
	if (element->type != DL_Display &&
	  (x0 <= po->x && x1 >= po->x + po->width) &&
	  (y0 <= po->y && y1 >= po->y + po->height)) {
	  DlElement *pE = createDlElement(DL_Element,(XtPointer)element,NULL);
	  if (pE) {
	    insertDlElement(l2, pE);
	  } else {
	    sprintf(string,"\nselectedElementsLookup: realloc failed!");
	    dmSetAndPopupWarningDialog(currentDisplayInfo,string,"Ok",NULL,NULL);
	    return;
	  }
	}
      }
      element = element->prev;
    }
    return;
  }
}


Boolean dmResizeDisplayList(
  DisplayInfo *displayInfo,
  Dimension newWidth, 
  Dimension newHeight)
{
  DlElement *elementPtr;
  float sX, sY;
  Dimension oldWidth, oldHeight;

  elementPtr = FirstDlElement(displayInfo->dlElementList);
  oldWidth = elementPtr->structure.display->object.width;
  oldHeight = elementPtr->structure.display->object.height;

  /* simply return (value FALSE) if no real change */
  if (oldWidth == newWidth && oldHeight == newHeight) return (FALSE);

  /* resize the display, then do selected elements */
  elementPtr->structure.display->object.width = newWidth;
  elementPtr->structure.display->object.height = newHeight;

  /* proceed with scaling...*/
  sX = (float) ((float)newWidth/(float)oldWidth);
  sY = (float) ((float)newHeight/(float)oldHeight);

  resizeDlElementList(displayInfo->dlElementList,0,0,sX,sY);
  return (TRUE);
}



/*
 * function to resize the selected display elements based on new drawingArea.
 *
 *   return value is a boolean saying whether resized actually occurred.
 *   this function resizes the selected elements when the whole display
 *   is resized.
 */

Boolean dmResizeSelectedElements(
  DisplayInfo *displayInfo,
  Dimension newWidth, 
  Dimension newHeight)
{
  DlElement *elementPtr;
  float sX, sY;
  Dimension oldWidth, oldHeight;

  elementPtr = FirstDlElement(displayInfo->dlElementList);
  oldWidth = elementPtr->structure.display->object.width;
  oldHeight = elementPtr->structure.display->object.height;

  /* simply return (value FALSE) if no real change */
  if (oldWidth == newWidth && oldHeight == newHeight) return (FALSE);

  /* resize the display, then do selected elements */
  elementPtr->structure.display->object.width = newWidth;
  elementPtr->structure.display->object.height = newHeight;

  /* proceed with scaling...*/
  sX = (float) ((float)newWidth/(float)oldWidth);
  sY = (float) ((float)newHeight/(float)oldHeight);

  resizeDlElementReferenceList(displayInfo->selectedDlElementList,0,0,sX,sY);
  return (TRUE);
}

void resizeDlElementReferenceList(
  DlList *dlElementList,
  int x,
  int y,
  float scaleX,
  float scaleY)
{
  DlElement *dlElement;
  if (dlElementList->count < 1) return;
  dlElement = FirstDlElement(dlElementList);
  while (dlElement) {
    DlElement *ele = dlElement->structure.element;
    if (ele->type != DL_Display) {
      int w = ele->structure.rectangle->object.width;
      int h = ele->structure.rectangle->object.height;
      int xOffset = (int) (scaleX * (float) w + 0.5) - w;
      int yOffset = (int) (scaleY * (float) h + 0.5) - h;
      if (ele->run->scale) {
        ele->run->scale(ele,xOffset,yOffset);
      }
      w = ele->structure.rectangle->object.x - x;
      h = ele->structure.rectangle->object.y - y;
      xOffset = (int) (scaleX * (float) w + 0.5) - w;
      yOffset = (int) (scaleY * (float) h + 0.5) - h;
      if (ele->run->move) {
        ele->run->move(ele,xOffset,yOffset);
      }
    }
    dlElement = dlElement->next;
  }
}

void resizeDlElementList(
  DlList *dlElementList,
  int x,
  int y,
  float scaleX,
  float scaleY)
{
  DlElement *ele;
  if (dlElementList->count < 1) return;
  ele = FirstDlElement(dlElementList);
  while (ele) {
    if (ele->type != DL_Display) {
      int w = ele->structure.rectangle->object.width;
      int h = ele->structure.rectangle->object.height;
      int xOffset = (int) (scaleX * (float) w + 0.5) - w;
      int yOffset = (int) (scaleY * (float) h + 0.5) - h;
      if (ele->run->scale) {
        (*ele->run->scale)(ele,xOffset,yOffset);
      }

      w = ele->structure.rectangle->object.x - x;
      h = ele->structure.rectangle->object.y - y;
      xOffset = (int) (scaleX * (float) w + 0.5) - w;
      yOffset = (int) (scaleY * (float) h + 0.5) - h;

      if (ele->run->move) {
        (*ele->run->move)(ele,xOffset,yOffset);
      }
    }
    ele = ele->next;
  }
}

void shiftDlElementList(DlList * dlElementList, int x, int y)

{
  DlElement *ele;

  if (dlElementList->count < 1) 
    return;

  ele = FirstDlElement(dlElementList);

  while (ele) {
    if (ele->type != DL_Display) {
      if (ele->run->move) 
        (*ele->run->move)(ele, x, y);
    }
    ele = ele->next;
  }
}

/******************************************
 ************ rubberbanding, etc.
 ******************************************/


GC xorGC;

/*
 * initialize rubberbanding
 */

void initializeRubberbanding()
{
/*
 * create the xorGC and rubberbandCursor for drawing while dragging
 */
  xorGC = XCreateGC(display,rootWindow,0,NULL);
  XSetSubwindowMode(display,xorGC,IncludeInferiors);
  XSetFunction(display,xorGC,GXinvert);
#if 0
#endif
  XSetForeground(display,xorGC,WhitePixel(display,screenNum));
#if 0
  XSetForeground(display,xorGC,getPixelFromColormapByString(display,screenNum,
	cmap,"grey50"));
#endif
}




/*
 * do rubberbanding
 */
void doRubberbanding(
  Window window,
  Position *initialX,
  Position *initialY,
  Position *finalX,
  Position *finalY)
{
  XEvent event;

  int x0, y0, x1, y1;
  unsigned int w, h;

  *finalX = *initialX;
  *finalY = *initialY;
  x0 = *initialX;
  y0 = *initialY;
  x1 = x0;
  y1 = y0;
  w = (Dimension) 0;
  h = (Dimension) 0;


/* have all interesting events go to window */
  XGrabPointer(display,window,FALSE,
	(unsigned int) (ButtonMotionMask|ButtonReleaseMask),
        GrabModeAsync,GrabModeAsync,None,rubberbandCursor,CurrentTime);

/* grab the server to ensure that XORing will be okay */
  XGrabServer(display);
  XDrawRectangle(display,window, xorGC,
	MIN(x0,x1), MIN(y0,y1), w, h);

/*
 * now loop until the button is released
 */
  while (TRUE) {
	XtAppNextEvent(appContext,&event);
	switch (event.type) {
		case ButtonRelease:
		/* undraw old one */
			XDrawRectangle(display,window, xorGC,
			    MIN(x0,x1), MIN(y0,y1), w, h);
			XUngrabServer(display);
			XUngrabPointer(display,CurrentTime);
			XFlush(display);
			*initialX =  MIN(x0,event.xbutton.x);
			*initialY =  MIN(y0,event.xbutton.y);
			*finalX   =  MAX(x0,event.xbutton.x);
			*finalY   =  MAX(y0,event.xbutton.y);
			return;		/* return from while(TRUE) */
		case MotionNotify:
		/* undraw old one */
			XDrawRectangle(display,window, xorGC,
			    MIN(x0,x1), MIN(y0,y1), w, h);
		/* update current coordinates */
			x1 = event.xbutton.x;
			y1 = event.xbutton.y;
			w =  (MAX(x0,x1) - MIN(x0,x1));
			h =  (MAX(y0,y1) - MIN(y0,y1));
		/* draw new one */
			XDrawRectangle(display,window, xorGC,
			    MIN(x0,x1), MIN(y0,y1), w, h); 
			break;
		default:
			XtDispatchEvent(&event);
	}
  }
}




/*
 * do (multiple) dragging  of all elements in global selectedElementsArray
 *	RETURNS: boolean indicating whether drag ended in the window
 *	(and hence was valid)
 */
Boolean doDragging(
  Window window,
  Dimension daWidth,
  Dimension daHeight,
  Position initialX, 
  Position initialY, 
  Position *finalX,
  Position *finalY)
{
  int minX, maxX, minY, maxY, groupWidth, groupHeight,
	groupDeltaX0, groupDeltaY0, groupDeltaX1, groupDeltaY1;
  XEvent event;
  int xOffset, yOffset;
  DisplayInfo *cdi;
  int xdel, ydel;
  DlElement *dlElement;

  /* if on current display, simply return */
  if (currentDisplayInfo == NULL) return (False);

  cdi = currentDisplayInfo;

  xOffset = 0;
  yOffset = 0;

  minX = INT_MAX; minY = INT_MAX;
  maxX = INT_MIN; maxY = INT_MIN;

  /* have all interesting events go to window */
  XGrabPointer(display,window,FALSE,
    (unsigned int)(ButtonMotionMask|ButtonReleaseMask),
        GrabModeAsync,GrabModeAsync,None,dragCursor,CurrentTime);
  /* grab the server to ensure that XORing will be okay */
  XGrabServer(display);

  /* as usual, type in union unimportant as long as object is 1st thing...*/
  dlElement = FirstDlElement(cdi->selectedDlElementList);
  while (dlElement) {
    DlElement *pE = dlElement->structure.element;
    if (pE->type != DL_Display) {
      register DlObject * po = GetObjectFromElement(pE);

      if (po) {
	XDrawRectangle(display,window, xorGC, 
		       po->x + xOffset, po->y + yOffset, 
		       po->width , po->height);
	minX = MIN(minX, (int)po->x);
	maxX = MAX(maxX, (int)po->x + (int)po->width);
	minY = MIN(minY, (int)po->y);
	maxY = MAX(maxY, (int)po->y + (int)po->height);
      }
    }
    dlElement = dlElement->next;
  }
  groupWidth = maxX - minX;
  groupHeight = maxY - minY;
  /* how many pixels is the cursor position from the left edge of all objects */
  groupDeltaX0 = initialX - minX;
  /* how many pixels is the cursor position from the top edge of all objects */
  groupDeltaY0 = initialY - minY;
  /* how many pixels is the cursor position from the right edge of all objects */
  groupDeltaX1 = groupWidth - groupDeltaX0;
/* how many pixels is the cursor position from the bottom edge of all objects */
  groupDeltaY1 = groupHeight - groupDeltaY0;

/*
 * now loop until the button is released
 */
  while (TRUE) {
    XtAppNextEvent(appContext,&event);
    switch (event.type) {
    case ButtonRelease:
      /* undraw old ones */
      dlElement = FirstDlElement(cdi->selectedDlElementList);
      while (dlElement) {
        DlElement *pE = dlElement->structure.element;
        if (pE->type != DL_Display) {
          register DlObject * po = GetObjectFromElement(pE);

	  if (po)
	    XDrawRectangle(display,window, xorGC,
			   po->x + xOffset, po->y + yOffset,
			   po->width , po->height);
        }
        dlElement = dlElement->next;
      }
			XUngrabServer(display);
			XUngrabPointer(display,CurrentTime);
      *finalX = initialX + xOffset;
      *finalY = initialY + yOffset;
      /* (always return true - for clipped dragging...) */
      return (True);	/* return from while(TRUE) */
    case MotionNotify:
      /* undraw old ones */
      dlElement = FirstDlElement(cdi->selectedDlElementList);
      while (dlElement) {
        DlElement *pE = dlElement->structure.element;
        if (pE->type != DL_Display) {
          register DlObject * po = GetObjectFromElement(pE);

	  if (po)
	    XDrawRectangle(display,window, xorGC,
			   po->x + xOffset, po->y + yOffset,
			   po->width , po->height);
        }

        dlElement = dlElement->next;
      }
      /* update current coordinates */
      if (event.xmotion.x < groupDeltaX0)
        xdel = groupDeltaX0;
      else
      if (event.xmotion.x > (int)(daWidth-groupDeltaX1))
        xdel =  daWidth - groupDeltaX1;
      else
        xdel =  event.xmotion.x;
      if (event.xmotion.y < groupDeltaY0)
        ydel = groupDeltaY0;
      else
      if (event.xmotion.y > (int)(daHeight-groupDeltaY1))
        ydel =  daHeight - groupDeltaY1;
      else
        ydel =  event.xmotion.y;

			xOffset = xdel - initialX;
			yOffset  = ydel - initialY;
      dlElement = FirstDlElement(cdi->selectedDlElementList);
      while (dlElement) {
        DlElement *pE = dlElement->structure.element;
        if (pE->type != DL_Display) {
          register DlObject * po = GetObjectFromElement(pE);

	  if (po)
	    XDrawRectangle(display,window, xorGC,
			   po->x + xOffset, po->y + yOffset,
			   po->width , po->height);
        }
        dlElement = dlElement->next;
      }
      break;
    default:
      XtDispatchEvent(&event);
    }
  }
}




/*
 * do PASTING (with drag effect) of all elements in global
 *		 clipboardElementsArray
 *	RETURNS: DisplayInfo ptr indicating whether drag ended in a display
 *		 and the positions in that display
 */
DisplayInfo *doPasting(
  Position *displayX, 
  Position *displayY,
  int *offsetX,
  int *offsetY)
{
  XEvent event;
  DisplayInfo *displayInfo;
  int dx, dy, xul, yul, xlr, ylr;
  Window window, childWindow, root, child;
  int rootX, rootY, winX, winY;
  unsigned int mask;
  DlElement *dlElement = NULL;

  /* if no clipboard elements, simply return */
  if (IsEmpty(clipboard)) return NULL;

  window = RootWindow(display,screenNum);

  /* get position of upper left element in display */
  xul = INT_MAX;
  yul = INT_MAX;
  xlr = 0;
  ylr = 0;
  /* try to normalize for paste such that cursor is in middle of pasted objects */
  dlElement = FirstDlElement(clipboard);
  while (dlElement) {
    if (dlElement->type != DL_Display) {
      register DlObject * po = GetObjectFromElement(dlElement);

      if (po) {
	xul = MIN(xul, po->x);
	yul = MIN(yul, po->y);
	xlr = MAX(xlr, po->x + po->width);
	ylr = MAX(ylr, po->y + po->height);
      }
    }
    dlElement = dlElement->next;
  }
  dx = (xul + xlr)/2;
  dy = (yul + ylr)/2;

  /* update offsets to be added when paste is done */
  *offsetX = -dx;
  *offsetY = -dy;

  if (!XQueryPointer(display,window,&root,&child,&rootX,&rootY,
                     &winX,&winY,&mask)) {
    XtAppWarning(appContext,"doPasting: query pointer error");
  }


  /* have all interesting events go to window  - including some for WM's sake */
  XGrabPointer(display,window,False, (unsigned int)(PointerMotionMask|
               ButtonReleaseMask|ButtonPressMask|EnterWindowMask),
               GrabModeAsync,GrabModeAsync,None,dragCursor,CurrentTime);
  /* grab the server to ensure that XORing will be okay */
  XGrabServer(display);

  /* as usual, type in union unimportant as long as object is 1st thing...*/
  dlElement = FirstDlElement(clipboard);
  while (dlElement) {
    if (dlElement->type != DL_Display) {
      register DlObject * po = GetObjectFromElement(dlElement);

      if (po)
	XDrawRectangle(display,window, xorGC, 
		       rootX + po->x - dx, rootY + po->y - dy,
		       po->width, po->height);
    }
    dlElement = dlElement->next;
  }

  /*
   * now loop until the button is released
   */
  while (TRUE) {
    XtAppNextEvent(appContext,&event);
    switch (event.type) {
    case ButtonRelease:
      /* undraw old ones */
      dlElement = FirstDlElement(clipboard);
      while (dlElement) {
        if (dlElement->type != DL_Display) {
          register DlObject * po = GetObjectFromElement(dlElement);

	  if (po)
	    XDrawRectangle(display,window, xorGC, 
			   rootX + po->x - dx, rootY + po->y - dy,
			   po->width, po->height);
        }
        dlElement = dlElement->next;
      }
      XUngrabServer(display);
      XUngrabPointer(display,CurrentTime);
      XSync(display,False);
      while (XtAppPending(appContext)) {
        XtAppNextEvent(appContext,&event);
        XtDispatchEvent(&event);
      }
      displayInfo = pointerInDisplayInfo;
      if (displayInfo) {
        XTranslateCoordinates(display,window,
                              XtWindow(displayInfo->drawingArea),
                              rootX,rootY,
                              &winX,&winY,&childWindow);
      }
      *displayX =  winX;
      *displayY =  winY;
      return (displayInfo);

    case MotionNotify:
      /* undraw old ones */
      dlElement = FirstDlElement(clipboard);
      while (dlElement) {
        if (dlElement->type != DL_Display) {
          register DlObject * po = GetObjectFromElement(dlElement);

	  if (po)
	    XDrawRectangle(display,window, xorGC,
			   rootX + po->x - dx, rootY + po->y - dy,
			   po->width, po->height);
        }
        dlElement = dlElement->next;
      }
      /* update current coordinates */
      rootX = event.xbutton.x_root;
      rootY = event.xbutton.y_root;

      /* draw new ones */
      dlElement = FirstDlElement(clipboard);
      while (dlElement) {
        if (dlElement->type != DL_Display) {
          register DlObject * po = GetObjectFromElement(dlElement);

	  if (po)
	    XDrawRectangle(display,window, xorGC,
			   rootX + po->x - dx, rootY + po->y - dy,
			   po->width, po->height);
        }
        dlElement = dlElement->next;
      }
      break;

    default:
      XtDispatchEvent(&event);
      break;
    }
  }
}




/*
 * function to see if specified element is already in the global
 *	selectedElementsArray and return True or False based on that evaluation
 */
Boolean displayAlreadySelected (DlElement *element)
{
  DlElement *dlElement, *pE;

  pE = element->structure.element;
  if (pE->type != DL_Display) return (False);
  if (!currentDisplayInfo) return (False);

  if (IsEmpty(currentDisplayInfo->selectedDlElementList)) return False;
  dlElement = FirstDlElement(currentDisplayInfo->selectedDlElementList);
  while (dlElement) {
    DlElement *pE = element->structure.element;
    if ( dlElement->structure.element == pE )
      return True;
    dlElement = dlElement->next;
  }
  return (False);
}



Boolean alreadySelected(DlElement *element)
{
  DlElement *dlElement;

  if (!currentDisplayInfo) return (False);
  if (IsEmpty(currentDisplayInfo->selectedDlElementList)) return False;
  dlElement = FirstDlElement(currentDisplayInfo->selectedDlElementList);
  while (dlElement) {
    DlElement *pE = element->structure.element;
    if ((pE->type != DL_Display) && (dlElement->structure.element == pE))
      return True;
    dlElement = dlElement->next;
  }
  return (False);
}




static void toggleHighLightRectangles(
	  DisplayInfo *displayInfo, 
	  int         xOffset, 
	  int         yOffset)
{
  DlElement *dlElement = FirstDlElement(displayInfo->selectedDlElementList);

  while (dlElement) 
  {
    if (dlElement->type != DL_Display) 
    {
     int                  width;
     int                  height;
     register  DlObject * po = GetObjectFromElement(dlElement);
     
     if (po)
       {
	 width  = po->width + xOffset;
	 height = po->height + yOffset;
	 
	 if (width > 0 && height > 0 && displayInfo->drawingArea != NULL)  {
	   XDrawRectangle(XtDisplay(displayInfo->drawingArea),
			  XtWindow(displayInfo->drawingArea),xorGC,
			  po->x, po->y, (Dimension)width,  (Dimension)height);
	 }
       }
    }
    dlElement = dlElement->next;
  }
}

/*
 * do (multiple) resizing of all elements in global selectedElementsArray
 *	RETURNS: boolean indicating whether resize ended in the window
 *	(and hence was valid)
 */
Boolean doResizing(Window     window,
		   Position   initialX, 
		   Position   initialY, 
		   Position * finalX, 
		   Position * finalY)
{
  XEvent      event;
  int         xOffset = 0;
  int         yOffset = 0;
  Boolean     inWindow = True;


  if (currentDisplayInfo == NULL) return False;

  /* 
   * have all interesting events go to window 
   */
  XGrabPointer(display,window,FALSE,
	       (unsigned int) (ButtonMotionMask|ButtonReleaseMask),
	       GrabModeAsync,GrabModeAsync,None,resizeCursor,CurrentTime);

  /* 
   * grab the server to ensure that XORing will be okay 
   */
  XGrabServer(display);

  toggleHighLightRectangles(currentDisplayInfo, xOffset, yOffset);

  /*
   * now loop until the button is released
   */
  while (TRUE) {
    XtAppNextEvent(appContext,&event);

    switch (event.type) 
    {
      case ButtonRelease:
        /* undraw old ones */
        toggleHighLightRectangles(currentDisplayInfo, xOffset, yOffset);

        XUngrabServer(display);
        XUngrabPointer(display, CurrentTime);

        *finalX =  initialX + xOffset;
        *finalY =  initialY + yOffset;

        return (inWindow);	/* return from while(TRUE) */
	break;

      case MotionNotify:
        /* undraw old ones */
        toggleHighLightRectangles(currentDisplayInfo, xOffset, yOffset);

        /* update current coordinates */
        xOffset = event.xbutton.x - initialX;
        yOffset = event.xbutton.y - initialY;

        /* draw new ones */
        toggleHighLightRectangles(currentDisplayInfo, xOffset, yOffset);

        break;

      default:
        XtDispatchEvent(&event);
    }
  }
}

/*
 * function to delete composite's children/grandchildren... WIDGETS ONLY
 *  this does a depth-first search for any descendent composites...
 */
void deleteWidgetsInComposite(DisplayInfo *displayInfo, DlElement *ele)
{
  DlElement *child;

  if (ele->type == DL_Composite) {

    child = FirstDlElement(ele->structure.composite->dlElementList);
    while (child != NULL) {
      /* if composite, delete any children */
      if (child->type == DL_Composite) {
        deleteWidgetsInComposite(displayInfo,child);
      } else
      if (child->widget) {
        /* lookup widget of specified x,y,width,height and destroy */
        XtDestroyWidget(child->widget);
        child->widget = NULL;
      }
      child = child->next;
    }
  }
}


void copySelectedElementsIntoClipboard()
{
  DisplayInfo *cdi;
  DlElement *dlElement;

  if (!currentDisplayInfo) return;
  if (IsEmpty(currentDisplayInfo->selectedDlElementList)) return;

  cdi = currentDisplayInfo;
  if (!IsEmpty(clipboard)) {
    destroyDlDisplayList(clipboard);
  }
  
  dlElement = FirstDlElement(cdi->selectedDlElementList);
  while (dlElement) {
    DlElement *element = dlElement->structure.element;
    if (element->type != DL_Display) {
      DlElement *pE = element->run->create(element);
      appendDlElement(clipboard,pE);
    }
    dlElement = dlElement->next;
  }
}

void copyElementsIntoDisplay()
{
  DisplayInfo *cdi;
  Position displayX, displayY;
  int offsetX, offsetY;
  int deltaX, deltaY;

  DisplayInfo *displayInfo;
  DlElement *dlElement;

  /*
   * since elements are stored in clipboard in front-to-back order
   * they can be pasted/copied into display in clipboard index order
   */

  /* MDA -  since doPasting() can change currentDisplayInfo,
     clear old highlights now */
  displayInfo = displayInfoListHead;
  while (displayInfo) {
    currentDisplayInfo = displayInfo;
    unselectElementsInDisplay();
    displayInfo = displayInfo->next;
  }
  enableControllerRC (currentDisplayInfo);

  cdi = doPasting(&displayX,&displayY,&offsetX,&offsetY);
  deltaX = displayX + offsetX;
  deltaY = displayY + offsetY;

  if (cdi) {
  /* make sure pasted window is on top and has focus (and updated status) */
    XRaiseWindow(display,XtWindow(cdi->shell));
    XSetInputFocus(display,XtWindow(cdi->shell),RevertToParent,CurrentTime);
    currentDisplayInfo = cdi;
    enableControllerRC (currentDisplayInfo);
  } else {
    fprintf(stderr,"\ncopyElementsIntoDisplay:  can't discern current display");
    return;
  }

  /***
   *** now do actual element creation (with insertion into display list)
   ***/
  destroyDlDisplayList(cdi->selectedDlElementList);
  dlElement = FirstDlElement(clipboard);
  while (dlElement) {
    if (dlElement->type != DL_Display) {
      DlElement *pE, *pSE;
      if ( ! checkControllerObject (cdi, dlElement) ) {
	invalidObjectWarning (cdi, dlElement->type);
      }
      else {
	pE = (*dlElement->run->create)(dlElement);
	if (pE) {
	  appendDlElement(cdi->dlElementList,pE);
	  /* execute the structure */
	  if (pE->run->move) {
	    (*pE->run->move)(pE, deltaX, deltaY);
	  }
	  if (pE->run->execute) {
	    (*pE->run->execute)(cdi, pE);
	  }
	  pSE = createDlElement(DL_Element,(XtPointer)pE,NULL);
	  if (pSE) {
	    appendDlElement(cdi->selectedDlElementList,pSE);
	  }
	}
      }
    }
    dlElement = dlElement->next;
  }
  highlightSelectedElements();
  if (cdi->selectedDlElementList->count == 1) {
    setResourcePaletteEntries();
  }
}



void deleteElementsInDisplay()
{
  DlElement * dlElement;

  if (!currentDisplayInfo) return;
  if (IsEmpty(currentDisplayInfo->selectedDlElementList)) return;

  /* unhighlight, unselect,  and clear resource palette */
  unhighlightSelectedElements();
  /* unselectSelectedElements(); VTR */
  clearResourcePaletteEntries();

  dlElement = FirstDlElement(currentDisplayInfo->selectedDlElementList);
  while (dlElement) {
    DlElement *elementPtr = dlElement->structure.element;
    if (elementPtr->type != DL_Display) {
      /* now delete the selected element */
      if (elementPtr->widget) {
         /* lookup widget of specified x,y,width,height and destroy */
         XtDestroyWidget(elementPtr->widget);
         elementPtr->widget = NULL;
      }
      /* if composite, delete any widget children */
      deleteWidgetsInComposite(currentDisplayInfo,elementPtr);

      removeDlElement(currentDisplayInfo->dlElementList,elementPtr);
      if (elementPtr->run->destroy)
	(*elementPtr->run->destroy)(elementPtr);
    }
    dlElement = dlElement->next;
  }

  destroyDlDisplayList(currentDisplayInfo->selectedDlElementList);

  /* (MDA) could use a new element-lookup based on region (write routine
   *      which returns all elements which intersect rather than are
   *      bounded by a given region) and do partial traversal based on
   *      those elements in start and end regions.  this could be much
   *      more efficient and not suffer from the "flash" updates
   */
   dmTraverseNonWidgetsInDisplayList(currentDisplayInfo);

}


void unselectElementsInDisplay()
{
  if (!currentDisplayInfo) return;
  if (IsEmpty(currentDisplayInfo->selectedDlElementList)) return;
  /* unhighlight and clear resource palette */
  unhighlightSelectedElements();
  unselectSelectedElements();
  clearResourcePaletteEntries();

}


/*
 * select all renderable objects in display - note that this excludes the
 *	display itself
 */

void selectAllElementsInDisplay()
{
  DisplayInfo *cdi;
  DlElement *dlElement;

  if (!currentDisplayInfo) return;
  cdi = currentDisplayInfo;

  /* unhighlight and clear resource palette */
  unhighlightSelectedElements();
  destroyDlDisplayList(cdi->selectedDlElementList);
  clearResourcePaletteEntries();

  dlElement = FirstDlElement(cdi->dlElementList);
  while (dlElement) {
    if (dlElement->type != DL_Display) {
      DlElement *pE;
      pE = createDlElement(DL_Element,(XtPointer)dlElement,NULL);
      if (pE) {
        appendDlElement(cdi->selectedDlElementList,pE);
      }
    }
    dlElement = dlElement->next;
  }
  highlightSelectedElements();
  if (cdi->selectedDlElementList->count == 1) {
    setResourcePaletteEntries();
  }
}




/*
 * move elements further up (traversed first) in display list
 *  so that these are "behind" other objects
 */
void lowerSelectedElements(DisplayInfo *pD)
{
  DlElement *pE = 0; /* point to the element in the selected element list */
  DlElement *pF = 0; /* point to the first element in
                        the display element list */

  if (IsEmpty(pD->selectedDlElementList))
    return;

  pF = FirstDlElement(pD->dlElementList);
  pE = LastDlElement(pD->selectedDlElementList);

  while (pE) {
    DlElement * pT = pE->structure.element;

    if (pT->type != DL_Display && pF != pT) {
      removeDlElement(pD->dlElementList,pT);
      insertAfter(pD->dlElementList,pF,pT);
    }

    pE = pE->prev;
  }

  /* unhighlight and clear resource palette */
  unhighlightSelectedElements();
  unselectSelectedElements();
  clearResourcePaletteEntries();

  dmTraverseNonWidgetsInDisplayList(currentDisplayInfo);
}



/*
 * move elements further down (traversed last) in display list
 *  so that these are "in front of" other objects
 */
void raiseSelectedElements(DisplayInfo* pD)
{
  DlElement *pE = 0;
  if (IsEmpty(pD->selectedDlElementList)) return;
  pE = FirstDlElement(pD->selectedDlElementList);
#if 0
  printf("\nelement list :\n");
  dumpDlElementList(pD->dlElementList);
  printf("\nselected element list :\n");
  dumpDlElementList(pD->selectedDlElementList);
#endif
  while (pE) {
    DlElement *pT = pE->structure.element;
    if (pT->type != DL_Display) {
      removeDlElement(pD->dlElementList,pT);
      appendDlElement(pD->dlElementList,pT);
    }
    pE = pE->next;
  }
#if 0
  printf("\nelement list :\n");
  dumpDlElementList(pD->dlElementList);
  printf("\nselected element list :\n");
  dumpDlElementList(pD->selectedDlElementList);
#endif
  /* unhighlight and clear resource palette */
  unhighlightSelectedElements();
  unselectSelectedElements();
  clearResourcePaletteEntries();

  dmTraverseNonWidgetsInDisplayList(currentDisplayInfo);
}



/*
 * ungroup any grouped (composite) elements which are currently selected.
 *  this removes the appropriate Composite element and moves any children
 *  to reflect their new-found autonomy...
 */
void ungroupSelectedElements()
{
  DisplayInfo *cdi;
  DlElement *ele, *dlElement;


  /* unhighlight */
  unhighlightSelectedElements();

  cdi = currentDisplayInfo;
  dlElement = FirstDlElement(cdi->selectedDlElementList);
  while (dlElement) {
    ele = dlElement->structure.element;
    if (ele->type == DL_Composite) {
      insertDlListAfter(cdi->dlElementList,ele->prev,
                        ele->structure.composite->dlElementList);
      removeDlElement(cdi->dlElementList,ele);
      DM2KFREE(ele->structure.composite->dlElementList);
      DM2KFREE(ele->structure.composite);
      DM2KFREE(ele);
    }
    dlElement = dlElement->next;
  }

  /* unselect and clear resource palette */
  unselectSelectedElements();
  clearResourcePaletteEntries();

  dmTraverseNonWidgetsInDisplayList(currentDisplayInfo);
}

/*
 * align selected elements by top, bottom, left, or right edges
 */
void alignSelectedElements(int alignment)
{
  DlElement * ele;
  DlElement * dlElement;
  int         minX, minY;
  int         maxX, maxY;
  int         deltaX, deltaY;
  int         x0, y0;
  int         xOffset, yOffset;
  int         elementCount;

  if (!currentDisplayInfo) 
    return;

  if (IsEmpty(currentDisplayInfo->selectedDlElementList)) 
    return;

  minX = minY = INT_MAX;
  maxX = maxY = INT_MIN;

  unhighlightSelectedElements();

  dlElement = FirstDlElement(currentDisplayInfo->selectedDlElementList);

  /* loop and get min/max (x,y) values 
   */
  elementCount = 0;
  while (dlElement) 
  {
    register DlObject * po = GetObjectFromElement(dlElement);

    if (po != NULL)
      {
	minX = MIN(minX, po->x);
	minY = MIN(minY, po->y);
	
	x0 = (po->x + po->width);
	maxX = MAX(maxX,x0);
	
	y0 = (po->y + po->height);
	maxY = MAX(maxY,y0);
      }	

    dlElement = dlElement->next;
    elementCount++;
  }

  if (elementCount < 2)
    return;
  
  deltaX = (minX + maxX)/2;
  deltaY = (minY + maxY)/2;

  /* loop and set x,y values, and move if widgets 
   */
  dlElement = FirstDlElement(currentDisplayInfo->selectedDlElementList);
  while (dlElement) 
  {
    ele = dlElement->structure.element;

    /* can't move the display 
     */
    if (ele->type != DL_Display) {
      switch(alignment) 
      {
      case HORIZ_LEFT:
        xOffset = minX - ele->structure.rectangle->object.x;
        yOffset = 0;
        break;
      case HORIZ_CENTER:
        /* want   x + w/2 = dX  , therefore   x = dX - w/2   */
        xOffset = (deltaX - ele->structure.rectangle->object.width/2)
		              - ele->structure.rectangle->object.x;
        yOffset = 0;
        break;
      case HORIZ_RIGHT:
        /* want   x + w = maxX  , therefore   x = maxX - w  */
        xOffset = (maxX - ele->structure.rectangle->object.width)
                  - ele->structure.rectangle->object.x;
        yOffset = 0;
        break;
      case VERT_TOP:
        xOffset = 0;
        yOffset = minY - ele->structure.rectangle->object.y;
        break;
      case VERT_CENTER:
        /* want   y + h/2 = dY  , therefore   y = dY - h/2   */
        xOffset = 0;
        yOffset = (deltaY - ele->structure.rectangle->object.height/2)
                  - ele->structure.rectangle->object.y;
        break;
      case VERT_BOTTOM:
        /* want   y + h = maxY  , therefore   y = maxY - h  */
        xOffset = 0;
	      yOffset = (maxY - ele->structure.rectangle->object.height)
                   - ele->structure.rectangle->object.y;
        break;
      }

      if (ele->run->move) 
        (*ele->run->move)(ele,xOffset,yOffset);

      if (ele->widget) {
        XtMoveWidget(ele->widget,
		     (Position) ele->structure.rectangle->object.x,
		     (Position) ele->structure.rectangle->object.y);
      }
    }

    dlElement = dlElement->next;
  }


  /* retraverse all non-widgets (since potential window damage can result from
   * the movement of objects) 
   */
  dmTraverseNonWidgetsInDisplayList(currentDisplayInfo);

  highlightSelectedElements();
}




#if _not_used_


/*
 * moves specified <src> element to position just after specified <dst> element
 */
void moveElementAfter(DlElement  * dst,
		      DlElement  * src,
		      DlElement ** tail)
{
  if (src == *tail) {
    src->prev->next = NULL;
    *tail = src->prev;
  } 
  else {
    src->prev->next = src->next;
    src->next->prev = src->prev;
  }
  
  if (dst == *tail) {
    dst->next = src;
    src->next = NULL;
    src->prev = dst;
    *tail = src;
  } 
  else {
    dst->next->prev = src;
    src->next = dst->next;
    dst->next = src;
    src->prev = dst;
  }
}

/*
 * move all selected elements to position after specified element
 *	N.B.: this can move them up or down in relative visibility
 *	("stacking order" a la painter's algorithm)
 */
void  moveSelectedElementsAfterElement(DisplayInfo * displayInfo,
				       DlElement   * afterThisElement)
{
  DlElement * dlElement;

  if (displayInfo == NULL) 
    return;

  if (IsEmpty(displayInfo->selectedDlElementList)) 
    return;

  dlElement = LastDlElement(displayInfo->selectedDlElementList);

  while (dlElement)
  {
    /* if display was selected, skip over it (can't raise/lower it) 
     */
    DlElement *pE = dlElement->structure.element;

    if (pE && pE->type != DL_Display) 
    {
#if 0
      moveElementAfter(afterThisElement,pE,&(displayInfo->dlElementListTail));
#endif
      afterThisElement = afterThisElement->next;
    }
    dlElement = dlElement->prev;
  }

}

#endif /* _not_used_ */

/*
 * return Channel ptr given a widget id
 */
UpdateTask * getUpdateTaskFromWidget(Widget widget)
{
  DisplayInfo * displayInfo;
  UpdateTask  * pt;

  if (!(displayInfo = dmGetDisplayInfoFromWidget(widget)))
     return NULL; 

  pt = displayInfo->updateTaskListHead.next; 
  while (pt) {
     if (*((Widget *) pt->clientData) == widget) {
	return pt;
     }
     pt = pt->next;
  }
  return NULL;

}


/*
 * return UpdateTask ptr given a DisplayInfo* and x,y positions
 */
UpdateTask *getUpdateTaskFromPosition(
  DisplayInfo *displayInfo,
  int x,
  int y)
{
  UpdateTask *ptu, *ptuSaved = NULL;
  int minWidth, minHeight;
  
  if (displayInfo == (DisplayInfo *)NULL) return NULL;

  minWidth = INT_MAX;	 	/* according to XPG2's values.h */
  minHeight = INT_MAX;

  ptu = displayInfo->updateTaskListHead.next;
  while (ptu) {
    if (x >= (int)ptu->rectangle.x &&
	 x <= (int)ptu->rectangle.x + (int)ptu->rectangle.width &&
	 y >= (int)ptu->rectangle.y &&
	 y <= (int)ptu->rectangle.y + (int)ptu->rectangle.height) {
  /* eligible element, see if smallest so far */
      if ((int)ptu->rectangle.width < minWidth &&
           (int)ptu->rectangle.height < minHeight) {
	   minWidth = ptu->rectangle.width;
	   minHeight = ptu->rectangle.height;
	   ptuSaved = ptu;
      }
    }
    ptu = ptu->next;
  }
  return ptuSaved;
}



/*
 * generate a name-value table from the passed-in argument string
 *	returns a pointer to a NameValueTable as the function value,
 *	and the number of values in the second parameter
 *	Syntax: argsString: "a=b,c=d,..."
 */

NameValueTable *generateNameValueTable(const char *argsString, int *numNameValues)
{
 char           * copyOfArgsString;
 char           * name;
 char           * value;
 char           * s1;
 char             nameEntry[128];
 char             valueEntry[128];
 int              i, j, tableIndex, numPairs, numEntries, lastnonblank;
 NameValueTable * nameTable;
 Boolean          first;


 nameTable = NULL;
 copyOfArgsString = NULL;
 tableIndex = 0;

 if (argsString != NULL) 
   {
     copyOfArgsString = STRDUP(argsString);

     /* see how many a=b name/value pairs are in the string */
     numPairs = 0;
     i = 0;

     while (copyOfArgsString[i++] != '\0')
       if (copyOfArgsString[i] == '=') 
	 numPairs++;
     
     tableIndex = 0;
     first = True;

     for (numEntries = 0; numEntries < numPairs; numEntries++) {
       /* at least one pair, proceed 
	*/
       if (first) {
	 first = False;
	 nameTable = (NameValueTable *)calloc(numPairs,sizeof(NameValueTable));
	 /* name = value, name = value, ...  therefore */
	 /* name delimited by "=" and value delimited by ","  */
	 s1 = copyOfArgsString;
       } else {
	 s1 = NULL;
       }
       
       name = strtok(s1,"=");
       value = strtok(NULL,",");

       if (name != NULL && value != NULL) {
	 /* found legitimate name/value pair, put in table 
	  */
	 j = 0;
	 for (i = 0; i < (int) STRLEN(name); i++) {
	   if (!isspace(name[i]))
	     nameEntry[j++] =  name[i];
	 }
	 nameEntry[j] = '\0';
	 j = 0;
	 lastnonblank = 0;
	 for (i = 0; i < (int) STRLEN(value); i++) {
	   if (!isspace(value[i]) || j) {
	     valueEntry[j++] =  value[i];
	     if (!isspace(value[i]))
	       lastnonblank = j;
	   }
	 }
	 valueEntry[lastnonblank] = '\0';
	 nameTable[tableIndex].name = STRDUP(nameEntry);
	 nameTable[tableIndex].value = STRDUP(valueEntry);
	 tableIndex++;
       }
     }
     if (copyOfArgsString) free(copyOfArgsString); 
 } else {
   /* no pairs */
 }

 *numNameValues = tableIndex;
 return (nameTable);

}


/*
 * lookup name in name-value table, return associated value (or NULL if no
 *	match)
 */

char *lookupNameValue(
  NameValueTable *nameValueTable,
  int numEntries,
  char *name)
{
  extern NameValueTable defaultNameValueTable[];
  extern int defaultNameValueTableSize;
  int i;

  /* T. Straumann: avoid null ptr arg to string functions */
  if (!name) return NULL;
 
  if (nameValueTable != NULL && numEntries > 0) {
    for (i = 0; i < numEntries; i++)
      if (!strcmp(nameValueTable[i].name,name))
	return (nameValueTable[i].value);
  }

  for (i = 0; i < defaultNameValueTableSize; i++ ) {
     if (!strcmp(defaultNameValueTable[i].name,name))
	return (defaultNameValueTable[i].value);
  }
  /* fallback is value of env-variable - if it exists, else NULL */
  return (getenv(name));
}


/*
 * free the name value table
 *	first, all the strings pointed to by its entries
 *	then the table itself
 */

void freeNameValueTable(
  NameValueTable *nameValueTable,
  int numEntries)
{
  int i;
  if (nameValueTable != NULL) {
    for (i = 0; i < numEntries; i++) {
      if (nameValueTable[i].name != NULL) free ((char *)nameValueTable[i].name);
      if (nameValueTable[i].value != NULL) free ((char *)
		nameValueTable[i].value);
    }
    free ((char *)nameValueTable);
  }

}


/*
 * utility function to perform macro substitutions on input string, putting
 *	substituted string in specified output string (up to sizeOfOutputString
 *	bytes)
 */
void performMacroSubstitutions(DisplayInfo *displayInfo,
	char *inputString, char *outputString, int sizeOfOutputString)
{
  int i, j, k, n;
  char *value, name[MAX_TOKEN_LENGTH];

  outputString[0] = '\0';

  /* T. Straumann: added paranoia */
  if (!inputString) return; 

  if (!displayInfo) {
    strncpy(outputString,inputString,sizeOfOutputString-1);
    outputString[sizeOfOutputString-1] = '\0';
    return;
  }

  i = 0; j = 0; k = 0;
  if (inputString && STRLEN(inputString) > (size_t)0) {
    while (inputString[i] != '\0' && j < sizeOfOutputString-1) {
      if ( inputString[i] != '$') {
	outputString[j++] = inputString[i++];
      } else {
     /* found '$', see if followed by '(' */
	if (inputString[i+1] == '(' ) {
	   i = i+2;
	   while (inputString[i] != ')'  && inputString[i] != '\0' ) {
	      name[k++] = inputString[i++];
	   }
           name[k] = '\0';
	  /* now lookup macro */
	   value = lookupNameValue(displayInfo->nameValueTable,
		displayInfo->numNameValues,name);
           if (value) {
	     n = 0;
	     while (value[n] != '\0' && j < sizeOfOutputString-1)
		outputString[j++] = value[n++];
           }
	  /* to skip over ')' */
	   i++;
	} else {
	   outputString[j++] = inputString[i++];
	}
      }
      outputString[j] = '\0';
    }

  } else {
    outputString[0] = '\0';
  }
  if (j >= sizeOfOutputString-1) fprintf(stderr,"\n%s%s",
	" performMacroSubstitutions: substitutions failed",
	" - output buffer not large enough");
}



/*
 * colorMenuBar - get VUE and its "ColorSetId" straightened out...
 *   color the passed in widget (usually menu bar) and its children
 *   to the specified foreground/background colors
 */
void colorMenuBar(
  Widget widget,
  Pixel fg,
  Pixel bg)
{
  Cardinal numChildren;
  WidgetList children;
  Arg args[4];
  int i;
  Pixel localFg, top, bottom,select;

  XtSetArg(args[0],XmNchildren,&children);
  XtSetArg(args[1],XmNnumChildren,&numChildren);
  XtGetValues(widget,args,2);

  XmGetColors(XtScreen(widget),cmap,bg,&localFg,&top,&bottom,&select);
  XtSetArg(args[0],XmNforeground,fg);
  XtSetArg(args[1],XmNbackground,bg);

  XtSetArg(args[2],XmNtopShadowColor,top);
  XtSetArg(args[3],XmNbottomShadowColor,bottom);
  XtSetValues(widget,args,4);

  for (i = 0; i < numChildren; i++) {
    XtSetValues(children[i],args,2);
  }
}

#ifdef __cplusplus
void questionDialogCb(Widget, 
  XtPointer clientData,
  XtPointer callbackStruct)
#else
void questionDialogCb(Widget widget, 
  XtPointer clientData,
  XtPointer callbackStruct)
#endif
{
  DisplayInfo *displayInfo = (DisplayInfo *) clientData;
  XmAnyCallbackStruct *cbs = (XmAnyCallbackStruct *) callbackStruct;
  switch (cbs->reason) {
    case XmCR_OK:
      displayInfo->questionDialogAnswer = 1;
      break;
    case XmCR_CANCEL:
      displayInfo->questionDialogAnswer = 2;
      break;
    case XmCR_HELP:
      displayInfo->questionDialogAnswer = 3;
      break;
    default :
      displayInfo->questionDialogAnswer = -1;
      break;
  }
}

/*
 * function to create (if necessary), set and popup a display's question dialog
 */
void dmSetAndPopupQuestionDialog(DisplayInfo * displayInfo,
				 char        * message,
				 char        * okBtnLabel,
				 char        * cancelBtnLabel,
				 char        * helpBtnLabel)
{
  XmString xmString;
  XEvent event;

  /* 
   * create the dialog if necessary 
   */
  if (displayInfo->questionDialog == NULL) 
  {
    /* this doesn't seem to be working (and should check if MWM is running) */
    displayInfo->questionDialog = XmCreateQuestionDialog(displayInfo->shell,
							 "questionDialog",
							 NULL, 0);
    XtVaSetValues(displayInfo->questionDialog, 
		  XmNdialogStyle,XmDIALOG_APPLICATION_MODAL, NULL);

    XtVaSetValues(XtParent(displayInfo->questionDialog),
		  XmNtitle,"Question ?",NULL);

    XtAddCallback(displayInfo->questionDialog,
		  XmNokCallback,questionDialogCb, displayInfo);
    XtAddCallback(displayInfo->questionDialog,
		  XmNcancelCallback,questionDialogCb, displayInfo);
    XtAddCallback(displayInfo->questionDialog,
		  XmNhelpCallback,questionDialogCb, displayInfo);

    XtAddCallback(displayInfo->questionDialog,
		  XmNdestroyCallback, destroyAnyWidgetCB,
		  (XtPointer)&displayInfo->questionDialog);

  }

  if (message == NULL) return;

  xmString = XmStringCreateLtoR(message, "MESSAGE_TAG");
  XtVaSetValues(displayInfo->questionDialog,XmNmessageString,xmString,NULL);
  XmStringFree(xmString);

  /*
   * OK button
   */
  if (okBtnLabel) 
  {
    xmString = XmStringCreateLtoR(okBtnLabel,"BUTTON_TAG");
    XtVaSetValues(displayInfo->questionDialog,XmNokLabelString,xmString,NULL);
    XmStringFree(xmString);

    XtManageChild(XmMessageBoxGetChild(displayInfo->questionDialog,
				       XmDIALOG_OK_BUTTON));
  } 
  else {
    XtUnmanageChild(XmMessageBoxGetChild(displayInfo->questionDialog,
					 XmDIALOG_OK_BUTTON));
  }


  /*
   * Cancel button
   */
  if (cancelBtnLabel) 
  {
    xmString = XmStringCreateLtoR(cancelBtnLabel,"BUTTON_TAG");
    XtVaSetValues(displayInfo->questionDialog,
		  XmNcancelLabelString,xmString,NULL);
    XmStringFree(xmString);
    XtManageChild(XmMessageBoxGetChild(displayInfo->questionDialog,
				       XmDIALOG_CANCEL_BUTTON));
  } 
  else 
  {
    XtUnmanageChild(XmMessageBoxGetChild(displayInfo->questionDialog,
					 XmDIALOG_CANCEL_BUTTON));
  }


  /*
   * Help button
   */
  if (helpBtnLabel) 
  {
    xmString = XmStringCreateLtoR(helpBtnLabel, "BUTTON_TAG");
    XtVaSetValues(displayInfo->questionDialog,
		  XmNhelpLabelString,xmString,NULL);
    XmStringFree(xmString);

    XtManageChild(XmMessageBoxGetChild(displayInfo->questionDialog,
				       XmDIALOG_HELP_BUTTON));
  } 
  else 
  {
    XtUnmanageChild(XmMessageBoxGetChild(displayInfo->questionDialog,
					 XmDIALOG_HELP_BUTTON));
  }

  /*
   * reset dialog answer
   */
  displayInfo->questionDialogAnswer = 0;

  /*
   * Popup dialog
   */
  XtManageChild(displayInfo->questionDialog);
  XSync(display,FALSE);


  /* force Modal (blocking dialog) */
  /* !!! The XtAddGrab interferes with the normal behaviour of parent shell (F.P.) !!!
  | XtAddGrab(XtParent(displayInfo->questionDialog),True,False);
  */
  XmUpdateDisplay(XtParent(displayInfo->questionDialog));

  /*
   * Process events
   */
  while (!displayInfo->questionDialogAnswer || XtAppPending(appContext)) {
    XtAppNextEvent(appContext,&event);
    XtDispatchEvent(&event);
  }

  /*
   * Popdown dialog
   */
  XtUnmanageChild(displayInfo->questionDialog);
}

#ifdef __cplusplus
void warningDialogCb(Widget,
                     XtPointer clientData,
                     XtPointer callbackStruct)
#else
void warningDialogCb(Widget widget,
                     XtPointer clientData,
                     XtPointer callbackStruct)
#endif
{
  DisplayInfo *displayInfo = (DisplayInfo *) clientData;
  XmAnyCallbackStruct *cbs = (XmAnyCallbackStruct *) callbackStruct;
  switch (cbs->reason) {
    case XmCR_OK:
      displayInfo->warningDialogAnswer = WARNING_OK;
      break;
    case XmCR_CANCEL:
      displayInfo->warningDialogAnswer = WARNING_CANCEL;
      break;
    case XmCR_HELP:
      displayInfo->warningDialogAnswer = WARNING_HELP;
      break;
    default :
      displayInfo->warningDialogAnswer = WARNING_ERROR;
      break;
  }
}

/*
 * function to create (if necessary), set and popup a display's warning dialog
 */
void dmSetAndPopupWarningDialog(DisplayInfo    *displayInfo,
                               char           *message,
                               char           *okBtnLabel,
                               char           *cancelBtnLabel,
                               char           *helpBtnLabel)
{
  extern Boolean silentFlag;
  XmString xmString;
  Arg args[10];
  int n;

  /* create the dialog if necessary 
   */
  if (displayInfo->warningDialog == NULL) 
  {
    xmString = XmStringCreateLtoR("Warning !", "MESSAGE_TAG");

    n = 0;
    XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
    XtSetArg(args[n], XmNdialogTitle, xmString); n++;
    displayInfo->warningDialog = XmCreateWarningDialog(displayInfo->shell,
						   "warningDialog",args,n);
    XmStringFree(xmString);
    XtAddCallback(displayInfo->warningDialog,XmNokCallback,
		  warningDialogCb,displayInfo);
    XtAddCallback(displayInfo->warningDialog,XmNcancelCallback,
		  warningDialogCb,displayInfo);
    XtAddCallback(displayInfo->warningDialog,XmNhelpCallback,
		  warningDialogCb,displayInfo);
    XtAddCallback(displayInfo->warningDialog,
		  XmNdestroyCallback, destroyAnyWidgetCB,
		  (XtPointer)&displayInfo->warningDialog);
  }

  if (message == NULL) return;

  if ( !silentFlag ) {
    char strg[512];
    char *msg;

    fprintf(stderr, "===!! %s WARNING !!===\n", programName);
    strncpy (strg, message, sizeof(strg)-1);
    strg[sizeof(strg)-1] = '\0';
    msg = strtok (strg, "\n");
    while ( msg ) {
      if ( STRLEN (msg)) fprintf(stderr, "        %s\n", msg);
      msg = strtok (NULL, "\n");
    }
  }

  xmString = XmStringCreateLtoR(message, "MESSAGE_TAG");
  XtVaSetValues(displayInfo->warningDialog,XmNmessageString,xmString,NULL);
  XmStringFree(xmString);

  /*
   * OK button
   */ 
  if (okBtnLabel) 
  {
    xmString = XmStringCreateLtoR(okBtnLabel, "BUTTON_TAG");
    XtVaSetValues(displayInfo->warningDialog,XmNokLabelString,xmString,NULL);
    XmStringFree(xmString);
    XtManageChild(XmMessageBoxGetChild(displayInfo->warningDialog,
				       XmDIALOG_OK_BUTTON));
  } 
  else 
  {
    XtUnmanageChild(XmMessageBoxGetChild(displayInfo->warningDialog,
					 XmDIALOG_OK_BUTTON));
  }


  /*
   * Cancel button
   */ 
  if (cancelBtnLabel) {
    xmString = XmStringCreateLtoR(cancelBtnLabel, "BUTTON_TAG");
    XtVaSetValues(displayInfo->warningDialog,
		  XmNcancelLabelString,xmString,NULL);
    XmStringFree(xmString);

    XtManageChild(XmMessageBoxGetChild(displayInfo->warningDialog,
				       XmDIALOG_CANCEL_BUTTON));
  } 
  else 
  {
    XtUnmanageChild(XmMessageBoxGetChild(displayInfo->warningDialog,
					 XmDIALOG_CANCEL_BUTTON));
  }


  /*
   * Help button
   */ 
  if (helpBtnLabel) 
  {
    xmString = XmStringCreateLtoR(helpBtnLabel, "BUTTON_TAG");
    XtVaSetValues(displayInfo->warningDialog,
		  XmNhelpLabelString,xmString,NULL);
    XmStringFree(xmString);
    XtManageChild(XmMessageBoxGetChild(displayInfo->warningDialog,
				       XmDIALOG_HELP_BUTTON));
  } 
  else 
  {
    XtUnmanageChild(XmMessageBoxGetChild(displayInfo->warningDialog,
					 XmDIALOG_HELP_BUTTON));
  }

  /* reset answer
   */ 
  displayInfo->warningDialogAnswer = 0;

  XtManageChild(displayInfo->warningDialog);
  XSync(display,FALSE);

#if 0
  /* force Modal (blocking dialog) */
  XtAddGrab(XtParent(displayInfo->warningDialog),True,False);
  XmUpdateDisplay(XtParent(displayInfo->warningDialog));

  while (!displayInfo->warningDialogAnswer || XtAppPending(appContext)) {
    XtAppNextEvent(appContext,&event);
    XtDispatchEvent(&event);
  }

  XtRemoveGrab(XtParent(displayInfo->warningDialog));
  XtUnmanageChild(displayInfo->warningDialog);
#endif
}



void closeDisplay(Widget w) 
{
  DisplayInfo *newDisplayInfo;

  newDisplayInfo = dmGetDisplayInfoFromWidget(w);

  if (newDisplayInfo == currentDisplayInfo) {
#if 0
    highlightAndSetSelectedElements(NULL,0,0);
#endif
    clearResourcePaletteEntries();
    currentDisplayInfo = NULL;
  }

  if (newDisplayInfo->hasBeenEditedButNotSaved) 
  {
    char warningString[2*MAX_FILE_CHARS];
    char *tmp, *tmp1;

    strcpy(warningString,"Save before closing display :\n");
	/* T. Straumann: added paranoia */
    tmp = tmp1 = newDisplayInfo->dlFile->name ? newDisplayInfo->dlFile->name : "" ;
    
    while (*tmp != '\0') {
      if (*tmp++ == '/') 
	tmp1 = tmp;
    }

    strcat(warningString,tmp1);
    dmSetAndPopupQuestionDialog(newDisplayInfo,warningString,
				"Yes","No","Cancel");

    switch (newDisplayInfo->questionDialogAnswer) 
    {
      case 1 :
        /* Yes, save display */
        if (dm2kSaveDisplay(newDisplayInfo,
               newDisplayInfo->dlFile->name,True) == False) return;
        break;

      case 2 :
        /* No, return */
        break;

      case 3 :
        /* Don't close display */
        return;

      default :
        return;
    }
  }

  /* remove newDisplayInfo from displayInfoList and cleanup 
   */
  dmRemoveDisplayInfo(newDisplayInfo);

  if (displayInfoListHead == NULL) {
    disableEditFunctions();
  }
}



Pixel extractColor(DisplayInfo          * displayInfo, 
		   double                 value, 
		   register ColorRule   * colorRule,
		   int                    defaultColor) 
{
  int i;

  if (colorRule == NULL)
    return displayInfo->colormap[defaultColor];

  for (i = 0; i < colorRule->count; i++) {
    if (value <= colorRule->entries[i].upperBoundary && 
	value >= colorRule->entries[i].lowerBoundary) 
      {
	return displayInfo->colormap[colorRule->entries[i].colorIndex]; 
      }
  }

  return displayInfo->colormap[defaultColor];
}

#ifdef __TED__
void GetWorkSpaceList(Widget w) {
  Atom *paWs;
  char *pchWs;
  DtWsmWorkspaceInfo *pWsInfo;
  unsigned long numWorkspaces;

  if (DtWsmGetWorkspaceList(XtDisplay(w),
                             XRootWindowOfScreen(XtScreen(w)),
                             &paWs, (int *)&numWorkspaces) == Success)
  {
     int i;
     for (i=0; i<numWorkspaces; i++) {
       DtWsmGetWorkspaceInfo(XtDisplay(w),
                              XRootWindowOfScreen(XtScreen(w)),
                              paWs[i],
                              &pWsInfo);
       pchWs = (char *) XmGetAtomName (XtDisplay(w),
                                       pWsInfo->workspace);
       printf ("workspace %d : %s\n",pchWs);
     }
  }
}
#endif


/* Convert hex digits to ascii */
static char hex_digit_to_ascii[16]={'0','1','2','3','4','5','6','7','8','9',
    'a','b','c','d','e','f'};
 
int localCvtLongToHexString(
    long source,
    char  *pdest)
{
  long  val,temp;
  char  digit[10];
  int   i,j;
  char  *startAddr = pdest;
 
  *pdest++ = '0';
  *pdest++ = 'x';
  if(source==0) {
    *pdest++ = '0';
  } else {
    val = source;
    temp = 0xf & val;
    val = val >> 4;
    digit[0] = hex_digit_to_ascii[temp];
    if (val < 0) val = val & 0xfffffff;
    for (i=1; val!=0; i++) {
      temp = 0xf & val;
      val = val >> 4;
      digit[i] = hex_digit_to_ascii[temp];
    }
    for(j=i-1; j>=0; j--) {
      *pdest++ = digit[j];
    }

  }
  *pdest = 0;
  return((int)(pdest-startAddr));
}

/*
 * DlList and DlElement processing routings
 */

DlList *createDlList() 
{
  DlList *dlList = DM2KALLOC(DlList);

  if (dlList) {
    dlList->head = dlList->tail = NULL;
    dlList->count = 0;
  }
  return dlList;
}

void appendDlElement(DlList *l, DlElement *p) 
{
  p->prev = l->tail;
  p->next = NULL;

  if (l->tail != NULL) l->tail->next = p;
  l->tail = p;

  if (l->head == NULL) l->head = p;

  l->count++;
}

void insertDlElement(DlList *l, DlElement *p) 
{
  if (l->tail == NULL) {
    appendDlElement(l, p);
    return;
  }

  p->prev = NULL;
  p->next = l->head;

  if (l->head != NULL) l->head->prev = p;
  l->head = p;

  l->count++;
}

void insertAfter(DlList *l, DlElement *p1, DlElement *p2) 
{
  p2->prev = p1;
  p2->next = p1->next;

  if (l->tail == p1) l->tail = p2;
  else               p1->next->prev = p2;

  p1->next = p2;
  l->count++;
}

void insertDlListAfter(DlList *l1, DlElement *p, DlList *l2) 
{
  if (IsEmpty(l2)) return;

  FirstDlElement(l2)->prev = p;
  LastDlElement(l2)->next = p->next;

  if (LastDlElement(l1) == p) {
    LastDlElement(l1) = LastDlElement(l2);
  } else {
    p->next->prev = LastDlElement(l2);
  }

  p->next = FirstDlElement(l2);
  l1->count += l2->count;

  emptyDlList(l2);
}

void appendDlList(DlList *l1, DlList *l2) 
{
  if (IsEmpty(l2)) return;

  if (IsEmpty(l1)) {
    memcpy((char*)l1, (char*)l2, sizeof(DlList));
    emptyDlList(l2);
  }
  else {
    insertDlListAfter(l1, LastDlElement(l1), l2);
  }
}

void emptyDlList(DlList *l) 
{
  l->head = l->tail = 0;
  l->count = 0;
}

static void deallocateDlElements(DlElement *p)
{
  if (p) {
    deallocateDlElements(p->next);
    freeDlElement(p);
  }
}

void deallocateAllDlElements(DlList *l) 
{
  deallocateDlElements(FirstDlElement(l));
  emptyDlList(l);
}

void removeDlElement(DlList *l, DlElement *p) 
{
  l->count--;

  if (l->head == p) l->head = p->next;
  else              p->prev->next = p->next;

  if (l->tail == p) l->tail = p->prev;
  else              p->next->prev = p->prev;
  
  p->next = p->prev = 0;
}

void dumpDlElementList(DlList *l) 
{
  DlElement *p = NULL;
  int       i = 0;
  DlObject  *po ;

  printf("Number of Element = %ld\n",l->count);

  p = FirstDlElement(l);
  while (p) {
    if (p->type == DL_Element) {
      printf("%03d (%s)\n",
             i++,elementStringTable[p->structure.element->type-DL_Element]);
    } else {
      printf("%03d %s\n",i++,elementStringTable[p->type-DL_Element]);
    }

    po = &(p->structure.display->object);
    
    printf ("x=%-5d  y=%-5d  width=%-5d  height=%-5d\n",
	    po->x, po->y, po->width, po->height);

    p = p->next;
  }
  return;
}

static int isIdenticalNameValueTables(
	NameValueTable * oneTable,
	NameValueTable * otherTable,
	int              values)
{
  int i;
  
  /* check if all name:value pairs in one exist on other
   */
  /* T. Straumann: added paranoia check */
  for (i=0; i<values; i++) {
	if ( !oneTable[i].name || !oneTable[i].value || !oneTable[i].name || !otherTable[i].value)
		return FALSE;
  }

  for (i = 0; i < values; i++) {
    int j;
    int not_found = 1;

    for (j = 0; j < values; j++)  {
      if (strcmp(oneTable[i].name, otherTable[j].name) == 0
	  &&
	  strcmp(oneTable[i].value, otherTable[j].value) == 0) 
	{
	  not_found = 0;
	  break;
	}
    }

    if (not_found)
      return FALSE;
  }

  /* and vice versa
   */
  for (i = 0; i < values; i++) {
    int j;
    int not_found = 1;

    for (j = 0; j < values; j++)  {
      if (strcmp(oneTable[j].name, otherTable[i].name) == 0
	  &&
	  strcmp(oneTable[j].value, otherTable[i].value) == 0) 
	{
	  not_found = 0;
	  break;
	}
    }

    if (not_found)
      return FALSE;
  }

  return TRUE;
}

#define MAXSUBDIR 64

#if 0
/*
 * notice: ``original'' will be rewritten !!
 */
static char * clerefyFileName (char * original)
{
  char *ptr, *subdirs[MAXSUBDIR];
  int len;
  int i;
  int count;    /* count of subdirs on path */
  int slash_in; /* put slash in front of path or not */


  /* let's devide full file name into array of tokens
   */
  if (strtok(original, "/") == NULL)
    return NULL;

  subdirs[count = 0] = original;
  while ((ptr = strtok(NULL, "/")) != NULL) {
    if (++count >= MAXSUBDIR-1) return NULL;
    else                        subdirs[count] = ptr;
  }
    
  count++;


  /* process ``.'' and ``..'' subdirectories
   */
  if      (subdirs[0] && strcmp(subdirs[0], "/..") == 0)
    return NULL;
  else if (subdirs[0] && strcmp(subdirs[0], "/.") == 0)
    subdirs[0] = NULL;

  for (i = 0; i < count; i++) {
    if (subdirs[i] && strcmp(subdirs[i], "..") == 0) 
    {
      int last;

      for(last = i-1; last >= 0; last--)
	if (subdirs[last] != NULL)
	  break;

      if (last < 0)
	return NULL;

      subdirs[last] = subdirs[i] = NULL;
    }
    else if (subdirs[i] && strcmp(subdirs[i], ".") == 0) {
      subdirs[i] = NULL;
    }
  }


  /* calculate new file name string lenght
   */
  len =0;
  for (i = 0; i < count; i++) {
    if (subdirs[i] != NULL) 
      len += STRLEN(subdirs[i]) +1;
  }


  /* construct new file name string from ``subdirs'' array
   */
  if (*original == '/') {
    len++;
    slash_in = 1;
  } else 
    slash_in = 0;

  ptr = (char *) malloc (len);

  if (ptr == NULL)
    return NULL;

  *ptr = '\0';

  for (i = 0; i < count; i++) {
    if (subdirs[i] != NULL)  {
      /* put separator ``/'' between subdirectories
       */
      if (*ptr != '\0' || ( slash_in && subdirs[i][0] != '/' )) strcat(ptr, "/");
      strcat(ptr, subdirs[i]);
    }
  }

  return ptr;
}
#endif

static int isIdenticalDisplayInfos (DisplayInfo * one, DisplayInfo * other)
{
  if (one->versionNumber != other->versionNumber)
    return FALSE;

  /* T. Straumann: added paranoia */
  if (one->dlFile && other->dlFile && one->dlFile->name && other->dlFile->name &&
      strcmp(one->dlFile->name, other->dlFile->name))
    return FALSE;

  if (one->numNameValues != other->numNameValues)
     return FALSE;

  if (one->numNameValues != 0) 
  {
    if (!isIdenticalNameValueTables(one->nameValueTable,
				    other->nameValueTable,
				    other->numNameValues)) 
      return FALSE;
  }
  
  return TRUE;
}


DisplayInfo * lookupIdenticalDisplayInfo (DisplayInfo * newdisplayInfo)
{
  DisplayInfo * displayInfo = displayInfoListHead;

  while (displayInfo != NULL) 
  {
    if (newdisplayInfo != displayInfo 
	&&
	isIdenticalDisplayInfos (newdisplayInfo, displayInfo))
      break;

    displayInfo = displayInfo->next;
  }

  return displayInfo;
}

DisplayInfo * lookupIdenticalDisplayInfo2 
  (DisplayInfo * displayInfoListHead,
   DisplayInfo * newdisplayInfo)
{
  DisplayInfo * displayInfo = displayInfoListHead;

  while (displayInfo != NULL) 
  {
    if (newdisplayInfo != displayInfo 
	&&
	isIdenticalDisplayInfos (newdisplayInfo, displayInfo))
      break;

    displayInfo = displayInfo->next;
  }

  return displayInfo;
}


DisplayInfo * choiseDisplayInfo (Cursor cursor)
{
  XEvent        event;
  Widget        widget;
  DisplayInfo * displayInfo = NULL;

  if (!displayInfoListHead) 
    return NULL;

  if (displayInfoListHead != displayInfoListTail) 
    {
      /* more than one display, query user 
       */
      widget = XmTrackingEvent(mainShell, cursor, False, &event);
      
      if (widget) 
	displayInfo = dmGetDisplayInfoFromWidget(widget);
    } 
  else 
    {
      /* only one display */
      displayInfo = displayInfoListHead;
    }
  
  return displayInfo;
}

char * dm2kMalloc(size_t size)
{
  char * alc;

  if (size == 0)
    size = sizeof(double);
  
  alc = (char*) malloc (size);

  if (alc == NULL)
    return NULL;

  return memset (alc, 0, size);
}


size_t dm2kStrlen(const char * a)
{
  if (a == NULL)
    return (size_t) 0;

  return strlen(a);
}

char * dm2kStrdup(const char * a)
{
  char * copy;

  if (a == NULL)
    return NULL;

  /* ! Safer? 5 instead of 1 */
  copy = malloc (strlen(a) + 5);
  if (copy == NULL)
    return NULL;

  return strcpy(copy, a);
}

/*
 * it make s a copy of string ``from'', and saves in ``to''
 */ 
void renewString(char ** to, char * from)
{
  if (to == NULL) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  if (*to == NULL) {
    if (from == NULL) {
      /* EMPTY */;
    }
    else {
      *to =  STRDUP(from);
    }
  }
  else {
    if (*to == from) {
      *to = (char*) realloc (from, STRLEN(from) + 1);
    }
    else if (from == NULL) 
    {
      free(*to);
      *to = NULL;
    }
    else 
    {
      if (strlen(*to) == strlen(from)) {
	strcpy(*to, from);
      } else {
	free(*to);
	*to = STRDUP(from);
      }
    }
  }
}

/* ---------------------------------------------------- */

static void freeMacros(char ** ar, int count)
{
  int i;

  for (i = 0; i < count; i++)
    DM2KFREE(ar[i]);

  DM2KFREE(ar);
}


static int isUniqueMacro(const char * checkIt, const char ** ar, int count)
{
  int i;

  for (i = 0; i < count; i++)
    if (STREQL(checkIt, ar[i]))
      return 0;

  return 1;
}

int getAllMacros(const char * fileName, char *** result)
{
  FILE  * stream;
  char    buffer[MAX_TOKEN_LENGTH];
  char  * expbuf; 
  int     n = 0;

  if (result == NULL) 
    return 0;

  *result = NULL;

  if (fileName == NULL) 
    return 0;

  stream = dmOpenUseableFile(fileName);
  if (stream == NULL)
    return 0;

  *result = NULL;

#define EXPBUF_SIZE 1024
  if ( !(expbuf = malloc( EXPBUF_SIZE ))) return 0;
  /* $(...) is checked for 
   */
  compile ("\\$([^)]*", expbuf, expbuf+EXPBUF_SIZE
#if defined(__hpux) || defined(__linux__)
	   , '\0'
#endif
); 

  while (fgets(buffer, MAX_TOKEN_LENGTH, stream)) {
    char * iterator = buffer;

#ifdef __linux__
	/* T. Straumann: 'locs' is not implemented in gnu libc5
	 *				 (and not POSIX -- folks, please stick to the standard)
	 *				 I hope we may silently ignore 'locs' feature.
	 */
	char *locs;
#endif
    locs = 0;
    while(step(iterator, expbuf)) {
      char tmp[MAX_TOKEN_LENGTH+1];
      int len = MIN(loc2-loc1, MAX_TOKEN_LENGTH);

      strncpy(tmp, loc1, len);
      tmp[len] = '\0';

      if (isUniqueMacro(&tmp[2], (const char **)*result, n)) {
	char  * newMacro;

	newMacro = STRDUP(&tmp[2]); /* skip "$(" */
	if (newMacro == NULL) {
	  freeMacros(*result, n);
	  *result = NULL;
	  
	  fclose(stream);
	  DM2KFREE(expbuf);
	  return 0;
	}

	n++;
	REALLOC(char*, (*result), n);
	if (*result == NULL) {
	  fclose(stream);
	  DM2KFREE(expbuf);
	  return 0;
	}

	(*result)[n-1] = newMacro;
      }

      locs = iterator = loc2; 
    }
  }

  fclose(stream);
  DM2KFREE(expbuf);

  return n;
}
