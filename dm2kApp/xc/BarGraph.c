/*************************************************************************
 * Program: BarGraph.c                                                   *
 * Author : Paul D. Johnston, Mark Anderson, DMW Software.               *
 * Mods   : 1.0 Original                                                 *
 *          ?       - Integration with DM2K.                             *
 *          June 94 - Added bar graph fill from center.                  *
 *************************************************************************/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>

#include "Xc.h"
#include "Control.h"
#include "Value.h"
#include "BarGraphP.h"
#include "cvtFast.h"
#include "scale.h"

#ifndef MIN
# define  MIN(a,b)    (((a) < (b)) ? (a) :  (b))
#endif
#ifndef MAX
# define  MAX(a,b)    (((a) > (b)) ? (a) :  (b))
#endif

#define LONG_SEGMENT   5
#define SHORT_SEGMENT  3
#define WIDTH_RATIO    (0.6)
#define MARKER_SIZE(w) (w->barGraph.markers_num == 0 ? 0 :8)
#define BAR_SIGM_GAP   4
#define SHORT_S(w)     (SHORT_SEGMENT * (MIN(w->core.width,w->core.height)/100 + 1))
#define LONG_S(w)      (LONG_SEGMENT * (MIN(w->core.width,w->core.height)/100 + 1))

/****** Macro redefinition for offset. */
#define offset(field) XtOffset(BarGraphWidget, field)

/****** Declare widget methods */
static void ClassInitialize();
static void Initialize();
static void Redisplay();
static void Destroy();
static void Resize();
static XtGeometryResult QueryGeometry();
static Boolean SetValues();

/* Declare functions and variables private to this widget */
static void Draw_display();
static void Get_value();
static void Print_bounds();
static void DrawLogSegments();
static void DrawLinearSegments();
static TickDescription * AutoSegment();
static float GetDimentionValue();
static float GetDimentionValue2();

/* Define the widget's resource list */
static XtResource resources[] = {
  {
    XcNorient,
    XcCOrient,
    XcROrient,
    sizeof(XcOrient),
    offset(barGraph.orient),
    XtRString,
    "vertical"
  },
  {
    XcNfillmod,
    XcCFillmod,
    XcRFillmod,
    sizeof(XcFillmod),
    offset(barGraph.fillmod),
    XtRString,
    "from edge"
  },
  {
    XcNbarForeground,
    XtCColor,
    XtRPixel,
    sizeof(Pixel),
    offset(barGraph.bar_foreground),
    XtRString,
    XtDefaultForeground
  },
  {
    XcNbarBackground,
    XtCColor,
    XtRPixel,
    sizeof(Pixel),
    offset(barGraph.bar_background),
    XtRString,
    XtDefaultBackground
  },
  {
    XcNscaleColor,
    XtCColor,
    XtRPixel,
    sizeof(Pixel),
    offset(barGraph.scale_pixel),
    XtRString,
    XtDefaultForeground
  },
  {
    XcNscaleSegments,
    XcCScaleSegments,
    XtRInt,
    sizeof(int),
    offset(barGraph.num_segments),
    XtRImmediate,
    (XtPointer)7
  },
  {
    XcNvalueVisible,
    XcCValueVisible,
    XtRBoolean,
    sizeof(Boolean),
    offset(barGraph.value_visible),
    XtRString,
    "True"
  },
  {
    XcNinterval,
    XcCInterval,
    XtRInt,
    sizeof(int),
    offset(barGraph.interval),
    XtRImmediate,
    (XtPointer)0
  },
  {
    XcNupdateCallback,
    XtCCallback,
    XtRCallback,
    sizeof(XtPointer),
    offset(barGraph.update_callback),
    XtRCallback,
    NULL
  },
  {
    XcNautoSegment,
    XcCAutoSegment,
    XtRBoolean,
    sizeof(Boolean),
    offset(barGraph.auto_segment),
    XtRString,
    "True"
  },
  {
    XcNscaleFontList, 
    XcCScaleFontList, 
    XtRPointer,
    sizeof(XFontStruct **),
    offset(barGraph.scale_fonts), 
    XtRImmediate,
    (XtPointer)0
  },
  {
    XcNscaleFontListNum,
    XcCScaleFontListNum,
    XtRInt,
    sizeof(int),
    offset(barGraph.scale_fonts_num), 
    XtRImmediate,
    (XtPointer)0
  },
  {
    XcNbarOnly,
    XcCBarOnly,
    XtRBoolean,
    sizeof(Boolean),
    offset(barGraph.show_bar_only),
    XtRString,
    "False"
  },
  {
    XcNmarkers, 
    XcCMarkers, 
    XtRPointer,
    sizeof(double *),
    offset(barGraph.markers), 
    XtRImmediate,
    (XtPointer)0
  },
  {
    XcNmarkersColors, 
    XcCMarkersColors, 
    XtRPointer,
    sizeof(Pixel *),
    offset(barGraph.markers_colors), 
    XtRImmediate,
    (XtPointer)0
  },
  {
    XcNmarkersNum,
    XcCMarkersNum,
    XtRInt,
    sizeof(int),
    offset(barGraph.markers_num), 
    XtRImmediate,
    (XtPointer)0
  },
  {
    XcNlogScale,
    XcCLogScale,
    XtRBoolean,
    sizeof(Boolean),
    offset(barGraph.show_logarithm),
    XtRString,
    "False"
  },
};

/* Widget Class Record initialization */
BarGraphClassRec barGraphClassRec = {
  {
  /* core_class part */
    (WidgetClass) &valueClassRec,		/* superclass */
    "BarGraph",					/* class_name */
    sizeof(BarGraphRec),			/* widget_size */
    ClassInitialize,				/* class_initialize */
    NULL,					/* class_part_initialize */
    FALSE,					/* class_inited */
    Initialize,					/* initialize */
    NULL,					/* initialize_hook */
    XtInheritRealize,				/* realize */
    NULL,					/* actions */
    0,						/* num_actions */
    resources,					/* resources */
    XtNumber(resources),			/* num_resources */
    NULLQUARK,					/* xrm_class */
    TRUE,					/* compress_motion */
    TRUE,					/* compress_exposure */
    TRUE,					/* compress_enterleave */
    TRUE,					/* visible_interest */
    Destroy,					/* destroy */
    Resize,					/* resize */
    Redisplay,					/* expose */
    SetValues,					/* set_values */
    NULL,					/* set_values_hook */
    XtInheritSetValuesAlmost,			/* set_values_almost */
    NULL,					/* get_values_hook */
    NULL,					/* accept_focus */
    XtVersion,					/* version */
    NULL,					/* callback_private */
    NULL,					/* tm_table */
    QueryGeometry,				/* query_geometry */
    NULL,					/* display_accelerator */
    NULL,					/* extension */
  }, 
/****** Control class part, value class part, barGraph class part */
  { 0, }, { 0, }, { 0, }
};

