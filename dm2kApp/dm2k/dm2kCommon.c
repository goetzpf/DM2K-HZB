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
 * .02  09-11-95        vong    conform to c++ syntax
 *
 *****************************************************************************
*/

#include "dm2k.h"
#include <ctype.h>
#include <errno.h>

#include <X11/keysym.h>
#include <X11/IntrinsicP.h>
#include <Xm/MwmUtil.h>

void parseAttr(DisplayInfo *displayInfo, DlBasicAttribute *attr);
void parseDynamicAttr(DisplayInfo *displayInfo, DlDynamicAttribute *dynAttr);
void parseDynAttrMod(DisplayInfo *displayInfo, DlDynamicAttribute *dynAttr);
void parseDynAttrParam(DisplayInfo *displayInfo, DlDynamicAttribute *dynAttr);
void parseOldDlColor( DisplayInfo *, FILE *, DlColormapEntry *);

static DlList *dlElementFreeList = 0;
static DisplayInfo *grahicRuleDisplayInfoListHead = NULL;
static DisplayInfo *grahicRuleDisplayInfoListTail = NULL;

int initDm2kCommon() {
  if (dlElementFreeList) return 0;
  if ((dlElementFreeList = createDlList())) {
    return 0;
  } else {
    return -1;
  }
}

DlFile *createDlFile(DisplayInfo *displayInfo)
{
  DlFile * dlFile;

  dlFile = DM2KALLOC(DlFile);
  if (dlFile == NULL) 
    return 0;

  renewString(&dlFile->name, "newDisplay.adl");

  dlFile->versionNumber = DM2K_VERSION_NUMBER;

  return(dlFile);
}

DlFile *parseFile(DisplayInfo *displayInfo)
{
  char     token[MAX_TOKEN_LENGTH];
  TOKEN    tokenType;
  int      nestingLevel = 0;
  DlFile * dlFile = createDlFile(displayInfo);;

  if (dlFile == NULL) 
    return 0;

  dlFile->versionNumber = 0;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"name")) {
        getToken(displayInfo,token);
        getToken(displayInfo,token);
        renewString(&dlFile->name,token);
      }
      else if (STREQL(token,"version")) 
      {
        getToken(displayInfo,token);
        getToken(displayInfo,token);
        dlFile->versionNumber = atoi(token);
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

  return dlFile;
}

void writeDlFile(FILE   * stream,
		 DlFile * dlFile,
		 int      level)
{
  int i;
  char indent[256]; level=MIN(level,256-2);
  int versionNumber = DM2K_VERSION_NUMBER;

  for (i = 0; i < level; i++) indent[i] = '\t';
  indent[i] = '\0';

  fprintf(stream,"\n%sfile {",indent);

  if (dlFile->name != NULL)
    fprintf(stream,"\n%s\tname=\"%s\"",indent,dlFile->name);

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
    fprintf(stream,"\n%s\tversion=%06d",indent,versionNumber);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
    fprintf(stream,"\n%s\tversion=%06d",indent,20199);
  }
#endif
  fprintf(stream,"\n%s}",indent);
}

/*
 * this function gets called for executing the colormap section of
 *  each display list, for new display creation and for edit <-> execute
 *  transitions.  we could be more clever here for the edit/execute
 *  transitions and not re-execute the colormap info, but that would
 *  require changes to the cleanup code, etc...  hence let us leave
 *  this as is (since the colors are properly being freed (ref-count
 *  being decremented) and performance seems fine...
 *  -- for edit <-> execute type running, performance is not a big issue
 *  anyway and there is no additional cost incurred for the straight
 *  execute time running --
 */
void executeDlColormap(DisplayInfo *displayInfo, DlColormap *dlColormap)
{
  extern Boolean verboseFlag;

  int i;
  Dimension width, height;
  XtGCMask valueMask;
  XGCValues values;

  if (!displayInfo) return;
  /* already have a colormap - don't allow a second one! */
#if 0
  if (displayInfo->colormap) return;
#else
  if (displayInfo->colormap) free(displayInfo->colormap);
#endif

  displayInfo->colormap = (Pixel *) calloc(dlColormap->ncolors,
                                           sizeof(Pixel));
  displayInfo->dlColormapSize = dlColormap->ncolors;

  /**** allocate the X colormap from dlColormap data ****/
  for (i = 0; i < dlColormap->ncolors; i++) {
    XColor color;
    /* scale [0,255] to [0,65535] */
    color.red   = (unsigned short) COLOR_SCALE*(dlColormap->dl_color[i].r); 
    color.green = (unsigned short) COLOR_SCALE*(dlColormap->dl_color[i].g); 
    color.blue  = (unsigned short) COLOR_SCALE*(dlColormap->dl_color[i].b); 
    /* allocate a shareable color cell with closest RGB value */
    if (XAllocColor(display,cmap,&color)) {
       if ((color.red == 0) && (color.green == 0) && (color.blue == 1))
	  transparentPixel = color.pixel;
       displayInfo->colormap[displayInfo->dlColormapCounter] = color.pixel;
    } else {
       if ( verboseFlag )
	  fprintf(stderr,"\nexecuteDlColormap: couldn't allocate requested color");
       displayInfo->colormap[displayInfo->dlColormapCounter] = unphysicalPixel;
    }
    
    if (displayInfo->dlColormapCounter < displayInfo->dlColormapSize) 
      displayInfo->dlColormapCounter++;
    else
      fprintf(stderr,"\nexecuteDlColormap:  too many colormap entries");
    	/* just keep rewriting that last colormap entry */
  }
  
  if (displayInfo->drawingArea == NULL)
    return;

  /*
   * set the foreground and background of the display 
   */
  XtVaSetValues(displayInfo->drawingArea,
      XmNbackground,
      displayInfo->colormap[displayInfo->drawingAreaBackgroundColor],
      NULL);

  /* and create the drawing area pixmap */
  XtVaGetValues(displayInfo->drawingArea,
      XmNwidth,(Dimension *)&width,
      XmNheight,(Dimension *)&height,
      NULL);
  if (displayInfo->drawingAreaPixmap) {
    XFreePixmap(display,displayInfo->drawingAreaPixmap);
  }
  displayInfo->drawingAreaPixmap =
        XCreatePixmap(display, RootWindow(display,screenNum),
                      MAX(1,width),MAX(1,height),
                      DefaultDepth(display,screenNum));

  /* create the pixmap GC */
  valueMask = GCForeground | GCBackground ;
  values.foreground = 
      displayInfo->colormap[displayInfo->drawingAreaBackgroundColor];
  values.background =
      displayInfo->colormap[displayInfo->drawingAreaBackgroundColor];
  if (displayInfo->pixmapGC) {
    XFreeGC(display,displayInfo->pixmapGC);
  }
  displayInfo->pixmapGC = XCreateGC(display,
      XtWindow(displayInfo->drawingArea),valueMask,&values); 
  /* (MDA) don't generate GraphicsExpose events on XCopyArea() */
  XSetGraphicsExposures(display,displayInfo->pixmapGC,FALSE);

  XFillRectangle(display,displayInfo->drawingAreaPixmap,
                 displayInfo->pixmapGC,0,0,width,height);
  XSetForeground(display,displayInfo->pixmapGC,
      displayInfo->colormap[displayInfo->drawingAreaForegroundColor]);

/* create the initial display GC */
  valueMask = GCForeground | GCBackground ;
  values.foreground = 
      displayInfo->colormap[displayInfo->drawingAreaForegroundColor];
  values.background = 
      displayInfo->colormap[displayInfo->drawingAreaBackgroundColor];
  if (displayInfo->gc) {
    XFreeGC(display,displayInfo->gc);
  }
  displayInfo->gc = XCreateGC(display,XtWindow(displayInfo->drawingArea),
                              valueMask,&values);
}


DlColormap *createDlColormap(
  DisplayInfo *displayInfo)
{
  DlColormap *dlColormap;

  dlColormap = DM2KALLOC(DlColormap);

  if (dlColormap == NULL) 
    return 0;

  /* structure copy */
  *dlColormap = defaultDlColormap;

  return(dlColormap);
}


void parseDlColor(
  DisplayInfo *displayInfo,
  FILE *filePtr,
  DlColormapEntry *dlColor)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;
  int counter = 0;

  FILE *savedFilePtr;

