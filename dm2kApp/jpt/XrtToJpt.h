/* XeWrapper  defines marcoes to map XRT terms to Xe terms.
*
*
*/

#ifndef _Xrt_Wrapper_
#define _Xrt_Wrapper_

/*#include "PlotterP.h"
#include "XYLinePlotP.h"
#include "DataHandle.h"
#include "Text.h"
#include "Text2.h"
*/
#include "At.h"
#include "Plotter.h"

/*#define XRT_VERSION 3*/

/* map classes */
#define xtXrtGraphWidgetClass atPlotterWidgetClass 
#define XtXrtGraphWidgetClass XtJptWidgetClass
#define XtXrtGraphWidget XtJptWidget

/*map resources */
#define XtNxrtBackgroundColor XtNplotBackgroundColor
#define XtNxrtForegroundColor XtNplotForegroundColor
#define XtNxrtHeaderBackgroundColor XtNplotHeaderBackgroundColor
#define XtNxrtHeaderForegroundColor XtNplotHeaderForegroundColor
#define XtNxrtHeaderAdjust XtNplotHeaderAdjust
#define XtNxrtHeaderBorder XtNplotHeaderBorder
#define XtNxrtHeaderBorderWidth XtNplotHeaderBorderWidth
#define XtNxrtHeaderFont XtNplotHeaderFont
#define XtNxrtHeaderHeight XtNplotHeaderHeight
#define XtNxrtHeaderStrings XtNplotHeaderStrings
#define XtNxrtHeaderWidth XtNplotHeaderWidth
#define XtNxrtHeaderX XtNplotHeaderX
#define XtNxrtHeaderXUseDefault XtNplotHeaderXUseDefault
#define XtNxrtHeaderY XtNplotHeaderY
#define XtNxrtHeaderYUseDefault XtNplotHeaderYUseDefault

#define XtNxrtFooterBackgroundColor XtNplotFooterBackgroundColor
#define XtNxrtFooterForegroundColor XtNplotFooterForegroundColor
#define XtNxrtFooterFont XtNplotFooterFont
#define XtNxrtFooterAdjust XtNplotFooterAdjust
#define XtNxrtFooterBorder XtNplotFooterBorder
#define XtNxrtFooterBorderWidth XtNplotFooterBorderWidth
#define XtNxrtFooterHeight XtNplotFooterHeight
#define XtNxrtFooterStrings XtNplotFooterStrings
#define XtNxrtFooterWidth XtNplotFooterWidth
#define XtNxrtFooterX XtNplotFooterX
#define XtNxrtFooterXUseDefault XtNplotFooterXUseDefault
#define XtNxrtFooterY XtNplotFooterY
#define XtNxrtFooterYUseDefault XtNplotFooterYUseDefault
#define XtNxrtLegendBackgroundColor XtNplotLegendBackgroundColor
#define XtNxrtLegendForegroundColor XtNplotLegendForegroundColor
#define XtNxrtLegendAnchor XtNplotLegendAnchor
#define XtNxrtLegendBorder XtNplotLegendBorder
#define XtNxrtLegendBorderWidth XtNplotLegendBorderWidth
#define XtNxrtLegendFont XtNplotLegendFont
#define XtNxrtLegendHeight XtNplotLegendHeight
#define XtNxrtLegendOrientation XtNplotLegendOrientation
#define XtNxrtLegendShow XtNplotLegendShow
#define XtNxrtLegendWidth XtNplotLegendWidth
#define XtNxrtLegendX XtNplotLegendX
#define XtNxrtLegendXUseDefault XtNplotLegendXUseDefault
#define XtNxrtLegendY XtNplotLegendY
#define XtNxrtLegendYUseDefault XtNplotLegendYUseDefault