WidgetClass xcBarGraphWidgetClass = (WidgetClass)&barGraphClassRec;

static void ClassInitialize() {
/*************************************************************************
 * ClassInitialize: This method initializes the BarGraph widget class.   *
 *   It registers resource value converter functions with Xt.            *
 *************************************************************************/
   XtAddConverter(XtRString, XcROrient, CvtStringToOrient, NULL, 0);
}

static void Initialize(request, new)
/*************************************************************************
 * Initialize: This is the initialize method for the BarGraph widget.    *
 *   It validates user-modifiable instance resources and initializes     *
 *   private widget variables and structures.  This function also        *
 *   creates any server resources (i.e., GCs, fonts, Pixmaps, etc.) used *
 *   by this widget.  This method is called by Xt when the application   *
 *   calls XtCreateWidget().                                             *
 *************************************************************************/
  BarGraphWidget request, new; {
    DPRINTF(("BarGraph: executing Initialize...\n"));

/****** Validate public instance variable settings.
        Check orientation resource setting. */
    if ((new->barGraph.orient != XcVert) && (new->barGraph.orient != XcHoriz)){
      XtWarning("BarGraph: invalid orientation setting");
      new->barGraph.orient = XcVert;
    }

/****** Check the interval resource setting. */
    if (new->barGraph.interval >0) {
      new->barGraph.interval_id = 
	XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)new),
          new->barGraph.interval, Get_value, new);
   }

/****** Check the scaleSegments resource setting. */
   if (new->barGraph.num_segments < MIN_SCALE_SEGS) {
      XtWarning("BarGraph: invalid number of scale segments");
      new->barGraph.num_segments = MIN_SCALE_SEGS;
   }
   else if (new->barGraph.num_segments > MAX_SCALE_SEGS) {
      XtWarning("BarGraph: invalid number of scale segments");
      new->barGraph.num_segments = MAX_SCALE_SEGS;
   }
   
/****** Check the valueVisible resource setting. */
   if ((new->barGraph.value_visible != True) &&
     (new->barGraph.value_visible != False)) {
      XtWarning("BarGraph: invalid valueVisible setting");
      new->barGraph.value_visible = True;
   }

/****** Check the autoSegment resource setting. */
   if ((new->barGraph.auto_segment != True) &&
     (new->barGraph.auto_segment != False)) {
      XtWarning("BarGraph: invalid autoSegment setting");
      new->barGraph.auto_segment = True;
   }

/* Initialize the BarGraph width and height. */
   if (new->core.width < MIN_BG_WIDTH) {
     if (new->barGraph.show_bar_only)
       new->core.width = MAX(1,new->core.width);
     else
       new->core.width = MIN_BG_WIDTH; 
   }

   if (new->core.height < MIN_BG_HEIGHT) {
     if (new->barGraph.show_bar_only)
       new->core.height = MAX(1,new->core.height);
     else
       new->core.height = MIN_BG_HEIGHT; 
   }


/* Markers local copy */
   if (new->barGraph.markers_num > 0) {
     double * dataCopy = (double*) 
                        calloc (new->barGraph.markers_num, sizeof(double));
     Pixel  * colorCopy = (Pixel*) 
                        calloc (new->barGraph.markers_num, sizeof(Pixel));
     register int i;

     if (dataCopy == NULL || colorCopy == NULL) {
       if (dataCopy != NULL)  free((char*) dataCopy);
       if (colorCopy != NULL) free((char*) colorCopy);

       new->barGraph.markers        = NULL;
       new->barGraph.markers_colors = NULL;
       new->barGraph.markers_num    = 0;
     }
     else {
       for (i = 0; i < new->barGraph.markers_num; i++) {
	 dataCopy[i]  = new->barGraph.markers[i];
	 colorCopy[i] = new->barGraph.markers_colors[i];
       }

       new->barGraph.markers_colors = colorCopy;
       new->barGraph.markers        = dataCopy;
     }
   }
   else {
     new->barGraph.markers_colors = NULL;
     new->barGraph.markers        = NULL;
   }

/* Set the initial geometry of the BarGraph elements. */
   Resize(new);

DPRINTF(("BarGraph: done Initialize\n"));
}

