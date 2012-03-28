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
 *                              - change "engr notation" to "engr. notation"
 *                              - strip chart has two more fields
 *                                "period" and "oldUnits".
 *                              - polyLine has a new field 
 *                                "isFallingOrRisingLine".
 *
 * .03  09-07-95        vong    - remove all the falling line and rising line
 *                                stuff
 *
 * .04  09-02-97        romsky  - DlList has more obvious structure;
 *                              - AssociatedMenuItem for Composite element;
 *
 *
 *****************************************************************************
 *
 *      24-02-97    Fabien  Addition of symbols and element in structure
 *                          for Message Button Type
 *                          and for Controlers sensitivity.
 *
 *****************************************************************************
*/

/****************************************************************************
 * Display list header definition                                           *
 * Mods: DMW - Added 'from center' option to stringValueTable, and          *
 *         FROM_CENTER to FillMode for DlBar                                *
 *       DMW - Added DlByte structure.                                      *
 ****************************************************************************/
#ifndef __DISPLAYLIST_H__
#define __DISPLAYLIST_H__

#include <stdio.h>
#include <sys/types.h>
#include <math.h>
#include <string.h>

#define MAX_TOKEN_LENGTH	4096	/* max size of strings in adl  */
#define MAX_RELATED_DISPLAYS	24	/* max # of related displays   */
#define MAX_SHELL_COMMANDS 	24	/* max # of shell commands     */
#define MAX_PENS		8	/* max # of pens on strip chart*/
#define MAX_TRACES		8	/* max # of traces on cart.plot*/
#define MAX_FILE_CHARS		256	/* max # of chars in filename  */
#define DL_MAX_COLORS		65	/* max # of colors for display */
#define DL_COLORS_COLUMN_SIZE	5	/* # of colors in each column  */

#define STREQL(a,b)            (strcmp(((a)?(a):""),((b)?(b):"")) == 0)

#if defined(__cplusplus) && defined(SUNOS4) && !defined(__GNUG__)
#define _MALLOC_TYPE (malloc_t)
#else
#define _MALLOC_TYPE 
#endif

#define REALLOC(type,ptr,count) \
   ptr = (type*) realloc(_MALLOC_TYPE ptr, count * sizeof(type))


#define CARE_PRINT(string) ((string) != NULL ? (string) : "")

#include "enums.h"

XmString xmStringValueTable[NUMBER_STRING_VALUES];

/* addition for sensitivity */
/* ------------------------ */
#define NUM_SENSITIVE_MODES   2       /* IF_NOT_ZERO IF_ZERO */
typedef VisibilityMode SensitivityMode ;
#define FIRST_SENSITIVE_MODE IF_NOT_ZERO

#define MAX_OPTIONS		9	/* was NUM_TEXT_FORMATS	*/
#define ILLEGAL_DISPLAY        -1

#if defined(ALLOCATE_STORAGE)
Alignment alignmentTranslation[] = {ALIGNMENT_NW, ALIGNMENT_N, ALIGNMENT_NE, ALIGNMENT_NW, ALIGNMENT_N, ALIGNMENT_NE};
#else
extern Alignment alignmentTranslation[];
#endif

/* Color Rule 
 */
typedef struct {
  double lowerBoundary;
  double upperBoundary;
  int    colorIndex;
} colorRule_t;

typedef struct _ColorRule {
  char              * name;
  colorRule_t       * entries;
  int                 count;
  struct _ColorRule * next;
} ColorRule;

extern ColorRule * colorRuleHead;
extern int         colorRuleCounts;

struct _DisplayInfo;

enum {
  SET_FOREGROUND = 1<<0,
  SET_BACKGROUND = 1<<1
};

typedef struct _GREData {
  struct _GREData     * next;
  char                * adlFileName;
  char                * elementName;
  char                * macro;
  struct _DlList      * dlElementList;
  int                   x;
  int                   y;
  short                 width;
  short                 height;
} GREData;

typedef struct _GraphicRuleEntry {
  double    lowerBoundary;
  double    upperBoundary;
  GREData * data;
} GraphicRuleEntry;