#define XtNxrtGraphBackgroundColor XtNplotGraphBackgroundColor
#define XtNxrtGraphForegroundColor XtNplotGraphForegroundColor
#define XtNxrtGraph3dShading XtNplotGraph3dShading
#define XtNxrtGraphBorder XtNplotGraphBorder
#define XtNxrtGraphBorderWidth XtNplotGraphBorderWidth
#define XtNxrtGraphDepth XtNplotGraphDepth
#define XtNxrtGraphHeight XtNplotGraphHeight
#define XtNxrtGraphHeightUseDefault XtNplotGraphHeightUseDefault
#define XtNxrtGraphInclination XtNplotGraphInclination
#define XtNxrtGraphMarginBottom XtNplotGraphMarginBottom
#define XtNxrtGraphMarginBottomUseDefault XtNplotGraphMarginBottomUseDefault
#define XtNxrtGraphMarginLeft XtNplotGraphMarginLeft
#define XtNxrtGraphMarginLeftUseDefault XtNplotGraphMarginLeftUseDefault
#define XtNxrtGraphMarginRight XtNplotGraphMarginRight
#define XtNxrtGraphMarginRightUseDefault XtNplotGraphMarginRightUseDefault
#define XtNxrtGraphMarginTop XtNplotGraphMarginTop
#define XtNxrtGraphMarginTopUseDefault XtNplotGraphMarginTopUseDefault
#define XtNxrtGraphRotation XtNplotGraphRotation
#define XtNxrtGraphShowOutlines XtNplotGraphShowOutlines
#define XtNxrtGraphWidth XtNplotGraphWidth
#define XtNxrtGraphWidthUseDefault XtNplotGraphWidthUseDefault
#define XtNxrtGraphX XtNplotGraphX
#define XtNxrtGraphXUseDEfault XtNplotGraphXUseDefault
#define XtNxrtGraphY XtNplotGraphY
#define XtNxrtGraphYUseDEfault XtNplotGraphYUseDefault

#define XtNxrtAxisFont XtNplotAxisFont
#define XtNxrtBorder XtNplotBorder
#define XtNxrtDataAreaBackgroundColor XtNplotDataAreaBackgroundColor
#define XtNxrtDataAreaForegroundColor XtNplotdataAreaForegroundColor

#define XtNxrtData XtNplotData
#define XtNxrtData2 XtNplotData2
#define XtNxrtDataStyles XtNplotDataStyles
#define XtNxrtDataStyles2 XtNplotDataStyles2
#define XtNxrtDataStylesUseDefault XtNplotDataStylesUseDefault
#define XtNxrtDataStyles2UseDefault XtNplotDataStyles2UseDefault
#define XtNxrtMarkerDataStyle XtNplotMarkerDataStyle
#define XtNxrtMarkerDataStyleUseDefault XtNplotMarkerDataStyleUseDefault
#define XtNxrtOtherDataStyle XtNplotOtherDataStyle
#define XtNxrtOtherDataStyleUseDefault XtNplotOtherDataStyleUseDefault

#define XtNxrtAxisBoundingBox XtNplotAxisBoundingBox
#define XtNxrtDebug XtNplotDebug
#define XtNxrtDoubleBuffer XtNplotDoubleBuffer
#define XtNxrtExposeCallback XtNplotExposeCallback

#define XtNxrtXMax XtNplotXMax
#define XtNxrtXMin XtNplotXMin
#define XtNxrtYMax XtNplotYMax
#define XtNxrtYMin XtNplotYMin
#define XtNxrtXMarker XtNplotXMarker
#define XtNxrtYMarker XtNplotYMarker