static void Redisplay(w, event)
/*************************************************************************
 * Redisplay : This function is the BarGraph's Expose method.  It redraws*
 *   the BarGraph's 3D rectangle background, Value Box, label, Bar       *
 *   indicator, and the Scale.  All drawing takes place within the       *
 *   widget's window (no need for an off-screen pixmap).                 *
 *************************************************************************/
  BarGraphWidget w;
  XExposeEvent *event; 
{
  int j;
  char upper[30], lower[30];

/****** Check to see whether or not the widget's window is mapped */
  if (!XtIsRealized((Widget)w) || !w->core.visible) return;
  DPRINTF(("BarGraph: executing Redisplay\n"));

  if (w->barGraph.show_bar_only) {
    Draw_display(w, XtDisplay(w), XtWindow(w), w->control.gc);
    DPRINTF(("BarGraph: done Redisplay\n"));
    return;
  }
  
/****** Draw the 3D rectangle background for the BarGraph
  XSetClipMask(XtDisplay(w), w->control.gc, None);
  Rect3d(w, XtDisplay(w), XtWindow(w), w->control.gc,
  0, 0, w->core.width, w->core.height, RAISED); */

/****** Draw the Label string */
  XSetClipRectangles(XtDisplay(w), w->control.gc, 0, 0, 
		     &(w->barGraph.face), 1, Unsorted); 

  if (!w->barGraph.noLabel) {
    XSetForeground(XtDisplay(w), w->control.gc, w->control.label_pixel);
    XDrawString(XtDisplay(w), XtWindow(w), w->control.gc,
		w->barGraph.lbl.x, w->barGraph.lbl.y, 
		w->control.label, strlen(w->control.label));
  }

/****** Draw the Scale */
  if (!w->barGraph.noScale) {
    if (w->barGraph.num_segments > 0) {
      XSetForeground(XtDisplay(w), w->control.gc, w->barGraph.scale_pixel);
      XDrawLine(XtDisplay(w), XtWindow(w), w->control.gc,
		w->barGraph.scale_line.x1, w->barGraph.scale_line.y1,
		w->barGraph.scale_line.x2, w->barGraph.scale_line.y2);

/****** Draw the max and min value segments */
#if 0
      if (w->barGraph.orient == XcVert) {
      } else {
	XDrawLine(XtDisplay(w), XtWindow(w), w->control.gc,
		  w->barGraph.scale_line.x1, 
		  w->barGraph.scale_line.y1 - w->barGraph.seg_length, 
		  w->barGraph.scale_line.x1, w->barGraph.scale_line.y1);
	XDrawLine(XtDisplay(w), XtWindow(w), w->control.gc,
		  w->barGraph.scale_line.x2, 
		  w->barGraph.scale_line.y2 - w->barGraph.seg_length, 
		  w->barGraph.scale_line.x2, w->barGraph.scale_line.y2);
    }
#endif

/****** Now draw the rest of the Scale segments */
    if (w->barGraph.show_logarithm) 
      DrawLogSegments(w);
    else if (w->barGraph.segmDesc != NULL)
      DrawLinearSegments(w);
    }
  }

/****** Draw markers indicators */
  if (w->barGraph.markers_num > 0) {
    int       mx;
    int       my;
    XGCValues values;
    XtGCMask  mask; 
    GC        gc = (GC)0; 
    
    if (w->barGraph.orient == XcVert) {
      mx = w->barGraph.face.x + w->barGraph.face.width - MARKER_SIZE(w);
      my = w->barGraph.scale_line.y2;
    } else {
      mx = w->barGraph.scale_line.x1;
      my = w->barGraph.face.y;
    }
    
    for (j = 0; j < w->barGraph.markers_num; j++) {
      float dim = GetDimentionValue(w, w->barGraph.markers[j]);
      
      if (dim < 0.0)
	continue;
      
      if (gc == (GC)0) {
	values.graphics_exposures = False;
	values.background = w->control.background_pixel;
	values.line_width = 2;
	
	mask = GCBackground | GCGraphicsExposures | GCLineWidth; 
	
	gc = XCreateGC(XtDisplay(w), RootWindowOfScreen(XtScreen(w)),
		       mask, &values);
      }
      
      XSetForeground(XtDisplay(w), gc, w->barGraph.markers_colors[j]);
      
      if (w->barGraph.orient == XcVert) {
	XDrawLine(XtDisplay(w), XtWindow(w), gc,
		  mx+1,                my - (int)dim,
		  mx+MARKER_SIZE(w)-1, my - (int)dim);
      } else {
	XDrawLine(XtDisplay(w), XtWindow(w), gc,
		  mx + (int)dim,  my+1,
		  mx + (int)dim , my+MARKER_SIZE(w)-1);
      }
    }
    
    if (gc != (GC)0)
      XFreeGC(XtDisplay(w),gc);
  }

  if (!w->barGraph.noScale) {
/****** Draw the max and min value string indicators */
    Print_bounds(w, upper, lower);
   
    XDrawString(XtDisplay(w), XtWindow(w), w->control.gc,
		w->barGraph.max_val.x, w->barGraph.max_val.y, 
		upper, strlen(upper)); 
    XDrawString(XtDisplay(w), XtWindow(w), w->control.gc,
		w->barGraph.min_val.x, w->barGraph.min_val.y,
		lower, strlen(lower)); 
  }

/****** Draw the Bar indicator border */
   Rect3d(w, XtDisplay(w), XtWindow(w), w->control.gc,
	  w->barGraph.bar.x - w->control.shade_depth, 
	  w->barGraph.bar.y - w->control.shade_depth,
	  w->barGraph.bar.width + (2 * w->control.shade_depth),  
	  w->barGraph.bar.height + (2 * w->control.shade_depth), DEPRESSED);

/****** Draw the Value Box */
   if (w->barGraph.value_visible == True && !w->barGraph.noScale)
     Rect3d(w, XtDisplay(w), XtWindow(w), w->control.gc,
	    w->value.value_box.x - w->control.shade_depth, 
	    w->value.value_box.y - w->control.shade_depth,
	    w->value.value_box.width + (2 * w->control.shade_depth),  
	    w->value.value_box.height + (2 * w->control.shade_depth), DEPRESSED);

/****** Draw the new values of Bar indicator and the value string */
   Draw_display(w, XtDisplay(w), XtWindow(w), w->control.gc);
   DPRINTF(("BarGraph: done Redisplay\n"));

}

/*******************************************************************
 NAME:		SetValues.
 DESCRIPTION:
   This is the set_values method for this widget. It validates resource
settings set with XtSetValues. If a resource is changed that would
require re-drawing the widget, return True.

*******************************************************************/
static Boolean SetValues(cur, req, new)
  BarGraphWidget cur, req, new; {
    Boolean do_redisplay = False, do_resize = False;

    DPRINTF(("BarGraph: executing SetValues\n"));

   if (new->core.width < MIN_BG_WIDTH) {
     if (new->barGraph.show_bar_only)
       new->core.width = MAX(1,new->core.width);
     else
       new->core.width = MIN_BG_WIDTH; 
     do_redisplay = True;
   }

   if (new->core.height < MIN_BG_HEIGHT) {
     if (new->barGraph.show_bar_only)
       new->core.height = MAX(1,new->core.height);
     else
       new->core.height = MIN_BG_HEIGHT; 
     do_redisplay = True;
   }


/* Check widget color resource settings. */
   if ((new->barGraph.bar_foreground != cur->barGraph.bar_foreground) ||
	(new->barGraph.bar_background != cur->barGraph.bar_background) ||
	(new->barGraph.scale_pixel != cur->barGraph.scale_pixel)) 
      do_redisplay = True;

/* Check orientation resource setting. */
   if (new->barGraph.orient != cur->barGraph.orient)
   {
      do_redisplay = True;
      if ((new->barGraph.orient != XcVert) && (new->barGraph.orient != XcHoriz))
      {
         XtWarning("BarGraph: invalid orientation setting");
         new->barGraph.orient = XcVert;
      }
   }

/* Check the interval resource setting. */
   if (new->barGraph.interval != cur->barGraph.interval) 
   {
      if (cur->barGraph.interval > 0)
	    XtRemoveTimeOut (cur->barGraph.interval_id);
      if (new->barGraph.interval > 0)
         new->barGraph.interval_id = 
		XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)new),
				new->barGraph.interval, Get_value, new);
   }

/* Keep the autoSegment resource uneditedable. */
   new->barGraph.auto_segment = cur->barGraph.auto_segment;

/* Check the scaleSegments resource setting. */
   if ( ! new->barGraph.auto_segment &&
	new->barGraph.num_segments != cur->barGraph.num_segments)
   {
      if (new->barGraph.num_segments < MIN_SCALE_SEGS)
      {
         XtWarning("BarGraph: invalid number of scale segments");
         new->barGraph.num_segments = MIN_SCALE_SEGS;
      }
      else if (new->barGraph.num_segments > MAX_SCALE_SEGS)
      {
         XtWarning("BarGraph: invalid number of scale segments");
         new->barGraph.num_segments = MAX_SCALE_SEGS;
      }
   }

/* Check the valueVisible resource setting. */
   if (new->barGraph.value_visible != cur->barGraph.value_visible)
   {
      do_redisplay = True;
      if ((new->barGraph.value_visible != True) &&
		(new->barGraph.value_visible != False))
      {
         XtWarning("BarGraph: invalid valueVisible setting");
         new->barGraph.value_visible = True;
      }
   }


