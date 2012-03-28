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
 * .04  12-01-95        vong    clean up the textUpdateDraw routine
 *
 *****************************************************************************
 *
 *      18-03-97        Fabien  Add object info
 *
 *****************************************************************************
*/

#include "dm2k.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <cvtFast.h>
#ifdef __cplusplus
}
#endif

typedef struct _TextUpdate {
  DlElement     *dlElement;
  Record        *record;
  UpdateTask    *updateTask;
  int           fontIndex;
} TextUpdate;

static void textUpdateUpdateValueCb(XtPointer cd);
static void textUpdateDraw(XtPointer cd);
static void textUpdateDestroyCb(XtPointer cd);
static void textUpdateName(XtPointer, char **, short *, int *);
static void textUpdateInheritValues(ResourceBundle *pRCB, DlElement *p);
static void textUpdateGetValues(ResourceBundle *pRCB, DlElement *p);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);
static void destroyDlElement(DlElement *dlElement) ;

static DlDispatchTable textUpdateDlDispatchTable = {
       createDlTextUpdate,
       destroyDlElement,
       executeMethod,
       writeDlTextUpdate,
       NULL,
       textUpdateGetValues,
       textUpdateInheritValues,
       NULL,
       NULL,
       genericMove,
       genericScale,
       NULL,
       genericObjectInfo
};

static void destroyDlTextUpdate(register  DlTextUpdate *dlTextUpdate)
{
  if (dlTextUpdate == NULL)
    return;

  objectAttributeDestroy(&(dlTextUpdate->object));
  monitorAttributeDestroy(&(dlTextUpdate->monitor));

  free ((char *)dlTextUpdate);
}

static void destroyDlElement(DlElement *dlElement) 
{
   destroyDlTextUpdate(dlElement->structure.textUpdate);
   free ((char *)dlElement);
}