#define XtNxrtSetLabels XtNplotSetLabels
#define XtNxrtSetLabels2 XtNplotSetLabels2
#define XtNxrtPointLabels XtNplotPointLabels
#define XtNxrtPointLabels2 XtNplotPointsLabels2
#define XtNxrtTimeBase XtNplotTimeBase
#define XtNxrtTimeFormat XtNplotTimeFormat
#define XtNxrtTimeFormatUseDefault XtNplotTimeFormatUseDefault
#define XtNxrtTimeUnit XtNplotTimeUnit
#define XtNxrtType XtNplotType
#define XtNxrtType2 XtNplotType2
#define XtNxrtXAnnoPlacement XtNplotXAnnoPlacement
#define XtNxrtYAnnoPlacement XtNplotYAnnoPlacement
#define XtNxrtY2AnnoPlacement XtNplotYAnnoPlacement
#define XtNxrtXAnnotationMethod XtNplotXAnnotationMethod
#define XtNxrtYAnnotationMethod XtNplotYAnnotationMethod
#define XtNxrtY2AnnotationMethod XtNplotY2AnnotationMethod
#define XtNxrtXAnnotationRotation XtNplotXAnnotationRotation
#define XtNxrtYAnnotationRotation XtNplotYAnnotationRotation
#define XtNxrtY2AnnotationRotation XtNplotY2AnnotationRotation
#define XtNxrtXAxisLabelFormat XtNplotXAxisLabelFormat
#define XtNxrtYAxisLabelFormat XtNplotYAxisLabelFormat
#define XtNxrtY2AxisLabelFormat XtNplotY2AxisLabelFormat

