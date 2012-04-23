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
 *                              - using new screen update dispatch mechanism
 *                              - the strip chart is rewritten.
 * .03  09-12-95        vong    conform to c++ syntax
 *
 *****************************************************************************
*/

#include "dm2k.h"
#include <sys/time.h>
#include "getArchiveData.h"

#include <Xm/DrawingAP.h>
#include <Xm/DrawP.h>
#include <X11/keysym.h>

#define STRIP_MARGIN 0.18

typedef struct _StripChart {
  DlElement   * dlElement;            /* strip chart data */
  Record      * record[MAX_PENS];     /* array of data */
  UpdateTask  * updateTask;
  int           nChannels;             /* number of channels ( <= MAX_PENS) */
  Boolean       updateEnable;          /* strip chart update enable */

  /* strip chart data */

  Dimension     w;
  Dimension     h;

  /* these (X0,Y0), (X1,Y1) are relative to main window/pixmap */
  unsigned int    dataX0;            /* upper left corner - data region X */
  unsigned int    dataY0;            /* upper left corner - data region Y */
  unsigned int    dataX1;            /* lower right corner - data region X */
  unsigned int    dataY1;            /* lower right corner - data region Y */
  unsigned int    dataWidth;         /* width of data region */
  unsigned int    dataHeight;        /* height of data region */

  int             shadowThickness;
  double          timeInterval;          /* time for each pixel */
  double          nextAdvanceTime;       /* time for advance one pixel */
  Pixmap          pixmap;
  GC              gc;
  double          value[MAX_PENS];
  double          maxVal[MAX_PENS];
  double          minVal[MAX_PENS];
  int             nextSlot;
  XtIntervalId    timerid;
} StripChart;

typedef struct {
  double hi;
  double lo;
  unsigned int numDot;
  unsigned int mask;
  char format;
  int decimal;
  int width;
  double value;
  double step;
} Range;

typedef struct {
  int axisLabelFont;
  int axisLabelFontHeight;
  int xAxisLabelWidth;
  int xAxisLabelHeight;
  int yAxisLabelWidth;
  int yAxisLabelHeight;
  int titleFont;
  int titleFontHeight;
  int margin;
  int markerHeight;
  int numYAxisLabel;
  int lineSpace;
  int yAxisLabelOffset;
  int yAxisLabelTextWidth;
  int shadowThickness;
} StripChartConfigData;

char *stripChartWidgetName = "stripChart";
static void stripChartDraw(XtPointer cd);
static void stripChartUpdateTaskCb(XtPointer cd);
static void stripChartUpdateValueCb(XtPointer cd);
static void stripChartUpdateGraphicalInfoCb(XtPointer cd);
#if 0
static void stripChartDestroyCb(XtPointer cd);
#endif
static void redisplayStrip(Widget, XtPointer, XtPointer);
static void stripChartUpdateGraph(XtPointer);
static StripChart *stripChartAlloc(DisplayInfo *,DlElement *, UpdateTask **);
static void freeStripChart(XtPointer);
static void stripChartName(XtPointer, char **, short *, int *);
static void configStripChart(XtPointer, XtIntervalId *);
static void stripChartInheritValues(ResourceBundle *pRCB, DlElement *p);
static void stripChartGetValues(ResourceBundle *pRCB, DlElement *p);
static void stripChartDrawArchiveData(StripChart * psc);

static char* titleStr = "Strip Chart";
static Range range[MAX_PENS];
static Range nullRange = {0.0, 0.0, 0, 0, 0, 0, 0, 0.0, 0.0};
static StripChartConfigData sccd;

static void destroyDlElement (DlElement * element);
static UpdateTask * executeMethod (DisplayInfo * ,DlElement *);

static DlDispatchTable stripChartDlDispatchTable = {
         createDlStripChart,
         destroyDlElement,
         executeMethod,
         writeDlStripChart,
         NULL,
         stripChartGetValues,
         stripChartInheritValues,
         NULL,
         NULL,
         genericMove,
         genericScale,
         NULL};

static void destroyDlStripChart(DlStripChart *dlStripChart)
{
  int penNumber;

  if (dlStripChart == NULL)
    return;

  objectAttributeDestroy(&(dlStripChart->object));
  plotcomAttributeInit(&(dlStripChart->plotcom));

  for (penNumber = 0; penNumber < MAX_PENS; penNumber++)
    penAttributeInit((DlPen *)&(dlStripChart->pen[penNumber]));

  free ((char *)dlStripChart);
}

static void destroyDlElement(register DlElement *dlElement) 
{
  destroyDlStripChart(dlElement->structure.stripChart);
   free ((char *)dlElement);
}

static int calcLabelFontSize(StripChart *psc) {
  int width;
  int fontHeight;
  int i;

  width = (psc->w < psc->h) ? psc->w : psc->h;
  if (width > 1000) {
    fontHeight = 18;
  } else
  if (width > 900) {
    fontHeight = 16;
  } else
  if (width > 750) {
    fontHeight = 14;
  } else
  if (width > 600) {
    fontHeight = 12;
  } else
  if (width > 400) {
    fontHeight = 10;
  } else {
    fontHeight = 8;
  }
  for (i = MAX_FONTS - 1; i >= 0 ; i--) {
    if ((fontTable[i]->ascent + fontTable[i]->descent) <= fontHeight) {
      break;
    }
  }
  if (i<0) i = 0;
  return i;
}

static int calcTitleFontSize(StripChart *psc) {
  int width;
  int fontHeight;
  int i;

  width = (psc->w < psc->h) ? psc->w : psc->h;
  if (width > 1000) {
    fontHeight = 26;
  } else
  if (width > 900) {
    fontHeight = 24;
  } else
  if (width > 750) {
    fontHeight = 22;
  } else
  if (width > 600) {
    fontHeight = 20;
  } else
  if (width > 500) {
    fontHeight = 18;
  } else
  if (width > 400) {
    fontHeight = 16;
  } else
  if (width > 300) {
    fontHeight = 14;
  } else
  if (width > 250) {
    fontHeight = 12;
  } else
  if (width > 200) {
    fontHeight = 10;
  } else {
    fontHeight = 8;
  }
  for (i = MAX_FONTS - 1; i >= 0 ; i--) {
    if ((fontTable[i]->ascent + fontTable[i]->descent) <= fontHeight) {
      break;
    }
  }
  if (i<0) i = 0;
  return i;
}

static void calcFormat(double value, char *format, int *decimal, int *width) {
  double order = 0.0;
  if (value == 0.0) {
    *format = 'f';
    *decimal = 1;
    *width = 3;
    return;
  } else {
    order = log10(fabs(value));
  }
  if (order > 5.0 || order < -4.0) {
    *format = 'e';
    *decimal = 1;
  } else {
    *format = 'f';
    if (order < 0.0) {
      *decimal = (int) (order) * -1 + 2;
    } else {
      *decimal = 1;
    }
  }
  if (order >= 4.0) {
    *width = 7;
  } else
  if (order >= 3.0) {
    *width = 6;
  } else
  if (order >= 2.0) {
    *width = 5;
  } else
  if (order >= 1.0) {
    *width = 4;
  } else
  if (order >= 0.0) {
    *width = 3;
  } else
  if (order >= -1.0) {
    *width = 4;
  } else
  if (order >= -2.0) {
    *width = 5;
  } else
  if (order >= -3.0) {
    *width = 6;
  } else {
    *width = 7;
  }
}

static int calcMargin(StripChart *psc) {
  int width;
  int margin;
  width = (psc->h < psc->w) ? psc->h : psc->w;
  if (width > 1000) {
    margin = 6;
  } else
  if (width > 800) {
    margin = 5;
  } else
  if (width > 600) {
    margin = 4;
  } else
  if (width > 400) {
    margin = 3;
  } else
  if (width > 300) {
    margin = 2;
  } else
  if (width > 200) {
    margin = 1;
  } else {
    margin = 0;
  }
  return margin;
}

static int calcMarkerHeight(StripChart *psc) {
  int width;
  int markerHeight;
  width = (psc->h < psc->w) ? psc->h : psc->w;
  if (width > 1000) {
    markerHeight = 6;
  } else
  if (width > 800) {
    markerHeight = 5;
  } else
  if (width > 600) {
    markerHeight = 4;
  } else
  if (width > 400) {
    markerHeight = 3;
  } else
  if (width > 300) {
    markerHeight = 2;
  } else {
    markerHeight = 1;
  }
  return markerHeight;
}