/*
 * (MDA) have to be sneaky for these colormap parsing routines:
 *	since possibly external colormap, save and restore 
 *	external file ptr in  displayInfo so that getToken()
 *	works with displayInfo and not the filePtr directly
 */
  savedFilePtr = displayInfo->filePtr;
  displayInfo->filePtr = filePtr;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
      case T_WORD: {
	 if (strcmp(token, "None")) {
	    unsigned long color = (unsigned long)strtol(token,(char **)NULL,16);
	    if (counter < DL_MAX_COLORS) {
	       dlColor[counter].r = (color & 0x00ff0000) >> 16;
	       dlColor[counter].g = (color & 0x0000ff00) >> 8;
	       dlColor[counter].b = color & 0x000000ff;
	       counter++;
	    }
	 } else {
	    
	 }
        getToken(displayInfo,token);
	break;
      }
      case T_LEFT_BRACE:
	nestingLevel++; break;
      case T_RIGHT_BRACE:
	nestingLevel--; break;
    default:
       break;
    }
  } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
		&& (tokenType != T_EOF) );

/* and restore displayInfo->filePtr to previous value */
  displayInfo->filePtr = savedFilePtr;
}

void parseOldDlColor(
  DisplayInfo *displayInfo,
  FILE *filePtr,
  DlColormapEntry *dlColor)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;
 
  FILE *savedFilePtr;
 
/*
 * (MDA) have to be sneaky for these colormap parsing routines:
 *      since possibly external colormap, save and restore
 *      external file ptr in  displayInfo so that getToken()
 *      works with displayInfo and not the filePtr directly
 */
  savedFilePtr = displayInfo->filePtr;
  displayInfo->filePtr = filePtr;
 
  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
      case T_WORD:
        if (STREQL(token,"r")) {
          getToken(displayInfo,token);getToken(displayInfo,token);
          dlColor->r = atoi(token);
        } else
        if (STREQL(token,"g")) {
          getToken(displayInfo,token);getToken(displayInfo,token);
          dlColor->g = atoi(token);
        } else
        if (STREQL(token,"b")) {
          getToken(displayInfo,token);getToken(displayInfo,token);
          dlColor->b = atoi(token);
        } else if (STREQL(token,"inten")) {
          getToken(displayInfo,token);getToken(displayInfo,token);
          dlColor->inten = atoi(token);
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
 
/* and restore displayInfo->filePtr to previous value */
  displayInfo->filePtr = savedFilePtr;
}

/* parseColormap and parseDlColor have two arguments, since could in fact
 *   be parsing and external colormap file, hence need to pass in the 
 *   explicit file ptr for the current colormap file
 */
DlColormap *parseColormap(
  DisplayInfo *displayInfo,
  FILE *filePtr)
{
  char token[MAX_TOKEN_LENGTH];
  char msg[2*MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;
  DlColormap *dlColormap = createDlColormap(displayInfo);
  DlColormapEntry dummyColormapEntry;
/*  DlElement *dlTarget;*/
  int counter;

  FILE *savedFilePtr;

/*
 * (MDA) have to be sneaky for these colormap parsing routines:
 *	since possibly external colormap, save and restore 
 *	external file ptr in  displayInfo so that getToken()
 *	works with displayInfo and not the filePtr directly
 */
  savedFilePtr = displayInfo->filePtr;
  displayInfo->filePtr = filePtr;

  /* initialize some data in structure */
  dlColormap->ncolors = 0;

  /* new colormap, get values (pixel values are being stored) */
  counter = 0;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"ncolors")) {
        getToken(displayInfo,token);
        getToken(displayInfo,token);
        dlColormap->ncolors = atoi(token);
        if (dlColormap->ncolors > DL_MAX_COLORS) {
          sprintf(msg,"%s%s%s",
            "Maximum # of colors in colormap exceeded;\n\n",
            "truncating color space, but will continue...\n\n",
            "(you may want to change the colors of some objects)");
          fprintf(stderr,"\n%s\n",msg);
          dmSetAndPopupWarningDialog(displayInfo, msg,"Ok",NULL,NULL);
        }
      } else
      if (STREQL(token,"dl_color")) {
        /* continue parsing but throw away "excess" colormap entries */
        if (counter < DL_MAX_COLORS) {
          parseOldDlColor(displayInfo,filePtr,&(dlColormap->dl_color[counter]));
          counter++;
        } else {
          parseOldDlColor(displayInfo,filePtr,&dummyColormapEntry);
          counter++;
        }
      } else
      if (STREQL(token,"colors")) {
        parseDlColor(displayInfo,filePtr,dlColormap->dl_color);
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
  } while ((tokenType != T_RIGHT_BRACE) && (nestingLevel > 0) &&
           (tokenType != T_EOF));

  /*
   *  now since a valid colormap element has been brought into display list,
   *  remove the external cmap reference in the dlDisplay element
   * /
  if ((dlTarget = FirstDlElement(displayInfo->dlElementList)))
    DM2KFREE(dlTarget->structure.display->cmap);
  */
  /* restore the previous filePtr */
  displayInfo->filePtr = savedFilePtr;
  return (dlColormap);
}


void writeDlColormap(
  FILE *stream,
  DlColormap *dlColormap,
  int level)
{
  int i;
  char indent[256]; level=MIN(level,256-2);

  for (i = 0; i < level; i++) indent[i] = '\t';
  indent[i] = '\0';

  fprintf(stream,"\n%s\"color map\" {",indent);
  fprintf(stream,"\n%s\tncolors=%d",indent,dlColormap->ncolors);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
  	fprintf(stream,"\n%s\tcolors {",indent);

		for (i = 0; i < dlColormap->ncolors; i++) {
   		 fprintf(stream,"\n\t\t%s%06x,",indent,
              dlColormap->dl_color[i].r*0x10000+
              dlColormap->dl_color[i].g*0x100 +
              dlColormap->dl_color[i].b);
  	}
		fprintf(stream,"\n\t%s}",indent);
#ifdef SUPPORT_0201XX_FILE_FORMAT
	} else {
		for (i = 0; i < dlColormap->ncolors; i++) {
  		fprintf(stream,"\n%s\tdl_color {",indent);
  		fprintf(stream,"\n%s\t\tr=%d",indent,dlColormap->dl_color[i].r);
  		fprintf(stream,"\n%s\t\tg=%d",indent,dlColormap->dl_color[i].g);
  		fprintf(stream,"\n%s\t\tb=%d",indent,dlColormap->dl_color[i].b);
  		fprintf(stream,"\n%s\t\tinten=%d",indent,dlColormap->dl_color[i].inten);
  		fprintf(stream,"\n%s\t}",indent);
    }
	}
#endif
  fprintf(stream,"\n%s}",indent);
}

void executeDlBasicAttribute(DisplayInfo *displayInfo,
                        DlBasicAttribute *attr)
{
  unsigned long gcValueMask;
  XGCValues gcValues;

  if (displayInfo->gc == NULL)
    return;

  gcValueMask = (GCForeground | 
		 GCBackground | 
		 GCLineStyle  | 
		 GCLineWidth  |
		 GCCapStyle   | 
		 GCJoinStyle  | 
		 GCFillStyle);

  gcValues.foreground = displayInfo->colormap[attr->clr];
  gcValues.background = displayInfo->colormap[attr->clr];

  switch (attr->style) {
    case SOLID : gcValues.line_style = LineSolid; break;
    case DASH  : gcValues.line_style = LineOnOffDash; break;
    default    : gcValues.line_style = LineSolid; break;
  }

  gcValues.line_width = attr->width;
  gcValues.cap_style  = CapButt;
  gcValues.join_style = JoinRound;
  gcValues.fill_style = FillSolid;

  XChangeGC(display,displayInfo->gc,gcValueMask,&gcValues);
}

