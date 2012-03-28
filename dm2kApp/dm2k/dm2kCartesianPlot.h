#ifndef CARTESIAN_PLOT_H
#define CARTESIAN_PLOT_H

#define CP_XDATA_COLUMN         0
#define CP_YDATA_COLUMN         1
#define CP_COLOR_COLUMN         2
 
#define CP_APPLY_BTN    0
#define CP_CLOSE_BTN    1
 
#ifndef USE_XRT
typedef void XrtData;
#endif

typedef struct {
        float axisMin;
        float axisMax;
        Boolean isCurrentlyFromChannel;
} CartesianPlotAxisRange;

typedef enum {
    CP_XYScalar,
    CP_XScalar,         CP_YScalar,
    CP_XVector,         CP_YVector,
    CP_XVectorYScalar,
    CP_YVectorXScalar,
    CP_XYVector
} XYChannelTypeEnum;

typedef struct {
  struct _CartesianPlot *cartesianPlot;
  XrtData               *xrtData;
  int                   trace;
  Record                *recordX;
  Record                *recordY;
  XYChannelTypeEnum     type;
} XYTrace;

typedef struct _CartesianPlot {
        DlElement      *dlElement;
        XYTrace         xyTrace[MAX_TRACES];
        XYTrace         eraseCh;
        XYTrace         triggerCh;
        UpdateTask      *updateTask;
        int             nTraces;        /* number of traces ( <= MAX_TRACES) */
        XrtData         *xrtData1, *xrtData2;    /* XrtData                  */
        /* used for channel-based range determination - filled in at connect */
        CartesianPlotAxisRange  axisRange[3];    /* X, Y, Y2 _AXIS_ELEMENT   */
        eraseMode_t     eraseMode;               /* erase mode               */
        Boolean         dirty1;                  /* xrtData1 needs screen update */
        Boolean         dirty2;                  /* xrtData2 needs screen update */
        TS_STAMP        startTime;
        Boolean         timeScale;
} CartesianPlot;

#endif

/****************************************************************************
 * CARTESIAN PLOT DATA
 *********************************************************************/
#ifndef DM2K_CARTPLOT_EXCL_GLOBALS
static Widget cpMatrix = NULL, cpForm = NULL;
static String cpColumnLabels[] = {"X Data","Y Data","Color",};
static int cpColumnMaxLengths[] = {MAX_TOKEN_LENGTH-1,MAX_TOKEN_LENGTH-1,6,};
static short cpColumnWidths[] = {36,36,6,};
static unsigned char cpColumnLabelAlignments[] = {XmALIGNMENT_CENTER,XmALIGNMENT_CENTER,XmALIGNMENT_CENTER,};
/* and the cpCells array of strings (filled in from globalResourceBundle...)
*/
static String cpRows[MAX_TRACES][3];
static String *cpCells[MAX_TRACES];
static String dashes = "******";
 
static Pixel cpColorRows[MAX_TRACES][3];
static Pixel *cpColorCells[MAX_TRACES];

/*********************************************************************
 * CARTESIAN PLOT AXIS DATA
 *********************************************************************/
 
/*
 * for the Cartesian Plot Axis Dialog, use the following static globals
 *   (N.B. This dialog has semantics for both EDIT and EXECUTE time
 *    operation)
 */
 
/* Widget cpAxisForm defined in dm2k.h since execute-time needs it too */
 
/* define array of widgets (for X, Y1, Y2) */
static Widget axisRangeMenu[3];                 /* X_AXIS_ELEMENT =0 */
static Widget axisStyleMenu[3];                 /* Y1_AXIS_ELEMENT=1 */
static Widget axisRangeMin[3], axisRangeMax[3]; /* Y2_AXIS_ELEMENT=2 */
static Widget axisRangeMinRC[3], axisRangeMaxRC[3];
static Widget axisTimeFormat;
#endif

 
#define CP_X_AXIS_STYLE   0
#define CP_Y_AXIS_STYLE   1
#define CP_Y2_AXIS_STYLE  2
#define CP_X_RANGE_STYLE  3
#define CP_Y_RANGE_STYLE  4
#define CP_Y2_RANGE_STYLE 5
#define CP_X_RANGE_MIN    6
#define CP_Y_RANGE_MIN    7
#define CP_Y2_RANGE_MIN   8
#define CP_X_RANGE_MAX    9
#define CP_Y_RANGE_MAX    10
#define CP_Y2_RANGE_MAX   11
#define CP_X_TIME_FORMAT  12
#define CP_PLOT_STYLE     13
 
#define MAX_CP_AXIS_ELEMENTS    20
#define MAX_CP_AXIS_BUTTONS     MAX(NUM_CARTESIAN_PLOT_RANGE_STYLES,\
                                    MAX(NUM_CP_TIME_FORMATS,NUM_CARTESIAN_PLOT_AXIS_STYLES))
