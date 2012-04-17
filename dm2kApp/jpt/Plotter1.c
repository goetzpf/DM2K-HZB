/*
 *      Plotter.c
 *      Based on the AthenaTools Plotter Widget Set - Version 6.0
 *      G.Lei Aug 1998,                 new resources and methods added
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Sun Jul 19 19:23:41 1992, patchlevel 1
 *                                      AtPlotterGetLegendWidth() added
 *      klin, Mon Jul 27 14:17:20 1992, patchlevel 2
 *                                      Resources XtNlegendLeft and
 *                                      XtNautoRedisplay added.
 *                                      Resource XtNusePixmap and
 *                                      drawing to a pixmap added.
 *                                      Resource XtNuseCursor and
 *                                      callback cursors added.
 *                                      Resource XtNbusyCallback and
 *                                      busy callback added.
 *      klin, Sun Aug  2 18:24:39 1992, patchlevel 3
 *                                      Layout callback and some stuff for
 *                                      aligning axis positions added.
 *                                      Resource XtNtitleHeigth and
 *                                      AtPlotterGetTitleHeigth() added.
 *                                      Resources XtNxxxCursor added.
 *                                      Callbacks for entering and leaving
 *                                      the plotter window added.
 *                                      Method query_geometry added.
 *      klin, Fri Aug  7 10:33:36 1992, Minor changes to keep
 *                                      ANSI C compilers quiet.
 *      klin, Sat Aug 15 10:08:25 1992, patchlevel 4
 *                                      Resources XtNslideCallback and
 *                                      XtNslideCursor and needed stuff added.
 *                                      Resources XtNselectCallback and
 *                                      XtNselectCursor and needed stuff added.
 *                                      Minor changes in callbacks.
 *                                      Changed <At/..> to <X11/At/..>.
 */
static char SCCSid[] = "@(#) Plotter V6.0  92/08/15  Plotter.c";

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
#include <time.h>
#include <X11/Xlib.h>
#include <X11/IntrinsicP.h>
#include <Xm/TransltnsP.h>
#include <Xm/XmStrDefs.h>
#include <Xm/XmP.h>
#include <Xm/ManagerP.h>
#include <Xm/DrawingAP.h>

#include "At.h"
#include "AtConverters.h"
#include "Scale.h"
#include "PlotterP.h"
#include "Plot.h"
#include "AxisCoreP.h"
#include "XYAxis.h"
#include "Text2.h"
#include "DataHandle.h"
#include "XYPlot.h"
#include "XYLinePlot.h"

/*
 *   The resources
 */
