/*************************************************************************
* Jpt Jefferson Lab Plotting Toolkit 
*               Version 1.0 
* Copyright (c) 1998 Southeastern Universities Research Association,
*               Thomas Jefferson National Accelerator Facility
*
* This software was developed under a United States Government license
* described in the NOTICE file included as part of this distribution.
*
* Author: Ge Lei (leige@jlab.org)
*         
**************************************************************************/ 

/*
 *      At.h
 *      Common include file for all JPT files
 *      Based on the AthenaTools Plotter Widget Set - Version 6.0
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Sat Aug 15 10:31:50 1992, patchlevel 4
 *                                      Changed <At/..> to <X11/At/..>.
 *      klin, Fri Dec 11 15:55:12 1992, patchlevel 5
 *                                      Removed XtPointer macro.
 *                                      Include patchlevel.h for version defs.
 *      SCCSid[] = "@(#) Plotter V6.0  92/12/11  At.h"
 *      Copyright 1992 by University of Paderborn
 */


#ifndef _Jpt_At_h
#define _Jpt_At_h

#include <stdio.h>
#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __STDC__
# include <stdlib.h>
#endif

#include <math.h>
#include <assert.h>

#if  defined(SYSV) || defined(SVR4) || defined(VMS)
#include <string.h>
#else
#include <strings.h>
#endif

#include <float.h>
#define PLOTTER_HUGE_VAL   FLT_MAX
#define PLOT_HUGE_VAL   FLT_MAX

/*
#if (__STDC__ | WINNT | __VMS | VMS)
#include <float.h>
#define PLOTTER_HUGE_VAL   FLT_MAX
#else
#include <values.h>
#define PLOTTER_HUGE_VAL  MAXFLOAT
#endif
*/
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>

#if XtVersion >= 1106999
typedef char* caddr_t;
#endif

#ifndef P
# if defined (__STDC__) || defined (__cplusplus)
#  define P(args) args
# else
#  define P(args) ()
# endif
#endif

#ifndef Min
# define Min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef Max
# define Max(a, b) ((a) > (b) ? (a) : (b))
#endif

#define LIN_MIN 0.0
#define LIN_MAX 1.0
#define LOG_MIN 0.1
#define LOG_MAX 1.0
#define TIC_INT 1.0
#define TMSTR_MAXLEN 48

#define PlotStrings char **
#define PlotMarkerMethod PlotAnnoMethod

#define arr_nsets						a.nsets
#define arr_npoints						a.npoints
#define arr_data						a.data
#define arr_hole						a.hole
#define arr_xdata						arr_data.xp
#define arr_ydata						arr_data.yp
#define arr_xel(j)						arr_xdata[j]
#define arr_yel(i, j)					arr_ydata[i][j]

#define gen_nsets						g.nsets
#define gen_data						g.data
#define gen_hole						g.hole
#define gen_npoints(i)					gen_data[i].npoints
#define gen_xdata(i)					gen_data[i].xp
#define gen_ydata(i)					gen_data[i].yp
#define gen_xel(i, j)					gen_xdata(i)[j]
#define gen_yel(i, j)					gen_ydata(i)[j]


typedef struct {
  int pix_x, pix_y;
  int yaxis;
  float x, y;
  } PlotMapResult;

typedef enum  {
  PLOT_RGN_IN_GRAPH = -100,
  PLOT_RGN_IN_LEGEND,
  PLOT_RGN_IN_HEADER,
  PLOT_RGN_IN_FOOTER,
  PLOT_RGN_NOWHERE
  } PlotRegion;

typedef enum	{
	PLOT_PLOT = 0,
	PLOT_BAR,
	PLOT_PIE,
	PLOT_STACKING_BAR,
	PLOT_AREA
} PlotType;				/* types of basic graphs supported */

/*
 *   The plot line types
 */

typedef enum {
     AtPlotLINES = 0,             /* lines, default */
     AtPlotAREAS,
     AtPlotPOINTS,            /* points */
     AtPlotIMPULSES,          /* impulses */
     AtPlotSTEPS,             /* staircase */
     AtPlotBARS,              /* bars */
     AtPlotLINEPOINTS,        /* lines with points */
     AtPlotLINEIMPULSES,      /* lines with impulses */
     AtPlotINVALID            /* invalid: use default */
} AtPlotLineType;

/*
 *   The plot line styles
 */

typedef enum {
     AtLineNONE = 0,
     AtLineSOLID,             /* solid, default */
     AtLineDOTTED,            /* dotted 1 */
     AtLineDASHED,            /* dashed 2 */
     AtLineDOTDASHED,         /* dot dashed 1 */
     AtLineDOTTED2,           /* dotted 2 */
     AtLineDOTTED3,           /* dotted 3 */
     AtLineDOTTED4,           /* dotted 4 */
     AtLineDOTTED5,           /* dotted 5 */
     AtLineDASHED3,           /* dashed 3 */
     AtLineDASHED4,           /* dashed 4 */
     AtLineDASHED5,           /* dashed 5 */
     AtLineDOTDASHED2,        /* dot dashed 2 */
     AtLineINVALID            /* invalid: use default */
} AtPlotLineStyle;