/* Markers local copy */
   if (cur->barGraph.markers_num != new->barGraph.markers_num) {
     double * dataCopy;
     Pixel  * colorCopy;
     register int i;


     if (cur->barGraph.markers_colors != NULL) {
       free((char*)cur->barGraph.markers_colors);
       cur->barGraph.markers_colors = NULL;
     }

     if (cur->barGraph.markers != NULL) {
       free((char*)cur->barGraph.markers);
       cur->barGraph.markers = NULL;
     }

     if (new->barGraph.markers_num) {
       dataCopy = (double*) 
	 calloc (new->barGraph.markers_num, sizeof(double));
       colorCopy = (Pixel*) 
	 calloc (new->barGraph.markers_num, sizeof(Pixel));

       if (dataCopy == NULL || colorCopy == NULL) {
	 if (dataCopy != NULL) free((char*) dataCopy);
	 if (colorCopy != NULL) free((char*) colorCopy);
	 
	 new->barGraph.markers        =  NULL;
	 new->barGraph.markers_colors = NULL;
	 new->barGraph.markers_num    = 0;
       }
       else {
	 for (i = 0 ; i < new->barGraph.markers_num; i++) {
	   dataCopy[i]  = new->barGraph.markers[i];
	   colorCopy[i] = new->barGraph.markers_colors[i];
	 }
	 
	 new->barGraph.markers_colors = colorCopy;
	 new->barGraph.markers        = dataCopy;
       } 
     }
   }

/* Check to see if the value has changed. */
   if ((((new->value.datatype == XcLval) || (new->value.datatype == XcHval)) && 
		(new->value.val.lval != cur->value.val.lval))
      || ((new->value.datatype == XcFval) && 
		(new->value.val.fval != cur->value.val.fval)))
   {
      do_redisplay = True;
   }

/* Check to see whether we have to show just bar strip */
   if (new->barGraph.show_bar_only == cur->barGraph.show_bar_only) {
     if (XtIsRealized((Widget)new)) 
       XSetWindowBorder(XtDisplay(new),XtWindow(new), 0);
     new->core.border_width = 0;
     do_resize = True;
   }

/* (MDA) want to force resizing if min/max changed  or decimals setting */
   if (new->value.decimals != cur->value.decimals) do_resize = True;

   if ( ((new->value.datatype == XcLval) || (new->value.datatype == XcHval)) &&
        ( (new->value.lower_bound.lval != cur->value.lower_bound.lval) ||
          (new->value.upper_bound.lval != cur->value.upper_bound.lval) ))
   {
      do_resize = True;
   }
   else if ( (new->value.datatype == XcFval) &&
        ( (new->value.lower_bound.fval != cur->value.lower_bound.fval) ||
          (new->value.upper_bound.fval != cur->value.upper_bound.fval) ))
   {
      do_resize = True;
   }
   if (do_resize) {
     Resize(new);
     do_redisplay = True;
   }


DPRINTF(("BarGraph: done SetValues\n"));
   return do_redisplay;

}  /* end of SetValues */