static UpdateTask * executeMethod
   (DisplayInfo * displayInfo, 
    DlElement   * dlElement)
{
  XRectangle clipRect[1];
  int usedHeight, usedWidth, availWidth, availHeight, x, y;
  int localFontIndex;
  size_t nChars;
  DlTextUpdate *dlTextUpdate = dlElement->structure.textUpdate;
  UpdateTask * updateTask = NULL;

  if (displayInfo->traversalMode == DL_EXECUTE) {
    TextUpdate *ptu;
    ptu = DM2KALLOC(TextUpdate);
    if (ptu == NULL) {
      dm2kPrintf("UpdateTask: memory allocation error\n");
      return updateTask;
    }

    ptu->dlElement = dlElement;
    dlTextUpdate->object.runtimeDescriptor = (XtPointer) ptu;

    updateTask = ptu->updateTask = updateTaskAddTask(displayInfo,
						     &(dlTextUpdate->object),
						     textUpdateDraw,
						     (XtPointer)ptu);

    if (ptu->updateTask == NULL) {
      dm2kPrintf("textUpdateCreateRunTimeInstance : memory allocation error\n");
    } else {
      updateTaskAddDestroyCb(ptu->updateTask,textUpdateDestroyCb);
      updateTaskAddNameCb(ptu->updateTask,textUpdateName);
    }
    ptu->record = dm2kAllocateRecord(dlTextUpdate->monitor.rdbk,
				     textUpdateUpdateValueCb,
				     NULL,
				     (XtPointer) ptu);

    ptu->fontIndex = dmGetBestFontWithInfo(fontTable,
					   MAX_FONTS,DUMMY_TEXT_FIELD,
					   dlTextUpdate->object.height, dlTextUpdate->object.width,
					   &usedHeight, &usedWidth, FALSE);        /* don't use width */

    drawWhiteRectangle(ptu->updateTask);

  } else {

    /* Since no ca callbacks to put up text, put up dummy region */
#ifdef ENABLE_TRANSPARENT
     if (displayInfo->colormap[ dlTextUpdate->monitor.bclr ] !=
	 transparentPixel) {
#endif
	XSetForeground(display,displayInfo->gc,
		       displayInfo->colormap[ dlTextUpdate->monitor.bclr]);
	XFillRectangle(display, XtWindow(displayInfo->drawingArea),
		       displayInfo->gc,
		       dlTextUpdate->object.x,dlTextUpdate->object.y,
		       dlTextUpdate->object.width, dlTextUpdate->object.height);
	XFillRectangle(display,displayInfo->drawingAreaPixmap,
		       displayInfo->gc,
		       dlTextUpdate->object.x,dlTextUpdate->object.y,
		       dlTextUpdate->object.width, dlTextUpdate->object.height);
#ifdef ENABLE_TRANSPARENT
     }
#endif

    XSetForeground(display,displayInfo->gc,
		   displayInfo->colormap[dlTextUpdate->monitor.clr]);
#ifdef ENABLE_TRANSPARENT
    if (displayInfo->colormap[ dlTextUpdate->monitor.bclr] != transparentPixel) {
#endif
       XSetBackground(display,displayInfo->gc,
		      displayInfo->colormap[dlTextUpdate->monitor.bclr]);
#ifdef ENABLE_TRANSPARENT
    }
#endif
    
    nChars = STRLEN(dlTextUpdate->monitor.rdbk);
    localFontIndex = 
      dmGetBestFontWithInfo(fontTable,
			    MAX_FONTS,dlTextUpdate->monitor.rdbk,
			    dlTextUpdate->object.height, 
			    dlTextUpdate->object.width, 
			    &usedHeight, &usedWidth, 
			    FALSE);	/* don't use width */
    usedWidth = XTextWidth(fontTable[localFontIndex],
			   dlTextUpdate->monitor.rdbk,
			   nChars);

    /* clip to bounding box (especially for text) */
    clipRect[0].x = dlTextUpdate->object.x;
    clipRect[0].y = dlTextUpdate->object.y;
    clipRect[0].width  = dlTextUpdate->object.width;
    clipRect[0].height =  dlTextUpdate->object.height;
    XSetClipRectangles(display,displayInfo->gc,0,0,clipRect,1,YXBanded);

    XSetFont(display,displayInfo->gc,fontTable[localFontIndex]->fid);

    availWidth  = dlTextUpdate->object.width - usedWidth;
    availHeight = dlTextUpdate->object.height - fontTable[localFontIndex]->ascent;

    x = dlTextUpdate->object.x + availWidth / 2;
    y = dlTextUpdate->object.y + availHeight / 2;

    switch (dlTextUpdate->alignment) {
    case ALIGNMENT_NW:
    case ALIGNMENT_N:
    case ALIGNMENT_NE:
       y = dlTextUpdate->object.y;
       break;
       
    case ALIGNMENT_SW:
    case ALIGNMENT_S:
    case ALIGNMENT_SE:
       y = dlTextUpdate->object.y + availHeight;
       break;
    default:
       break;
    }
    
    switch (dlTextUpdate->alignment) {
    case ALIGNMENT_NW:
    case ALIGNMENT_W:
    case ALIGNMENT_SW:
       x = dlTextUpdate->object.x;
     break;
     
    case ALIGNMENT_NE:
    case ALIGNMENT_E:
    case ALIGNMENT_SE:
       x = dlTextUpdate->object.x + availWidth;
       break;
    default:
       break;
    }

    XDrawString(display,displayInfo->drawingAreaPixmap,
		displayInfo->gc,
		x, y + fontTable[localFontIndex]->ascent,
		dlTextUpdate->monitor.rdbk,STRLEN(dlTextUpdate->monitor.rdbk));
    XDrawString(display,XtWindow(displayInfo->drawingArea),
		displayInfo->gc,
		x, y + fontTable[localFontIndex]->ascent,
		dlTextUpdate->monitor.rdbk,STRLEN(dlTextUpdate->monitor.rdbk));

    /* and turn off clipping on exit */
    XSetClipMask(display,displayInfo->gc,None);
  }

  return updateTask;
}


static void textUpdateDestroyCb(XtPointer cd) {
  TextUpdate *ptu = (TextUpdate *) cd;
  if (ptu) {
    dm2kDestroyRecord(ptu->record);
    free((char *)ptu);
  }
  return;
}

static void textUpdateUpdateValueCb(XtPointer cd) {
  Record *pd = (Record *) cd;
  TextUpdate *ptu = (TextUpdate *) pd->clientData;
  updateTaskMarkUpdate(ptu->updateTask);
}