#define XtNxrtXAxisLogarithmic XtNplotXAxisLogarithmic
#define XtNxrtYAxisLogarithmic XtNplotYAxisLogarithmic
#define XtNxrtY2AxisLogarithmic XtNplotY2AxisLogarithmic
#define XtNxrtXAxisMax XtNplotXAxisMax
#define XtNxrtYAxisMax XtNplotYAxisMax
#define XtNxrtY2AxisMax XtNplotY2AxisMax
#define XtNxrtXAxisMaxUseDefault XtNplotXAxisMaxUseDefault
#define XtNxrtYAxisMaxUseDefault XtNplotYAxisMaxUseDefault
#define XtNxrtY2AxisMaxUseDefault XtNplotY2AxisMaxUseDefault
#define XtNxrtXAxisMin XtNplotXAxisMin
#define XtNxrtYAxisMin XtNplotYAxisMin
#define XtNxrtY2AxisMin XtNplotY2AxisMin
#define XtNxrtXAxisMinUseDefault XtNplotXAxisMinUseDefault
#define XtNxrtYAxisMinUseDefault XtNplotYAxisMinUseDefault
#define XtNxrtY2AxisMinUseDefault XtNplotY2AxisMinUseDefault
#define XtNxrtXAxisReversed XtNplotXAxisReversed
#define XtNxrtYAxisReversed XtNplotYAxisReversed
#define XtNxrtY2AxisReversed XtNplotY2AxisReversed
#define XtNxrtXAxisShow XtNplotXAxisShow
#define XtNxrtYAxisShow XtNplotYAxisShow
#define XtNxrtY2AxisShow XtNplotY2AxisShow
#define XtNxrtXGrid XtNplotXGrid
#define XtNxrtYGrid XtNplotYGrid
#define XtNxrtY2Grid XtNplotY2Grid
#define XtNxrtXGridUseDefault XtNplotXGridUseDefault
#define XtNxrtYGridUseDefault XtNplotYGridUseDefault
#define XtNxrtY2GridUseDefault XtNplotY2GridUseDefault
#define XtNxrtXGridDataStyle XtNplotXGridDataStyle
#define XtNxrtYGridDataStyle XtNplotYGridDataStyle
#define XtNxrtY2GridDataStyle XtNplotY2GridDataStyle
#define XtNxrtXGridDataStyleUseDefault XtNplotXGridDataStyleUseDefault
#define XtNxrtYGridDataStyleUseDefault XtNplotYGridDataStyleUseDefault
#define XtNxrtY2GridDataStyleUseDefault XtNplotY2GridDataStyleUseDefault
#define XtNxrtXLabels XtNplotXLabels
#define XtNxrtYLabels XtNplotYLables
#define XtNxrtY2Labels XtNplotY2Labels
#define XtNxrtXMarker XtNplotXMarker
#define XtNxrtYMarker XtNplotYMarker
#define XtNxrtXMarkerDataStyle XtNplotXMarkerDataStyle
#define XtNxrtYMarkerDataStyle XtNplotYMarkerDataStyle
#define XtNxrtXMarkerDataStyleUseDefault XtNplotXMarkerDataStyleUseDefault
#define XtNxrtYMarkerDataStyleUseDefault XtNplotYMarkerDataStyleUseDefault
#define XtNxrtXMarkerMethod XtNplotXMarkerMethod
#define XtNxrtXMarkerPoint XtNplotXMarkerPoint
#define XtNxrtXMarkerSet XtNplotXMarkerSet
#define XtNxrtXMarkerShow XtNplotXMarkerShow
#define XtNxrtYMarkerShow XtNplotYMarkerShow
#define XtNxrtXMax XtNplotXMax
#define XtNxrtYMax XtNplotYMax
#define XtNxrtY2Max XtNplotY2Max
#define XtNxrtXMaxUseDefault XtNplotXMaxUseDefault
#define XtNxrtYMaxUseDefault XtNplotYMaxUseDefault
#define XtNxrtY2MaxUseDefault XtNplotYMaxUseDefault
#define XtNxrtXMin XtNplotXMin
#define XtNxrtYMin XtNplotYMin
#define XtNxrtY2Min XtNplotY2Min
#define XtNxrtXMinUseDefault XtNplotXMinUseDefault
#define XtNxrtYMinUseDefault XtNplotYMinUseDefault
#define XtNxrtY2MinUseDefault XtNplotY2MinUseDefault
#define XtNxrtXNum XtNplotXNum
#define XtNxrtYNum XtNplotYNum
#define XtNxrtY2Num XtNplotY2Num
#define XtNxrtXNumUseDefault XtNplotXNumUseDefault
#define XtNxrtYNumUseDefault XtNplotYNumUseDefault
#define XtNxrtY2NumUseDefault XtNplotY2NumUseDefault
#define XtNxrtXNumMethod XtNplotXNumMethod
#define XtNxrtYNumMethod XtNplotYNumMethod
#define XtNxrtY2NumMethod XtNplotY2NumMethod
#define XtNxrtXOrigin XtNplotXOrigin
#define XtNxrtYOrigin XtNplotYOrigin
#define XtNxrtY2Origin XtNplotY2Origin
#define XtNxrtXOriginBase XtNplotXOriginBase
#define XtNxrtYOriginBase XtNplotYOriginBase
#define XtNxrtY2OriginBase XtNplotY2OriginBase
#define XtNxrtXOriginPlacement XtNplotXOriginPlacement
#define XtNxrtYOriginPlacement XtNplotYOriginPlacement
#define XtNxrtY2OriginPlacement XtNplotY2OriginPlacement
#define XtNxrtXOriginUseDefault XtNplotXOriginUseDefault
#define XtNxrtYOriginUseDefault XtNplotYOriginUseDefault
#define XtNxrtY2OriginUseDefaultt XtNplotY2OriginUseDefault
#define XtNxrtXPrecision XtNplotXPrecision
#define XtNxrtYPrecision XtNplotYPrecision
#define XtNxrtY2Precision XtNplotY2Precision
#define XtNxrtXPrecisionUseDefault XtNplotXPrecisionUseDefault
#define XtNxrtYPrecisionUseDefault XtNplotYPrecisionUseDefault
#define XtNxrtY2PrecisionUseDefault XtNplotY2PrecisionUseDefault
#define XtNxrtXTick XtNplotXTick
#define XtNxrtYTick XtNplotYTick
#define XtNxrtY2Tick XtNplotY2Tick
#define XtNxrtXTickUseDefault XtNplotXTickUseDefault
#define XtNxrtYTickUseDefault XtNplotYTickUseDefault
#define XtNxrtY2TickUseDefault XtNplotY2TickUseDefault
#define XtNxrtXTitle XtNplotXTitle
#define XtNxrtYTitle XtNplotYTitle
#define XtNxrtY2Title XtNplotY2Title
#define XtNxrtXTitleRotation XtNplotXTitleRotation
#define XtNxrtYTitleRotation XtNplotYTitleRotation
#define XtNxrtY2TitleRotation XtNplotY2TitleRotation
#define XtNxrtYAnnotationAngle XtNplotYAnnotationAngle
#define XtNxrtY2AnnotationAngle XtNplotY2AnnotationAngle
#define XtNxrtYAnnotationAngleUseDefault XtNplotYAnnotationAngleUseDefault
#define XtNxrtY2AnnotationAngleUseDefault XtNplotY2AnnotationAngleUseDefault
#define XtNxrtYAxis100Percent XtNplotYAxis100Percent
#define XtNxrtY2Axis100Percent XtNplotY2Axis100Percent
#define XtNxrtYAxisConst XtNplotYAxisConst
#define XtNxrtYAxisMult XtNplotYAxisMult
#define XtNxrtZoomAxisCallback XtNplotZoomAxisCallback

