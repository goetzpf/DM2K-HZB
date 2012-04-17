/*
 *      PlotterP.h
 *      Based on the AthenaTools Plotter Widget Set - Version 6.0
 *      Ge Lei, Aug 1999,               new resources added
 *                                      use font-id instead of font family
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Sun Jul 19 19:23:41 1992, patchlevel 1
 *                                      AtPlotterGetLegendWidth() added.
 *      klin, Mon Jul 27 14:17:43 1992, patchlevel 2
 *                                      Resources XtNlegendLeft and
 *                                      XtNautoRedisplay added.
 *                                      Resource XtNusePixmap and
 *                                      drawing to a pixmap added.
 *                                      Resource XtNuseCursor and
 *                                      callback cursors added.
 *                                      Resource XtNbusyCallback and
 *                                      busy callback added.
 *      klin, Sun Aug  2 18:24:27 1992, patchlevel 3
 *                                      Layout callback and some stuff for
 *                                      aligning axis positions added.
 *                                      Resource XtNtitleHeight and
 *                                      AtPlotterGetTitleHeight() added.
 *                                      Resources XtNxxxCursor added.
 *      klin, Sat Aug 15 10:25:48 1992, patchlevel 4
 *                                      Resources XtNslideCallback and
 *                                      XtNslideCursor and needed stuff added.
 *                                      Resources XtNselectCallback and
 *                                      XtNselectCursor and needed stuff added.
 *                                      Typos fixed.
 *                                      Changed <At/..> to <X11/At/..>.
 *
 *      SCCSid[] = "@(#) Plotter V6.0  92/08/15  PlotterP.h"
 */

/*

Copyright 1992 by University of Paderborn
Copyright 1990,1991 by the Massachusetts Institute of Technology

All rights reserved.

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of the firms, institutes
or employers of the authors not be used in advertising or publicity
pertaining to distribution of the software without specific, written
prior permission.

THE AUTHORS AND THEIR FIRMS, INSTITUTES OR EMPLOYERS DISCLAIM ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL THE AUTHORS AND THEIR FIRMS,
INSTITUTES OR EMPLOYERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*/

#ifndef _Jpt_PlotterP_h
#define _Jpt_PlotterP_h

/*#include <Xm/PrimitiveP.h>*/
#include <Xm/DrawingAP.h>

#include "At.h"
#include "Plotter.h"
#include "Text.h"
#include "AxisCore.h"
#include "DataHandle.h"
#include "Text2.h"

#include <X11/cursorfont.h>
#include <X11/ConstrainP.h>

#define DEFAULT_COLOR_TOTAL 6
#define PLOT_DATA_NEW 8
#define PLOT_DATA_CHANGED 4
#define PLOT_DATA_HANDLE 2
#define PLOT_DATA_TYPE 1
#define PLOT_DATA_ARRAY_HANDLE 2
#define PLOT_DATA_GENERAL_HANDLE 3
#define PLOT_DATA_ARRAY_CHANGED 6
#define PLOT_DATA_GENERAL_CHANGED 7

typedef enum {
     PLOT_NORMAL = 0,
     PLOT_ZOOM,
     PLOT_SCALE
} PlotZoomState;

typedef struct ranking{       /* A private data structure */
     AtPlotWidget   child;
     int            rank_order;
     struct ranking *prev;
     struct ranking *next;
} Rank;

typedef struct {
     float    xmin,  xmax;
     float    x2min, x2max;
     float    ymin,  ymax;
     float    y2min, y2max;
} AtPlotterBoundingBox;

/*
 *   All these coords are with 0,0 in top left.  This is the opposite
 *   to the intuitive graph numbering, so some care is definitly required....
 *
 *   The Y axis is along the LH edge, the Y2 axis the RH edge, the X
 *   axis is bottom, X2 axis is the top.  Point (x1,y1) is top LH
 *   corner, (x2,y2) is bottom RH corner.
 *
 */

typedef struct {
     int       x1, y1, x2, y2;               /* the plotting area */
     int      width, height;                /* the size of same */
     int       title_x, title_y;             /* where the title goes */
     int       legend_x, legend_y;           /* where the legend goes */
     int      title_width, title_height;
     int      legend_width, legend_height;  /* Size of legend */
} AtPlotterLayout;

typedef struct {
     int x1, x2, y1, y2;
} PlotArea;