static void calcYAxisLabelWidth(StripChart *psc) {
  int i;
  int cnt;
  int maxWidth = 0;
  int maxDot = 0;

  sccd.axisLabelFont = calcLabelFontSize(psc);
  sccd.axisLabelFontHeight = fontTable[sccd.axisLabelFont]->ascent +
                             fontTable[sccd.axisLabelFont]->descent;
  sccd.markerHeight = calcMarkerHeight(psc);
  sccd.lineSpace = 3;
  sccd.margin = calcMargin(psc);;
  sccd.shadowThickness = 3;

  /* clear ranges */
  for (i = 0; i<psc->nChannels; i++) {
    range[i] = nullRange;
  }
  /* remove any duplicated settings */

  cnt = 0;

  for (i = 0; i < psc->nChannels; i++) {
    int found = 0;
    int j;

    for (j=0; j<cnt; j++) {
      if ((psc->record[i]->hopr == range[j].hi) &&
          (psc->record[i]->lopr == range[j].lo)) {
        found = 1;
        range[j].numDot++;
        range[j].mask = range[j].mask | (0x0001 << i);
        break;
      }
    }

    if (!found) {
      char f1, f2;
      int  d1, d2;
      int  w1, w2;

      range[cnt].hi = psc->record[i]->hopr;
      range[cnt].lo = psc->record[i]->lopr;
      range[cnt].mask = range[cnt].mask | (0x0001 << i);
      range[cnt].numDot = 1;
      calcFormat(range[cnt].hi, &f1, &d1, &w1);
      calcFormat(range[cnt].lo, &f2, &d2, &w2);
      range[cnt].format = (f1 == 'e' || f2 == 'e') ? 'e' : 'f';
      range[cnt].decimal = (d1 > d2) ? d1 : d2;
      if (range[cnt].hi < 0.0) w1++;
      if (range[cnt].lo < 0.0) w2++;
      range[cnt].width = (w1 > w2) ? w1 : w2;
      cnt++;
    }
  }
  sccd.numYAxisLabel = cnt;
  if (sccd.numYAxisLabel == 1) {
    range[0].numDot = 0;
  }
  for (i = 0; i < sccd.numYAxisLabel; i++) {
    int width;
    char *text = "-8.8888888888";

    width = XTextWidth(fontTable[sccd.axisLabelFont],text,range[i].width);
    if (width > maxWidth)
      maxWidth = width;
    if (range[i].numDot > maxDot) maxDot = range[i].numDot;
  }
  sccd.yAxisLabelWidth = maxWidth + (maxDot) * sccd.lineSpace;
  sccd.yAxisLabelTextWidth = maxWidth;
}

static void calcTitleHeight(StripChart *psc) {
  sccd.titleFont = calcTitleFontSize(psc);
  sccd.titleFontHeight = fontTable[sccd.titleFont]->ascent +
                             fontTable[sccd.titleFont]->descent;
}

static void calcXAxisLabelWidth(StripChart *psc) {
  char format;
  int decimal;
  int width;

  calcFormat(psc->dlElement->structure.stripChart->period,
             &format, &decimal, &width);
  width = width + 1;
  sccd.xAxisLabelWidth = XTextWidth(fontTable[sccd.axisLabelFont],
                             "-0.0000000",width) + psc->dataWidth;
}

static void calcYAxisLabelHeight(StripChart *psc) {
  sccd.yAxisLabelHeight = sccd.axisLabelFontHeight * sccd.numYAxisLabel
                          + psc->dataHeight;
}

static StripChart *stripChartAlloc
   (DisplayInfo * displayInfo,
    DlElement   * dlElement,
    UpdateTask ** updateTask) 
{
  StripChart *psc;
  DlStripChart *dlStripChart = dlElement->structure.stripChart;

  *updateTask = NULL;

  psc = (StripChart *) calloc(1,sizeof(StripChart));

  if (psc == NULL) return psc;

  psc->w = dlStripChart->object.width;
  psc->h = dlStripChart->object.height;
  psc->dataX0 =  (int) (STRIP_MARGIN*psc->w);
  psc->dataY0 =  (int) (STRIP_MARGIN*psc->h);
  psc->dataX1 =  (int) ((1.0 - STRIP_MARGIN)*psc->w);
  psc->dataY1 =  (int) ((1.0 - STRIP_MARGIN)*psc->h);
  psc->dataWidth = psc->dataX1 - psc->dataX0;
  psc->dataHeight = psc->dataY1 - psc->dataY0;

  psc->updateEnable = False;
  psc->dlElement = dlElement;
  psc->pixmap = (Pixmap) NULL;
  psc->nextAdvanceTime = dm2kTime();
  psc->timerid = (XtIntervalId)0;

  switch(dlStripChart->units) {
  case MILLISECONDS:
    psc->timeInterval =
         dlStripChart->period * 0.001 / (double) psc->dataWidth;
    break;
  case SECONDS:
    psc->timeInterval =
         dlStripChart->period / (double)psc->dataWidth;
    break;
  case MINUTES:
    psc->timeInterval =
         dlStripChart->period * 60 / (double) psc->dataWidth;
    break;
  default:
    dm2kPrintf("\nexecuteDlStripChart : unknown time unit\n");
    psc->timeInterval = 60/ (double) psc->dataWidth;
    break;
  }

  *updateTask = psc->updateTask =
      updateTaskAddTask(displayInfo, &(dlStripChart->object),
                        stripChartUpdateTaskCb,
                        (XtPointer)psc);
  if (psc->updateTask == NULL) {
    dm2kPrintf("memory allocation error at executeDlStripChart\n");
  } else {
    updateTaskAddDestroyCb(psc->updateTask,freeStripChart);
    updateTaskAddNameCb(psc->updateTask,stripChartName);
  }
  return psc;
}

static void freeStripChart(XtPointer cd) {
  StripChart *psc = (StripChart *) cd;
  int i;

  if(psc->timerid) {
     XtRemoveTimeOut(psc->timerid);
     psc->timerid=0;
  }
 
  if (psc == NULL) return;
  for (i = 0; i < psc->nChannels; i++) {
    dm2kDestroyRecord(psc->record[i]);
  }
  if (psc->pixmap) {
    XFreePixmap(XtDisplay(psc->dlElement->widget),psc->pixmap);
    psc->pixmap = (Pixmap) NULL;
    XFreeGC(XtDisplay(psc->dlElement->widget),psc->gc);
    psc->gc = NULL;
  }
  free((char *)psc);
  psc = NULL;
}


