/****************************************************************************
 FILE:		BarGraphP.h
 CONTENTS:	Private definitions for the BarGraph widget.
 AUTHOR:	Paul D. Johnston
 Date		Action
 ---------	------------------------------------
 15 Apr 91	Created.
 21 Jun 94      Modified to handle different fill modes
 ****************************************************************************/

#ifndef __XC_BARGRAPHP_H
#define __XC_BARGRAPHP_H

/****** Include the superclass private headers */
#include <X11/CoreP.h>
#include "Xc.h"
#include "ValueP.h"
#include "BarGraph.h"
#include "scale.h"

/****** Private declarations and definitions */
#define MIN_BG_WIDTH		14
#define MIN_BG_HEIGHT		14
#define MAX_BG_WIDTH		500
#define MAX_BG_HEIGHT		500
#define MIN_SCALE_SEGS		0
#define MAX_SCALE_SEGS		52

/****** Class part - minimum of one member required */
typedef struct {
   int dummy;
} BarGraphClassPart;

/****** Class record */
typedef struct _BarGraphClassRec {
   CoreClassPart core_class;
   ControlClassPart control_class;
   ValueClassPart value_class;
   BarGraphClassPart barGraph_class;
} BarGraphClassRec;

/****** Declare the widget class record as external for widget source file */
extern BarGraphClassRec barGraphClassRec;

/****** Instance part */
typedef struct {
   XcOrient orient;			/* BarGraph's orientation */
   XcFillmod fillmod;                   /* BarGraph's fill mode */
   int interval;			/* Time interval for updateCallback */
   XtCallbackList update_callback;	/* The updateCallback function */
   Pixel bar_background;		/* Background color of the bar */
   Pixel bar_foreground;		/* Foreground color of the bar */
   Pixel scale_pixel;			/* Color of the Scale indicator */
   int num_segments;			/* Number of segments in the Scale */
   Boolean auto_segment;                /* Calculate segments automaticaly */
   XFontStruct ** scale_fonts;          /* Array of fonts suitable for scale 
					 * labels */ 
   int            scale_fonts_num;      /* the number of fonts in the array */
   Boolean        show_bar_only;        /* Display just bar w/o scales and
					 * borders */

   Boolean        value_visible;        /* Enable/Disable display of the 
					 * value in the Value Box */
   double       * markers;
   Pixel        * markers_colors;
   int            markers_num;
   Boolean        show_logarithm;


/******* Private instance variables */
   XRectangle face;			/* Geometry of the BarGraph face */
   XPoint lbl;				/* Location of the Label string */
   XRectangle bar;			/* Rectangle for the Bar indicator */
   XSegment scale_line;			/* Scale line along Bar indicator */
   int seg_length;			/* Length of Scale line segments */
   XPoint max_val;			/* Point at which to draw the max
					 * value string on the Scale */
   XPoint min_val;			/* Point at which to draw the min
					 * value string on the Scale */
   int interval_id;			/* Xt TimeOut interval ID */
   TickDescription * segmDesc;
   Boolean           noScale;
   Boolean           noValue;
   Boolean           noLabel;
} BarGraphPart;

/****** Instance record */
typedef struct _BarGraphRec {
   CorePart core;
   ControlPart control;
   ValuePart value;
   BarGraphPart barGraph;
} BarGraphRec;

/****** Declare widget class functions here */

#endif  /* BARGRAPHP_H */

