/**
 ** toplevel Graph Plotter test program
 **/


#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>

#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <Xm/PushB.h>
#include <Xm/BulletinB.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/Scale.h>
#include <Xm/RowColumn.h>
#include <Xm/List.h>


#include <stdio.h>

#include <math.h>		/* very Very VERY IMPORTANT ! */

#include "GraphX.h"             /* overall GraphX include file
				   defines data types, plot types,
				   functions and macros            */


#define WIDTH   384
#define HEIGHT  384

#define NBUFFERS 1
#define NPTS 100 
#define TWOPI 6.28


static XtWorkProc animateGraph();

static Boolean animate = FALSE;
static Boolean booleanSleep = FALSE;
static XtWorkProcId animateId = NULL;
static Graph *graph;
/* for the application contexts... */
static XtAppContext app;



/* 
 * TRANSLATIONS
 */

static void toggleGraphAnimation(w,event,params,num_params)
  Widget w;
  XEvent *event;
  String *params;
  int    num_params;
{


   if (animate) {
      animate = FALSE;
      XtRemoveWorkProc(animateId);
   } else {
      animate = TRUE;
      /* must call this after XtInitialize() */
      animateId = XtAppAddWorkProc(app,(XtWorkProc)animateGraph,NULL);
   }
}

static void toggleSleep(w,event,params,num_params)
  Widget w;
  XEvent *event;
  String *params;
  int    num_params;
{
   booleanSleep = (!booleanSleep);
}


static void graphPrintPlot(w,event,params,num_params)
  Widget w;
  XEvent *event;
  String *params;
  int    num_params;
{
  graphPrint(graph);
}


static void graphLine(w,event,params,num_params)
  Widget w;
  XEvent *event;
  String *params;
  int    num_params;
{
  graphSetDisplayType(graph,GraphLine);
  graphDraw(graph);
}

static void graphPoint(w,event,params,num_params)
  Widget w;
  XEvent *event;
  String *params;
  int    num_params;
{
  graphSetDisplayType(graph,GraphPoint);
  graphDraw(graph);
}

static void graphBar(w,event,params,num_params)
  Widget w;
  XEvent *event;
  String *params;
  int    num_params;
{
  graphSetDisplayType(graph,GraphBar);
  graphSetBins(graph,0,100);
  graphDraw(graph);
}




static void quit(w,event,params,num_params)
  Widget w;
  XEvent *event;
  String *params;
  int    num_params;
{
/*
 * clean up our space like a good program
 */
  graphTerm(graph);

  exit(0);
}



/*
 * CALLBACKS 
 */


static void redisplayGraph(w,graph,call_data)
  Widget w;
  Graph **graph;	/* double indirection since filling in actual */
  XtPointer call_data;	/* address of Graph structure after XtAddCallback */
{

  graphRefresh(*graph);	/* copy pixmap onto window (this is quick) */	

}


static void resizeGraph(w,graph,call_data)
  Widget w;
  Graph **graph;	/* double indirection since filling in actual */
  XtPointer call_data;	/* address of Graph structure after XtAddCallback */
{

  graphResize(*graph);
  graphDraw(*graph);
}




static XtWorkProc animateGraph()
{
int i,j;
double temp;

if (booleanSleep) sleep(1);

for (i=0; i<graph->nBuffers; i++)
  for (j=0; j<graph->nValues[i]; j++){
    temp = (  (((double) random())/pow(2.0,31.0) > 0.5) ?
         - 0.2*((double) random())/pow(2.0,31.0) :
           0.2*((double) random())/pow(2.0,31.0) );

    graph->dataBuffer[i][j].x += temp;
    graph->dataBuffer[i][j].y += temp;
  }

   graphDraw(graph);

/* return FALSE so this WorkProc keeps getting called (just in case...) */
return FALSE;
}


static char defaultTranslations[] = 
		"Ctrl<Key>p: graphPrintPlot()    \n\
		 <Key>a: toggleGraphAnimation()    \n\
		 <Key>s: toggleSleep()    \n\
		 <Key>l: graphLine()    \n\
		 <Key>p: graphPoint()    \n\
		 <Key>b: graphBar()    \n\
		 <Key>q: quit() ";

static XtActionsRec actionsTable[] = {
	{"toggleGraphAnimation", (XtActionProc)toggleGraphAnimation},
	{"toggleSleep", (XtActionProc)toggleSleep},
	{"graphLine", (XtActionProc)graphLine},
	{"graphPoint", (XtActionProc)graphPoint},
	{"graphPrintPlot", (XtActionProc)graphPrintPlot},
	{"graphBar", (XtActionProc)graphBar},
	{"quit", (XtActionProc)quit},
};





