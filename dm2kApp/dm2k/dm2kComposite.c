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
 *                              correct the falling line and rising line to
 *                              polyline geometry calculation
 * .03  09-08-95        vong    conform to c++ syntax
 *
 *****************************************************************************
 *
 *      24-02-97    Fabien      check for valid element type
 *
 *****************************************************************************
*/

#include "dm2k.h"

static void destroyDlElement(DlElement *dlElement);
static void compositeMove(DlElement *element, int xOffset, int yOffset);
static void compositeScale(DlElement *element, int xOffset, int yOffset);
static void compositeGetValues(ResourceBundle *pRCB, DlElement *p);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);

static DlDispatchTable compositeDlDispatchTable = {
         createDlComposite,
         destroyDlElement,
         executeMethod,
         writeDlComposite,
         NULL,
         compositeGetValues,
         NULL,
         NULL,
         NULL,
         compositeMove,
         compositeScale,
         NULL
};


static void destroyDlComposite(DlComposite * dlComposite) 
{
  if (dlComposite == NULL)
    return;

  DM2KFREE(dlComposite->compositeName);
  DM2KFREE(dlComposite->chan);
  destroyDlDisplayList(dlComposite->dlElementList);
  DM2KFREE(dlComposite->dlElementList);
  deleteAMIList(dlComposite->ami);

  free((char *)dlComposite);
}

static void destroyDlElement(DlElement *dlElement) 
{
  if (dlElement == NULL || dlElement->type != DL_Composite) {
    INFORM_INTERNAL_ERROR();
    return;
  }

  destroyDlComposite(dlElement->structure.composite);

  free((char *) dlElement);
}

UpdateTask ** globalUpdateTasks = NULL;
int           globalUpdateTasksNum = 0;

static UpdateTask * executeMethod
  (DisplayInfo * displayInfo, 
   DlElement   * dlElement)
{
  extern Boolean silentFlag;

  DlComposite *dlComposite = dlElement->structure.composite;
  DlElement *element;
  UpdateTask * updateTask = NULL;

  if (displayInfo->traversalMode == DL_EDIT) {
    element = FirstDlElement(dlComposite->dlElementList);
    while (element) {
      if ( checkControllerObject (displayInfo, element) ) {
	(*element->run->execute)(displayInfo, element);
      }
      else {
	invalidObjectWarning (displayInfo, element->type);
      }
      element = element->next;
    }
  } 
  else if (displayInfo->traversalMode == DL_EXECUTE) {
    if (dlComposite->visible) {
      element = FirstDlElement(dlComposite->dlElementList);
      while (element) {
	if ( checkControllerObject (displayInfo, element) ) {
	  UpdateTask * ut;
	  element->displayInfo = displayInfo;
	  ut = (*element->run->execute)(displayInfo, element);
	  
	  if (ut != NULL) {
	    globalUpdateTasksNum++;
	    REALLOC(UpdateTask *, globalUpdateTasks, globalUpdateTasksNum);
	    
	    if (globalUpdateTasks != NULL)
	      globalUpdateTasks[globalUpdateTasksNum-1] = ut;
	    else
	      globalUpdateTasksNum = 0;
	  }
	}
	else {
	  if ( element->widget ) {
	    XtDestroyWidget (element->widget);
	    element->widget = NULL;
	  }
	  if ( !silentFlag ) {
	    fprintf(stderr, 
		    "\n<== Invalid Element type in Composite (%d) "
		    "for this type of display (%s) ==>\n",
		    element->type, stringValueTable[displayInfo->displayType]);

	    fprintf(stderr, 
		    "    Display file : %s\n     -- Object ignored --\n", 
		    displayInfo->dlFile->name);
	  }
	}
        element = element->next;
      }
    }
  }
  return updateTask;
}