/* map resources class */
#define XtCXrtAdjust                 XtCPlotAdjust
#define XtCXrtBackgroundColor        XtCPlotBackgroundColor
#define XtCXrtBorder                 XtCPlotBorder
#define XtCXrtData                   XtCPlotData
#define XtCXrtDataStyles             XtCPlotDataStyles
#define XtCXrtDimension              XtCPlotDimension
#define XtCXrtDimensionUseDefault    XtCPlotDimensionUseDefault
#define XtCXrtDrawOption             XtCPlotDrawOption
#define XtCXrtForegroundColor        XtCPlotForegroundColor
#define XtCXrtGraphMargin            XtCPlotGraphMargin
#define XtCXrtGraphMarginUseDefault  XtCPlotGraphMarginUseDefault
#define XtCXrtPosition               XtCPlotPosition
#define XtCXrtPositionUseDefault     XtCPlotPositionUseDefault 
#define XtCXrtStrings                XtCPlotStrings


/* map data type */
#define XrtAdjust PlotAdjust
#define XrtAlign PlotAlign
#define XrtAnnoMethod PlotAnnoMethod
#define XrtArrayData PlotArrayData
#define XrtArray PlotArray
#define XrtBorder PlotBorder
#define XrtData PlotData
#define XrtDataHandle PlotDataHandle
#define XrtDataStyle PlotDataStyle
#define XrtDataType PlotDataType
#define XrtGeneral PlotGeneral
#define XrtGeneralData PlotGeneralData
#define XrtMapResult PlotMapResult
#define XrtRegion PlotRegion
#define XrtTextDesc PlotTextDesc
#define XrtTextHandle PlotTextHandle
#define XrtTextPosition PlotTextPosition
#define XrtTimeUnit PlotTimeUnit

/* macro */
#define XRT_DATA_ARRAY PLOT_DATA_ARRAY
#define XRT_DATA_GENERAL PLOT_DATA_GENERAL
#define XRT_GENERAL XRT_DATA_GENERAL
#define XRT_ARRAY XRT_DATA_ARRAY

#define XRT_HUGE_VAL PLOT_HUGE_VAL 

#define	XRT_PLOT	PLOT_PLOT
#define	XRT_BAR		PLOT_BAR
#define	XRT_PIE		PLOT_PIE
#define	XRT_STACKING_BAR	PLOT_STACKING_BAR
#define	XRT_AREA	PLOT_AREA

#define XRT_TEXT_ATTACH_PIXEL PLOT_TEXT_ATTACH_PIXEL 
#define XRT_TEXT_ATTACH_VALUE PLOT_TEXT_ATTACH_VALUE
#define XRT_TEXT_ATTACH_DATA PLOT_TEXT_ATTACH_DATA
#define	XRT_TEXT_ATTACH_DATA_VALUE PLOT_TEXT_ATTACH_DATA_VALUE