void parseBasicAttribute(DisplayInfo *displayInfo,
                         DlBasicAttribute *attr) {
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
      case T_WORD:
        if (STREQL(token,"clr")) {
          getToken(displayInfo,token);
          getToken(displayInfo,token);
          attr->clr = atoi(token) % DL_MAX_COLORS;
        } else
        if (STREQL(token,"style")) {
          getToken(displayInfo,token);
          getToken(displayInfo,token);
          if (STREQL(token,"solid")) {
              attr->style = SOLID;
          } else
          if (STREQL(token,"dash")) {
              attr->style = DASH;
          }
        } else
        if (STREQL(token,"fill")) {
          getToken(displayInfo,token);
          getToken(displayInfo,token);
          if (STREQL(token,"solid")) {
              attr->fill = F_SOLID;
          } else
          if (STREQL(token,"outline")) {
              attr->fill = F_OUTLINE;
          }
        } else
        if (STREQL(token,"width")) {
          getToken(displayInfo,token);
          getToken(displayInfo,token);
          attr->width = atoi(token);
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

void parseOldBasicAttribute(DisplayInfo *displayInfo,
                            DlBasicAttribute *attr)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;

  attr->clr = 0;
  attr->style = SOLID;
  attr->fill = F_SOLID;
  attr->width = 0;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
      case T_WORD:
	if (STREQL(token,"attr"))
          parseAttr(displayInfo,attr);
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

void writeDlBasicAttribute(FILE *stream, DlBasicAttribute *attr, int level)
{
  char indent[256]; level=MIN(level,256-2);

  memset(indent,'\t',level);
  indent[level] = '\0';

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
  	fprintf(stream,"\n%s\"basic attribute\" {",indent);
  	fprintf(stream,"\n%s\tclr=%d",indent,attr->clr);
  	if (attr->style != SOLID)
    	fprintf(stream,"\n%s\tstyle=\"%s\"",indent,stringValueTable[attr->style]);
  	if (attr->fill != F_SOLID)
    	fprintf(stream,"\n%s\tfill=\"%s\"",indent,stringValueTable[attr->fill]);
  	if (attr->width != 0)
    	fprintf(stream,"\n%s\twidth=%d",indent,attr->width);
  	fprintf(stream,"\n%s}",indent);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  } else {
  	fprintf(stream,"\n%s\"basic attribute\" {",indent);
  	fprintf(stream,"\n%s\tattr {",indent);
  	fprintf(stream,"\n%s\t\tclr=%d",indent,attr->clr);
    fprintf(stream,"\n%s\t\tstyle=\"%s\"",indent,stringValueTable[attr->style]);
    fprintf(stream,"\n%s\t\tfill=\"%s\"",indent,stringValueTable[attr->fill]);
    fprintf(stream,"\n%s\t\twidth=%d",indent,attr->width);
  	fprintf(stream,"\n%s\t}",indent);
  	fprintf(stream,"\n%s}",indent);
  }
#endif
}

#ifdef __cplusplus
void createDlObject(
  DisplayInfo *,
  DlObject *object)
#else
void createDlObject(
  DisplayInfo *displayInfo,
  DlObject *object)
#endif
{
  object->x = globalResourceBundle.x;
  object->y = globalResourceBundle.y;
  object->width = globalResourceBundle.width;
  object->height = globalResourceBundle.height;
}

void objectAttributeInit(DlObject *object) 
{
  object->x = 0;
  object->y = 0;
  object->width = 10;
  object->height = 10;
}

void objectAttributeSet(DlObject *object, int x, int y, unsigned int width,
	unsigned int height) 
{
  object->x = x;
  object->y = y;
  object->width = width;
  object->height = height;
}

void objectAttributeCopy(DlObject * to, DlObject * from) 
{
  if (from != NULL && to != NULL) 
    *to = *from;
}

void overrideAttributeCopy(DlOverrideFields * to, DlOverrideFields * from) 
{
  if (from != NULL && to != NULL) 
    *to = *from;
}

void overrideAttributeInit(DlOverrideFields * to) 
{
   if (to != NULL) {
      to->displayLowLimit = MAXFLOAT;
      to->displayHighLimit = MAXFLOAT;
      to->displayPrecision = -1;
   }
}

void
writeDlOverride(
  FILE *stream,
  DlOverrideFields *dlOverride,
  int level)
{
  int i;
  char indent[256]; level=MIN(level,256-2);

  for (i = 0;  i < MIN(level,256-2); i++) 
     indent[i] = '\t';
  indent[i] = '\0';
  
#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif
     fprintf(stream,"\n%soverride {",indent);
     if (dlOverride->displayLowLimit != MAXFLOAT) 
	fprintf(stream,"\n%s\tlowLimit=%f", indent, dlOverride->displayLowLimit);
     if (dlOverride->displayHighLimit != MAXFLOAT) 
	fprintf(stream,"\n%s\thighLimit=%f", indent, dlOverride->displayHighLimit);
     if (dlOverride->displayPrecision >= 0) 
	fprintf(stream,"\n%s\tprecision=%d", indent, dlOverride->displayPrecision);
     fprintf(stream,"\n%s}",indent);
#ifdef SUPPORT_0201XX_FILE_FORMAT
  }
#endif
}

void parseOverride(
  DisplayInfo *displayInfo,
  DlOverrideFields *override)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;

  override->displayLowLimit = MAXFLOAT;
  override->displayHighLimit = MAXFLOAT;
  override->displayPrecision = -1;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
       if (STREQL(token,"lowLimit")) {
	  getToken(displayInfo,token);
	  getToken(displayInfo,token);
	  override->displayLowLimit = atof(token);
       }
       else if (STREQL(token,"highLimit")) {
	  getToken(displayInfo,token);
	  getToken(displayInfo,token);
	  override->displayHighLimit = atof(token);
       }
       else if (STREQL(token,"precision")) {
	  getToken(displayInfo,token);
	getToken(displayInfo,token);
	override->displayPrecision = atoi(token);
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


/*ARGSUSED*/
#ifdef __cplusplus
void objectAttributeDestroy(DlObject * ) 
#else
void objectAttributeDestroy(DlObject * object) 
#endif
{
  return;
}

void basicAttributeInit(DlBasicAttribute *attr) 
{
  attr->clr   = 0;
  attr->style = SOLID;
  attr->fill  = F_SOLID;
  attr->width = 0;
}


void basicAttributeCopy(DlBasicAttribute * to,
			DlBasicAttribute * from) 
{
  if (from != NULL && to != NULL) 
    *to = *from;
}

/*ARGSUSED*/
#ifdef __cplusplus
void basicAttributeDestroy(DlBasicAttribute * ) 
#else
void basicAttributeDestroy(DlBasicAttribute * attr)
#endif
{
  return;
}


void dynamicAttributeInit(DlDynamicAttribute *dynAttr) 
{
  dynAttr->clr       = STATIC;
  dynAttr->vis       = V_STATIC;
  dynAttr->colorRule = NULL;
  dynAttr->chan      = NULL;
}

void dynamicAttributeDestroy(DlDynamicAttribute *dynAttr) 
{
  DM2KFREE(dynAttr->chan);
}

void dynamicAttributeCopy(DlDynamicAttribute * to,
			  DlDynamicAttribute * from) 
{
  if (from != NULL && to != NULL) {
    to->clr       = from->clr;
    to->vis       = from->vis;
    to->colorRule = from->colorRule;
    to->chan      = NULL;
    renewString(&to->chan, from->chan);
  }
}


void freeDlElement(DlElement *);
#if 0
static DlElement * _createDlElement(DlElement * element);
#endif
DlElement * createDlElement(DlElementType, XtPointer, DlDispatchTable *);

/*ARGSUSED*/
void writeDlElement(FILE *stream, DlElement *DlElement, int level) 
{return;}

/*ARGSUSED*/
static UpdateTask * executeMethod (DisplayInfo * in, DlElement * el)
{return NULL;}

static DlDispatchTable elementDlDispatchTable = {
         NULL,
         freeDlElement,
         executeMethod,
	 writeDlElement,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL};

#if 0
/*ARGSUSED*/
static DlElement * _createDlElement(DlElement * element)
{
  return NULL;
}
#endif

extern Boolean debugFlag;

DlElement * createDlElement(DlElementType     type,
			    XtPointer         structure,
			    DlDispatchTable * dlDispatchTable)
{
  DlElement *dlElement;
  
  if (dlElementFreeList->count > 0) {
    dlElement = dlElementFreeList->tail;
    removeDlElement(dlElementFreeList,dlElement);
    
    if ( debugFlag )
      printf("createDlElement, %ld\n",dlElementFreeList->count);
  } 
  else {
    dlElement = DM2KALLOC(DlElement);
  }

  if (dlElement == NULL) 
    return 0;

  dlElement->displayInfo = NULL;
  dlElement->type = type;
  dlElement->structure.composite = (DlComposite *) structure;

  if (dlDispatchTable) {
    dlElement->run = dlDispatchTable;
  } 
  else {
    if ( debugFlag )
      printf("createDlElement\n");
    
    dlElement->run = &elementDlDispatchTable;
  }
  
  dlElement->widget = 0;
  dlElement->data   = 0;
  dlElement->next   = 0;
  dlElement->prev   = 0;
  dlElement->actif  = False;

  return dlElement;
}

void freeDlElement(DlElement *dlElement) 
{
  appendDlElement(dlElementFreeList,dlElement);
  if ( debugFlag )
    printf("freeDlElement, %ld\n",dlElementFreeList->count);
}
  
ColorRule * getColorRuleByName(char * name)
{
  register ColorRule * colorRule;

  if (name == NULL)
    return NULL;

  for (colorRule = colorRuleHead; colorRule; colorRule = colorRule->next) {
    if (STREQL(name,colorRule->name))
      return colorRule;
  }

  return NULL;
}

GraphicRule * getGraphicRuleByName(char * name)
{
  register GraphicRule *graphicRule;

  if (name == NULL)
    return NULL;

  for (graphicRule = graphicRuleHead; 
       graphicRule != NULL; 
       graphicRule = graphicRule->next)
  {
    if (STREQL(name, graphicRule->name))
      return graphicRule;
  }

  return NULL;
}

Boolean getBooleanByName(char * token)
{
  if (STREQL(token,"on") || STREQL(token,"yes") ||
      STREQL(token,"On") || STREQL(token,"Yes") ||
      STREQL(token,"ON") || STREQL(token,"YES") )
    return True;
  else if (STREQL(token,"off") || STREQL(token,"no") ||
	   STREQL(token,"Off") || STREQL(token,"No") ||
	   STREQL(token,"OFF") || STREQL(token,"NO") )
    return False;
  
  return False;
}

void parseDynamicAttribute(DisplayInfo *displayInfo,
                           DlDynamicAttribute *dynAttr) 
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;

  do {
    switch((tokenType = getToken(displayInfo,token)) ) 
      {
      case T_WORD:
        if (STREQL(token,"clr")) 
	{
          getToken(displayInfo,token);
          getToken(displayInfo,token);

          if (STREQL(token,"discrete") || STREQL(token,"color rule")) {
            dynAttr->clr = DISCRETE;
	    dynAttr->colorRule = getColorRuleByName("set#1");
	  } 
	  else if (STREQL(token,"static"))
            dynAttr->clr = STATIC;
          else if (STREQL(token,"alarm"))
            dynAttr->clr = ALARM;
        } 
	else if (STREQL(token,"vis"))
	{
          getToken(displayInfo,token);
          getToken(displayInfo,token);

          if (STREQL(token,"static"))
            dynAttr->vis = V_STATIC;
          else if (STREQL(token,"if not zero"))
            dynAttr->vis = IF_NOT_ZERO;
          else if (STREQL(token,"if zero"))
            dynAttr->vis = IF_ZERO;
        } 
	else if (STREQL(token,"colorRule")) 
	{
          getToken(displayInfo,token);
          getToken(displayInfo,token);
	  dynAttr->colorRule = getColorRuleByName(token);
        } 
	else if (STREQL(token,"chan")) 
	{
          getToken(displayInfo,token);
          getToken(displayInfo,token);
          if (STRLEN(token) > (size_t) 0) 
            renewString(&dynAttr->chan,token);
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
  } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
	    && (tokenType != T_EOF) );
}

void parseOldDynamicAttribute(DisplayInfo *displayInfo,
			      DlDynamicAttribute *dynAttr)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;


  dynAttr->clr       = STATIC;	/* ColorMode, actually */
  dynAttr->colorRule = 0; 
  dynAttr->vis       = V_STATIC;
  dynAttr->chan      = NULL;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
      case T_WORD:
	if (STREQL(token,"attr"))
          parseDynamicAttr(displayInfo,dynAttr);
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


static const DlObject defaultObject = {0,0,5,5};

DlElement *parseFallingLine(DisplayInfo *displayInfo)
{
  char         token[MAX_TOKEN_LENGTH];
  TOKEN        tokenType;
  int          nestingLevel = 0;
  DlPolyline * dlPolyline;
  DlElement  * dlElement;

  /* Rising Line is replaced by PolyLine 
   */
  dlPolyline = (DlPolyline *) calloc(1,sizeof(DlPolyline));

  /* initialize some data in structure 
   */
  dlPolyline->object = defaultObject;
  dlPolyline->nPoints = 0;
  dlPolyline->points = (XPoint *)NULL;
  dlPolyline->isFallingOrRisingLine = True;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"object"))
	parseObject(displayInfo,&(dlPolyline->object));
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


  dlPolyline->points = (XPoint *)calloc(2,sizeof(XPoint));
  dlPolyline->nPoints = 2;
  dlPolyline->points[0].x = dlPolyline->object.x;
  dlPolyline->points[0].y = dlPolyline->object.y;
  dlPolyline->points[1].x = dlPolyline->object.x + dlPolyline->object.width; 
  dlPolyline->points[1].y = dlPolyline->object.y + dlPolyline->object.height;

  dlElement = DM2KALLOC(DlElement);
  if (dlElement == NULL)
    return NULL;

  dlElement->type = DL_Polyline;
  dlElement->structure.polyline = dlPolyline;
  dlElement->next = NULL;

  return dlElement;
}