static void textUpdateDraw(XtPointer cd) {
  TextUpdate *ptu = (TextUpdate *) cd;
  Record *pd = (Record *) ptu->record;
  DlTextUpdate *dlTextUpdate = ptu->dlElement->structure.textUpdate;
  DisplayInfo *displayInfo = ptu->updateTask->displayInfo;
  Display *display = XtDisplay(displayInfo->drawingArea);
  char textField[MAX_TOKEN_LENGTH];
  int i;
  XGCValues gcValues;
  unsigned long gcValueMask;
  Boolean isNumber;
  double value = 0.0;
  int precision = 0;
  int textWidth = 0;
  int strLen = 0;

  if (pd && pd->connected) {
     SET_IF_NOT_MAXFLOAT(pd->lopr,dlTextUpdate->override.displayLowLimit);
     SET_IF_NOT_MAXFLOAT(pd->hopr,dlTextUpdate->override.displayHighLimit);
     SET_IF_NOT_MINUSONE(pd->precision,dlTextUpdate->override.displayPrecision);

    if (pd->readAccess) {
      textField[0] = '\0';
      switch (pd->dataType) {
      case DBF_STRING :
	if (pd->array) {
	  strncpy(textField,(const char *)pd->array, 
		  MAX_TEXT_UPDATE_WIDTH-1);
	  textField[MAX_TEXT_UPDATE_WIDTH-1] = '\0';
	}
	isNumber = False;
	break;
      case DBF_ENUM :
	if (pd->precision >= 0 && pd->hopr+1 > 0) {
	  i = (int) pd->value;
	  /* T. Straumann: I'm Mr. Paranoia */
	  if (i >= 0 && i < (int) pd->hopr+1 && pd->stateStrings[i]){
	    strncpy(textField, (const char *)pd->stateStrings[i], 
		    MAX_TEXT_UPDATE_WIDTH-1);
	    textField[MAX_TEXT_UPDATE_WIDTH-1] = '\0';
	  } else {
	    textField[0] = ' '; textField[1] = '\0';
	  }
	  isNumber = False;
	} else {
	  value = pd->value;
	  isNumber = True;
	}
	break;
      case DBF_CHAR :
	if (dlTextUpdate->format == STRING) {
	  if (pd->array) {
	    strncpy(textField,(const char *)pd->array,
		    MIN(pd->elementCount,(MAX_TOKEN_LENGTH-1)));
	    textField[MAX_TOKEN_LENGTH-1] = '\0';
	  }
	  isNumber = False;
	  break;
	}
      case DBF_INT :
      case DBF_LONG :
      case DBF_FLOAT :
      case DBF_DOUBLE :
	value = pd->value;
	SET_IF_NOT_MINUSONE(pd->precision,dlTextUpdate->override.displayPrecision);
	precision = pd->precision;
	if (precision < 0) precision = 0;
	isNumber = True;
	break;
      default :
	dm2kPrintf("textUpdateUpdateValueCb: %s %s %s\n",
		   "unknown channel type for",dlTextUpdate->monitor.rdbk, ": cannot attach TextUpdate");
	dm2kPostTime();
	break;
      }
      if (isNumber) {
        switch (dlTextUpdate->format) {
	case DECIMAL:
	case STRING:
	  cvtDoubleToString(value,textField,precision);
	  break;
	case EXPONENTIAL:
#if 0
	  cvtDoubleToExpString(value,textField,precision);
#endif
          if (isnan(value)) {
            sprintf(textField,"---");
          } else {
            sprintf(textField,"%.*e",precision,value);
          }
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
	  dm2kPrintf("textUpdateUpdateValueCb: %s %s %s\n",
		     "unknown channel type for",dlTextUpdate->monitor.rdbk, ": cannot attach TextUpdate");
	  dm2kPostTime();
	  break;
        }
      }

#ifdef ENABLE_TRANSPARENT
      if ( displayInfo->colormap[dlTextUpdate->monitor.bclr] != transparentPixel ) {	 
#endif
	 XSetForeground(display,displayInfo->gc, displayInfo->colormap[dlTextUpdate->monitor.bclr]);
	 XFillRectangle(display, XtWindow(displayInfo->drawingArea),
			displayInfo->gc,
			dlTextUpdate->object.x,dlTextUpdate->object.y,
			dlTextUpdate->object.width,
			dlTextUpdate->object.height);
#ifdef ENABLE_TRANSPARENT
      }
#endif

      /* calculate the color */
      gcValueMask = GCForeground | GCBackground;
      switch (dlTextUpdate->clrmod) {
      case STATIC :
      case DISCRETE:
	gcValues.foreground = displayInfo->colormap[dlTextUpdate->monitor.clr];
	break;
      case ALARM :
	gcValues.foreground =  alarmColorPixel[pd->severity];
	break;
      }
      gcValues.background = displayInfo->colormap[dlTextUpdate->monitor.bclr];
      XChangeGC(display,displayInfo->gc, gcValueMask,&gcValues);

      i = ptu->fontIndex;
      strLen = STRLEN(textField);
      textWidth = XTextWidth(fontTable[i],textField,strLen);

      /* for compatibility reason, only the HORIZ_CENTER,
       * HORIZ_RIGHT, VERT_BOTTOM and VERT_CENTER
       * will recalculate the font size if the number does
       * not fit. */
      if (dlTextUpdate->object.width  < textWidth) {
        switch(dlTextUpdate->alignment) {
	case ALIGNMENT_N:
	case ALIGNMENT_C:
	case ALIGNMENT_S:
	case ALIGNMENT_NE:
	case ALIGNMENT_E:
	case ALIGNMENT_SE:
	  while (i > 0) {
	    i--;
	    textWidth = XTextWidth(fontTable[i],textField,strLen);
	    if (dlTextUpdate->object.width > textWidth) break;
	  }
	  break;
	default :
	  break;
        }
      }

      /* print text */
      {
        int x, y, availWidth, availHeight;
        XSetFont(display,displayInfo->gc,fontTable[i]->fid);

	availWidth  = dlTextUpdate->object.width - textWidth;
	availHeight = dlTextUpdate->object.height - fontTable[i]->ascent;

	x = dlTextUpdate->object.x + availWidth / 2;
	y = dlTextUpdate->object.y + availHeight / 2;
	
	switch (dlTextUpdate->alignment) {
	case ALIGNMENT_NW:
	case ALIGNMENT_N:
	case ALIGNMENT_NE:
	   y = dlTextUpdate->object.y;
	   break;
	   
	case ALIGNMENT_SW:
	case ALIGNMENT_S:
	case ALIGNMENT_SE:
	   y = dlTextUpdate->object.y + availHeight;
	   break;
	default:
	   break;
	}
	
	switch (dlTextUpdate->alignment) {
	case ALIGNMENT_NW:
	case ALIGNMENT_W:
	case ALIGNMENT_SW:
	   x = dlTextUpdate->object.x;
	   break;
	   
	case ALIGNMENT_NE:
	case ALIGNMENT_E:
	case ALIGNMENT_SE:
	   x = dlTextUpdate->object.x + availWidth;
	   break;
	default:
	   break;
	}
        XDrawString(display,XtWindow(displayInfo->drawingArea),
		    displayInfo->gc, x, y + fontTable[i]->ascent,
		    textField,strLen);
      }
    } else {
      /* no read access */
      draw3DPane(ptu->updateTask,
		 ptu->updateTask->displayInfo->colormap[dlTextUpdate->monitor.bclr]);
      draw3DQuestionMark(ptu->updateTask);
    }
  } else {
    /* no connection or disconnected */
    drawWhiteRectangle(ptu->updateTask);
  }
}