/*
 *   The plot marker types
 */

typedef enum {
     AtMarkNONE = 0,
     AtMarkRECTANGLE,         /* rectangle, default */
     AtMarkPLUS,              /* plus sign */
     AtMarkXMARK,             /* x-mark sign */
     AtMarkSTAR,              /* star */
     AtMarkDIAMOND,           /* diamond */
     AtMarkDOT,
     AtMarkCIRCLE,
     AtMarkCROSS,
     AtMarkTRIANGLE1,         /* bottom triangle */
     AtMarkTRIANGLE2,         /* top triangle */
     AtMarkTRIANGLE3,         /* left triangle */
     AtMarkTRIANGLE4,         /* right triangle */
     AtMarkINVALID            /* invalid: use default */
} AtPlotMarkType;

/* PlotFillStyle */
typedef enum	{
	PLOT_FPAT_NONE = 0,
	PLOT_FPAT_SOLID,
	PLOT_FPAT_25_PERCENT,
	PLOT_FPAT_50_PERCENT,
	PLOT_FPAT_75_PERCENT,
	PLOT_FPAT_HORIZ_STRIPE,
	PLOT_FPAT_VERT_STRIPE,
	PLOT_FPAT_45_STRIPE,
	PLOT_FPAT_135_STRIPE,
	PLOT_FPAT_DIAG_HATCHED,
	PLOT_FPAT_CROSS_HATCHED
} AtPlotFillStyle;

typedef struct  {
	AtPlotLineStyle   lpat;          /* line pattern */
	AtPlotFillStyle   fpat;          /* fill pattern */
	char              *color;         /* line color   */
	int               width;         /* line width   */
	AtPlotMarkType    point;         /* point style  */
	char              *pcolor;        /* point color  */
	int               psize;         /* point size - pixels */
	unsigned long     res1;           /* reserved */
	unsigned long     res2;           /* reserved */
} PlotDataStyle;

typedef enum	{
	PLOT_BORDER_NONE = 0,
	PLOT_BORDER_SHADOW,
	PLOT_BORDER_PLAIN,
	PLOT_BORDER_ETCHED_IN = XmSHADOW_ETCHED_IN,
	PLOT_BORDER_ETCHED_OUT = XmSHADOW_ETCHED_OUT,
	PLOT_BORDER_3D_OUT = XmSHADOW_OUT,
	PLOT_BORDER_3D_IN = XmSHADOW_IN
} PlotBorder;

typedef enum	{
	PLOT_ANNO_VALUES = 0,
	PLOT_ANNO_POINT_LABELS,
	PLOT_ANNO_VALUE_LABELS,
	PLOT_ANNO_TIME_LABELS
} PlotAnnoMethod;

typedef enum {
	PLOT_TMUNIT_SECONDS = 1,
	PLOT_TMUNIT_MINUTES,
	PLOT_TMUNIT_HOURS,
	PLOT_TMUNIT_DAYS,
	PLOT_TMUNIT_WEEKS,
	PLOT_TMUNIT_MONTHS,
	PLOT_TMUNIT_YEARS
} PlotTimeUnit;

/*typedef enum	{
	PLOT_DATA_ARRAY =0,
	PLOT_DATA_GENERAL
} PlotDataType;*/
typedef int PlotDataType;
#define PLOT_DATA_ARRAY 0
#define PLOT_DATA_GENERAL 1

typedef struct	{		/* The data values in an PlotArray struct */
	float		 *xp;
	float		**yp;
} PlotArrayData;

typedef struct	{
	PlotDataType	 type;			
	float		 hole;
	int		 nsets;
	int		 npoints;
	PlotArrayData	 data;
} PlotArray;

typedef struct {			/* this is one set of general data */
	int		 npoints;
	float		*xp;
	float		*yp;
} PlotGeneralData;

typedef struct	{	
	PlotDataType	 type;			/* = PLOT_DATA_GENERAL */
	float		 hole;
	int		 nsets;
	PlotGeneralData	*data;
} PlotGeneral;

typedef union	{
	PlotArray		a;
	PlotGeneral		g;
} PlotData;

typedef PlotData * PlotDataHandle; 

typedef enum	{
	PLOT_TEXT_ATTACH_PIXEL = 0,
	PLOT_TEXT_ATTACH_VALUE,
	PLOT_TEXT_ATTACH_DATA,
	PLOT_TEXT_ATTACH_DATA_VALUE
} PlotAttachType;

typedef enum	{
	PLOT_ADJUST_LEFT = XmALIGNMENT_BEGINNING,
	PLOT_ADJUST_CENTER = XmALIGNMENT_CENTER,
	PLOT_ADJUST_RIGHT = XmALIGNMENT_END
} PlotAdjust;

