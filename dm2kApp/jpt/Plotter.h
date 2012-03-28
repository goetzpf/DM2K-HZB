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
 *      Plotter.h
 *      Based on the AthenaTools Plotter Widget Set - Version 6.0
 *      The AthenaTools Plotter Widget Set - Version 6.0
 *      klin, Tue Jul  7 13:59:47 1992
 *      klin, Sun Jul 19 19:23:41 1992, patchlevel 1
 *                                      AtPlotterGetLegendWidth() added.
 *      klin, Mon Jul 27 14:17:31 1992, patchlevel 2
 *                                      Resources XtNlegendLeft and
 *                                      XtNautoRedisplay added.
 *                                      Resource XtNusePixmap and
 *                                      drawing to a pixmap added.
 *                                      Resource XtNuseCursor and
 *                                      callback cursors added.
 *                                      Resource XtNbusyCallback and
 *                                      busy callback added.
 *      klin, Sun Aug  2 18:24:12 1992, patchlevel 3
 *                                      Layout callback and some stuff for
 *                                      aligning axis positions added.
 *                                      Resource XtNtitleHeigth and
 *                                      AtPlotterGetTitleHeight() added.
 *                                      Resources XtNxxxCursor added.
 *      klin, Sat Aug 15 10:02:01 1992, patchlevel 4
 *                                      Resources XtNslideCallback and
 *                                      XtNslideCursor and needed stuff added.
 *                                      Resources XtNselectCallback and
 *                                      XtNselectCursor and needed stuff added.
 *                                      Minor changes in callbacks.
 *                                      Changed <At/..> to <X11/At/..>.
 *
 *      SCCSid[] = "@(#) Plotter V6.0  92/08/15  Plotter.h"

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

#ifndef _Jpt_AtPlotter
#define _Jpt_AtPlotter