static void Resize(w)
/*************************************************************************
 * Resize: This is the resize method of the BarGraph widget. It re-sizes *
 *   the BarGraph's graphics based on the new width and height of the    *
 *   widget's window.                                                    *
 *************************************************************************/
  BarGraphWidget w; 
{
  char          upper[30];
  char          lower[30];
  int           fontScaleHeight = 0;
  int           fontLabelHeight = 0;
  int           fontValueHeight = 0;
  int           shadeDepth = w->control.shade_depth;
  int           shadeDepth2 = 2*shadeDepth;
  XFontStruct * valueFont = w->control.font;
  XFontStruct * scaleFont = w->control.font;
  XFontStruct * labelFont = w->control.font;
  char        * label = w->control.label;
  int           labelStrlen = strlen(label);
  int           maxValueWidth;
  int           minValueWidth;
  int           maxScaleLabelWidth;


  DPRINTF(("BarGraph: executing Resize\n"));

  if (w->barGraph.show_bar_only) {
    w->barGraph.bar.x = 0;
    w->barGraph.bar.y = 0;
    w->barGraph.bar.width = w->core.width;
    w->barGraph.bar.height = w->core.height;
    return;
  }

  /* Heights of fonts 
   */
  fontScaleHeight = scaleFont->ascent; /* a number has not descent */

  if (w->barGraph.value_visible == True)
    fontValueHeight = valueFont->ascent; /* a number has not descent */

  if (labelStrlen > 1 || (labelStrlen == 1 && w->control.label[0] != ' ')) {
    fontLabelHeight = labelFont->ascent + labelFont->descent;
    w->barGraph.noLabel = False;
  }
  else
    w->barGraph.noLabel = True;

  /* Calculate min/max string attributes 
   */
   Print_bounds(w, upper, lower);

   maxValueWidth = XTextWidth(scaleFont, upper, strlen(upper));
   minValueWidth = XTextWidth(scaleFont, lower, strlen(lower));
   
   maxScaleLabelWidth = MAX(maxValueWidth,minValueWidth) + shadeDepth2;

   
   w->barGraph.noScale = !w->barGraph.auto_segment;
   w->barGraph.noValue = False;

   if (!w->barGraph.auto_segment)
     fontScaleHeight = 0;

   if (w->barGraph.orient == XcVert) 
   {
     if (w->core.width < 
	 maxScaleLabelWidth + 2*shadeDepth2 + BAR_SIGM_GAP +
	 MARKER_SIZE(w) + LONG_S(w)) 
       {
	 maxScaleLabelWidth = 0;
	 maxValueWidth      = 0;
	 minValueWidth      = 0;
	 
	 w->barGraph.noScale = True;
       }
   }


  /* Establish the new Value Box geometry. 
   */
  if (w->barGraph.value_visible == True && !w->barGraph.noValue) {
    w->value.value_box.x      = ((int)w->core.width - maxScaleLabelWidth)/2;
    w->value.value_box.y      = (int)w->core.height - fontValueHeight - shadeDepth;
    w->value.value_box.width  = maxScaleLabelWidth;
    w->value.value_box.height = fontValueHeight;

    /* Set the position of the displayed value within the Value Box. */
    Position_val(w);
  }


  /* Set width and height for face.
   */
  if (w->core.height < fontValueHeight + fontLabelHeight + 2*shadeDepth2) {
    w->barGraph.face.width = w->barGraph.face.height = 0;
  } 
  else {

    if (w->barGraph.noScale) {
      w->barGraph.face.x      = 0 ;
      w->barGraph.face.y      = fontLabelHeight != 0 ? fontLabelHeight + shadeDepth2 : 0;
      w->barGraph.face.width  = w->core.width;
      w->barGraph.face.height = w->core.height;
      w->barGraph.face.height -= fontValueHeight != 0 ? fontValueHeight + shadeDepth2 : 0 ;
    }
    else {
      w->barGraph.face.x      = shadeDepth;
      w->barGraph.face.y      = fontLabelHeight != 0 ? fontLabelHeight + shadeDepth2 : shadeDepth;
      w->barGraph.face.y++;

      w->barGraph.face.width  = w->core.width - 2*w->barGraph.face.x;

      w->barGraph.face.height = w->core.height - w->barGraph.face.y;
      w->barGraph.face.height -= fontValueHeight != 0 ? fontValueHeight + shadeDepth2 : shadeDepth;
      w->barGraph.face.height -= 2;
    }
  }


  /* Set the new label location. 
   */
  if (!w->barGraph.noLabel){
    w->barGraph.lbl.x = (short)(w->core.width -
				XTextWidth(labelFont, label, labelStrlen))/2;
    w->barGraph.lbl.y = (short)(shadeDepth + labelFont->ascent);
  }


  /* Resize the Bar indicator 
   */


  if (w->barGraph.orient == XcVert) 
  {
    if (w->barGraph.noScale) {
      w->barGraph.bar.x = (short)w->barGraph.face.x;
      w->barGraph.bar.x += (short)shadeDepth;
    
      w->barGraph.bar.y = (short)w->barGraph.face.y;
      w->barGraph.bar.y += (short) MAX(shadeDepth, fontScaleHeight/2);
    
      w->barGraph.bar.width = (unsigned short)w->barGraph.face.width;
      w->barGraph.bar.width -= (short) (shadeDepth2 + MARKER_SIZE(w));

      w->barGraph.bar.height = (unsigned short) w->barGraph.face.height;
      w->barGraph.bar.height -= 2*MAX(shadeDepth, fontScaleHeight/2);
    }
    else {
      w->barGraph.bar.x = (short)w->barGraph.face.x;
      w->barGraph.bar.x += (short)maxScaleLabelWidth; 
      w->barGraph.bar.x += (short)(w->barGraph.num_segments > 0 ? 
				   LONG_S(w)+BAR_SIGM_GAP : 0);
      w->barGraph.bar.x += (short)shadeDepth;
    
      w->barGraph.bar.y = (short)w->barGraph.face.y;
      w->barGraph.bar.y += (short) MAX(shadeDepth, fontScaleHeight/2);
    
      w->barGraph.bar.width = (unsigned short)w->barGraph.face.width;
      w->barGraph.bar.width += w->barGraph.face.x;
      w->barGraph.bar.width -= (short) (w->barGraph.bar.x + MARKER_SIZE(w));
      w->barGraph.bar.width -= shadeDepth;

      w->barGraph.bar.height = (unsigned short) w->barGraph.face.height;
      w->barGraph.bar.height -= 2*MAX(shadeDepth, fontScaleHeight/2);
    }
  }
  else 
  {
    if (w->barGraph.noScale) {
      w->barGraph.bar.x = (short)w->barGraph.face.x;
      w->barGraph.bar.x += (short)shadeDepth;

      w->barGraph.bar.y = (short)(w->barGraph.face.y + shadeDepth + MARKER_SIZE(w));
      w->barGraph.bar.width = (unsigned short)w->barGraph.face.width;
      w->barGraph.bar.width -= (short)shadeDepth2;

      w->barGraph.bar.height = (unsigned short) w->barGraph.face.height;
      w->barGraph.bar.height -= MARKER_SIZE(w) + shadeDepth2;
    }
    else {
      w->barGraph.bar.x = (short)w->barGraph.face.x;
      w->barGraph.bar.x += (short) MAX(shadeDepth, maxScaleLabelWidth/2);
      
      w->barGraph.bar.y = (short)(w->barGraph.face.y + shadeDepth + MARKER_SIZE(w));
      
      w->barGraph.bar.width = (unsigned short)w->barGraph.face.width;
      w->barGraph.bar.width -= (short) 
	MAX(shadeDepth2, w->barGraph.num_segments > 0 ? maxScaleLabelWidth : 0);
      
      w->barGraph.bar.height = (unsigned short) w->barGraph.face.height;
      w->barGraph.bar.height -= MARKER_SIZE(w) + shadeDepth2;
      w->barGraph.bar.height -= (short)(w->barGraph.num_segments > 0 ? 
			   LONG_S(w) + BAR_SIGM_GAP + fontScaleHeight: 0);
    }
  }


  if (!w->barGraph.noScale) 
  {
    /* Resize the Scale line. 
     */
    if (w->barGraph.orient == XcVert)
    {
      w->barGraph.scale_line.x1 = w->barGraph.bar.x - shadeDepth;
      w->barGraph.scale_line.x1 -= BAR_SIGM_GAP;
      w->barGraph.scale_line.y1 = w->barGraph.bar.y;
      w->barGraph.scale_line.x2 = w->barGraph.scale_line.x1;
      w->barGraph.scale_line.y2 = w->barGraph.bar.y + w->barGraph.bar.height;
    }
    else
    {
      w->barGraph.scale_line.x1 = w->barGraph.bar.x;
      w->barGraph.scale_line.y1 = w->barGraph.bar.y + w->barGraph.bar.height;
      w->barGraph.scale_line.y1 += shadeDepth + BAR_SIGM_GAP;
      
      w->barGraph.scale_line.x2 = w->barGraph.bar.x + w->barGraph.bar.width;
      w->barGraph.scale_line.y2 = w->barGraph.scale_line.y1;
    }
    
    /* Calculate the number of segments 
     */
    w->barGraph.segmDesc = AutoSegment(w);
    w->barGraph.num_segments = w->barGraph.segmDesc ?
      w->barGraph.segmDesc->ticks : 0;
    
    
    /* Set the position of the max and min value strings 
     */
    if (w->barGraph.orient == XcVert)
    {
      w->barGraph.max_val.x = w->barGraph.scale_line.x1 - LONG_S(w) - maxValueWidth;
      w->barGraph.max_val.y = w->barGraph.face.y + scaleFont->ascent;
      
      w->barGraph.min_val.x = w->barGraph.scale_line.x2 - LONG_S(w) - minValueWidth;
      w->barGraph.min_val.y = w->barGraph.face.y + w->barGraph.face.height - fontScaleHeight;
	w->barGraph.min_val.y += scaleFont->ascent;
    }
    else
    {
      w->barGraph.max_val.x = w->barGraph.scale_line.x2 - maxValueWidth/2;
      w->barGraph.max_val.y = w->barGraph.scale_line.y2 + LONG_S(w) + scaleFont->ascent;
      
      w->barGraph.min_val.x = w->barGraph.scale_line.x1 - minValueWidth/2;
      w->barGraph.min_val.y = w->barGraph.max_val.y;
    }
  }
}


/*******************************************************************
 NAME:		QueryGeometry.		
 DESCRIPTION:
   This function is the widget's query_geometry method.  It simply
checks the proposed size and returns the appropriate value based on
the proposed size.  If the proposed size is greater than the maximum
appropriate size for this widget, QueryGeometry returns the recommended
size.
*******************************************************************/

