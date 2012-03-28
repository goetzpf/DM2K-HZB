
/**
 ** toplevel Surface Plotter test program
 **
 ** MDA - 1 June 1990
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

#include <math.h>

#include "GraphX.h"	/* get those graphX type definitions */

#define X_SIZE 40
#define Y_SIZE 40

#define ROTATION_SIZE 5.0


static XtWorkProc animateSurface();

/*** have to keep these global due to the ad hoc structure of this
     program.  This allows easy (but non-reentrant) running of this
     program/application - animation and translation routines don't
     automatically get the information that the callbacks get, hence
     we'll cheat a bit.  A better way would be to register this data
     somewhere and let the resize/redisplay routines use them ***/

static Widget canvas;
static Surface *surface;

static Boolean animate = FALSE;
static Boolean shaded = FALSE;
static XtWorkProcId animateId = NULL;

static int surfaceWidth = 384;
static int surfaceHeight = 384;

static float xAngle = 45.0, yAngle = 0.0, zAngle = 45.0;



/* 
 * TRANSLATIONS
 */

static void printPlot(w,event,params,num_params)
Widget w;
XEvent *event;
String *params;
int    num_params;
{

  surfacePrint(surface);

}


static void rotateSurface(w,event,params,num_params)
Widget w;
XEvent *event;
String *params;
int    num_params;
{

switch(atoi(params[0])) {

  case 1 :	xAngle -= ROTATION_SIZE;
		if (xAngle < 0.0) xAngle += 360.0;
		break;

  case 2 :	xAngle += ROTATION_SIZE;
		if (xAngle > 360.0) xAngle -= 360.0;
		break;

  case 3 :	yAngle -= ROTATION_SIZE;
		if (yAngle < 0.0) yAngle += 360.0;
		break;

  case 4 :	yAngle += ROTATION_SIZE;
		if (yAngle > 360.0) yAngle -= 360.0;
		break;
  }

surfaceSetView(surface, xAngle, yAngle, zAngle);
surfaceDraw(surface);

}



static void toggleRenderMode(w,event,params,num_params)
Widget w;
XEvent *event;
String *params;
int    num_params;
{
   if (shaded) {
      shaded = FALSE;
      surfaceSetRenderMode(surface,SurfaceSolid);
   } else {
      shaded = TRUE;
      surfaceSetRenderMode(surface,SurfaceShaded);
   }
   surfaceDraw(surface);
}


static void toggleSurfaceAnimation(w,event,params,num_params)
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
      animateId = XtAddWorkProc((XtWorkProc)animateSurface,NULL);
   }
}



static void quit(w,event,params,num_params)
Widget w;
XEvent *event;
String *params;
int    num_params;
{
  surfaceTerm(surface);
  exit(0);
}



/*
 * CALLBACKS 
 */


static void redisplaySurface(w,surface,call_data)
Widget w;
Surface **surface;     /*  double indirection since filling in actual */
XtPointer call_data;     /* address of Surface struct later than XtAddCallback */
{

  surfaceRefresh(*surface);

}


static void resizeSurface(w,mySurface,call_data)
Widget w;
Surface **mySurface;     /*  double indirection since filling in actual */
XtPointer call_data;     /* address of Surface struct later than XtAddCallback */
{

   surfaceResize(surface);
   surfaceDraw(surface);

}




static XtWorkProc animateSurface()
{

yAngle -= ROTATION_SIZE;
if (yAngle < 0.0) yAngle += 360.0;
xAngle -= ROTATION_SIZE;
if (xAngle < 0.0) xAngle += 360.0;

surfaceSetView(surface, xAngle, yAngle, zAngle);
surfaceDraw(surface);

/* return FALSE so this WorkProc keeps getting called (just in case...) */
return FALSE;

}

static char defaultTranslations[] =
        "Ctrl<Key>p: printPlot()  \n\
         <Key>l: rotateSurface(1)  \n\
         <Key>h: rotateSurface(2)  \n\
         <Key>j: rotateSurface(3)  \n\
         <Key>k: rotateSurface(4)  \n\
         <Key>a: toggleSurfaceAnimation()  \n\
         <Key>s: toggleRenderMode()  \n\
         <Key>q: quit() ";

static XtActionsRec actionsTable[] = {
        {"rotateSurface", (XtActionProc)rotateSurface},
        {"toggleSurfaceAnimation", (XtActionProc)toggleSurfaceAnimation},
        {"toggleRenderMode", (XtActionProc)toggleRenderMode},
        {"printPlot", (XtActionProc)printPlot},
        {"quit", (XtActionProc)quit},
};




main(argc,argv)
  int   argc;
  char *argv[];
{
  Widget topLevel;
  Arg args[5];
  int n;

  XtTranslations transTable;

  Display *display;
  Window window;
  int screen;

  int i, j;
  double mesh[X_SIZE*Y_SIZE], x[X_SIZE], y[Y_SIZE];
  double r;
  char *foreColor = "black";
  char *backColor = "light blue";
  char *dataColor = "blue";

/* these are filled in by graphXGetBestFont() calls */
  char *titleFont, *axesFont;
  int   titleFontSize, axesFontSize;


/*
 * Initialize the Intrinsics right away
 */
   topLevel = XtInitialize(argv[0], "Surface Plotter", NULL, 0, &argc, argv);




/*
 * Register new actions and compile translations table
 */
   XtAddActions(actionsTable,XtNumber(actionsTable));
   transTable = XtParseTranslationTable(defaultTranslations);


/*
 * setup a canvas widget to draw in
 */
   n = 0;
   XtSetArg(args[n], XmNwidth, surfaceWidth); n++;
   XtSetArg(args[n], XmNheight,surfaceHeight); n++;

   canvas = XtCreateManagedWidget("canvas", xmDrawingAreaWidgetClass,
                                        topLevel, args, n);

/*
 * merge program-defined translations with existing translations
 */
   XtOverrideTranslations(canvas,transTable);
   XtOverrideTranslations(topLevel,transTable);

/*
 * add the expose and resize callbacks, passing surface as the data of 
 *   the callback
 */
   XtAddCallback(canvas, XmNexposeCallback, 
	(XtCallbackProc)redisplaySurface, &surface);
   XtAddCallback(canvas, XmNresizeCallback,
	(XtCallbackProc)resizeSurface, &surface);

/*
 *  need to realize the widgets for windows to be created...
 */

   XtRealizeWidget(topLevel);


/*
 * now proceed with surface generation
 */

   display = XtDisplay(topLevel);
   screen  = DefaultScreen(display);
   window  = XtWindow(canvas);

   surface = surfaceInit(display,screen,window);

   for (i=0; i<X_SIZE; i++) x[i] = 4.0*(i*(1.0/(X_SIZE-1)));
   for (j=0; j<Y_SIZE; j++) y[j] = 4.0*(j*(1.0/(Y_SIZE-1)));
   for (i=0; i<X_SIZE; i++)
      for(j=0; j<Y_SIZE; j++) {
        r = sin(x[i]*y[j]);
        *(mesh + i + j * X_SIZE) =  r;
      }


   titleFontSize = GraphX_TitleFontSize(surface);
   titleFont = graphXGetBestFont(display,"times","bold","r", titleFontSize);


   /* setup the Surface */

   surfaceSet(surface, mesh, x, y, X_SIZE, Y_SIZE, "Surface Plot", 
                titleFont,
                foreColor, backColor, dataColor, SurfaceExternal);
   surfaceSetView(surface, xAngle, yAngle, zAngle);

   surfaceDraw(surface);


   XtMainLoop();
}