typedef struct _GraphicRule {
  char                * name;
  GraphicRuleEntry    * entries;
  int                   count;
  int                   refCount;
  struct _GraphicRule * next;
} GraphicRule;


#if defined(ALLOCATE_STORAGE)
GraphicRule * graphicRuleHead = NULL;
GraphicRule * graphicRuleTail = NULL;
int           graphicRuleCounts = 0;
GREData     * headGREData = NULL;
#else
extern GraphicRule * graphicRuleHead;
extern GraphicRule * graphicRuleTail;
extern int           graphicRuleCounts;
extern GREData     * headGREData;
#endif


/*********************************************************************
 *  controllers are also monitors (controllers are a sub-class of    *
 *  monitors) -> order must be consistent with all of the following: *
 * controllers:                                                      *
 *    DL_Valuator DL_ChoiceButton    DL_MessageButton DL_TextEntry   *
 *    DL_Menu,    DL_RelatedDisplay, DL_ShellCommand                 *
 *  monitors:                                                        *
 *    DL_Meter      DL_TextUpdate    DL_Bar        DL_Indicator      *
 *    DL_StripChart DL_CartesianPlot DL_SurfacePlot                  *
 *  statics acting as monitors (dynamics):                           *
 *    DL_Rectangle    DL_Oval       DL_Arc   DL_Text                 *
 *    DL_Polyline     DL_Polygon                                     *
 *********************************************************************/

typedef enum {
  /*  self 
   */
  DL_Element       =100,

  /* basics 
   */
  DL_Composite     =101,
  DL_Display       =102,

  /* controllers 
   */
  DL_ChoiceButton  =103,
  DL_Menu          =104,
  DL_MessageButton =105,
  DL_RelatedDisplay=106,
  DL_ShellCommand  =107,
  DL_TextEntry     =108,
  DL_Valuator  	   =109,

  /* monitors 
   */
  DL_Bar           =110,
  DL_Byte          =111,
  DL_CartesianPlot =112,
  DL_Indicator     =113,
  DL_Meter         =114,
  DL_StripChart    =115,
  DL_TextUpdate    =116,

  /* primitives 
   */
  DL_Arc           =117,
  DL_Image         =118,
  DL_Line          =119,
  DL_Oval          =120,
  DL_Polygon       =121,
  DL_Polyline      =122,
  DL_Rectangle     =123,
  DL_Text          =124,
  DL_DynSymbol     =125
} DlElementType;

#define MIN_DL_ELEMENT_TYPE     DL_Element
#define MAX_DL_ELEMENT_TYPE     DL_DynSymbol
#define NUM_DL_ELEMENT_TYPES	((MAX_DL_ELEMENT_TYPE-MIN_DL_ELEMENT_TYPE)+1)
#define FIRST_RENDERABLE	DL_Composite

#define ELEMENT_HAS_WIDGET(type) \
                             ((type >= DL_Display && type <= DL_StripChart))

#define ELEMENT_IS_MONITOR(type) \
                             ((type >= DL_Bar && type <= DL_TextUpdate))

#define ELEMENT_IS_CONTROLLER(type) \
                             ((type >= DL_ChoiceButton && type <= DL_Valuator))

#define ELEMENT_IS_PIMITIVE(type) \
                             ((type >= DL_Arc && type <=  DL_DynSymbol))

/* this macro defines those elements which occupy space/position and can
 *  be rendered.  Note: Composite is not strictly renderable because no
 *  pixels are affected as a result of it's creation...
 *  DL_Display appears to be a sort of exception, since it's creation gives
 *  the backcloth upon which all rendering actually occurs.
 */
#define ELEMENT_IS_RENDERABLE(type) \
	((type >= FIRST_RENDERABLE) ? True : False)


/*********************************************************************
 * Nested structures                                                 *
 *********************************************************************/

#define AMI_SYSTEM_SCRIPT      0
#define AMI_NEWDISPLAY         1
#define AMI_NEWDISPLAY_REPLACE 2