static XtGeometryResult QueryGeometry(w, proposed, answer)
  BarGraphWidget w;
  XtWidgetGeometry *proposed, *answer; {

/****** Set the request mode mask for the returned answer. */
   answer->request_mode = CWWidth | CWHeight;

/****** Set the recommended size. */
   answer->width = (w->core.width > MAX_BG_WIDTH)
	? MAX_BG_WIDTH : w->core.width;
   answer->height = (w->core.height > MAX_BG_HEIGHT)
	? MAX_BG_HEIGHT : w->core.height;

/* 
 * Check the proposed dimensions. If the proposed size is larger than
 * appropriate, return the recommended size.
 */
   if (((proposed->request_mode & (CWWidth | CWHeight)) == (CWWidth | CWHeight))
	&& proposed->width == answer->width 
	&& proposed->height == answer->height)
      return XtGeometryYes;
   else if (answer->width == w->core.width && answer->height == w->core.height)
      return XtGeometryNo;
   else
      return XtGeometryAlmost;

}  /* end of QueryGeometry */

/*******************************************************************
 NAME:		Destroy.
 DESCRIPTION:
   This function is the widget's destroy method.  It simply releases
any server resources acquired during the life of the widget.

*******************************************************************/
static void Destroy(w)
  BarGraphWidget w; 
{
  if (w->core.being_destroyed)
    return;

  if (w->barGraph.interval > 0) 
    XtRemoveTimeOut (w->barGraph.interval_id);
  
  if (w->barGraph.markers_colors) {
    free((char*)w->barGraph.markers_colors);
    w->barGraph.markers_colors = NULL;
  }

  if (w->barGraph.markers) {
    free((char*)w->barGraph.markers);
    w->barGraph.markers = NULL;
  }
}

/*******************************************************************
 NAME:		Get_value.		
 DESCRIPTION:
   This function is the time out procedure called at XcNinterval 
intervals.  It calls the application registered callback function to
get the latest value and updates the BarGraph display accordingly.

*******************************************************************/
static void Get_value(client_data, id)
XtPointer client_data;
XtIntervalId *id; {
    static XcCallData call_data;
    BarGraphWidget w = (BarGraphWidget)client_data;
   
/****** Get the new value by calling the application's callback if it exists */
   if (w->barGraph.update_callback == NULL) return;

/****** Re-register this TimeOut procedure for the next interval */
   if (w->barGraph.interval > 0)
      w->barGraph.interval_id = 
        XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)w),
        w->barGraph.interval, Get_value, client_data);

/* Set the widget's current value and datatype before calling the callback */
   call_data.dtype = w->value.datatype;
   call_data.decimals = w->value.decimals;
   if ((w->value.datatype == XcLval) || (w->value.datatype == XcHval))  
      call_data.value.lval = w->value.val.lval;
   else if (w->value.datatype == XcFval)
      call_data.value.fval = w->value.val.fval;
   XtCallCallbacks((Widget)w, XcNupdateCallback, &call_data);

/****** Update the new value, update the BarGraph display */
   if ((w->value.datatype == XcLval) || (w->value.datatype == XcHval))  
	w->value.val.lval = call_data.value.lval;
   else if (w->value.datatype == XcFval)
	w->value.val.fval = call_data.value.fval;

   if (XtIsRealized((Widget)w))
      Draw_display(w, XtDisplay(w), XtWindow(w), w->control.gc);
}

void XcBGUpdateValue(_w, value)
/*************************************************************************
 * Xc Bar Graph Update Value:  This convenience function is called by the*
 *   application in order to update the value (a little quicker than     *
 *   using XtSetArg/XtSetValue).  The application passes the new value   *
 *   to be updated with.                                                 *
 *************************************************************************/
  Widget _w;
  XcVType *value; {

    BarGraphWidget w = (BarGraphWidget) _w;
    
    if (!w->core.visible) return;

/****** Update the new value, then update the BarGraph display. */
   if (value != NULL) {
      if ((w->value.datatype == XcLval) || (w->value.datatype == XcHval))  
	 w->value.val.lval = value->lval;
      else if (w->value.datatype == XcFval)
	 w->value.val.fval = value->fval;

      if (XtIsRealized((Widget)w))
         Draw_display(w, XtDisplay(w), XtWindow(w), w->control.gc);
   }
}



/*******************************************************************
 NAME:		XcBGUpdateBarForeground.		
 DESCRIPTION:
   This convenience function is called by the application in order to
update the value (a little quicker than using XtSetArg/XtSetValue).
The application passes the new value to be updated with.

*******************************************************************/

void XcBGUpdateBarForeground(_w, pixel)
Widget _w;
unsigned long pixel; {

  BarGraphWidget w = (BarGraphWidget)_w;

/* Local variables */
    if (!w->core.visible) return;
    
/* Update the new value, then update the BarGraph display. */
   if (w->barGraph.bar_foreground != pixel) {
      w->barGraph.bar_foreground = pixel;
      if (XtIsRealized((Widget)w))
         Draw_display(w, XtDisplay(w), XtWindow(w), w->control.gc);
   }
}

static TickDescription * AutoSegment(w)
     BarGraphWidget w;
{
  Boolean           linear;
  int               fontSize;
  int               lPos, hPos;
  double            lVal = 0.0, hVal = 0.0;

  if (!w->barGraph.auto_segment)
    return NULL;

  if ((w->value.datatype == XcLval) || (w->value.datatype == XcHval)) {
    lVal = (double)w->value.lower_bound.lval;
    hVal = (double)w->value.upper_bound.lval;
  }
  else if (w->value.datatype == XcFval) {
    lVal = (double)w->value.lower_bound.fval;
    hVal = (double)w->value.upper_bound.fval;
  }

  if (w->barGraph.orient == XcVert)
    lPos = (float)(w->barGraph.bar.height);
  else 
    lPos = (float)(w->barGraph.bar.width);

  hPos = 0;

  return GetTickDescription(linear = True, fontSize=10,
			    lPos, hPos, lVal, hVal);
}