static void textUpdateName(XtPointer cd, char **name, short *severity, int *count) {
  TextUpdate *pa = (TextUpdate *) cd;
  *count = 1;
  name[0] = pa->record->name;
  severity[0] = pa->record->severity;
}

DlElement *createDlTextUpdate(DlElement *p)
{
  DlTextUpdate *dlTextUpdate;
  DlElement *dlElement;

  dlTextUpdate = DM2KALLOC(DlTextUpdate);

  if (dlTextUpdate == NULL)
    return 0;

  if (p != NULL) {
    objectAttributeCopy(&(dlTextUpdate->object), &(p->structure.textUpdate->object));
    monitorAttributeCopy(&(dlTextUpdate->monitor), &(p->structure.textUpdate->monitor));
    overrideAttributeCopy(&(dlTextUpdate->override), &(p->structure.textUpdate->override));

    dlTextUpdate->clrmod = p->structure.textUpdate->clrmod;
    dlTextUpdate->alignment = p->structure.textUpdate->alignment;
    dlTextUpdate->format = p->structure.textUpdate->format;
  } 
  else {
    objectAttributeInit(&(dlTextUpdate->object));
    monitorAttributeInit(&(dlTextUpdate->monitor));
    overrideAttributeInit(&(dlTextUpdate->override));

    dlTextUpdate->clrmod = STATIC;
    dlTextUpdate->alignment = ALIGNMENT_NW;
    dlTextUpdate->format = DECIMAL;
  }

  dlElement = createDlElement(DL_TextUpdate, (XtPointer) dlTextUpdate,
			      &textUpdateDlDispatchTable);

  if (dlElement == NULL)
    destroyDlTextUpdate(dlTextUpdate);

  return(dlElement);
}