DlElement *createDlComposite(DlElement *p) 
{
  DlComposite *dlComposite;
  DlElement *dlElement;

  dlComposite = DM2KALLOC(DlComposite);
  if (dlComposite == NULL) 
    return NULL;

  if (p != NULL) {
    DlElement *child;

    objectAttributeCopy(&(dlComposite->object), 
			&(p->structure.composite->object));

    dlComposite->vis     = p->structure.composite->vis;
    dlComposite->visible = p->structure.composite->visible;

    renewString(&dlComposite->chan, p->structure.composite->chan);
    renewString(&dlComposite->compositeName,
		p->structure.composite->compositeName);

    dlComposite->ami = copyAMIList(p->structure.composite->ami);

    dlComposite->dlElementList = createDlList();
    if (dlComposite->dlElementList == NULL) {
       destroyDlComposite(dlComposite);
      return NULL;
    }

    /* copy all children */
    child = FirstDlElement(p->structure.composite->dlElementList);
    while (child) {
      DlElement *copy = (*child->run->create)(child);
      if (copy) 
        appendDlElement(dlComposite->dlElementList,copy);
      child = child->next;
    }
  } else {
    objectAttributeInit(&(dlComposite->object));

    dlComposite->compositeName    = NULL;
    dlComposite->vis              = V_STATIC;
    dlComposite->chan             = NULL;
    dlComposite->ami              = NULL;
    dlComposite->visible          = True;

    dlComposite->dlElementList    = createDlList();
    if (dlComposite->dlElementList == NULL) {
      destroyDlComposite(dlComposite);
      return NULL;
    }
  }

  dlElement = createDlElement(DL_Composite,
			      (XtPointer) dlComposite,
			      &compositeDlDispatchTable);

  if (dlElement == NULL) 
    destroyDlComposite(dlComposite);

  return dlElement;
}

DlElement * groupObjects(DisplayInfo *displayInfo)
{
  DlComposite *dlComposite;
  DlElement *dlElement, *elementPtr;
  int minX, minY, maxX, maxY;

  /* if there is no element selected, return */
  if (IsEmpty(displayInfo->selectedDlElementList)) return 0;

  if (!(dlElement = createDlComposite(NULL))) return 0;
  appendDlElement(displayInfo->dlElementList,dlElement);
  dlComposite = dlElement->structure.composite;

/*
 *  now loop over all selected elements and and determine x/y/width/height
 *    of the newly created composite and insert the element.
 */
  minX = INT_MAX; minY = INT_MAX;
  maxX = INT_MIN; maxY = INT_MIN;

  elementPtr = FirstDlElement(displayInfo->selectedDlElementList);
  while (elementPtr) { 
    DlElement *pE = elementPtr->structure.element;
    if (pE->type != DL_Display) {
      DlObject *po = &(pE->structure.rectangle->object);
      minX = MIN(minX,po->x);
      maxX = MAX(maxX,(int)(po->x+po->width));
      minY = MIN(minY,po->y);
      maxY = MAX(maxY,(int)(po->y+po->height));
      removeDlElement(displayInfo->dlElementList,pE);
      appendDlElement(dlComposite->dlElementList,pE);
    }
    elementPtr = elementPtr->next;
  }

  dlComposite->object.x = minX;
  dlComposite->object.y = minY;
  dlComposite->object.width = maxX - minX;
  dlComposite->object.height = maxY - minY;

  clearResourcePaletteEntries();
  unhighlightSelectedElements();
  destroyDlDisplayList(displayInfo->selectedDlElementList);
  if (!(elementPtr = createDlElement(DL_Element,NULL,NULL))) {
    return 0;
  }
  elementPtr->structure.element = dlElement;
  appendDlElement(displayInfo->selectedDlElementList,elementPtr);
  highlightSelectedElements();
  currentActionType = SELECT_ACTION;
  currentElementType = DL_Composite;
  setResourcePaletteEntries();

  return(dlElement);
}