DlElement *parseRisingLine(DisplayInfo *displayInfo)
{
  char         token[MAX_TOKEN_LENGTH];
  TOKEN        tokenType;
  int          nestingLevel = 0;
  DlPolyline * dlPolyline;
  DlElement  * dlElement;

  /* Rising Line is replaced by PolyLine */
  dlPolyline = (DlPolyline *) calloc(1,sizeof(DlPolyline));

  /* initialize some data in structure 
   */
  dlPolyline->object = defaultObject;
  dlPolyline->nPoints = 0;
  dlPolyline->points = (XPoint *)NULL;
  dlPolyline->isFallingOrRisingLine = True;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"object"))
	parseObject(displayInfo,&(dlPolyline->object));
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
  
  dlPolyline->points = (XPoint *)calloc(2,sizeof(XPoint)); 
  dlPolyline->nPoints = 2;
  dlPolyline->points[0].x = dlPolyline->object.x;
  dlPolyline->points[0].y = dlPolyline->object.y + dlPolyline->object.height;
  dlPolyline->points[1].x = dlPolyline->object.x + dlPolyline->object.width;
  dlPolyline->points[1].y = dlPolyline->object.y;
  
  dlElement = DM2KALLOC(DlElement);
  if (dlElement == NULL) 
    return NULL;

  dlElement->type = DL_Polyline;
  dlElement->structure.polyline = dlPolyline;
  dlElement->next = NULL;

  return dlElement;
}


/**********************************************************************
 *********    nested objects (not to be put in display list   *********
 **********************************************************************/