#define XRT_BORDER_NONE PLOT_BORDER_NONE 
#define XRT_BORDER_3D_OUT PLOT_BORDER_3D_OUT
#define XRT_BORDER_3D_IN PLOT_BORDER_3D_IN
#define XRT_BORDER_SHADOW PLOT_BORDER_SHADOW
#define XRT_BORDER_PLAIN PLOT_BORDER_PLAIN
#define XRT_BORDER_ETCHED_IN PLOT_BORDER_ETCHED_IN
#define XRT_BORDER_ETCHED_OUT PLOT_BORDER_ETCHED_OUT

#define XRT_ADJUST_LEFT PLOT_ADJUST_LEFT 
#define XRT_ADJUST_RIGHT PLOT_ADJUST_RIGHT
#define XRT_ADJUST_CENTER PLOT_ADJUST_CENTER 

#define XRT_ANCHOR_NORTH PLOT_ANCHOR_NORTH
#define XRT_ANCHOR_SOUTH  PLOT_ANCHOR_SOUTH
#define XRT_ANCHOR_EAST	PLOT_ANCHOR_EAST		
#define XRT_ANCHOR_WEST	 PLOT_ANCHOR_WEST
#define XRT_ANCHOR_NORTHEAST PLOT_ANCHOR_NORTHEAST
#define XRT_ANCHOR_NORTHWEST PLOT_ANCHOR_NORTHWEST
#define XRT_ANCHOR_SOUTHEAST PLOT_ANCHOR_SOUTHEAST
#define XRT_ANCHOR_SOUTHWEST PLOT_ANCHOR_SOUTHWEST
#define XRT_ANCHOR_HOME	PLOT_ANCHOR_HOME
#define XRT_ANCHOR_BEST PLOT_ANCHOR_BEST

#define XRT_RGN_IN_GRAPH PLOT_RGN_IN_GRAPH
#define XRT_RGN_IN_LEGEND PLOT_RGN_IN_LEGEND
#define XRT_RGN_IN_HEADER PLOT_RGN_IN_HEADER
#define XRT_RGN_IN_FOOTER PLOT_RGN_IN_FOOTER
#define XRT_RGN_NOWHERE PLOT_RGN_NOWHERE

#define XRT_ANNO_VALUES PLOT_ANNO_VALUES
#define XRT_ANNO_POINT_LABELS PLOT_ANNO_POINT_LABELS
#define XRT_ANNO_VALUE_LABELS PLOT_ANNO_VALUE_LABELS
#define XRT_ANNO_TIME_LABELS PLOT_ANNO_TIME_LABELS

#define XRT_TMUNIT_SECONDS PLOT_TMUNIT_SECONDS
#define XRT_TMUNIT_MINUTES PLOT_TMUNIT_MINUTES
#define XRT_TMUNIT_HOURS PLOT_TMUNIT_HOURS
#define XRT_TMUNIT_DAYS PLOT_TMUNIT_DAYS
#define XRT_TMUNIT_WEEKS PLOT_TMUNIT_WEEKS
#define XRT_TMUNIT_MONTHS PLOT_TMUNIT_MONTHS
#define XRT_TMUNIT_YEARS PLOT_TMUNIT_YEARS


/* define line pattern */
#define XrtLinePattern AtPlotLineStyle
#define XRT_LPAT_NONE AtLineNONE
#define XRT_LPAT_SOLID AtLineSOLID
#define XRT_LPAT_DOTTED AtLineDOTTED4
#define XRT_LPAT_SHORT_DASH AtLineDASHED3
#define XRT_LPAT_LSL_DASH AtLineDASHED4
#define XRT_LPAT_LONG_DASH AtLineDASHED5
#define XRT_LPAT_DASH_DOT AtLineDOTDASHED2