static void stripChartConfig(StripChart *psc) {
  DlStripChart *dlStripChart = psc->dlElement->structure.stripChart;
  DisplayInfo *displayInfo = psc->updateTask->displayInfo;
  Display *display = XtDisplay(psc->dlElement->widget);
  Widget widget = psc->dlElement->widget;
  GC gc; 
  int i;
  int width;
  int height;
  int dropXAxisUnitLabel = False;
  int dropYAxisUnitLabel = False;
  int dropTitleLabel = False;
  int squeezeSpace = False;
  int widthExt;
  int heightExt;

  calcYAxisLabelWidth(psc);

  /* use the width of y-axis label to set all margins */
  width = sccd.shadowThickness + sccd.yAxisLabelWidth + (sccd.margin) * 2 + sccd.markerHeight + 1;
  psc->dataX0 = width;
  psc->dataY0 = width;
  psc->dataX1 = psc->w - psc->dataX0 - 1;
  psc->dataY1 = psc->h - psc->dataY0 - 1;
  psc->dataWidth = psc->dataX1 - psc->dataX0 + 1;
  psc->dataHeight = psc->dataY1 - psc->dataY0 + 1;

  calcTitleHeight(psc);

  /* set the height of the top margin, shrink the height of right and bottom if applicable. */
  height = sccd.shadowThickness + sccd.titleFontHeight + sccd.axisLabelFontHeight + sccd.margin * 3 + 1;
  psc->dataY0 = height;
  psc->dataHeight = psc->dataY1 - psc->dataY0 + 1;
  if (psc->dataY0 < psc->dataX0) {
    /* shrink the margins */
    psc->dataX1 = psc->w - psc->dataY0 - 1;
    psc->dataY1 = psc->h - psc->dataY0;
    psc->dataWidth = psc->dataX1 - psc->dataX0 + 1;
    psc->dataHeight = psc->dataY1 - psc->dataY0 + 1;
  }

  /* calc the height of the bottom margin, expands if necessary */
  height = sccd.shadowThickness + sccd.axisLabelFontHeight * 2 + sccd.margin * 3 + sccd.markerHeight + 2;
  if ((psc->h - psc->dataY1 - 1) < height) {
    psc->dataY1 = psc->h - height - 1;
    psc->dataHeight = psc->dataY1 - psc->dataY0 + 1;
  }
  if ((psc->w - psc->dataX1 - 1) < height) {
    psc->dataX1 = psc->w - height - 1;
    psc->dataWidth = psc->dataX1 - psc->dataX0 + 1;
  }

  calcYAxisLabelHeight(psc);

  /* make sure the height of y-axis label won't bigger than the height of the graph allowed */
  heightExt = (sccd.yAxisLabelHeight - psc->dataHeight + 1) / 2;
  if (heightExt + sccd.shadowThickness > psc->dataY0) {
    psc->dataY0 = heightExt + sccd.shadowThickness;
    psc->dataHeight = psc->dataY1 - psc->dataY0 + 1;
  }
  if (heightExt + sccd.shadowThickness > (psc->h - psc->dataY1)) {
    psc->dataY1 = psc->h - heightExt - sccd.shadowThickness - 1;
    psc->dataHeight = psc->dataY1 - psc->dataY0 + 1;
  }

  calcXAxisLabelWidth(psc);

  /* make sure the x-axis and y-axis labels not overlapping each other */
  widthExt = (sccd.xAxisLabelWidth - psc->dataWidth + 1) / 2;

  if ((widthExt > (sccd.margin + sccd.markerHeight))
      && (height > (sccd.margin + sccd.markerHeight))) {
    /* shrink the graph */
    sccd.yAxisLabelOffset = widthExt - sccd.margin - sccd.markerHeight;
    psc->dataX0 = psc->dataX0 + sccd.yAxisLabelOffset ;
    psc->dataWidth = psc->dataX1 - psc->dataX0 + 1;
  }

  /* make sure the width of x-axis label won't bigger than the widht of the graph allowed */
  if (widthExt + sccd.shadowThickness > (psc->w - psc->dataX1 - 1)) {
    psc->dataX1 = psc->w - widthExt - sccd.shadowThickness - 1;
    psc->dataWidth = psc->dataX1 - psc->dataX0 + 1;
  }

  /* do the second pass */
  /* if the graph is too small do somethings */
  if (psc->dataHeight < psc->h / 2) {
    squeezeSpace = True;
    if ((dlStripChart->plotcom.title == NULL) 
      || (STRLEN(dlStripChart->plotcom.title) == 0)) {
      dropTitleLabel = True;
      psc->dataY0 -= sccd.titleFontHeight;
      psc->dataHeight = psc->dataY1 - psc->dataY0 + 1;
      if ((psc->w - psc->dataX1 - 1) > psc->dataY0) {
        psc->dataX1 = psc->w - psc->dataY0 - 1;
        psc->dataWidth = psc->dataX1 - psc->dataX0 + 1;
      }
    }
    if ((dlStripChart->plotcom.xlabel == NULL) 
      || (STRLEN(dlStripChart->plotcom.xlabel) == 0)) {
      dropXAxisUnitLabel = True;
      psc->dataY1 += sccd.axisLabelFontHeight;
      psc->dataHeight = psc->dataY1 - psc->dataY0 + 1;
      if ((psc->w - psc->dataX1 - 1) > (psc->h - psc->dataY1 - 1)) {
        psc->dataX1 = psc->w - (psc->h - psc->dataY1 - 1) - 1;
        psc->dataWidth = psc->dataX1 - psc->dataX0 + 1;
      }
    }
    if ((dlStripChart->plotcom.ylabel == NULL) 
      || (STRLEN(dlStripChart->plotcom.ylabel) == 0)) {
      dropYAxisUnitLabel = True;
      psc->dataY0 -= sccd.axisLabelFontHeight;
      psc->dataHeight = psc->dataY1 - psc->dataY0 + 1;
      if ((psc->w - psc->dataX1 - 1) > psc->dataY0) {
        psc->dataX1 = psc->w - psc->dataY0 - 1;
        psc->dataWidth = psc->dataX1 - psc->dataX0 + 1;
      }
    }
    /* make sure the height of y-axis label won't bigger than the height of the graph allowed */
    if (heightExt + sccd.shadowThickness > psc->dataY0) {
      psc->dataY0 = heightExt + sccd.shadowThickness;
      psc->dataHeight = psc->dataY1 - psc->dataY0 + 1;
    }
    if (heightExt + sccd.shadowThickness> (psc->h - psc->dataY1)) {
      psc->dataY1 = psc->h - heightExt- sccd.shadowThickness - 1;
      psc->dataHeight = psc->dataY1 - psc->dataY0 + 1;
    }
    /* make sure the width of x-axis label won't bigger than the widht of the graph allowed */
    if (widthExt + sccd.shadowThickness > (psc->w - psc->dataX1 - 1)) {
      psc->dataX1 = psc->w - widthExt - sccd.shadowThickness - 1;
      psc->dataWidth = psc->dataX1 - psc->dataX0 + 1;
    }
  }

  /* make sure the size is bigger than zero */
  if (psc->dataX1 <= psc->dataX0) {
    psc->dataWidth = 2;
    psc->dataX1 = psc->dataX0 + 1;
  }
  if (psc->dataY1 <= psc->dataY0) {
    psc->dataHeight = 2;
    psc->dataY1 = psc->dataY0 + 1;
  }

  psc->pixmap =
    XCreatePixmap(display,XtWindow(widget),
                  psc->w, psc->h,
                  DefaultDepth(XtDisplay(widget),
                  DefaultScreen(XtDisplay(widget))));
  psc->gc = XCreateGC(display,XtWindow(widget),
                       (unsigned long)NULL,NULL);
  gc = psc->gc;

  /* fill the background */
  XSetForeground(display,gc,displayInfo->colormap[dlStripChart->plotcom.bclr]);
  XFillRectangle(display,psc->pixmap,gc,0,0,
                 psc->w, psc->h);
  /* draw the rectangle which enclosed the graph */
  XSetForeground(display,gc,displayInfo->colormap[dlStripChart->plotcom.clr]);
  XSetLineAttributes(display,gc,0,LineSolid,CapButt,JoinMiter);
  XDrawRectangle(display,psc->pixmap,gc,
                 psc->dataX0-1, psc->dataY0-1,
                 psc->dataWidth+1,psc->dataHeight+1);

  /* draw title */
  if (dlStripChart->plotcom.title) {
    int textWidth;
    char *title = dlStripChart->plotcom.title;
    int strLen = STRLEN(title);

    textWidth = XTextWidth(fontTable[sccd.titleFont],title,strLen);
    XSetForeground(display,gc,displayInfo->colormap[dlStripChart->plotcom.clr]);
    XSetFont(display,gc,fontTable[sccd.titleFont]->fid);
    XDrawString(display,psc->pixmap,gc,
                ((int)psc->w - textWidth)/2,
                sccd.shadowThickness + sccd.margin + fontTable[sccd.titleFont]->ascent,
                title,strLen);
  }

  /* draw y-axis label */
  if (dlStripChart->plotcom.ylabel) {
    int textWidth;
    char *label = dlStripChart->plotcom.ylabel;
    int strLen = STRLEN(label);
    int x, y;

    textWidth = XTextWidth(fontTable[sccd.axisLabelFont],label,strLen);
    XSetForeground(display,gc,displayInfo->colormap[dlStripChart->plotcom.clr]);
    XSetFont(display,gc,fontTable[sccd.axisLabelFont]->fid);
    x = psc->dataX0;
    y = sccd.shadowThickness + sccd.margin*2 + sccd.titleFontHeight + fontTable[sccd.axisLabelFont]->ascent;
    XDrawString(display,psc->pixmap,gc,x,y,label,strLen);
  }

  /* draw y-axis scale */
  XSetFont(display,gc,fontTable[sccd.axisLabelFont]->fid);
  {
    int i;
    int labelHeight;
    int nDiv;
    Pixel fg;
    double interval;
    double nextTick;
    double verticalSpacing; 

    /* calculate how many divisions are needed */
    if (squeezeSpace) {
      verticalSpacing = 1.0;
    } else {
      verticalSpacing = 2.0;
    }  
    labelHeight = (int) (((double)sccd.numYAxisLabel + verticalSpacing)
                  * (double) sccd.axisLabelFontHeight);
    nDiv = (psc->dataHeight - 1) / labelHeight;
    if (nDiv > 10) {
      nDiv = 10;
    } else
    if (nDiv == 9) {
      nDiv = 8;
    } else
    if (nDiv == 7) {
      nDiv = 6;
    }

    interval = (double) (psc->dataHeight - 1) / (double) nDiv;
    nextTick = 0.0;

    for (i=0; i<sccd.numYAxisLabel; i++) {
      range[i].step = (range[i].hi - range[i].lo)/(double)nDiv;
      range[i].value = range[i].hi;
    }
    fg = displayInfo->colormap[dlStripChart->plotcom.clr];
    for (i=0; i< nDiv + 1; i++) {
      int j;
      for (j = 0; j<sccd.numYAxisLabel; j++) {
        double yoffset;
        int w;
        int x, y;
        char text[20];
        int strLen;

        if (i == nDiv) {
          range[j].value = range[j].lo;
        }
        if (range[j].format == 'e') {
          sprintf(text,"%.1e",range[j].value);
        } else {
          sprintf(text,"%.*f",range[j].decimal,range[j].value);
        }
        strLen = STRLEN(text);
        XSetForeground(display, gc, fg);
        w = XTextWidth(fontTable[sccd.axisLabelFont],text,strLen);
        x = psc->dataX0 - 1 - sccd.markerHeight - sccd.yAxisLabelOffset- sccd.margin - w;
        yoffset = nextTick - (double) labelHeight / 2 + ((double)j + verticalSpacing/2.0) *
                  sccd.axisLabelFontHeight +
                  (double) fontTable[sccd.axisLabelFont]->ascent;
        y = psc->dataY0 + (int) yoffset;
        XDrawString(display,psc->pixmap,gc,x,y,text,strLen);
        range[j].value = range[j].value - range[j].step;

        if (range[j].numDot > 0) {   
          int k;
          int count = 0;
          for (k = MAX_PENS - 1; k >= 0; k--) {
            if (range[j].mask & (0x1 << k)) {
              XSetForeground(display,gc,displayInfo->colormap[dlStripChart->pen[k].clr]);
              x = psc->dataX0 - sccd.yAxisLabelTextWidth - sccd.markerHeight
                  - sccd.margin - sccd.yAxisLabelOffset;
              XFillRectangle(display, psc->pixmap, gc,
                  x-(count+1)*sccd.lineSpace, y-fontTable[sccd.axisLabelFont]->ascent,
                  2, fontTable[sccd.axisLabelFont]->ascent);
#if 0
              XDrawLine(display, psc->pixmap, gc,
                  x-count*sccd.lineSpace-1, y-fontTable[sccd.axisLabelFont]->ascent,
                  x-count*sccd.lineSpace-1, y);
#endif

              count++;
            }  
          } 
        }
      }
      XSetForeground(display, gc, fg);
      XDrawLine(display, psc->pixmap, gc,
        psc->dataX0 - 2 - (sccd.markerHeight - 1), psc->dataY0 + (int) nextTick,
        psc->dataX0 - 2, psc->dataY0 + (int) nextTick);
      nextTick = nextTick + interval;
    }
  }

  /* calculate the value time/pixel */
  switch (dlStripChart->units) {
    case MILLISECONDS:
    psc->timeInterval =
         dlStripChart->period * 0.001 / (double) psc->dataWidth;
    break;
  case SECONDS:
    psc->timeInterval =
         dlStripChart->period / (double)psc->dataWidth;
    break;
  case MINUTES:
    psc->timeInterval =
         dlStripChart->period * 60 / (double) psc->dataWidth;
    break;
  default:
    dm2kPrintf("\nexecuteDlStripChart : unknown time unit\n");
    psc->timeInterval = 60/ (double) psc->dataWidth;
    break;
  }

  /* draw x-axis label and scale */
  {
    int i;
    char text[10];
    int  tw;
    char format;
    int  decimal;
    int  width;
    int  nDiv;
    double step;
    double value;
    double nextTick;
    int x, y;
  
    calcFormat(dlStripChart->period, &format, &decimal, &width);
    width = width + 1;   /* add the plus size */
    tw = XTextWidth(fontTable[sccd.axisLabelFont],"-0.000000",width);
    nDiv = (psc->dataWidth - 1)/tw;
    if (nDiv > 10) {
      nDiv = 10;
    } else
    if (nDiv == 9) {
      nDiv = 8;
    } else
    if (nDiv == 7) {
      nDiv = 6;
    }
    nextTick = 0.0;

    step = dlStripChart->period/nDiv;
    value = 0.0;
    y = psc->dataY1 + 1 + sccd.axisLabelFontHeight + sccd.margin + sccd.markerHeight + 1;

    XSetForeground(display, gc, displayInfo->colormap[dlStripChart->plotcom.clr]);
    for (i=0; i< nDiv + 1; i++) {
      double xoffset;
      int w;
      int strLen;
      nextTick = (double) (psc->dataWidth - 1) * ((double) i / (double) nDiv);
      if (format == 'e') {
        sprintf(text,"%.1e",value);
      } else {
        sprintf(text,"%.*f",decimal,value);
      }
      strLen = STRLEN(text);
      w = XTextWidth(fontTable[sccd.axisLabelFont],text,strLen);
      xoffset = nextTick + (double) w / 2.0;
      x = psc->dataX1 - (int) xoffset;
      XDrawLine(display, psc->pixmap, gc,
        psc->dataX1 - (int) nextTick, psc->dataY1 + 2,
        psc->dataX1 - (int) nextTick, psc->dataY1 + 2 + sccd.markerHeight);
      XDrawString(display,psc->pixmap,gc,x,y,text,strLen);
      value = value - step;
    }

    /* determine the x-axis label */
    if (dropXAxisUnitLabel == False) {
      int strLen;
      int textWidth;
      char *label;

      if ((dlStripChart->plotcom.xlabel)
           && (STRLEN(dlStripChart->plotcom.xlabel) > (size_t) 0)) {
        label = dlStripChart->plotcom.xlabel;
      } else {
        switch (dlStripChart->units) {
          case MILLISECONDS :
            label = "time (ms)";
            break;
          case SECONDS :
            label = "time (sec)";
            break;
          case MINUTES :
            label = "time (min)";
            break;
          default :
            label = "time (sec)";
            break;
        }
      }
      strLen = STRLEN(label);
      textWidth = XTextWidth(fontTable[sccd.axisLabelFont],label,strLen);

      x = psc->dataX0 + (psc->dataWidth - textWidth)/2;
      y = psc->dataY1 + 1 + sccd.axisLabelFontHeight + 
	sccd.margin * 2 + sccd.markerHeight +
          fontTable[sccd.axisLabelFont]->ascent + 1;

      XDrawString(display,psc->pixmap,gc,x,y,label,strLen);
    }

    
#if 0
    if (!((dlStripChart->plotcom.xlabel) && (STRLEN(dlStripChart->plotcom.xlabel) > 0))) {
      int strLen;
      int textWidth;
      /* print the timeInterval */
      sprintf(text,"time/pixel = %.3f sec",psc->timeInterval);
      strLen = STRLEN(text);
      textWidth = XTextWidth(fontTable[sccd.axisLabelFont],text,strLen);
      x = psc->dataX0 + (psc->dataWidth - textWidth)/2;
      y = psc->dataY1 + sccd.axisLabelFontHeight + sccd.margin * 2 + sccd.markerHeight +
          fontTable[sccd.axisLabelFont]->ascent;
      XDrawString(display,psc->pixmap,gc,x,y,text,strLen);
    }
#endif
  }


  {  /* draw the shadow */
      GC topGC = ((struct _XmDrawingAreaRec *)widget)->manager.top_shadow_GC;
      GC bottomGC = ((struct _XmDrawingAreaRec *)widget)->manager.bottom_shadow_GC;
      _XmDrawShadows(display,psc->pixmap,topGC,bottomGC,0,0,
		     dlStripChart->object.width,dlStripChart->object.height,
		     sccd.shadowThickness-1,XmSHADOW_OUT);
  }

  XCopyArea(display,psc->pixmap,XtWindow(widget), gc,
	    0,0,psc->w,psc->h,0,0);

  for (i = 0; i < psc->nChannels; i++) {
    psc->maxVal[i] = psc->minVal[i] = psc->value[i] = psc->record[i]->value;
  }

  psc->nextAdvanceTime = dm2kTime() + psc->timeInterval;

  /* specified the time interval 
   */
  updateTaskSetScanRate(psc->updateTask,psc->timeInterval);
  psc->updateEnable = True;

  stripChartDrawArchiveData(psc);
  stripChartDraw ((XtPointer)psc);
}