typedef struct _AssociatedMenuItem {
        struct _AssociatedMenuItem *next;
	struct _AssociatedMenuItem *prev;
	char                       *label;
        int                        actionType;
        char                       *command;
        char                       *args;
} AssociatedMenuItem;

typedef struct {
	int                       clr;
        EdgeStyle                 style;
	FillStyle                 fill;
	unsigned int              width;
} DlBasicAttribute;

enum {
  DYNATTR_COLORMODE  = 1<<0,
  DYNATTR_VISIBILITY = 1<<1,
  DYNATTR_COLORRULE  = 1<<2,
  DYNATTR_CHANNEL    = 1<<3,
  DYNATTR_ALL        = DYNATTR_COLORMODE  |
                       DYNATTR_VISIBILITY |
                       DYNATTR_COLORRULE  |
                       DYNATTR_CHANNEL
};

typedef struct {
	ColorMode                 clr;
	VisibilityMode            vis;
        ColorRule               * colorRule;
	char                    * chan;
} DlDynamicAttribute;
	
typedef struct {
	int                       x;
	int                       y;
	unsigned int              width;
	unsigned int              height;
	XtPointer                 runtimeDescriptor;    /* -> element descriptor */
} DlObject;

typedef struct {
	char                    * rdbk;
	int                       clr;
	int                       bclr;
} DlMonitor;

typedef struct {
	char                    * ctrl;
	int                       clr;
	int                       bclr;
} DlControl;

typedef struct {
/*        DisplayLimitType    displayLimitType;*/
        double              displayLowLimit;
        double              displayHighLimit;
        int                 displayPrecision;
} DlOverrideFields;

/* Controler sensitivity */
typedef struct {
	char                    * chan;
	SensitivityMode           mode;
} DlSensitive;

typedef struct {
	char                    * title;
	char                    * xlabel;
	char                    * ylabel;
	int                       clr;
	int                       bclr;
	char                    * package;
} DlPlotcom;

typedef struct {
        CartesianPlotAxisStyle    axisStyle;
        CartesianPlotRangeStyle   rangeStyle;
        float                     minRange;
	float                     maxRange;
        CartesianPlotTimeFormat_t timeFormat;
} DlPlotAxisDefinition;

typedef struct {
	char                 * label;
	char                 * name;
	char                 * args;
        relatedDisplayMode_t   mode;
} DlRelatedDisplayEntry;

typedef struct {
	char               * label;
	char               * command;
	char               * args;
} DlShellCommandEntry;

typedef struct {
	int                 r, g, b;
	int                 inten;
} DlColormapEntry;

typedef struct {
	char              * chan;
	char              * utilChan;
	int                 clr;
} DlPen;

typedef struct {
	char              * xdata;
	char              * ydata;
	int                 data_clr;
} DlTrace;

/*********************************************************************
 * Top Level structures                                              *
 *********************************************************************/

typedef struct {
	char              * name;
        int                 versionNumber;
} DlFile;

typedef struct {
	DlObject            object;
	int                 clr;
	int                 bclr;
	DisplayType         displayType;
	char               * cmap;
} DlDisplay;

typedef struct {
	int                 ncolors;
	DlColormapEntry     dl_color[DL_MAX_COLORS];
} DlColormap;

/****** Shapes */

typedef struct {
	DlObject            object;
	DlBasicAttribute    attr;
        DlDynamicAttribute  dynAttr;
} DlRectangle;

typedef struct {
	DlObject            object;
	DlBasicAttribute    attr;
        DlDynamicAttribute  dynAttr;
} DlOval;

typedef struct {
	DlObject            object;
	DlBasicAttribute    attr;
        DlDynamicAttribute  dynAttr;
	int                 begin;
	int                 path;
} DlArc;

typedef struct {
	DlObject            object;
	DlBasicAttribute    attr;
        DlDynamicAttribute  dynAttr;
	char              * textix;
	Alignment           alignment;
} DlText;

typedef struct {
	DlObject             object;
        DlDynamicAttribute   dynAttr;
        int                  fit;
        GraphicRule        * graphicRule;
        int                  bclr;
        char               * opaque;
} DlDynSymbol;