typedef struct _AtPlotterPart {
     /* Resources */
     char              **title;
     Pixel               title_color;
     Pixel               plotarea_color;
     int                 title_x;
     int                 title_y;
     int                 title_width;
     int                 title_height;
     Boolean             title_x_dft;
     Boolean             title_y_dft;
     Boolean             show_title;
     PlotBorder          title_border;
     int                 title_border_width;

     Boolean             show_legend;
     Boolean             legend_left;
     Dimension           legend_width; 
     Dimension           legend_spacing;
     int                 legend_x;
     int                 legend_y;
     int                 legend_height;
     Boolean             legend_x_dft;
     Boolean             legend_y_dft;
     PlotBorder          legend_border;
     int                 legend_border_width;

     int                 graph_x;
     int                 graph_y;
     Dimension           graph_width;
     Dimension           graph_height;
     Boolean             graph_x_dft;
     Boolean             graph_y_dft;
     PlotBorder          graph_border;
     int                 graph_border_width;

     Dimension           margin_width;
     Dimension           margin_height;
     Boolean             rank_children;
     Boolean             auto_redisplay;
     Boolean             use_cursors;
     Boolean             use_pixmap;
     /* Cursors */
     Cursor              plotter_cursor;
     Cursor              busy_cursor;
     Cursor              click_cursor;
     Cursor              motion_cursor;
     Cursor              drag_cursor;
     Cursor              slide_cursor;
     Cursor              select_cursor;
     /* Callbacks */
     XtCallbackList      layout_callback;
     XtCallbackList      busy_callback;
     XtCallbackList      motion_callback;
     XtCallbackList      click_callback;
     XtCallbackList      drag_callback;
     XtCallbackList      slide_callback;
     XtCallbackList      select_callback;
     /* Private widgets. All are also children of this widget */
     AtAxisCoreWidget    xaxis;
     AtAxisCoreWidget    yaxis;
     AtAxisCoreWidget    x2axis;
     AtAxisCoreWidget    y2axis;
     
     /*XmPrimitive*/
     Pixel               xm_fore;  /* XmNforeground */
     Dimension           shadow_thickness; /* XmNshadowThickness */
     XtPointer           user_data; /* XmNuserData */
     /*Xrt Extend */
     PlotData          * plot_data;
     PlotData          * plot_data2;
     PlotDataStyle     **plot_data_styles;
     PlotDataStyle     **plot_data_styles2;
     PlotDataStyle      *marker_style;
     PlotDataStyle      *xmarker_style;
     PlotDataStyle      *ymarker_style;
     PlotMarkerMethod    xmarker_method;
     PlotMarkerMethod    ymarker_method;
     PlotType            plot_type;
     PlotType            plot_type2;

/*  Font and Color */
     Font                axis_font;
     Font                footer_font;
     Font                title_font;
     Font                legend_font;
     char *              widget_back;
     char *              widget_fore;
     char *              graph_back;
     char *              graph_fore;
     char *              data_back;  
     char *              data_fore;
     char *              title_back;
     char *              title_fore;
     char *              legend_back;
     char *              legend_fore;

     char             ** xlabel;
     char             ** ylabel;
     char             ** y2label;
     char              * xtitle;
     char              * ytitle;
     char              * y2title;
     float               xaxis_max;
     float               xaxis_min;
     float               yaxis_max;
     float               yaxis_min;
     float               y2axis_max;
     float               y2axis_min;
     float               xmax;
     float               xmin;
     float               ymax;
     float               ymin;
     float               y2max;
     float               y2min;
     float               xmarker;
     float               ymarker;
     float               xtic; /* subtic_interval of axiscore */
     float               ytic;
     float               y2tic;
     float               xnum; /* tic_interval of axiscore */
     float               ynum;
     float               y2num;
     int                 xprecision;
     int                 yprecision;
     int                 y2precision;     

     Boolean             xtic_dft;
     Boolean             ytic_dft;
     Boolean             y2tic_dft;
     Boolean             xnum_dft;
     Boolean             ynum_dft;
     Boolean             y2num_dft;
     Boolean             xaxis_max_dft;
     Boolean             yaxis_max_dft;
     Boolean             y2axis_max_dft;
     Boolean             xaxis_min_dft;
     Boolean             yaxis_min_dft;
     Boolean             y2axis_min_dft;
     Boolean             xmax_dft;
     Boolean             ymax_dft;
     Boolean             y2max_dft;
     Boolean             xmin_dft;
     Boolean             ymin_dft;
     Boolean             y2min_dft;
     Boolean             xprecision_dft;
     Boolean             yprecision_dft;
     Boolean             y2precision_dft;
     Boolean             xmarker_ds_dft; /*xmarker data style use default*/
     Boolean             ymarker_ds_dft; 
     Boolean             y2marker_ds_dft;
     Boolean             marker_ds_dft;
     Boolean             marker_show; 
     Boolean             xmarker_show;
     Boolean             ymarker_show;
     Boolean             data_styles_dft;
     Boolean             data_styles2_dft;
     Boolean             double_buffer;
     Boolean             draw_frame;
     Boolean             ylog;     /*if y axis use logarithmic*/
     Boolean             y2log;
     Boolean             accurate_tic_label;

     int                 xmarker_point;
     int                 xmarker_set;
     int                 ymarker_point;
     int                 ymarker_set;
     char                **set_labels; /*labels for the legends */
     char                **set_labels2; 

     time_t              time_base;
     char               *time_format;
     Boolean             time_format_dft; /*TimeFormatUseDefault*/
     PlotTimeUnit        time_unit;
     PlotAnnoMethod      xanno_method;   /*XtNplotXAnnotationMethod*/
     PlotAnnoMethod      yanno_method;   /*XtNplotYAnnotationMethod*/
     PlotAnnoMethod      y2anno_method;   /*XtNplotY2AnnotationMethod*/
     float               yaxis_const;   /*XtNplotYAxisConst*/
     float               yaxis_mult;    /*XtNplotYAxisMult*/

     int                 graph_margin_top;
     int                 graph_margin_bottom;
     int                 graph_margin_left;
     int                 graph_margin_right;
     Boolean             graph_margin_top_dft;
     Boolean             graph_margin_bottom_dft;
     Boolean             graph_margin_left_dft;
     Boolean             graph_margin_right_dft;

     /* Private state */
     Rank                *ordered_children;  /* rank ordered list of all children */
     /* TextList */
     TextList  *         text_list;
     GC                  title_gc;
     GC                  legend_gc;
     GC                  select_gc;
     GC                  drag_gc;
     GC                  plotarea_gc;
     GC                  graph_gc;    /* xrt graph gc */
     GC                  axis_gc;
     GC                  top_shadow_gc; /*XmPrimitive top shadow gc*/
     GC                  bottom_shadow_gc; /*XmPrimitive bottom shadow gc*/
     GC                  marker_gc;
     XFontStruct         *axis_fs;   /* font struct for axes */
     XFontStruct         *legend_fs;
     AtText              *title_text;
     AtPlotterLayout     layout;
     AtPlotterLayout     layout_bak;
     AtAxisPositions     positions;          /* Current axis positions */
     AtPlotterBoundingBox bounding_box;      /* As accepted by the axes */
     AtPlotterBoundingBox raw_bounding_box;  /* Raw from plot data */
     /* States for callbacks */
     Boolean             in_select;
     Boolean             in_click;
     Boolean             in_drag;
     Boolean             in_slide;
     Boolean             can_slide;
     Boolean             in_legend;
     /* Positions for callbacks */
     int                 drag_x;
     int                 drag_y;
     int                 drag_width;
     int                 drag_height;
     float              slide_x1;
     float              slide_y1;
     float              slide_x2;
     float              slide_y2;
     int                 selected_item;      /* Selected legend item */
     int                 selected_item_y;
     int                 legend_item;        /* Item in selection */
     int                 legend_item_y;
    
     int                title_strnum; /*number of title strings*/
     int                legend_num;  /*number of legend strings*/
     int                xlabel_strnum; /*number of strings for x label*/
     int                ylabel_strnum; /*number of strings for y label*/
     int                ds_num; /*number of data style*/
     int                ds2_num; /*number of data style2 */

     /* The state for the redraw processing */
     Boolean             expose_requested;
     Boolean             redraw_required;
     Boolean             rescale_required;
     Boolean             layout_required;
     Boolean             redisplay_required;
     Boolean             position_required;  /* Set axis positions */
     Boolean             in_layout_mode;     /* Help stop infinite loops! */
     /* The current and callback cursors */
     Cursor              current_cursor;
     Cursor              top_left_cursor;
     Cursor              top_right_cursor;
     Cursor              bottom_left_cursor;
     Cursor              bottom_right_cursor;
     short               cursor_mask;
     /* The pixmap and pixmap variables */
     Pixmap              pixmap;
     Pixmap              pixmap_backup;
     PlotZoomState       ifzoom;
     GC                  pixmap_gc;
     Dimension           pixmap_width;
     Dimension           pixmap_height;
     Boolean             pixmap_required;
     Boolean             displayed;
     Pixel               default_color[DEFAULT_COLOR_TOTAL+4];
     int                 default_color_num;

     Widget             *data_widget;
     Widget             *data2_widget;
     int                 data_widget_num;
     int                 data2_widget_num;
     PlotArea            axis_area;
     PlotArea            zoom_box;
     int                 zoom_limit;
} AtPlotterPart;