#ifdef __cplusplus
static void redisplayFakeStrip(Widget widget, XtPointer cd, XtPointer)
#else
static void redisplayFakeStrip(Widget widget, XtPointer cd, XtPointer cbs)
#endif
{

  DlStripChart *dlStripChart = (DlStripChart *) cd;
  int usedWidth, usedHeight;
  int i;
  DisplayInfo *displayInfo;

  if (!(displayInfo = dmGetDisplayInfoFromWidget(widget)))
     return;


  if (dlStripChart) {
    GC topGC;
    GC bottomGC;
    Dimension shadowThickness;
    int x, y, w, h;
    int textWidth;
    int strLen = STRLEN(titleStr);

    XSetLineAttributes(display,displayInfo->gc,0,
                                LineSolid,CapButt,JoinMiter);
    XSetForeground(display,displayInfo->gc,
        displayInfo->colormap[dlStripChart->plotcom.bclr]);
    XFillRectangle(display,XtWindow(widget),displayInfo->gc,
        0,0,dlStripChart->object.width,dlStripChart->object.height);
    XSetForeground(display,displayInfo->gc,
        displayInfo->colormap[dlStripChart->plotcom.clr]);
    x = (int)(STRIP_MARGIN*dlStripChart->object.width);
    y = (int)(STRIP_MARGIN*dlStripChart->object.height);
    w = (unsigned int) ((1-STRIP_MARGIN*2)*dlStripChart->object.width);
    h = (unsigned int) ((1-STRIP_MARGIN*2)*dlStripChart->object.height); 
    XDrawRectangle(display,XtWindow(widget),displayInfo->gc,x,y,w,h);
    i = dmGetBestFontWithInfo(fontTable,MAX_FONTS,titleStr,y,w,
           &usedHeight,&usedWidth,TRUE);
    textWidth = XTextWidth(fontTable[i],titleStr,strLen);
    x = (dlStripChart->object.width - textWidth)/2;
    XSetFont(display,displayInfo->gc,fontTable[i]->fid);
    XDrawString(display,XtWindow(widget),displayInfo->gc,
        x, (int)(0.15*dlStripChart->object.height),titleStr,strLen);
    XtVaGetValues(widget,
        XmNshadowThickness,&shadowThickness,
        NULL);
    topGC = ((struct _XmDrawingAreaRec *)widget)->manager.top_shadow_GC;
    bottomGC = ((struct _XmDrawingAreaRec *)widget)->manager.bottom_shadow_GC;
    _XmDrawShadows(XtDisplay(widget),XtWindow(widget),topGC,bottomGC,0,0,
        dlStripChart->object.width,dlStripChart->object.height,
        shadowThickness,XmSHADOW_OUT);
  }
}



