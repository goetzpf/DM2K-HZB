/****************************************************************************
 FILE:		BarGraph.h
 CONTENTS:	Public header file for the BarGraph widget.
 AUTHOR:	Paul D. Johnston
 Date		Action
 ---------	------------------------------------
 5/23/92	Changed the widget class name so that it is preceded
		by 'xc' with the first major word capitalized.
 4/15/92	Created.
 ****************************************************************************/

#ifndef __XC_BARGRAPH_H
#define __XC_BARGRAPH_H

/****** Superclass header */
#include "Value.h"

/****** Define widget resource names, classes, and representation types.
        Use these resource strings in your resource files */
#define XcNbarBackground	"barBackground"
#define XcNbarForeground	"barForeground"
#define XcNscaleColor		"scaleColor"
#define XcNscaleSegments	"scaleSegments"
#define XcCScaleSegments	"ScaleSegments"

#define XcNvalueVisible		"valueVisible"
#define XcCValueVisible		"ValueVisible"

#define XcNautoSegment          "autoSegment"
#define XcCAutoSegment          "AutoSegment"
#define XcNscaleFontList        "scaleFontList"
#define XcCScaleFontList        "ScaleFontList"
#define XcNscaleFontListNum     "scaleFontListNum"
#define XcCScaleFontListNum     "ScaleFontListNum"
#define XcNbarOnly              "barOnly"
#define XcCBarOnly              "BarOnly"

#define XcNmarkers              "markers"
#define XcCMarkers              "Markers"

#define XcNmarkersColors        "markersColors"
#define XcCMarkersColors        "MarkersColors"

#define XcNmarkersNum           "markersNum"
#define XcCMarkersNum           "MarkersNum"

#define XcNlogScale             "logScale"
#define XcCLogScale             "LogScale"


/****** Class record declarations */
extern WidgetClass xcBarGraphWidgetClass;

typedef struct _BarGraphClassRec *BarGraphWidgetClass;
typedef struct _BarGraphRec *BarGraphWidget;

/****** Widget functions */
extern void XcBGUpdateValue         Xc_PROTO((Widget w, XcVType *value));
extern void XcBGUpdateBarForeground Xc_PROTO((Widget w, unsigned long pixel));

#endif