static float dflt_dbl = 0.0;
static float dflt_huge = PLOTTER_HUGE_VAL;
static int   dflt_pos = 0;
static int   margin_dft = 0;
#define off(field) XtOffset(AtPlotterWidget, plotter.field)
static XtResource plotter_resources[] = {
  {
     XtNplotHeaderStrings, XtCPlotStrings,
     XtRPlotStrings, sizeof(char **),
     off(title), XtRPlotStrings, (XtPointer) NULL
  },
  {
     XtNtitleColor, XtCForeground,
     XtRPixel, sizeof(Pixel),
     off(title_color), XtRString, (XtPointer) XtDefaultForeground
  },
  {
     XtNplotAreaColor, XtCForeground,
     XtRPixel, sizeof(Pixel),
     off(plotarea_color), XtRString, (XtPointer) XtDefaultBackground
  },
  {
     XtNshowTitle, XtCShowTitle,
     XtRBoolean, sizeof(Boolean),
     off(show_title), XtRImmediate, (XtPointer) True
  },
  {  
     XtNplotHeaderX, XtCPlotPosition,
     XtRInt, sizeof(int),
     off(title_x), XtRInt, (XtPointer) &dflt_pos
  },
  {  
     XtNplotHeaderY, XtCPlotPosition,
     XtRInt, sizeof(int),
     off(title_y), XtRInt, (XtPointer) &dflt_pos
  },
  {  
     XtNplotHeaderXUseDefault, XtCPlotPositionUseDefault,
     XtRBoolean, sizeof(Boolean),
     off(title_x_dft), XtRImmediate, (XtPointer) True
  },
  {  
     XtNplotHeaderYUseDefault, XtCPlotPositionUseDefault,
     XtRBoolean, sizeof(Boolean),
     off(title_y_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotHeaderHeight, XtCPlotDimension,
     XtRInt, sizeof(int),
     off(title_height), XtRInt, (XtPointer) &dflt_pos
  },
  {  
     XtNplotHeaderWidth, XtCPlotDimension,
     XtRInt, sizeof(int),
     off(title_width), XtRInt, (XtPointer) &dflt_pos
  },
  {
     XtNplotHeaderBorder, XtCPlotBorder,
     XtRInt, sizeof(int),
     off(title_border), XtRImmediate, (XtPointer) 0
  },
  {
     XtNplotHeaderBorderWidth, XtCPlotDimension,
     XtRInt, sizeof(int),
     off(title_border_width), XtRImmediate, (XtPointer) 0
  },
  {
     XtNplotLegendShow, XtCShowLegend,
     XtRBoolean, sizeof(Boolean),
     off(show_legend), XtRImmediate, (XtPointer) True
  },
  {
     XtNlegendLeft, XtCLegendLeft,
     XtRBoolean, sizeof(Boolean),
     off(legend_left), XtRImmediate, (XtPointer) False
  },
  {
     XtNlegendWidth, XtCLegendWidth,
     XtRDimension, sizeof(Dimension),
     off(legend_width), XtRImmediate, (XtPointer) 0
  },
  {
     XtNlegendSpacing, XtCMargin,
     XtRDimension, sizeof(Dimension),
     off(legend_spacing), XtRImmediate, (XtPointer) 2
  },
  {  
     XtNplotLegendX, XtCPlotPosition,
     XtRInt, sizeof(int),
     off(legend_x), XtRInt, (XtPointer) &dflt_pos
  },
  {  
     XtNplotLegendY, XtCPlotPosition,
     XtRInt, sizeof(int),
     off(legend_y), XtRInt, (XtPointer) &dflt_pos
  },
  {  
     XtNplotLegendXUseDefault, XtCPlotPositionUseDefault,
     XtRBoolean, sizeof(Boolean),
     off(legend_x_dft), XtRImmediate, (XtPointer) True
  },
  {  
     XtNplotLegendYUseDefault, XtCPlotPositionUseDefault,
     XtRBoolean, sizeof(Boolean),
     off(legend_y_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotLegendBorder, XtCPlotBorder,
     XtRInt, sizeof(int),
     off(legend_border), XtRImmediate, (XtPointer) 0
  },
  {
     XtNplotLegendBorderWidth, XtCPlotDimension,
     XtRInt, sizeof(int),
     off(legend_border_width), XtRImmediate, (XtPointer) 0
  },
  {  
     XtNplotGraphX, XtCPlotPosition,
     XtRInt, sizeof(int),
     off(graph_x), XtRInt, (XtPointer) &dflt_pos
  },
  {  
     XtNplotGraphY, XtCPlotPosition,
     XtRInt, sizeof(int),
     off(graph_y), XtRInt, (XtPointer) &dflt_pos
  },
  {  
     XtNplotGraphXUseDefault, XtCPlotPositionUseDefault,
     XtRBoolean, sizeof(Boolean),
     off(graph_x_dft), XtRImmediate, (XtPointer) True
  },
  {  
     XtNplotGraphYUseDefault, XtCPlotPositionUseDefault,
     XtRBoolean, sizeof(Boolean),
     off(graph_y_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotGraphBorder, XtCPlotBorder,
     XtRInt, sizeof(int),
     off(graph_border), XtRImmediate, (XtPointer) 0
  },
  {
     XtNplotGraphBorderWidth, XtCPlotDimension,
     XtRInt, sizeof(int),
     off(graph_border_width), XtRImmediate, (XtPointer) 0
  },
  {
     XtNmarginWidth, XtCMargin,
     XtRDimension, sizeof(Dimension),
     off(margin_width), XtRImmediate, (XtPointer) 1
  },
  {
     XtNmarginHeight, XtCMargin,
     XtRDimension, sizeof(Dimension),
     off(margin_height), XtRImmediate, (XtPointer) 1
  },
  {
     XtNrankChildren, XtCRankChildren,
     XtRBoolean, sizeof(Boolean),
     off(rank_children), XtRImmediate, (XtPointer) False
  },
  {
     XtNautoRedisplay, XtCAutoRedisplay,
     XtRBoolean, sizeof(Boolean),
     off(auto_redisplay), XtRImmediate, (XtPointer) True
  },
  {
     XtNuseCursors, XtCUseCursors,
     XtRBoolean, sizeof(Boolean),
     off(use_cursors), XtRImmediate, (XtPointer) True
  },
  /* The plotter cursors */
  {
     XtNplotterCursor, XtCCursor,
     XtRCursor, sizeof(Cursor),
     off(plotter_cursor), XtRString, (XtPointer) None
  },
  {
     XtNbusyCursor, XtCCursor,
     XtRCursor, sizeof(Cursor),
     off(busy_cursor), XtRString, (XtPointer) "watch"
  },
  {
     XtNmotionCursor, XtCCursor,
     XtRCursor, sizeof(Cursor),
     off(motion_cursor), XtRString, (XtPointer) "crosshair"
  },
  {
     XtNclickCursor, XtCCursor,
     XtRCursor, sizeof(Cursor),
     off(click_cursor), XtRString, (XtPointer) "crosshair"
  },
  {
     XtNdragCursor, XtCCursor,
     XtRCursor, sizeof(Cursor),
     off(drag_cursor), XtRString, (XtPointer) "crosshair"
  },
  {
     XtNslideCursor, XtCCursor,
     XtRCursor, sizeof(Cursor),
     off(slide_cursor), XtRString, (XtPointer) "fleur"
  },
  {
     XtNselectCursor, XtCCursor,
     XtRCursor, sizeof(Cursor),
     off(select_cursor), XtRString, (XtPointer) "hand1"
  },
  /* The plotter callbacks */
  {
     XtNlayoutCallback, XtCCallback,
     XtRCallback, sizeof(XtCallbackList),
     off(layout_callback), XtRCallback, (XtPointer) NULL
  },
  {
     XtNbusyCallback, XtCCallback,
     XtRCallback, sizeof(XtCallbackList),
     off(busy_callback), XtRCallback, (XtPointer) NULL
  },
  {
     XtNmotionCallback, XtCCallback,
     XtRCallback, sizeof(XtCallbackList),
     off(motion_callback), XtRCallback, (XtPointer) NULL
  },
  {
     XtNclickCallback, XtCCallback,
     XtRCallback, sizeof(XtCallbackList),
     off(click_callback), XtRCallback, (XtPointer) NULL
  },
  {
     XtNdragCallback, XtCCallback,
     XtRCallback, sizeof(XtCallbackList),
     off(drag_callback), XtRCallback, (XtPointer) NULL
  },
  {
     XtNslideCallback, XtCCallback,
     XtRCallback, sizeof(XtCallbackList),
     off(slide_callback), XtRCallback, (XtPointer) NULL
  },
  {
     XtNselectCallback, XtCCallback,
     XtRCallback, sizeof(XtCallbackList),
     off(select_callback), XtRCallback, (XtPointer) NULL
  },
  /*XmPrimitive*/
  {
     XmNshadowThickness, XmCShadowThickness,
     XtRDimension, sizeof(Dimension),
     off(shadow_thickness), XtRImmediate, (XtPointer)1
  }, 
  {
     XmNuserData, XmCUserData,
     XtRPointer, sizeof(XtPointer),
     off(user_data), XtRPointer, (XtPointer)NULL
  },
  {
     XtNplotData, XtCPlotData,
     XtRPointer, sizeof(PlotData *),
     off(plot_data), XtRImmediate, (XtPointer) NULL
  } ,
  {
     XtNplotData2, XtCPlotData,
     XtRPointer, sizeof(PlotData *),
     off(plot_data2), XtRImmediate, (XtPointer) NULL
  } ,
  {
     XtNplotDataStyles, XtCPlotDataStyles,
     XtRPointer, sizeof(PlotDataStyle **),
     off(plot_data_styles), XtRPointer, (XtPointer)NULL
  },
  {
     XtNplotDataStyles2, XtCPlotDataStyles,
     XtRPointer, sizeof(PlotDataStyle **),
     off(plot_data_styles2), XtRPointer, (XtPointer)NULL
  },
  {
     XtNplotMarkerDataStyle, XtCPlotDataStyle,
     XtRPointer, sizeof(PlotDataStyle *),
     off(marker_style), XtRPointer, (XtPointer)NULL
  },
  {
     XtNplotXMarkerDataStyle, XtCPlotDataStyle,
     XtRPointer, sizeof(PlotDataStyle *),
     off(xmarker_style), XtRPointer, (XtPointer)NULL
  },
  {
     XtNplotYMarkerDataStyle, XtCPlotDataStyle,
     XtRPointer, sizeof(PlotDataStyle *),
     off(ymarker_style), XtRPointer, (XtPointer)NULL
  },
  {
     XtNplotXMarkerMethod, XtCPlotMarkerMethod,
     XtRInt, sizeof(int),
     off(xmarker_method), XtRInt, (XtPointer) 0
  },
  {
     XtNplotYMarkerMethod, XtCPlotMarkerMethod,
     XtRInt, sizeof(int),
     off(ymarker_method), XtRInt, (XtPointer) 0
  },
  {
     XtNplotType, XtCPlotType,
     XtRInt, sizeof(PlotType),
     off(plot_type), XtRImmediate, (XtPointer) PLOT_PLOT
  },
  {
     XtNplotType2, XtCPlotType,
     XtRInt, sizeof(PlotType),
     off(plot_type2), XtRImmediate, (XtPointer) PLOT_PLOT
  },
  {
     XtNplotDataStylesUseDefault, XtCPlotDataStylesUseDefault,
     XtRBoolean, sizeof(Boolean),
     off(data_styles_dft), XtRImmediate, (XtPointer)True
  },
  {
     XtNplotDataStyles2UseDefault, XtCPlotDataStylesUseDefault,
     XtRBoolean, sizeof(Boolean),
     off(data_styles2_dft), XtRImmediate, (XtPointer)True
  },
  {
     XtNplotDoubleBuffer, XtCPlotDoubleBuffer,
     XtRBoolean, sizeof(Boolean),
     off(double_buffer), XtRImmediate, (XtPointer)False
  },
  {
     XtNplotAxisBoundingBox, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(draw_frame), XtRImmediate, (XtPointer)False
  },
  {  XtNplotAxisFont, XtCFont,
     XtRInt, sizeof(Font),
     off(axis_font), XtRImmediate, NULL
  },
  {  XtNplotFooterFont, XtCFont,
     XtRInt, sizeof(Font),
     off(footer_font), XtRImmediate, NULL
  },
  {  XtNplotHeaderFont, XtCFont,
     XtRInt, sizeof(Font),
     off(title_font), XtRImmediate, NULL
  },
  {  XtNplotLegendFont, XtCFont,
     XtRInt, sizeof(Font),
     off(legend_font), XtRImmediate, NULL
  },
  {
     XtNplotBackgroundColor, XtCPlotBackgroundColor,
     XtRString, sizeof(String),
     off(widget_back), XtRString, "White"
  },
  {
     XtNplotForegroundColor, XtCPlotForegroundColor,
     XtRString, sizeof(String),
     off(widget_fore), XtRString, (XtPointer)"Black"
  },
  {
     XtNplotGraphBackgroundColor, XtCPlotBackgroundColor,
     XtRString, sizeof(String),
     off(graph_back), XtRString, (XtPointer)NULL
  },
  {
     XtNplotGraphForegroundColor, XtCPlotForegroundColor,
     XtRString, sizeof(String),
     off(graph_fore), XtRString, (XtPointer)NULL
  }, 
  {
     XtNplotDataAreaBackgroundColor, XtCPlotBackgroundColor,
     XtRString, sizeof(String),
     off(data_back), XtRString, (XtPointer)NULL
  },
  {
     XtNplotDataAreaForegroundColor, XtCPlotForegroundColor,
     XtRString, sizeof(String),
     off(data_fore), XtRString, (XtPointer)NULL
  },
  {
     XtNplotHeaderBackgroundColor, XtCPlotBackgroundColor,
     XtRString, sizeof(String),
     off(title_back), XtRString, (XtPointer)NULL
  },
  {
     XtNplotHeaderForegroundColor, XtCPlotForegroundColor,
     XtRString, sizeof(String),
     off(title_fore), XtRString, (XtPointer)NULL
  },
  {
     XtNplotLegendBackgroundColor, XtCPlotBackgroundColor,
     XtRString, sizeof(String),
     off(legend_back), XtRString, (XtPointer)NULL
  },
  {
     XtNplotLegendForegroundColor, XtCPlotForegroundColor,
     XtRString, sizeof(String),
     off(legend_fore), XtRString, (XtPointer)NULL
  }, 
  {
     XtNplotXLabels, XtCPlotStrings,
     XtRPlotStrings, sizeof(char **),
     off(xlabel), XtRPlotStrings, (XtPointer)NULL
  },
  {
     XtNplotYLabels, XtCPlotStrings,
     XtRPlotStrings, sizeof(char **),
     off(ylabel), XtRPlotStrings, (XtPointer)NULL
  },
  {
     XtNplotXTitle, XtCPlotXTitle,
     XtRString, sizeof(char *),
     off(xtitle), XtRString, (XtPointer)NULL
  },
  {
     XtNplotYTitle, XtCPlotYTitle,
     XtRString, sizeof(char *),
     off(ytitle), XtRString, (XtPointer)NULL
  },
  {
     XtNplotY2Title, XtCPlotYTitle,
     XtRString, sizeof(char *),
     off(y2title), XtRString, (XtPointer)NULL
  },
  {  XtNplotXAxisMax, XtCPlotAxisBounds,
     XtRFloat, sizeof(float),
     off(xaxis_max), XtRFloat, (XtPointer) &dflt_huge
  },
  {  XtNplotXAxisMin, XtCPlotAxisBounds,
     XtRFloat, sizeof(float),
     off(xaxis_min), XtRFloat, (XtPointer) &dflt_huge
  },
  {  XtNplotYAxisMax, XtCPlotAxisBounds,
     XtRFloat, sizeof(float),
     off(yaxis_max), XtRFloat, (XtPointer) &dflt_huge
  },
  {  XtNplotYAxisMin, XtCPlotAxisBounds,
     XtRFloat, sizeof(float),
     off(yaxis_min), XtRFloat, (XtPointer) &dflt_huge
  },
  {  XtNplotY2AxisMax, XtCPlotAxisBounds,
     XtRFloat, sizeof(float),
     off(y2axis_max), XtRFloat, (XtPointer) &dflt_huge
  },
  {  XtNplotY2AxisMin, XtCPlotAxisBounds,
     XtRFloat, sizeof(float),
     off(y2axis_min), XtRFloat, (XtPointer) &dflt_huge
  },
  {  XtNplotXMax, XtCPlotAxisBounds,
     XtRFloat, sizeof(float),
     off(xmax), XtRFloat, (XtPointer) &dflt_huge
  },
  {  XtNplotXMin, XtCPlotAxisBounds,
     XtRFloat, sizeof(float),
     off(xmin), XtRFloat, (XtPointer) &dflt_huge
  },
  {  XtNplotYMax, XtCPlotAxisBounds,
     XtRFloat, sizeof(float),
     off(ymax), XtRFloat, (XtPointer) &dflt_huge
  },
  {  XtNplotYMin, XtCPlotAxisBounds,
     XtRFloat, sizeof(float),
     off(ymin), XtRFloat, (XtPointer) &dflt_huge
  },
  {  XtNplotY2Max, XtCPlotAxisBounds,
     XtRFloat, sizeof(float),
     off(y2max), XtRFloat, (XtPointer) &dflt_huge
  },
  {  XtNplotY2Min, XtCPlotAxisBounds,
     XtRFloat, sizeof(float),
     off(y2min), XtRFloat, (XtPointer) &dflt_huge
  },
  {  XtNplotXMarker, XtCPlotMarker,
     XtRFloat, sizeof(float),
     off(xmarker), XtRFloat, (XtPointer) &dflt_dbl
  },
  {  XtNplotYMarker, XtCPlotMarker,
     XtRFloat, sizeof(float),
     off(ymarker), XtRFloat, (XtPointer) &dflt_dbl
  },
  {  XtNplotXTick, XtCPlotAxisParams,
     XtRFloat, sizeof(float),
     off(xtic), XtRFloat, (XtPointer) &dflt_huge
  },
  {  XtNplotYTick, XtCPlotAxisParams,
     XtRFloat, sizeof(float),
     off(ytic), XtRFloat, (XtPointer) &dflt_huge
  },
  {  XtNplotY2Tick, XtCPlotAxisParams,
     XtRFloat, sizeof(float),
     off(y2tic), XtRFloat, (XtPointer) &dflt_huge
  },
  {  XtNplotXNum, XtCPlotAxisParams,
     XtRFloat, sizeof(float),
     off(xnum), XtRFloat, (XtPointer) &dflt_huge
  },
  {  XtNplotYNum, XtCPlotAxisParams,
     XtRFloat, sizeof(float),
     off(ynum), XtRFloat, (XtPointer) &dflt_huge
  },
  {  XtNplotY2Num, XtCPlotAxisParams,
     XtRFloat, sizeof(float),
     off(y2num), XtRFloat, (XtPointer) &dflt_huge
  },
  {  XtNplotXPrecision, XtCPlotPrecision,
     XtRInt, sizeof(int),
     off(xprecision), XtRInt, (XtPointer)0
  },
  {  XtNplotYPrecision, XtCPlotPrecision,
     XtRInt, sizeof(int),
     off(yprecision), XtRInt, (XtPointer) 0
  },
  {  XtNplotY2Precision, XtCPlotPrecision,
     XtRInt, sizeof(int),
     off(y2precision), XtRInt, (XtPointer) 0
  },
  {
     XtNplotXTickUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(xtic_dft), XtRImmediate, (XtPointer)True
  },
  {
     XtNplotYTickUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(ytic_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotY2TickUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(y2tic_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotXNumUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(xnum_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotYNumUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(ynum_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotY2NumUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(y2num_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotXAxisMaxUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(xaxis_max_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotYAxisMaxUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(yaxis_max_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotY2AxisMaxUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(y2axis_max_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotXAxisMinUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(xaxis_min_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotYAxisMinUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(yaxis_min_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotY2AxisMinUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(y2axis_min_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotXMaxUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(xmax_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotYMaxUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(ymax_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotY2MaxUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(y2max_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotXMinUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(xmin_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotYMinUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(ymin_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotY2MinUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(y2min_dft), XtRImmediate, (XtPointer) True
  },
  {
     XtNplotXPrecisionUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(xprecision_dft), XtRImmediate, (XtPointer)True
  },
  {
     XtNplotYPrecisionUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(yprecision_dft), XtRImmediate, (XtPointer)True
  },
  {
     XtNplotY2PrecisionUseDefault, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(y2precision_dft), XtRImmediate, (XtPointer)True
  },
  { 
     XtNplotMarkerDataStyleUseDefault, XtCPlotDataStylesUseDefault,
     XtRBoolean, sizeof(Boolean),
     off(marker_ds_dft), XtRImmediate, (XtPointer)True
  },
  { 
     XtNplotXMarkerDataStyleUseDefault, XtCPlotDataStylesUseDefault,
     XtRBoolean, sizeof(Boolean),
     off(xmarker_ds_dft), XtRImmediate, (XtPointer)True
  },
  { 
     XtNplotYMarkerDataStyleUseDefault, XtCPlotDataStylesUseDefault,
     XtRBoolean, sizeof(Boolean),
     off(ymarker_ds_dft), XtRImmediate, (XtPointer)True
  },
  { 
     XtNplotYAxisLogarithmic, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(ylog), XtRImmediate, (XtPointer)False
  },
  { 
     XtNplotY2AxisLogarithmic, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(y2log), XtRImmediate, (XtPointer)False
  },
  { 
     XtNplotAccurateTicLabel, XtCPlotDrawOption,
     XtRBoolean, sizeof(Boolean),
     off(accurate_tic_label), XtRImmediate, (XtPointer)False
  },
  {
     XtNplotMarkerShow, XtCPlotMarkerShow,
     XtRBoolean, sizeof(Boolean),
     off(marker_show), XtRImmediate, (XtPointer)False
  },
  {
     XtNplotXMarkerShow, XtCPlotMarkerShow,
     XtRBoolean, sizeof(Boolean),
     off(xmarker_show), XtRImmediate, (XtPointer)False
  },
  {
     XtNplotYMarkerShow, XtCPlotMarkerShow,
     XtRBoolean, sizeof(Boolean),
     off(ymarker_show), XtRImmediate, (XtPointer)False
  },
  {
     XtNplotSetLabels, XtCPlotSetLabels,
     XtRPlotStrings, sizeof(String *),
     off(set_labels), XtRPointer, (XtPointer) NULL
  },
  {
     XtNplotSetLabels2, XtCPlotSetLabels,
     XtRPlotStrings, sizeof(String *),
     off(set_labels2), XtRPointer, (XtPointer) NULL
  },
  {
     XtNplotXMarkerPoint, XtCPlotMarkerPoint,
     XtRInt, sizeof(int),
     off(xmarker_point), XtRInt, (XtPointer) 0
  },
  {
     XtNplotYMarkerPoint, XtCPlotMarkerPoint,
     XtRInt, sizeof(int),
     off(ymarker_point), XtRInt, (XtPointer) 0
  },
  {
     XtNplotXMarkerSet, XtCPlotMarkerSet,
     XtRInt, sizeof(int),
     off(xmarker_set), XtRInt, (XtPointer) 0
  },
  {
     XtNplotYMarkerSet, XtCPlotMarkerSet,
     XtRInt, sizeof(int),
     off(ymarker_set), XtRInt, (XtPointer) 0
  },
  {
     XtNplotTimeBase, XtCPlotTimeBase,
     XtRLong, sizeof(time_t),
     off(time_base), XtRImmediate, (XtPointer) 0
  },
  {
     XtNplotTimeFormat, XtCPlotTimeFormat,
     XtRString, sizeof(String),
     off(time_format), XtRPointer, (XtPointer) NULL
  },
  {
     XtNplotTimeFormatUseDefault, XtCPlotTimeFormatUseDefault,
     XtRBoolean, sizeof(Boolean),
     off(time_format_dft), XtRImmediate, (XtPointer) TRUE
  },
  {
     XtNplotTimeUnit, XtCPlotTimeUnit,
     XtRInt, sizeof(int),
     off(time_unit), XtRImmediate, (XtPointer) PLOT_TMUNIT_SECONDS 
  },
  {
     XtNplotXAnnotationMethod, XtCPlotAnnotationMethod,
     XtRInt, sizeof(int),
     off(xanno_method), XtRImmediate, (XtPointer) PLOT_ANNO_VALUES
  },
  {
     XtNplotYAnnotationMethod, XtCPlotAnnotationMethod,
     XtRInt, sizeof(int),
     off(yanno_method), XtRImmediate, (XtPointer) PLOT_ANNO_VALUES
  },
  {
     XtNplotY2AnnotationMethod, XtCPlotAnnotationMethod,
     XtRInt, sizeof(int),
     off(y2anno_method), XtRImmediate, (XtPointer) PLOT_ANNO_VALUES
  },
  {
     XtNplotYAxisConst, XtCPlotYAxisConst,
     XtRFloat, sizeof(float),
     off(yaxis_const), XtRImmediate, (XtPointer) 0
  },
  {
     XtNplotYAxisMult, XtCPlotYAxisMult,
     XtRFloat, sizeof(float),
     off(yaxis_mult), XtRImmediate, (XtPointer) 0
  },
  {
     XtNplotGraphMarginBottom, XtCPlotMargin,
     XtRInt, sizeof(int),
     off(graph_margin_bottom), XtRImmediate, (XtPointer) 0
  },
  {
     XtNplotGraphMarginLeft, XtCPlotMargin,
     XtRInt, sizeof(int),
     off(graph_margin_left), XtRImmediate, (XtPointer)0
  },
  {
     XtNplotGraphMarginRight, XtCPlotMargin,
     XtRInt, sizeof(int),
     off(graph_margin_right), XtRImmediate, (XtPointer) 0
  },
  {
     XtNplotGraphMarginTop, XtCPlotMargin,
     XtRInt, sizeof(int),
     off(graph_margin_top), XtRImmediate, (XtPointer) 0
  },
  { 
     XtNplotGraphMarginBottomUseDefault, XtCPlotGraphMarginUseDefault,
     XtRBoolean, sizeof(Boolean),
     off(graph_margin_bottom_dft), XtRImmediate, (XtPointer)True
  },
  { 
     XtNplotGraphMarginLeftUseDefault, XtCPlotGraphMarginUseDefault,
     XtRBoolean, sizeof(Boolean),
     off(graph_margin_left_dft), XtRImmediate, (XtPointer)True
  },
  { 
     XtNplotGraphMarginRightUseDefault, XtCPlotGraphMarginUseDefault,
     XtRBoolean, sizeof(Boolean),
     off(graph_margin_right_dft), XtRImmediate, (XtPointer)True
  },
  { 
     XtNplotGraphMarginTopUseDefault, XtCPlotGraphMarginUseDefault,
     XtRBoolean, sizeof(Boolean),
     off(graph_margin_top_dft), XtRImmediate, (XtPointer)True
  }
};
#undef off

/*
 *   The default bounding box is a nonsensical one, with xmin > xmax.
 *   Unless it is modified to a sensible value, the plotter will ignore it.
 *   This is so that TextPlots (annotations), axes and similar plot types
 *   won't be involved in the axis scale computations.
 */

static BoundingBox default_bounding_box = {1.0, 0.0, 1.0, 0.0};

#define off(field) XtOffset(AtPlotterConstraints, plotter.field)
static XtResource constraint_resources[] = {
  {
     XtNdisplayed, XtCDisplayed,
     XtRBoolean, sizeof(Boolean),
     off(displayed), XtRImmediate, (XtPointer) True
  },
  {
     XtNlegendName, XtCLegendName,
     XtRString, sizeof(String),
     off(legend_name), XtRString, (XtPointer) NULL
  },
  {
     XtNuseX2Axis, XtCUseX2Axis,
     XtRBoolean, sizeof(Boolean),
     off(use_x2_axis), XtRImmediate, (XtPointer) False
  },
  {
     XtNuseY2Axis, XtCUseY2Axis,
     XtRBoolean, sizeof(Boolean),
     off(use_y2_axis), XtRImmediate, (XtPointer) False
  },
  {
     XtNrankOrder, XtCRankOrder,
     XtRInt, sizeof(int),
     off(rank_order), XtRImmediate, (XtPointer) 0
  }
};
#undef off

static void ClassInitialize P(());
static void Initialize P((AtPlotterWidget, AtPlotterWidget));
/*static void Destroy P((AtPlotterWidget));*/
static void Destroy P((Widget));
static void Resize P((AtPlotterWidget));
static void Realize P((AtPlotterWidget, XtValueMask *, XSetWindowAttributes *));
static int Redisplay P((AtPlotterWidget, XEvent *, Region));
static Boolean SetValues P((AtPlotterWidget, AtPlotterWidget, AtPlotterWidget));
static XtGeometryResult QueryGeometry P((AtPlotterWidget, XtWidgetGeometry *,
					 XtWidgetGeometry *));

static void InsertChild P((Widget));
static void DeleteChild P((Widget));
static void ConstraintInitialize P((Widget, Widget));
static void ConstraintDestroy P((Widget));
static Boolean ConstraintSetValues P((Widget, Widget, Widget));

static Boolean Layout P((AtPlotterWidget));
static Boolean RecalcLegend P((AtPlotterWidget));
static void RedrawLegend (AtPlotterWidget, Region, Boolean);

static void ReRankOrderChildren P((Widget));
static void RankOrderChildren P((Widget));
static void RankOrderRemove P((Widget));

static void InstallHandlers P((AtPlotterWidget));
static void RemoveHandlers P((AtPlotterWidget));
static void EnterLeave P((AtPlotterWidget, XtPointer, XEvent *, int));

static void HandleMotion P((AtPlotterWidget, XMotionEvent *));
static void StartSelection P((AtPlotterWidget, XButtonPressedEvent *));
static void Drag P((AtPlotterWidget, XMotionEvent *));
static void EndSelection P((AtPlotterWidget, XButtonReleasedEvent *));
static void StartSliding P((AtPlotterWidget, XButtonPressedEvent *));
static void Slide P((AtPlotterWidget, XMotionEvent *));
static void EndSliding P((AtPlotterWidget, XButtonReleasedEvent *));
static void Cancel P((AtPlotterWidget, XMotionEvent *));
static int Move P((AtPlotterWidget, XMotionEvent *));
static int MoveNotify P((AtPlotterWidget, XButtonPressedEvent *));

static void SetDragPositions P((AtPlotterWidget));
static void GetAxisPositions P((AtPlotterWidget, AtAxisPositions *));
static Boolean SetAxisPositions P((AtPlotterWidget, AtAxisPositions *));

/*
 *   The default translations
 */

static char defaultTranslations[] =
     "Ctrl<Btn1Down>:   graph-zoom-start() \n\
     Ctrl<Btn1Motion>:  zoom-motion() \n\
     Ctrl<Btn1Up>:      graph-zoom-end() \n\
     Shift<Btn1Down>:   axis-zoom-start() \n\
     Shift<Btn1Motion>: zoom-motion() \n\
     Shift<Btn1Up>:     axis-zoom-end() \n\
     <Key>r:    reset()  ";

     /*Ctrl<Btn2Down>:   scale-start() \n\
     Ctrl<Btn2Motion>:  scale-motion() ";
     <Btn1Down>:    start-selection() \n\
     <Btn1Motion>:  drag() \n\
     <Btn1Up>:      end-selection() \n\
     <Btn2Down>:    start-sliding() \n\
     <Btn2Motion>:  slide() \n\
     <Btn2Up>:      end-sliding() \n\
     <Btn3Down>:    cancel() \n\
     <Key>Escape:   cancel() \n\
     <Motion>:      motion-notify() ";
     */
/*
 *   The actions
 */

static XtActionsRec actions[] =
{
     { "axis-zoom-start",     (XtActionProc) AxisZoomStart},
     { "graph-zoom-start",    (XtActionProc) GraphZoomStart},
     { "zoom-motion",         (XtActionProc) ZoomMotion},
     { "graph-zoom-end",      (XtActionProc) GraphZoomEnd},
     { "axis-zoom-end",       (XtActionProc) AxisZoomEnd},
     { "reset",               (XtActionProc) Reset}
/*     { "scale-start",         (XtActionProc) ScaleStart},
     { "scale-motion",        (XtActionProc) ScaleMotion},
     { "move-notify",         (XtActionProc) MoveNotify},
     { "move",                (XtActionProc) Move},
     { "motion-notify",       (XtActionProc) HandleMotion },
     { "start-selection",     (XtActionProc) StartSelection },
     { "drag",                (XtActionProc) Drag },
     { "end-selection",       (XtActionProc) EndSelection},
     { "start-sliding",       (XtActionProc) StartSliding },
     { "slide",               (XtActionProc) Slide },
     { "end-sliding",         (XtActionProc) EndSliding },
     { "cancel",              (XtActionProc) Cancel }*/
};


#define superclass (&constraintClassRec)

externaldef(compositeclassrec) AtPlotterClassRec atPlotterClassRec = {
  { /******* CoreClassPart *******/
    /* superclass           */  (WidgetClass) superclass,
    /* class_name           */  "AtPlotter",
    /* widget_size          */  sizeof(AtPlotterRec),
    /* class_initialize     */  (XtProc) ClassInitialize,
    /* class_part_initialize*/  NULL,
    /* class_inited         */  FALSE,
    /* initialize           */  (XtInitProc) Initialize,
    /* initialize_hook      */  NULL,
    /* realize              */  (XtRealizeProc) Realize,
    /* actions              */  actions,
    /* num_actions          */  XtNumber(actions),
    /* resources            */  plotter_resources,
    /* num_resources        */  XtNumber(plotter_resources),
    /* xrm_class            */  NULLQUARK,
    /* compress_motion      */  TRUE,
    /* compress_exposure    */  XtExposeCompressMultiple |
				XtExposeGraphicsExposeMerged,
    /* compress_enterleave  */  FALSE,
    /* visible_interest     */  FALSE,
    /* destroy              */  (XtWidgetProc) Destroy,
    /* resize               */  (XtWidgetProc) Resize,
    /* expose               */  (XtExposeProc) Redisplay,
    /* set_values           */  (XtSetValuesFunc) SetValues,
    /* set_values_hook      */  NULL,
    /* set_values_almost    */  XtInheritSetValuesAlmost,
    /* get_values_hook      */  NULL,
    /* accept_focus         */  NULL,
    /* version              */  XtVersion,
    /* callback_offsets     */  NULL,
    /* tm_table             */  defaultTranslations,
    /* query_geometry       */  (XtGeometryHandler) QueryGeometry,
    /* display_accelerator  */  NULL,
    /* extension            */  NULL
  },
  { /**** CompositeClassPart ****/
    /* geometry_handler     */  NULL,
    /* change_managed       */  NULL,
    /* insert_child         */  (XtWidgetProc) InsertChild,
    /* delete_child         */  (XtWidgetProc) DeleteChild,
    /* extension            */  NULL
  },
  { /**** ConstraintClassPart ****/
    /* resources            */  constraint_resources,
    /* num_resources        */  XtNumber(constraint_resources),
    /* constraint_size      */  sizeof(AtPlotterConstraintsRec),
    /* initialize           */  (XtInitProc) ConstraintInitialize,
    /* destroy              */  (XtWidgetProc) ConstraintDestroy,
    /* set_values           */  (XtSetValuesFunc) ConstraintSetValues,
    /* extension            */  NULL,
  },
  { /**** AtPlotterClassPart ****/
    /* meaningless field    */  0
  }
};

WidgetClass atPlotterWidgetClass = (WidgetClass) &atPlotterClassRec;


/*
 *   Class initialize
 */

static void ClassInitialize()
{
     static CompositeClassExtensionRec ext;

     ext.next_extension = NULL;
     ext.record_type = NULLQUARK;
     ext.version = XtCompositeExtensionVersion;
     ext.record_size = sizeof(CompositeClassExtensionRec);
     ext.accepts_objects = True;
     atPlotterClassRec.composite_class.extension = (XtPointer) &ext;
     AtRegisterFloatConverter();
     AtRegisterDoubleConverter();
     XmRegisterConverters() ;
     
     *SCCSid = *SCCSid;       /* Keep gcc quiet */
}

/*
 *   Helper functions
 */

static void GetTitle P((AtPlotterWidget));
static void GetTitle(AtPlotterWidget w)
{
     if (w->plotter.title != NULL)
	  w->plotter.title_text =
	    At2TextCreate((Widget)w, w->plotter.title[0], w->plotter.title_font);
     else
	  w->plotter.title_text = NULL;
}

static void FreeTitle P((AtPlotterWidget));
static void FreeTitle(w)
AtPlotterWidget w;
{
  int i;

     if (w->plotter.title_text) AtTextDestroy(w->plotter.title_text);
     w->plotter.title_text = NULL;
     /*if (w->plotter.title) 
       for (i=0; i<w->plotter.title_strnum; i++) 
         if (w->plotter.title[i]) {
	   free(w->plotter.title[i]); 
	   w->plotter.title[i] = NULL;
	   }*/
     w->plotter.title_strnum = 0;
     w->plotter.title = NULL;
}

static void GetLegendTitle P((AtPlotterWidget));
static void GetLegendTitle(w)
AtPlotterWidget w;
{
 /*    if (w->plotter.legend_title != NULL)
	  w->plotter.legend_title_text =
	    At2TextCreate((Widget)w, w->plotter.legend_title, w->plotter.legend_font);
     else
	  w->plotter.legend_title_text = NULL;*/
}

static void FreeLegendTitle P((AtPlotterWidget));
static void FreeLegendTitle(w)
AtPlotterWidget w;
{
    /* if (w->plotter.legend_title_text)
	  AtTextDestroy(w->plotter.legend_title_text);
     w->plotter.legend_title_text = NULL;*/
}



static void GetLegendText P((AtPlotterConstraints, AtPlotterWidget));
static void GetLegendText(c, p)
AtPlotterConstraints c;
AtPlotterWidget p;
{
     if (c->plotter.legend_name != NULL) {
	  c->plotter.legend_name =
	       AtNewString(c->plotter.legend_name);
	  c->plotter.legend_text =
	     At2TextCreate((Widget)p, c->plotter.legend_name, p->plotter.legend_font);
     }
     else
	  c->plotter.legend_text = NULL;
}

static void FreeLegendText P((AtPlotterConstraints, AtPlotterConstraints));
static void FreeLegendText(cur, new)
AtPlotterConstraints cur, new;
{
     if (cur->plotter.legend_name != NULL)
	  XtFree(cur->plotter.legend_name);
     if (new->plotter.legend_text != NULL) {
	  AtTextDestroy(new->plotter.legend_text);
	  new->plotter.legend_text = NULL;
     }
     cur->plotter.legend_name = NULL;
     cur->plotter.legend_text = NULL;
}

/*
 *   Helper routines for pixmaps
 */

/* Align pixmap width and height */
static int AlignPixmap P((int));
static int AlignPixmap(n)
int n;
{
     int m;

     if (n <= PIXMAP_ALIGNMENT)
	  n = PIXMAP_ALIGNMENT;
     else if(m = (n % PIXMAP_ALIGNMENT))
	  n += PIXMAP_ALIGNMENT - m;
     return n;
}

/* Get pixmap gc */
static void GetPixmapGC P((AtPlotterWidget));
static void GetPixmapGC(pw)
AtPlotterWidget pw;
{
     XGCValues gcv;

     if (pw->plotter.pixmap_gc)
	  XtReleaseGC((Widget) pw, pw->plotter.pixmap_gc);
     gcv.foreground = pw->core.background_pixel;
     pw->plotter.pixmap_gc = XtGetGC((Widget) pw, GCForeground, &gcv);
}

/* Free pixmap */
static void FreePixmap P((AtPlotterWidget));
static void FreePixmap(pw)
AtPlotterWidget pw;
{
     AtPlotterPart *pp = &pw->plotter;

     if (pp->pixmap)
	  XFreePixmap(XtDisplay(pw), pp->pixmap);
     if (pp->pixmap_gc)
	  XtReleaseGC((Widget) pw, pp->pixmap_gc);
     pp->pixmap = NULL;
     pp->pixmap_gc = NULL;
     pp->pixmap_width = pp->pixmap_height = 0;
     pp->pixmap_required = False;
}

/* Get pixmap */
static void GetPixmap P((AtPlotterWidget));
static void GetPixmap(pw)
AtPlotterWidget pw;
{
     AtPlotterPart *pp = &pw->plotter;
     unsigned int w, h, d;

     w = AlignPixmap(pw->core.width);
     h = AlignPixmap(pw->core.height);
     if ( !pp->pixmap || (w != pp->pixmap_width || h != pp->pixmap_height)) {
	  d = DefaultDepthOfScreen(XtScreen(pw));
	  if (pp->pixmap) 
	       XFreePixmap(XtDisplay(pw), pp->pixmap);
	  pp->pixmap = XCreatePixmap(XtDisplay(pw), XtWindow(pw), w, h, d);
	  if (pp->pixmap_backup) 
	       XFreePixmap(XtDisplay(pw), pp->pixmap_backup);
	  pp->pixmap_backup = XCreatePixmap(XtDisplay(pw), XtWindow(pw), w, h, d);
	  if ( !pp->pixmap_gc)
	       GetPixmapGC(pw);
	  pp->pixmap_width = w;
	  pp->pixmap_height = h;
     }
     XFillRectangle(XtDisplay(pw), pp->pixmap, pp->pixmap_gc, 0, 0, w, h);
     pp->pixmap_required = False;
}

int freeStrings(char **src, int n)
{
  int i;

  if (!src) return(0);
  for (i=0; i<n; i++) 
    if (src[i]) {
      XtFree(src[i]);
      src[i] = NULL;
      }
  free(src);
  src = NULL;
}


int copyStrings(char **src, char **des)
{
  char **tmp;
  int i, n;

  if (!src) return(0);
  n = 0;
  while(src[n]) n++;
  tmp = (char **)malloc(sizeof(char *) * (n+1));
  if (!tmp)return(0);
  for (i=0; i<n; i++) {
    tmp[i] = (char *)XtNewString(src[i]);
    }
  tmp[n] = NULL;
  des = tmp;
  return(n);
}


int copyStyle(PlotDataStyle **ds, PlotDataStyle **nds)
{
int i, n;
PlotDataStyle **tmp;

  if (!ds) return(0);
  n = 0;
  while (ds[n]) n++;
  tmp = (PlotDataStyle **)malloc(sizeof(PlotDataStyle *) * (n+1));
  if (!tmp) {
    nds = NULL;
    return(0);
    }
  for (i=0; i<n; i++) {
    tmp[i] = (PlotDataStyle *)malloc(sizeof(PlotDataStyle));
    tmp[i]->lpat = ds[i]->lpat;
    tmp[i]->fpat = ds[i]->fpat;
    tmp[i]->color = (char *)XtNewString(ds[i]->color);
    tmp[i]->width = ds[i]->width;
    tmp[i]->point = ds[i]->point;
    tmp[i]->pcolor = (char *)XtNewString(ds[i]->pcolor);
    tmp[i]->psize = ds[i]->psize;
    }
  tmp[n] = NULL;
  nds = tmp;
  return(n);
}


/*
 *   Initialize
 */

static void Initialize(request, new)
AtPlotterWidget request, new;
{
     XGCValues gcv;
     AtPlotterPart *pp = &new->plotter;
     Boolean mirror;
     AtAxisCoreWidget xaxis, yaxis, x2axis, y2axis;
     Pixel tmp_pixel, fore, back, top, bottom;
     Widget line;
     int i, j;
     float maxtmp=30;
     Colormap colormap;
     int screen_num;
     Arg args[30];
     XtGCMask gcv_mask;
     char precision[8];

         /* Make private copies of string resource */
   pp->widget_back = XtNewString(pp->widget_back);
   pp->widget_fore = XtNewString(pp->widget_fore);
   pp->graph_back = XtNewString(pp->graph_back);
   pp->graph_fore = XtNewString(pp->graph_fore);
   pp->data_back = XtNewString(pp->data_back);
   pp->data_fore = XtNewString(pp->data_fore);
   pp->title_back = XtNewString(pp->title_back);
   pp->title_fore = XtNewString(pp->title_fore);
   pp->legend_back = XtNewString(pp->legend_back);
   pp->legend_fore = XtNewString(pp->legend_fore);
   if (pp->xtitle) pp->xtitle = XtNewString(pp->xtitle);
   if (pp->ytitle) pp->ytitle = XtNewString(pp->ytitle);
   if (pp->y2title) pp->y2title = XtNewString(pp->y2title);

   pp->title_strnum = copyStrings(pp->title, pp->title);
   pp->legend_num = copyStrings(pp->set_labels, pp->set_labels);
   pp->ds_num = copyStyle(pp->plot_data_styles, pp->plot_data_styles);
   pp->ds2_num = copyStyle(pp->plot_data_styles2, pp->plot_data_styles2);

  pp->raw_bounding_box.xmin = pp->raw_bounding_box.ymin = PLOTTER_HUGE_VAL;
  pp->raw_bounding_box.x2min = pp->raw_bounding_box.y2min = PLOTTER_HUGE_VAL;
  pp->raw_bounding_box.xmax = pp->raw_bounding_box.ymax = -PLOTTER_HUGE_VAL;
  pp->raw_bounding_box.x2max = pp->raw_bounding_box.y2max = -PLOTTER_HUGE_VAL;
  pp->bounding_box.xmin = pp->bounding_box.ymin = PLOTTER_HUGE_VAL;
  pp->bounding_box.x2min = pp->bounding_box.y2min = PLOTTER_HUGE_VAL;
  pp->bounding_box.xmax = pp->bounding_box.ymax = -PLOTTER_HUGE_VAL;
  pp->bounding_box.x2max = pp->bounding_box.y2max = -PLOTTER_HUGE_VAL;
  pp->layout.x1 = pp->layout.y1 = pp->layout.x2 = pp->layout.y2 = 0;
  pp->layout.width = pp->layout.height = 0;
  pp->layout.title_x = pp->layout.title_y = 0;
  pp->layout.legend_x = pp->layout.legend_y = 0;
  pp->layout.legend_width = pp->layout.legend_height = 0;
  pp->layout.title_width = pp->layout.title_height = 0;
  pp->zoom_box.x1 = pp->zoom_box.x2 = pp->zoom_box.y1 = pp->zoom_box.y2 = 0;
  pp->ifzoom = PLOT_NORMAL;
   pp->data_widget = NULL;
   pp->data2_widget = NULL;
   pp->data_widget_num = 0;
   pp->data2_widget_num = 0;
   pp->displayed = False;
   pp->text_list = NULL;
   pp->default_color_num = 0;
   pp->ordered_children = NULL;
   pp->use_pixmap = True; 
   pp->pixmap = pp->pixmap_backup = None;
   pp->pixmap_gc = NULL;
   pp->pixmap_required = pp->use_pixmap;

   back = PlotColor((Widget)new, pp->widget_back, "white");
   fore = PlotColor((Widget)new, pp->widget_fore, "black");
   PlotterInitDefaultColor((Widget)new);
   pp->default_color[6]=fore;
   pp->default_color[7]=back;
   if (pp->data_back) 
     pp->plotarea_color = WnColorB((Widget)new, pp->data_back);
   else if (pp->graph_back) 
     pp->plotarea_color = WnColorB((Widget)new, pp->graph_back);
     else pp->plotarea_color = back;
   gcv.foreground = pp->plotarea_color;
   gcv.background = fore;
   pp->plotarea_gc = XtGetGC((Widget) new, GCForeground | GCBackground, &gcv);

   /*gcv.foreground = back;
   pp->graph_gc = XtGetGC((Widget)new, GCForeground |GCBackground, &gcv);
   */
   if (!pp->title_font) 
     pp->title_font = XLoadFont(XtDisplay(new), "fixed");
   GetTitle(new);
   if (pp->title_fore) 
     pp->title_color = gcv.foreground = WnColorF((Widget)new, pp->title_fore);
   else pp->title_color = gcv.foreground = fore;
   if (pp->title_back) 
     gcv.background = WnColorB((Widget)new, pp->title_back);
   else gcv.background = back;
   gcv.font = pp->title_font;
   pp->title_gc = XtGetGC((Widget) new, GCForeground | GCBackground | GCFont, &gcv);
   
   if (!pp->legend_font) 
     pp->legend_font = XLoadFont(XtDisplay(new), "fixed");
   /*GetLegendTitle(new);*/
   gcv_mask = GCForeground | GCBackground;
   if (pp->legend_fore) 
     gcv.foreground = WnColorF((Widget)new, pp->legend_fore);
   else gcv.foreground = fore;
   if (pp->legend_back)
     gcv.background = WnColorB((Widget)new, pp->legend_back);
   else  gcv.background = back;
   if (pp->legend_font) {
     gcv.font = pp->legend_font;
     gcv_mask |= GCFont;
     }
   pp->legend_gc = XtGetGC((Widget) new, gcv_mask, &gcv);
   tmp_pixel = gcv.background;
   gcv.background = gcv.foreground;
   gcv.foreground = tmp_pixel;
   /*gcv.foreground = new->core.background_pixel;
   gcv.background = pp->legend_color; */
   pp->select_gc = XtGetGC((Widget) new, GCForeground | GCBackground, &gcv);

/* graph_gc has the graph_fore as background, graph_back as foreground */
/* axis_gc has the graph_fore as foreground, graph_back as background */
/* data_fore and data_back are supposed as the default color of plots, if the 
 * plot style is not set */

   if (pp->graph_fore) 
     gcv.background = WnColorF((Widget)new, pp->graph_fore);
   else gcv.background = fore;
   if (pp->graph_back)
     gcv.foreground = WnColorB((Widget)new, pp->graph_back);
   else gcv.foreground = back;
   pp->graph_gc = XtGetGC((Widget)new, GCForeground |GCBackground, &gcv);
   tmp_pixel = gcv.foreground;
   gcv.foreground = gcv.background;
   gcv.background = tmp_pixel;
   /* suppose the "fixed" font is alwayse available */
   if (pp->axis_font) 
     gcv.font = pp->axis_font;
   else  
     gcv.font = pp->axis_font = XLoadFont(XtDisplay((Widget)new), "fixed");
   pp->axis_fs = XQueryFont(XtDisplay((Widget)new), pp->axis_font);
   gcv.line_width = 1;
   pp->axis_gc = XtGetGC((Widget)new, GCForeground | GCBackground | GCFont |
	   GCLineWidth, &gcv); 

   gcv.foreground = new->core.background_pixel ^ pp->title_color;
   gcv.function = GXxor;
   pp->drag_gc = XtGetGC((Widget) new, GCForeground | GCFunction, &gcv);

   /* get gc for top and bottom shadow */
   screen_num = DefaultScreen(XtDisplay((Widget)new));
   colormap = XDefaultColormap(XtDisplay((Widget)new),screen_num);
   XmGetColors(XtScreen(new), colormap, back, NULL,
	 &top, &bottom, NULL);
   gcv.foreground = top;
   gcv.background = new->core.background_pixel;
   pp->top_shadow_gc = XtGetGC((Widget)new, GCForeground | GCBackground, &gcv);
   gcv.foreground = bottom;
   pp->bottom_shadow_gc=XtGetGC((Widget)new, GCForeground | GCBackground, &gcv);
   
   /* get marker gc if needed */
   pp->marker_gc = NULL;
   if (pp->marker_style) { 
     if (pp->marker_style->width<=0) pp->marker_style->width = 1;
     if (pp->marker_style->width>10) pp->marker_style->width = 10;
     if (pp->marker_style->color) {
       gcv.foreground = WnColorF((Widget)new, pp->marker_style->color);
       gcv.line_width = pp->marker_style->width;
       pp->marker_gc = XtGetGC((Widget)new, GCForeground | GCLineWidth, &gcv);
       }
     }

     /* Create the drag and slide cursors */
   pp->top_left_cursor  = XCreateFontCursor(XtDisplay(new), XC_ul_angle);
   pp->top_right_cursor = XCreateFontCursor(XtDisplay(new), XC_ur_angle);
   pp->bottom_left_cursor = XCreateFontCursor(XtDisplay(new), XC_ll_angle);
   pp->bottom_right_cursor = XCreateFontCursor(XtDisplay(new), XC_lr_angle);

     /* Set the current cursor */
   if (pp->use_cursors && pp->plotter_cursor) {
     pp->current_cursor = pp->plotter_cursor;
   }
   else {
     pp->current_cursor = None;
   }

     /* Set these state flags */
   pp->rescale_required = True;
   pp->layout_required = True; 
   pp->redraw_required = True;
   pp->expose_requested = True;  /* Redraw will come from map */

     /* Reset the other state flags and data */
   pp->redisplay_required = False;
   pp->in_layout_mode = False;
   pp->position_required = False;
   pp->positions.position = AtPositionNONE;
   pp->in_select = pp->in_click = FALSE;
   pp->in_drag = pp->in_slide = pp->can_slide = False;
   pp->selected_item = NO_ITEM;
  
  i = 0;
  XtSetArg(args[i], XtNdrawGrid, False); i++;
  XtSetArg(args[i], XtNplotAxisFont, pp->axis_font); i++;
  if (pp->graph_fore) {
    fore = WnColorF((Widget)new, pp->graph_fore);
    XtSetArg(args[i], XtNlabelColor, fore); i++;
    XtSetArg(args[i], XtNnumberColor, fore); i++;
    }
  if (pp->draw_frame) {
    XtSetArg(args[i], XtNdrawFrame, True); i++;
    }
  j = i;
  if (pp->xlabel) {
    XtSetArg(args[j], XtNplotTicLabelStrings, pp->xlabel); j++;
    XtSetArg(args[j], XtNplotTicLabelUseDefault, False); j++;
    }
  if (pp->xanno_method == PLOT_ANNO_TIME_LABELS) {
    pp->xnum_dft = False;
    if (pp->xnum==PLOTTER_HUGE_VAL) pp->xnum = 1;
    }
  if (pp->xnum!=PLOTTER_HUGE_VAL) pp->xnum_dft = False;
  if (pp->xtic!=PLOTTER_HUGE_VAL) pp->xtic_dft = False;
  if (pp->xmax!=PLOTTER_HUGE_VAL) pp->xmax_dft = False;
  if (pp->xmin!=PLOTTER_HUGE_VAL) pp->xmin_dft = False;
  if (pp->xaxis_max!=PLOTTER_HUGE_VAL) pp->xaxis_max_dft = False;
  if (pp->xaxis_min!=PLOTTER_HUGE_VAL) pp->xaxis_min_dft = False;
  if (pp->xprecision > 0) pp->xprecision_dft = False;
  XtSetArg(args[j], XtNplotMaxUseDefault, (pp->xmax_dft && pp->xaxis_max_dft)); j++;
  XtSetArg(args[j], XtNplotMinUseDefault, (pp->xmin_dft && pp->xaxis_min_dft)); j++;
  XtSetArg(args[j], XtNplotNumUseDefault, pp->xnum_dft); j++;
  XtSetArg(args[j], XtNplotTickUseDefault, pp->xtic_dft); j++;
  if (!pp->xaxis_max_dft) {
    XtSetArg(args[j], XtNmax, PlotFloatToArgVal(pp->xaxis_max)); j++;
    }
  else if (!pp->xmax_dft) {
      XtSetArg(args[j], XtNmax, PlotFloatToArgVal(pp->xmax)); j++;
      }
  if (!pp->xaxis_min_dft) {
    XtSetArg(args[j], XtNmin, PlotFloatToArgVal(pp->xaxis_min)); j++;
    }
  else if (!pp->xmin_dft) {
      XtSetArg(args[j], XtNmin, PlotFloatToArgVal(pp->xmin)); j++;
      }
  if ((!pp->xmax_dft) || (!pp->xmin_dft) || (!pp->xaxis_min) || (!pp->xaxis_max)) {
    XtSetArg(args[j], XtNautoScale, False); j++;
    }
  if (! pp->xnum_dft) {
    XtSetArg(args[j], XtNticInterval, PlotFloatToArgVal(pp->xnum)); j++;
    }
  if (!pp->xtic_dft) {
    XtSetArg(args[j], XtNsubticInterval, PlotFloatToArgVal(pp->xtic)); j++;
    }
  if (!pp->xprecision_dft) {
    precision[0]='%'; precision[1] = '.';
    sprintf(&precision[2], "%1df", pp->xprecision);
    XtSetArg(args[j], XtNlinTicFormat, precision); j++;
    XtSetArg(args[j], XtNlogTicFormat, precision); j++;
    }
  if (pp->xtitle) {
    XtSetArg(args[j], XtNlabel, pp->xtitle); j++;
    }
  XtSetArg(args[j], XtNplotAnnotationMethod, pp->xanno_method); j++;
  XtSetArg(args[j], XtNplotTimeFormat, pp->time_format); j++;
    
  xaxis=(AtAxisCoreWidget)XtCreateWidget("x_axis", atXYAxisWidgetClass, 
        (Widget)new, args, j);

  /*XtSetArg(args[j], XtNmirror, True); j++;
  x2axis=(AtAxisCoreWidget)XtCreateWidget("x2_axis", atXYAxisWidgetClass,
	 (Widget)new, args, j);*/ 
  
  
  XtSetArg(args[i], XtNvertical, True); i++;
  j = i;
  if (pp->ymax!=PLOTTER_HUGE_VAL) pp->ymax_dft = False;
  if (pp->ymin!=PLOTTER_HUGE_VAL) pp->ymin_dft = False;
  if (pp->yaxis_max!=PLOTTER_HUGE_VAL) pp->yaxis_max_dft = False;
  if (pp->yaxis_min!=PLOTTER_HUGE_VAL) pp->yaxis_min_dft = False;
  XtSetArg(args[j], XtNplotMaxUseDefault, (pp->ymax_dft && pp->yaxis_max_dft)); j++;
  XtSetArg(args[j], XtNplotMinUseDefault, (pp->ymin_dft && pp->yaxis_min_dft)); j++;
  if (pp->ynum!=PLOTTER_HUGE_VAL)  pp->ynum_dft = False;
  if (pp->ytic!=PLOTTER_HUGE_VAL) pp->ytic_dft = False;
  if (pp->yprecision > 0) pp->yprecision_dft = False;
  XtSetArg(args[j], XtNplotNumUseDefault, pp->ynum_dft); j++;
  XtSetArg(args[j], XtNplotTickUseDefault, pp->ytic_dft); j++;
  if (!pp->yaxis_max_dft) {
    XtSetArg(args[j], XtNmax, PlotFloatToArgVal(pp->yaxis_max)); j++;
    }
  else if (!pp->ymax_dft) {
    XtSetArg(args[j], XtNmax, PlotFloatToArgVal(pp->ymax)); j++;
    }
  if (!pp->yaxis_min_dft) {
    XtSetArg(args[j], XtNmin, PlotFloatToArgVal(pp->yaxis_min)); j++;
    }
  else if (!pp->ymin_dft) {
    XtSetArg(args[j], XtNmin, PlotFloatToArgVal(pp->ymin)); j++;
    }
  if ((!pp->ymax_dft) || (!pp->ymin_dft)||(!pp->yaxis_min)||(!pp->yaxis_max)) {
    XtSetArg(args[j], XtNautoScale, False); j++;
    }
  if (! pp->ynum_dft) {
    XtSetArg(args[j], XtNticInterval, PlotFloatToArgVal(pp->ynum)); j++;
    }
  if (! pp->ytic_dft) {
    XtSetArg(args[j], XtNsubticInterval, PlotFloatToArgVal(pp->ytic)); j++;
    }
  if (pp->ylog) {
    XtSetArg(args[j], XtNaxisTransform, AtTransformLOGARITHMIC); j++;
    }
  if (!pp->yprecision_dft) {
    precision[0]='%'; precision[1] = '.';
    sprintf(&precision[2], "%1df", pp->yprecision);
    XtSetArg(args[j], XtNlinTicFormat, precision); j++;
    XtSetArg(args[j], XtNlogTicFormat, precision); j++;
    }
  if (pp->ytitle) {
    XtSetArg(args[j], XtNlabel, pp->ytitle); j++;
    }

  yaxis=(AtAxisCoreWidget)XtCreateWidget("y_axis", atXYAxisWidgetClass, 
        (Widget)new, args, j);
 
  j = i;
  if (pp->y2max!=PLOTTER_HUGE_VAL) pp->y2max_dft = False;
  if (pp->y2min!=PLOTTER_HUGE_VAL) pp->y2min_dft = False;
  if (pp->y2axis_max!=PLOTTER_HUGE_VAL) pp->y2axis_max_dft = False;
  if (pp->y2axis_min!=PLOTTER_HUGE_VAL) pp->y2axis_min_dft = False;
  XtSetArg(args[j], XtNplotMaxUseDefault, (pp->y2max_dft && pp->y2axis_max_dft)); j++;
  XtSetArg(args[j], XtNplotMinUseDefault, (pp->y2min_dft && pp->y2axis_min_dft)); j++;
  if (pp->y2num!=PLOTTER_HUGE_VAL)  pp->y2num_dft = False;
  if (pp->y2tic!=PLOTTER_HUGE_VAL) pp->y2tic_dft = False;
  if (pp->y2precision > 0) pp->y2precision_dft = False;
  XtSetArg(args[j], XtNplotNumUseDefault, pp->y2num_dft); j++;
  XtSetArg(args[j], XtNplotTickUseDefault, pp->y2tic_dft); j++;
  if (!pp->y2axis_max_dft) {
    XtSetArg(args[j], XtNmax, PlotFloatToArgVal(pp->y2axis_max)); j++;
    }
  else if (!pp->y2max_dft) {
    XtSetArg(args[j], XtNmax, PlotFloatToArgVal(pp->y2max)); j++;
    }
  if (!pp->y2axis_min_dft) {
    XtSetArg(args[j], XtNmin, PlotFloatToArgVal(pp->y2axis_min)); j++;
    }
  else if (!pp->y2min_dft) {
    XtSetArg(args[j], XtNmin, PlotFloatToArgVal(pp->y2min)); j++;
    }
  if ((!pp->y2max_dft)||(!pp->y2min_dft)||(!pp->y2axis_min)||(!pp->y2axis_max)) {
    XtSetArg(args[j], XtNautoScale, False); j++;
    }
  if (! pp->y2num_dft) {
    XtSetArg(args[j], XtNticInterval, PlotFloatToArgVal(pp->y2num)); j++;
    }
  if (! pp->y2tic_dft) {
    XtSetArg(args[j], XtNsubticInterval, PlotFloatToArgVal(pp->y2tic)); j++;
    }
  if (!pp->y2precision_dft) {
    precision[0]='%'; precision[1] = '.';
    sprintf(&precision[2], "%1df", pp->y2precision);
    XtSetArg(args[j], XtNlinTicFormat, precision); j++;
    XtSetArg(args[j], XtNlogTicFormat, precision); j++;
    }
  if (pp->y2log) {
    XtSetArg(args[j], XtNaxisTransform, AtTransformLOGARITHMIC); j++;
    }
  if (pp->y2title) {
    XtSetArg(args[j], XtNlabel, pp->y2title); j++;
    }

  XtSetArg(args[j], XtNmirror, 1); j++;
  y2axis = (AtAxisCoreWidget)XtCreateWidget("y2_axis", atXYAxisWidgetClass,
	 (Widget)new, args, j);

  new->plotter.xaxis = xaxis;
  new->plotter.yaxis = yaxis;
  new->plotter.x2axis = NULL;
  new->plotter.y2axis = y2axis;
  
  if (pp->plot_data!=NULL) {
    PlotterCreatePlot((Widget)new, pp->plot_data, 1); 
  } 

  if (pp->plot_data2) PlotterCreatePlot((Widget)new, pp->plot_data2, 2);
}

/*
 *   A set of helper routines for the redraw protocol.
 *
 *   The first few of these are local to this file, the last few are the
 *   routines that plotwidget children use to request redraws.
 */

/* Request a synthetic expose event if none has happened yet */
static void RequestSyntheticExpose P((AtPlotterWidget));
static void RequestSyntheticExpose(pw)
AtPlotterWidget pw;
{
     XExposeEvent ev;

     if ( !pw->plotter.expose_requested && XtWindow(pw)) {
	  ev.type = Expose;
	  ev.display = XtDisplay(pw);
	  ev.window = XtWindow(pw);
	  ev.x = ev.y = ev.width = ev.height = ev.count = 0;
	  XSendEvent(XtDisplay(pw), XtWindow(pw), False, 0, (XEvent *) &ev);
	  pw->plotter.expose_requested = True;
     }
}


/*
 *   ExtendedList maintainence.  These are lists of from, to pairs for
 *   each plot for areas requiring rescaling (due to plot extend calls).
 *   An ExtendedList of from>to implies the whole plot, and these
 *   routines take care of deleteing partials if there is a full rescale
 *   present.
 */

#define cwel (CONSTRAINTS(cw)->plotter.extended_list)

static void DestroyExtendedList P((AtPlotWidget));
static void DestroyExtendedList(cw)
AtPlotWidget cw;
{
     ExtendedList *c, *n;
     for (c = cwel; c; c = n) {
	  n = c->next;
	  XtFree((char *)c);
     }
     cwel = NULL;
}

static void AddExtendedList P((AtPlotWidget, int, int));
static void AddExtendedList(cw, from, to)
AtPlotWidget cw;
int from, to;
{
     ExtendedList *el;
     
     if (from > to) {
	  /* Is a complete rescale, so forget any partials */
	  DestroyExtendedList(cw);
     } else if (cwel && cwel->from > cwel->to) {
	  /* A complete rescale already exists, drop this one */
	  return;
     }
     el = (ExtendedList *)XtMalloc(sizeof (ExtendedList));
     el->from = from;
     el->to = to;
     /* Either this is a complete rescale request or an additional
      *      partial one */
     el->next = cwel;
     cwel = el;
}

#undef cwel

/*
 *   Calculate the new raw bounding box for the whole plot.
 *   Return TRUE if  it changed, else FALSE.
 *   This is modified by the axes to make the actual bounding box.
 */

static Boolean NewRawBoundingBox P((AtPlotterWidget));
static Boolean NewRawBoundingBox(pw)
AtPlotterWidget pw;
{
  AtPlotterBoundingBox nbb;
  BoundingBox *cbbp;
  int i;
  Boolean ret;

  nbb.xmin = nbb.ymin = nbb.x2min = nbb.y2min = PLOTTER_HUGE_VAL;
  nbb.xmax = nbb.ymax = nbb.x2max = nbb.y2max = -PLOTTER_HUGE_VAL;

  for (i = 0; i < NUMCHILDREN(pw); i++) {
    if ((XtIsSubclass((Widget)CHILD(pw, i), atPlotWidgetClass)) || 
	  (NTHCHILDISDISPLAYED(pw, i))) {
       cbbp = &(CONSTRAINT(pw, i)->plotter.bounding_box);
       if (cbbp->xmin > cbbp->xmax) continue;
       /* Is a child w/o boundingbox */
       /* if a plot linked to y2-axis, not merge it's x range into the x-axis*/
       /*if (CONSTRAINT(pw, i)->plotter.use_x2_axis) {
	    nbb.x2max = Max(nbb.x2max, cbbp->xmax);
	    nbb.x2min = Min(nbb.x2min, cbbp->xmin);
       } else {
	    nbb.xmax = Max(nbb.xmax, cbbp->xmax);
	    nbb.xmin = Min(nbb.xmin, cbbp->xmin);
       }*/
       if (!CONSTRAINT(pw, i)->plotter.use_x2_axis) {
	    nbb.xmax = Max(nbb.xmax, cbbp->xmax);
	    nbb.xmin = Min(nbb.xmin, cbbp->xmin);
       } else if (!pw->plotter.plot_data) {
	    nbb.xmax = Max(nbb.xmax, cbbp->xmax);
	    nbb.xmin = Min(nbb.xmin, cbbp->xmin);
       }
       if (CONSTRAINT(pw, i)->plotter.use_y2_axis) {
	    nbb.y2min = Min(nbb.y2min, cbbp->ymin);
	    nbb.y2max = Max(nbb.y2max, cbbp->ymax);
       } else {
	    nbb.ymin = Min(nbb.ymin, cbbp->ymin);
	    nbb.ymax = Max(nbb.ymax, cbbp->ymax);
       }
     }
   }
#define dif(fld) (nbb.fld != pw->plotter.raw_bounding_box.fld)
     ret = dif(xmin) || dif(xmax) || dif(ymin) || dif(ymax) ||
	  dif(x2min) || dif(x2max) || dif(y2min) || dif(y2max);
#undef dif
     pw->plotter.raw_bounding_box = nbb;
     return ret;
} /*NewRawBoundingBox*/

/*
 *   Merge the second boundingbox into the first, return True if the bb changed.
 *   (These are per-plot BB,s in the constraint record).
 *   This is used by the data extended code.
 *
 */

static Boolean MergeBoundingBox P((BoundingBox *, BoundingBox *));
static Boolean MergeBoundingBox(ob, nb)
BoundingBox *ob, *nb;
{
     BoundingBox old;
     Boolean ret;

     old = *ob;
     ob->xmax = Max(ob->xmax, nb->xmax);
     ob->xmin = Min(ob->xmin, nb->xmin);
     ob->ymax = Max(ob->ymax, nb->ymax);
     ob->ymin = Min(ob->ymin, nb->ymin);
#define dif(fld) (old.fld != ob->fld)
     ret = dif(xmin) || dif(xmax) || dif(ymin) || dif(ymax);
#undef dif

     return ret;
}

/*
 *   These routines are called by children as well as the parent to
 *   request redrawing as appropriate.
 */

void AtPlotterPlotExtended(cw, bb, from, to)
AtPlotWidget cw;
BoundingBox *bb;
int from, to;
{
     AtPlotterWidget pw = ParentPlotter(cw);
     Boolean bb_changed;

     XtCheckSubclass((Widget) cw, atPlotWidgetClass,
		     "AtPlotterPlotExtended requires an AtPlot widget");

     bb_changed = MergeBoundingBox(&CONSTRAINTS(cw)->plotter.bounding_box, bb);

     if (ISDISPLAYED(cw)) {
	  if (bb_changed && NewRawBoundingBox(pw)) {
	       /* Overall bb has changed, so request an overall rescale */
	       pw->plotter.rescale_required = True;
	  }
	  /* This plot has been extended, but still fits on the graph */
	  AddExtendedList(cw, from, to);
	  RequestSyntheticExpose(pw);
     }
}

/*
 *   Refresh is set true if fast_update is on and we can erase the old
 *   one without having to redraw everything.
 */

void AtPlotterPlotDataChanged(cw, bb, refresh)
AtPlotWidget cw;
BoundingBox *bb;
int refresh;
{
     AtPlotterWidget pw = ParentPlotter(cw);

     XtCheckSubclass((Widget) cw, atPlotWidgetClass,
		     "AtPlotterDataChanged requires an AtPlot widget");

     CONSTRAINTS(cw)->plotter.bounding_box = *bb;

     if (ISDISPLAYED(cw)) {
	  if (NewRawBoundingBox(pw)) {
	       /* Overall bb has changed, so request an overall rescale */
	       pw->plotter.rescale_required = True;
	  }
	  /*
	   * This plot has been changed, but overall bb is the same.
	   * Request a redraw and a rescale on the whole of this plot
	   */
	  AddExtendedList(cw, 0, -1);
	  if (refresh) {
	       AtPlotterRefreshRequired(cw);
	  } else {
	       pw->plotter.redraw_required = True;
	       RequestSyntheticExpose(pw);
	  }
     }
}

void AtPlotterRefreshRequired(cw)
AtPlotWidget cw;
{
     if (!XtIsSubclass((Widget) cw, atPlotWidgetClass))
	  XtAppError(XtWidgetToApplicationContext((Widget) cw),
		     "AtPlotterRefreshRequired requires an AtPlot widget");

     if (ISDISPLAYED(cw))  {
	  CONSTRAINTS(cw)->plotter.needs_refresh = True;
	  RequestSyntheticExpose(ParentPlotter(cw));
     }
}

void AtPlotterRedrawRequired(cw)
AtPlotWidget cw;
{
     if (!XtIsSubclass((Widget) cw, atPlotWidgetClass))
	  XtAppError(XtWidgetToApplicationContext((Widget) cw),
		     "AtPlotterRedrawRequired requires an AtPlot widget");

     if (ISDISPLAYED(cw)) {
	  ParentPlotter(cw)->plotter.redraw_required = True;
	  RequestSyntheticExpose(ParentPlotter(cw));
     }
}

void AtPlotterLayoutRequired(cw)
AtPlotWidget cw;
{
     if (!XtIsSubclass((Widget) cw, atPlotWidgetClass))
	  XtAppError(XtWidgetToApplicationContext((Widget) cw),
		     "AtPlotterLayoutRequired requires an AtPlot widget");

     if (ISDISPLAYED(cw)) {
	  ParentPlotter(cw)->plotter.layout_required = True;
	  RequestSyntheticExpose(ParentPlotter(cw));
     }
}

void AtPlotterRescaleRequired(cw)
AtPlotWidget cw;
{
     if (!XtIsSubclass((Widget) cw, atPlotWidgetClass))
	  XtAppError(XtWidgetToApplicationContext((Widget) cw),
		     "AtPlotterRescaleRequired requires an AtPlot widget");

     if (ISDISPLAYED(cw)) {
	  ParentPlotter(cw)->plotter.rescale_required = True;
	  RequestSyntheticExpose(ParentPlotter(cw));
     }
}

/* Request a complete recalc of just this plot - used by axis code */
void AtPlotterRecalcThisPlot(cw)
AtPlotWidget cw;
{
     AtPlotterWidget pw = ParentPlotter(cw);

     XtCheckSubclass((Widget) cw, atPlotWidgetClass,
		     "AtPlotterDaChanged requires an AtPlot widget");

     if (ISDISPLAYED(cw)) {
	  if (pw->plotter.in_layout_mode) {
	       /* We have to use this one to avoid infinite loops */
	       pw->plotter.rescale_required = True;
	  } else {
	       AddExtendedList(cw, 0, -1);
	       RequestSyntheticExpose(pw);
	  }
     }
}

/*
 *   Get legend width and title height
 *
 *   These routines may be called from other plotters or applications
 *   to get the maximal legend width and title height from the plotter.
 */

int AtPlotterGetLegendWidth(pw)
AtPlotterWidget pw;
{
     AtPlotterConstraints c;
     AtPlotterPart *pp = &pw->plotter;
     int w = 0, i;

     if ( !XtIsRealized((Widget) pw))
	  w = -1;
     else {
	  for(i = 0; i < NUMCHILDREN(pw); i++) {
	       c = CONSTRAINT(pw, i);
	       if (XtIsSubclass((Widget)pw, atPlotWidgetClass)) 
	         if (c->plotter.legend_text) {
		    w = Max(w, AtTextWidth(c->plotter.legend_text));
	         }
	  }
	  w += ICON_WIDTH + 2 * pp->margin_width;
	  /*w = Max(w, AtTextWidth(pp->legend_title_text));*/
     }
     return w;
}

int AtPlotterGetTitleHeight(pw)
AtPlotterWidget pw;
{
     AtPlotterPart *pp = &pw->plotter;
     int h = 0;

     if ( !XtIsRealized((Widget) pw))
	  h = -1;
     else if (pp->title_text)
	  h = AtTextHeight(pp->title_text);
     return h;
}

/*
 *   Get and set plotter's axis positions
 *
 *   These routines may be called from other plotters or applications
 *   to get the current axis positions and to set required axis positions,
 *   i.e. to align axis positions of multiple plotters.
 */

Boolean AtPlotterGetAxisPositions(pw, ap)
AtPlotterWidget pw;
AtAxisPositions *ap;
{
     if ( !XtIsRealized((Widget) pw) || ap == NULL)
	  return False;

     /*
      * Recalc the plotter layout if needed and get
      * the current axis positions.
      */
     if (pw->plotter.layout_required) {
	  (void) Layout(pw);
	  pw->plotter.layout_required = False;
     }
     GetAxisPositions(pw, ap);
     return True;
}

void AtPlotterSetAxisPositions(pw, ap)
AtPlotterWidget pw;
AtAxisPositions *ap;
{
     if ( !XtIsRealized((Widget) pw) || ap == NULL)
	  return;

     /*
      * If there is any axis position alignment required
      * recalc the layout if needed. Then set the required
      * axis positions and force redisplay if there were
      * any changes detected.
      */
     if (ap->position && !pw->plotter.position_required) {
	  if (pw->plotter.layout_required) {
	       (void) Layout(pw);
	       pw->plotter.layout_required = False;
	  }
	  if (SetAxisPositions(pw, ap)) {
	       pw->plotter.position_required = True;
	       RequestSyntheticExpose(pw);
	  }
     }
}

/*
 *   Get and set plotter's selected plot widget
 */

Widget AtPlotterGetSelectedPlot(pw)
AtPlotterWidget pw;
{
     int i;

     if (XtIsRealized((Widget) pw) && pw->plotter.selected_item != NO_ITEM) {
	  for (i = 0; i < NUMCHILDREN(pw); i++) {
	       if (i == pw->plotter.selected_item)
		    return (Widget) CHILD(pw, i);
	  }
     }
     return NULL;
}

Boolean AtPlotterSetSelectedPlot(pw, w)
AtPlotterWidget pw;
Widget w;
{
     int i;

     if (XtIsRealized((Widget) pw)) {
	  for (i = 0; i < NUMCHILDREN(pw); i++) {
	       if (w = (Widget) CHILD(pw, i)) {
		    pw->plotter.selected_item = i;
		    if (pw->plotter.show_legend)
			 RequestSyntheticExpose(pw);
		    return True;
	       }
	  }
     }
     return False;
}

/*
 *   These private ones are called from this file
 *   and are passed the parent widget
 */

static void RescaleRequired P((AtPlotterWidget));
static void RescaleRequired(pw)
AtPlotterWidget pw;
{
     pw->plotter.rescale_required = True;
     RequestSyntheticExpose(pw);
}

static void LayoutRequired P((AtPlotterWidget));
static void LayoutRequired(pw)
AtPlotterWidget pw;
{
     pw->plotter.layout_required = True;
     RequestSyntheticExpose(pw);
}

static void RedrawRequired P((AtPlotterWidget));
static void RedrawRequired(pw)
AtPlotterWidget pw;
{
     if (pw->plotter.redisplay_required)
	  pw->plotter.expose_requested = False;
     pw->plotter.redraw_required = True;
     RequestSyntheticExpose(pw);
}

/*
 *   The guts of the rescale/redraw/ relayout code
 *
 *   1) Layout.  Recalculates all the layouts, returns TRUE if the size
 *   or pixel position of any of the four axes changed.
 *
 *   XXX - assume the axes know enough to answer AxisWidth sensibly!
 *   XXX - also assumes axis.max & axis.min are determined!
 */
static Boolean scaleLayout(AtPlotterWidget pw)
{
     AtPlotterLayout *lp = &pw->plotter.layout;
     AtPlotterPart *pp = &pw->plotter;
     AtPlotterLayout old;
     int new_x1, new_y1, new_x2, new_y2;
     Boolean changed;
     double a, b, c, d;
     int i, j, huge = 1;
     float x1, y1, x2, y2;

     i = 8 * sizeof(short);
     for (j = 1; j<i; j++) huge = huge * 2;
     
     old = pp->layout;
     new_x1 = pp->graph_margin_left;
     new_y1 = pp->graph_margin_top;
     new_x2 = pw->core.width - pp->graph_margin_right;
     new_y2 = pw->core.height - pp->graph_margin_bottom;
  
     /* use the following equations to get parameters a, b, c, d,
     * then to get new location of the elements in old layout 
     * new_x1 = c * 0 + d;
     * new_x2 = c * pw->core.width + d;
     * new_y1 = a * 0 + b;
     * new_y2 = a * pw->core.height +b;
     */

     d = (double)new_x1;
     c = (double)(new_x2 - d)/pw->core.width;
     b = (double)new_y1;
     a = (double)(new_y2 - b)/pw->core.height;

     lp->x1 = pp->layout.x1 * c + d;
     lp->x2 = pp->layout.x2 * c + d;
     lp->y1 = pp->layout.y1 * a + b;
     lp->y2 = pp->layout.y2 * a + b;
     /*x1 = pp->layout.x1 * c + d;
     x2 = pp->layout.x2 * c + d;
     y1 = pp->layout.y1 * a + b;
     y2 = pp->layout.y2 * a + b;
     if ((x1<-huge) || (x2>huge) || (y1<-huge) || (y2>huge) {
       pp->ifzoom = PLOT_OVERFLOW;
       return(False);
       }
     lp->x1 = x1; lp->x2 = x2; 
     lp->y1 = y1; lp->y2 = y2; */
     /*lp->width = pp->layout.width * c;
     lp->height = pp->layout.height * a;*/
     lp->width = lp->x2 - lp->x1 + 1;
     lp->height = lp->y2 - lp->y1 + 1;
     lp->title_x = pp->layout.title_x * c + d;
     lp->title_y = pp->layout.title_y * a + b;
     lp->title_width = pp->layout.title_width * c;
     lp->title_height = pp->layout.title_height * a;
     if ((pp->show_legend)&&(pp->set_labels)) {
       lp->legend_x = pp->layout.legend_x * c + d;
       lp->legend_y = pp->layout.legend_y * a + b;
       lp->legend_width = pp->layout.legend_width * c;
       lp->legend_height = pp->layout.legend_height *c;
       }

#define dif(fld) (lp->fld != old.fld)
     changed = dif(x1) || dif(x2) || dif(y1) || dif(y2);
#undef dif

     return changed;
}


static Boolean Layout(pw)
AtPlotterWidget pw;
{
     AtPlotterLayout *lp = &pw->plotter.layout;
     AtPlotterPart *pp = &pw->plotter;
     AtPlotterLayout old;
     int new_x0, new_y0, new_w, new_h;
     int xwid, ywid, x2wid, y2wid;
     Boolean changed;
     AtAxisCoreWidget    xaxis;

     /* Layout is set from axis positions! No recalc needed */
     if (pp->positions.position)
	  return False;

     if (pp->ifzoom == PLOT_NORMAL) 
       pp->layout_bak = pp->layout;
     if (pp->ifzoom == PLOT_SCALE) {
       changed = scaleLayout(pw);
       return(changed);
       }
     old = pp->layout;

     /*lp->x1 = MARGIN*2 + pp->margin_width + pp->shadow_thickness;
     lp->y1 = MARGIN + pp->margin_height + pp->shadow_thickness;
     lp->x2 = pw->core.width - 1 - MARGIN*4 - pp->margin_width*2 - 
	      pp->shadow_thickness*2;
     lp->y2 = pw->core.height - 1 - MARGIN*2 - pp->margin_height*2 - 
	      pp->shadow_thickness*2; */

     /*if the new graph is larger than the old graph, the margins are negative,
     * otherwise the margins are possitive */
/*     new_x0 = pp->graph_margin_left;
     new_y0 = pp->graph_margin_top;
     new_w = pw->core.width - pp->graph_margin_right;
     new_h = pw->core.height - pp->graph_margin_bottom;
*/
     /* The basic layout */
     lp->x1 = pp->shadow_thickness + pp->graph_margin_left + pp->margin_width;
     lp->y1 = pp->shadow_thickness + pp->graph_margin_top + pp->margin_height;
     lp->x2 = pw->core.width - 1 - pp->shadow_thickness - pp->graph_margin_right - pp->margin_width;
     lp->y2 = pw->core.height - 1 - pp->shadow_thickness - pp->graph_margin_bottom - pp->margin_height;

     /* The legend at RHS or LHS of the plotting area */
     if ((pp->show_legend)&&(pp->set_labels)) {
	  if (pp->legend_left) {
	       lp->legend_x = lp->x1 + pp->margin_width;
	       lp->x1 += lp->legend_width + pp->margin_width;
	  }
	  else {
	       lp->x2 -= lp->legend_width + pp->margin_width;
	       lp->legend_x = lp->x2 + pp->margin_width;
	  }
     }

     /* Assume title at top at present */
     if (pp->show_title && pp->title_text) {
	  lp->title_y = lp->y1 + MARGIN + 
	      ((AtTextAscent(pp->title_text) > 0) ?
		  AtTextAscent(pp->title_text)-1 : 0);
	  if (pp->title_height > 0)
	       lp->y1 += pp->title_height + pp->margin_height;
	  else 
	       lp->y1 += AtTextHeight(pp->title_text) + pp->margin_height;
     }

     /* Calculate the "width" of the axes */
     xwid = ywid = x2wid = y2wid = 0;
     if (pp->xaxis && ISDISPLAYED(pp->xaxis))
	  xwid = AtAxisWidth(pp->xaxis);
     else xwid = MARGIN *4;
     if (pp->yaxis && ISDISPLAYED(pp->yaxis))
	  ywid = AtAxisWidth(pp->yaxis);
     else ywid = MARGIN *4;
     /* x2axis is for the x scale of y2 plot. No display 
     if (pp->x2axis && ISDISPLAYED(pp->x2axis))
	  x2wid = AtAxisWidth(pp->x2axis);
     else x2wid = MARGIN *4;*/
     x2wid = MARGIN *4;
     /*if (pp->y2axis && ISDISPLAYED(pp->y2axis))*/
     if (pp->plot_data2)
	  y2wid = AtAxisWidth(pp->y2axis);
     else y2wid = MARGIN *4;

     lp->y1 += x2wid;
     lp->y2 -= xwid;
     lp->x1 += ywid;
     lp->x2 -= y2wid;

     xaxis = (AtAxisCoreWidget)(pp->xaxis);
     /*if ((xaxis->axiscore.max_num_height+1)/2 > (pw->core.width -1 - lp->x2))
       lp->x2 = pw->core.width -1 - (xaxis->axiscore.max_num_height+1)/2;
*/
     lp->width  = lp->x2 - lp->x1 + 1;
     lp->height = lp->y2 - lp->y1 + 1;

     /* Don't infinite-loop in braindead small windows! */
     if ((lp->x2 - lp->x1) < 1) {
	  lp->width = 1;
	  lp->x2 = lp->x1 + 1;
     } else lp->width  = lp->x2 - lp->x1 + 1;
     if ((lp->y2 - lp->y1) < 1) {
	  lp->height = 1;
	  lp->y2 = lp->y1 + 1;
     } else lp->height = lp->y2 - lp->y1 + 1;

     if (pp->show_title && pp->title_text) {
	  lp->title_x = lp->x1 +
	       (lp->width - AtTextWidth(pp->title_text)) / 2;
     }

     if (pp->show_legend) {
	  lp->legend_y = lp->y1 + (lp->height - lp->legend_height)/2;
     }

#define dif(fld) (lp->fld != old.fld)
     changed = dif(x1) || dif(x2) || dif(y1) || dif(y2);
#undef dif

     return changed;
}

/*
 *   Get the current axis positions from the plotter layout.
 */

static void GetAxisPositions(pw, ap)
AtPlotterWidget pw;
AtAxisPositions *ap;
{
     AtPlotterLayout *lp = &pw->plotter.layout;

     /* Calc the relative positions of the axes from layout */
     ap->yaxis  = pw->core.x + lp->x1;
     ap->y2axis = pw->core.x + lp->x2;
     ap->xaxis  = pw->core.y + lp->y2;
     ap->x2axis = pw->core.y + lp->y1;
     ap->position = AtPositionNONE;
}

/*
 *   Recalc the plotter layout from possible external given axis positions.
 *   Dependent on required axes as defined in the position mask
 *   set the axis positions and recalc legend width and/or title height.
 *   Return True if any axis position has changed, False otherwise.
 */

static Boolean SetAxisPositions(pw, np)
AtPlotterWidget pw;
AtAxisPositions *np;
{
     AtPlotterPart *pp = &pw->plotter;
     AtPlotterLayout *lp = &pw->plotter.layout;
     AtAxisPositions *ap = &pw->plotter.positions;
     Dimension w;
     Boolean changed = False;

     /* Check for changes */
     ap->position = AtPositionNONE;
#define dif(v) (np->v != ap->v)
     if (np->position & AtPositionXAXES) {
	  if(dif(xaxis) || dif(x2axis)) {
	       changed = True;
	       ap->position |= AtPositionXAXES;
	       ap->xaxis  = np->xaxis;
	       ap->x2axis = np->x2axis;
	  }
     }
     if (np->position & AtPositionYAXES) {
	  if(dif(yaxis) || dif(y2axis)) {
	       changed = True;
	       ap->position |= AtPositionYAXES;
	       ap->yaxis  = np->yaxis;
	       ap->y2axis = np->y2axis;
	  }
     }
#undef dif

     if (changed) {
	  /* Layout from X axis positions */
	  if (ap->position & AtPositionXAXES) {
	       lp->y1 = ap->x2axis - pw->core.y;
	       lp->y2 = ap->xaxis  - pw->core.y;
	       lp->height = lp->y2 - lp->y1 + 1;
	  }
	  /* Layout from Y axis positions */
	  if (ap->position & AtPositionYAXES) {
	       lp->x1 = ap->yaxis  - pw->core.x;
	       lp->x2 = ap->y2axis - pw->core.x;
	       lp->width  = lp->x2 - lp->x1 + 1;
	  }
	  /* Legend */
	  if (pp->show_legend) {
	       w = 0;
	       if (pp->legend_left) {
		    if (pp->yaxis && ISDISPLAYED(pp->yaxis))
			 w = AtAxisWidth(pp->yaxis);
		    lp->legend_x = lp->x1 - w - lp->legend_width
				   - pp->margin_width;
	       }
	       else {
		    if (pp->y2axis && ISDISPLAYED(pp->y2axis))
			 w = AtAxisWidth(pp->y2axis);
		    lp->legend_x = lp->x2 + w + pp->margin_width;
	       }
	       lp->legend_y = lp->y1 + (lp->height - lp->legend_height)/2;
	  }
	  /* Title */
	  if (pp->show_title && pp->title_text) {
	       lp->title_y = pp->margin_height + 
	          (AtTextAscent(pp->title_text) > 0) ?
		     AtTextAscent(pp->title_text)-1 : 0;
	       lp->title_x = lp->x1 + (lp->width
			     - AtTextWidth(pp->title_text)) / 2;
	  }
     }

     return changed;
} /*SetAxisPositions*/

/* get high pixel and low pixel from xaxis to x2axis */
int X2AxisAskRange(AtAxisCoreWidget x2axis, AtAxisCoreWidget xaxis, 
        AtPlotterBoundingBox *bb)
{

  x2axis->axiscore.scale->lowpix = xaxis->axiscore.scale->lowpix;
  x2axis->axiscore.scale->highpix = xaxis->axiscore.scale->highpix;
  x2axis->axiscore.scale->low = bb->x2min;
  x2axis->axiscore.scale->high = bb->x2max;
  AtScaleCalc(x2axis->axiscore.scale);
}


/*
 *   Ask the axes to actually decide on endpoints.
 *   Returns True if any endpoints changed.
 */

static Boolean DecideAxisValues P((AtPlotterWidget));
static Boolean DecideAxisValues(pw)
AtPlotterWidget pw;
{
     AtPlotterBoundingBox *bbp = &pw->plotter.bounding_box;
     AtPlotterPart *pp = &pw->plotter;
     AtPlotterBoundingBox obb;
     float dummy_min, dummy_max;
     Boolean changed, datax = False, datay = False, datay2 = False;
     
     /* First, make a copy of the raw bounding box as a starting point. */
     obb = pw->plotter.bounding_box;
     pw->plotter.bounding_box = pw->plotter.raw_bounding_box;

     /*
      * Is there is some data plot depending on this axis, make sure
      * we have a valid one.  Otherwise, if there is a displayed axis
      * connected to it, do a dummy AskRange so that frame axes can
      * get min/max set.
      */
     if (bbp->xmax > bbp->xmin) {
	  datax = True;
	  if (pp->xaxis)
	       AtAxisAskRange(pp->xaxis, &bbp->xmin, &bbp->xmax);
	  else
	       XtAppError(XtWidgetToApplicationContext((Widget) pw),
			  "AtPlotter has no X axis defined");
     }
     else if (pp->xaxis && ISDISPLAYED(pp->xaxis)) {
	  dummy_min = LIN_MIN;
	  dummy_max = LIN_MAX;
	  AtAxisAskRange(pp->xaxis, &dummy_min, &dummy_max);
     }
     if (bbp->ymax > bbp->ymin) {
	  datay = True;
	  if (pp->yaxis)
	       AtAxisAskRange(pp->yaxis, &bbp->ymin, &bbp->ymax);
	  else
	       XtAppError(XtWidgetToApplicationContext((Widget) pw),
			  "AtPlotter has no Y axis defined");
     }
     else if (pp->yaxis && ISDISPLAYED(pp->yaxis)) {
	  dummy_min = LIN_MIN;
	  dummy_max = LIN_MAX;
	  AtAxisAskRange(pp->yaxis, &dummy_min, &dummy_max);
     }

  /*   if (bbp->x2max > bbp->x2min) {
       
	  if (pp->x2axis)
	       AtAxisAskRange(pp->x2axis, &bbp->x2min, &bbp->x2max);
	  else
	       XtAppError(XtWidgetToApplicationContext((Widget) pw),
			  "AtPlotter has no X2 axis defined");
        
     }
     else if (pp->x2axis && ISDISPLAYED(pp->x2axis)) {
	  dummy_min = LIN_MIN;
	  dummy_max = LIN_MAX;
	  AtAxisAskRange(pp->x2axis, &dummy_min, &dummy_max);
     }*/

     if (pp->plot_data2)
     if (bbp->y2max > bbp->y2min) {
	  datay2 = True;
	  if (pp->y2axis)
	       AtAxisAskRange(pp->y2axis, &bbp->y2min, &bbp->y2max);
	  else
	       XtAppError(XtWidgetToApplicationContext((Widget) pw),
			  "AtPlotter has no Y2 axis defined");
     }
     else if (pp->y2axis && ISDISPLAYED(pp->y2axis)) {
	  dummy_min = LIN_MIN;
	  dummy_max = LIN_MAX;
	  AtAxisAskRange(pp->y2axis, &dummy_min, &dummy_max);
     }
     
/* try to set XtNplotXmax, XtNplotXMin, XtNplotYMax, XtNplotYMin of plotter */
    pp->xaxis_max = (datax) ? bbp->xmax : LIN_MAX;
    pp->xaxis_min = (datax) ? bbp->xmin : LIN_MIN;
    pp->yaxis_max = (datay) ? bbp->ymax : LIN_MAX;
    pp->yaxis_min = (datay) ? bbp->ymin : LIN_MIN; 
    pp->y2axis_max = (datay) ? bbp->y2max : LIN_MAX;
    pp->y2axis_min = (datay) ? bbp->y2min : LIN_MIN; 
    if (pp->ifzoom == PLOT_NORMAL) {
      pp->xmax = pp->xaxis_max;
      pp->xmin = pp->xaxis_min;
      pp->ymax = pp->yaxis_max;
      pp->ymin = pp->yaxis_min; 
      pp->y2max = pp->y2axis_max;
      pp->y2min = pp->y2axis_min; 
      }
    if (!datax) {
      bbp->xmin = LIN_MIN;
      bbp->xmax = LIN_MAX;
      }
    if (!datay) {
      bbp->ymin = LIN_MIN;
      bbp->ymax = LIN_MAX;
      }
    if (!datay2) {
      bbp->y2min = LIN_MIN;
      bbp->y2max = LIN_MAX;
      }
#define dif(fld) (obb.fld != bbp->fld)
     /*changed = dif(xmin) || dif (xmax) || dif(x2min) || dif (x2max) ||
	       dif(ymin) || dif (ymax) || dif(y2min) || dif (y2max);*/
     if (!datax || !datay || !datay2) changed = True;
     else
       changed = dif(xmin) || dif (xmax) ||
	       dif(ymin) || dif (ymax) || dif(y2min) || dif (y2max);
#undef dif

     return changed;
} /* end of DecideAxisValues */

/*
 *   Actually redraw the entire plot
 */

int Redraw(pw, win, drw, region)
AtPlotterWidget pw;
Window win;
Drawable drw;
Region region;
{
     AtPlotterPart *pp = &pw->plotter;
     AtPlotterLayout *lp = &pw->plotter.layout;
     int i;
     TextList *next;
     Dimension w, h; 
     XPoint poly[5];
     AtAxisCoreWidget xpw = (AtAxisCoreWidget)XtNameToWidget((Widget)pw, "x_axis");
     float newx;
     int x;
     AtAxisCoreWidget tmpchild;
   
     w = ((Widget)pw)->core.width;
     h = ((Widget)pw)->core.height;

     /* Now the plot area if needed */
   /*  if (pw->core.background_pixel != pw->plotter.plotarea_color) { */
	  if (region) {
	       XSetRegion(XtDisplay(pw), pp->graph_gc, region);
	       XSetRegion(XtDisplay(pw), pp->plotarea_gc, region);
	       }
           
	  XFillRectangle(XtDisplay(pw), drw, pp->graph_gc,
			 pp->shadow_thickness, pp->shadow_thickness, 
			 w - 2 * pp->shadow_thickness, 
			 h - 2 * pp->shadow_thickness); 
	  XFillRectangle(XtDisplay(pw), drw, pp->plotarea_gc,
			 lp->x1, lp->y1, lp->width, lp->height);
	  if (region) {
	       XSetClipMask(XtDisplay(pw), pw->plotter.graph_gc, None);
	       XSetClipMask(XtDisplay(pw), pw->plotter.plotarea_gc, None);
	       }
 /*    }  */

     /* First, the title */
     if (pw->plotter.show_title && pw->plotter.title_text) {
	  At2TextDraw(XtDisplay(pw), win, drw, pw->plotter.title_gc,
		     pw->plotter.title_text, pw->plotter.layout.title_x,
		     pw->plotter.layout.title_y);
     }
/*if (!pp->plot_data && !pp->plot_data2 ) return(1);*/
     /* Now the legend */
     if (pw->plotter.show_legend && pw->plotter.set_labels)
	  RedrawLegend(pw, region, False);

     /* Now the children, either in rank order or birth order */
     if (pw->plotter.rank_children) {
	  Rank *rp;

	  for (rp = pw->plotter.ordered_children; rp; rp = rp->next) {
	       if (!ISDISPLAYED(rp->child)) {
		 continue;
               }
	       AtPlotDraw(rp->child, XtDisplay(pw), drw, region, False);
	  }
     } else {
	  for (i = 0; i < NUMCHILDREN(pw); i++) {
	       if (!NTHCHILDISDISPLAYED(pw, i)) continue;
	       tmpchild = (AtAxisCoreWidget)CHILD(pw, i);
	       if (tmpchild==pp->x2axis) continue;
	       if ((tmpchild==pp->y2axis) && (!pp->plot_data2)) continue;
	       AtPlotDraw(CHILD(pw, i), XtDisplay(pw), drw, region, False);
	  }
     }
  if (pw->plotter.text_list) {
    next=pw->plotter.text_list;
    do {
      /*if (next->th->state == TEXT_ON) */
      if (next->th->attached) {
	PlotTextGetPosition((Widget)pw, next->th);
	XCopyArea(XtDisplay(pw), next->th->newp, drw, next->th->gc, 0, 0,
	  next->th->width, next->th->height, next->th->x, next->th->y);
        next->th->display_state = DISPLAYED;
	if (next->th->td.connected) 
	  XDrawLine(XtDisplay(pw), drw, next->th->gc, next->th->x1,
	    next->th->y1, next->th->x2, next->th->y2);
	}
      next=next->next;
    }while (next);
  }
/*  if (pp->marker_show) {
      newx = pp->xmarker;
      x =  AtScaleUserToPixel(xpw->axiscore.scale, newx);
      XDrawLine(XtDisplay(pw), XtWindow(pw), pp->axis_gc, 
	x, pp->layout.y1, x, pp->layout.y2);
     }*/
	  
	  poly[0].x = 0; poly[0].y = 0;
	  poly[1].x = w; poly[1].y = 0;
          poly[2].x = w - pp->shadow_thickness; 
	  poly[2].y = pp->shadow_thickness;
	  poly[3].x = pp->shadow_thickness; 
	  poly[3].y = pp->shadow_thickness;
	  poly[4].x = 0; poly[4].y = 0;
	  XFillPolygon(XtDisplay(pw), drw, pp->top_shadow_gc,
	       poly, 5, Complex, CoordModeOrigin);
          poly[1].x = 0; poly[1].y = h;
	  poly[2].x = pp->shadow_thickness; 
	  poly[2].y = h - pp->shadow_thickness;
          XFillPolygon(XtDisplay(pw), drw, pp->top_shadow_gc,
	       poly, 5, Complex, CoordModeOrigin);
          poly[0].x = w; poly[0].y = h;
	  poly[3].x = w - pp->shadow_thickness; 
          poly[3].y = h - pp->shadow_thickness;
          poly[4].x = w; poly[4].y = h;
          XFillPolygon(XtDisplay(pw), drw, pp->bottom_shadow_gc,
	       poly, 5, Complex, CoordModeOrigin);
          poly[1].x = w; poly[1].y = 0;
          poly[2].x = w - pp->shadow_thickness;
          poly[2].y = pp->shadow_thickness;
          XFillPolygon(XtDisplay(pw), drw, pp->bottom_shadow_gc,           
               poly, 5, Complex, CoordModeOrigin);
  if (pp->ifzoom == PLOT_NORMAL)
    XCopyArea(XtDisplay(pw), pp->pixmap, pp->pixmap_backup, pp->pixmap_gc,
      0,0, pp->pixmap_width, pp->pixmap_height, 0, 0);
}

/*
 *   Redisplay
 *        General redisplay function called on exposure events.
 *        THIS IS THE ONLY ROUTINE THAT DRAWS ON THE SCREEN!
 *
 *   The algorithm:
 *
 *   Clear the screen/pixmap if required.
 *   If rescale is required, calculate min/max of axes
 *        (so layout knows how wide axes are);
 *   Do global relayout if required.
 *   Get axis positions from layout and call layout callback if required.
 *        If there were any changes in axis positions set them.
 *   Do global rescale if required (e.g. if relayout changed pixel size)
 *        Else rescale each plot according to extended_list, if any
 *   Then do the redraw, clipped by the region (if any);
 *        Either a full redraw:
 *             title
 *             legend
 *             axes
 *             each displayed child in order (rank or birth)
 *        Or a refresh of each one if requested.
 *   Forget all requested redraws/rescales/etc.
 */

static int Redisplay(pw, event, region)
AtPlotterWidget pw;
XEvent *event;
Region region;
{
#define ev ((XExposeEvent *) event)
     AtPlotterPart *pp = &pw->plotter;
     Drawable drw;
     Window win;
     AtBusyCallbackData cbd;
     Boolean pixels_moved = False;
     Boolean numbers_moved = False;
     Boolean full_refresh = False;
     Boolean reposition =False; /*for text */
     int i;
     TextList *next;
     AtAxisCoreWidget xpw = (AtAxisCoreWidget)XtNameToWidget((Widget)pw, "x_axis");
     float newx;
     int x;
     int try_recalc = 0;
     /* Don't redraw when a synthetic events occurs! */
     if ( !pp->auto_redisplay && ev->send_event) {
	  if (!pp->auto_redisplay)
	  pp->redisplay_required = True;
	  return;
     }
     win = XtWindow(pw);

     /* if expose event happens, copy the pixmap to window, or redraw without recalc */
     if (!pp->layout_required && !pp->rescale_required && !pp->redraw_required
	 && !pp->position_required && !pp->redisplay_required) {
	 if (pp->pixmap && pp->pixmap_gc) 
	   XCopyArea(XtDisplay(pw), pp->pixmap, win, pp->pixmap_gc, 0, 0,
		    pp->pixmap_width, pp->pixmap_height, 0, 0);
	 return;
	 }

     /* Set the busy cursor */
     if (pp->use_cursors)
	  XDefineCursor(XtDisplay(pw), XtWindow(pw), pp->busy_cursor);

     /* Deliver the busy callback */
     /*if (HasBusyCB(pw)) {
	  cbd.reason = AtBusyPLOTTER;
	  cbd.busy = True;
	  XtCallCallbacks((Widget) pw, XtNbusyCallback, (XtPointer) &cbd);
     }
     else
	  cbd.busy = False; */

     
     /* Get the pixmap if needed */
     if (pp->pixmap_required) {
	  GetPixmap(pw);
	  }
     drw = pp->pixmap; 

     if (pp->redisplay_required) {      /* Redraw all! */
	  full_refresh = True;
	  region = NULL;
     }
     else if (ev->send_event)           /* Is synthetic! */
	  region = NULL;

     /*
      * If the event covers (nearly) the whole window,
      * ignore the region (for speed!)
      */
     if (region) {
	  if (ev->x < 10 && ev->y < 10 &&
	      ev->width > pw->core.width - 20 &&
	      ev->height > pw->core.height - 20)
	       region = NULL;
	  full_refresh = True;
     }
#undef ev

     /*
      * Come back to here if the length of an axis changed by > 25%
      * and the axis indicates tic_interval et al needs recalculating,
      * or if one of the calc routines has set rescale_required or
      * layout_required (probably an axis because something like the
      * number width has changed).
      */

recalc_again:
     try_recalc++;
     pp->in_layout_mode = True;

     if (pp->rescale_required)
	  numbers_moved |= DecideAxisValues(pw);

     if (pp->layout_required || RecalcLegend(pw))
	  pixels_moved |= Layout(pw); 

     /*
      * Get the current axis positions and call layout callback list procs
      * to give applications or other plotters the chance to do some axis
      * alignments if this is needed.
      */
    /* if (HasLayoutCB(pw)) {
	  AtAxisPositions ap;

	  GetAxisPositions(pw, &ap);
	  XtCallCallbacks((Widget) pw, XtNlayoutCallback, (XtPointer) &ap);
	  if (SetAxisPositions(pw, &ap))
		pixels_moved = pp->redraw_required = True;
     }*/
     /*
      * Layout of axis positions are set from axis position alignments
      */
     if (pp->position_required)
	  pixels_moved = pp->redraw_required = True;

     if (pixels_moved) {
	  AtPlotterLayout *lp = &pp->layout;
	  Boolean ti_changed = False;

	  /*
	   * With the y axes, must swap min and max values as window is
	   * measured 0=top, we want 0 to be bottom.
	   */
	  if (pp->xaxis && ISDISPLAYED(pp->xaxis))
	       ti_changed |=
		    AtAxisSetPosition(pp->xaxis, lp->x1, lp->y2, lp->x2,
				      lp->y2, lp->y2 - lp->y1);
	  if (pp->yaxis && ISDISPLAYED(pp->yaxis))
	       ti_changed |=
		    AtAxisSetPosition(pp->yaxis, lp->x1, lp->y2, lp->x1,
				      lp->y1, lp->x2 - lp->x1);
        /*  if (pp->x2axis && pp->plot_data2) 
	     AtScaleResize(pp->x2axis->axiscore.scale, lp->x1, lp->x2);

	  if (pp->x2axis && ISDISPLAYED(pp->x2axis))
	       ti_changed |=
		    AtAxisSetPosition(pp->x2axis, lp->x1, lp->y1, lp->x2,
				      lp->y1, lp->y2 - lp->y1); */
	  if (pp->plot_data2 && pp->y2axis && ISDISPLAYED(pp->y2axis))
	       ti_changed |=
		    AtAxisSetPosition(pp->y2axis, lp->x2, lp->y2, lp->x2,
				      lp->y1, lp->x2 - lp->x1);
	  if (ti_changed) {
	    if ((try_recalc<4) && (pp->ifzoom==PLOT_NORMAL)) 
	       goto recalc_again; 
	       }
     }

     pp->rescale_required = pp->layout_required = False;

     if (pixels_moved || numbers_moved) {
	  /* Need to rescale the entire graph */
	  /* Must do the axes first, as they may request recalc */
	  if (pp->xaxis && ISDISPLAYED(pp->xaxis))
	       AtPlotRecalc((AtPlotWidget) pp->xaxis, NULL, NULL, 0, 0);
	/*  if (pp->x2axis && ISDISPLAYED(pp->x2axis))
	       AtPlotRecalc((AtPlotWidget) pp->x2axis, NULL, NULL, 0, 0); */
	  if (pp->yaxis && ISDISPLAYED(pp->yaxis))
	       AtPlotRecalc((AtPlotWidget) pp->yaxis, NULL, NULL, 0, 0);
	  if (pp->plot_data2)
	    if (pp->y2axis && ISDISPLAYED(pp->y2axis)) {
	       AtPlotRecalc((AtPlotWidget) pp->y2axis, NULL, NULL, 0, 0);
               }

	  if (pp->layout_required || pp->rescale_required) {
	    if ((try_recalc<4) && (pp->ifzoom==PLOT_NORMAL)) 
	       goto recalc_again; 
	  }

	  for (i = 0; i < NUMCHILDREN(pw); i++) {
	       AtPlotWidget ch = CHILD(pw, i);
#define cha (AtAxisCoreWidget) ch

	       if ( !ISDISPLAYED(ch))
		    continue;
	       if (XtIsSubclass((Widget) ch, atAxisCoreWidgetClass)) {
		    if (cha != pp->xaxis && cha != pp->yaxis &&
			cha != pp->x2axis && cha != pp->y2axis) {
			 XtAppWarning(XtWidgetToApplicationContext((Widget) ch),
				      "AtAxisCore is displayed but not attached");
		    }
	       }
	       else {
		    AtPlotRecalc(ch,
				 /*AtAxisGetScale(USESX2AXIS(ch) ?
						pp->x2axis : pp->xaxis),*/
				 AtAxisGetScale(pp->xaxis),
				 AtAxisGetScale(USESY2AXIS(ch) ?
						pp->y2axis : pp->yaxis),
				 0, -1);
	       }
	  }
     }
     else {
	  /* Not entire graph, perhaps individual chunks? */
	  for (i = 0; i < NUMCHILDREN(pw); i++) {
	       ExtendedList *ep;

	       if ( !NTHCHILDISDISPLAYED(pw, i) ||
		    !(ep = CONSTRAINT(pw, i)->plotter.extended_list))
		    continue;
	       while (ep) {
	         if (XtIsSubclass((Widget) CHILD(pw, i), atPlotWidgetClass)) 
		    AtPlotRecalc(CHILD(pw, i),
			/*AtAxisGetScale(USESX2AXIS(CHILD(pw, i)) ?
						pp->x2axis : pp->xaxis),*/
			AtAxisGetScale(pp->xaxis),
			AtAxisGetScale(USESY2AXIS(CHILD(pw, i)) ?
						pp->y2axis : pp->yaxis),
				 ep->from, ep->to);
		    ep = ep->next;
	       }
	       if (XtIsSubclass((Widget)CHILD(pw, i), atPlotWidgetClass)) 
	       CONSTRAINT(pw, i)->plotter.needs_refresh = True;
	  }
     }

     if (pp->layout_required || pp->rescale_required) {
	  reposition = True; /* for text reposition */
	    if ((try_recalc<4) && (pp->ifzoom==PLOT_NORMAL)) 
	  goto recalc_again; 
     }
/* position or reposition of the text */
  
  if (pw->plotter.text_list) {
    next=pw->plotter.text_list;
    do {
      if (!next->th->positioned) {
	PlotTextGetPosition((Widget)pw, next->th);
	next->th->positioned = True;
	}
      else if (reposition && 
	(next->th->td.position.pixel.type!=PLOT_TEXT_ATTACH_PIXEL))
	    PlotTextGetPosition((Widget)pw, next->th);
      next=next->next;
    }while (next);
  }


/* end of recalc */


     /* First, clear the screen/pixmap if that has been requested */
     if (pp->redraw_required ||
	 pp->expose_requested && (pixels_moved || numbers_moved)) {
	  if (pp->use_pixmap)
	       XFillRectangle(XtDisplay(pw), pp->pixmap, pp->pixmap_gc,
			      0, 0, pp->pixmap_width, pp->pixmap_height);
	  else
	       XClearWindow(XtDisplay(pw), win);
	  region = NULL;      /* Be sure: unset region! */
     }

    if (pp->redraw_required || pixels_moved || numbers_moved) {
	  Redraw(pw, win, drw, region); /* Redraw the whole lot */

	  if (pp->can_slide)            /* May be, drag postions have changed */
	       SetDragPositions(pw);
     }
     else {
	  /* Perhaps one of the plots wants redrawing */
	  if (pp->rank_children) {
	    Rank *rp;

	    for (rp = pp->ordered_children; rp; rp = rp->next) {
	      if ( !ISDISPLAYED(rp->child))
		 continue;
	      if (XtIsSubclass((Widget)rp->child, atPlotWidgetClass)) 
	        if (CONSTRAINTS(rp->child)->plotter.needs_refresh)
		 AtPlotDraw(rp->child, XtDisplay(pw), drw, region, True);
	       }
	  }
	  else {
	    for (i = 0; i < NUMCHILDREN(pw); i++) {
	      if ( !NTHCHILDISDISPLAYED(pw, i))
		 continue;
	      if (XtIsSubclass((Widget)CHILD(pw, i), atPlotWidgetClass))
	        if (CONSTRAINT(pw, i)->plotter.needs_refresh){
		  AtPlotDraw(CHILD(pw, i), XtDisplay(pw), drw, region, True);
		  }
	    }
	  }
     }

     /* Now forget all requests */
     pp->redraw_required = pp->rescale_required = pp->layout_required = False;
     pp->expose_requested = False;

     for (i = 0; i < NUMCHILDREN(pw); i++) {
	  AtPlotterConstraints c = CONSTRAINT(pw, i);

	  c->plotter.needs_refresh = False;
	  DestroyExtendedList(CHILD(pw, i));
	  c->plotter.extended_list = NULL;
     }

     /* Reset state variables */
     pp->in_layout_mode = False;
     pp->redisplay_required = False;

     /* Now set the current axis positions and reset flag */
     GetAxisPositions(pw, &pw->plotter.positions);
     pp->position_required = False;

     /* Copy pixmap to window */
     if(pp->use_pixmap)
	  XCopyArea(XtDisplay(pw), drw, win, pp->pixmap_gc, 0, 0,
		    pp->pixmap_width, pp->pixmap_height, 0, 0);
     PlotterDrawXMarker((Widget)pw);

     /* Deliver the busy callback */
     if (cbd.busy) {
	  cbd.reason = AtBusyPLOTTER;
	  cbd.busy = False;
	  XtCallCallbacks((Widget) pw, XtNbusyCallback, (XtPointer) &cbd);
     }

     /* Reset the current cursor */
     if (pp->use_cursors)
	  XDefineCursor(XtDisplay(pw), XtWindow(pw), pp->current_cursor);

  pp->displayed = True; /* for PlotText */

} /* end of Redisplay */

/*
 *   Destroy
 *   Clean up allocated resources when the widget is destroyed.
 */

static void Destroy(Widget w)
{
  AtPlotterWidget pw = (AtPlotterWidget) w;
  TextList *next;
  int i;

  if (pw) {
     AtPlotterPart *pp = &pw->plotter;

     /* Free up the private data */
     /* Free our private copies of string resource */ 
     FreeTitle(pw);
     XtReleaseGC((Widget) pw, pp->title_gc);
     XtReleaseGC((Widget) pw, pp->legend_gc);
     XtReleaseGC((Widget) pw, pp->select_gc);
     XtReleaseGC((Widget) pw, pp->drag_gc);
     XtReleaseGC((Widget) pw, pp->plotarea_gc);
     XtReleaseGC((Widget) pw, pp->axis_gc);
     XtReleaseGC((Widget) pw, pp->graph_gc);
     if (pp->marker_gc) XtReleaseGC((Widget) pw, pp->marker_gc);
     if (pp->top_shadow_gc) XtReleaseGC((Widget)pw, pp->top_shadow_gc);
     if (pp->bottom_shadow_gc) XtReleaseGC((Widget)pw, pp->bottom_shadow_gc);

     if (pp->text_list) {
       next=pp->text_list;
       do {
         if (next->th->gc) XtReleaseGC((Widget)pw, next->th->gc);
         next=next->next;
       }while (next);
     }

     /* Free the linked list of ordered_children */
     if (pp->ordered_children) {
	  Rank *tmp = pp->ordered_children;

	  while (tmp->next) {
	       tmp = tmp->next;
	       XtFree((char *) tmp->prev);
	  }
	  XtFree((char *) tmp);
     }

     /* Free the drag callback cursors */
     XFreeCursor(XtDisplay(pw), pp->top_left_cursor);
     XFreeCursor(XtDisplay(pw), pp->top_right_cursor);
     XFreeCursor(XtDisplay(pw), pp->bottom_left_cursor);
     XFreeCursor(XtDisplay(pw), pp->bottom_right_cursor);

     /* Free pixmap and pixmap gc */
     if (pp->pixmap)
	  FreePixmap(pw);
     if (pp->use_cursors) RemoveHandlers(pw);

     /* free widgets and data-styles */
     /*for (i=0; i<pp->ds_num; i++) {
       XtFree(pp->plot_data_styles[i]->color);
       pp->plot_data_styles[i]->color = NULL;
       XtFree(pp->plot_data_styles[i]->pcolor);
       pp->plot_data_styles[i]->pcolor = NULL;
       free(pp->plot_data_styles[i]);
       pp->plot_data_styles[i] = NULL;
       }
     if (pp->ds_num>0) {
       free (pp->plot_data_styles);
       pp->plot_data_styles = NULL;
       }
     for (i=0; i<pp->ds2_num; i++) {
       XtFree(pp->plot_data_styles2[i]->color);
       pp->plot_data_styles2[i]->color = NULL;
       XtFree(pp->plot_data_styles2[i]->pcolor);
       pp->plot_data_styles2[i]->pcolor = NULL;
       free(pp->plot_data_styles2[i]);
       pp->plot_data_styles2[i] = NULL;
       }
     if (pp->ds2_num>0) {
       free(pp->plot_data_styles2);
       pp->plot_data_styles2 = NULL;
       } */

     if (pp->data_widget_num >0) {
       free(pp->data_widget);
       pp->data_widget = NULL;
       }
     if (pp->data2_widget_num >0) {
       free(pp->data2_widget);
       pp->data2_widget = NULL;
       }
   }   
}

/*
 *   Resize
 */

static void Resize(pw)
AtPlotterWidget pw;
{
     if (XtWindow(pw))
	  LayoutRequired(pw);
     if (pw->plotter.use_pixmap)
	  pw->plotter.pixmap_required = True;
     if (pw->plotter.ifzoom != PLOT_NORMAL){
       pw->plotter.ifzoom=PLOT_NORMAL;
       XtVaSetValues((Widget)pw, XtNplotGraphMarginBottomUseDefault, True,
         XtNplotGraphMarginTopUseDefault, True,
         XtNplotGraphMarginLeftUseDefault, True,
         XtNplotGraphMarginRightUseDefault, True,
         XtNplotXMinUseDefault, True,
         XtNplotXMaxUseDefault, True,
         XtNplotYMaxUseDefault, True,
         XtNplotYMinUseDefault, True,
         XtNplotY2MaxUseDefault, True,
         XtNplotY2MinUseDefault, True,
         XtNplotXAxisMinUseDefault, True,
         XtNplotXAxisMaxUseDefault, True,
         XtNplotYAxisMaxUseDefault, True,
         XtNplotYAxisMinUseDefault, True,
         XtNplotY2AxisMaxUseDefault, True,
         XtNplotY2AxisMinUseDefault, True,
         NULL);
       }
}


/*
 *   Realize
 */

static void Realize(w, vm, wa)
AtPlotterWidget w;
XtValueMask *vm;
XSetWindowAttributes *wa;
{
  AtAxisCoreWidget xaxis, yaxis;
  AtPlotterPart *pp = &w->plotter;
  AtAxisCorePart *ac;
  TextList *next;

     (superclass->core_class.realize)((Widget) w, vm, wa);
     /* Calculate initial boundingbox */
     (void) NewRawBoundingBox(w);
     (void) RecalcLegend(w);
     /* Install enter/leave callbacks for internal cursors */
     if (w->plotter.use_cursors) {
	  InstallHandlers(w);
	  XDefineCursor(XtDisplay(w), XtWindow(w), w->plotter.plotter_cursor);
     }
  
  /* calculate the size of the text_area, get GC and Pixmap for the text_area,
     draw strings and borders on the pixmap*/
  if (w->plotter.text_list) {
    next=w->plotter.text_list;
    do {
	PlotTextCalc((Widget)w, next->th);
        PlotTextGetGC((Widget)w, next->th);
	PlotTextDraw((Widget)w, next->th);
      next=next->next;
    }while (next);
  }
} /* end of Realize */


int destroyDataWidget(AtPlotterPart *pp)
{
  int n = 0;
  
  if (!pp->data_widget) return(0);
  for (n=0; n<pp->data_widget_num; n++) 
     if (pp->data_widget[n]) {
       XtDestroyWidget(pp->data_widget[n]);
       pp->data_widget[n] = NULL;
     }
  pp->data_widget = NULL;
  pp->data_widget_num = 0;
}



/*
 *   SetValues
 */
static Boolean SetValues(current, request, new)
AtPlotterWidget current, request, new;
{
#define Changed(field) (new->plotter.field != current->plotter.field)
     AtPlotterPart *pp = &new->plotter;
     XGCValues gcv;
     Boolean redraw = False;
     Boolean layout = False;
     Boolean recalc_legend = False;
     Boolean rescale = False;
     Boolean mirror;
     int i;
     Arg args[30];
     int n = 0;
     Widget xaxis, yaxis, y2axis;
     XtGCMask value_mask = 0;
     AtAxisCoreWidget xpw = (AtAxisCoreWidget)XtNameToWidget((Widget)new, "x_axis");
     float oldx, newx;
     int x;

  yaxis = (Widget)pp->yaxis;
  y2axis = (Widget)pp->y2axis;
/* set widget foreground or background will effect all color-settings */
  if (Changed(graph_fore) && pp->graph_fore)
    if (!Changed(data_fore)) pp->data_fore = pp->graph_fore;
  if (Changed(graph_back) && pp->graph_back)
    if (!Changed(data_back)) pp->data_back = pp->graph_back;
  
  if (Changed(widget_fore) && pp->widget_fore) {
      if (!Changed(graph_fore)) pp->graph_fore = pp->widget_fore;
      if (!Changed(data_fore)) pp->data_fore = pp->widget_fore;
      if (!Changed(title_fore)) pp->title_fore = pp->widget_fore;
      if (!Changed(legend_fore)) pp->legend_fore = pp->widget_fore;
      pp->default_color[6] = PlotColor((Widget)new, pp->widget_fore, "black");
      redraw = True;
      }
  if (Changed(widget_back) && pp->widget_back) {
      if (!Changed(graph_back)) pp->graph_back = pp->widget_back;
      if (!Changed(data_back)) pp->data_back = pp->widget_back;
      if (!Changed(title_back)) pp->title_back = pp->widget_back;
      if (!Changed(legend_back)) pp->legend_back = pp->widget_back;
      pp->default_color[7] = PlotColor((Widget)new, pp->widget_back, "white");
      redraw = True;
      }

  if (Changed(graph_back)) {
    gcv.foreground = WnColorF((Widget)new, pp->graph_back);
    XtReleaseGC((Widget) new, pp->graph_gc);
    pp->graph_gc = XtGetGC((Widget)new, GCForeground, &gcv);
    }

  if (Changed(title_fore) || Changed(title_font)) {
    XtReleaseGC((Widget) new, pp->title_gc);
    value_mask = GCForeground | GCFont;
    if (pp->title_fore) gcv.foreground = WnColorF((Widget)new, pp->title_fore);
    else if (pp->widget_fore) 
	gcv.foreground = WnColorF((Widget)new, pp->widget_fore);
	else gcv.foreground = WnColorF((Widget)new, "black");
    
    if (Changed(title_font)) {
      AtTextRenewFont((Widget)new, pp->title_text, pp->title_font);
      }
    gcv.font = pp->title_font;
    pp->title_gc = XtGetGC((Widget) new, value_mask, &gcv);
    redraw = True;
    }

  if (Changed(legend_fore) || Changed(legend_font)) {
    XtReleaseGC((Widget) new, pp->legend_gc);
    value_mask = GCForeground | GCFont;
    if (pp->legend_fore) 
       gcv.foreground = WnColorF((Widget)new, pp->legend_fore);
    else if (pp->widget_fore) 
       gcv.foreground = WnColorF((Widget)new, pp->widget_fore);
       else gcv.foreground = WnColorF((Widget)new, "black");
    gcv.font = pp->legend_font;
    pp->legend_gc = XtGetGC((Widget) new, value_mask, &gcv);
    redraw = True;
    }

  /*if (Changed(legend_fore) || new->core.background_pixel != current->core.background_pixel) {
    XtReleaseGC((Widget) new, pp->select_gc);
    gcv.foreground = new->core.background_pixel;
    gcv.background = WnColorF((Widget)new, pp->legend_fore);
    pp->select_gc = XtGetGC((Widget) new, GCForeground|GCBackground, &gcv);
    redraw = True;
  }*/

  if (Changed(data_back)) {
    XtReleaseGC((Widget) new, pp->plotarea_gc);
    gcv.foreground = WnColorB((Widget)new, pp->data_back);
    pp->plotarea_color = gcv.foreground;
    pp->plotarea_gc = XtGetGC((Widget) new, GCForeground, &gcv);
    redraw = True;
   }

  if (Changed(show_title)) {
	  layout = redraw = True;
     }

  if (Changed(show_legend)) {
     if (pp->show_legend)
	       (void) RecalcLegend(new);
     layout = redraw = True;
     }

  if (Changed(legend_left)) {
	  if (pp->show_legend)
	       layout = redraw = True;
     }

  if (pp->show_legend &&
	 (Changed(legend_width) || Changed(legend_spacing))) {
	  recalc_legend = layout = redraw = True;
     }

  if (pp->show_title && Changed(title_height)) {
	  layout = redraw = True;
     }

  if (Changed(margin_width) || Changed(margin_height)) {
	  layout = True;
     }

  if (new->core.background_pixel != current->core.background_pixel) {
	  if (pp->use_pixmap)
	       GetPixmapGC(new);
     }

  if (Changed(axis_font) || Changed(graph_back) || Changed(graph_fore)) {
       XtReleaseGC((Widget)new, pp->axis_gc);
       gcv.foreground = WnColorF((Widget)new, pp->graph_fore);
       gcv.background = WnColorB((Widget)new, pp->graph_back);
       if (pp->axis_font) gcv.font = pp->axis_font;
       else gcv.font = pp->axis_font = 
	    XLoadFont(XtDisplay((Widget)new), "fixed");
       pp->axis_fs = XQueryFont(XtDisplay((Widget)new), pp->axis_font);
       gcv.line_width = 1;
       pp->axis_gc = XtGetGC((Widget)new, GCForeground |GCBackground | GCFont
	       | GCLineWidth, &gcv);
       if (pp->ytitle) 
	 if (pp->yaxis->axiscore.label_text) 
	   if (pp->yaxis->axiscore.label_text->rotated_image) {
	     XDestroyImage(pp->yaxis->axiscore.label_text->rotated_image);
	     pp->yaxis->axiscore.label_text->rotated_image = NULL;
	     }
       if (pp->y2title) 
	 if (pp->y2axis->axiscore.label_text) 
	   if (pp->y2axis->axiscore.label_text->rotated_image) {
	     XDestroyImage(pp->y2axis->axiscore.label_text->rotated_image);
	     pp->y2axis->axiscore.label_text->rotated_image = NULL;
	     }
       redraw = True;
      }

  if (Changed(time_base)) redraw = True;

  if (Changed(plot_type)) {
     for (i=0; i<pp->data_widget_num; i++) 
       if (pp->data_widget[i])
	 XtVaSetValues(pp->data_widget[i], XtNplotType, pp->plot_type, NULL);
     redraw = True;
     }

  if (Changed(plot_type2)) {
     for (i=0; i<pp->data2_widget_num; i++) 
       if (pp->data2_widget[i])
	 XtVaSetValues(pp->data2_widget[i], XtNplotType, pp->plot_type2, NULL);
     redraw = True;
     }
  
  if (Changed(graph_margin_left)) pp->graph_margin_left_dft = False;
  if (Changed(graph_margin_right)) pp->graph_margin_right_dft = False;
  if (Changed(graph_margin_top)) pp->graph_margin_top_dft = False;
  if (Changed(graph_margin_bottom)) pp->graph_margin_bottom_dft = False;
  if (Changed(graph_margin_left) || Changed(graph_margin_right) ||
	Changed(graph_margin_top) || Changed(graph_margin_bottom)) {
	       layout = True;
	       redraw = True;
     }
  else {
     if (Changed(graph_margin_left_dft) && pp->graph_margin_left_dft) 
	pp->graph_margin_left = 0;
     if (Changed(graph_margin_right_dft) && pp->graph_margin_right_dft) 
	pp->graph_margin_right = 0;
     if (Changed(graph_margin_top_dft) && pp->graph_margin_top_dft) 
	pp->graph_margin_top = 0;
     if (Changed(graph_margin_bottom_dft) && pp->graph_margin_bottom_dft) 
	pp->graph_margin_bottom = 0;
     if (Changed(graph_margin_left_dft) || Changed(graph_margin_right_dft) ||
       Changed(graph_margin_top_dft) || Changed(graph_margin_bottom_dft)) {
	       layout = True;
	       redraw = True;
	       }
     }
    
  xaxis = (Widget)pp->xaxis;
  if (xaxis) {
      n=0;
      if (Changed(xaxis_max)) {
	pp->xaxis_max_dft = False;
        XtSetArg(args[n], XtNmax, PlotFloatToArgVal(pp->xaxis_max)); n++;
	XtSetArg(args[n], XtNplotMaxUseDefault, False); n++;
	}
      else if (Changed(xmax)) {
	pp->xmax_dft = False;
	XtSetArg(args[n], XtNmax, PlotFloatToArgVal(pp->xmax)); n++;
        XtSetArg(args[n], XtNplotMaxUseDefault, False); n++;
        }  
      else if (Changed(xaxis_max_dft)) {
	XtSetArg(args[n], XtNplotMaxUseDefault, pp->xaxis_max_dft); n++;
	}
      else if (Changed(xmax_dft)) {
	XtSetArg(args[n], XtNplotMaxUseDefault, pp->xmax_dft); n++;
	}
      /*else if (Changed(xmax_dft) && pp->xmax_dft) {
	  XtSetArg(args[n], XtNplotMaxUseDefault, True); n++;
	  pp->xmax = dflt_huge;
	}*/
      /*if (Changed(xmin)) {
        XtSetArg(args[n], XtNmin, PlotFloatToArgVal(pp->xmin)); n++;
        XtSetArg(args[n], XtNplotMinUseDefault, False); n++;
        pp->xmin_dft = False;
      } else {
	if (Changed(xmin_dft) && pp->xmin_dft) {
          XtSetArg(args[n], XtNplotMinUseDefault, True); n++;
	  pp->xmin = dflt_huge;
	  }
        }
      if (n>0) {
        XtSetArg(args[n], XtNautoScale, False); n++;
        }*/
      if (Changed(xaxis_min)) {
	pp->xaxis_min_dft = False;
        XtSetArg(args[n], XtNmin, PlotFloatToArgVal(pp->xaxis_min)); n++;
	XtSetArg(args[n], XtNplotMinUseDefault, False); n++;
	}
      else if (Changed(xmin)) {
	pp->xmin_dft = False;
	XtSetArg(args[n], XtNmin, PlotFloatToArgVal(pp->xmin)); n++;
        XtSetArg(args[n], XtNplotMinUseDefault, False); n++;
        }  
      else if (Changed(xaxis_min_dft)) {
	XtSetArg(args[n], XtNplotMinUseDefault, pp->xaxis_min_dft); n++;
	}
      else if (Changed(xmin_dft)) {
	XtSetArg(args[n], XtNplotMinUseDefault, pp->xmin_dft); n++;
	}
      if (Changed(xanno_method)) {
	XtSetArg(args[n], XtNplotAnnotationMethod, pp->xanno_method); n++;
        if (pp->xanno_method == PLOT_ANNO_TIME_LABELS) {
          pp->xnum_dft = False;
          XtSetArg(args[n], XtNplotNumUseDefault, False); n++;
          if (pp->xnum==PLOTTER_HUGE_VAL) pp->xnum = 1;
          }
        }
      if (Changed(xnum)) {
	XtSetArg(args[n], XtNticInterval, PlotFloatToArgVal(pp->xnum)); n++;
	XtSetArg(args[n], XtNplotNumUseDefault, False); n++;
        pp->xnum_dft = False;
      } else {
	if (Changed(xnum_dft) && pp->xnum_dft) {
          XtSetArg(args[n], XtNplotNumUseDefault, True); n++;
	  pp->xnum = dflt_huge;
	  }
        }
      if (Changed(xtic)) {
	XtSetArg(args[n], XtNsubticInterval, PlotFloatToArgVal(pp->xtic)); n++;
	XtSetArg(args[n], XtNplotTickUseDefault, False); n++;
        pp->xtic_dft = False;
      } else {
	if (Changed(xtic_dft) && pp->xtic_dft) {
          XtSetArg(args[n], XtNplotTickUseDefault, True); n++;
	  pp->xtic = dflt_huge;
	  }
        }
      if (Changed(axis_font)) {
	XtSetArg(args[n], XtNplotAxisFont, pp->axis_font); n++;
	}
      if (Changed(draw_frame)) {
        XtSetArg(args[n], XtNdrawFrame, True); n++;
        }
      if (Changed(xtitle)) {
	XtSetArg(args[n], XtNlabel, pp->xtitle); n++;
	}
      if (Changed(time_format)) {  
	redraw = True;
	XtSetArg(args[n], XtNplotTimeFormat, pp->time_format); n++;
        }
      if (n>0) 
	XtSetValues(xaxis, args, n);
    }

  if (yaxis) {
      n=0;
     /* if (Changed(ymax)) {
        XtSetArg(args[n], XtNmax, PlotFloatToArgVal(pp->ymax)); n++;
	XtSetArg(args[n], XtNplotMaxUseDefault, False); n++;
        pp->ymax_dft = False;
      } else {
	if (Changed(ymax_dft) && pp->ymax_dft) {
          XtSetArg(args[n], XtNplotMaxUseDefault, True); n++;
	  pp->ymax = dflt_huge;
	  }
        }
      if (Changed(ymin)) {
        XtSetArg(args[n], XtNmin, PlotFloatToArgVal(pp->ymin)); n++;
        XtSetArg(args[n], XtNplotMinUseDefault, False); n++;
        pp->ymin_dft = False;
      } else {
	if (Changed(ymin_dft) && pp->ymin_dft) {
          XtSetArg(args[n], XtNplotMinUseDefault, True); n++;
	  pp->ymin = dflt_huge;
	  }
        }
      if (n>0) {
        XtSetArg(args[n], XtNautoScale, False); n++;
        }
	*/
      if (Changed(yaxis_max)) {
	pp->yaxis_max_dft = False;
        XtSetArg(args[n], XtNmax, PlotFloatToArgVal(pp->yaxis_max)); n++;
	XtSetArg(args[n], XtNplotMaxUseDefault, False); n++;
	}
      else if (Changed(ymax)) {
	pp->ymax_dft = False;
	XtSetArg(args[n], XtNmax, PlotFloatToArgVal(pp->ymax)); n++;
        XtSetArg(args[n], XtNplotMaxUseDefault, False); n++;
        }  
      else if (Changed(yaxis_max_dft)) {
	XtSetArg(args[n], XtNplotMaxUseDefault, pp->yaxis_max_dft); n++;
	}
      else if (Changed(ymax_dft)) {
	XtSetArg(args[n], XtNplotMaxUseDefault, pp->ymax_dft); n++;
	}
      if (Changed(yaxis_min)) {
	pp->yaxis_min_dft = False;
        XtSetArg(args[n], XtNmin, PlotFloatToArgVal(pp->yaxis_min)); n++;
	XtSetArg(args[n], XtNplotMinUseDefault, False); n++;
	}
      else if (Changed(ymin)) {
	pp->ymin_dft = False;
	XtSetArg(args[n], XtNmin, PlotFloatToArgVal(pp->ymin)); n++;
        XtSetArg(args[n], XtNplotMinUseDefault, False); n++;
        }  
      else if (Changed(yaxis_min_dft)) {
	XtSetArg(args[n], XtNplotMinUseDefault, pp->yaxis_min_dft); n++;
	}
      else if (Changed(ymin_dft)) {
	XtSetArg(args[n], XtNplotMinUseDefault, pp->ymin_dft); n++;
	}
      if (Changed(ynum)) {
	XtSetArg(args[n], XtNticInterval, PlotFloatToArgVal(pp->ynum)); n++;
	XtSetArg(args[n], XtNplotNumUseDefault, False); n++;
        pp->ynum_dft = False;
      } else {
	if (Changed(ynum_dft) && pp->ynum_dft) {
          XtSetArg(args[n], XtNplotNumUseDefault, True); n++;
	  pp->ynum = dflt_huge;
	  }
        }
      if (Changed(ytic)) {
	XtSetArg(args[n], XtNsubticInterval, PlotFloatToArgVal(pp->ytic)); n++;
	XtSetArg(args[n], XtNplotTickUseDefault, False); n++;
        pp->ytic_dft = False;
      } else {
	if (Changed(ytic_dft) && pp->ytic_dft) {
          XtSetArg(args[n], XtNplotTickUseDefault, True); n++;
	  pp->ytic = dflt_huge;
	  }
        }
      if (Changed(axis_font)) {
	XtSetArg(args[n], XtNplotAxisFont, pp->axis_font); n++;
	}
      if (Changed(draw_frame)) {
        XtSetArg(args[n], XtNdrawFrame, True); n++;
        }
      if (Changed(ytitle)) {
	XtSetArg(args[n], XtNlabel, pp->ytitle); n++;
	}
      if (Changed(ylog)) {
        if (pp->ylog) 
          XtSetArg(args[n], XtNaxisTransform, AtTransformLOGARITHMIC); 
        else XtSetArg(args[n], XtNaxisTransform, AtTransformLINEAR); 
	n++;
	}
      if (n>0) 
	XtSetValues(yaxis, args, n);
    }

  if (pp->plot_data2 && y2axis) {
      n=0;
      if (Changed(y2axis_max)) {
	pp->y2axis_max_dft = False;
        XtSetArg(args[n], XtNmax, PlotFloatToArgVal(pp->y2axis_max)); n++;
	XtSetArg(args[n], XtNplotMaxUseDefault, False); n++;
	}
      else if (Changed(y2max)) {
	pp->y2max_dft = False;
	XtSetArg(args[n], XtNmax, PlotFloatToArgVal(pp->y2max)); n++;
        XtSetArg(args[n], XtNplotMaxUseDefault, False); n++;
        }  
      else if (Changed(y2axis_max_dft)) {
	XtSetArg(args[n], XtNplotMaxUseDefault, pp->y2axis_max_dft); n++;
	}
      else if (Changed(y2max_dft)) {
	XtSetArg(args[n], XtNplotMaxUseDefault, pp->y2max_dft); n++;
	}
      if (Changed(y2axis_min)) {
	pp->y2axis_min_dft = False;
        XtSetArg(args[n], XtNmin, PlotFloatToArgVal(pp->y2axis_min)); n++;
	XtSetArg(args[n], XtNplotMinUseDefault, False); n++;
	}
      else if (Changed(y2min)) {
	pp->y2min_dft = False;
	XtSetArg(args[n], XtNmin, PlotFloatToArgVal(pp->y2min)); n++;
        XtSetArg(args[n], XtNplotMinUseDefault, False); n++;
        }  
      else if (Changed(y2axis_min_dft)) {
	XtSetArg(args[n], XtNplotMinUseDefault, pp->y2axis_min_dft); n++;
	}
      else if (Changed(y2min_dft)) {
	XtSetArg(args[n], XtNplotMinUseDefault, pp->y2min_dft); n++;
	}
      if (Changed(y2num)) {
	XtSetArg(args[n], XtNticInterval, PlotFloatToArgVal(pp->y2num)); n++;
	XtSetArg(args[n], XtNplotNumUseDefault, False); n++;
        pp->y2num_dft = False;
      } else {
	if (Changed(y2num_dft) && pp->y2num_dft) {
          XtSetArg(args[n], XtNplotNumUseDefault, True); n++;
	  pp->y2num = dflt_huge;
	  }
        }
      if (Changed(y2tic)) {
	XtSetArg(args[n], XtNsubticInterval, PlotFloatToArgVal(pp->y2tic)); n++;
	XtSetArg(args[n], XtNplotTickUseDefault, False); n++;
        pp->y2tic_dft = False;
      } else {
	if (Changed(y2tic_dft) && pp->y2tic_dft) {
          XtSetArg(args[n], XtNplotTickUseDefault, True); n++;
	  pp->y2tic = dflt_huge;
	  }
        }
      if (Changed(axis_font)) {
	XtSetArg(args[n], XtNplotAxisFont, pp->axis_font); n++;
	}
      if (Changed(draw_frame)) {
        XtSetArg(args[n], XtNdrawFrame, True); n++;
        }
      if (Changed(y2title)) {
	XtSetArg(args[n], XtNlabel, pp->y2title); n++;
	}
      if (Changed(y2log)) {
        if (pp->y2log) 
          XtSetArg(args[n], XtNaxisTransform, AtTransformLOGARITHMIC); 
        else XtSetArg(args[n], XtNaxisTransform, AtTransformLINEAR); 
	n++;
	}
      if (n>0) 
	XtSetValues(y2axis, args, n);
    }


  if (Changed(rank_children))
	  redraw = True;

  if (Changed(title)) {
	  FreeTitle(current);
	  pp->title_strnum = copyStrings(pp->title, pp->title);
	  GetTitle(new);
	  if (pp->show_title) {
	       layout = True;
	       redraw = True;
	  }
     }

  if (Changed(set_labels)) {
	  FreeLegendTitle(new);
	  freeStrings(pp->set_labels, pp->legend_num);
	  pp->legend_num = copyStrings(pp->set_labels, pp->set_labels);
	  GetLegendTitle(new);
	  redraw = True;
	  recalc_legend = True;
     }

  /*if (ifPlotDataChanged(current->plotter.plot_data, pp->plot_data)) {
      destroyDataWidget(pp);
      PlotterCreatePlot((Widget)new, pp->plot_data, 1); 
      rescale = True;
      redraw = True;
      }
  else if (((pp->plot_data->a.type & PLOT_DATA_CHANGED) == PLOT_DATA_CHANGED)
      || ((pp->plot_data->a.type & PLOT_DATA_NEW) == PLOT_DATA_NEW)) {
      plotDataChanged(new, pp->plot_data, 1);
      redraw = True;
      }*/

  /*if (ifPlotDataChanged(current->plotter.plot_data, pp->plot_data)) {
      plotDataChanged(new, pp->plot_data, 1);
      redraw = True;
      }*/
  if (plotIfDataChanged(new, pp->plot_data, current->plotter.plot_data, 1))
      redraw = True;

  /*if (Changed(plot_data2)) {
      plotDataChanged(new, pp->plot_data2, 2);
      rescale = True;
      redraw = True;
      }*/
  /*if (ifPlotDataChanged(current->plotter.plot_data2, pp->plot_data2)) {
      plotDataChanged(new, pp->plot_data2, 2);
      redraw = True;
      }*/
  if (plotIfDataChanged(new, pp->plot_data2, current->plotter.plot_data2, 2))
      redraw = True;

  if (Changed(marker_style) && pp->marker_style) {
      if (pp->marker_gc) XtReleaseGC((Widget) new, pp->marker_gc);
      if (pp->marker_style->width<=0) pp->marker_style->width = 1;
      if (pp->marker_style->width>10) pp->marker_style->width = 10;
      if (pp->marker_style->color) {
        gcv.foreground = WnColorF((Widget)new, pp->marker_style->color);
	gcv.line_width = pp->marker_style->width;
        pp->marker_gc = XtGetGC((Widget)new, GCForeground | GCLineWidth, &gcv);
        }
      }
  if ((Changed(marker_style) && pp->marker_style || Changed(xmarker))
	&& XtIsRealized((Widget)new)) {  
      oldx = current->plotter.xmarker;
      x =  AtScaleUserToPixel(xpw->axiscore.scale, oldx);
      if (current->plotter.marker_style) 
	XCopyArea(XtDisplay((Widget)new), current->plotter.pixmap, 
	  XtWindow((Widget)new), pp->axis_gc, x-2, pp->layout.y1, 
	  current->plotter.marker_style->width+4, 
	  pp->layout.height, x-2, pp->layout.y1);
      else XCopyArea(XtDisplay((Widget)new), current->plotter.pixmap,
	  XtWindow((Widget)new), pp->axis_gc, x-2, pp->layout.y1,
	  5, pp->layout.height, x-2, pp->layout.y1);
      }

     
     /* Have looked at all the appropriate fields, so do the work! */
  if (recalc_legend) {
	  if (RecalcLegend(new))
	       layout = True;
	  else
	       redraw = True;
     }
  if (rescale)
	  RescaleRequired(new);
  if (layout)
	  LayoutRequired(new);
  if (redraw)
	  RedrawRequired(new);

  if (Changed(marker_style) || Changed(xmarker) )
       if (!rescale && !layout && !redraw) {
         newx = pp->xmarker;
         x =  AtScaleUserToPixel(xpw->axiscore.scale, newx);
         if (x<=pp->layout.x1) {
	   x = pp->layout.x1+2;
	   pp->xmarker = AtScalePixelToUser(xpw->axiscore.scale, x);
	   }
         if (x>=pp->layout.x2) {
	   x = pp->layout.x2-2;
	   pp->xmarker = AtScalePixelToUser(xpw->axiscore.scale, x);
	   }
         if (pp->marker_gc)
           XDrawLine(XtDisplay(new), XtWindow(new), pp->marker_gc, 
	     x, pp->layout.y1, x, pp->layout.y2);
         else XDrawLine(XtDisplay(new), XtWindow(new), pp->axis_gc,
	     x, pp->layout.y1, x, pp->layout.y2);
         }

  return False;

#undef Changed
}  /*end of SetValues */

/*
 *   Query geometry
 */

static XtGeometryResult QueryGeometry(pw, req, rep)
AtPlotterWidget pw;
XtWidgetGeometry *req, *rep;
{
     rep->request_mode = CWWidth | CWHeight;
     rep->width = pw->core.width < DEFAULT_WIDTH
		  ? DEFAULT_WIDTH : pw->core.width;
     rep->height = pw->core.height < DEFAULT_HEIGHT
		   ? DEFAULT_HEIGHT : pw->core.height;
     if ( ((req->request_mode & (CWWidth | CWHeight)) == (CWWidth | CWHeight))
	  && rep->width == req->width && rep->height == req->height)
	  return XtGeometryYes;
     else if (rep->width == pw->core.width && rep->height == pw->core.height)
	  return XtGeometryNo;
     else
	  return XtGeometryAlmost;
}

/*
 *   Insert/delete child
 */

static void InsertChild(w)
Widget w;
{
     AtPlotterWidget p = ParentPlotter(w);


     /* Warn if child of wrong class */
     if (!XtIsSubclass(w, atPlotWidgetClass))
	  XtAppWarning(XtWidgetToApplicationContext(w),
		       "Attempt to add child that is not a subclass of AtPlot");

     /* Call the superclass's insert_child proc to actually add the child */
     (*superclass->composite_class.insert_child)(w);

     /*
      * Maintain this list even if no rank order is requested, so if
      * rank order is wanted later the list is ready.
      */
     RankOrderChildren(w);
     if (XtIsRealized((Widget) p)) {
	  if (RecalcLegend(p))
	       LayoutRequired(p);
	  if (NewRawBoundingBox(p))
	       RescaleRequired(p);
	  else
	       RedrawRequired(p);
     }
}

static void DeleteChild(w)
Widget w;
{
     AtPlotterWidget p = ParentPlotter(w);

     int i;

     if (p->plotter.selected_item != NO_ITEM) {
	  for (i = 0; i < NUMCHILDREN(p); i++)
	       if (w == (Widget) CHILD(p, i))
		    break;
	  if (i == p->plotter.selected_item)
	       p->plotter.selected_item = NO_ITEM;
	  else if (i < p->plotter.selected_item)
	       p->plotter.selected_item--;
     }

     /* Call the superclass's delete_child proc to actually delete the child */
     (*superclass->composite_class.delete_child)(w);
     RankOrderRemove(w);
     if (XtIsRealized((Widget) p)) {
	  if (RecalcLegend(p))
	       LayoutRequired(p);
	  if (NewRawBoundingBox(p))
	       RescaleRequired(p);
	  else
	       RedrawRequired(p);
     }
}

/*
 *   Constraint initialize/destroy/set values
 */

static void ConstraintInitialize(request, new)
Widget request, new;
{
     AtPlotterConstraints c = CONSTRAINTS((AtPlotWidget)new);
     AtPlotterWidget p = ParentPlotter(new);

     GetLegendText(c, p);
     c->plotter.bounding_box = default_bounding_box;
     c->plotter.extended_list = NULL;
     c->plotter.needs_refresh = False;
}

static void ConstraintDestroy(w)
Widget w;
{
     AtPlotterConstraints c = CONSTRAINTS((AtPlotWidget)w);
     FreeLegendText(c, c);
     DestroyExtendedList((AtPlotWidget)w);
}

static Boolean ConstraintSetValues(current, request, new)
Widget current, request, new;
{
#define Changed(field) (newc->plotter.field != curc->plotter.field)
     AtPlotterConstraints newc = (AtPlotterConstraints)new->core.constraints;
     AtPlotterConstraints curc =(AtPlotterConstraints)current->core.constraints;
     AtPlotterWidget parent = ParentPlotter(new);
     Boolean redraw = False;
     Boolean rescale = False;
     Boolean layout = False;


     if (Changed(displayed) || Changed(use_y2_axis) || Changed(use_x2_axis)) {
	  redraw = True;
	  rescale = NewRawBoundingBox(parent);
	  if (Changed(displayed) && XtIsSubclass(new, atAxisCoreWidgetClass))
	       layout = rescale = True;
     }

  /*   if (Changed(legend_name)) {
	  FreeLegendText(curc, newc);
	  GetLegendText(newc, parent);
	  layout = RecalcLegend(parent);
     }*/

     if (Changed(rank_order))  {
	  ReRankOrderChildren(new);
	  if (parent->plotter.rank_children)
	       redraw = True;
     }

     if (layout)
	  LayoutRequired(parent);
     if (rescale)
	  RescaleRequired(parent);
     if (redraw)
	  RedrawRequired(parent);

     return False;

#undef Changed
}

/*
 *   The routines for handling the legend
 *
 *   Recalculate the layout of the legend,
 *   return True if the width or the height changed.
 */

static Boolean RecalcLegend(pw)
AtPlotterWidget pw;
{
     AtPlotterPart *pp = &pw->plotter;
     AtPlotterConstraints c;
     int h = 0, w = 0, i;
     Boolean changed;

     if (pp->show_legend == False)
	  return False;

     /*h = AtTextHeight(pp->legend_title_text) + pp->margin_height;*/
     h = pp->margin_height;
     for(i = 0; i < NUMCHILDREN(pw); i++) {
	  if ( !NTHCHILDISDISPLAYED(pw, i))
	       continue;
	  c = CONSTRAINT(pw, i);
	  if (c->plotter.legend_text != NULL) {
	       h += AtTextHeight(c->plotter.legend_text) + pp->legend_spacing;
	       w = Max(w, AtTextWidth(c->plotter.legend_text));
	  }
     }
     if (pp->legend_width > 0)
	  w = pp->legend_width;
     else {
	  w += ICON_WIDTH + 2 * pp->margin_width;
	/*  w = Max(w, AtTextWidth(pp->legend_title_text));*/
     }
     changed = (w != pp->layout.legend_width) || (h != pp->layout.legend_height);
     pp->layout.legend_height = h;
     pp->layout.legend_width = w;

     return changed;
}

static void RedrawLegend(AtPlotterWidget w, Region region, Boolean copy)
{
     AtPlotterPart *pp = &w->plotter;
     AtPlotterLayout *lp = &w->plotter.layout;
     AtText *t;
     Window win;
     Drawable drw;
     GC gc;
     int y, i;

     if (pp->show_legend == False)
	  return;

     win = XtWindow(w);
     drw = pp->use_pixmap ? pp->pixmap : win;

     if (region) {
	  XSetRegion(XtDisplay(w), pp->legend_gc, region);
	  XSetRegion(XtDisplay(w), pp->select_gc, region);
     }

     XFillRectangle(XtDisplay(w), drw, pp->select_gc,
		   lp->legend_x - 1, lp->legend_y - 1,
		   lp->legend_width + 3, lp->legend_height + 3);

     y = lp->legend_y;
   /*  AtTextDrawJustified(XtDisplay(w), win, drw, pp->legend_gc,
			 pp->legend_title_text,
			 AtTextJUSTIFY_CENTER, AtTextJUSTIFY_CENTER,
			 lp->legend_x, y,
			 lp->legend_width,
			 AtTextHeight(pp->legend_title_text));*/

    /* y += AtTextHeight(pp->legend_title_text) + pp->margin_height;*/
     y += pp->margin_height;
     for(i = 0; i < NUMCHILDREN(w); i++ ) {
	  if ( !NTHCHILDISDISPLAYED(w, i))
	       continue;
	  if (t = CONSTRAINT(w, i)->plotter.legend_text) {
	       if (i == pp->selected_item) {
		    XFillRectangle(XtDisplay(w), drw, pp->legend_gc,
				   lp->legend_x+2*pp->margin_width+ICON_WIDTH-1,
				   y,
				   lp->legend_width-2*pp->margin_width-ICON_WIDTH+2,
				   AtTextHeight(t));
		    gc = pp->select_gc;
		    pp->selected_item_y = y;
	       }
	       else
		    gc = pp->legend_gc;
	       At2TextDraw(XtDisplay(w), win, drw, gc, t,
			  lp->legend_x+2*pp->margin_width+ICON_WIDTH,
			  y + (AtTextAscent(t)>0) ? AtTextAscent(t)-1 : 0);
	       AtPlotDrawIcon(CHILD(w, i), XtDisplay(w), drw,
			      lp->legend_x, y, ICON_WIDTH,
			      AtTextHeight(t), region);
	       y += AtTextHeight(t) + pp->legend_spacing;
	  }
     }

     if (copy)
	  XCopyArea(XtDisplay(w), drw, win, pp->pixmap_gc,
		    lp->legend_x-1, lp->legend_y-1,
		    lp->legend_width+3, lp->legend_height+2,
		    lp->legend_x-1, lp->legend_y-1);

     if (region) {
	  XSetClipMask(XtDisplay(w), pp->legend_gc, None);
	  XSetClipMask(XtDisplay(w), pp->select_gc, None);
     }
}

/*
 *   RankOrderChildren
 *   Sorts the children of Plotter widget on a list according to their
 *   Ranking rather than their birth order.  The lowest ranking child is
 *   drawn first, whereas, the highest ranking one is drawn last. The
 *   highest ranking plot is therfore always visible (never covered by
 *   its siblings, if they overlap).  This is useful, for example, if
 *   you have several sets of Barcharts that overlap and you want to
 *   control which set should be completely visible (in the foreground)
 *   at a given time, and in what order the others should cover each
 *   other.
 */

#define ORDLIST parent->plotter.ordered_children

static Rank* getnode P((void));
static Rank* getnode ()
{
     return ((Rank*) XtMalloc(sizeof(Rank)));
}

static void RankOrderChildren(w)
Widget w;
{
     AtPlotterWidget parent = ParentPlotter(w);
     AtPlotterConstraints pcons = (AtPlotterConstraints) w->core.constraints;
     Rank *locate, *newnode;
     Boolean found = False;

     if ( !XtIsSubclass(w, atPlotWidgetClass))    /* Don't rank axes */
	  return;

     if (ORDLIST == NULL) {
	  ORDLIST = getnode();
	  ORDLIST->prev = NULL;
	  ORDLIST->next = NULL;
	  ORDLIST->child = (AtPlotWidget)w;
	  ORDLIST->rank_order = pcons->plotter.rank_order;
	  /*
	   * Higher rank children go on top of lower rank ones.
	   * rankOrder is a constraint resource of Plotter
	   */
	  return;
     }
     for (locate = ORDLIST; locate != NULL; locate = locate->next) {
	  if (pcons->plotter.rank_order < locate->rank_order) {
	       /* Should be inserted right before locate */
	       newnode = getnode();          /* Get a new node */
	       newnode->child = (AtPlotWidget)w;
	       newnode->rank_order = pcons->plotter.rank_order;
	       newnode->prev = locate->prev;
	       newnode->next = locate;
	       locate->prev = newnode;
	       if (newnode->prev == NULL)    /* First on the list */
		    ORDLIST = newnode;
	       else
		    (newnode->prev)->next = newnode;
	       found = True;
	       break;
	  }
     }
     if ( !found) {      /* Highest order so far, insert at end of list */
	  for (locate=ORDLIST; locate->next != NULL; locate=locate->next)
	       ;
	  newnode = getnode();               /* Get a new node */
	  newnode->child = (AtPlotWidget)w;
	  newnode->rank_order = pcons->plotter.rank_order;
	  newnode->prev = locate;
	  newnode->next = locate->next;
	  locate->next = newnode;
     }
}

/*
 *   ReRankOrderChildren
 *   Remove the child whose ranking changed form the ordered_children list.
 *   Then it will reinsert the removed child into the list
 *   according to its new rankOrder.
 */

static void ReRankOrderChildren(w)
Widget w;
{
     RankOrderRemove(w);
     RankOrderChildren(w);
}

static void RankOrderRemove(w)
Widget w;
{
     AtPlotterWidget parent = ParentPlotter(w);
     Rank *locate;

     if (!XtIsSubclass(w, atPlotWidgetClass))     /* Don't rank axes */
	  return;

     for (locate = ORDLIST; locate != NULL; locate = locate->next)  {
	  if (locate->child == (AtPlotWidget) w) {
	       if (locate->next)
		    (locate->next)->prev = locate->prev;
	       if (locate->prev == NULL)          /* Head of list */
		    ORDLIST = locate->next;
	       else
		    (locate->prev)->next = locate->next;
	       XtFree ((char *) locate);
	       break;
	  }
     }
}

#undef ORDLIST

/*
 *      Callback routines
 *
 *      A lot of ideas and algorithms for the motion and drag callback
 *      procs are taken from plotter v4 by Chris Craig a.o.
 */

#define UseCursors(pw)   (pw->plotter.use_cursors)
#define InSelect(pw)     (pw->plotter.in_select)
#define InClick(pw)      (pw->plotter.in_click)
#define InDrag(pw)       (pw->plotter.in_drag)
#define InSlide(pw)      (pw->plotter.in_slide)
#define CanSlide(pw)     (pw->plotter.can_slide)
#define InLegend(pw)     (pw->plotter.in_legend)

#define InPlottingRegion(pw, event) \
    ((event->x >= pw->plotter.layout.x1 - 2) &&\
     (event->x <= pw->plotter.layout.x2 + 2) &&\
     (event->y >= pw->plotter.layout.y1 - 2) &&\
     (event->y <= pw->plotter.layout.y2 + 2))

#define InLegendRegion(pw, event) \
    ((pw->plotter.show_legend) &&\
     (event->x >= pw->plotter.layout.legend_x) &&\
     (event->x <= pw->plotter.layout.legend_x + pw->plotter.layout.legend_width) &&\
     (event->y >= pw->plotter.layout.legend_y) &&\
     (event->y <= pw->plotter.layout.legend_y + pw->plotter.layout.legend_height))

#define AdjustXY(pw, event, x, y) \
     x = event->x;\
     y = event->y;\
     if      (x < pw->plotter.layout.x1) x = pw->plotter.layout.x1;\
     else if (x > pw->plotter.layout.x2) x = pw->plotter.layout.x2;\
     if      (y < pw->plotter.layout.y1) y = pw->plotter.layout.y1;\
     else if (y > pw->plotter.layout.y2) y = pw->plotter.layout.y2

#define InDragRange(pw) ((pw->plotter.drag_width*pw->plotter.drag_height) > 36)

#define SetPointer(pw, x, y) \
     XWarpPointer(XtDisplay(pw), None, XtWindow(pw), 0, 0, 0, 0, x, y)

#define ResetStates(pw) pw->plotter.in_select = pw->plotter.in_click = \
			pw->plotter.in_drag = pw->plotter.in_slide = \
			pw->plotter.in_legend =  False

/*#define X1Scale(pw, x) AtScalePixelToUser(pw->plotter.xaxis->axiscore.scale, x)
#define Y1Scale(pw, y) AtScalePixelToUser(pw->plotter.yaxis->axiscore.scale, y)
#define X2Scale(pw, x) AtScalePixelToUser(pw->plotter.x2axis->axiscore.scale, x)
#define Y2Scale(pw, y) AtScalePixelToUser(pw->plotter.y2axis->axiscore.scale, y)*/

#define DefCursor(pw, cursor) XDefineCursor(XtDisplay(pw), XtWindow(pw), cursor)
#define UndefCursor(pw) XDefineCursor(XtDisplay(pw), XtWindow(pw), pw->plotter.current_cursor)

#define CM_NONE     0x00
#define CM_TOP      0x10
#define CM_BOTTOM   0x20
#define CM_LEFT     0x01
#define CM_RIGHT    0x02
#define CM_TOPLEFT  0x11
#define CM_TOPRIGHT 0x12
#define CM_BOTLEFT  0x21
#define CM_BOTRIGHT 0x22
#define CM_SLIDE    0x30

/*
 *   Internal enter/leave callback procs
 */

/* Install enter/leave callback proc */
static void InstallHandlers(w)
AtPlotterWidget w;
{
     XtAddEventHandler((Widget) w, EnterWindowMask, False,
		       (XtEventHandler) EnterLeave, (XtPointer) True);
     XtAddEventHandler((Widget) w, LeaveWindowMask, False,
		       (XtEventHandler) EnterLeave, (XtPointer) False);
}

/* Remove enter/leave callback proc */
static void RemoveHandlers(w)
AtPlotterWidget w;
{
     XtRemoveEventHandler((Widget) w, EnterWindowMask, False,
			  (XtEventHandler) EnterLeave, (XtPointer) True);
     XtRemoveEventHandler((Widget) w, LeaveWindowMask, False,
			  (XtEventHandler) EnterLeave, (XtPointer) False);
}

/* Enter/leave callback: set the appropriate cursor */
static void EnterLeave(pw, enter, ev, cont)
AtPlotterWidget pw;
XtPointer enter;
XEvent *ev;
int cont;
{
     AtPlotterPart *pp = &pw->plotter;

     if (UseCursors(pw) && HasMotionCB(pw)) {
	  if (enter) {
	       pp->current_cursor = pp->motion_cursor;
	       DefCursor(pw, pp->motion_cursor);
	  }
	  else {
	       pp->current_cursor = pp->plotter_cursor;
	       UndefCursor(pw);
	  }
     }
}

/*
 *   Motion callback procs
 */

/* Set up motion callback data and call callback list procs */
static void SendMotionCallback P((AtPlotterWidget, int, int));
static void SendMotionCallback(pw, x, y)
AtPlotterWidget pw;
int x, y;
{
     AtPointCallbackData data;

     data.reason = AtPointMOTION;
     data.pixelx = x;
     data.pixely = y;
     data.x1 = X1Scale(pw, x);
     data.y1 = Y1Scale(pw, y);
     data.x2 = pw->plotter.x2axis ? X2Scale(pw, x) : 0.0;
     data.y2 = pw->plotter.y2axis ? Y2Scale(pw, y) : 0.0;
     XtCallCallbacks((Widget) pw, XtNmotionCallback, (XtPointer) &data);
}

/* Motion callback handler: check callback/range and send callback */
static void HandleMotion(pw, event)
AtPlotterWidget pw;
XMotionEvent *event;
{
     int x, y;

     AdjustXY(pw, event, x, y);
     if (HasMotionCB(pw) && InPlottingRegion(pw, event))
	  SendMotionCallback(pw, x, y);
}

/*
 *   Selection (drag and click) callback procs
 */

/* Calc and set drag positions/dimensions, i.e. after redisplay orresize */
static void SetDragPositions(pw)
AtPlotterWidget pw;
{
     AtPlotterPart *pp = &pw->plotter;
     AtPlotterLayout *lp = &pw->plotter.layout;
     int x, y;

     if (X1Scale(pw, lp->x1) > pp->slide_x1 || X1Scale(pw, lp->x2) < pp->slide_x2 ||
	 Y1Scale(pw, lp->y2) > pp->slide_y1 || Y1Scale(pw, lp->y1) < pp->slide_y2) {
	  pp->can_slide = False;
	  return;
     }
     pp->drag_x = AtScaleUserToPixel(pp->xaxis->axiscore.scale, pp->slide_x1);
     x = AtScaleUserToPixel(pp->xaxis->axiscore.scale, pp->slide_x2);
     pp->drag_y = AtScaleUserToPixel(pp->yaxis->axiscore.scale, pp->slide_y2);
     y = AtScaleUserToPixel(pp->yaxis->axiscore.scale, pp->slide_y1);
     pp->drag_width  = x - pp->drag_x;
     pp->drag_height = y - pp->drag_y;
}

/* Draw the drag rectangle, set the appropriate cursor */
static void DrawDragRect P((AtPlotterWidget));
static void DrawDragRect(pw)
AtPlotterWidget pw;
{
     AtPlotterPart *pp = &pw->plotter;
     Cursor cursor;
     int x, y;
     unsigned int w, h;
     short cm;

     cm = CM_NONE;
     if (pp->drag_width < 0) {
	  w = -pp->drag_width;
	  x = pp->drag_x - w;
	  cm |= CM_LEFT;
     }
     else {
	  w = pp->drag_width;
	  x = pp->drag_x;
	  cm |= CM_RIGHT;
     }
     if (pp->drag_height < 0) {
	  h = -pp->drag_height;
	  y = pp->drag_y - h;
	  cm |= CM_TOP;
     }
     else {
	  h = pp->drag_height;
	  y = pp->drag_y;
	  cm |= CM_BOTTOM;
     }
     if (InSlide(pw))
	  cm = CM_SLIDE;

     XDrawRectangle(XtDisplay(pw), XtWindow(pw), pp->drag_gc, x, y, w, h);

     if (UseCursors(pw)) {
	  if(cm != pp->cursor_mask) {
	       switch(cm) {
		    case CM_TOPLEFT:
			 cursor = pp->top_left_cursor;
			 break;
		    case CM_TOPRIGHT:
			 cursor = pp->top_right_cursor;
			 break;
		    case CM_BOTLEFT:
			 cursor = pp->bottom_left_cursor;
			 break;
		    case CM_BOTRIGHT:
			 cursor = pp->bottom_right_cursor;
			 break;
		    case CM_SLIDE:
			 cursor = pp->slide_cursor;
			 break;
	       }
	       DefCursor(pw, cursor);
	       pp->cursor_mask = cm;
	  }
     }
}

/* Erase the drag rectangle */
#define EraseDragRect(pw) DrawDragRect(pw)

/* Fill up click callback data and call callback proc list */
static void SendClickCallback P((AtPlotterWidget, int, int));
static void SendClickCallback(pw, x, y)
AtPlotterWidget pw;
int x, y;
{
     AtPointCallbackData data;

     data.reason = AtPointCLICK;
     data.pixelx = x;
     data.pixely = y;
     data.x1 = X1Scale(pw, x);
     data.y1 = Y1Scale(pw, y);
     data.x2 = pw->plotter.x2axis ? X2Scale(pw, x) : 0.0;
     data.y2 = pw->plotter.y2axis ? Y2Scale(pw, y) : 0.0;
     XtCallCallbacks((Widget) pw, XtNclickCallback, (XtPointer) &data);
}

/* Draw the legend rectangle */
static void DrawLegendRect P((AtPlotterWidget));
static void DrawLegendRect(pw)
AtPlotterWidget pw;
{
     AtPlotterPart *pp = &pw->plotter;
     AtText *t;

     if(t = CONSTRAINT(pw, pp->legend_item)->plotter.legend_text) {
	  XDrawRectangle(XtDisplay(pw), XtWindow(pw), pp->drag_gc,
			 pp->layout.legend_x - 1,
			 pp->legend_item_y - 1,
			 pp->layout.legend_width + 2,
			 AtTextHeight(t) + 1);
     }
}



/* Erase the legend rectangle */
#define EraseLegendRect(pw) DrawLegendRect(pw)

/* Select legend item */
static void SelectInLegend P((AtPlotterWidget, XButtonPressedEvent *));
static void SelectInLegend(pw, event)
AtPlotterWidget pw;
XButtonPressedEvent *event;
{
     AtPlotterPart *pp = &pw->plotter;
     AtText *t;
     int i, y, y1, y2;

     y  = pp->layout.legend_y + /*AtTextHeight(pp->legend_title_text)*/ +
	  pp->margin_height;
     y1 = y - pp->legend_spacing / 2;

     /* If in title region: deselect item */
     if (event->y < y1) {
	  pp->legend_item = NO_ITEM;
	  return;
     }

     /* Search for item to select */
     for (i = 0; i < NUMCHILDREN(pw); i++) {
	  if ( !NTHCHILDISDISPLAYED(pw, i))
	       continue;
	  if(t = CONSTRAINT(pw, i)->plotter.legend_text) {
	       y2 = y1 + AtTextHeight(t) + pp->legend_spacing;
	       if (event->y >= y1 && event->y <= y2)
		    break;
	       y1 += AtTextHeight(t) + pp->legend_spacing;
	       y  += AtTextHeight(t) + pp->legend_spacing;
	  }
     }

     /* Select item if found */
     if (i < NUMCHILDREN(pw)) {
	  pp->legend_item = i;
	  pp->legend_item_y = y;
	  DrawLegendRect(pw);
     }
}

/* Start selection for drag and click callback */
static void StartSelection(pw, event)
AtPlotterWidget pw;
XButtonPressedEvent *event;
{
     AtPlotterPart *pp = &pw->plotter;
     int x, y;

     ResetStates(pw);

     if (InPlottingRegion(pw, event)) {
	  if (HasClickCB(pw)) {
	       pp->in_click = True;
	       if (UseCursors(pw)) {
		    DefCursor(pw, pp->click_cursor);
	       }
	  }
	  if (HasDragCB(pw)) {
	       AdjustXY(pw, event, x, y);
	       pp->drag_x = x;
	       pp->drag_y = y;
	       pp->in_select = True;
	       pp->in_drag = False;
	       if (UseCursors(pw)) {
		    DefCursor(pw, pp->drag_cursor);
	       }
	  }
	  if (HasMotionCB(pw)) {
	       AdjustXY(pw, event, x, y);
	       SendMotionCallback(pw, x, y);
	  }
     }
     else if (HasSelectCB(pw) && InLegendRegion(pw, event)) {
	  if (UseCursors(pw)) {
	       DefCursor(pw, pp->select_cursor);
	  }
	  pp->in_legend = True;
	  pp->legend_item = pp->selected_item;
	  pp->legend_item_y = pp->selected_item_y;
	  SelectInLegend(pw, event);
     }
}

/* Drag handler */
static void Drag(pw, event)
AtPlotterWidget pw;
XMotionEvent *event;
{
     AtPlotterPart *pp = &pw->plotter;
     int x, y;

     if (InSelect(pw)) {
	  if (InDrag(pw))
	       EraseDragRect(pw);
	  AdjustXY(pw, event, x, y);
	  if ( !InPlottingRegion(pw, event))
	       SetPointer(pw, x, y);
	  pp->drag_width  = x - pp->drag_x;
	  pp->drag_height = y - pp->drag_y;
	  DrawDragRect(pw);
	  pp->in_drag = True;
     }
     else if (InLegend(pw)) {
	  if (pp->legend_item != NO_ITEM)
	       EraseLegendRect(pw);
	  if (InLegendRegion(pw, event))
	       SelectInLegend(pw, (XButtonPressedEvent *) event);
	  else {
	       if (UseCursors(pw)) {
		    UndefCursor(pw);
		    pp->cursor_mask = CM_NONE;
	       }
	       RedrawLegend(pw, NULL, pp->use_pixmap);
	       pp->legend_item = NO_ITEM;
	       pp->in_legend = False;
	  }
     }

     if (HasMotionCB(pw) && InPlottingRegion(pw, event)) {
	  AdjustXY(pw, event, x, y);
	  SendMotionCallback(pw, x, y);
     }
}

/* End selection: check and call drag/click callback list procs */
static void EndSelection(pw, event)
AtPlotterWidget pw;
XButtonReleasedEvent *event;
{
     AtPlotterPart *pp = &pw->plotter;
     AtRectangleCallbackData recd;
     AtSelectCallbackData seld;
     int x, y;

     if (InSelect(pw)) {
	  if (InDrag(pw))
	       EraseDragRect(pw);
	  if (UseCursors(pw)) {
	       UndefCursor(pw);
	       pp->cursor_mask = CM_NONE;
	  }
	  if (InDrag(pw) && InDragRange(pw)) {
	       AdjustXY(pw, event, x, y);
	       if ( !InPlottingRegion(pw, event))
		    SetPointer(pw, x, y);
		recd.reason  = AtRectangleDRAG;
		recd.pixelx1 = Min(pp->drag_x, x);
		recd.pixelx2 = Max(pp->drag_x, x);
		recd.pixely1 = Max(pp->drag_y, y);
		recd.pixely2 = Min(pp->drag_y, y);
		recd.x11 = X1Scale(pw, recd.pixelx1);
		recd.x12 = X1Scale(pw, recd.pixelx2);
		recd.y11 = Y1Scale(pw, recd.pixely1);
		recd.y12 = Y1Scale(pw, recd.pixely2);
		if (pw->plotter.x2axis) {
		    recd.x21 = X2Scale(pw, recd.pixelx1);
		    recd.x22 = X2Scale(pw, recd.pixelx2);
		}
		else
		    recd.x21 = recd.x22 = 0.0;
		if (pw->plotter.y2axis) {
		    recd.y21 = Y2Scale(pw, recd.pixely1);
		    recd.y22 = Y2Scale(pw, recd.pixely2);
		}
		else
		   recd.y21 = recd.y22 = 0.0;
		pw->plotter.slide_x1 = recd.x11;
		pw->plotter.slide_x2 = recd.x12;
		pw->plotter.slide_y1 = recd.y11;
		pw->plotter.slide_y2 = recd.y12;
		pw->plotter.can_slide = True;
		XtCallCallbacks((Widget) pw, XtNdragCallback, (XtPointer) &recd);
	  }
     }
     else if (InLegend(pw)) {
	  if (UseCursors(pw)) {
	       UndefCursor(pw);
	       pp->cursor_mask = CM_NONE;
	  }
	  if (InLegendRegion(pw, event)) {
	       pp->selected_item = pp->legend_item;
	       pp->selected_item_y = pp->legend_item_y;
	       if (pp->selected_item != NO_ITEM) {
		    seld.reason = AtSelectSELECTED;
		    seld.widget = (Widget) CHILD(pw, pp->selected_item);
	       }
	       else {
		    seld.reason = AtSelectDESELECTED;
		    seld.widget = NULL;
	       }
	       XtCallCallbacks((Widget) pw, XtNselectCallback, (XtPointer) &seld);
	  }
	  RedrawLegend(pw, NULL, pp->use_pixmap);
	  pp->legend_item = NO_ITEM;
     }

     if (InClick(pw)) {
	  if (UseCursors(pw)) {
	       UndefCursor(pw);
	       pp->cursor_mask = CM_NONE;
	  }
	  if (InPlottingRegion(pw, event)) {
	       AdjustXY(pw, event, x, y);
	       SendClickCallback(pw, x, y);
	  }
     }

     ResetStates(pw);

     if (HasMotionCB(pw) && InPlottingRegion(pw, event)) {
	  AdjustXY(pw, event, x, y);
	  SendMotionCallback(pw, x, y);
     }
}

/*
 *   Slide callback procs
 */

/* Setup callback data and call slide callback list procs */
static void SendSlideCallback P((AtPlotterWidget, int, int));
static void SendSlideCallback(pw, x, y)
AtPlotterWidget pw;
int x, y;
{
     AtPlotterPart *pp = &pw->plotter;
     AtRectangleCallbackData data;

     data.reason  = AtRectangleSLIDE;
     data.pixelx1 = x;
     data.pixelx2 = x + pp->drag_width;
     data.pixely1 = y + pp->drag_height;
     data.pixely2 = y;
     data.x11 = X1Scale(pw, data.pixelx1);
     data.x12 = X1Scale(pw, data.pixelx2);
     data.y11 = Y1Scale(pw, data.pixely1);
     data.y12 = Y1Scale(pw, data.pixely2);
     if (pw->plotter.x2axis) {
	  data.x21 = X2Scale(pw, data.pixelx1);
	  data.x22 = X2Scale(pw, data.pixelx2);
     }
     else
	  data.x21 = data.x22 = 0.0;
     if (pw->plotter.y2axis) {
	  data.y21 = Y2Scale(pw, data.pixely1);
	  data.y22 = Y2Scale(pw, data.pixely2);
     }
     else
	  data.y21 = data.y22 = 0.0;
     pw->plotter.slide_x1 = data.x11;
     pw->plotter.slide_x2 = data.x12;
     pw->plotter.slide_y1 = data.y11;
     pw->plotter.slide_y2 = data.y12;
     XtCallCallbacks((Widget) pw, XtNslideCallback, (XtPointer) &data);
}

/* Start sliding */
static void StartSliding(pw, event)
AtPlotterWidget pw;
XButtonPressedEvent *event;
{
     AtPlotterPart *pp = &pw->plotter;
     int x, y;

     ResetStates(pw);

     if (CanSlide(pw) && HasSlideCB(pw) && InPlottingRegion(pw, event)) {
	  SetPointer(pw, pp->drag_x, pp->drag_y);
	  pp->in_slide = True;
	  DrawDragRect(pw);
     }

     if (HasMotionCB(pw) && InPlottingRegion(pw, event)) {
	  AdjustXY(pw, event, x, y);
	  SendMotionCallback(pw, x, y);
     }
}

/* Slide handler: check, setup callback data and call slide callback procs */
static void Slide(pw, event)
AtPlotterWidget pw;
XMotionEvent *event;
{
     AtPlotterPart *pp = &pw->plotter;
     int x, y;
     Boolean set = False;

     if (InSlide(pw)) {
	  EraseDragRect(pw);
	  AdjustXY(pw, event, x, y);
	  if ((x + pp->drag_width) > pp->layout.x2) {
	       pp->drag_x = x = pp->layout.x2 - pp->drag_width;
	       set = True;
	  }
	  else
	       pp->drag_x = x;
	  if ((y + pp->drag_height) > pp->layout.y2) {
	       pp->drag_y = y = pp->layout.y2 - pp->drag_height;
	       set = True;
	  }
	  else
	       pp->drag_y = y;
	  if (set || !InPlottingRegion(pw, event))
	       SetPointer(pw, x, y);
	  pp->in_slide = True;
	  DrawDragRect(pw);
	  SendSlideCallback(pw, x, y);
     }

     if (HasMotionCB(pw) && InPlottingRegion(pw, event)) {
	  AdjustXY(pw, event, x, y);
	  SendMotionCallback(pw, x, y);
     }
}

/* End sliding: check and call slide callback list procs */
static void EndSliding(pw, event)
AtPlotterWidget pw;
XButtonReleasedEvent *event;
{
     AtPlotterPart *pp = &pw->plotter;
     int x, y;

     if (InSlide(pw)) {
	  EraseDragRect(pw);
	  if (UseCursors(pw)) {
	       UndefCursor(pw);
	       pp->cursor_mask = CM_NONE;
	  }
	  AdjustXY(pw, event, x, y);
	  if ( !InPlottingRegion(pw, event))
	       SetPointer(pw, x, y);
	  if ((x + pp->drag_width) > pp->layout.x2)
	       pp->drag_x = pp->layout.x2 - pp->drag_width;
	  else
	       pp->drag_x = x;
	  if ((y + pp->drag_height) > pp->layout.y2)
	       pp->drag_y = pp->layout.y2 - pp->drag_height;
	  else
	       pp->drag_y = y;
	  pp->can_slide = True;
	  SendSlideCallback(pw, x, y);
     }

     ResetStates(pw);

     if (HasMotionCB(pw) && InPlottingRegion(pw, event)) {
	  AdjustXY(pw, event, x, y);
	  SendMotionCallback(pw, x, y);
     }
}

/* Cancel all actions */
static void Cancel(pw, event)
AtPlotterWidget pw;
XMotionEvent *event;
{
     AtPlotterPart *pp = &pw->plotter;
     int x, y;

     if (InSelect(pw) || InDrag(pw) || InSlide(pw)) {
	  EraseDragRect(pw);
	  if (UseCursors(pw)) {
	       UndefCursor(pw);
	       pw->plotter.cursor_mask = CM_NONE;
	  }
     }
     if (InLegend(pw)) {
	  if (UseCursors(pw)) {
	       UndefCursor(pw);
	       pw->plotter.cursor_mask = CM_NONE;
	  }
	  RedrawLegend(pw, NULL, pp->use_pixmap);
     }

     ResetStates(pw);

     if (HasMotionCB(pw) && InPlottingRegion(pw, event)) {
	  AdjustXY(pw, event, x, y);
	  SendMotionCallback(pw, x, y);
     }
}

static int Move(pw, event)
AtPlotterWidget pw;
XMotionEvent *event;
{/*
  AtPlotterPart *pp = &pw->plotter;
  AtPlotterLayout *lp = &pw->plotter.layout;
  static int x0, y0, x1, y1;
  static GC localgc;
  XGCValues gcv;
  Display *dis = XtDisplay((Widget)pw);
  Drawable win = XtWindow((Widget)pw);

  pp->motion_w = event->x - pp->motion_x0;
  pp->motion_h = event->y - pp->motion_y0;

  if (localgc == None) {
    localgc = XCreateGC(dis, win,0L,NULL);
    XCopyGC(dis, pp->plotarea_gc, GCForeground |GCBackground, localgc);
    XSetFunction(dis, localgc, GXxor);
    }

  if (pp->motion_w > 0) {   
    XCopyArea(dis, win, win, localgc, lp->x1, lp->y1, pp->motion_w,
      pp->motion_h, lp->x1, lp->y1);
    XCopyArea(dis, pp->pixmap, win, pp->plotarea_gc, lp->x1, lp->y1, 
      lp->width - pp->motion_w, lp->height - pp->motion_h, 
      lp->x1+pp->motion_w, lp->y1+pp->motion_h);
    }
  pp->motion_x0 = event->x;
  pp->motion_y0 = event->y;
  */
}

static int MoveNotify(pw, event)
AtPlotterWidget pw;
XButtonPressedEvent *event;
{
  /*AtPlotterPart *pp = &pw->plotter;
  int x, y;
  AtPlotterLayout *lp = &pw->plotter.layout;
  static GC localgc;
  XGCValues gcv;
  Display *dis = XtDisplay((Widget)pw);
  Drawable win = XtWindow((Widget)pw);

    
  pp->motion_x0 = event->x;
  pp->motion_y0 = event->y;
  if (localgc == None) {
    localgc = XCreateGC(dis, win,0L,NULL);
    XCopyGC(dis, pp->plotarea_gc, GCForeground |GCBackground, localgc);
    XSetFunction(dis, localgc, GXxor);
    }
  if (!pp->pixmap) {
    return(0);
    }
  XFillRectangle(dis, win, pp->graph_gc, pp->shadow_thickness,
    pp->shadow_thickness,20,
    pp->pixmap_height - 2 * pp->shadow_thickness);
  XCopyArea(dis, pp->pixmap, win, pp->plotarea_gc, pp->shadow_thickness,
    pp->shadow_thickness,
      pp->pixmap_width - 20 - pp->shadow_thickness*2, 
      pp->pixmap_height - pp->shadow_thickness*2, 
      pp->shadow_thickness + 20, pp->shadow_thickness);
  XCopyArea(dis, win, pp->pixmap, pp->plotarea_gc, 0, 0, pp->pixmap_width,
     pp->pixmap_height, 0, 0);
     */
}