typedef enum	{
	PLOT_ANCHOR_NORTH	= 0x10,
	PLOT_ANCHOR_SOUTH	= 0x20,
	PLOT_ANCHOR_EAST	= 0x01,
	PLOT_ANCHOR_WEST	= 0x02,
	PLOT_ANCHOR_NORTHEAST	= 0x11,
	PLOT_ANCHOR_NORTHWEST	= 0x12,
	PLOT_ANCHOR_SOUTHEAST	= 0x21,
	PLOT_ANCHOR_SOUTHWEST	= 0x22,
	/* for text areas only, not for legend */
	PLOT_ANCHOR_HOME	= 0x00,
	PLOT_ANCHOR_BEST	= 0x100
} PlotAnchor;


typedef union	{
	struct	{
		PlotAttachType	type;
		int	x, y;
	} pixel;
	struct	{
		PlotAttachType	    type;
		int		dataset;
		float		x, y;
	} value;
	struct	{
		PlotAttachType	type;
		int	dataset;
		int	set, point;
	} data;
	struct	{
		PlotAttachType	type;
		int	dataset;
		int	set, point;
		float	y;
	} data_value;
} PlotTextPosition;


typedef struct	{	/* Attached text structure */
	PlotTextPosition	  position;
	char			**strings;
	PlotAnchor		  anchor;
	int			  offset;
	int			  connected;
	PlotAdjust		  adjust;
	char			 *fore_color;
	char			 *back_color;
	PlotBorder	  	  border;
	int			  border_width;
	Font			  font;
	char			 *psfont;
	int			  psfont_size;
	XRectangle		  coords;	/* read-only */
} PlotTextDesc;


typedef void	*PlotTextHandle;	/* handle to attached text */

typedef enum {
    TEXT_NONE = 0,
    TEXT_CREATED,
    TEXT_NO_WINDOW,
    TEXT_ON,
    TEXT_OFF
    } TextHandleState;

typedef enum {
    UNDISPLAY = 0,
    TO_DISPLAY,
    DISPLAYED
    } TextDisplayState;

typedef struct {
   Boolean attached, positioned;
   TextDisplayState display_state;
   Pixel fore, back;
   GC          gc;
   Pixmap      newp;
   PlotTextDesc td;
   Position x, y, x1, y1, x2, y2;
   Dimension width, height;
   int str_num;
  } TextHandle;

typedef struct {
   TextHandle *th;
   void *next;
 } TextList;

/*
typedef enum  {
  XRT_RGN_IN_GRAPH = -100,
  XRT_RGN_IN_LEGEND,
  XRT_RGN_IN_HEADER,
  XRT_RGN_IN_FOOTER,
  XRT_RGN_NOWHERE
  } PlotRegion;

typedef enum	{
	XRT_BORDER_NONE = 0,
	XRT_BORDER_SHADOW,
	XRT_BORDER_PLAIN,
	XRT_BORDER_ETCHED_IN = XmSHADOW_ETCHED_IN,
	XRT_BORDER_ETCHED_OUT = XmSHADOW_ETCHED_OUT,
	XRT_BORDER_3D_OUT = XmSHADOW_OUT,
	XRT_BORDER_3D_IN = XmSHADOW_IN
} PlotBorder;

typedef enum	{
	XRT_ANNO_VALUES = 0,
	XRT_ANNO_POINT_LABELS,
	XRT_ANNO_VALUE_LABELS,
	XRT_ANNO_TIME_LABELS
} PlotAnnoMethod;

typedef enum {
	XRT_TMUNIT_SECONDS = 1,
	XRT_TMUNIT_MINUTES,
	XRT_TMUNIT_HOURS,
	XRT_TMUNIT_DAYS,
	XRT_TMUNIT_WEEKS,
	XRT_TMUNIT_MONTHS,
	XRT_TMUNIT_YEARS
} PlotTimeUnit;

typedef enum	{
	XRT_TEXT_ATTACH_PIXEL = 0,
	XRT_TEXT_ATTACH_VALUE,
	XRT_TEXT_ATTACH_DATA,
	XRT_TEXT_ATTACH_DATA_VALUE
} PlotAttachType;

typedef enum	{
	XRT_ADJUST_LEFT = XmALIGNMENT_BEGINNING,
	XRT_ADJUST_CENTER = XmALIGNMENT_CENTER,
	XRT_ADJUST_RIGHT = XmALIGNMENT_END
} PlotAdjust;

typedef enum	{
	XRT_ANCHOR_NORTH	= 0x10,
	XRT_ANCHOR_SOUTH	= 0x20,
	XRT_ANCHOR_EAST	= 0x01,
	XRT_ANCHOR_WEST	= 0x02,
	XRT_ANCHOR_NORTHEAST	= 0x11,
	XRT_ANCHOR_NORTHWEST	= 0x12,
	XRT_ANCHOR_SOUTHEAST	= 0x21,
	XRT_ANCHOR_SOUTHWEST	= 0x22,
	XRT_ANCHOR_HOME	= 0x00,
	XRT_ANCHOR_BEST	= 0x100
} PlotAnchor;
*/
/* function definition */
extern int strcasecmp  P((const char *, const char *));
/*extern int strncasecmp P((char *, char *, size_t));
*/

#ifdef __cplusplus
};
#endif

#endif /* _At_h */