typedef struct _AtPlotterRec {
     CorePart            core;
     CompositePart       composite;
     ConstraintPart      constraint;
     /*XmManagerPart       manager;
     XmDrawingAreaPart   drawing_area; */
     AtPlotterPart       plotter;
} AtPlotterRec;

typedef struct _AtPlotterClassPart {
     int                 peek_a_boo;
} AtPlotterClassPart;

typedef struct _AtPlotterClassRec {
     CoreClassPart           core_class;
     CompositeClassPart      composite_class;
     ConstraintClassPart     constraint_class;
 /*    XmManagerClassPart      manager_class;
     XmDrawingAreaClassPart  drawing_area_class; */
     AtPlotterClassPart      plotter_class;
} AtPlotterClassRec;

extern AtPlotterClassRec atPlotterClassRec;

/*
 *   A linked list of from,to pairs for the extended list
 */

typedef struct _ExtendedList {
     struct _ExtendedList *next;
     int from, to;
} ExtendedList;

typedef struct _AtPlotterConstraintsPart {
     /* Resources */
     Boolean        displayed;
     String         legend_name;
     Boolean        use_x2_axis;
     Boolean        use_y2_axis;
     int            rank_order;

     /* Private  state */
     AtText         *legend_text;
     BoundingBox    bounding_box;
     /* This is the state kept for the redraw routine */
     Boolean        needs_refresh;
     ExtendedList   *extended_list;
} AtPlotterConstraintsPart;