main(argc,argv)
  int   argc;
  char *argv[];
{
  Widget topLevel2, canvas2;
  Arg args[5];
  int n;

  XtTranslations transTable;

  int i, j;
  int nValues[NBUFFERS];
  XYdataPoint *data[NBUFFERS];

  Display *display = NULL;
  int screen;
  Window window2;

  static char *dataColors[] = {"yellow","orange",};

/* these are filled in by graphXGetBestFont() calls */
  char *titleFont, *axesFont;
  int titleFontSize, axesFontSize;


/*
 * first let's initialize our data buffers
 */
  for (i = 0; i < NBUFFERS; i++)
     data[i] = (XYdataPoint *) malloc((unsigned) NPTS * sizeof(XYdataPoint));



  printf("\n\t enter l to display as line\n\t p = point\n\t b = bar chart\n");

/*
 * try application contexts approach to shells
 */
   XtToolkitInitialize();
   app = XtCreateApplicationContext();

   display = XtOpenDisplay(app,NULL,argv[0], 
		"application context", NULL,0,&argc,argv);
   if (display == NULL) {
      fprintf(stderr,"\ntestGraph: Can't open display!\n");
      exit(1);
   }

   XtSetArg(args[0],XtNiconName,"1000 pt XY plot");
   topLevel2 = XtAppCreateShell("Test", "shell2", applicationShellWidgetClass,
		display,args,1);




/*
 * Register new actions and compile translations table
 */
   XtAppAddActions(app,actionsTable,XtNumber(actionsTable));
   transTable = XtParseTranslationTable(defaultTranslations);


/*
 * setup a canvas widget to draw in
 */
   n = 0;
   XtSetArg(args[n], XmNwidth, WIDTH); n++;
   XtSetArg(args[n], XmNheight, HEIGHT); n++;
   canvas2 = XmCreateDrawingArea(topLevel2,"drawingArea",args,n);
   XtManageChild(canvas2);

/*
 * merge program-defined translations with existing translations
 */
   XtOverrideTranslations(canvas2,transTable);
   XtOverrideTranslations(topLevel2,transTable);

/*
 * add the expose and resize callbacks, passing pointer to graph as the data of 
 *   the callback
 */
   XtAddCallback(canvas2, XmNexposeCallback,
	(XtCallbackProc)redisplayGraph, &graph);
   XtAddCallback(canvas2, XmNresizeCallback,
	(XtCallbackProc)resizeGraph, &graph);


/*
 *  need to realize the widgets for windows to be created...
 */

   XtRealizeWidget(topLevel2);

   screen  = DefaultScreen(display);
   window2  = XtWindow(canvas2);


/*
 * now proceed with graph generation
 */

   graph = graphInit(display,screen,window2);

   for (i = 0; i < NBUFFERS; i++) {
     nValues[i] = 0;
     for (j=0; j<NPTS; j++) {
    /* just the normal X value (but a ~random one) */
	data[i][j].x = (double) (-TWOPI + 2.0*TWOPI*j/((NPTS-1)) +
				0.5 - 0.1*((double) random())/pow(2.0,31.0));

    /* and this is the normal Y or Bin value */
	data[i][j].y = cos(data[i][j].x) + 0.5 - 
			0.1*((double) random())/pow(2.0,31.0);
	nValues[i]++;
     }
   }

/*
 * (MDA) add zoom type event handling
 */
   graphSetInteractive(graph,True);


   titleFontSize = GraphX_TitleFontSize(graph);
   titleFont = graphXGetBestFont(display,"times","bold","r", titleFontSize);
   axesFontSize = GraphX_AxesFontSize(graph);
   axesFont = graphXGetBestFont(display,"times","medium","r",axesFontSize);
 

   /* do a normal graph/XY plot (as scatter plot), 
			with data external to graph */
  /*   -- pass NULL as dataColor ==> do own color allocation */

   graphSet(graph, NBUFFERS, NPTS, data, nValues, GraphLine, 
     "An Animated XY Plot", 
     titleFont,
     "X Axis", "Y Axis", 
     axesFont,
     "white", "black", dataColors, GraphExternal);

   /* do the drawing so that expose event can map pixmap onto window */
   graphDraw(graph);

   XtAppMainLoop(app);
}