#ifdef __cplusplus
extern "C" {
#endif

#include "At.h"
#include "Plot.h"

/*
 *   Resource names
 */

#ifndef XtNtitle
#define XtNtitle              "title"
#endif
#define XtNlegendTitle        "legendTitle"

#define XtNtitleSize          "titleSize"
#define XtNlegendTitleSize    "legendTitleSize"
#define XtNlegendSize         "legendSize"

#define XtNtitleStyle         "titleStyle"
#define XtNlegendTitleStyle   "legendTitleStyle"
#define XtNlegendStyle        "legendStyle"

#define XtNtitleColor         "titleColor"
#define XtNplotAreaColor      "plotAreaColor"

#define XtNshowTitle          "showTitle"
#define XtNshowLegend         "showLegend"
#define XtNtitleHeight        "titleHeight"
#define XtNlegendLeft         "legendLeft"
#define XtNlegendWidth        "legendWidth"
#define XtNlegendSpacing      "legendSpacing"

#define XtNmarginWidth        "marginWidth"
#define XtNmarginHeight       "marginHeight"

#define XtNrankChildren       "rankChildren"
#define XtNautoRedisplay      "autoRedisplay"
#define XtNuseCursors         "useCursors"
#define XtNusePixmap          "usePixmap"

#define XtNplotterCursor      "plotterCursor"
#define XtNbusyCursor         "busyCursor"
#define XtNmotionCursor       "motionCursor"
#define XtNclickCursor        "clickCursor"
#define XtNdragCursor         "dragCursor"
#define XtNslideCursor        "slideCursor"
#define XtNselectCursor       "selectCursor"

#define XtNlayoutCallback     "layoutCallback"
#define XtNbusyCallback       "busyCallback"
#define XtNmotionCallback     "motionCallback"
#define XtNclickCallback      "clickCallback"
#define XtNdragCallback       "dragCallback"
#define XtNslideCallback      "slideCallback"
#define XtNselectCallback     "selectCallback"

#define XtNxAxis              "xAxis"
#define XtNyAxis              "yAxis"
#define XtNx2Axis             "x2Axis"
#define XtNy2Axis             "y2Axis"

#define XtNplotAccurateTicLabel "plotAccurateTicLabel"
#define XtNplotData           "plotData"
#define XtNplotData2           "plotData2"
#define XtNplotDataStyles     "plotDataStyles"
#define XtNplotDataStyles2    "plotDataStyles2"
#define XtNplotDataStyleUseDefault "plotDataStyleUseDefault"
#define XtNplotDataStyles2UseDefault "plotDataStyles2UseDefault"
#define XtNplotMarkerDataStyle "plotMarkerDataStyle"
#define XtNplotMarkerDataStyleUseDefault "plotMarkerDataStyleUseDefault"
#define XtNplotOtherDataStyle "plotOtherDataStyle"
#define XtNplotOtherDataStyleUseDefault "plotOtherDataStyleUseDefault"
#define XtNplotDoubleBuffer "plotDoubleBuffer"
#define XtNplotAxisBoundingBox "plotAxisBoundingBox"
#define XtNplotTimeBase "plotTimeBase"
#define XtNplotTimeFormat "plotTimeFormat"
#define XtNplotTimeFormatUseDefault "plotTimeFormatUseDefault"
#define XtNplotTimeUnit "plotTimeUnit"
#define XtNplotType "plotType"
#define XtNplotType2 "plotType2"

#define XtNplotXTitle "plotXTitle"
#define XtNplotYTitle "plotYTitle"
#define XtNplotY2Title "plotY2Title"
#define XtNplotXMax "plotXMax" 
#define XtNplotXMin "plotXMin"
#define XtNplotYMax "plotYMax" 
#define XtNplotYMin "plotYMin" 
#define XtNplotXMarker "plotXMarker" 
#define XtNplotYMarker "plotYMarker"
#define XtNplotAxisBoundingBox "plotAxisBoundingBox"

#define XtNplotBackgroundColor "plotBackgroundColor"
#define XtNplotForegroundColor "plotForegroundColor"
#define XtNplotHeaderBackgroundColor "plotHeaderBackgroundColor"
#define XtNplotHeaderForegroundColor "plotHeaderForegroundColor"
#define XtNplotHeaderAdjust "plotHeaderAdjust"
#define XtNplotHeaderBorder "plotHeaderBorder"
#define XtNplotHeaderBorderWidth "plotHeaderBorderWidth"
#define XtNplotHeaderFont "plotHeaderFont"
#define XtNplotHeaderHeight "plotHeaderHeight"
#define XtNplotHeaderStrings "plotHeaderStrings"
#define XtNplotHeaderWidth "plotHeaderWidth"
#define XtNplotHeaderX "plotHeaderX"
#define XtNplotHeaderXUseDefault "plotHeaderXUseDefault"
#define XtNplotHeaderY "plotHeaderY"
#define XtNplotHeaderYUseDefault "plotHeaderYUseDefault"

#define XtNplotFooterBackgroundColor "plotFooterBackgroundColor"
#define XtNplotFooterForegroundColor "plotFooterForegroundColor"
#define XtNplotFooterFont "plotFooterFont"
#define XtNplotFooterAdjust "plotFooterAdjust"
#define XtNplotFooterBorder "plotFooterBorder"
#define XtNplotFooterBorderWidth "plotFooterBorderWidth"
#define XtNplotFooterHeight "plotFooterHeight"
#define XtNplotFooterStrings "plotFooterStrings"
#define XtNplotFooterWidth "plotFooterWidth"
#define XtNplotFooterX "plotFooterX"
#define XtNplotFooterXUseDefault "plotFooterXUseDefault"
#define XtNplotFooterY "plotFooterY"
#define XtNplotFooterYUseDefault "plotFooterYUseDefault"
#define XtNplotLegendBackgroundColor "plotLegendBackgroundColor"
#define XtNplotLegendForegroundColor "plotLegendForegroundColor"
#define XtNplotLegendAnchor "plotLegendAnchor"
#define XtNplotLegendBorder "plotLegendBorder"
#define XtNplotLegendBorderWidth "plotLegendBorderWidth"
#define XtNplotLegendFont "plotLegendFont"
#define XtNplotLegendHeight "plotLegendHeight"
#define XtNplotLegendOrientation "plotLegendOrientation"
#define XtNplotLegendShow "plotLegendShow"
#define XtNplotLegendWidth "plotLegendWidth"
#define XtNplotLegendX "plotLegendX"
#define XtNplotLegendXUseDefault "plotLegendXUseDefault"
#define XtNplotLegendY "plotLegendY"
#define XtNplotLegendYUseDefault "plotLegendYUseDefault"

#define XtNplotGraphBackgroundColor "plotGraphBackgroundColor"
#define XtNplotGraphForegroundColor "plotGraphForegroundColor"
#define XtNplotGraph3dShading "plotGraph3dShading"
#define XtNplotGraphBorder "plotGraphBorder"
#define XtNplotGraphBorderWidth "plotGraphBorderWidth"
#define XtNplotGraphDepth "plotGraphDepth"
#define XtNplotGraphHeight "plotGraphHeight"
#define XtNplotGraphHeightUseDefault "plotGraphHeightUseDefault"
#define XtNplotGraphInclination "plotGraphInclination"
#define XtNplotGraphMarginBottom "plotGraphMarginBottom"
#define XtNplotGraphMarginBottomUseDefault "plotGraphMarginBottomUseDefault"
#define XtNplotGraphMarginLeft "plotGraphMarginLeft"
#define XtNplotGraphMarginLeftUseDefault "plotGraphMarginLeftUseDefault"
#define XtNplotGraphMarginRight "plotGraphMarginRight"
#define XtNplotGraphMarginRightUseDefault "plotGraphMarginRightUseDefault"
#define XtNplotGraphMarginTop "plotGraphMarginTop"
#define XtNplotGraphMarginTopUseDefault "plotGraphMarginTopUseDefault"
#define XtNplotGraphRotation "plotGraphRotation"
#define XtNplotGraphShowOutlines "plotGraphShowOutlines"
#define XtNplotGraphWidth "plotGraphWidth"
#define XtNplotGraphWidthUseDefault "plotGraphWidthUseDefault"
#define XtNplotGraphX "plotGraphX"
#define XtNplotGraphXUseDefault "plotGraphXUseDefault"
#define XtNplotGraphY "plotGraphY"
#define XtNplotGraphYUseDefault "plotGraphYUseDefault"

#define XtNplotAxisFont "plotAxisFont"
#define XtNplotBorder "plotBorder"
#define XtNplotDataAreaBackgroundColor "plotDataAreaBackgroundColor"
#define XtNplotDataAreaForegroundColor "plotdataAreaBackgroundColor"

#define XtNplotData "plotData"
#define XtNplotData2 "plotData2"
#define XtNplotDataStyles "plotDataStyles"
#define XtNplotDataStyles2 "plotDataStyles2"
#define XtNplotDataStylesUseDefault "plotDataStylesUseDefault"

#define XtNplotSetLabels "plotSetLabels"
#define XtNplotSetLabels2 "plotSetLabels2"
#define XtNplotPointLabels "plotPointLabels"
#define XtNplotPointLabels2 "plotPointsLabels2"
#define XtNplotType "plotType"
#define XtNplotType2 "plotType2"
#define XtNplotXAnnoPlacement "plotXAnnoPlacement"
#define XtNplotYAnnoPlacement "plotYAnnoPlacement"
#define XtNplotY2AnnoPlacement "plotYAnnoPlacement"
#define XtNplotXAnnotationMethod "plotXAnnotationMethod"
#define XtNplotYAnnotationMethod "plotYAnnotationMethod"
#define XtNplotY2AnnotationMethod "plotY2AnnotationMethod"
#define XtNplotXAnnotationRotation "plotXAnnotationRotation"
#define XtNplotYAnnotationRotation "plotYAnnotationRotation"
#define XtNplotY2AnnotationRotation "plotY2AnnotationRotation"
#define XtNplotXAxisLabelFormat "plotXAxisLabelFormat"
#define XtNplotYAxisLabelFormat "plotYAxisLabelFormat"
#define XtNplotY2AxisLabelFormat "plotY2AxisLabelFormat"

#define XtNplotXAxisLogarithmic "plotXAxisLogarithmic"
#define XtNplotYAxisLogarithmic "plotYAxisLogarithmic"
#define XtNplotY2AxisLogarithmic "plotY2AxisLogarithmic"
#define XtNplotXAxisMax "plotXAxisMax"
#define XtNplotYAxisMax "plotYAxisMax"
#define XtNplotY2AxisMax "plotY2AxisMax"
#define XtNplotXAxisMaxUseDefault "plotXAxisMaxUseDefault"
#define XtNplotYAxisMaxUseDefault "plotYAxisMaxUseDefault"
#define XtNplotY2AxisMaxUseDefault "plotY2AxisMaxUseDefault"
#define XtNplotXAxisMin "plotXAxisMin"
#define XtNplotYAxisMin "plotYAxisMin"
#define XtNplotY2AxisMin "plotY2AxisMin"
#define XtNplotXAxisMinUseDefault "plotXAxisMinUseDefault"
#define XtNplotYAxisMinUseDefault "plotYAxisMinUseDefault"
#define XtNplotY2AxisMinUseDefault "plotY2AxisMinUseDefault"
#define XtNplotXAxisReversed "plotXAxisReversed"
#define XtNplotYAxisReversed "plotYAxisReversed"
#define XtNplotY2AxisReversed "plotY2AxisReversed"
#define XtNplotXAxisShow "plotXAxisShow"
#define XtNplotYAxisShow "plotYAxisShow"
#define XtNplotY2AxisShow "plotY2AxisShow"
#define XtNplotXGrid "plotXGrid"
#define XtNplotYGrid "plotYGrid"
#define XtNplotY2Grid "plotY2Grid"
#define XtNplotXGridUseDefault "plotXGridUseDefault"
#define XtNplotYGridUseDefault "plotYGridUseDefault"
#define XtNplotY2GridUseDefault "plotY2GridUseDefault"
#define XtNplotXGridDataStyle "plotXGridDataStyle"
#define XtNplotYGridDataStyle "plotYGridDataStyle"
#define XtNplotY2GridDataStyle "plotY2GridDataStyle"
#define XtNplotXGridDataStyleUseDefault "plotXGridDataStyleUseDefault"
#define XtNplotYGridDataStyleUseDefault "plotYGridDataStyleUseDefault"
#define XtNplotY2GridDataStyleUseDefault "plotY2GridDataStyleUseDefault"
#define XtNplotXLabels "plotXLabels"
#define XtNplotYLabels "plotYLables"
#define XtNplotY2Labels "plotY2Labels"
#define XtNplotXMarker "plotXMarker"
#define XtNplotYMarker "plotYMarker"
#define XtNplotXMarkerDataStyle "plotXMarkerDataStyle"
#define XtNplotYMarkerDataStyle "plotYMarkerDataStyle"
#define XtNplotXMarkerDataStyleUseDefault "plotXMarkerDataStyleUseDefault"
#define XtNplotYMarkerDataStyleUseDefault "plotYMarkerDataStyleUseDefault"
#define XtNplotXMarkerMethod "plotXMarkerMethod"
#define XtNplotYMarkerMethod "plotYMarkerMethod"
#define XtNplotXMarkerPoint "plotXMarkerPoint"
#define XtNplotYMarkerPoint "plotYMarkerPoint"
#define XtNplotXMarkerSet "plotXMarkerSet"
#define XtNplotYMarkerSet "plotYMarkerSet"
#define XtNplotMarkerShow "plotMarkerShow"
#define XtNplotXMarkerShow "plotXMarkerShow"
#define XtNplotYMarkerShow "plotYMarkerShow"
#define XtNplotXMax "plotXMax"
#define XtNplotYMax "plotYMax"
#define XtNplotY2Max "plotY2Max"
#define XtNplotXMaxUseDefault "plotXMaxUseDefault"
#define XtNplotYMaxUseDefault "plotYMaxUseDefault"
#define XtNplotY2MaxUseDefault "plotY2MaxUseDefault"
#define XtNplotXMin "plotXMin"
#define XtNplotYMin "plotYMin"
#define XtNplotY2Min "plotY2Min"
#define XtNplotXMinUseDefault "plotXMinUseDefault"
#define XtNplotYMinUseDefault "plotYMinUseDefault"
#define XtNplotY2MinUseDefault "plotY2MinUseDefault"
#define XtNplotXNum "plotXNum"
#define XtNplotYNum "plotYNum"
#define XtNplotY2Num "plotY2Num"
#define XtNplotXNumUseDefault "plotXNumUseDefault"
#define XtNplotYNumUseDefault "plotYNumUseDefault"
#define XtNplotY2NumUseDefault "plotY2NumUseDefault"
#define XtNplotXNumMethod "plotXNumMethod"
#define XtNplotYNumMethod "plotYNumMethod"
#define XtNplotY2NumMethod "plotY2NumMethod"
#define XtNplotXOrigin "plotXOrigin"
#define XtNplotYOrigin "plotYOrigin"
#define XtNplotY2Origin "plotY2Origin"
#define XtNplotXOriginBase "plotXOriginBase"
#define XtNplotYOriginBase "plotYOriginBase"
#define XtNplotY2OriginBase "plotY2OriginBase"
#define XtNplotXOriginPlacement "plotXOriginPlacement"
#define XtNplotYOriginPlacement "plotYOriginPlacement"
#define XtNplotY2OriginPlacement "plotY2OriginPlacement"
#define XtNplotXOriginUseDefault "plotXOriginUseDefault"
#define XtNplotYOriginUseDefault "plotYOriginUseDefault"
#define XtNxrY2OriginUseDefaultt "plotY2OriginUseDefault"
#define XtNplotXPrecision "plotXPrecision"
#define XtNplotYPrecision "plotYPrecision"
#define XtNplotY2Precision "plotY2Precision"
#define XtNplotXPrecisionUseDefault "plotXPrecisionUseDefault"
#define XtNplotYPrecisionUseDefault "plotYPrecisionUseDefault"
#define XtNplotY2PrecisionUseDefault "plotY2PrecisionUseDefault"
#define XtNplotXTick "plotXTick"
#define XtNplotYTick "plotYTick"
#define XtNplotY2Tick "plotY2Tick"
#define XtNplotXTickUseDefault "plotXTickUseDefault"
#define XtNplotYTickUseDefault "plotYTickUseDefault"
#define XtNplotY2TickUseDefault "plotY2TickUseDefault"
#define XtNplotXTitle "plotXTitle"
#define XtNplotYTitle "plotYTitle"
#define XtNplotY2Title "plotY2Title"
#define XtNplotXTitleRotation "plotXTitleRotation"
#define XtNplotYTitleRotation "plotYTitleRotation"
#define XtNplotY2TitleRotation "plotY2TitleRotation"
#define XtNplotYAnnotationAngle "plotYAnnotationAngle"
#define XtNplotY2AnnotationAngle "plotY2AnnotationAngle"
#define XtNplotYAnnotationAngleUseDefault "plotYAnnotationAngleUseDefault"
#define XtNplotY2AnnotationAngleUseDefault "plotY2AnnotationAngleUseDefault"
#define XtNplotYAxis100Percent "plotYAxis100Percent"
#define XtNplotY2Axis100Percent "plotY2Axis100Percent"
#define XtNplotYAxisConst "plotYAxisConst"
#define XtNplotYAxisMult "plotYAxisMult"
#define XtNplotZoomAxisCallback "plotZoomAxisCallback"

/*
 *   Resource classes
 */

#ifndef XtCTitle
#define XtCTitle              "Title"
#endif
#define XtCLegendTitle        "LegendTitle"

#define XtCShowTitle          "ShowTitle"
#define XtCShowLegend         "ShowLegend"
#define XtCTitleHeight        "TitleHeight"
#define XtCLegendLeft         "LegendLeft"
#define XtCLegendWidth        "LegendWidth"

#define XtCRankChildren       "RankChildren"
#define XtCAutoRedisplay      "AutoRedisplay"
#define XtCUseCursors         "UseCursors"
#define XtCUsePixmap          "UsePixmap"

#define XtCXAxis              "XAxis"
#define XtCYAxis              "YAxis"
#define XtCX2Axis             "X2Axis"
#define XtCY2Axis             "Y2Axis"

#define XtCPlotAdjust         "PlotAdjust"
#define XtCPlotAnnotationMethod "PlotAnnotationMethod"
#define XtCPlotAxisBounds     "PlotAxisBounds"
#define XtCPlotAxisParams     "PlotAxisParams"
#define XtCPlotBackgroundColor "PlotBackgroundColor"
#define XtCPlotBorder         "PlotBorder"
#define XtCPlotData           "PlotData"
#define XtCPlotDataStyles     "PlotDataStyles"
#define XtCPlotDataStyle      "PlotDataStyle"
#define XtCPlotDataStylesUseDefault "PlotDataStylesUseDefault"
#define XtCPlotDimension      "PlotDimension"
#define XtCPlotDimensionUseDefault "PlotDimensionUseDefault"
#define XtCPlotDoubleBuffer "PlotDoubleBuffer"
#define XtCPlotDrawOption "PlotDrawOption"
#define XtCPlotForegroundColor "PlotForegroundColor"


#define XtCPlotGraphBorder "PlotGraphBorder"
#define XtCPlotGraphBorderWidth "PlotGraphBorderWidth"
#define XtCPlotGraphMargin "PlotGraphMargin"
#define XtCPlotGraphMarginUseDefault "PlotGraphMarginUseDefault"
#define XtCPlotGraphX "PlotGraphX"
#define XtCPlotGraphY "PlotGraphY"
#define XtCPlotGraphXUseDefault "PlotGraphXUseDefault"
#define XtCPlotGraphYUseDefault "PlotGraphYUseDefault"
#define XtCPlotHeaderX "PlotHeaderX"
#define XtCPlotHeaderY "PlotHeaderY"
#define XtCPlotHeaderXUseDefault "PlotHeaderXUseDefault"
#define XtCPlotHeaderYUseDefault "PlotHeaderYUseDefault"
#define XtCPlotHeaderWidth "PlotHeaderWidth"
#define XtCPlotHeaderHeight "PlotHeaderHeight"
#define XtCPlotHeaderBorder "PlotHeaderBorder"
#define XtCPlotHeaderBorderWidth "PlotHeaderBorderWidth"
#define XtCPlotLegendX "PlotLegendX"
#define XtCPlotLegendY "PlotLegendY"
#define XtCPlotLegendXUseDefault "PlotLegendXUseDefault"
#define XtCPlotLegendYUseDefault "PlotLegendYUseDefault"
#define XtCPlotLegendBorder "PlotLegendBorder"
#define XtCPlotLegendBorderWidth "PlotLegendBorderWidth"


#define XtCPlotMargin "PlotMargin"
#define XtCPlotMarginUseDefault "PlotMarginUseDefault"
#define XtCPlotMarker "PlotMarker"
#define XtCPlotMarkerMethod "PlotMarkerMethod"
#define XtCPlotMarkerPoint "PlotMarkerPoint"
#define XtCPlotMarkerSet "PlotMarkerSet"
#define XtCPlotMarkerShow "PlotMarkerShow"
#define XtCPlotPosition   "PlotPosition"
#define XtCPlotPositionUseDefault "PlotPositionUseDefault"
#define XtCPlotPrecision "PlotPrecision"
#define XtCPlotSetLabels "PlotSetLabels"
#define XtCPlotString "PlotString"
#define XtCPlotStrings "PlotStrings"
#define XtCPlotTimeBase             "PlotTimeBase"
#define XtCPlotTimeFormat           "PlotTimeFormat"
#define XtCPlotTimeFormatUseDefault "PlotTimeFormatUseDefault"
#define XtCPlotTimeUnit             "PlotTimeUnit"
#define XtCPlotType      "PlotType"
#define XtCPlotXMax "PlotXMax" 
#define XtCPlotXMin "PlotXMin"
#define XtCPlotXTitle "PlotXTitle"
#define XtCPlotYAxisConst "PlotYAxisConst"
#define XtCPlotYAxisMult  "PlotYAxisMult"
#define XtCPlotYMax "PlotYMax" 
#define XtCPlotYMin "PlotYMin" 
#define XtCPlotYTitle "PlotYTitle"

#define XtRPlotAnnotationMethod "PlotAnnotationMethod"
#define XtRPlotDataStyles "XtRPlotDataStyles"
#define XtRLong          "Long"
#define XtRPlotStrings   "XtRPlotStrings"

/*
 *   Constraint resource names and classes
 */

#define XtNdisplayed          "displayed"
#define XtNlegendName         "legendName"
#define XtNuseX2Axis          "useX2Axis"
#define XtNuseY2Axis          "useY2Axis"
#define XtNrankOrder          "rankOrder"

#define XtCDisplayed          "Displayed"
#define XtCLegendName         "LegendName"
#define XtCUseX2Axis          "UseX2Axis"
#define XtCUseY2Axis          "UseY2Axis"
#define XtCRankOrder          "RankOrder"

/*
 * Jpt functions 
 */
#define WnColor(w, c) ColorNameToPixel(w, c)

extern PlotDataHandle PlotDataCreate P((PlotDataType hData, int nsets, int npoints));
extern void PlotDataDestroy P((PlotDataHandle hData));
extern int PlotDataGetLastPoint P((PlotDataHandle hData, int set));
extern int PlotDataGetNPoints P((PlotDataHandle hData, int set));
extern int PlotDataGetNSets P((PlotDataHandle));
extern double PlotDataGetXElement P((PlotDataHandle hData, int set, int point));
extern double PlotDataGetYElement P((PlotDataHandle hData, int set, int point));
extern int PlotDataSetHole P((PlotDataHandle hData, double hole));
extern int PlotDataSetLastPoint P((PlotDataHandle hData, int set, int npoints));
extern int PlotDataSetNPoints P((PlotDataHandle hData, int set, int npoints));
extern int PlotDataSetXElement P((PlotDataHandle hData, int set, int point, double x));
extern int PlotDataSetYElement P((PlotDataHandle hData, int set, int point, double y));

extern Pixel PlotColor P((Widget, char *, char *));
extern PlotRegion PlotMap P((Widget, int, int, int, PlotMapResult *));
extern PlotData *PlotMakeData P((PlotDataType, int, int, int));
extern PlotData *PlotMakeDataFromFile P((char *, char *));
extern XtArgVal PlotFloatToArgVal P((float));
extern char ** PlotDupStrings P((char **));
extern PlotDataStyle ** PlotDupDataStyles P((PlotDataStyle **));
extern void PlotFreeStrings P((char **));
extern void PlotFreeDataStyles P((PlotDataStyle **));

extern PlotTextHandle PlotTextCreate P((Widget, PlotTextDesc *));
extern int PlotTextDetail P((Widget, PlotTextHandle, PlotTextDesc *));
extern void PlotTextUpdate P((Widget, PlotTextHandle, PlotTextDesc *));
extern int PlotTextDetach P((Widget, PlotTextHandle));
extern int PlotTextAttach P((Widget, PlotTextHandle));
extern void PlotTextDestroy P((Widget, PlotTextHandle));
extern int PlotGetTextHandles P((Widget graph, PlotTextHandle **list));

extern void PlotDestroyData P((PlotData *, Boolean));
extern void PlotSetNthDataStyle P((Widget, int, PlotDataStyle *));
extern void PlotSetNthDataStyle2 P((Widget, int,PlotDataStyle *));
extern Pixel ColorNameToPixel P((Widget, char *));
extern Pixel WnColorD P((String, Display *));
extern Pixel WnColorB P((Widget, char *));
extern Pixel WnColorF P((Widget, char *));
extern int copyStrings P((char **src, char **des));
/*
 * AtPlotter common data structures and functions
 */
/*
 *   Axis positions and set position mask values
 */

typedef struct {
     char position;                /* Position mask */
     Position xaxis;               /* The axis positions */
     Position x2axis;
     Position yaxis;
     Position y2axis;
} AtAxisPositions;

#define AtPositionNONE   0x00      /* Nothing to position */
#define AtPositionXAXES  0x01      /* Position the X axes */
#define AtPositionYAXES  0x02      /* Position the Y axes */

/*
 *   Callback data types
 */

/* Client data from busy callback */

#define AtBusyPLOTTER    1
#define AtBusyPOSTSCRIPT 2

typedef struct {
     int reason;                   /* Reason for this callback */
     Boolean busy;                 /* True if busy, False otherwise */
} AtBusyCallbackData;

/* Client data from motion and click callbacks */

#define AtPointMOTION    1
#define AtPointCLICK     2

typedef struct {
     int reason;                   /* Reason for this callback */
     Position pixelx, pixely;      /* Pixel    point coordinates */
     float x1, y1;                /* 1st axes point coordinates */
     float x2, y2;                /* 2nd axes point coordinates */
} AtPointCallbackData;

/* Client data from drag and slide callbacks */

#define AtRectangleDRAG  1
#define AtRectangleSLIDE 2

typedef struct {
     int reason;                   /* The reason for this callback */
     Position pixelx1, pixely1;    /* Pixel lower left */
     Position pixelx2, pixely2;    /* Pixel upper rigth */
     float x11, y11, x12, y12;    /* 1st axes range coordinates */
     float x21, y21, x22, y22;    /* 2nd axes range coordinates */
} AtRectangleCallbackData;

/* Client data from select callback */

#define AtSelectSELECTED      1
#define AtSelectDESELECTED    2

typedef struct {
     int reason;                   /* The reason for this callback */
     Widget widget;                /* The selected/deselected widget */
} AtSelectCallbackData;


/*
 *   Class record constants
 */

extern WidgetClass atPlotterWidgetClass;

typedef struct _AtPlotterClassRec * AtPlotterWidgetClass;
typedef struct _AtPlotterRec      * AtPlotterWidget;

void AtPlotterGeneratePostscript P((char *, AtPlotterWidget, char *,
				    int, int, int, int, int));
extern void AtPlotterDrawPS P((FILE *, AtPlotterWidget, int, int, int, int));

/*
 *   These functions are for the plot children to communicate with the parent
 */

typedef struct {
    float xmin, xmax, ymin, ymax;
} BoundingBox;

extern int PlotterDrawXMarker P((Widget));
extern void AtPlotterPlotExtended P((AtPlotWidget, BoundingBox *, int, int));
extern void AtPlotterPlotDataChanged P((AtPlotWidget, BoundingBox *, int));
extern void AtPlotterRefreshRequired P((AtPlotWidget));
extern void AtPlotterRedrawRequired P((AtPlotWidget));
extern void AtPlotterLayoutRequired P((AtPlotWidget));
extern void AtPlotterRescaleRequired P((AtPlotWidget));
extern void AtPlotterRecalcThisPlot P((AtPlotWidget));

extern int Redraw P((AtPlotterWidget, Window, Drawable, Region));

/*
 *   These may be called from applications to get
 *   the plotters legend width and title height
 */

extern int AtPlotterGetLegendWidth P((AtPlotterWidget));
extern int AtPlotterGetTitleHeight P((AtPlotterWidget));

/*
 *   These may be called from applications to get and set
 *   the plotters axis positions, i.e. for axis alignments.
 */

extern Boolean AtPlotterGetAxisPositions P((AtPlotterWidget, AtAxisPositions *));
extern void AtPlotterSetAxisPositions P((AtPlotterWidget, AtAxisPositions *));

/*
 *   These may be used to get and set selected plot widgets.
 */

extern Widget AtPlotterGetSelectedPlot P((AtPlotterWidget));
extern Boolean AtPlotterSetSelectedPlot P((AtPlotterWidget, Widget));

#ifdef __cplusplus
};
#endif

#endif /* _AtPlotter_h */