void parseObject(DisplayInfo *displayInfo, DlObject *object)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;

  do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	    case T_WORD:
		if (STREQL(token,"x")) {
			getToken(displayInfo,token);
			getToken(displayInfo,token);
			object->x = atoi(token);
		} else if (STREQL(token,"y")) {
			getToken(displayInfo,token);
			getToken(displayInfo,token);
			object->y = atoi(token);
		} else if (STREQL(token,"width")) {
			getToken(displayInfo,token);
			getToken(displayInfo,token);
			object->width = atoi(token);
		} else if (STREQL(token,"height")) {
			getToken(displayInfo,token);
			getToken(displayInfo,token);
			object->height = atoi(token);
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


void parseAttr(DisplayInfo *displayInfo, DlBasicAttribute *attr)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
      case T_WORD:
	if (STREQL(token,"clr")) {
	  getToken(displayInfo,token);
	  getToken(displayInfo,token);
	  attr->clr = atoi(token) % DL_MAX_COLORS;
	} else
        if (STREQL(token,"style")) {
	  getToken(displayInfo,token);
	  getToken(displayInfo,token);
	  if (STREQL(token,"solid")) {
	    attr->style = SOLID;
	  } else
          if (STREQL(token,"dash")) {
	    attr->style = DASH;
	  }
	} else
        if (STREQL(token,"fill")) {
	  getToken(displayInfo,token);
	  getToken(displayInfo,token);
	  if (STREQL(token,"solid")) {
	    attr->fill = F_SOLID;
	  } else
          if (STREQL(token,"outline")) {
	    attr->fill = F_OUTLINE;
	  }
	} else
        if (STREQL(token,"width")) {
	  getToken(displayInfo,token);
	  getToken(displayInfo,token);
	  attr->width = atoi(token);
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




void parseDynamicAttr(DisplayInfo *displayInfo, DlDynamicAttribute *dynAttr)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
      case T_WORD:
	if (STREQL(token,"mod")) {
	  parseDynAttrMod(displayInfo,dynAttr);
	} else
        if (STREQL(token,"param")) {
	  parseDynAttrParam(displayInfo,dynAttr);
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



void parseDynAttrMod(DisplayInfo *displayInfo, DlDynamicAttribute *dynAttr)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
      case T_WORD:
	if (STREQL(token,"clr")) {
	  getToken(displayInfo,token);
	  getToken(displayInfo,token);
          if (STREQL(token,"discrete") || STREQL(token,"color rule")) {
            dynAttr->clr = DISCRETE;
	    dynAttr->colorRule = getColorRuleByName("set#1");
	  } 
	  else if (STREQL(token,"static"))
	    dynAttr->clr = STATIC;
	  else if (STREQL(token,"alarm"))
	    dynAttr->clr = ALARM;
	} 
	else if (STREQL(token,"vis")) 
	{
	  getToken(displayInfo,token);
	  getToken(displayInfo,token);
	  if (STREQL(token,"static"))
	    dynAttr->vis = V_STATIC;
	  else if (STREQL(token,"if not zero"))
	    dynAttr->vis = IF_NOT_ZERO;
	  else if (STREQL(token,"if zero"))
	    dynAttr->vis = IF_ZERO;
        } 
	else if (STREQL(token,"colorRule")) 
	{
          getToken(displayInfo,token);
          getToken(displayInfo,token);
	  dynAttr->colorRule = getColorRuleByName(token);
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



void parseDynAttrParam(DisplayInfo *displayInfo, DlDynamicAttribute *dynAttr)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
      case T_WORD:
	if (STREQL(token,"chan")) {
  	  getToken(displayInfo,token);
	  getToken(displayInfo,token);
	  if (STRLEN(token) > (size_t) 0) 
	    renewString(&dynAttr->chan,token);
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


DlColormap *parseAndExtractExternalColormap(DisplayInfo *displayInfo, char *filename)
{
  DlColormap *dlColormap;
  FILE *externalFilePtr, *savedFilePtr;
  char token[MAX_TOKEN_LENGTH];
  char msg[512];		/* since common longest filename is 255... */
  TOKEN tokenType;
  int nestingLevel = 0;

  dlColormap = NULL;
  externalFilePtr = dmOpenUseableFile(filename);
  if (externalFilePtr == NULL) {
	sprintf(msg,
	  "Can't open \n\n        \"%s\" (.adl)\n\n%s",filename,
	  "to extract external colormap - check cmap specification");
	dmSetAndPopupWarningDialog(displayInfo,msg,"Ok",NULL,NULL);
	fprintf(stderr,
		"\nparseAndExtractExternalColormap:can't open file %s (.adl)\n",
		filename);
	return NULL;
  } else {

    savedFilePtr = displayInfo->filePtr;
    displayInfo->filePtr = externalFilePtr;

    do {
      switch( (tokenType=getToken(displayInfo,token)) ) {
      case T_WORD:
	if (STREQL(token, "color map") || STREQL(token,"<<color map>>")) {
	  dlColormap = 
	    parseColormap(displayInfo,externalFilePtr);
	  /* restore old filePtr */
	  displayInfo->filePtr = savedFilePtr;
	  /* don't want to needlessly parse, so we'll return here */
	  return (dlColormap);
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
    } while (tokenType != T_EOF);

    fclose(externalFilePtr);

   /* restore old filePtr */
    displayInfo->filePtr = savedFilePtr;
  }

  return NULL;
}







/***
 *** the lexical analyzer
 ***/

/*
 * a lexical analyzer (as a state machine), based upon ideas from
 *	"Advanced Unix Programming"  by Marc J. Rochkind, with extensions:
 * understands macros of the form $(xyz), and substitutes the value in
 *	displayInfo's nameValueTable..name with nameValueTable..value
 */
/*
 * Get and classify token 
 */
static TOKEN getToken2(FILE           * filePtr, 
		       char           * word,
		       NameValueTable * nameValueTable,
		       int              nameValueTableNum)
{
  enum {NEUTRAL,INQUOTE,INWORD,INMACRO} state = NEUTRAL, savedState = NEUTRAL;
  int c;
  char *w, *value;
  char *m, macro[MAX_TOKEN_LENGTH];
  int j;

  w = word;
  m = macro;

  errno = 0;
  while ( (c=getc(filePtr)) != EOF ) {
    switch (state) 
      {
      case NEUTRAL:
	switch(c) 
	  {
	  case '=' : return (T_EQUAL);
	  case '{' : return (T_LEFT_BRACE);
	  case '}' : return (T_RIGHT_BRACE);
	  case '"' : state = INQUOTE; 
	    break;

	  case '$' : c=getc(filePtr);
	    /* only do macro substitution if in execute mode 
	     */
	    if (globalDisplayListTraversalMode == DL_EXECUTE
		&& c == '(' ) {
	      state = INMACRO;
	    } else {
	      *w++ = '$';
	      ungetc(c, filePtr); /* *w++ = c; Thomas Birke */
	    }
	    break;

	  case ' ' :
	  case '\t':
	  case '\n': 
	    break;

	    /* for constructs of the form (a,b) 
	     */
	  case '(' :
	  case ',' :
	  case ')' : *w++ = c; *w = '\0'; 
	    return (T_WORD);

	  default  : state = INWORD;
	    *w++ = c;
	    break;
	  }
	break;

      case INQUOTE:
	switch(c) 
	  {
	  case '"' : *w = '\0'; 
	    return (T_WORD);

	  case '$' : c=getc(filePtr);
	    /* only do macro substitution if in execute mode 
	     */
	    if (globalDisplayListTraversalMode == DL_EXECUTE
		&& c == '(' ) {
	      savedState = INQUOTE;
	      state = INMACRO;
	    } else {
	      *w++ = '$';
	      ungetc(c,filePtr); /* it was *w++ = c; , 
				  * changed by Thomas Birke from Bessy;
				  */
	    }
	    break;

	  default  : *w++ = c;
	    break;
	  }
	break;

      case INMACRO:
	switch(c) 
	  {
	  case ')' : *m = '\0';
	    value = lookupNameValue(nameValueTable, nameValueTableNum, macro);

	    if (value != NULL) {
	      for (j = 0; j < (int) STRLEN(value); j++) {
		*w++ = value[j];
	      }
	    } else {
	      *w++ = '$';
	      *w++ = '(';
	      for (j = 0; j < (int) STRLEN(macro); j++) {
		*w++ = macro[j];
	      }
	      *w++ = ')';
	    }
	    state = savedState;
	    m = macro;
	    break;

	  default  : *m++ = c;
	    break;
	  }
	break;

      case INWORD:
	switch(c) 
	  {
	  case ' ' :
	  case '\n':
	  case '\t':
	  case '=' :
	  case '(' :
	  case ',' :
	  case ')' :
	  case '"' : ungetc(c,filePtr); *w = '\0'; return (T_WORD);
	  default  : *w++ = c;
	    break;
	  }
	break;
      }
  }
  if ((c == EOF) && errno)
     fprintf(stderr, "error reading adl-file\n\n\t%s\n", strerror(errno));

  return (T_EOF);
}


TOKEN getToken(DisplayInfo *displayInfo, char *word)
{
  return getToken2(displayInfo->filePtr, word, 
		   displayInfo->nameValueTable, 
		   displayInfo->numNameValues);
}


void writeDlDynamicAttribute (
   FILE               * stream, 
   DlDynamicAttribute * dynAttr,
   unsigned long        mask,
   int                  level)
{
  char indent[256]; level=MIN(level,256-2);

  if (mask & DYNATTR_CHANNEL && dynAttr->chan == NULL)
      return;

  level=MIN(level,256-2);
  memset(indent,'\t',level);
  indent[level] = '\0';

#ifdef SUPPORT_0201XX_FILE_FORMAT
  if (Dm2kUseNewFileFormat) {
#endif

    fprintf(stream,"\n%s\"dynamic attribute\" {",indent);

    if (mask & DYNATTR_COLORMODE)
      fprintf(stream,"\n%s\tclr=\"%s\"",indent,stringValueTable[dynAttr->clr]);

    if (mask & DYNATTR_VISIBILITY)
      fprintf(stream,"\n%s\tvis=\"%s\"",indent,stringValueTable[dynAttr->vis]);

    if (mask & DYNATTR_COLORRULE) {
      if (dynAttr->colorRule != NULL) {
	fprintf(stream,"\n%s\tcolorRule=\"%s\"",
		indent,
		dynAttr->colorRule->name);
      }
      else if (mask & DYNATTR_COLORMODE && 
	       dynAttr->clr == DISCRETE && 
	       colorRuleHead != NULL)
      {
	fprintf(stream,"\n%s\tcolorRule=\"%s\"",
		indent,
		colorRuleHead->name);
      }
    }

    if (mask & DYNATTR_CHANNEL && (dynAttr->chan != NULL) && dynAttr->chan[0])
      fprintf(stream,"\n%s\tchan=\"%s\"",indent,dynAttr->chan);

    fprintf(stream,"\n%s}",indent);


#ifdef SUPPORT_0201XX_FILE_FORMAT
  } 
  else if (mask != 0)
  {
    fprintf(stream,"\n%s\"dynamic attribute\" {",indent);
    fprintf(stream,"\n%s\tattr {",indent);
    fprintf(stream,"\n%s\t\tmod {",indent);

    if (mask & DYNATTR_COLORMODE)
      fprintf(stream,"\n%s\t\t\tclr=\"%s\"",
	      indent,stringValueTable[dynAttr->clr]);

    if (mask & DYNATTR_VISIBILITY)
      fprintf(stream,"\n%s\t\t\tvis=\"%s\"",
	      indent,stringValueTable[dynAttr->vis]);

    if (mask & DYNATTR_COLORRULE) {
      if (dynAttr->colorRule != NULL) {
	fprintf(stream,"\n%s\t\t\tcolorRule=\"%s\"",
		indent,
		dynAttr->colorRule->name);
      }
      else if (mask & DYNATTR_COLORMODE &&
	       dynAttr->clr == DISCRETE && 
	       colorRuleHead != NULL) 
      {
	fprintf(stream,"\n%s\tcolorRule=\"%s\"",
		indent,
		colorRuleHead->name);
      }
    }
  
    fprintf(stream,"\n%s\t\t}",indent);
    fprintf(stream,"\n%s\t\tparam {",indent);

    if (mask & DYNATTR_CHANNEL && (dynAttr->chan != NULL) && dynAttr->chan[0])
      fprintf(stream,"\n%s\t\t\tchan=\"%s\"",indent,dynAttr->chan);

    fprintf(stream,"\n%s\t\t}",indent);
    fprintf(stream,"\n%s\t}",indent);
    fprintf(stream,"\n%s}",indent);
  }
#endif
}



void writeDlObject(
  FILE *stream,
  DlObject *dlObject,
  int level)
{
  char indent[256]; level=MIN(level,256-2);

  memset(indent,'\t',level);
  indent[level] = '\0';

  fprintf(stream,"\n%sobject {",indent);
  fprintf(stream,"\n%s\tx=%d",indent,dlObject->x);
  fprintf(stream,"\n%s\ty=%d",indent,dlObject->y);
  fprintf(stream,"\n%s\twidth=%d",indent,dlObject->width);
  fprintf(stream,"\n%s\theight=%d",indent,dlObject->height);
  fprintf(stream,"\n%s}",indent);
}

void genericMove(DlElement *dlElement, int xOffset, int yOffset) {
  dlElement->structure.rectangle->object.x += xOffset;
  dlElement->structure.rectangle->object.y += yOffset;
}

void widgetMove(DlElement *dlElement, int xOffset, int yOffset) {
  dlElement->structure.rectangle->object.x += xOffset;
  dlElement->structure.rectangle->object.y += yOffset;
  if (dlElement->widget)
    XtMoveWidget(dlElement->widget,
                 dlElement->structure.rectangle->object.x,
                 dlElement->structure.rectangle->object.y);
}

void genericScale(DlElement *dlElement, int xOffset, int yOffset) {
  int width, height;
  width = (dlElement->structure.rectangle->object.width + xOffset);
  dlElement->structure.rectangle->object.width = MAX(1,width);
  height = (dlElement->structure.rectangle->object.height + yOffset);
  dlElement->structure.rectangle->object.height = MAX(1,height);
}

static char * defaultColorRule[] = {
"colorRule  set#1",
"	 0.00   0.00   20",
"	 1.00   1.00   16", /* it was 15 before 1.08.97 */
"	 2.00   2.00   22",
"	 3.00   3.00   17",
"	 4.00   4.00   35",
"	 5.00   5.00   55",
"	 6.00   6.00   40",
"	 7.00   7.00   40",
"	 8.00   8.00   30",
"	 9.00   9.00   32",
"	10.00  10.00   30",
"	11.00  11.00   32",
"	12.00  12.00   30",
"	13.00  13.00   32",
"	14.00  14.00   30",
"	15.00  15.00   32",
"colorRule  set#2",
"	 0.00   0.00   20",
"	 1.00   1.00   24",
"	 2.00   2.00   16", /* it was 15 before 1.08.97 */
"	 3.00   3.00   19",
"	 4.00   4.00   55",
"	 5.00   5.00   30",
"	 6.00   6.00   53", /* it was 50 before 1.08.97 */
"	 7.00   7.00   54",
"	 8.00   8.00   45",
"	 9.00   9.00   47",
"	10.00  10.00   40",
"	11.00  11.00   42",
"	12.00  12.00   32",
"	13.00  13.00   33",
"	14.00  14.00    4",
"	15.00  15.00   14",
"colorRule  set#3",
"	-0.01   0.01  32",
"	 0.99   1.01  33",
"	 1.99   2.01  34",
"	 2.99   3.01  35",
"	 3.99   4.01  36",
"	 4.99   5.01  37",
"	 5.99   6.01  38",
"	 6.99   7.01  39",
"	 7.99   8.01  40",
"	 8.99   9.01  41",
"	 9.99  10.01  42",
"	10.99  11.01  43",
"	11.99  12.01  44",
"	12.99  13.01  45",
"	13.99  14.01  46",
"	14.99  15.01  47",
"colorRule  set#4",
"	 1.00   1.00   15",
"	 0.00   0.00   20",
"	 2.00   2.00   22",
"	 3.00   3.00   17",
"	 4.00   4.00   35",
"	 5.00   5.00   55",
"	 6.00   6.00   40",
"	 7.00   7.00   40",
"	 8.00   8.00   30",
"	 9.00   9.00   32",
"	10.00  10.00   30",
"	11.00  11.00   32",
"	12.00  12.00   30",
"	13.00  13.00   32",
"	14.00  14.00   30",
"	15.00  15.00   32"
};

char * getNextLineFromColorRule(char * buffer, int maxRead, FILE * file)
{
  if (file)
    return fgets(buffer, maxRead, file);
  else {
    static int nextLineNumber = 0;

    if (nextLineNumber < XtNumber(defaultColorRule)) {
      strcpy(buffer, defaultColorRule[nextLineNumber]);
      nextLineNumber++;
      return defaultColorRule[nextLineNumber-1];
    }
    else
      return NULL;
  }
}


#define LINE_SIZE 250

void readColorRulesFromFile(char * filename)
{
  FILE      * file;
  char        line[LINE_SIZE+1];
  ColorRule * colorRule= NULL;

   /* T. Straumann:	we must check for filename==0 because this
	* 				routine is called with 'getenv()' as arg.
	*				We also must make sure the file could be
	*				opened
	*/
  if ( (!filename || !(file = fopen(filename,"r")) ) && 
       !(file = fopen("ColorRules", "r")) ) 
     return;

  while(getNextLineFromColorRule(line, LINE_SIZE, file)) {
    char * tmp = strchr (line, (int)';');

    if (tmp) *tmp = '\0';

    if (line[0] != '\0')
      {
	char  dummy[50];
	char  name[LINE_SIZE];
	float lower;
	float upper;
	int   index;	
	int   i;


	/* try to read "colorRule <setname>" string
	 */
	*dummy = *name = '\0';
	sscanf(line,"%s %s", dummy, name);
	
	if (strcmp(dummy,"colorRule") == 0) {

	  colorRule = DM2KALLOC(ColorRule);

	  if (colorRule == NULL) {
	    fprintf(stderr, "%s:%d\ncouldn't allocate memmory\n", __FILE__, __LINE__);
	    return;
	  }
	  
	  colorRule->name = STRDUP(name);

	  if (colorRule->name == NULL) {
	    fprintf(stderr, "%s:%d\ncouldn't allocate memmory\n",
		    __FILE__, __LINE__);
	    free((char*)colorRule);
	    return;
	  }

	  colorRule->entries = NULL;
	  colorRule->count   = 0;
	  colorRule->next    = colorRuleHead;
	  colorRuleHead      = colorRule;
	  colorRuleCounts++;

	  continue;
	}

	/* if there is not yet any color rule set defined, just skip the line
	 */
	if (colorRule == NULL)
	  continue;

	/* if the line is empty, just skip it
	 */
	for (i = 0; isspace(line[i]) && line[i] != '\0' ; i++) 
	  /*EMPTY*/;

	if (line[i] == '\0')
	  continue;

	/* try to read color rule set entry;
	 * in form of two reals and one integer
	 */
	sscanf(line,"%f %f %d", &lower, &upper, &index);	

	REALLOC(colorRule_t, colorRule->entries, ++colorRule->count);

	colorRule->entries[colorRule->count - 1].lowerBoundary = lower;
	colorRule->entries[colorRule->count - 1].upperBoundary = upper;
	if (index < 0 || index > 64) {
	  index = 0;
	}

	colorRule->entries[colorRule->count - 1].colorIndex = index;
      }
  }
  fclose(file);

  /* reverse the list of color rules
   */
  {
    ColorRule * copy = NULL;

    while(colorRuleHead != NULL) {
      ColorRule * tmp = colorRuleHead;

      colorRuleHead = colorRuleHead->next;
      tmp->next = copy;
      copy = tmp;
    }

    colorRuleHead = copy;
  }

}
#undef LINE_SIZE

static DlElement * getCompositeByName(DlList * list, char * name)
{
  DlElement * element;

  if (list == NULL || name == NULL)
    return NULL;

  element = FirstDlElement(list);

  while (element != NULL) 
  {
    if (element->type == DL_Composite && 
	STREQL(element->structure.composite->compositeName, name))
      return element;
    
    element = element->next;
  }
  
  return NULL;
}

static void makeElementListForFraphicRule(
  GREData     * data,
  char        * elementName,
  DisplayInfo * displayInfo)
{
  DlElement * element;

  if (displayInfo == NULL)
    return;

  element = getCompositeByName(displayInfo->dlElementList, elementName);

  if (element == NULL)
    return;

  if (NULL == (data->dlElementList = createDlList()))
    return;

  data->x      = element->structure.composite->object.x;
  data->y      = element->structure.composite->object.y;
  data->width  = element->structure.composite->object.width;
  data->height = element->structure.composite->object.height;
    
  element = createDlComposite(element);
  appendDlElement(data->dlElementList, element);
}


static void buildGREData(
  GREData * data,
  char    * adlFileName,
  char    * elementName,
  char    * macroName)
{
  FILE        * filePtr = NULL;
  DisplayInfo * displayInfo;
  DisplayInfo * tmp;
  Boolean       addNewDisplayInfoToList = True;

  int canAccess = 0;
  char fullPathName[1024];

  displayInfo = createInitializedDisplayInfo();
  if (displayInfo == NULL)
    return;

/*==========================*/

  if ((canAccess = !access(adlFileName, R_OK|F_OK))) {
     strcpy( fullPathName, adlFileName );
  } else {
     char name[1024];
     char *dir = NULL;
     int startPos;
     dir = getenv(DISPLAY_LIST_ENV);
     if (dir != NULL) {
        startPos = 0;
        while (extractStringBetweenColons(dir,name,startPos,&startPos)) {
	   if (STRLEN(name)+STRLEN(adlFileName)+2 < (size_t) 1024) {
	      strcpy(fullPathName,name);
	      strcat(fullPathName,"/");
	      strcat(fullPathName,adlFileName);
	      if ((canAccess = !access(fullPathName,R_OK|F_OK))) break;
	   }
        }
     }
  }

/*==========================*/

  if (canAccess)
     filePtr = fopen(fullPathName, "ra");
  if (filePtr == NULL)
    return;

  displayInfo->filePtr = filePtr;
  tmp = dmDisplayListParse2(grahicRuleDisplayInfoListHead,
			    displayInfo, filePtr, macroName, fullPathName);

  if (tmp != displayInfo) {
    free((char*)displayInfo);
    displayInfo = tmp;
    addNewDisplayInfoToList = False;
  }
  
  fclose(filePtr);

  if (displayInfo == NULL)
    return;

  /* executeDlColormap(displayInfo, displayInfo->dlColormap); */

  if (addNewDisplayInfoToList) {
    /* append to end of the list of static displayInfos
     */
    displayInfo->next = NULL;
    displayInfo->prev = grahicRuleDisplayInfoListTail;
    
    if (grahicRuleDisplayInfoListHead == NULL) 
      grahicRuleDisplayInfoListHead = displayInfo;
    
    if (grahicRuleDisplayInfoListTail != NULL) 
      grahicRuleDisplayInfoListTail->next = displayInfo;
    
    grahicRuleDisplayInfoListTail = displayInfo;
  }

  /* copy sublist of elements from the element of given name*/
  makeElementListForFraphicRule(data, elementName, displayInfo);

  /* link the GREData into global list */
  data->next = headGREData;
  headGREData = data;
}

static void destroyGrahicRuleDisplayInfoList()
{
  DisplayInfo * displayInfo;

  if (grahicRuleDisplayInfoListHead == NULL) 
    return;

  displayInfo = grahicRuleDisplayInfoListHead;

  grahicRuleDisplayInfoListHead = grahicRuleDisplayInfoListTail = NULL;

  while(displayInfo)
    {
      DisplayInfo * toBeDeleted = displayInfo;


      displayInfo = displayInfo->next;

      /* cleaup resources and free display list 
       */
      dmCleanupDisplayInfo(toBeDeleted,True);
      freeNameValueTable(toBeDeleted->nameValueTable,
			 toBeDeleted->numNameValues);

      destroyDlDisplayList(toBeDeleted->selectedDlElementList);
      destroyDlDisplayList(toBeDeleted->dlElementList);
      DM2KFREE(toBeDeleted->selectedDlElementList);
      DM2KFREE(toBeDeleted->dlElementList);
      DM2KFREE(toBeDeleted->dlFile);
      DM2KFREE(toBeDeleted->dlColormap);
      DM2KFREE(toBeDeleted);
    }
}


static GREData * getGREData(
  float   lowerBoundary,
  float   upperBoundary,
  char  * adlFileName,
  char  * elementName,
  char  * macroName)
{
  GREData * data = headGREData;

  for(data = headGREData; data != NULL; data = data->next)
    {
      if (STREQL(data->adlFileName,adlFileName) 
	  && STREQL(data->elementName, elementName)
	  && STREQL(data->macro, macroName))
	break;
    }

  if (data != NULL) 
    return data;

  data = DM2KALLOC(GREData);
  if (data == NULL) 
    return NULL;
  
  data->adlFileName   = STRDUP(adlFileName);
  data->elementName   = STRDUP(elementName);
  data->macro         = STRDUP(macroName);
  
  data->dlElementList = NULL;
  
  buildGREData(data, adlFileName, elementName, macroName);
  return data;
}
	  


#define LINE_SIZE 10000

void readGraphicRulesFromFile(char * filename)
{
  FILE        * file;
  char          line[LINE_SIZE+1];
  GraphicRule * graphicRule= NULL;
  DisplayInfo * cDI = currentDisplayInfo;

#define RETURN  currentDisplayInfo = cDI; return

   /* T. Straumann:	we must check for filename==0 because this
	* 				routine is called with 'getenv()' as arg.
	*/
  if ( NULL==filename || NULL==(file = fopen(filename,"r")) )
    file = fopen("GraphicRules", "r");

  if (file == NULL) {
    RETURN;
  }

  while(fgets(line, LINE_SIZE, file)) 
  {
    char * tmp = strchr (line, (int)';');

    if (tmp) 
      *tmp = '\0';

    if (line[0] != '\0')
      {
	char  dummy[50];
	char  name[LINE_SIZE];
	int   i;


	/* try to read "graphicRule <setname>" string
	 */
	*dummy = *name = '\0';
	sscanf(line,"%s %s", dummy, name);
	
	if (STREQL(dummy,"graphicRule")) {

	  graphicRule = DM2KALLOC(GraphicRule);

	  if (graphicRule == NULL) {
	    fprintf(stderr, "%s:%d\ncouldn't allocate memmory\n",
		    __FILE__, __LINE__);
	    destroyGrahicRuleDisplayInfoList();
	    RETURN;
	  }
	  
	  graphicRule->name = STRDUP(name);

	  if (graphicRule->name == NULL) {
	    fprintf(stderr, "couldn't allocate memmory(%s:%d)\n",
		    __FILE__, __LINE__);
	    free((char*)graphicRule);
	    destroyGrahicRuleDisplayInfoList();
	    RETURN;
	  }

	  graphicRule->entries  = NULL;
	  graphicRule->count    = 0;
	  graphicRule->refCount = 0;
	  graphicRule->next     = graphicRuleHead;
	  graphicRuleHead       = graphicRule;
	  graphicRuleCounts++;

	  continue;
	}

	/* if there is not yet any color rule set defined, just skip the line
	 */
	if (graphicRule == NULL)
	  continue;

	/* if the line is empty, just skip it
	 */
	for (i = 0; isspace(line[i]) && line[i] != '\0' ; i++) 
	  /*EMPTY*/;

	if (line[i] == '\0')
	  continue;
	else
	{
	  float lowerBoundary;
	  float upperBoundary;
	  char  adlFileName[MAX_TOKEN_LENGTH];
	  char  elementName[MAX_TOKEN_LENGTH];
	  char  macroName[MAX_TOKEN_LENGTH];
	  
	  GraphicRuleEntry * entry;
	  GREData          * data;

	  adlFileName[0]    = '\0';
	  elementName[0]    = '\0';
	  macroName[0]      = '\0';

	  /* try to read color rule set entry;
	   * in form of :
	   * [0] (float) : lowerBoundary
	   * [1] (float) : upperBoundary
	   * [2] (char*) : adl file name
	   * [3] (char*) : element name
	   * [4] (char*) : macro
	   */
	  sscanf(line, "%f %f %s %s %s",
		 &lowerBoundary,
		 &upperBoundary,
		 adlFileName,
		 elementName,
		 macroName);
 

	  data = getGREData(lowerBoundary,
			    upperBoundary,
			    adlFileName,
			    elementName,
			    macroName);
	  if (data == NULL) {
	    destroyGrahicRuleDisplayInfoList();
	    RETURN;
	  }

	  REALLOC(GraphicRuleEntry, graphicRule->entries, ++graphicRule->count);

	  if (graphicRule->entries == NULL) {
	    destroyGrahicRuleDisplayInfoList();
	    RETURN;
	  }

	  entry = & graphicRule->entries[graphicRule->count - 1];
	  
	  entry->lowerBoundary = (double)lowerBoundary;
	  entry->upperBoundary = (double)upperBoundary;
	  entry->data          = data;
	}
      }
  }

  /* reverce the list of graphic rules
   */
  {
    register GraphicRule * newList = NULL;
    register GraphicRule * tmp;

    while(graphicRuleHead != NULL) {
      tmp = graphicRuleHead;
      graphicRuleHead = graphicRuleHead->next;
      tmp->next = newList;
      newList = tmp;
    }

    graphicRuleHead = newList;
  }

  fclose(file);
  destroyGrahicRuleDisplayInfoList();

  RETURN;
#undef RETURN
}

void fileMenuDialogCallback(
  Widget w,
  XtPointer clientData,
  XtPointer callbackStruct)
{
  int btn = (int) clientData;
  XmAnyCallbackStruct *call_data = (XmAnyCallbackStruct *) callbackStruct;
  XmSelectionBoxCallbackStruct *select;
  char *filename, warningString[2*MAX_FILE_CHARS];
  XmString warningXmstring;

  if (btn >= FILE_DISPLAY_LIST_ENV) {
    Widget dialog = XtParent(w);
    char * adlDir = getenv(DISPLAY_LIST_ENV);
    Arg    al[12];
    int    ac;
    int    nth = btn - FILE_DISPLAY_LIST_ENV;
       
    if (nth > 0) {
       dialog = XtParent(XtParent(dialog));
       nth--;
       if (nth) {
	  nth--;
       } else {
	  adlDir = ".";
       }
    }

    if (adlDir != NULL) {
      char dlPath[FULLPATHNAME_SIZE+1];
      char dirMask[FULLPATHNAME_SIZE+1];
      XmString xmstrings[4];
      int startPos = 0;
      char *mask;

      XmListDeselectAllItems(XmFileSelectionBoxGetChild(dialog,XmDIALOG_LIST));
      while (nth >= 0) {
	 extractStringBetweenColons(adlDir,dlPath,startPos,&startPos);
	 nth--;
      }
      XtVaGetValues( dialog, XmNuserData, &mask, NULL );
      ac = 0;
      sprintf(dirMask, "%s%s", dlPath, mask);
      xmstrings[0] = XmStringCreateLtoR ( dirMask, (XmStringCharSet)XmFONTLIST_DEFAULT_TAG );
      XtSetArg(al[ac], XmNdirMask, xmstrings[0]); ac++;
      xmstrings[1] = XmStringCreateLtoR ( dlPath, (XmStringCharSet)XmFONTLIST_DEFAULT_TAG );
      XtSetArg(al[ac], XmNdirSpec, xmstrings[1]); ac++;
      xmstrings[2] = XmStringCreateLtoR ( dlPath, (XmStringCharSet)XmFONTLIST_DEFAULT_TAG );
      XtSetArg(al[ac], XmNdirectory, xmstrings[2]); ac++;
      xmstrings[3] = XmStringCreateLtoR ( mask, (XmStringCharSet)XmFONTLIST_DEFAULT_TAG );
      XtSetArg(al[ac], XmNpattern, xmstrings[3]); ac++;
      
      XtSetValues(dialog, al, ac);

	  /* T. Straumann:  DONT FREE LOCAL VAR
       *	 			free(dlPath);
	   */
      XmStringFree ( xmstrings [ 0 ] );
      XmStringFree ( xmstrings [ 1 ] );
      XmStringFree ( xmstrings [ 2 ] );
      XmStringFree ( xmstrings [ 3 ] );
    }
    return;
  }

  switch(call_data->reason)
    {
    case XmCR_CANCEL:
      XtUnmanageChild(w);
      break;

    case XmCR_OK:
      switch(btn) 
	{
	case FILE_OPEN_BTN: 
	  {
	    FILE *filePtr;
	    char *filename;
	    
	    XmSelectionBoxCallbackStruct *call_data =
	      (XmSelectionBoxCallbackStruct *) callbackStruct;
	    
	    /* if no list element selected, simply return */
	    if (call_data->value == NULL) return;
	    
	    /* get the filename string from the selection box */
	    XmStringGetLtoR(call_data->value, XmSTRING_DEFAULT_CHARSET, &filename);
	    
	    if (filename) {
	      filePtr = fopen(filename,"r");
	      if (filePtr) {
		XtUnmanageChild(w);
		dmDisplayListParse(NULL,filePtr,NULL,filename,
				   NULL,(Boolean)False);
		enableEditFunctions();
		fclose(filePtr);
	      }
	      XtFree(filename);
	    }
	  }
	  break;

	case FILE_CLOSE_BTN:
	  dmRemoveDisplayInfo(currentDisplayInfo);
	  currentDisplayInfo = NULL;
	  break;

	case FILE_SAVE_AS_BTN:
	  select = (XmSelectionBoxCallbackStruct *)call_data;
	  XmStringGetLtoR(select->value,XmSTRING_DEFAULT_CHARSET,&filename);
	  dm2kSaveDisplay(currentDisplayInfo,filename,False);
	  sprintf(warningString,"%s", "Name of file to save display in:");
	  warningXmstring = XmStringCreateSimple( warningString);
	  XtVaSetValues(saveAsPD,XmNselectionLabelString, warningXmstring,NULL);
	  XmStringFree(warningXmstring);
	  XtFree(filename);
	  XtUnmanageChild(w);
	  break;

	case FILE_EXIT_BTN:
	  dm2kClearImageCache();
	  dm2kCATerminate();
	  dmTerminateX();
	  exit(0);
	  break;

	case FILE_FACEPLATELOAD_BTN : {
	  char *filename;
	    
	  XmSelectionBoxCallbackStruct *call_data =
	    (XmSelectionBoxCallbackStruct *) callbackStruct;
	  
	  /* if no list element selected, simply return */
	  if (call_data->value == NULL) return;
	  
	  /* get the filename string from the selection box */
	  XmStringGetLtoR(call_data->value, XmSTRING_DEFAULT_CHARSET, &filename);
	  
	  XtUnmanageChild(w);
	  if (filename) {
	     openFaceplate( filename );
	     XtFree(filename);
	  }
	}
	break;
	}
    }
}


Widget fillFileSelBox(Widget openFSD) 
{
   Arg args[4];
   int n = 0;
   Widget button;
   XmString xmstring;
   char * dlPath;

   /* T. Straumann: check for dlPath == NULL */
   dlPath = getenv( DISPLAY_LIST_ENV );
   if (!dlPath) dlPath=".";

   if ( strchr( dlPath, ':') ) {
      char *adlEnv = STRDUP( dlPath );
      int nth = 1;
      char *merke = adlEnv;
      char dir[1024];
      int startPos = 0;
      Widget wp, pulldown;
      /* DISPLAY_LIST_ENV has more than one component.
	 create a popup-menu to choose one component from */
      n = 0;
      XtSetArg(args[n],XmNtearOffModel, XmTEAR_OFF_DISABLED ); n++;
      button = XmCreateSimpleOptionMenu( openFSD, "OptionMenu", args, n );
      pulldown = XmCreatePulldownMenu( openFSD, "Pulldown", args, n );
      XtVaSetValues( button, XmNsubMenuId, pulldown, NULL );
      n = 0;
      wp = XtCreateManagedWidget( ".", xmPushButtonGadgetClass,
				  pulldown, args, n );
      XtAddCallback( wp, XmNactivateCallback, fileMenuDialogCallback, 
		     (XtPointer)(FILE_DISPLAY_LIST_ENV + nth) );
      nth++;
      
      while (extractStringBetweenColons(adlEnv,dir,startPos,&startPos)) {
	 wp = XtCreateManagedWidget( dir,
					 xmPushButtonGadgetClass,
					 pulldown, args, n );
	 XtAddCallback( wp, XmNactivateCallback, fileMenuDialogCallback, 
			(XtPointer)(FILE_DISPLAY_LIST_ENV + nth) );
	 nth++;
      }
      XtManageChild(button);
      free( merke );
   } else {
      /* assume DISPLAY_LIST_ENV has only one component.
	 create a button that sets the corresponding path */
	  /* T. Straumann: be consistent; show the path rather
	   *			   than the name of the env. variable.
	   */
      xmstring = XmStringCreateLtoR ( dlPath, 
				      (XmStringCharSet)XmFONTLIST_DEFAULT_TAG );
      n = 0;
      XtSetArg(args[n], XmNlabelString, xmstring); n++;
      button = XmCreatePushButton ( openFSD, "epicsDisplayList", args, n );
      XmStringFree ( xmstring );
      
      XtAddCallback(button, XmNactivateCallback,
		    fileMenuDialogCallback,(XtPointer)FILE_DISPLAY_LIST_ENV);
      
      XtManageChild( button );
   }

   return button;
}