static void Draw_display( w, display, drawable, gc)
/*************************************************************************
 * Draw Display: This function redraws the Bar indicator and the value   *
 *   string in the Value Box.                                            *
 *************************************************************************/
  BarGraphWidget w;
  Display *display;
  Drawable drawable;
  GC gc; {
    int  xy;
    char *temp;
    float dim, range;

/****** Draw the Bar indicator, fill the Bar with its background color */
    XSetForeground(display, gc, w->barGraph.bar_background); 
    XFillRectangle(display, drawable, gc, 
		   w->barGraph.bar.x, w->barGraph.bar.y, 
		   w->barGraph.bar.width+1, w->barGraph.bar.height+1); 

    if (w->barGraph.orient == XcVert) range = (float)(w->barGraph.bar.height);
    else range = (float)(w->barGraph.bar.width);

/****** Figure dim value */
    dim = GetDimentionValue2(w, &w->value);

#if 0
    if ((w->value.datatype == XcLval) || (w->value.datatype == XcHval))
      dim = Correlate(((float)(w->value.val.lval)
      - (float)(w->value.lower_bound.lval)),
      ((float)(w->value.upper_bound.lval) - 
      (float)(w->value.lower_bound.lval)), range);
    else if (w->value.datatype == XcFval)
      dim = Correlate((w->value.val.fval - w->value.lower_bound.fval),
      (w->value.upper_bound.fval - w->value.lower_bound.fval), range);
#endif

/****** Draw the bar */
    if ((int)dim < 1) dim = 1;
    XSetForeground(display, gc, w->barGraph.bar_foreground); 
    if (w->barGraph.orient == XcVert) {
      if (w->barGraph.fillmod == XcCenter) {
        xy = w->barGraph.bar.height;
        if (dim > (float)(xy/2)) 
	  XFillRectangle(display, drawable, gc, 
			 w->barGraph.bar.x, (w->barGraph.bar.y + xy - (int)dim), 
			 w->barGraph.bar.width+1, (int)(dim - (xy/2))+1);
        else 
	  XFillRectangle(display, drawable, gc, 
			 w->barGraph.bar.x, (w->barGraph.bar.y + (int)(xy/2)), 
			 w->barGraph.bar.width+1, (int)((xy/2) - dim)+1);
      }
      else 
        XFillRectangle(display, drawable, gc, w->barGraph.bar.x,
		       (w->barGraph.bar.y + w->barGraph.bar.height - (int)dim),
		       w->barGraph.bar.width+1, (int)dim+1);
    }
    else {
      if (w->barGraph.fillmod == XcCenter) {
        xy = w->barGraph.bar.width;
        if (dim > (float)(xy/2)) 
	  XFillRectangle(display, drawable, gc, 
			 (w->barGraph.bar.x + (int)(xy/2)), w->barGraph.bar.y,
			 (int)(dim - (xy/2))+1, w->barGraph.bar.height+1);
        else 
	  XFillRectangle(display, drawable, gc, 
			 (w->barGraph.bar.x + (int)dim), w->barGraph.bar.y,
			 (int)((xy/2) - dim+1), w->barGraph.bar.height+1);
       }
       else 
	 XFillRectangle(display, drawable, gc, w->barGraph.bar.x,
			w->barGraph.bar.y, (int)dim+1, w->barGraph.bar.height+1);
    }

/****** If the value string is supposed to be displayed, draw it */
    if (w->barGraph.value_visible == True) {
/****** Clear the Value Box by re-filling it with its background color. */
      XSetForeground(display, gc, w->value.value_bg_pixel); 
      XFillRectangle(display, drawable, gc,
		     w->value.value_box.x, w->value.value_box.y, 
		     w->value.value_box.width+1, w->value.value_box.height+1); 

/****** Now draw the value string in its foreground color, clipped by the
        Value Box.  */
      XSetForeground(display, gc, w->value.value_fg_pixel); 
      XSetClipRectangles(display, gc, 0, 0, 
			 &(w->value.value_box), 1, Unsorted); 

      temp = Print_value(w->value.datatype, &w->value.val, w->value.decimals);
      
      Position_val(w);
      
      XDrawString(display, drawable, gc,
    	w->value.vp.x, w->value.vp.y, temp, strlen(temp)); 
    }

/****** Reset the clip_mask to no clipping */
    XSetClipMask(display, gc, None);

}

/*******************************************************************
 NAME:		Print_bounds.		
 DESCRIPTION:
   This is a utility function used by the Redisplay and Resize methods to
print the upper and lower bound values as strings for displaying and resizing
purposes.

*******************************************************************/

static void Print_bounds(w, upper, lower)
BarGraphWidget w;
char *upper, *lower;
{

   if (w->value.datatype == XcLval)
   {
      cvtLongToString(w->value.upper_bound.lval, upper);
      cvtLongToString(w->value.lower_bound.lval, lower);
   }
   else if (w->value.datatype == XcHval)
   {
      cvtLongToHexString(w->value.upper_bound.lval, upper);
      cvtLongToHexString(w->value.lower_bound.lval, lower);
   }
   else if (w->value.datatype == XcFval)
   {
      cvtFloatToString(w->value.upper_bound.fval, upper,
                                (unsigned short)w->value.decimals);
      cvtFloatToString(w->value.lower_bound.fval, lower,
                                (unsigned short)w->value.decimals);
   }

}

static int ShowLogSegment(w, low, high, value, flag)
     BarGraphWidget w;
     double         low;
     double         high;
     double         value;
     int            flag;
{
  static float range;
  static float compareTo;
  static int   longSegment;
  static int   shortSegment;
  int          dim;
  short        segmLength;

  if (flag == 0) {
    longSegment = LONG_S(w);
    shortSegment = SHORT_S(w);

    if (w->barGraph.orient == XcVert)
      range = (float)(w->barGraph.bar.height);
    else
      range = (float)(w->barGraph.bar.width);

    compareTo = log10(high) - log10(low);
    return 0;
  }

  if (value < low || value > high)
    return 0;

  dim = (int)Correlate((float)(log10(value) - log10(low)), compareTo, range);

  if (flag == 1) 
    segmLength = longSegment;
  else
    segmLength = shortSegment; 

  if (w->barGraph.orient == XcVert) {
    dim = w->barGraph.scale_line.y2 - (int)dim;
      
    XDrawLine(XtDisplay(w), XtWindow(w), w->control.gc,
	      w->barGraph.scale_line.x1 - segmLength, dim,
	      w->barGraph.scale_line.x1,              dim);
  }
  else {
    dim = w->barGraph.scale_line.x1 + (int)(dim);
    
    XDrawLine(XtDisplay(w), XtWindow(w), w->control.gc,
	      dim, w->barGraph.scale_line.y1,
	      dim, w->barGraph.scale_line.y1 + segmLength);
  }

  return dim;
}


static void DrawSegmentLabel(w, value, offset)
  BarGraphWidget w;
  float         value;
  int           offset;
{
  char  string[40];
  int   valueWidth;
  XFontStruct * scaleFont = w->control.font;
  short x;
  short y;
	  
  cvtFloatToString(value, string, (unsigned short)w->value.decimals);

  valueWidth = XTextWidth(scaleFont, string, strlen(string));
	  
  x = w->barGraph.scale_line.x1 - LONG_S(w) - valueWidth;
  y = offset + scaleFont->ascent/2;

  XDrawString(XtDisplay(w), XtWindow(w), w->control.gc,
	      x, y, string, strlen(string)); 
}