typedef struct {
     AtPlotterConstraintsPart plotter;
} AtPlotterConstraintsRec, *AtPlotterConstraints;

#define ICON_WIDTH        16  /* Width of icons */
#define PIXMAP_ALIGNMENT 128  /* Alignment of pixmap width/height */
#define DEFAULT_WIDTH    256  /* Default plotter width */
#define DEFAULT_HEIGHT   128  /* Default plotter height */
#define NO_ITEM          -1   /* No item selected */

#define NUMCHILDREN(w) (w->composite.num_children)
#define CHILD(w,i) ((AtPlotWidget)(w->composite.children[i]))
#define CONSTRAINT(w,i) \
     ((AtPlotterConstraints)(((Widget) CHILD(w,i))->core.constraints))
#define CONSTRAINTS(cw) \
     ((AtPlotterConstraints)(((Widget) (cw))->core.constraints))

#define NTHCHILDISDISPLAYED(pw, i) (CONSTRAINT(pw, i)->plotter.displayed)
#define ISDISPLAYED(cw)  (CONSTRAINTS(cw)->plotter.displayed)
#define USESX2AXIS(cw)   (CONSTRAINTS(cw)->plotter.use_x2_axis)
#define USESY2AXIS(cw)   (CONSTRAINTS(cw)->plotter.use_y2_axis)

#define HasLayoutCB(pw) \
     (XtHasCallbacks((Widget) pw, XtNlayoutCallback) == XtCallbackHasSome)
#define HasBusyCB(pw) \
     (XtHasCallbacks((Widget) pw, XtNbusyCallback) == XtCallbackHasSome)
#define HasClickCB(pw) \
     (XtHasCallbacks((Widget) pw, XtNclickCallback) == XtCallbackHasSome)
#define HasMotionCB(pw) \
     (XtHasCallbacks((Widget) pw, XtNmotionCallback) == XtCallbackHasSome)
#define HasDragCB(pw) \
     (XtHasCallbacks((Widget) pw, XtNdragCallback) == XtCallbackHasSome)
#define HasSlideCB(pw) \
     (XtHasCallbacks((Widget) pw, XtNslideCallback) == XtCallbackHasSome)
#define HasSelectCB(pw) \
     (XtHasCallbacks((Widget) pw, XtNselectCallback) == XtCallbackHasSome)

/* The AtPlotterWidget parent of the child OBJECT cw */
#define ParentPlotter(cw) ((AtPlotterWidget) XtParent((Widget) (cw)))
#define AtNewString(str) str = XtNewString(str)
#define MARGIN      2    /* Pixels between elemnts - the same as axiscore */
#define X1Scale(pw, x) AtScalePixelToUser(pw->plotter.xaxis->axiscore.scale, x)
#define Y1Scale(pw, y) AtScalePixelToUser(pw->plotter.yaxis->axiscore.scale, y)
#define X2Scale(pw, x) AtScalePixelToUser(pw->plotter.x2axis->axiscore.scale, x)
#define Y2Scale(pw, y) AtScalePixelToUser(pw->plotter.y2axis->axiscore.scale, y)

extern void AxisZoomStart P((AtPlotterWidget pw, XButtonPressedEvent *event));
extern void GraphZoomStart P((AtPlotterWidget pw, XButtonPressedEvent *event));
extern void ZoomMotion P((AtPlotterWidget pw, XMotionEvent *event));
extern void AxisZoomEnd P((AtPlotterWidget pw, XButtonReleasedEvent *event));
extern void GraphZoomEnd P((AtPlotterWidget pw, XButtonReleasedEvent *event));
extern void Reset P((AtPlotterWidget pw, XEvent *event));
extern void ScaleStart P((AtPlotterWidget pw, XButtonPressedEvent *event));
extern void ScaleMotion P((AtPlotterWidget pw, XMotionEvent *event));

#endif /* _PlotterP_h */