static UpdateTask * executeMethod
   (DisplayInfo * displayInfo, 
    DlElement   * dlElement)
{
  int              n;
  Arg              args[15];
  Widget           localWidget;
  DlStripChart   * dlStripChart = dlElement->structure.stripChart;
  WidgetUserData * userData;
  UpdateTask * updateTask = NULL;

  /* alloc servise structures
   */
  userData = DM2KALLOC (WidgetUserData);

  if (userData == NULL) {
    dm2kPrintf("executeDlStripChart: memory allocation error\n");
    return updateTask;
  }
  
  if (dlElement->widget == NULL) 
  {
    /* create the drawing widget for the strip chart */
    n = 0;
    XtSetArg(args[n],XmNx,       (Position)dlStripChart->object.x); n++;
    XtSetArg(args[n],XmNy,       (Position)dlStripChart->object.y); n++;
    XtSetArg(args[n],XmNwidth,   (Dimension)dlStripChart->object.width); n++;
    XtSetArg(args[n],XmNheight,  (Dimension)dlStripChart->object.height); n++;

    XtSetArg(args[n],XmNmarginWidth,     0); n++;
    XtSetArg(args[n],XmNmarginHeight,    0); n++;
    XtSetArg(args[n],XmNshadowThickness, 2); n++;

    XtSetArg(args[n],XmNforeground,(Pixel)
	     displayInfo->colormap[dlStripChart->plotcom.clr]); n++;
    XtSetArg(args[n],XmNbackground,(Pixel)
	     displayInfo->colormap[dlStripChart->plotcom.bclr]); n++;
    XtSetArg(args[n],XmNtraversalOn,False); n++;

    dlElement->widget = localWidget = 
      XmCreateDrawingArea(displayInfo->drawingArea,
			  stripChartWidgetName,args,n);

    /* if execute mode, create stripChart and setup all the channels 
     */
    if (displayInfo->traversalMode == DL_EXECUTE) 
    {
      int          i,j;
      StripChart * psc;

      psc = stripChartAlloc(displayInfo,dlElement, &updateTask);
      if (psc == NULL) {
        dm2kPrintf("memory allocation error at executeDlStripChart\n");
	DM2KFREE (userData);
        return updateTask;
      }

      /* connect channels */
      j = 0;
      for (i = 0; i < MAX_PENS; i++) {
        if ((dlStripChart->pen[i].chan != NULL) &&
	    dlStripChart->pen[i].chan[0]
	   ) {
          psc->record[j] = dm2kAllocateRecord(dlStripChart->pen[i].chan,
                   stripChartUpdateValueCb,
                   stripChartUpdateGraphicalInfoCb,
                   (XtPointer) psc);
          j++;
        }
      }
      /* record the number of channels in the strip chart */
      psc->nChannels = j;

      if (psc->nChannels == 0) {
        /* if no channel, create a fake channel */
        psc->nChannels = 1;
        psc->record[0] = dm2kAllocateRecord(" ",
					    NULL,
					    stripChartUpdateGraphicalInfoCb,
					    (XtPointer) psc);
      }

      userData->privateData    = (char*) psc;
      userData->updateTask = psc->updateTask;

      XtVaSetValues(localWidget, XmNuserData, (XtPointer) userData, NULL);
      XtAddCallback(localWidget, XmNexposeCallback, redisplayStrip,
		    (XtPointer)psc);

      /* destroy callback should free allocated memory
       */
      XtAddCallback(localWidget,XmNdestroyCallback,
		    (XtCallbackProc)freeUserDataCB, NULL);

      /* add in drag/drop translations 
       */
      XtOverrideTranslations(localWidget,parsedTranslations);

      /* fill in drawing area
       */
      drawWhiteRectangle(psc->updateTask);

    } 
    else if (displayInfo->traversalMode == DL_EDIT) 
    {

      XtAddCallback(localWidget,XmNexposeCallback,redisplayFakeStrip,
		    dlStripChart);

      /* add button press handlers */
      XtAddEventHandler(localWidget,
          ButtonPressMask,False,(XtEventHandler)handleButtonPress,
          (XtPointer)displayInfo);

      XtManageChild(localWidget);

    }
  } 
  else {
    DlObject *po = &(dlElement->structure.stripChart->object);

    XtVaSetValues(dlElement->widget,
		  XmNx, (Position) po->x,
		  XmNy, (Position) po->y,
		  XmNwidth, (Dimension) po->width,
		  XmNheight, (Dimension) po->height,
		  NULL);
  }

  return updateTask;
}

static void stripChartUpdateGraphicalInfoCb(XtPointer cd) {
  Record *pd = (Record *) cd;
  StripChart *psc = (StripChart *) pd->clientData;
  Widget widget = psc->dlElement->widget;
  int i;


  /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
  /* !!!!! This is a temperory work around !!!!! */
  /* !!!!! for the reconnection.           !!!!! */
  /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
  dm2kRecordAddGraphicalInfoCb(pd,NULL);

  /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
  /* !!!!! End work around                 !!!!! */
  /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

  /* make sure all the channel get their operating ranges */
  for (i = 0; i < psc->nChannels; i++) {
    if (psc->record[i]->precision < 0)
      return;  /* wait for other channels */
  }

  if (XtIsManaged(widget) == False) {
    XtManageChild(widget);
    if(!psc->timerid) psc->timerid = 
			 XtAppAddTimeOut(XtWidgetToApplicationContext(widget),100,configStripChart,psc);
  } else
  if ((psc->pixmap == (Pixmap) NULL) && (XtIsRealized(widget))) {
    stripChartConfig(psc);
  } else {
    /* do it half second later */
    if(!psc->timerid) psc->timerid = 
			 XtAppAddTimeOut(XtWidgetToApplicationContext(widget),100,configStripChart,psc);
  }
}