DlElement *parseTextUpdate(DisplayInfo *displayInfo)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;
  DlTextUpdate *dlTextUpdate;
  DlElement *dlElement = createDlTextUpdate(NULL);
  int i= 0;

  if (!dlElement) return 0;
  dlTextUpdate = dlElement->structure.textUpdate;


  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"object")) {
	parseObject(displayInfo,&(dlTextUpdate->object));
      } else if (STREQL(token,"monitor")) {
	parseMonitor(displayInfo,&(dlTextUpdate->monitor));
      } else if (STREQL(token,"override")) {
	parseOverride(displayInfo,&(dlTextUpdate->override));
      } else if (STREQL(token,"clrmod")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	for (i=FIRST_COLOR_MODE;i<FIRST_COLOR_MODE+NUM_COLOR_MODES;i++) {
	  if (STREQL(token,stringValueTable[i])) {
	    dlTextUpdate->clrmod = (ColorMode)i;
	    break;
	  }
	}
      } else if (STREQL(token,"format")) {
	int found = 0;
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	for (i=FIRST_TEXT_FORMAT;i<FIRST_TEXT_FORMAT+NUM_TEXT_FORMATS; i++) {
	  if (STREQL(token,stringValueTable[i])) {
	    dlTextUpdate->format = (TextFormat)i;
	    found = 1;
	    break;
	  }
	}
	if (found) {
	  break;
	} else
	  /* if not found, do the backward compatibility test */
	  if (STREQL(token,"decimal")) {
	    dlTextUpdate->format = DECIMAL;
	  } else if (STREQL(token,
			     "decimal- exponential notation")) {
	    dlTextUpdate->format = EXPONENTIAL;
	  } else if (STREQL(token,"engr. notation")) {
	    dlTextUpdate->format = ENGR_NOTATION;
	  } else if (STREQL(token,"decimal- compact")) {
	    dlTextUpdate->format = COMPACT;
	  } else if (STREQL(token,"decimal- truncated")) {
	    dlTextUpdate->format = TRUNCATED;
	    /* (MDA) allow for LANL spelling errors {like above, but with trailing space} */
	  } else if (STREQL(token,"decimal- truncated ")) {
	    dlTextUpdate->format = TRUNCATED;
	    /* (MDA) allow for LANL spelling errors {hexidecimal vs. hexadecimal} */
	  } else if (STREQL(token,"hexidecimal")) {
	    dlTextUpdate->format = HEXADECIMAL;
	  }
      } else if (STREQL(token,"alignment")) {
	 getToken(displayInfo,token);
	 getToken(displayInfo,token);

	 for (i = FIRST_ALIGNMENT; i < FIRST_ALIGNMENT+NUM_ALIGNMENTS; i++)
	 {
            if (STREQL(token,stringValueTable[i])) {
	       dlTextUpdate->alignment = (Alignment)i;
	       break;
            }
	 }
      }
      else if (STREQL(token,"align")) 
      {
	 getToken(displayInfo,token);
	 getToken(displayInfo,token);
	 
	 for (i = FIRST_TEXT_ALIGN; i < FIRST_TEXT_ALIGN+NUM_TEXT_ALIGNS; i++)
	 {
	    if (STREQL(token,stringValueTable[i])) {
	       dlTextUpdate->alignment = alignmentTranslation[i-FIRST_TEXT_ALIGN];
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

void writeDlTextUpdate(FILE *stream, DlElement *dlElement, int level) {
  char indent[256]; level=MIN(level,256-2);
  DlTextUpdate *dlTextUpdate = dlElement->structure.textUpdate;

  level=MIN(level,256-2);
  memset(indent,'\t',level);
  indent[level] = '\0';


#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
    fprintf(stream,"\n%s\"text update\" {",indent);
    writeDlObject(stream,&(dlTextUpdate->object),level+1);
    writeDlMonitor(stream,&(dlTextUpdate->monitor),level+1);
    writeDlOverride(stream,&(dlTextUpdate->override),level+1);

    if (dlTextUpdate->clrmod != STATIC) 
      fprintf(stream,"\n%s\tclrmod=\"%s\"",indent,
	      stringValueTable[dlTextUpdate->clrmod]);

    if (dlTextUpdate->alignment != ALIGNMENT_NW)
      fprintf(stream,"\n%s\talignment=\"%s\"",indent,
	      stringValueTable[dlTextUpdate->alignment]);

    if (dlTextUpdate->format != DECIMAL)
      fprintf(stream,"\n%s\tformat=\"%s\"",indent,
	      stringValueTable[dlTextUpdate->format]);
    fprintf(stream,"\n%s}",indent);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    fprintf(stream,"\n%s\"text update\" {",indent);
    writeDlObject(stream,&(dlTextUpdate->object),level+1);
    writeDlMonitor(stream,&(dlTextUpdate->monitor),level+1);

    fprintf(stream,"\n%s\tclrmod=\"%s\"",indent,
	    stringValueTable[dlTextUpdate->clrmod]);
    fprintf(stream,"\n%s\talignment=\"%s\"",indent,
	    stringValueTable[dlTextUpdate->alignment]);
    fprintf(stream,"\n%s\tformat=\"%s\"",indent,
	    stringValueTable[dlTextUpdate->format]);
    fprintf(stream,"\n%s}",indent);
  }
#endif
}

static void textUpdateGetValues(ResourceBundle *pRCB, DlElement *p) {
  DlTextUpdate *dlTextUpdate = p->structure.textUpdate;
  dm2kGetValues(pRCB,
		X_RC,          &(dlTextUpdate->object.x),
		Y_RC,          &(dlTextUpdate->object.y),
		WIDTH_RC,      &(dlTextUpdate->object.width),
		HEIGHT_RC,     &(dlTextUpdate->object.height),
		CTRL_RC,       &(dlTextUpdate->monitor.rdbk),
		CLR_RC,        &(dlTextUpdate->monitor.clr),
		BCLR_RC,       &(dlTextUpdate->monitor.bclr),
		CLRMOD_RC,     &(dlTextUpdate->clrmod),
		ALIGNMENT_RC,  &(dlTextUpdate->alignment),
		FORMAT_RC,     &(dlTextUpdate->format),
		LOW_LIMIT_RC,  &(dlTextUpdate->override.displayLowLimit),
		HIGH_LIMIT_RC, &(dlTextUpdate->override.displayHighLimit),
		VAL_PRECISION_RC, &(dlTextUpdate->override.displayPrecision),
		-1);
}

static void textUpdateInheritValues(ResourceBundle *pRCB, DlElement *p) {
  DlTextUpdate *dlTextUpdate = p->structure.textUpdate;
  dm2kGetValues(pRCB,
		CTRL_RC,       &(dlTextUpdate->monitor.rdbk),
		CLR_RC,        &(dlTextUpdate->monitor.clr),
		BCLR_RC,       &(dlTextUpdate->monitor.bclr),
		CLRMOD_RC,     &(dlTextUpdate->clrmod),
		ALIGNMENT_RC,  &(dlTextUpdate->alignment),
		FORMAT_RC,     &(dlTextUpdate->format),
		LOW_LIMIT_RC,  &(dlTextUpdate->override.displayLowLimit),
		HIGH_LIMIT_RC, &(dlTextUpdate->override.displayHighLimit),
		VAL_PRECISION_RC, &(dlTextUpdate->override.displayPrecision),
		-1);
}