typedef struct {
        DlObject               object;
        DlRelatedDisplayEntry  display[MAX_RELATED_DISPLAYS];
        int                    clr;
        int                    bclr;
        char                 * label;
        relatedDisplayVisual_t visual;
} DlRelatedDisplay;

typedef struct {
	DlObject            object;
	DlShellCommandEntry command[MAX_SHELL_COMMANDS];
	int                 clr;
	int                 bclr;
} DlShellCommand;

/****** Monitors */

typedef struct {
	DlObject            object;
	DlMonitor           monitor;
        DlOverrideFields     override;
	ColorMode           clrmod;
	Alignment           alignment;
	TextFormat          format;
} DlTextUpdate;

typedef struct {
	DlObject            object;
	DlMonitor           monitor;
        DlOverrideFields     override;
	LabelType           label;
	ColorMode           clrmod;
	Direction           direction;
} DlIndicator;

typedef struct {
	DlObject            object;
	DlMonitor           monitor;
        DlOverrideFields     override;
	LabelType           label;
	ColorMode           clrmod;
} DlMeter;

typedef struct {
	DlObject            object;
	DlMonitor           monitor;
        DlOverrideFields     override;
        DlDynamicAttribute  dynAttr;
	LabelType           label;
	Direction           direction;
	FillMode            fillmod;
        ScaleType           scaleType;
        ShowBar             barOnly;
        ShowAlarmLimits     showAlarmLimits;
        ShowScale           showScale;
} DlBar;

typedef struct {
        DlObject            object;
	DlMonitor           monitor;
	ColorMode           clrmod;
	Direction           direction;
	int                 sbit;
	int                 ebit;
} DlByte;

typedef struct {
	DlObject            object;
	DlPlotcom           plotcom;
        double              period;
	TimeUnits           units;
        double              delay;   /* the delay and oldUnits are 
				      * for compatible reason */

	TimeUnits           oldUnits; /* they will be removed for 
				       * future release */
	DlPen               pen[MAX_PENS];
} DlStripChart;

typedef struct {
	DlObject             object;
	DlPlotcom            plotcom;
	CartesianPlotStyle   style;
	EraseOldest          erase_oldest;
	int                  count;
	DlTrace              trace[MAX_TRACES];
	DlPlotAxisDefinition axis[3]; /* x = 0, y = 1, y2 = 2 */
	char                 *trigger;
	char                 *erase;
	eraseMode_t          eraseMode;
} DlCartesianPlot;

#define X_AXIS_ELEMENT	0
#define Y1_AXIS_ELEMENT	1
#define Y2_AXIS_ELEMENT	2

/****** Controllers */

typedef struct {
	DlObject             object;
	DlControl            control;
        DlOverrideFields     override;
	DlSensitive          sensitive;
	LabelType            label;
	ColorMode            clrmod;
	Direction            direction;
	double               dPrecision;
	/* private (run-time) data valuator needs for its operation */
	Boolean              enableUpdates;
	Boolean              dragging;
} DlValuator;

typedef struct {
	DlObject             object;
	DlControl            control;
	DlSensitive          sensitive;
	ColorMode            clrmod;
	Stacking             stacking;
} DlChoiceButton;

typedef struct {
	DlObject             object;
	DlControl            control;
	DlSensitive          sensitive;
	char               * label;
	char               * press_msg;
	char               * release_msg;
	ColorMode            clrmod;
	MessageButtonType    buttonType;
	int                  abclr;     /* toggle ON state background color */
	char               * alabel;    /* toggle ON state label */
	Dimension            shadowThickness;   /* shadow basic thickness */
	Dimension            borderWidth;       /* border basic width */
} DlMessageButton;

typedef struct {
	DlObject             object;
	DlControl            control;
	DlSensitive          sensitive;
	ColorMode            clrmod;
} DlMenu;

typedef struct {
	DlObject             object;
	DlControl            control;
	DlSensitive          sensitive;
	ColorMode            clrmod;
        TextFormat           format;
} DlTextEntry;

/****** Extensions */

typedef struct {
	DlObject             object;
	ImageType            imageType;
	char               * imageName;
	XtPointer            privateDataData;
} DlImage;