#ifdef __cplusplus
static void configStripChart(XtPointer cd, XtIntervalId *) {
#else
static void configStripChart(XtPointer cd, XtIntervalId *id) {
#endif
  StripChart *psc = (StripChart *) cd;
  Widget widget = psc->dlElement->widget;

  psc->timerid = (XtIntervalId)0;
  if ((psc->pixmap == (Pixmap) NULL) && (XtIsRealized(widget))) {
    stripChartConfig(psc);
  } else {
     /* do it half second later */
     if(!psc->timerid) psc->timerid = 
			  XtAppAddTimeOut(XtWidgetToApplicationContext(widget),100,configStripChart,psc);
  }
}

#ifdef __cplusplus
static void redisplayStrip(Widget widget, XtPointer cd, XtPointer) {
#else
static void redisplayStrip(Widget widget, XtPointer cd, XtPointer cbs) {
#endif
  StripChart *psc = (StripChart *) cd;
  GC gc = psc->gc;

  /*                    STRIP CHART GEOMETRY

      (0,0)
        *--------------------------------------------------|
        |                                                  |
        |                                                  |
        |                                                  |
        |                      TOP                         |
        |                                                  |
        |  (dataX0-1, dataY0-1)                            |
        |----------*---------------------------------------|
        |          |                   ^        |          |
        |          |< - - - dataWidth -+- - - ->|          |
        |          |                   |        |          |
        |          |                   |        |          |
        |          |                            |          |
        |   LEFT   |               dataHeight   |  RIGHT   |
        |          |                            |          |
        |          |                   |        |          |
        |          |                   |        |          |
        |          |                   |        |          |
        |          |                   V        |          |
        |---------------------------------------*----------|
        |                            (dataX1+1,dataY1+1)   |
        |                                                  |
        |                     BOTTOM                       |
        |                                                  |
        |                                                  |
        |                                                  |
        |--------------------------------------------------*
                                                         (w,h)
  */

  if (psc->pixmap) {
    /* copy the top region */
    XCopyArea(XtDisplay(widget), psc->pixmap, XtWindow(widget), gc, 0, 0,
          psc->w,psc->dataY0,0,0);
    /* copy the left region */
    XCopyArea(XtDisplay(widget), psc->pixmap, XtWindow(widget), gc, 0, psc->dataY0,
          psc->dataX0,psc->dataHeight,0,psc->dataY0);
    /* copy the bottom region */
    XCopyArea(XtDisplay(widget), psc->pixmap, XtWindow(widget), gc, 0, psc->dataY1+1,
          psc->w,(psc->h - psc->dataY1),0,psc->dataY1+1);
    /* copy the right region */
    XCopyArea(XtDisplay(widget), psc->pixmap, XtWindow(widget), gc, psc->dataX1+1,psc->dataY0,
          (psc->w - psc->dataX1), psc->dataHeight, psc->dataX1+1,psc->dataY0);
    /* draw graph */
    if (psc->updateEnable)
      stripChartDraw((XtPointer)psc);
  }
}

#if 0
#ifdef __cplusplus
static void stripChartDestroyCb(XtPointer) {
#else
static void stripChartDestroyCb(XtPointer cd) {
#endif
  return;
}
#endif

static void stripChartUpdateValueCb(XtPointer cd) {
  Record *pd = (Record *) cd;
  StripChart *psc = (StripChart *) pd->clientData;
  Boolean connected = True;
  Boolean readAccess = True;
  int i;
  Widget widget = psc->dlElement->widget;
  DlStripChart *dlStripChart = psc->dlElement->structure.stripChart;

  for (i = 0; i< psc->nChannels; i++) {
    Record *ptmp = psc->record[i];
    if (!ptmp->connected) {
      connected = False;
      break;
    }
    if (!ptmp->readAccess) {
      readAccess = False;
    }
  }

  if (connected) {
    if (readAccess) {
      if (widget) {
        if (XtIsManaged(widget) == False) {
          XtManageChild(widget);
          /* specified the time interval */
          updateTaskSetScanRate(psc->updateTask,psc->timeInterval);
        }
        if (psc->updateEnable) 
          stripChartUpdateGraph((XtPointer)cd);
      }
    } else {
      if (widget) {
        if (XtIsManaged(widget)) {
          XtUnmanageChild(widget);
          /* stop periodic update */
          updateTaskSetScanRate(psc->updateTask,0.0);
        }
      }
      draw3DPane(psc->updateTask,
          psc->updateTask->displayInfo->colormap[dlStripChart->plotcom.bclr]);
      draw3DQuestionMark(psc->updateTask);
    }
  } else {
    if ((widget) && (XtIsManaged(widget))) {
      XtUnmanageChild(widget);
      /* stop periodic update */
      updateTaskSetScanRate(psc->updateTask,0.0);
    }
    drawWhiteRectangle(psc->updateTask);
  }
}

static void stripChartUpdateTaskCb(XtPointer cd) {
  StripChart *psc = (StripChart *) cd;
  Boolean connected = True;
  Boolean readAccess = True;
  Widget widget = psc->dlElement->widget;
  DlStripChart *dlStripChart = psc->dlElement->structure.stripChart;
  int i;

  for (i = 0; i< psc->nChannels; i++) {
    Record *ptmp = psc->record[i];
    if (!ptmp->connected) {
      connected = False;
      break;
    }
    if (!ptmp->readAccess) {
      readAccess = False;
    }
  }

  if (connected) {
    if (readAccess) {
      if (widget) {
        if (XtIsManaged(widget) == False) {
          XtManageChild(widget);
          /* specified the time interval */
          updateTaskSetScanRate(psc->updateTask,psc->timeInterval);
        }
        if (psc->updateEnable) 
          stripChartDraw((XtPointer)psc);
      }
    } else {
      if (widget) {
        if (XtIsManaged(widget)) {
          XtUnmanageChild(widget);
          /* stop periodic update */
          updateTaskSetScanRate(psc->updateTask,0.0);
        }
      }
      draw3DPane(psc->updateTask,
          psc->updateTask->displayInfo->colormap[dlStripChart->plotcom.bclr]);
      draw3DQuestionMark(psc->updateTask);
    }
  } else {
    if ((widget) && (XtIsManaged(widget))) {
      XtUnmanageChild(widget);
      /* stop periodic update */
      updateTaskSetScanRate(psc->updateTask,0.0);
    }
    drawWhiteRectangle(psc->updateTask);
  }
}

static void stripChartUpdateGraph(XtPointer cd) 
{
  Record     * pd = (Record *) cd;
  StripChart * psc = (StripChart *) pd->clientData;
  double       currentTime;
  int          n = 0;

  /* get the time */
  currentTime = dm2kTime();

  while (psc->record[n] != pd) 
    n++;

  /* see whether is time to advance the graph for updating the graph 
   */
  if (currentTime > psc->nextAdvanceTime) 
  {
    Display      * display      = XtDisplay(psc->dlElement->widget);
    GC             gc           = psc->gc;
    DlStripChart * dlStripChart = psc->dlElement->structure.stripChart;
    DisplayInfo  * displayInfo  = psc->updateTask->displayInfo;
    int            i;
    int            totalPixel;    /* number of pixel needed to be drawn */
    XRectangle     rectangle;


      /* calculate how many pixel needed to draw */
    if (psc->nextAdvanceTime > currentTime)
    {
       totalPixel = 0;
    } else {
       totalPixel = 1 + (int) ((currentTime - psc->nextAdvanceTime) /
			       psc->timeInterval);
    }

    psc->nextAdvanceTime += psc->timeInterval * totalPixel;

    if (totalPixel > 0)
    {
       rectangle.x      = psc->dataX0;
       rectangle.y      = psc->dataY0;
       rectangle.width  = psc->dataWidth;
       rectangle.height = psc->dataHeight;
       
       XSetClipRectangles(display,gc,0,0,&rectangle,1,YXBanded);
       
       /* draw the background 
	*/
       XSetForeground(display, gc, 
		      displayInfo->colormap[dlStripChart->plotcom.bclr]);
       
       if (totalPixel <= 1) {
	  XDrawLine(display, psc->pixmap, gc,
		    psc->nextSlot + psc->dataX0, psc->dataY0,
		    psc->nextSlot + psc->dataX0, psc->dataY1);
       } 
       else {
	  int limit    = psc->dataWidth + psc->dataX0;
	  int nextSlot = psc->nextSlot + psc->dataX0;
	  
	  if ((nextSlot + totalPixel) > limit) 
	  {
	     /* if wraped, two filled is needed 
	      */
	     XFillRectangle(display, psc->pixmap, gc,
			    nextSlot, psc->dataY0, 
			    (limit-nextSlot), psc->dataHeight);
	     XFillRectangle(display, psc->pixmap, gc,
			 psc->dataX0, psc->dataY0, 
			    totalPixel-limit+nextSlot, psc->dataHeight);
	  } 
	  else 
	  {
	     /* if not wraped, do one fill */
	     XFillRectangle(display, psc->pixmap, gc,
			    nextSlot, psc->dataY0, totalPixel, psc->dataHeight);
	  }
       }
       
       /* draw each pen/channel 
	*/
       for (i=0; i < psc->nChannels; i++) 
       {
	  int       y1, y2;
	  double    base;
	  int       nextSlot = psc->nextSlot + psc->dataX0;
	  int       limit = psc->dataWidth + psc->dataX0;
	  Record  * p = psc->record[i];
	  
	  /* plot data */
	  base = p->hopr - p->lopr;
	  y1 =  psc->dataY0 + (int) ((psc->dataHeight - 1) *
				     (1.0 - ((psc->minVal[i] - p->lopr) / base)));
	  y2 =  psc->dataY0 + (int) ((psc->dataHeight - 1) *
				     (1.0 - ((psc->maxVal[i] - p->lopr) / base)));
	  
	  XSetForeground(display, gc, 
			 displayInfo->colormap[dlStripChart->pen[i].clr]);
	  
	  XDrawLine(display, psc->pixmap, gc, nextSlot, y1, nextSlot, y2);
	
	  if (totalPixel > 1) 
	  {
	     int y;
	     y =  psc->dataY0 + (int) ((psc->dataHeight - 1) *
				       (1.0 - ((psc->value[i] - p->lopr) / base)));
	    
	     /* fill the gap in between 
	      */
	     if ((nextSlot + totalPixel) > limit) 
	     {
		/* if wraped, two lines are needed 
		 */
		XDrawLine(display, psc->pixmap, gc,
			  nextSlot, y, limit, y);
		XDrawLine(display, psc->pixmap, gc,
			  psc->dataX0,y,
			  totalPixel-limit+nextSlot+psc->dataX0,y);
	     } 
	     else
	     {
		/* not wraped, one line only */
		XDrawLine(display, psc->pixmap, gc,
			  nextSlot, y, (nextSlot + totalPixel), y);
	     }
	  }
	
	  /* reset max, min to the last pen position 
	 */
	  psc->maxVal[i] = psc->minVal[i] = psc->value[i];
       }
    
       XSetClipOrigin(display,gc,0,0);
       XSetClipMask(display,gc,None);
       psc->nextSlot += totalPixel;
    
       if (psc->nextSlot >= psc->dataWidth) 
	  psc->nextSlot -= psc->dataWidth;
    }
  }
  
  /* remember the min and max and current pen position 
   */
  psc->value[n] = pd->value;
  
  if (pd->value > psc->maxVal[n]) {
     psc->maxVal[n] = pd->value;
  }

  if (pd->value < psc->minVal[n]) {
    psc->minVal[n] = pd->value;
  }

  return;
}

static void stripChartDrawArchiveData(StripChart * psc)
{
  Display      * display      = XtDisplay(psc->dlElement->widget);
  GC             gc           = psc->gc;
  DlStripChart * dlStripChart = psc->dlElement->structure.stripChart;
  DisplayInfo  * displayInfo  = psc->updateTask->displayInfo;
  int            i;
  XRectangle     rectangle;
  long           timeInterval;
  long           countPoints;
  float        * archiveData;
  long           returnedFromTime;
  long           returnedToTime;
  long           returnedCount;

#define PIXEL_STEP 4

  timeInterval = psc->dataWidth * psc->timeInterval;
  countPoints = psc->dataWidth / PIXEL_STEP;
  
  rectangle.x      = psc->dataX0;
  rectangle.y      = psc->dataY0;
  rectangle.width  = psc->dataWidth;
  rectangle.height = psc->dataHeight;
  
  XSetClipRectangles(display, gc, 0, 0, &rectangle, 1, YXBanded);
  
  /* draw each pen/channel 
   */
  for (i=0; i < psc->nChannels; i++) 
    {
      DlPen   * pen = &(dlStripChart->pen[i]);
      Record  * p = psc->record[i];
      XPoint    xPoints[3]; 
      int       j, n;
      float     yBase;
      short     xOffset;
      short     xStep;
      short     lastY;
      
      if (pen == NULL || *pen->utilChan == '\0')
	continue;

      if (getArchiveData(pen->utilChan, -timeInterval, 0, countPoints, 
			 REQUEST_MODE_ONE_SHOT,
			 &returnedFromTime, &returnedToTime, &archiveData,
			 &returnedCount))
	continue;

      XSetForeground(display, gc, 
		     displayInfo->colormap[dlStripChart->pen[i].clr]);
      
      yBase   = p->hopr - p->lopr;
      xOffset = psc->dataX1;
      xStep   = PIXEL_STEP;

      /* plot data 
       */
      lastY =  psc->dataY0 + (int) ((psc->dataHeight - 1) *
	       (1.0 - ((archiveData[returnedCount-1] - p->lopr) / yBase)));

      xPoints[2].x = xOffset;
      xPoints[2].y = lastY;

      for (j = returnedCount-2, n = 1; j >= 0; j--, n++) 
	{
	  short x = xOffset - n * xStep;
	  short y;

	  y = psc->dataY0 + (int)((psc->dataHeight - 1) *
				  (1.0 - ((archiveData[j] - p->lopr)/yBase)));

	  if (y != lastY || x <= psc->dataX0 || j == 0) {
	    xPoints[0].x = xPoints[2].x;
	    xPoints[0].y = xPoints[2].y;
	    xPoints[1].x = x;
	    xPoints[1].y = xPoints[0].y; 
	    xPoints[2].x = x;
	    xPoints[2].y = y;

	    if (xPoints[1].x == xPoints[2].x &&
		xPoints[1].y == xPoints[2].y)
	      XDrawLines(display, psc->pixmap, gc, xPoints,2,CoordModeOrigin);
	    else
	      XDrawLines(display, psc->pixmap, gc, xPoints,3,CoordModeOrigin);
	    
	    lastY = y;

	    if (x <= psc->dataX0)
	      break;
	  }
      }

      psc->value[i] = 
	psc->maxVal[i] = 
	  psc->minVal[i] = archiveData[returnedCount-1];
    }
  
  psc->nextSlot = 0;

  XSetClipOrigin(display,gc,0,0);
  XSetClipMask(display,gc,None);

  return;
}

static void stripChartDraw(XtPointer cd) {
  StripChart *psc = (StripChart *) cd;
  Display *display = XtDisplay(psc->dlElement->widget);
  Window  window = XtWindow(psc->dlElement->widget);
  GC      gc = psc->gc;
  int startPos;

  /* make sure the plot is up to date */
  stripChartUpdateValueCb((XtPointer)psc->record[0]);
  startPos = psc->nextSlot;
  if (startPos > psc->dataWidth) {
    startPos = 0;
  }
  if (startPos) {
    int x, w;
    /* copy the first half */
    x = startPos + psc->dataX0;
    w = psc->dataWidth - startPos;
    XCopyArea(display,psc->pixmap,window,gc,
              x, psc->dataY0, w, psc->dataHeight,
              psc->dataX0, psc->dataY0);
    /* copy the second half */
    x = w + psc->dataX0;
    XCopyArea(display,psc->pixmap,window,gc,
              psc->dataX0, psc->dataY0, startPos, psc->dataHeight,
              x, psc->dataY0);
  } else {
    XCopyArea(display,psc->pixmap,window,gc,
              psc->dataX0, psc->dataY0, psc->dataWidth, psc->dataHeight,
              psc->dataX0, psc->dataY0);
  }
}

static void stripChartName(XtPointer cd, char **name, short *severity, int *count) {
  StripChart *psc = (StripChart *) cd;
  int i;

  *count = psc->nChannels;
  for (i = 0; i < psc->nChannels; i++) {
    name[i] = psc->record[i]->name;
    severity[i] = psc->record[i]->severity;
  }
}

void linear_scale(double xmin, double xmax, int n,
		  double *xminp, double *xmaxp, double *dist);

DlElement *createDlStripChart(DlElement *p)
{
  DlStripChart *dlStripChart;
  DlElement *dlElement;
  int penNumber;


  dlStripChart = DM2KALLOC(DlStripChart);
  if (dlStripChart == NULL)
    return 0;

  if (p != NULL) {
    objectAttributeCopy(&(dlStripChart->object), &(p->structure.stripChart->object));
    plotcomAttributeCopy(&(dlStripChart->plotcom), &(p->structure.stripChart->plotcom));

    dlStripChart->period = p->structure.stripChart->period;
    dlStripChart->units = p->structure.stripChart->units;

    /* for backward compatible */
    dlStripChart->delay = p->structure.stripChart->delay;
    dlStripChart->oldUnits = p->structure.stripChart->oldUnits;

    for (penNumber = 0; penNumber < MAX_PENS; penNumber++)
      penAttributeCopy((DlPen *)&(dlStripChart->pen[penNumber]),
		       (DlPen *)&(p->structure.stripChart->pen[penNumber]));
  } 
  else {
    objectAttributeInit(&(dlStripChart->object));
    plotcomAttributeInit(&(dlStripChart->plotcom));
    dlStripChart->period = 60.0;
    dlStripChart->units = SECONDS;

    /* for backward compatible */
    dlStripChart->delay = -1.0;
    dlStripChart->oldUnits = SECONDS;

    for (penNumber = 0; penNumber < MAX_PENS; penNumber++)
      penAttributeInit((DlPen *)&(dlStripChart->pen[penNumber]));
  }


  dlElement = createDlElement(DL_StripChart, (XtPointer) dlStripChart,
			      &stripChartDlDispatchTable);

  if (dlElement == NULL)
    destroyDlStripChart(dlStripChart);

  return(dlElement);
}

DlElement *parseStripChart(DisplayInfo *displayInfo)
{
  char token[MAX_TOKEN_LENGTH];
  TOKEN tokenType;
  int nestingLevel = 0;
  DlStripChart *dlStripChart;
  DlElement *dlElement = createDlStripChart(NULL);
  int penNumber;
  int isVersion2_1_x = False;

  if (dlElement == NULL)
    return 0;

  dlStripChart = dlElement->structure.stripChart;

  do {
    switch( (tokenType=getToken(displayInfo,token)) ) {
    case T_WORD:
      if (STREQL(token,"object"))
	parseObject(displayInfo,&(dlStripChart->object));
      else if (STREQL(token,"plotcom"))
	parsePlotcom(displayInfo,&(dlStripChart->plotcom));
      else if (STREQL(token,"period")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	dlStripChart->period = atof(token);
	isVersion2_1_x = True;
      } 
      else if (STREQL(token,"delay")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	dlStripChart->delay = atoi(token);
      }
      else if (STREQL(token,"units")) {
	getToken(displayInfo,token);
	getToken(displayInfo,token);
	if (STREQL(token,"minute")) 
	  dlStripChart->units = MINUTES;
	else if (STREQL(token,"second")) 
	  dlStripChart->units = SECONDS;
	else if (STREQL(token,"milli second")) 
	  dlStripChart->units = MILLISECONDS;
	else if (STREQL(token,"milli-second")) 
	  dlStripChart->units = MILLISECONDS;
	else
	  fprintf(stderr,
		  "\nparseStripChart: illegal units %s,%s",token,
		  "default of SECONDS taken");
      } else if (!strncmp(token,"pen",3)) {
	penNumber = MIN(token[4] - '0', MAX_PENS-1);
	parsePen(displayInfo,&(dlStripChart->pen[penNumber]));
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

  if (isVersion2_1_x) {
    dlStripChart->delay = -1.0;  /* -1.0 is used as a indicator to save
                                    as new format */
  } else
    if (dlStripChart->delay > 0) {
      double val, dummy1, dummy2;
      switch (dlStripChart->units) {
      case MILLISECONDS:
        dummy1 = -0.060 * (double) dlStripChart->delay;
        break;
      case SECONDS:
        dummy1 = -60 * (double) dlStripChart->delay;
        break;
      case MINUTES:
        dummy1 = -3600.0 * (double) dlStripChart->delay;
        break;
      default:
        dummy1 = -60 * (double) dlStripChart->delay;
        break;
      }

      linear_scale(dummy1, 0.0, 2, &val, &dummy1, &dummy2);
      dlStripChart->period = -val; 
      dlStripChart->oldUnits = dlStripChart->units;
      dlStripChart->units = SECONDS;
    }

  return dlElement;

}

void writeDlStripChart( FILE *stream, DlElement *dlElement, int level) 
{
  int i;
  char indent[256]; level=MIN(level,256-2);
  DlStripChart *dlStripChart = dlElement->structure.stripChart;

  /* for the compatibility 
   */
  if (dlStripChart->delay > 0.0) {
    double val, dummy1, dummy2;
    switch (dlStripChart->oldUnits) {
      case MILLISECONDS:
        dummy1 = -0.060 * (double) dlStripChart->delay;
        break;
      case SECONDS:
        dummy1 = -60 * (double) dlStripChart->delay;
        break;
      case MINUTES:
        dummy1 = -3600.0 * (double) dlStripChart->delay;
        break;
      default:
	dummy1 = -60 * (double) dlStripChart->delay;
	break;
      }
    
    linear_scale(dummy1, 0.0, 2, &val, &dummy1, &dummy2);
    if (dlStripChart->period != -val  || dlStripChart->units != SECONDS) {
      dlStripChart->delay = -1;
    }
  }

  level =MIN(level,256-2);
  memset(indent,'\t',level);
  indent[level] = '\0';
  
  fprintf(stream,"\n%s\"strip chart\" {",indent);
  writeDlObject(stream,&(dlStripChart->object),level+1);
  writeDlPlotcom(stream,&(dlStripChart->plotcom),level+1);

  if (dlStripChart->delay < 0.0) {
    if (dlStripChart->period != 60.0) 
      fprintf(stream,"\n%s\tperiod=%f",indent,dlStripChart->period);
#ifdef SUPPORT_0201XX_FILE_FORMAT
    if (Dm2kUseNewFileFormat) {
#endif
      if (dlStripChart->units != SECONDS)
	fprintf(stream,"\n%s\tunits=\"%s\"",indent,
		stringValueTable[dlStripChart->units]);
#ifdef SUPPORT_0201XX_FILE_FORMAT
    } else {
      fprintf(stream,"\n%s\tunits=\"%s\"",indent,
	      stringValueTable[dlStripChart->units]);
    }
#endif
  } else {
    /* for the compatibility 
     */
    fprintf(stream,"\n%s\tdelay=%f",indent,dlStripChart->delay);
    fprintf(stream,"\n%s\tunits=\"%s\"",indent,
	    stringValueTable[dlStripChart->oldUnits]);
  }

  for (i = 0; i < MAX_PENS; i++) {
    writeDlPen(stream,&(dlStripChart->pen[i]),i,level+1);
  }

  fprintf(stream,"\n%s}",indent);
}

static void stripChartInheritValues(ResourceBundle *pRCB, DlElement *p) 
{
  DlStripChart *dlStripChart = p->structure.stripChart;

  dm2kGetValues(pRCB,
    CLR_RC,        &(dlStripChart->plotcom.clr),
    BCLR_RC,       &(dlStripChart->plotcom.bclr),
    -1);
}

static void stripChartGetValues(ResourceBundle *pRCB, DlElement *p) {
  DlStripChart *dlStripChart = p->structure.stripChart;
  dm2kGetValues(pRCB,
    X_RC,          &(dlStripChart->object.x),
    Y_RC,          &(dlStripChart->object.y),
    WIDTH_RC,      &(dlStripChart->object.width),
    HEIGHT_RC,     &(dlStripChart->object.height),
    TITLE_RC,      &(dlStripChart->plotcom.title),
    XLABEL_RC,     &(dlStripChart->plotcom.xlabel),
    YLABEL_RC,     &(dlStripChart->plotcom.ylabel),
    CLR_RC,        &(dlStripChart->plotcom.clr),
    BCLR_RC,       &(dlStripChart->plotcom.bclr),
    PERIOD_RC,     &(dlStripChart->period),
    UNITS_RC,      &(dlStripChart->units),
    SCDATA_RC,     &(dlStripChart->pen),
    -1);
}
