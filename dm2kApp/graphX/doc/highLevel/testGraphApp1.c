/**
 ** toplevel Graph Plotter test program
 **        using the graphApp layer of code...
 **
 ** (MDA) 1 September 1990
 **/

#include <stdio.h>
#include <math.h>

#include "GraphApp.h"    /* this includes all graphX type definitions */
                         /*   as well as all the relevant X includes  */

#define TWOPI 6.28
#define WIDTH   400
#define HEIGHT  400

#define NBUFFERS 7       /* number of buffers (sets of data) */
#define NPTS 200         /* number of points per buffer */


main(argc,argv)
  int   argc;
  char *argv[];
{
  GraphApp *graphApp;
  Graph *graph;
  int i, j, shellNumber;
  XYdataPoint *data[NBUFFERS];
  int nValues[NBUFFERS];

  static char *legendArray[] = {"data set #1", "data set #2", "data set #3",
        "data set #4", "data set #5", "data set # 6", "data set #7",};

/*
 * initiate an X connection (to the default display since the name
 *   being passed in here is NULL {use -display xxx or the DISPLAY 
 *   environment variable}) and application context (to allow multiple
 *   X applications in a single address space).
 */
   graphApp = graphAppInit(NULL,&argc,argv);


/*
 * and create a shell and canvas in this GraphApp context
 *  note that this defines two standard translations:
 *    <Ctrl>p  to print to the PostScript printer defined by PSPRINTER
 *    <Ctrl>q to terminate the program and close the X connection
 */
   shellNumber = graphAppInitShell(graphApp,HEIGHT,WIDTH,"shell #1");

   
/* ------------------------------------------------------------------ */
/*
 * allocate our data buffer
 */
   for (i=0; i<NBUFFERS; i++)
       data[i] = (XYdataPoint *) malloc((unsigned) NPTS * sizeof(XYdataPoint));

/*
 * now proceed with graph generation
 */
   graph = graphInit(GraphAppInfo(graphApp,shellNumber));

   for (i=0; i<NBUFFERS; i++){
     nValues[i] = 0;
     for (j=0; j<NPTS; j++) {
        data[i][j].x = (double) (-TWOPI + 2.0*TWOPI*j/((NPTS-1)) +
                                0.5 - 0.1*((double) random())/pow(2.0,31.0));
        data[i][j].y = cos(data[i][j].x) + 0.5 -
                        0.1*( ((double) i)*random())/pow(2.0,31.0);
        nValues[i]++;
     }
   }


   graphSet(graph, NBUFFERS, NPTS, data, nValues, GraphPoint, "An X-Y Plot",
     "*-*-times-bold-r-*-*-14-*-*",
     "X Axis", "Y Axis",
     "*-*-times-medium-r-*-*-12-*-*",
     "white", "black", NULL, GraphExternal);

   graphDraw(graph);

   graphSetLegend(graph," -- Legend -- ", legendArray);
   graphDrawLegend(graph);


/* ------------------------------------------------------------------ */

/*
 * now register the type of graphX graphic in this shell, so that the
 *   appropriate resize, expose and destroy callbacks can be attached...
 */
   graphAppRegisterGraphic(graphApp,shellNumber,GRAPH_TYPE,graph);


/*
 * and do the Main Loop
 */
   graphAppLoop(graphApp);

}