struct  _DlElement;
struct  _DlList;

typedef struct _DlComposite {
	DlObject             object;
	char               * compositeName;
	VisibilityMode       vis;
	char               * chan;
	struct _DlList     * dlElementList;
	Boolean              visible;	/* run-time visibility */
	AssociatedMenuItem * ami; 
} DlComposite;

/* (if DM2K ever leaves the X environment, a DlPoint should be defined and
 * substituted here for XPoint...) */

typedef struct {
	DlObject           object;
	DlBasicAttribute   attr;
        DlDynamicAttribute dynAttr;
	XPoint             *points;
	int                nPoints;
	int                isFallingOrRisingLine;
} DlPolyline;

typedef struct {
	DlObject           object;
	DlBasicAttribute   attr;
        DlDynamicAttribute dynAttr;
	XPoint             *points;
	int                nPoints;
} DlPolygon;

/*** NOTE:  DlObject must be first entry in each RENDERABLE structure!!!
 * display list in memory (with notion of composite/hierarchical structures)*/

typedef union {
  struct _DlElement *element;
  DlDisplay         *display;
  DlRectangle       *rectangle;
  DlOval            *oval;
  DlArc             *arc;
  DlText            *text;
  DlRelatedDisplay  *relatedDisplay;
  DlShellCommand    *shellCommand;
  DlTextUpdate      *textUpdate;
  DlIndicator       *indicator;
  DlMeter           *meter;
  DlBar             *bar;
  DlByte            *byte;
  DlStripChart      *stripChart;
  DlCartesianPlot   *cartesianPlot;
  DlValuator        *valuator;
  DlChoiceButton    *choiceButton;
  DlMessageButton   *messageButton;
  DlMenu            *menu;
  DlTextEntry       *textEntry;
  DlImage           *image;
  DlComposite       *composite;
  DlPolyline        *polyline;
  DlPolygon         *polygon;
  DlDynSymbol       *dynSymbol;
} DlStructurePtr;

struct _ResourceBundle;
struct _UpdateTask;


typedef struct {
  struct _DlElement *(*create)(struct _DlElement *);
  void (*destroy)           (struct _DlElement *);

  /* execute thyself method         */
  struct _UpdateTask * (*execute) (struct _DisplayInfo *, struct _DlElement *);

  /* write thyself (to file) method */
  void (*write)             (FILE *, struct _DlElement *, int); 

  void (*setValues)         (struct _ResourceBundle *, struct _DlElement *); 
  void (*getValues)         (struct _ResourceBundle *, struct _DlElement *); 
  void (*inheritValues)     (struct _ResourceBundle *, struct _DlElement *); 
  void (*setBackgroundColor)(struct _DlElement *, Pixel);
  void (*setForegroundColor)(struct _DlElement *, Pixel);
  void (*move)              (struct _DlElement *, int, int);
  void (*scale)             (struct _DlElement *, int, int);
  int  (*editVertex)        (struct _DlElement *, int, int);

  /* display object information (NULL : use the generic function) */
  void (*info)              (char *, Widget, struct _DisplayInfo *,
			     struct _DlElement *, XtPointer);
  /* entry point for deferred action (used in Dialog style display) */
  void (*deferredAction)    (struct _DlElement *, Boolean);
} DlDispatchTable; 


typedef struct _DlElement {
  DlElementType     type;
  DlStructurePtr    structure;
  struct _DisplayInfo *displayInfo; /* NULL or display of the element */
  DlDispatchTable   *run;
  Widget            widget;
  void              *data;
  struct _DlElement *next;     /* next element in display list   */
  struct _DlElement *prev;     /* previous element ...           */
  Boolean           actif;     /* True = active controler (when no widget) */
} DlElement;


typedef struct _DlList {
  DlElement *head;
  DlElement *tail;
  long      count;
} DlList;


#define FirstDlElement(x)    ((x)->head)
#define LastDlElement(x)     ((x)->tail)
#define IsEmpty(x)           ((x)->count <= 0)
#define NumberOfDlElement(x) ((x)->count)

#undef const
#endif