static void DrawLogSegments(w)
     BarGraphWidget w;
{
  typedef struct _SegmData {
    int    offset;
    float value;
  } SegmData; 

  register ValuePart * val = & w->value;
  double               low = 0.0, high = 0.0;
  int                  lowPower, highPower;
  int                  i, j;
  SegmData           * segments = NULL;
  int                  tmp;
  int                  n;
  int                  lastLongSegment = -1;
  int                  distanceBetweenLongSegments = -1;
  XFontStruct        * scaleFont = w->control.font;

  if ((val->datatype == XcLval) || (val->datatype == XcHval)) {
    high = (double)(val->upper_bound.lval);
    low  = (double)(val->lower_bound.lval);
  } 
  else if (val->datatype == XcFval) {
    high = (double)(val->upper_bound.fval);
    low  = (double)(val->lower_bound.fval);
  }

  low  = MAX(low, 1.0e-30);
  high = MAX(low, high);

  if (low == high)
    return;

  (void)ShowLogSegment(w, low, high, 0.0, 0);

  highPower = (int)ceil(log10(high));
  lowPower = (int)floor(log10(low));

  segments = (SegmData*) calloc 
    ((highPower-lowPower+1) * 9, sizeof(SegmData));

  n = 0;

  for (i = lowPower; i <= highPower; i++) {
    double d1 = pow(10.0, (double)i);

    tmp = ShowLogSegment(w, low, high, d1, 1);

    if (segments && tmp != 0) {

      if (lastLongSegment >= 0 && distanceBetweenLongSegments < 0) 
	distanceBetweenLongSegments = 
	  abs(tmp - segments[lastLongSegment].offset);

      lastLongSegment = n;

      segments[n].offset = tmp;
      segments[n].value = (float)d1;
      n++;
    }

    for (j = 2; j < 10; j++) {
      tmp = ShowLogSegment(w, low, high, (double)((double)j * d1), 2);
      if (segments && tmp != 0) {
	segments[n].offset = tmp;
	segments[n].value = (float)((double)j * d1);
	n++;
      }
    }
  }
  
  if (segments == NULL)
    return;

  {
    int minSize = 10000;

    for (i = n-1; i > 0; i--) {
      if (segments[i-1].offset != segments[i].offset)
	minSize = MIN(minSize, segments[i-1].offset - segments[i].offset);
    }

    if (minSize > w->control.font->ascent) 
      ;

    if (w->barGraph.orient == XcVert)
    {
      if (minSize > w->control.font->ascent) {
	for (i = 0; i < n; i++) {
	  DrawSegmentLabel(w, segments[i].value, segments[i].offset);
	} 
      } 
      else if (distanceBetweenLongSegments > scaleFont->ascent) {
	for (i = lastLongSegment; i > 0; i -= 9) {
	  DrawSegmentLabel(w, segments[i].value, segments[i].offset);
	}
      }
    }
  }
  
  if (segments)
    free((char*)segments);
}

static void DrawLinearSegments(w)
     BarGraphWidget w;
{
  float ratio;
  int   longSegment = LONG_S(w);
  int   shortSegment = SHORT_S(w);
  int   j;
  
  if (w->barGraph.orient == XcVert) 
    ratio = (float)w->barGraph.bar.height / 
      (float)(w->barGraph.segmDesc->ticks-1);
  else
    ratio = (float)w->barGraph.bar.width / 
      (float)(w->barGraph.segmDesc->ticks-1);
  
  for (j = 0; j < w->barGraph.segmDesc->ticks; j++)
  {
    register int segmLength;
    
    if (j % w->barGraph.segmDesc->ticksForLabel == 0)
      segmLength = longSegment;
    else
      segmLength = shortSegment;
    
    if (w->barGraph.orient == XcVert) {
      short y = w->barGraph.scale_line.y1 + (int)((float)j * ratio);
      
      XDrawLine(XtDisplay(w), XtWindow(w), w->control.gc,
		w->barGraph.scale_line.x1 - segmLength, y,
		w->barGraph.scale_line.x1, y);
    }
    else {
      short x = w->barGraph.scale_line.x1 + (int)((float)j * ratio);
      
      XDrawLine(XtDisplay(w), XtWindow(w), w->control.gc,
		x, w->barGraph.scale_line.y1,
		x, w->barGraph.scale_line.y1 + segmLength);
    }
  }
}

static float GetDimentionValue(w, data)
     BarGraphWidget w;
     double         data;
{
  register ValuePart * val = & w->value;
  float                dim = 0.0;
  float                value;
  float                range;
  float                compareTo;


  if (w->barGraph.orient == XcVert)
    range = (float)(w->barGraph.bar.height);
  else
    range = (float)(w->barGraph.bar.width);
  
  if ((val->datatype == XcLval) || (val->datatype == XcHval)) 
  {
    if (data > (double)(val->upper_bound.lval))
      return (float)-1.0;

    if (w->barGraph.show_logarithm) {
      value = (float)log10((double)val->lower_bound.lval);
      data = log10((double)data);
      compareTo = log10((double)val->upper_bound.lval) -
	          log10((double)val->lower_bound.lval);
    }
    else {
      value = (float)val->lower_bound.lval;
      compareTo = val->upper_bound.lval - val->lower_bound.lval;
    }

    dim = Correlate(data - value, compareTo, range);
  } 
  else if (val->datatype == XcFval) 
  {
    if (data > (double)(val->upper_bound.fval))
      return (float)-1.0;
    
    if (w->barGraph.show_logarithm) {
      value = (float)log10((double)val->lower_bound.fval);
      data = log10((double)data);
      compareTo = log10((double)val->upper_bound.fval) -
	          log10((double)val->lower_bound.fval);
    }
    else {
      value = (float)val->lower_bound.fval;
      compareTo = val->upper_bound.fval - val->lower_bound.fval;
    }

    dim = Correlate(data - value, compareTo, range);
  }
  return dim;
}

static float GetDimentionValue2(w, val)
     BarGraphWidget       w;
     register ValuePart * val;
{
  float data;
  float dim = 0.0;
  float value;
  float range;
  float compareTo;

  if (w->barGraph.orient == XcVert)
    range = (float)(w->barGraph.bar.height);
  else
    range = (float)(w->barGraph.bar.width);
  
  if ((val->datatype == XcLval) || (val->datatype == XcHval)) 
  {
    if (w->barGraph.show_logarithm) {
      value = (float)log10((double)val->lower_bound.lval);
      data = log10((double)w->value.val.lval);
      compareTo = log10((double)val->upper_bound.lval) -
	          log10((double)val->lower_bound.lval);
    }
    else {
      value = (float)val->lower_bound.lval;
      data = (float)(w->value.val.lval);
      compareTo = val->upper_bound.lval - val->lower_bound.lval;
    }

    dim = Correlate(data - value, compareTo, range);
  } 
  else if (val->datatype == XcFval) 
  {
    if (w->barGraph.show_logarithm) {
      value = (float)log10((double)val->lower_bound.fval);
      data = log10((double)w->value.val.fval);
      compareTo = log10((double)val->upper_bound.fval) -
	          log10((double)val->lower_bound.fval);
    }
    else {
      value = (float)val->lower_bound.fval;
      data = (float)(w->value.val.fval);
      compareTo = val->upper_bound.fval - val->lower_bound.fval;
    }

    dim = Correlate(data - value, compareTo, range);
  }
  return dim;
}