static void parseAMI(DisplayInfo        *displayInfo, 
		     AssociatedMenuItem *ami)
{
  char  token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int   nestingLevel = 0;
  extern char *actionTypeLabels[];

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"label")) 
      {
        getToken(displayInfo,token);
        getToken(displayInfo,token);
	if (STRLEN(token) > (size_t)0)
	  ami->label = STRDUP(token);
      } 
      else if (STREQL(token,"type")) 
      {
	register int i;

        getToken(displayInfo,token);
        getToken(displayInfo,token);
	
	ami->actionType = 0;
	for (i = 0; i < 3 /* XtNumber(actionTypeLabels)*/; i++) {
	  if (STREQL(token, actionTypeLabels[i])) {
	    ami->actionType = i;
	    break;
	  }
	}
      } 
      else if (STREQL(token,"command")) 
      {
        getToken(displayInfo,token);
        getToken(displayInfo,token);
	if (STRLEN(token) > (size_t)0)
	  ami->command = STRDUP(token);
      } 
      else if (STREQL(token,"args")) 
      {
        getToken(displayInfo,token);
        getToken(displayInfo,token);
	if (STRLEN(token) > (size_t)0)
	  ami->args = STRDUP(token);
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


DlElement *parseComposite(DisplayInfo *displayInfo)
{
  char               token[MAX_TOKEN_LENGTH];
  TOKEN              tokenType;
  int                nestingLevel = 0;
  DlComposite        *newDlComposite;
  DlElement          *dlElement = createDlComposite(NULL);
  AssociatedMenuItem *ami = NULL; /* last added ami */

  if (!dlElement) return 0;
  newDlComposite = dlElement->structure.composite;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"object")) 
      {
	parseObject(displayInfo,&(newDlComposite->object));
      } 
      else if (STREQL(token,"composite name"))
      {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	renewString(&newDlComposite->compositeName,token);
      } 
      else if (STREQL(token,"vis")) 
      {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	if (STREQL(token,"static"))	
	  newDlComposite->vis = V_STATIC;
	else if (STREQL(token,"if not zero")) 
	  newDlComposite->vis = IF_NOT_ZERO;
	else if (STREQL(token,"if zero"))
	  newDlComposite->vis = IF_ZERO;
      } 
      else if (STREQL(token,"chan")) 
      {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	renewString(&newDlComposite->chan,token);
      } 
      else if (STREQL(token,"children"))
      {
	parseAndAppendDisplayList(displayInfo, newDlComposite->dlElementList);
      } 
      else if (STREQL(token,"menuItem")) 
      { 
	 AssociatedMenuItem * newItem = DM2KALLOC(AssociatedMenuItem);

	 if (newItem) 
	   {
	     parseAMI(displayInfo, newItem);
	     
	     newItem->prev = ami;
	     newItem->next = NULL;
	     
	     if (ami) 
	       ami->next = newItem;
	     else	
	       newDlComposite->ami = newItem;
	     
	     ami = newItem;
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

static void writeDlCompositeChildren
  (FILE      * stream,
   DlElement * dlElement,
   int         level)
{
  int           i;
  char          indent[265];
  DlElement   * element;
  DlComposite * dlComposite = dlElement->structure.composite;

  for (i = 0; i < MIN(level, 265-1); i++) 
    indent[i] = '\t';

  indent[i] = '\0';

  fprintf(stream,"\n%schildren {",indent);

  element = FirstDlElement(dlComposite->dlElementList);
  while (element != NULL) {		/* any union member is okay here */
     (*element->run->write)(stream, element, level+1);
     element = element->next;
  }

  fprintf(stream,"\n%s}",indent);
}


void writeDlComposite(
  FILE      *stream,
  DlElement *dlElement,
  int        level)
{
  char                 indent[256];
  DlComposite        * dlComposite = dlElement->structure.composite;
  AssociatedMenuItem * ami = dlComposite->ami;
  extern char        * actionTypeLabels[];

  level = MIN(level, 256-2);
  memset (indent, '\t', ++level);
  indent[level] = '\0';

  /*
   * Object specific attributes
   */  
  fprintf(stream,"\n%scomposite {",indent);
  writeDlObject(stream,&(dlComposite->object),level+1);

  /*
   * Composite attributes
   */
  if (dlComposite->compositeName != NULL)
    fprintf(stream,"\n%s\t\"composite name\"=\"%s\"",indent,
	    dlComposite->compositeName);

  fprintf(stream,"\n%s\tvis=\"%s\"",indent,
	  stringValueTable[dlComposite->vis]);

  if ((dlComposite->chan != NULL) && dlComposite->chan[0] )
    fprintf(stream,"\n%s\tchan=\"%s\"",indent,dlComposite->chan);

  /*
   * associated menu items
   */
  for (ami = dlComposite->ami; ami != NULL; ami = ami->next) {
    fprintf(stream,"\n%s\tmenuItem {",      indent);
    fprintf(stream,"\n%s\t\tlabel=\"%s\"",  indent, CARE_PRINT(ami->label));
    fprintf(stream,"\n%s\t\ttype=\"%s\"",   indent, actionTypeLabels[ami->actionType]);
    fprintf(stream,"\n%s\t\tcommand=\"%s\"",indent, CARE_PRINT(ami->command));
    fprintf(stream,"\n%s\t\targs=\"%s\"",   indent, CARE_PRINT(ami->args));
    fprintf(stream,"\n%s\t}",               indent);
  }

  /*
   * children
   */
  writeDlCompositeChildren(stream,dlElement,level+1);
  fprintf(stream,"\n%s}",indent);
}

/*
 * recursive function to resize Composite objects (and all children, which
 *  may be Composite objects)
 *  N.B.  this is relative to outermost composite, not parent composite
 */
static void compositeScale(DlElement *dlElement, int xOffset, int yOffset)
{
  int width, height;
  float scaleX = 1.0, scaleY = 1.0;

  if (dlElement->type != DL_Composite) return;
  width = MAX(1,((int)dlElement->structure.composite->object.width
                  + xOffset));
  height = MAX(1,((int)dlElement->structure.composite->object.height
                  + yOffset));
  scaleX = (float)width/(float)dlElement->structure.composite->object.width;
  scaleY = (float)height/(float)dlElement->structure.composite->object.height;
  resizeDlElementList(dlElement->structure.composite->dlElementList,
                    dlElement->structure.composite->object.x,
                    dlElement->structure.composite->object.y,
                    scaleX,
                    scaleY);
  dlElement->structure.composite->object.width = width;
  dlElement->structure.composite->object.height = height;
}

/*
 * recursive function to move Composite objects (and all children, which
 *  may be Composite objects)
 */
static void compositeMove(DlElement *dlElement, int xOffset, int yOffset)
{
  DlElement *ele;

  if (dlElement->type != DL_Composite) return; 
  ele = FirstDlElement(dlElement->structure.composite->dlElementList);
  while (ele != NULL) {
    if (ele->type != DL_Display) {
#if 0
      if (ele->widget) {
        XtMoveWidget(widget,
          (Position) (ele->structure.rectangle->object.x + xOffset),
          (Position) (ele->structure.rectangle->object.y + yOffset));
      }
#endif
      if (ele->run->move)
        (*ele->run->move)(ele,xOffset,yOffset);
    }
    ele = ele->next;
  }
  dlElement->structure.composite->object.x += xOffset;
  dlElement->structure.composite->object.y += yOffset;
}

static void compositeGetValues(ResourceBundle *pRCB, DlElement *p) 
{
  DlComposite *dlComposite = p->structure.composite;
  int          x, y;
  unsigned int width, height;
  int xOffset, yOffset;

  dm2kGetValues(pRCB,
		X_RC,            &x,
		Y_RC,            &y,
		WIDTH_RC,        &width,
		HEIGHT_RC,       &height,
		ASSOM_RC,        &dlComposite->ami,
		COMPOSITE_NAME_RC, &dlComposite->compositeName,
		CHAN_RC,         &dlComposite->chan,
		-1);

  xOffset = (int) width - (int) dlComposite->object.width;
  yOffset = (int) height - (int) dlComposite->object.height;

  if (xOffset != 0 || yOffset != 0) 
    compositeScale(p,xOffset,yOffset);

  xOffset = x - dlComposite->object.x;
  yOffset = y - dlComposite->object.y;

  if (xOffset != 0 || yOffset != 0)
    compositeMove(p,xOffset,yOffset);
}