/* define fill pattern */
#define XrtFillPattern AtPlotFillStyle
/*#define XRT_FPAT_NONE 0
#define XRT_FPAT_SOLID 1
#define XRT_FPAT_25_PERCENT 2
#define XRT_FPAT_50_PERCENT 3
#define XRT_FPAT_75_PERCENT 4
#define XRT_FPAT_HORIZ_STRIPE 5
#define XRT_FPAT_VERT_STRIPE 6
#define XRT_FPAT_45_STRIPE 7
#define XRT_FPAT_135_STRIPE 8
#define XRT_FPAT_DIAG_HATCHED 9
#define XRT_FPAT_CROSS_HATCHED 10
*/
#define XRT_FPAT_NONE PLOT_FPAT_NONE
#define XRT_FPAT_SOLID PLOT_FPAT_SOLID
#define XRT_FPAT_25_PERCENT PLOT_FPAT_25_PERCENT
#define XRT_FPAT_50_PERCENT PLOT_FPAT_50_PERCENT
#define XRT_FPAT_75_PERCENT PLOT_FPAT_75_PERCENT
#define XRT_FPAT_HORIZ_STRIPE PLOT_FPAT_HORIZ_STRIPE
#define XRT_FPAT_VERT_STRIPE PLOT_FPAT_VERT_STRIPE
#define XRT_FPAT_45_STRIPE PLOT_FPAT_45_STRIPE
#define XRT_FPAT_135_STRIPE PLOT_FPAT_135_STRIPE
#define XRT_FPAT_DIAG_HATCHED PLOT_FPAT_DIAG_HATCHED
#define XRT_FPAT_CROSS_HATCHED PLOT_FPAT_CROSS_HATCHED


/*define point style */
#define XrtPoint AtPlotMarkType
/*#define XRT_POINT_NONE 0
#define XRT_POINT_SQUARE 1
#define XRT_POINT_BOX 1
#define XRT_POINT_VERT_LINE 1
#define XRT_POINT_HORIZ_LINE 1
#define XRT_POINT_STAR 4
#define XRT_POINT_DIAMOND 5
#define XRT_POINT_DOT 6
#define XRT_POINT_CIRCLE 7
#define XRT_POINT_CROSS 8
#define XRT_POINT_TRI 9
*/
#define XRT_POINT_NONE AtMarkNONE
#define XRT_POINT_SQUARE AtMarkRECTANGLE
#define XRT_POINT_BOX AtMarkRECTANGLE
#define XRT_POINT_VERT_LINE AtMarkRECTANGLE
#define XRT_POINT_HORIZ_LINE AtMarkRECTANGLE
#define XRT_POINT_STAR AtMarkSTAR
#define XRT_POINT_DIAMOND AtMarkDIAMOND
#define XRT_POINT_DOT AtMarkDOT
#define XRT_POINT_CIRCLE AtMarkCIRCLE
#define XRT_POINT_CROSS AtMarkCROSS
#define XRT_POINT_TRI AtMarkTRIANGLE1

/* map function names */
#define XrtMakeDataFromFile PlotMakeDataFromFile
#define XrtMakeData PlotMakeData
#define XrtMap PlotMap
#define XrtFloatToArgVal PlotFloatToArgVal

#define XrtDestroyData PlotDestroyData
#define XrtDupDataStyles PlotDupDataStyles
#define XrtDupStrings PlotDupStrings
#define XrtFreeDataStyles PlotFreeDataStyles
#define XrtFreeStrings PlotFreeStrings

#define XrtTextCreate PlotTextCreate
#define XrtTextDetail PlotTextDetail
#define XrtTextUpdate PlotTextUpdate
#define XrtTextDetach PlotTextDetach
#define XrtTextAttach PlotTextAttach
#define XrtTextDestroy PlotTextDestroy
#define XrtGetTextHandles PlotGetTextHandles

#define XrtSetNthDataStyle PlotSetNthDataStyle
#define XrtSetNthDataStyle2 PlotSetNthDataStyle2

#define XrtDataCreate PlotDataCreate
#define XrtDataGetNPoints PlotDataGetNPoints
#define XrtDataGetXElement PlotDataGetXElement
#define XrtDataGetYElement PlotDataGetYElement
#define XrtDataDestroy PlotDataDestroy
#define XrtDataSetHole PlotDataSetHole
#define XrtDataSetNPoints PlotDataSetNPoints
#define XrtDataSetXElement PlotDataSetXElement
#define XrtDataSetYElement PlotDataSetYElement

#endif
